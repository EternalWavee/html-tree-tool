#include<iostream>
#include<vector>
#include<stack>
#include<unordered_set>
#include<string>
#include<fstream>
#include<sstream>
#include <unordered_map>
#include <windows.h>
using namespace std;

unordered_set<string> voids = {
        "area","base","br","col","embed","hr","img",
        "input","link","meta","param","source","track","wbr"
};

struct node{
    vector<node*> children;
    string tag;
    int start_pos = 0;
    int end_pos = 0;//这样方便待会输出
    int size = 1;//以当前节点为根节点的子树的所有节点的数目。肯定包含自己所以初值是1
    node* parent;
    node(string t,int s,node* p):tag(t),start_pos(s),parent(p){end_pos = -1;}
    ~node(){for(auto child:children) delete child;}
};

node* build_tree(const string &html){
    node* root = new node("root",0,nullptr);
    stack<node*> st;
    st.push(root);
    int i = 0;
    int n = html.size();
    while(i < n){
        int next_bra = html.find('<', i);
        if(next_bra == string::npos) break;
        // 注释处理 <!-- ... -->
        if(next_bra + 4 < n && html.substr(next_bra, 4) == "<!--"){
            int end_pos = html.find("-->", next_bra + 4);
            i = (end_pos == string::npos ? n : end_pos + 3);
            continue;
        }
        // <!DOCTYPE ...> 或其他声明
        if(next_bra + 1 < n && html[next_bra + 1] == '!' && html.substr(next_bra, 4) != "<!--"){
            int end_pos = html.find_first_of(">", next_bra + 3);
            i = (end_pos == string::npos ? n : end_pos + 1);
            continue;
        }
        // 找到标签结束
        int tag_end = html.find(">", next_bra + 1);
        if(tag_end == string::npos) break;
        string tag_content = html.substr(next_bra + 1, tag_end - next_bra - 1);
        // 处理闭合标签 </xxx>
        if(!tag_content.empty() && tag_content[0] == '/'){
            string tag_name = tag_content.substr(1);
            tag_name = tag_name.substr(0, tag_name.find_first_of(" \t\n\r"));
            if(!st.empty()){
                node* top = st.top();
                if(top->tag == tag_name){
                    top->end_pos = tag_end + 1;
                    st.pop();
                } else if(voids.find(tag_name) != voids.end()){
                    // 闭合的 void 标签，忽略
                    cout << "[警告] 检测到 void 标签 </" << tag_name << ">，已忽略。\n";
                } else {
                    // 栈顶不匹配，尝试修正
                    cout << "[警告] 标签 </" << tag_name << "> 与栈顶 <" << top->tag << "> 不匹配，尝试修正。\n";
                    while(!st.empty() && st.top()->tag != tag_name){
                        st.top()->end_pos = tag_end + 1;
                        st.pop();
                    }
                    if(!st.empty() && st.top()->tag == tag_name){
                        st.top()->end_pos = tag_end + 1;
                        st.pop();
                    }
                }
            }
            i = tag_end + 1;
            continue;
        }
        // 处理开始标签
        string tag = tag_content;
        int space_pos = tag.find_first_of(" \t\n\r");
        if(space_pos != string::npos) tag = tag.substr(0, space_pos);
        // 判断是否自闭标签
        bool isVoid = (voids.find(tag) != voids.end()) || 
                      (!tag_content.empty() && tag_content.back() == '/');
        node* newnode = new node(tag, next_bra, st.top());
        if(!st.empty()) st.top()->children.push_back(newnode);
        if(!isVoid){
            st.push(newnode);
        } else {
            newnode->end_pos = tag_end + 1;
        }
        i = tag_end + 1;
    }
    // 自动补全栈中未闭合的标签
    while(!st.empty()){
        node* unclosed = st.top();
        st.pop();
        if(unclosed->tag != "root"){
            cout << "[警告] 标签 <" << unclosed->tag << "> 缺失闭合，已自动补全。\n";
            unclosed->end_pos = n;
        }
    }
    root->end_pos = n;
    return root;
}

int calcuSize(node* r) {
    if (r == nullptr)return 0; 
    for (auto& child : r->children)r->size += calcuSize(child);
    return r->size;
}

node * build(const string &html){
    node * root = build_tree(html);
    calcuSize(root);
    return root;
}

void printTree(node *n, const string &html, int depth){
    for(int i=0;i<depth;i++) cout << "  ";
    cout << "<" << n->tag << "> [" 
         << n->start_pos << "," << n->end_pos 
         << "]"
         << " size:" << n->size      
         << endl;

    for(auto child:n->children){
        printTree(child, html, depth + 1);
    }
}

//打印string函数
string extract(node* r,const string& html){
    return html.substr(r->start_pos,r->end_pos-r->start_pos);
}

string getParentDir(const string &path){
    int slash = path.find_last_of("/\\");
    if(slash == string::npos) return "./";
    return path.substr(0, slash + 1);
}

// 拿到文件名，例如 "E:/repo/abc.html" -> "abc"
string getBaseName(const string &path){
    int slash = path.find_last_of("/\\");
    int dot = path.find_last_of(".");
    if(dot == string::npos) dot = path.size();
    return path.substr(slash + 1, dot - slash - 1);
}

// [TODO] 功能1：用户输入子树结点数量上界m1和下界m2，则提取出所有满足要求的子树
void extractBySize(node* r,int m1,int m2,vector<node*>& result){
    if(!r)return;
    if(r->parent != nullptr&&r->size>=m1&&r->size<=m2)result.push_back(r);
    for(auto &child:r->children)extractBySize(child,m1,m2,result);
}
// 功能1：按 size
void printBySize(const string & html,node* r,int m1,int m2,const string& inputFile){
    vector<node*> result;
    extractBySize(r,m1,m2,result);
    unordered_map<int,int> counter;
    string dir  = getParentDir(inputFile);
    string base = getBaseName(inputFile);
    cout << "找到size在 [" << m1 << "," << m2 << "] 的子树 "
         << result.size() << " 棵：" << endl << endl;

    for (int i = 0; i < (int)result.size(); i++) {
        node* t = result[i];
        int sz = t->size;
        int idx = ++counter[sz];
        string outName = dir + "module_" + base + "_size_" +
                        to_string(sz) + "_" + to_string(idx) + ".html";

        ofstream fout(outName);
        if(fout.is_open()){
            fout << extract(t, html);
            fout.close();
        }

        cout << "---- 子树 " << (i+1) << " ----" << endl;
        cout << extract(t,html) << endl << endl;
        cout << "已保存为: " << outName << endl;
    }
}

// [TODO] 功能2：用户可以指定某种类型的标签（不包含任何特殊字符），提取标签范围内的所有内容
void extractByTag(node*r,const string&tag,vector<node*>&result){
    if(!r)return;
    if(r->tag==tag)result.push_back(r);
    for(auto& child:r->children)extractByTag(child,tag,result);
}
// 功能2：按 tag
void printByTag(const string&html,node*r,const string&tag,const string& inputFile){
    vector<node*> result;
    extractByTag(r,tag,result);

    unordered_map<int,int> counter; 
    string dir  = getParentDir(inputFile);
    string base = getBaseName(inputFile);

    cout<<"找到以"<<tag<<"为标签的子树"<<result.size()<<"棵："<<endl<<endl;
    for (int i = 0; i < (int)result.size(); i++) {
        node* t = result[i];
        int sz = t->size;
        int idx = ++counter[sz];   // 对相同 size 编号
        string outName = dir + "module_" + base + "_" + tag + "_" + to_string(idx) + ".html";
        ofstream fout(outName);
        if(fout.is_open()){
            fout << extract(t, html);
            fout.close();
        }
        cout << "---- 子树 " << (i+1) << " ----" << endl;
        cout << extract(t,html) << endl << endl;
        cout << "已保存为: " << outName << endl;
    }
}

//[TODO]功能3：将上述两种功能组合使用，提取出根结点为指定标签，结点数量满足要求的模块
void extractByTagSize(node*r,const string&tag,int m1,int m2,vector<node*> &result){
    if(!r)return;
    if(r->parent!=nullptr&&r->size>=m1&&r->size<=m2&&r->tag==tag)result.push_back(r);
    for(auto& child:r->children)extractByTagSize(child,tag,m1,m2,result);
}
// 功能3：按 tag + size
void printByTagSize(const string&html,node *r,const string &tag,int m1,int m2,const string& inputFile){
    vector<node*> result;
    extractByTagSize(r,tag,m1,m2,result);
    unordered_map<int,int> counter;
    string dir  = getParentDir(inputFile);
    string base = getBaseName(inputFile);

    cout<<"找到以"<<tag<<"为标签且大小在["<<m1<<","<<m2<<"]的子树"
        << result.size() <<"棵："<<endl<<endl;

    for (int i = 0; i < (int)result.size(); i++) {
        node* t = result[i];
        int sz = t->size;
        int idx = ++counter[sz];
        string outName = dir + "module_" + base + "_" + tag + "_" +to_string(sz)+ "_" + to_string(idx) + ".html";

        ofstream fout(outName);
        if(fout.is_open()){
            fout << extract(t, html);
            fout.close();
        }
        cout << "---- 子树 " << (i+1) << " ----" << endl;
        cout << extract(t,html) << endl << endl;
        cout << "已保存为: " << outName << endl;
    }
}

// 声明 processFile，用于处理单个文件
void processFile(const string& filePath, const string& html, const string* tag, const int* m1, const int* m2) {
    node* r = build(html);
    cout << "\n===== 处理文件: " << filePath << " =====\n";
    if (tag == nullptr && m1 != nullptr && m2 != nullptr) 
        printBySize(html, r, *m1, *m2, filePath);// 功能1：仅按 size
    else if (tag != nullptr && m1 == nullptr && m2 == nullptr)
        printByTag(html, r, *tag, filePath); // 功能2：仅按 tag
    else if (tag != nullptr && m1 != nullptr && m2 != nullptr)
        printByTagSize(html, r, *tag, *m1, *m2, filePath);// 功能3：tag + size
    else cerr << "参数组合不合法。\n";
    delete r;
}

// 处理目录，遍历所有 html 文件
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
        // 跳过 module_ 开头的文件
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

// 判断路径是文件还是目录
bool isFile(const string& path) {
    DWORD attr = GetFileAttributesA(path.c_str());
    return (attr != INVALID_FILE_ATTRIBUTES && !(attr & FILE_ATTRIBUTE_DIRECTORY));
}

bool isDirectory(const string& path) {
    DWORD attr = GetFileAttributesA(path.c_str());
    return (attr != INVALID_FILE_ATTRIBUTES && (attr & FILE_ATTRIBUTE_DIRECTORY));
}

// 处理单个文件，普通函数版本
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
        // 模式2: path tag
        tag = new string(argv[2]);
    } else if (argc == 4) {
        // 模式1: path min max
        m1 = new int(stoi(argv[2]));
        m2 = new int(stoi(argv[3]));
    } else if (argc == 5) {
        // 模式3: path tag min max
        tag = new string(argv[2]);
        m1 = new int(stoi(argv[3]));
        m2 = new int(stoi(argv[4]));
    } else {
        cerr << "参数数量不合法。\n";
        return 1;
    }
    
    if (isFile(path))handleSingleFile(path, tag, m1, m2);
    else if (isDirectory(path))handleDirectory(path, tag, m1, m2);
    delete tag;
    delete m1;
    delete m2;

    return 0;
}