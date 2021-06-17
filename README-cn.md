# Compiling-Principles-Course
这是我在同济大学编译原理课程（2021春）的大作业

---

## 目录
- [简介](#简介)
- [项目目录结构](#项目目录结构)
- [运行](#运行)
- [维护者](#维护者)
- [License](#License)

---

## 简介
本仓库为包含同济大学2020-2021年编译原理课程的大作业，其功能如下：
1. 词法分析器
2. 语法分析器
3. 语义分析器
4. 中间代码生成器
6. 目标代码生成器

---

## 项目目录结构
```bash
.
│  ir_generator.cpp
│  ir_generator.h
│  lextual.cpp
│  lextual.h
│  main.cpp
│  object_generator.cpp
│  object_generator.h
│  parser.cpp
│  parser.h
│
└─resources
        productions.txt
        right_test.txt
        wrong_test.txt
```

---

## 运行
- 操作系统：Windows10
- 编译器：g++ (i686-win32-dwarf-rev0, Built by MinGW-W64 project) 8.1.0
- 编译命令：g++ *.cpp -o D:\\compiler.exe -g -Wall -static-libgcc -fexec-charset=GBK -std=c++11

---

## 维护者
该仓库目前的维护者为https://github.com/teamwong111

如有侵权或其他问题请通过邮件联系wungjz1@gmail.com

---

## License
[MIT](https://github.com/teamwong111/Compiling-Principles-Course/blob/main/LICENSE)

---