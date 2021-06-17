#pragma once
#include <iostream>
#include <string.h>
#include <string>
#include <fstream>
using namespace std;

// 分析结果
struct Response {
    string result;
    int state;//state = 1, 代表成功 state = 0, 代表失败
};

// 词法分析器
class Lextual{
public:
    FILE* file = NULL;
    const char Keyword[10][10] = { "int", "void", "if", "else", "while", "return" };
    const char Unary_Operator[11] = { '+','-','*','/','=','<','>'};
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
        s_ano1,//注释DFA状态
        s_ano2,
        s_ano3,
        s_ano4,
        s_ano5,
        s_ano6
    };
    bool isKeyWord(char a[]);//判断关键词
    bool isUnaryOperator(char a);//判断单目运算符
    bool isBinaryOperator(char a[]);//判断双目运算符
    bool isBoundary(char a);//判断界符
    bool isAnotation(char a[]);//判断注释
    bool isNumber(char a);//0-9
    bool isLetter(char a);//a-z, A-Z
    Response dfa(const char* resultname);//dfa的构造和转移
    void readfile(const char* filename);//读文件
    Response getResult(const char* filename, const char* resultname);//产生分析结果
};
