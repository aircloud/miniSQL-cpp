//
//  NewSQL
//
//  Created by xiaotao Nie on 23/05/2017.
//  Copyright © 2017 xiaotao Nie. All rights reserved.
//
//#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file

#if 0

#include "catch.hpp"
#include "iostream"
#include "NewSQL.hpp"
#include "API.hpp"

using namespace std;

extern string delete_values[32];
extern vector<string> select_values;

extern int delete_num;
extern int select_num;

extern vector<int> select_offsets;

extern MBuffer m1;
extern MBuffer mybuffer;

/*

TEST_CASE( "Test Create Table", "[API1]" ) {
    //无论是测试程序还是正式程序开始前,都要初始化buffer,否则buffer部分没有办法正常工作
    m1.Init();
    mybuffer.Init();
    
    cout<<"Module:Test API Create Table begin:"<<endl;

    Table tbl1;
    
    tbl1.table_name = "test_table715";
    tbl1.attr_count = 3;
    tbl1.attrs[0].attr_name="id";
    tbl1.attrs[0].attr_type=INT;
    tbl1.attrs[0].attr_key_type=PRIMARY;
    tbl1.attrs[0].attr_len=4;
    
    tbl1.attrs[1].attr_name="name";
    tbl1.attrs[1].attr_type=CHAR;
    tbl1.attrs[1].attr_key_type=EMPTY;
    tbl1.attrs[1].attr_len=31;
    
    tbl1.attrs[2].attr_name="age";
    tbl1.attrs[2].attr_type=INT;
    tbl1.attrs[2].attr_key_type=EMPTY;
    tbl1.attrs[2].attr_len=4;
    
    API_Create_Table(tbl1);
    
    cout<<"Module:Test API Create Table end"<<endl;
}

*/

/*
TEST_CASE("Test Insert Table","[API2]"){
    m1.Init();
    mybuffer.Init();
    
    cout<<"Module:Test API Insert Table begin:"<<endl;
    
    Tuple tuple1;
    //tuple继承了table的public属性并且用到了,所以这边我先模拟一下
    //实际上可以用memcpy来搞这个事情
    
    tuple1.table_name = "test_table715";
    tuple1.attr_count = 3;
    tuple1.attrs[0].attr_name="id";
    tuple1.attrs[0].attr_type=INT;
    tuple1.attrs[0].attr_key_type=PRIMARY;
    tuple1.attrs[0].attr_len=4;
    
    tuple1.attrs[1].attr_name="name";
    tuple1.attrs[1].attr_type=CHAR;
    tuple1.attrs[1].attr_key_type=EMPTY;
    tuple1.attrs[1].attr_len=31;
    
    tuple1.attrs[2].attr_name="age";
    tuple1.attrs[2].attr_type=INT;
    tuple1.attrs[2].attr_key_type=EMPTY;
    tuple1.attrs[2].attr_len=4;
    
    //插入的时候我们需要注意的是,我们这里基本上都转化成字符串插入了,int和浮点数什么的,要写成字符串,否则会出错
    tuple1.attr_values[0]="51";
    tuple1.attr_values[1]="xiaotao3";
    tuple1.attr_values[2]="51";
    
    API_Insert(tuple1);
    
    Condition_list clist1;
    API_Select("test_table715",clist1);
    cout<<"Module:Test API Insert Table end."<<endl;
}

*/

 //在这个测试用例基础上增加了对选择部分列的测试:通过
TEST_CASE("Test Select Table","[API3]"){
    m1.Init();
    mybuffer.Init();
    
    map<string,int> columnSelect = {
        {"id",0},
        {"name",1},
        {"age",0}
    };
    
    cout<<"Module:Test API Select Table begin:"<<endl;
    
    Condition_list clist0;
    API_Select("test_table715",clist0);
    
    Condition_list clist1;
    Condition cond1;
    cond1.attr_name = "id";
    cond1.op_type = ">";
    cond1.cmp_value = "41";
    clist1.push_back(cond1);
    Condition cond2;
    cond2.attr_name = "age";
    cond2.op_type = "=";
    cond2.cmp_value = "45";
    clist1.push_back(cond2);
//    API_Select("test_table715",clist1);
    
    Condition_list clist2;
    Condition cond21;
    cond21.attr_name = "id";
    cond21.op_type = ">";
    cond21.cmp_value = "41";
    clist2.push_back(cond21);
    Condition cond22;
    cond22.attr_name = "id";
    cond22.op_type = "=";
    cond22.cmp_value = "49";
    clist2.push_back(cond22);

    Update_list ulist1;
    UpdateCondition ucond1;
    ucond1.attr_name="age";
    ucond1.UpdateType=DIRECTASSIGN;
    ucond1.UpdateValue="500";
    ulist1.push_back(ucond1);
    
//    API_Update("test_table715", clist2, ulist1);
    //API_Select("test_table715",clist2);

    
    cout<<"Module:Test API Select Table end."<<endl;
}


/*
TEST_CASE("Test Delete Table","[API4]"){
    m1.Init();
    mybuffer.Init();
    
    cout<<"Module:Test API Delete Table begin:"<<endl;
    
    Condition_list clist1;
    Condition cond11;
    cond11.attr_name = "id";
    cond11.op_type = ">";
    cond11.cmp_value = "41";
    clist1.push_back(cond11);
    Condition cond12;
    cond12.attr_name = "id";
    cond12.op_type = "=";
    cond12.cmp_value = "51";
    clist1.push_back(cond12);
    API_Delete("test_table715",clist1);
    
    Condition_list clist0;
    API_Select("test_table715",clist0);

    cout<<"Module:Test API Delete Table end."<<endl;
}
*/
/*
TEST_CASE("Test create Index","[API5]"){
    m1.Init();
    mybuffer.Init();
    
    cout<<"Module:Test API create Index begin:"<<endl;
    
    Index index1;
    index1.attr_name = "age";
    index1.table_name = "test_table715";
    index1.index_name = index1.attr_name;
    
    API_Create_Index(index1);
    
    cout<<"Module:Test API create Index end."<<endl;
}
*/
/*
TEST_CASE("Test Drop Index","[API6]"){
    m1.Init();
    mybuffer.Init();
    
    cout<<"Module:Test API create Index begin:"<<endl;
    
    Index index1;
    index1.attr_name = "age";
    index1.table_name = "test_table715";
    index1.index_name = index1.attr_name;
    
    API_Drop_Index(index1.index_name);
    
    cout<<"Module:Test API Drop Index end."<<endl;
}
*/
/*
TEST_CASE("Test Drop Table","[API7]"){
    m1.Init();
    mybuffer.Init();
    cout<<"Module:Test API Drop Table begin:"<<endl;
    
    string table_name1 = "test_table2";
    API_Drop_Table(table_name1);
    
    cout<<"Module:Test API Drop Table end."<<endl;

}
*/


#endif
