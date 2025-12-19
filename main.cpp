#include<iostream>
#include<string>
#include "tree.hpp"
#include "file_utils.hpp"
using namespace std;

int main(int argc, char* argv[]) {
    if (argc < 3) {
        cerr << "用法:\n"
             << " 1. mytool 路径 min max\n"
             << " 2. mytool 路径 tag\n"
             << " 3. mytool 路径 tag min max\n";
        return 1;
    }

    string exeName = argv[0];
    if (exeName.find("mytool") == string::npos && exeName.find("MYTOOL") == string::npos) {
        cerr << "请使用 mytool 执行程序。\n";
        return 1;
    }

    string path = argv[1];
    string* tag = nullptr;
    int* m1 = nullptr;
    int* m2 = nullptr;

    if (argc == 3) {
        tag = new string(argv[2]);
    } 
    else if (argc == 4) {
        m1 = new int(stoi(argv[2]));
        m2 = new int(stoi(argv[3]));
    } 
    else if (argc == 5) {
        tag = new string(argv[2]);
        m1 = new int(stoi(argv[3]));
        m2 = new int(stoi(argv[4]));
    } 
    else {
        cerr << "参数数量不合法。\n";
        return 1;
    }

    if (isFile(path)) handleSingleFile(path, tag, m1, m2);
    else if (isDirectory(path)) handleDirectory(path, tag, m1, m2);

    delete tag;
    delete m1;
    delete m2;

    return 0;
}
