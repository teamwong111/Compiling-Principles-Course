#pragma once
#include <iostream>
#include <string.h>
#include <string>
#include <fstream>
using namespace std;

// 显示分析结果,state = 1, 代表成功 state = 0, 代表失败
struct Response {
    string result;
    int state;
};

class Lextual{
public:
    FILE* file = NULL;
    const char Keyword[10][10] = { "int", "void", "if", "else", "while", "return" };
    const char Unary_Operator[11] = { '+','-','*','/','=','<','>'};//单目运算符
    const char Binary_Operator[20][3] = { "==","<=",">=","!=" };
    const char Boundary[20] = { ',','(',')','{','}',';','#', '[', ']' };
    const char Anotation[3][4] = { "/*","*/","//" };
    const int num_keyword = 6;
    const int num_uo = 7;
    const int num_bo = 4;
    const int num_boundary = 9;
    const int num_anotaion = 3;

    int state;
    enum states {
        s_num,
        s_letter,
        s_reject,
        //注释DFA状态
        s_ano1,
        s_ano2,
        s_ano3,
        s_ano4,
        s_ano5,
        s_ano6
    };

public:
    //判断关键词
    bool isKeyWord(char a[]);
    //判断单目运算符
    bool isUnaryOperator(char a);
    //判断双目运算符
    bool isBinaryOperator(char a[]);
    //判断界符
    bool isBoundary(char a);
    //判断注释
    bool isAnotation(char a[]);
    //0-9
    bool isNumber(char a);
    //a-z, A-Z
    bool isLetter(char a);
    //dfa的构造和转移
    Response dfa(const char* resultname);
    //读文件
    void readfile(string filename);
    //产生分析结果
    Response getResult(string filename, const char* resultname);
};
