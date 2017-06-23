//
//  RecordManager.hpp
//  NewSQL
//
//  Created by xiaotao Nie on 24/05/2017.
//  Copyright © 2017 xiaotao Nie. All rights reserved.
//
#ifndef _RecordManager_H_
#define _RecordManager_H_

#include "BufferManager.hpp"
#include "NewSQL.hpp"


int Insert(Tuple &tuple);

char* Convertvalue(Tuple &tuple);

bool Delete_tuple(vector<int> offsetlist, Table table, Condition_list list);

bool Select_tuple(vector<int> offsetlist, Table table, Condition_list list);

bool Select_tuple(vector<int> offsetlist, Table table, Condition_list list, map<string,int>columns);

#endif
