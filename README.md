# HTML Tree Extractor

2025 fall 问题求解与实践大作业
一个基于 C++ 的 HTML 子树提取工具，可根据标签类型或子树节点数量提取 HTML 内容，并保存为独立 HTML 文件。

## 功能

1. 按子树节点数量提取
2. 按标签类型提取
3. 按标签类型 + 子树节点数量提取
4. 自动处理自闭标签，未闭合标签可补全或提示报错

## 使用方法
- mytool 路径/文件 min max # 按节点数量
- mytool 路径/文件 tag # 按标签类型
- mytool 路径/文件 tag min max # 标签 + 节点数量

### 示例
```bash
g++ main.cpp -o mytool #先编译得到mytool工具
mytool.exe E:\repositoryOfCpp\htmlTree1\data\1.html 2 5
mytool.exe E:\repositoryOfCpp\htmlTree1\data p
mytool.exe E:\repositoryOfCpp\htmlTree1\data p 2 5
