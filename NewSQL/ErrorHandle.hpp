//
//  ErrorHandle.hpp
//  NewSQL
//
//  Created by xiaotao Nie on 29/05/2017.
//  Copyright © 2017 xiaotao Nie. All rights reserved.
//

#ifndef ErrorHandle_hpp
#define ErrorHandle_hpp

#include <iostream>

#include <string>

using namespace std;

void printError(string first);

void printError(string first, string second);

void printError(string first, string second, string third);

void notEnoughParam();

void overFlowParam();

void overFlowParam(string first,string second,string third);

#endif /* ErrorHandle_hpp */
