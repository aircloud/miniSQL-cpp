#pragma once
#include <regex>
#include <ctime>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>
#include "NewSQL.hpp"
#include "RecordManager.hpp"
#include "CatalogManager.hpp"
#include "IndexManager.hpp"
#include "ErrorHandle.hpp"


using namespace std;

//	创建表时的内部调用
void API_Create_Table(Table& table);

//	删除表时的内部调用
void API_Drop_Table(string table_name);

//	创建索引时的内部调用
void API_Create_Index(Index& index);

//	删除索引时的内部调用
void API_Drop_Index(string index_name);

//	插入纪录时的内部调用
void API_Insert(Tuple& tuple, bool ifoutput = true);

//	选择纪录时的内部调用
void API_Select(string table_name, Condition_list clist, bool ifoutput = true);

void API_Select(string table_name, Condition_list clist, map<string, int> columns,  bool ifoutput = true);

//	删除纪录时的内部调用
void API_Delete(string table_name, Condition_list clist, bool ifoutput = true);

//  更新记录时的内部调用
void API_Update(string table_name,Condition_list clist,Update_list ulist);

// 字符串分割函数
string split(string str, string pattern, int id);

vector<string> split(string str, string pattern);

vector<string> splitEscapeS(string str, string pattern);

vector<string> multiSplit(string str, string pattern);

vector<string> multiSplitEscapeS(string str, string pattern);

//画表函数
void API_Draw_result(Table& tbl);

void API_Draw_result(Table& tbl,map<string, int> columns);
//测时函数
double calculate_time(long start, long end);

long get_current_time();
