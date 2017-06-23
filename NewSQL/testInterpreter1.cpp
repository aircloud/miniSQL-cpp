//
//  testInterpreter1.cpp
//  NewSQL
//
//  Created by xiaotao Nie on 29/05/2017.
//  Copyright © 2017 xiaotao Nie. All rights reserved.
//

#if 0
#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file

#include <stdio.h>
#include "catch.hpp"
#include "iostream"
#include "Interpreter.hpp"

TEST_CASE( "Test Interpreter Manager ", "[BufferManager]" ) {
    cout<<"Test Interpreter Manager Begin"<<endl;
    enterInter("select ID,Name from City where ID > 10 and ID < 90;");
    cout<<"Test Interpreter Manager End"<<endl;
}


#endif
