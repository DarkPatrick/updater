#pragma once


#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <memory>


namespace JSONparser {
    /// <summary>����� ��� �������� ������� JSON �����, ��� ��������������� �������� � ���������� ������ � ���.</summary>
    class JsonParser {
        private:
        /// <summary>��������� ������ ��� �������� ��������������� ����� JSON ����� � �������� ������� � ���.</summary>
        struct TreeNode {
            std::string name;
            std::vector<TreeNode> children;
        };

        public:
        __declspec(dllexport) __stdcall
        JsonParser();
        /// <summary>�������� JSON ������� �� ���������� �����.</summary>
        __declspec(dllexport) __stdcall
        JsonParser(const char* file_name);
        __declspec(dllexport) __stdcall
        ~JsonParser();
        /// <summary>������� JSON ������.</summary>
        /// <param name='str_name'>��� ����������, ���������� JSON �����.</param>
        __declspec(dllexport)
        uint32_t __stdcall parseStringJSON(const char* str_name);
        /// <summary>�������� ���������� JSON ����� � ��� �������.</summary>
        /// <param name='file_name'>��� ������������ JSON �����.</param>
        __declspec(dllexport)
        uint32_t __stdcall parseFileJSON(const char* file_name);
        /// <summary>���������� �������� ����.</summary>
        /// <param name='path'>������ ���� � ���� ������� �� ��������� ��� �����</param>
        __declspec(dllexport)
        char* __stdcall getVal(char** path);

        private:
        uint32_t addWordToTree(const std::vector<std::string> &path);

        private:
        TreeNode name_tree;
    };
}
