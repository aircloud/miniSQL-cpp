//
//  testBuffer1.cpp
//  NewSQL
//
//  Created by xiaotao Nie on 23/05/2017.
//  Copyright © 2017 xiaotao Nie. All rights reserved.
//
#if 0

#include "catch.hpp"
#include "iostream"
#include "BufferManager.hpp"

TEST_CASE( "Test Buffer Manager ", "[BufferManager]" ) {
    cout<<"test1"<<endl;

    MBuffer m1;
    m1.Init();
    REQUIRE(Block_num("/Users/hh/NewSQL/test1.txt")==0);
    
    cout<<sizeof(m1.Buffer[0]->record)<<endl;
    cout<<m1.Buffer[0]->record+4<<endl;

    //REQUIRE("" == NULL );空字符串和NULL也不相等
}

#endif
