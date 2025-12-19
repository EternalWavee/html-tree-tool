#ifndef TREE_HPP
#define TREE_HPP

#include<iostream>
#include<vector>
#include<stack>
#include<unordered_set>
#include<string>
#include<fstream>
#include<sstream>
#include<unordered_map>
using namespace std;

unordered_set<string> voids = {
        "area","base","br","col","embed","hr","img",
        "input","link","meta","param","source","track","wbr"
};

struct node{
    vector<node*> children;
    string tag;
    int start_pos = 0;
    int end_pos = 0;
    int size = 1;
    node* parent;
    node(string t,int s,node* p):tag(t),start_pos(s),parent(p){end_pos = -1;}
    ~node(){for(auto child:children) delete child;}
};

node* build_tree(const string &html){
    node* root = new node("root",0,nullptr);
    stack<node*>st;
    st.push(root);
    int i=0;
    int n = html.size();
    while(i<n){
        int next_bra = html.find('<',i);
        if(next_bra==string::npos)break;
        if(next_bra+4<n&&html.substr(next_bra,4)=="<!--"){
            int end_pos = html.find("-->",next_bra+4);
            if(end_pos==string::npos){i = n;break;}
            i = end_pos+3;
            continue;
        }
        if(next_bra+3<n&&html.substr(next_bra,4)!="<!--"&&html[next_bra+1]=='!'){
            int end_pos = html.find_first_of(">",next_bra+3);
            if(end_pos==string::npos){i = n;break;}
            i = end_pos+1;
            continue;
        }
        int tag_start = next_bra;
        int tag_end = html.find(">",tag_start+1);
        if(tag_end==string::npos)break;
        string tag_content = html.substr(tag_start+1,tag_end-tag_start-1);
        if(tag_content.size()>0&&tag_content[0]=='/'){
            string tag_name = tag_content.substr(1);
            tag_name = tag_name.substr(0,tag_name.find_first_of(" \t\n\r"));
            if(!st.empty()&&st.top()->tag==tag_name){
                node* closed = st.top();
                closed->end_pos = tag_end+1;
                st.pop();
            }
            i = tag_end+1;
            continue;
        }
        string tag = tag_content;
        int space_pos = tag.find_first_of(" \t\n\r");
        if(space_pos!=string::npos){tag=tag.substr(0,space_pos);}

        bool isVoid = (voids.find(tag) != voids.end()) ||
                      (!tag_content.empty() && tag_content.back() == '/');

        node *newnode = new node(tag,tag_start,st.top());
        if(!st.empty()){
            st.top()->children.push_back(newnode);
        }
        if(!isVoid){
            st.push(newnode);
        }else{
            newnode->end_pos = tag_end + 1;
        }
        i=tag_end+1;
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

string extract(node* r,const string& html){
    return html.substr(r->start_pos,r->end_pos-r->start_pos);
}

string getParentDir(const string &path){
    int slash = path.find_last_of("/\\");
    if(slash == string::npos) return "./";
    return path.substr(0, slash + 1);
}

string getBaseName(const string &path){
    int slash = path.find_last_of("/\\");
    int dot = path.find_last_of(".");
    if(dot == string::npos) dot = path.size();
    return path.substr(slash + 1, dot - slash - 1);
}

// -------- 功能 1 --------
void extractBySize(node* r,int m1,int m2,vector<node*>& result){
    if(!r)return;
    if(r->parent != nullptr&&r->size>=m1&&r->size<=m2)result.push_back(r);
    for(auto &child:r->children)extractBySize(child,m1,m2,result);
}

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

// -------- 功能 2 --------
void extractByTag(node*r,const string&tag,vector<node*>&result){
    if(!r)return;
    if(r->tag==tag)result.push_back(r);
    for(auto& child:r->children)extractByTag(child,tag,result);
}

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
        int idx = ++counter[sz];
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

// -------- 功能 3 --------
void extractByTagSize(node*r,const string&tag,int m1,int m2,vector<node*> &result){
    if(!r)return;
    if(r->parent!=nullptr&&r->size>=m1&&r->size<=m2&&r->tag==tag)result.push_back(r);
    for(auto& child:r->children)extractByTagSize(child,tag,m1,m2,result);
}

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

#endif
