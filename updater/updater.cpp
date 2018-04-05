#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")
/*Свойства проекта - > Компоновщик - > Система -> Меняем в графе "Подсистема" (1 строка) на "Windows (/SUBSYSTEM:WINDOWS)"
Компоновщик - > Дополнительно - > Точка входа (1 строка) "mainCRTStartup"*/

#include "updater.h"


std::string downloaded_file;


int IsProcessRunning(char* pName) {
    int id_p;

    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    PROCESSENTRY32 pe;

    for (BOOL r = Process32First(snapshot, &pe); r; r = Process32Next(snapshot, &pe)) {
        if (std::string(pName) == std::string(pe.szExeFile)) {

            id_p = pe.th32ProcessID;
            CloseHandle(snapshot);

            return id_p;
        }
    }

    CloseHandle(snapshot);

    return 0;
}


void write_data(char *buffer, size_t elem_size, size_t elem_cnt, FILE *stream) {
    for (size_t i = 0; i < elem_cnt; ++i) {
        downloaded_file.push_back(buffer[i]);
    }
}


size_t downloadURL(const std::string &url, uint32_t max_tries = 3, uint32_t interval_ms = 100) {
    uint32_t tries = 0;

    downloaded_file = "";
    while ((tries++ < max_tries) && (!downloaded_file.size())) {
        CURL *curl;
        CURLcode res;

        curl = curl_easy_init();
        if (curl) {
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
            curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
            curl_easy_setopt(curl, CURLOPT_USERAGENT, "Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:59.0) Gecko/20100101 Firefox/59.0");
            curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, stdout);

            res = curl_easy_perform(curl);
            curl_easy_cleanup(curl);
        }

        if (!downloaded_file.size()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(interval_ms));
        }
    }

    return downloaded_file.size();
}


void putStrToShittyChar(const std::string str, char* shitty_char) {
    for (size_t i = 0; i < str.size(); ++i) {
        shitty_char[i] = str.at(i);
    }

    shitty_char[str.size()] = '\0';
}


uint32_t loadNewVersion(const std::string user_name, const std::string rep_name, const std::string cur_version, const std::string exe_name) {
    uint32_t result = 0;

    if (downloadURL("https://api.github.com/repos/" + user_name + "/" + rep_name + "/releases/latest", 3, 100)) {
        //std::cout << downloaded_file << std::endl;
        JSONparser::JsonParser parser;

        parser.parseStringJSON(downloaded_file.c_str());

        const uint32_t max_path_len = 8;
        char **path = new char*[max_path_len];

        for (auto i = 0; i < max_path_len; ++i) {
            path[i] = new char[256];
        }

        putStrToShittyChar("tag_name", path[0]);
        putStrToShittyChar("", path[1]);

        if (cur_version != std::string(parser.getVal(path))) {
            //std::cout << "2" << std::endl;
            std::string ret_val = "";
            uint32_t cnt = 0;

            do {
                putStrToShittyChar("assets", path[0]);
                putStrToShittyChar(std::to_string(cnt), path[1]);
                putStrToShittyChar("name", path[2]);
                putStrToShittyChar("", path[3]);
                ret_val = parser.getVal(path);
                //std::cout << "url: " << ret_val << std::endl;
                cnt++;
            } while ((ret_val.size()) && (ret_val != exe_name));

            if ((ret_val.size()) && (ret_val == exe_name)) {
                //std::cout << "3" << std::endl;
                std::string search_file = "https://github.com/" + user_name + "/" + rep_name + "/files/";
                size_t pos = 0;

                putStrToShittyChar("body", path[0]);
                putStrToShittyChar("", path[1]);
                ret_val = parser.getVal(path);

                if (ret_val.size()) {
                    //std::cout << ret_val << std::endl;

                    std::vector<std::string> file_names;
                    std::vector<std::string> link_names;
                    uint32_t real_name = 0, file_link = 0;

                    for (size_t i = 0; i < ret_val.size(); ++i) {
                        if ((ret_val.at(i) == u8'[') && (!file_link)) {
                            real_name = 1;
                            file_names.push_back("");
                        } else if ((ret_val.at(i) == u8']') && (!file_link)) {
                            real_name = 0;
                        } else if ((ret_val.at(i) == u8'(') && (!real_name)) {
                            file_link = 1;
                            link_names.push_back("");
                        } else if ((ret_val.at(i) == u8')') && (!real_name)) {
                            file_link = 0;
                        } else {
                            if (real_name) {
                                file_names.back() += ret_val.at(i);
                            } else if (file_link) {
                                link_names.back() += ret_val.at(i);
                            }
                        }
                    }

                    for (size_t i = 0; ((i < link_names.size()) && (i < file_names.size())); ++i) {
                        downloadURL(link_names.at(i), 3, 100);

                        std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
                        std::wstring wide = converter.from_bytes(file_names.at(i));

                        std::ofstream f1(wide, std::ios::binary);
                        f1 << downloaded_file;
                        f1.close();
                    }
                }

                putStrToShittyChar("assets", path[0]);
                putStrToShittyChar(std::to_string(--cnt), path[1]);
                putStrToShittyChar("browser_download_url", path[2]);
                putStrToShittyChar("", path[3]);
                ret_val = parser.getVal(path);
                if ((ret_val.size()) && (downloadURL(ret_val, 3, 100))) {
                    while (IsProcessRunning(const_cast<char*>(exe_name.c_str()))) {
                        system(("taskkill /f /im " + exe_name).c_str());
                    }

                    std::ofstream f1(exe_name, std::ios::binary);
                    f1 << downloaded_file;
                    f1.close();

                    system(("start " + exe_name).c_str());

                    result = 1;
                }
            }
        }

        for (auto i = 0; i < max_path_len; ++i) {
            if (path[i] != nullptr) {
                delete[] path[i];
            }
        }
        delete[] path;
    }

    return result;
}


uint32_t main(uint32_t argc, char** argv) {
    if (argc > 4) {
        std::string user_name(argv[1]), rep_name(argv[2]), cur_ver(argv[3]), exe_name(argv[4]);

        auto loaded = loadNewVersion(user_name, rep_name, cur_ver, exe_name);
    }

    return 0;
}
