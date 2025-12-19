#ifndef FILE_UTILS_HPP
#define FILE_UTILS_HPP

#include<iostream>
#include<fstream>
#include<sstream>
#include<string>
#include<windows.h>
using namespace std;

#include "tree.hpp"

bool isFile(const string& path) {
    DWORD attr = GetFileAttributesA(path.c_str());
    return (attr != INVALID_FILE_ATTRIBUTES && !(attr & FILE_ATTRIBUTE_DIRECTORY));
}

bool isDirectory(const string& path) {
    DWORD attr = GetFileAttributesA(path.c_str());
    return (attr != INVALID_FILE_ATTRIBUTES && (attr & FILE_ATTRIBUTE_DIRECTORY));
}

void processFile(const string& filePath, const string& html, const string* tag, const int* m1, const int* m2) {
    node* r = build(html);
    cout << "\n===== 处理文件: " << filePath << " =====\n";

    if (tag == nullptr && m1 != nullptr && m2 != nullptr) 
        printBySize(html, r, *m1, *m2, filePath);
    else if (tag != nullptr && m1 == nullptr && m2 == nullptr)
        printByTag(html, r, *tag, filePath);
    else if (tag != nullptr && m1 != nullptr && m2 != nullptr)
        printByTagSize(html, r, *tag, *m1, *m2, filePath);
    else cerr << "参数组合不合法。\n";

    delete r;
}

void handleSingleFile(const string& filePath, const string* tag, const int* m1, const int* m2) {
    ifstream file(filePath);
    if (!file.is_open()) {
        cerr << "无法打开文件: " << filePath << endl;
        return;
    }
    stringstream buffer;
    buffer << file.rdbuf();
    file.close();
    string html = buffer.str();

    processFile(filePath, html, tag, m1, m2);
}

void handleDirectory(const string& dirPath, const string* tag, const int* m1, const int* m2) {
    WIN32_FIND_DATAA findData;
    string searchPath = dirPath + "\\*.html";
    HANDLE hFind = FindFirstFileA(searchPath.c_str(), &findData);
    if (hFind == INVALID_HANDLE_VALUE) {
        cerr << "无法打开目录: " << dirPath << endl;
        return;
    }
    do {
        string fileName = findData.cFileName;
        if (fileName.rfind("module_", 0) == 0) continue;
        string filePath = dirPath + "\\" + fileName;

        ifstream file(filePath);
        if (!file.is_open()) {
            cerr << "无法打开文件: " << filePath << endl;
            continue;
        }
        stringstream buffer;
        buffer << file.rdbuf();
        file.close();
        string html = buffer.str();

        processFile(filePath, html, tag, m1, m2);

    } while (FindNextFileA(hFind, &findData));
    FindClose(hFind);
}

#endif
