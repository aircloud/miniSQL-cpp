#pragma once

#include <algorithm>
#include <list>
#include <vector>
#include <map>
#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include <cstring>
#include <cstdarg>
//#include "Interpreter/Lex/Analysis.hpp"

using namespace std;

#define MAX_ONE_LINE 256

#define WELCOME     "newSQL is started."
#define PROMPT      "\nnewSQL> " //提示
#define ENTER_SIGN  "\n     -> "
#define BYE     "Bye."

#define PRIMARY 267
#define INT  276
#define FLOAT  277
#define CHAR  278
#define UNIQUE 270
#define EMPTY 283

#define PRE "/Users/hh"

#define CREATE 201
#define INSERT 202
#define DROP 203
#define SELECT 205
#define DELETE 207
#define UPDATE 208

#define EXEC 301
#define QUIT 401

#define CREATEINDEX 601
#define DROPINDEX 603

#define DIRECTASSIGN 901
#define CALCULATEASSIGN 902

//这个只是临时方案

//	用于描述表中一个属性在表中的具体信息
struct Attribute
{
    string attr_name;
    int attr_type;	//属性的数据类型，分别为CHAR, FLOAT, INT
    int attr_key_type;//属性的主键类型，分别为PRIMARY, UNIQUE, NULL(EMPTY)
    int attr_len; 	//属性所存放的数据的长度，如果不是CHAR，则长度标记为4 这里有一个疑问就是FLOAT到底是多少...之前自己一直没有测试FLOAT这一块
    int attr_id;    //属性在表中是第几个
};

//	用于描述表的信息
struct Table
{
    string table_name;  //表名
    int attr_count;			//表中属性的总个数
    Attribute attrs[32];	//表的所有属性列表, 最多32个属性
    //return primary key id
    int getPrimaryKeyId() {
        for (int i = 0; i < attr_count; ++i)
        {
            if (attrs[i].attr_key_type == PRIMARY)
            {
                return i;
            }
        }
        //if no primary key
        return -1;
    }
    
    int searchAttrId(string att_name) {
        for (int i = 0; i < attr_count; ++i)
        {
            if (attrs[i].attr_name == att_name)
            {
                return i;
            }
        }
        //if no primary key
        return -1;
    }
    
    int length()
    {
        int len = 0;
        for (int i = 0; i < attr_count; ++i)
        {
            len += attrs[i].attr_len;
        }
        return len;
    }
};

//	用于描述判断条件的信息
struct Condition
{
    string attr_name;	//条件所对应的属性名
    string op_type;		//条件所用到的比较模式，分别为"<>", "=", ">=", "<=", "<", ">"
    string cmp_value;	//条件所需要进行比较的值
};
typedef list<Condition> Condition_list;


struct UpdateCondition
{
    string attr_name;
    int UpdateType;
    char UpdateOperator;
    string UpdateValue;
};

typedef vector<UpdateCondition> Update_list;

//	用于描述索引信息
struct Index
{
    string index_name;
    string table_name;
    string attr_name;	//	索引所对应的属性
};

struct Tuple: public Table
{
    string attr_values[32];
};

typedef struct CommandNode {
    int type;
    vector<string> command;
    CommandNode *next;
} CommandList,*CommandListNodes;

//暂时没有用到
typedef struct Node
{
    int coeff;
    int expo;
    Node *next;
}listNode,*list;

typedef map<string,int> typeMap;



