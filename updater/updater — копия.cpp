//#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")
/*Свойства проекта - > Компоновщик - > Система -> Меняем в графе "Подсистема" (1 строка) на "Windows (/SUBSYSTEM:WINDOWS)"
Компоновщик - > Дополнительно - > Точка входа (1 строка) "mainCRTStartup"*/

#include "updater.h"
#include "JSONparser.h"


std::string downloaded_file;


void write_data(char *buffer, size_t elem_size, size_t elem_cnt, FILE *stream) {
    //fwrite(buffer, elem_size, elem_cnt, stream);
    for (size_t i = 0; i < elem_cnt; ++i) {
        downloaded_file.push_back(buffer[i]);
    }
}


std::string findStrInDblQuotes(const std::string file, size_t &start_pos, const char terminate_symb = u8'\0') {
    std::string ret_val = "";

    while ((start_pos < file.size()) && (file.at(start_pos) != u8'\"') && (file.at(start_pos) != terminate_symb)) {
        start_pos++;
    }
    if (file.at(start_pos) != terminate_symb) {
        start_pos++;
        while ((start_pos < file.size()) && (file.at(start_pos) != u8'\"')) {
            ret_val += file.at(start_pos);
            start_pos++;
        }
        start_pos++;
    }

    return ret_val;
}


std::vector<std::string> getJsonParam(const std::string file, const std::string sector_name, const std::string param_name, size_t &start_pos) {
    std::string search_text = u8'\"' + sector_name + u8"\":{";
    std::vector<std::string> ret_list;

    start_pos = file.find(search_text, start_pos);
    if (start_pos != std::string::npos) {
        start_pos += search_text.size();
    }

    auto pos = start_pos;

    if (pos != std::string::npos) {
        auto open_br = 1;

        pos += search_text.size();
        while ((pos < file.size()) && (open_br)) {
            if (file.at(pos) == u8'{') {
                open_br++;
            } else if (file.at(pos) == u8'}') {
                open_br--;
            }
            pos++;
        }

        if (!open_br) {
            auto end_pos = pos;

            search_text = u8'\"' + param_name + u8"\":";
            pos = file.find(search_text, start_pos);
            if (pos < end_pos) {
                pos += search_text.size();
                if (file.at(pos) == u8'\"') {
                    std::string param_val = "";

                    pos++;
                    while ((pos < file.size()) && (file.at(pos) != u8'\"')) {
                        param_val += file.at(pos);
                        pos++;
                    }
                    ret_list.push_back(param_val);
                } else if (file.at(pos) == u8'[') {
                    std::string param;

                    do {
                        param = findStrInDblQuotes(file, pos, u8']');
                        if (param.size()) {
                            ret_list.push_back(param);
                        }
                    } while (param.size());
                }
            }
        }
    }

    return ret_list;
}


uint32_t loadNewVersion(const std::string user_name, const std::string rep_name, const std::string cur_version, const std::string sys_name) {
    CURL *curl;
    CURLcode res;

    curl = curl_easy_init();
    if (curl) {
        //std::string url = "https://raw.githubusercontent.com/" + user_name + "/" + rep_name + "/master/version.json";
        //std::string url = "https://github.com/DarkPatrick/gld_bootloader/releases/download/1.0.0/gld_boot_x86.exe";
        //std::string url = "https://github.com/DarkPatrick/gld_bootloader/releases/latest";
        std::string url = "https://api.github.com/repos/DarkPatrick/gld_bootloader/releases/latest";

        std::cout << url.c_str() << std::endl;
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:59.0) Gecko/20100101 Firefox/59.0");
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, stdout);

        res = curl_easy_perform(curl);

        /*
        if (res != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        }
        //*/
        curl_easy_cleanup(curl);

        std::ofstream f1("res.txt", std::ios::binary);
        f1 << downloaded_file;
        f1.close();

        size_t start_pos = 0;
        std::string version = "";

        try {
            std::string new_sys_name;

            do {
                start_pos = downloaded_file.find(u8"\"version\":{", start_pos);
                if (start_pos == std::string::npos) {
                    return 0;
                }

                auto inner_pos = start_pos;
                std::vector<std::string> params = getJsonParam(downloaded_file, u8"version", u8"system", inner_pos);

                new_sys_name = params.at(0);
                if (new_sys_name == sys_name) {
                    inner_pos = start_pos;
                    params = getJsonParam(downloaded_file, u8"version", u8"num", inner_pos);
                    if (params.at(0) != cur_version) {
                        inner_pos = start_pos;
                        params = getJsonParam(downloaded_file, u8"version", u8"file_list", inner_pos);
                        if (params.size()) {
                            for (auto i : params) {
                                //std::cout << i << std::endl;
                            }

                            return 1;
                        }
                    }
                }
                start_pos++;
            } while ((new_sys_name != sys_name) && (start_pos < downloaded_file.size()));
        } catch (...) {
            return 0;
        }
    }

    return 0;
}


uint32_t main(uint32_t argc, char** argv) {
    if (argc > 4) {
        std::string user_name(argv[1]), rep_name(argv[2]), cur_ver(argv[3]), exe_name(argv[4]);

        auto loaded = loadNewVersion(user_name, rep_name, cur_ver, exe_name);
    }

    //JSONparser::JsonParser parser;
    //parser.parseJSON("gj");

    return 0;
}
