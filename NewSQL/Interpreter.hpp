//
//  Interpreter.hpp
//  NewSQL
//
//  Created by xiaotao Nie on 28/05/2017.
//  Copyright © 2017 xiaotao Nie. All rights reserved.
//

#ifndef Interpreter_hpp
#define Interpreter_hpp

#include <stdio.h>
#include "API.hpp"
#include "NewSQL.hpp"
#include "ErrorHandle.hpp"
#include <cassert>
#include <vector>
#include <string>
#include <regex>
#include <fstream>
#include <sstream>

void userLoop();

void enterInter(string input);

string readFileIntoString(char * filename);

string escapeQuotes(string str);
string escape(string str);
string findInParent(string str);

void createEnter(CommandListNodes commandNode);
void insertEnter(CommandListNodes commandNode);
void dropEnter(CommandListNodes commandNode);
void selectEnter(CommandListNodes commandNode);
void deleteEnter(CommandListNodes commandNode);
void updateEnter(CommandListNodes commandNode);
void execEnter(CommandListNodes commandNode);
void quitEnter(CommandListNodes commandNode);

void createIndexEnter(CommandListNodes commandNode);
void dropIndexEnter(CommandListNodes commandNode);

string addByType(string variable1,string variable2,char oper,int type);

#endif /* Interpreter_hpp */
