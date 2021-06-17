# Compiling-Principles-Course
[中文文档](https://github.com/teamwong111/Compiling-Principles-Course/blob/main/README-cn.md)

It is my term porject repo of Compiling Principles Course(2021 spring) in Tongji University

---

## Contents
- [Introduction](#Introduction)
- [Structure](#Structure)
- [Running](#Running)
- [Defenders](#Defenders)
- [License](#License)

---

## Introduction
This repo contain the term project of Tongji University's Compiling Principles Course in 2020-2021.The features are as follows:
1. Lexical Analyzer
2. Parser
3. Semantic Analyzer
4. Intermediate Code Generator
5. Object Code Generator

---

## Structure
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

## Running
- OS：Windows10
- Compiler：g++ (i686-win32-dwarf-rev0, Built by MinGW-W64 project) 8.1.0
- Compile command：g++ *.cpp -o D:\\compiler.exe -g -Wall -static-libgcc -fexec-charset=GBK -std=c++11

---

## Defenders
The repo is currently owned by https://github.com/teamwong111 maintain

If I have infringement or you have any questions, please contact me by email wungjz1@gmail.com

---

## License
[MIT](https://github.com/teamwong111/Compiling-Principles-Course/blob/main/LICENSE)

---