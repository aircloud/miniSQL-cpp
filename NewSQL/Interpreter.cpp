//
//  Interpreter.cpp
//  NewSQL
//
//  Created by xiaotao Nie on 28/05/2017.
//  Copyright © 2017 xiaotao Nie. All rights reserved.
//

#include "Interpreter.hpp"

using namespace std;

extern string delete_values[32];
extern vector<string> select_values;

extern int delete_num;
extern int select_num;

extern vector<int> select_offsets;

extern MBuffer m1;
extern MBuffer mybuffer;

extern long beginTime,endTime;

typeMap enterTypeMap;
typeMap CharacterType = {
    {"char",CHAR},
    {"int",INT},
    {"float",FLOAT}
};

//
regex charType ("^(char|CHAR)$");
regex intType ("^(int|INT)$");
regex floatType ("^(float|FLOAT)$");

//之后调用regex_match,这里值得注意的地方就是这个是全局匹配的,也就是说要能匹配到所有字符,只匹配了代检测对象的一部分是不行的(也就是说要有^$开头结尾)
regex createType ("^(\\s)*?[(create)|(CREATE)].*?table.*?\\(.*?\\).*?$",regex::icase);
//目前只实现了一个,其他的待实现

//这里提取出一些可以复用的正则表达式
regex defineType ("^(\\s)*?$",regex::icase);

//这里更改之后又产生新的问题
regex quoteBeginEscape ("^(\\s)*?([\"'`\\(])(\\s)*?");
regex quoteEndEscape ("(\\s)*?[\"'`,\\)](\\s)*?$");

regex EnterEscape ("(\\r|\\n)");

regex beforeParent ("^.*?\\(");
regex afterParent ("\\)(\\s)*?$");

regex symbols ("^[\\(\\),;]*");

//是否包含条件
regex ifContainConditions ("^.*?[<=>]+.*?$");
//是否包含正确的条件
regex ifContainRightConditions ("^[^<>=]*?(>=|<=|<>|<|>|=)[^<>=]*?$");
//是否仅仅是条件
regex ifOnlyConditions ("^[<=>]+$");
//是否仅仅是条件并且条件正确
regex ifOnlyRightConditions ("^(>=|<=|<>|<|>|=)$");
//可以用于regex_search的符号条件
regex allConditions ("(>=|<=|<>|<|>|=)");
/*
 自己当时用的时候忘记了其中+ - * 都需要转义,因此最终结果是错的,
 还一度怀疑是不是js和c++的正则表达式是不是有不一样的,最后看来自己还是想多了
 */
//是否包含运算符号
regex ifContainOperator ("^.*?[\\+\\-\\*/].*?$");
//是否仅仅包含运算符号
regex pureOperator ("^[\\+\\-\\*/]$");
//是否通过条件符号结尾
regex endByOperator ("^.*?[\\+\\-\\*/]$");

string escape(string str){
    return regex_replace(regex_replace(str,quoteBeginEscape,""),quoteEndEscape,"");
}

string findInParent(string str){
    return regex_replace(regex_replace(str,beforeParent,""),afterParent,"");
}

void iniEnterTypeMap(){
    //lowercase:
    enterTypeMap["insert"]=INSERT;
    enterTypeMap["select"]=SELECT;
    enterTypeMap["drop"]=DROP;
    enterTypeMap["create"]=CREATE;
    enterTypeMap["exec"]=EXEC;
    enterTypeMap["delete"]=DELETE;
    enterTypeMap["quit"]=QUIT;
    enterTypeMap["update"]=UPDATE;
    
    //uppercase:
    enterTypeMap["INSERT"]=INSERT;
    enterTypeMap["SELECT"]=SELECT;
    enterTypeMap["DROP"]=DROP;
    enterTypeMap["CREATE"]=CREATE;
    enterTypeMap["EXEC"]=EXEC;
    enterTypeMap["DELETE"]=DELETE;
    enterTypeMap["QUIT"]=QUIT;
    enterTypeMap["UPDATE"]=UPDATE;

}

//整个程序的入口文件
void userLoop(){
    
    //无论是测试程序还是正式程序开始前,都要初始化buffer,否则buffer部分没有办法正常工作
    m1.Init();
    mybuffer.Init();
    
    cout<<WELCOME<<endl;
    
    char s[1000];
    
    string input = "",inputFragment="";
    
    while (true) {
        cout<<PROMPT;
        cin.getline(s,1000,'\n');
        for(int i = 0;i<1000;i++){
            //这个循环是为了防止向左向右这种字符对程序产生额外的影响
            if(s[i]=='\0')break;
            if((int)s[i]>0)input+=+s[i];
        }
        while(input.find(";")>=input.size()){
            cout<<ENTER_SIGN;
            cin.getline(s,1000);
            input+=" ";
            for(int i = 0;i<1000;i++){
                if(s[i]=='\0')break;
                if((int)s[i]>0)input+=+s[i];
            }
        }
        //cout<<input<<endl;
        enterInter(input);
        input = "";
        *s = NULL;
        cin.clear();
        cin.sync();   //清空流
    }
    
}

//REG_ICASE
void enterInter(string input){
    /* 一个不稳定的测试
     string sstr = "create index name_idx on City(Name);";
     string sls (" ,)(");
     cout<<sstr.find(sls.substr(3,4),25)<<endl;
     */
    
    iniEnterTypeMap();
    
    //匹配空格\制表符\换页符
    regex blank ("^(\\s)*?$",regex::icase);
    
    vector<string> commands = split(input,";");
    CommandListNodes p = new CommandList;
    
    int i = 0 ;
    //cout<<"commands.size():"<<commands.size()<<endl;
    for(;i<commands.size();i++){
        //cout<<"commands[i] "<<i<<" "<<commands[i]<<endl;
        //如果分割之后该项目全是空格\制表符\换页符跳过
        //由于我下面定义了新的字符串处理函数splitEscapeS,所以这里其实应该有更好的逻辑,但是现在自己还没有改
        if(regex_match(commands[i],blank))continue;
        
        //cout<<"sizeof(CommandList)"<<sizeof(CommandList)<<endl;
        //cout<<"enterTypeMap.size()"<<enterTypeMap.size()<<endl;
        commands[i] = regex_replace(commands[i], EnterEscape," ");
        //这里可能需要多值分割,
        //cout<<regex_replace(commands[i], EnterEscape,"")<<endl;
        //处理单行注释的情况
        if((commands[i][0] == '-') && (commands[i][1] == '-'))continue;

        p->command = multiSplitEscapeS(commands[i]," ,)(");
        
        //
        
        for(int s = 0 ; s < p->command.size(); s++){
            p->command[s] = escape(p->command[s]);
            //cout<<p->command[s];
        }
        
        p->type=enterTypeMap[(p->command)[0]];
        
        beginTime = get_current_time();
        
        switch (p->type) {
            case CREATE:
                
                if(p->command[1]=="index"){
                    createIndexEnter(p);
                    break;
                }
                
                if (!regex_match(commands[i],createType)){
                    printError(commands[i]);
                }
                createEnter(p);
                break;
            case INSERT:
                insertEnter(p);
                break;
            case DROP:
                
                if(p->command[1]=="index"){
                    dropIndexEnter(p);
                    break;
                }
                
                dropEnter(p);
                break;
            case SELECT:
                selectEnter(p);
                break;
            case DELETE:
                deleteEnter(p);
                break;
            case UPDATE:
                updateEnter(p);
                break;
            case EXEC:
                execEnter(p);
                break;
            case QUIT:
                quitEnter(p);
                //在这里要退出整个程序,调用exit使得程序正常退出
                exit(0);
                
                break;
            default:
                break;
        }
        
//        p=p->next;
    }
    //释放内存
//    if(listNodes){
//        while(listNodes->next){
//            p=listNodes;
//            listNodes=listNodes->next;
//            free(p);
//        }
//    }
    
    /*
     regex e ("select",regex::icase);
     
     string str (input);
     smatch m;
     while (std::regex_search(str,m,e)) {
     for (auto x:m) cout << x << " ";
     cout << std::endl;
     str = m.suffix().str();
     }
     */
}

/*
 #define CREATE 201
 #define INSERT 202
 #define DROP 203
 #define SELECT 205
 #define INSERT 206
 #define DELETE 207
 #define QUIT 401
 */
template <typename T>
int findFirstIndex(vector<T> vec,T target){
    for(int i = 0; i < vec.size(); i++){
        if(vec.at(i) == target)
            return i;
    }
    return -1;
}

//创建函数的入口部分
void createEnter(CommandListNodes commandNode){
    
    //一些初始化操作
    Table tbl1;
    vector<string> thisCommand = commandNode->command;
    
    /*
     * 检测部分,这里的检测越多用户体验越好
     */
    if(thisCommand.size()<8){
        notEnoughParam();
        return;
    }
    
    if(thisCommand[1]!="table"){
        printError(thisCommand[0],thisCommand[1],thisCommand[2]);
        return;
    }
    
    /*
     * 构造数据结构部分
     */
    
    tbl1.table_name = escape(thisCommand[2]);
    tbl1.attr_count = 0;
    int attr_index=0;
    int curr = 3;
    vector<string> keyLists;
    while(curr<thisCommand.size()){
        
        thisCommand[curr] = escape(thisCommand[curr]);
        
        if(thisCommand[curr] == "" || thisCommand[curr]==")"){
            curr++;
            continue;
        }
        
        if(thisCommand[curr] == "primary"){
            //这一行声明主键,一般声明主键都是在最后进行的
            //先要找出主键
            //这个地方到底加1还是2..
            curr+=2;
            int targetLocation = findFirstIndex(keyLists, findInParent(thisCommand[curr]));
            if(targetLocation != -1){
                tbl1.attrs[targetLocation].attr_key_type = PRIMARY;
            } else{
                printError(thisCommand[curr-1],thisCommand[curr],thisCommand[curr+1]);
            }
        }
        
        else{
            //这里处理这一行不是primary的情况
            keyLists.push_back(thisCommand[curr]);
            tbl1.attr_count+=1;
            tbl1.attrs[attr_index].attr_name = thisCommand[curr];
            curr++;
            thisCommand[curr] = escape(thisCommand[curr]);
            tbl1.attrs[attr_index].attr_id=attr_index;
            if(regex_match(thisCommand[curr],charType)){
                //atoi遇到非数字就自动结束转换了。
                tbl1.attrs[attr_index].attr_len = atoi(thisCommand[curr+1].c_str());
                curr++;
                tbl1.attrs[attr_index].attr_type = CHAR;
            } else if(regex_match(thisCommand[curr],intType)){
                tbl1.attrs[attr_index].attr_len = 4;
                tbl1.attrs[attr_index].attr_type = INT;
            } else if(regex_match(thisCommand[curr],floatType)){
                tbl1.attrs[attr_index].attr_len = 4;
                tbl1.attrs[attr_index].attr_type = FLOAT;
            }
            if(curr+1<thisCommand.size()){
                if(escape(thisCommand[curr+1])=="unique"){
                    curr++;
                    tbl1.attrs[attr_index].attr_key_type = UNIQUE;
                } else{
                    tbl1.attrs[attr_index].attr_key_type = NULL;
                }
            }
            attr_index++;
        }
        
        curr++;
    }
    
    API_Create_Table(tbl1);
    
    int i = 0;
    /*
    for(;i<(commandNode->command).size();i++){
        cout<<i<<":"<<escape((commandNode->command)[i])<<"--";
    }
     */
    cout<<endl;
}
void insertEnter(CommandListNodes commandNode){
    
    //这个地方需要用到CatalogManager中定义的Read_Table_Info函数
    Tuple tuple1;
    Table table1;
    vector<string> thisCommand = commandNode->command;
    
    /*
     * 检测部分,这里的检测越多用户体验越好
     */
    
    if(thisCommand.size()<7){
        notEnoughParam();
        return;
    }
    
    if(thisCommand[1]!="into"){
        printError(thisCommand[0],thisCommand[1],thisCommand[2]);
        return;
    }
    
    /*
     * 构造数据结构部分
     */
    
    string tableName = escape(thisCommand[2]);
    table1 = Read_Table_Info(tableName);
    tuple1.table_name = table1.table_name;
    tuple1.attr_count = table1.attr_count;
    //待检测这个是不是table有的时候真的复制好了
    memcpy(tuple1.attrs, table1.attrs, table1.attr_count * sizeof(Attribute));
    
    int attr_index=0;
    int curr = 4;
    
    while(curr<thisCommand.size()){
        
        if(regex_match(thisCommand[curr],symbols)){
            curr++;
            continue;
        }
        
        tuple1.attr_values[attr_index] = escape(thisCommand[curr]);
        curr++;
        attr_index++;
        
    }
    
    API_Insert(tuple1);
    
    /*
    int i = 0;
    for(;i<(commandNode->command).size();i++){
        cout<<(commandNode->command)[i]<<"--";
    }
     */
    cout<<endl;
}
void dropEnter(CommandListNodes commandNode){
    
    vector<string> thisCommand = commandNode->command;
    
    /*
     * 检测部分,这里的检测越多用户体验越好
     */
    
    if(thisCommand.size()<3){
        notEnoughParam();
        return;
    }
    
    if(thisCommand.size()>3){
        overFlowParam(thisCommand[0],thisCommand[1],thisCommand[2]);
        return;
    }
    
    if(thisCommand[1]!="table"){
        printError(thisCommand[0],thisCommand[1],thisCommand[2]);
        return;
    }
    
    /*
     * 构造数据结构部分
     */
    
    string table_name1 = thisCommand[2];
    API_Drop_Table(table_name1);
    
    /*
    int i = 0;
    for(;i<(commandNode->command).size();i++){
        cout<<(commandNode->command)[i]<<"--";
    }
     */
    cout<<endl;
}

void createIndexEnter(CommandListNodes commandNode){
    vector<string> thisCommand = commandNode->command;
    
    /*
     * 检测部分,这里的检测越多用户体验越好
     */
    
    if(thisCommand.size()<6){
        notEnoughParam();
        return;
    }
    
    if(thisCommand.size()>6){
        overFlowParam(thisCommand[1],thisCommand[2],thisCommand[3]);
        return;
    }
    
    if(thisCommand[1]!="index"){
        printError(thisCommand[0],thisCommand[1],thisCommand[2]);
        return;
    }
    
    /*
     * 构造数据结构部分
     */
    
    Index index1;
    index1.attr_name = escape(thisCommand[5]);
    index1.table_name =  escape(thisCommand[4]);
    index1.index_name = escape(thisCommand[2]);
    
    API_Create_Index(index1);
    
    /*
    int i = 0;
    for(;i<(commandNode->command).size();i++){
        cout<<(commandNode->command)[i]<<"--";
    }
     */
    cout<<endl;
    
}

void dropIndexEnter(CommandListNodes commandNode){
    
    vector<string> thisCommand = commandNode->command;
    
    /*
     * 检测部分,这里的检测越多用户体验越好
     */
    
    if(thisCommand.size()<5){
        notEnoughParam();
        return;
    }
    if(thisCommand.size()>5){
        overFlowParam(thisCommand[1],thisCommand[2],thisCommand[3]);
        return;
    }
    
    if(thisCommand[1]!="index"){
        printError(thisCommand[0],thisCommand[1],thisCommand[2]);
        return;
    }
    
    /*
     * 构造数据结构部分
     */
    
    //index 和 indexName 是不一样的 这里怎么获取indexName
    
    //这里其实还有一个问题漏洞,如果有重名index就比较扯淡
    
    Index index1;
    index1.index_name = thisCommand[2];
    
    API_Drop_Index(index1.index_name);
    
    /*
    int i = 0;
    for(;i<(commandNode->command).size();i++){
        cout<<(commandNode->command)[i]<<"--";
    }
     */
    cout<<endl;
}



void selectEnter(CommandListNodes commandNode){
    
    vector<string> thisCommand = commandNode->command;
    
    //如果是select * 的情况比较好处理
    //这里我觉得应该有一个? 名字链表？ map?
    
    /*
     * 检测部分,这里的检测越多用户体验越好
     */
    
    if(thisCommand.size()<4)
        notEnoughParam();
    
    /*
     * 构造数据结构部分
     */
    
    Condition_list clist1;
    
    string tableName1;
    
    Table table1;
    
    if(thisCommand.at(1)=="*"){
        //选择全部的情况
        
        if(thisCommand[2]!="from"){
            printError(thisCommand[1],thisCommand[2],thisCommand[3]);
            return;
        }
        
        tableName1 = thisCommand[3];
        
        int curr = 5;
        int conditionIndex;
        
        //这个时候，如果直接是select * from city这样的,是不执行这个while循环的
        while(curr<thisCommand.size()){
            //我们要处理很多种情况 : id>1   id > 1   id >1   id > 1 id> 1
            if(regex_match(thisCommand[curr], ifContainConditions)){
                smatch m;
                string division;
                
                if(regex_search(thisCommand[curr],m,allConditions)){
                    //包含了符号
                    
                    division = m[0].str();
                    
                    Condition cond11;
                    cond11.op_type = division;
                    
                    vector<string> thisConditionTemp = splitEscapeS(thisCommand[curr], division);
                    if(thisConditionTemp.size() == 1){
                        
                        // id> 1
                        cond11.attr_name = escape(thisConditionTemp[0]);
                        curr++;
                        cond11.cmp_value = escape(thisCommand[curr]);
                        curr++;

                    } else if(thisConditionTemp.size() == 2){
                        
                        //id>1
                        cond11.attr_name = escape(thisConditionTemp[0]);
                        cond11.cmp_value = escape(thisConditionTemp[1]);
                        curr++;

                    } else {
                        //不再预期内的出错情况
                        printError(thisCommand[curr-2],thisCommand[curr-1],thisCommand[curr]);
                        return;
                    }
                    
                    clist1.push_back(cond11);
                }
                else{
                    //非预期情况
                    printError(thisCommand[curr-2],thisCommand[curr-1],thisCommand[curr]);
                    return;
                }
                
            } else {
                
                if(escape(thisCommand[curr])=="and") {
                    curr++;
                    continue;
                }
                
                Condition cond11;
                cond11.attr_name = escape(thisCommand[curr]);
                curr++;
                
                if(regex_match(thisCommand[curr], ifOnlyConditions)){
                    // id > 1
                    if(regex_match(thisCommand[curr], ifOnlyRightConditions)){
                        cond11.op_type = thisCommand[curr];
                        curr++;
                        cond11.cmp_value = escape(thisCommand[curr]);
                        curr++;

                    } else{
                        printError(thisCommand[curr-2],thisCommand[curr-1],thisCommand[curr]);
                        return;
                    }
                } else{
                    // id >1
                    smatch m;
                    string division;
                    
                    if(regex_search(thisCommand[curr],m,allConditions)){
                        
                        division = m[0].str();
                        cond11.op_type = division;
                        
                        vector<string> thisConditionTemp = splitEscapeS(thisCommand[curr], division);
        
                        if(thisConditionTemp.size() == 1){
                            cond11.cmp_value = escape(thisConditionTemp[0]);
                            curr++;
                        } else {
                            //非预期情况
                            printError(thisCommand[curr-2],thisCommand[curr-1],thisCommand[curr]);
                            return;
                        }
                    } else{
                        //非预期情况
                        printError(thisCommand[curr-2],thisCommand[curr-1],thisCommand[curr]);
                        return;
                    }
                }
                
                clist1.push_back(cond11);

            }
        }
        
        //这里该处理了
        API_Select(tableName1,clist1);
        
    } else{
        //选择特定的列
        map<string, int> columns;
        
        int curr = 1;
        int conditionIndex;
        
        while(thisCommand[curr]!="from")
        {
            columns[escape(thisCommand[curr])] = 1;
            curr++;
        }
        
        //目前curr应该是指向from 加一个指向table名字
        curr++;
        
        tableName1 = thisCommand[curr];
        
        table1 = Read_Table_Info(tableName1);
        
        for(int i = 0 ;i<table1.attr_count;i++){
            if(columns.count(table1.attrs[i].attr_name)>0)
                continue;
            columns[table1.attrs[i].attr_name] = 0;
        }
        
        //原来是加一个,但是这里由于要跳过where,实际上是跳过两个
        curr+=2;
        //已经正确设置好要选的队列
        
        while(curr<thisCommand.size()){
            //我们要处理很多种情况 : id>1   id > 1   id >1   id > 1 id> 1
            if(regex_match(thisCommand[curr], ifContainConditions)){
                smatch m;
                string division;
                
                if(regex_search(thisCommand[curr],m,allConditions)){
                    //包含了符号
                    
                    division = m[0].str();
                    
                    Condition cond11;
                    cond11.op_type = division;
                    
                    vector<string> thisConditionTemp = splitEscapeS(thisCommand[curr], division);
                    if(thisConditionTemp.size() == 1){
                        
                        // id> 1
                        cond11.attr_name = escape(thisConditionTemp[0]);
                        curr++;
                        cond11.cmp_value = escape(thisCommand[curr]);
                        curr++;

                    } else if(thisConditionTemp.size() == 2){
                        
                        //id>1
                        cond11.attr_name = escape(thisConditionTemp[0]);
                        cond11.cmp_value = escape(thisConditionTemp[1]);
                        curr++;

                    } else {
                        //不再预期内的出错情况
                        printError(thisCommand[curr-2],thisCommand[curr-1],thisCommand[curr]);
                        return;
                    }
                    
                    clist1.push_back(cond11);
                }
                else{
                    //非预期情况
                    printError(thisCommand[curr-2],thisCommand[curr-1],thisCommand[curr]);
                    return;
                }
                
            } else {
                
                if(escape(thisCommand[curr])=="and") {
                    curr++;
                    continue;
                }
                
                Condition cond11;
                cond11.attr_name = escape(thisCommand[curr]);
                curr++;
                
                if(regex_match(thisCommand[curr], ifOnlyConditions)){
                    // id > 1
                    if(regex_match(thisCommand[curr], ifOnlyRightConditions)){
                        cond11.op_type = thisCommand[curr];
                        curr++;
                        cond11.cmp_value = escape(thisCommand[curr]);
                        curr++;

                    } else{
                        printError(thisCommand[curr-2],thisCommand[curr-1],thisCommand[curr]);
                        return;
                    }
                } else{
                    // id >1
                    smatch m;
                    string division;
                    
                    if(regex_search(thisCommand[curr],m,allConditions)){
                        
                        division = m[0].str();
                        cond11.op_type = division;
                        
                        vector<string> thisConditionTemp = splitEscapeS(thisCommand[curr], division);
                        
                        if(thisConditionTemp.size() == 1){
                            cond11.cmp_value = escape(thisConditionTemp[0]);
                            curr++;

                        } else {
                            //非预期情况
                            printError(thisCommand[curr-2],thisCommand[curr-1],thisCommand[curr]);
                            return;
                        }
                    } else{
                        //非预期情况
                        printError(thisCommand[curr-2],thisCommand[curr-1],thisCommand[curr]);
                        return;
                    }
                }
                
                clist1.push_back(cond11);
                
            }
        }
        
        //选择不同的列 这里调用不同的函数
        API_Select(tableName1,clist1,columns);
        
    }
    
    /*
    int i = 0;
    for(;i<(commandNode->command).size();i++){
        cout<<(commandNode->command)[i]<<"--";
    }
    */
    cout<<endl;
}

void deleteEnter(CommandListNodes commandNode){
    
    vector<string> thisCommand = commandNode->command;

    //delete的格式比select简单一些 例如
    //delete  from  City where ID > 20 and Population < 1000;

    /*
     * 检测部分,这里的检测越多用户体验越好
     */
    //delete也可以一个条件也没有,这个时候就是都删除
    if(thisCommand.size()<4)
        notEnoughParam();
    
    /*
     * 构造数据结构部分
     */
    int curr = 1;
    
    if(thisCommand[curr]=="*"){
        curr++;
    }
    
    if(thisCommand[curr]!="from"){
        printError(thisCommand[curr-2],thisCommand[curr-1],thisCommand[curr]);
        return;
    }
    
    curr++;
    
    Condition_list clist1;
    
    string tableName1;
    
    tableName1 = thisCommand[curr];
    curr+=2;
    //直接加到where的下一个
    
    while(curr<thisCommand.size()){
        //我们要处理很多种情况 : id>1   id > 1   id >1   id > 1 id> 1
        if(regex_match(thisCommand[curr], ifContainConditions)){
            smatch m;
            string division;
            
            if(regex_search(thisCommand[curr],m,allConditions)){
                //包含了符号
                
                division = m[0].str();
                
                Condition cond11;
                cond11.op_type = division;
                
                vector<string> thisConditionTemp = splitEscapeS(thisCommand[curr], division);
                if(thisConditionTemp.size() == 1){
                    
                    // id> 1
                    cond11.attr_name = escape(thisConditionTemp[0]);
                    curr++;
                    cond11.cmp_value = escape(thisCommand[curr]);
                    curr++;
                    
                } else if(thisConditionTemp.size() == 2){
                    
                    //id>1
                    cond11.attr_name = escape(thisConditionTemp[0]);
                    cond11.cmp_value = escape(thisConditionTemp[1]);
                    curr++;
                    
                } else {
                    //不再预期内的出错情况
                    printError(thisCommand[curr-2],thisCommand[curr-1],thisCommand[curr]);
                    return;
                }
                
                clist1.push_back(cond11);
            }
            else{
                //非预期情况
                printError(thisCommand[curr-2],thisCommand[curr-1],thisCommand[curr]);
                return;
            }
            
        } else {
            
            if(escape(thisCommand[curr])=="and") {
                curr++;
                continue;
            }
            
            Condition cond11;
            cond11.attr_name = escape(thisCommand[curr]);
            curr++;
            
            if(regex_match(thisCommand[curr], ifOnlyConditions)){
                // id > 1
                if(regex_match(thisCommand[curr], ifOnlyRightConditions)){
                    cond11.op_type = thisCommand[curr];
                    curr++;
                    cond11.cmp_value = escape(thisCommand[curr]);
                    curr++;
                    
                } else{
                    printError(thisCommand[curr-2],thisCommand[curr-1],thisCommand[curr]);
                    return;
                }
            } else{
                // id >1
                smatch m;
                string division;
                
                if(regex_search(thisCommand[curr],m,allConditions)){
                    
                    division = m[0].str();
                    cond11.op_type = division;
                    
                    vector<string> thisConditionTemp = splitEscapeS(thisCommand[curr], division);
                    
                    if(thisConditionTemp.size() == 1){
                        cond11.cmp_value = escape(thisConditionTemp[0]);
                        curr++;
                    } else {
                        //非预期情况
                        printError(thisCommand[curr-2],thisCommand[curr-1],thisCommand[curr]);
                        return;
                    }
                } else{
                    //非预期情况
                    printError(thisCommand[curr-2],thisCommand[curr-1],thisCommand[curr]);
                    return;
                }
            }
            
            clist1.push_back(cond11);
            
        }
    }

    //执行
    API_Delete(tableName1, clist1);
    /*
    int i = 0;
    for(;i<(commandNode->command).size();i++){
        cout<<(commandNode->command)[i]<<"--";
    }
    */
    cout<<endl;
}

void updateEnter(CommandListNodes commandNode){
    
    //这里值得注意的是set部分和where部分的等号都是既可以和前面联系在一起又可以和后面联系在一起
    
    //之前以为update是可以直接先delete然后再insert,现在看来由于update的选择条件并不是只有一个所以这样并不是很恰当
    //...... 又觉得应该可以..就选出先一个一个delete 然后一个一个insert 并且用同一个conditionList
    /*
     * 检测部分,这里的检测越多用户体验越好
     */
    vector<string> thisCommand = commandNode->command;
    
    if(thisCommand.size()<4)
        notEnoughParam();
    
    if(thisCommand[2]!="SET" && thisCommand[2]!="set"){
        printError(thisCommand[0],thisCommand[1],thisCommand[2]);
        return;
    }

    /*
     * 构造数据结构部分
     */
    
    string tableName1;

    Condition_list clist1;

    Update_list ulist1;
    
    tableName1 = escape(thisCommand[1]);
    
    Table table1 = Read_Table_Info(tableName1);
    
    int curr = 3;
    
    while(curr!=thisCommand.size() && thisCommand[curr]!="where" && thisCommand[curr]!="WHERE"){
        
        UpdateCondition uThisCondition;
        
        vector<string> tobeHandledUpdates;
        
        string toBeHandleExpression = escape(thisCommand[curr]);
        
        //cout<<"toBeHandleExpression.find('='):"<<toBeHandleExpression.find('=')<<endl;
        
        while(toBeHandleExpression.find('=') >= (toBeHandleExpression.size()-1)){
            curr++;
            toBeHandleExpression += escape(thisCommand[curr]);
        }
        
        curr+=1;
        
        if(regex_match(toBeHandleExpression,ifContainOperator)){
            if(!regex_match(toBeHandleExpression.substr(toBeHandleExpression.size()-2,1),ifContainOperator)){
                toBeHandleExpression += escape(thisCommand[curr]);
                curr+=1;
            } else{
               
            }
        }
        
        else if(!regex_match(thisCommand[curr],ifContainOperator)){
            //表明是纯粹赋值
        } else if(regex_match(thisCommand[curr],pureOperator)){
            toBeHandleExpression+=escape(thisCommand[curr]);
            toBeHandleExpression+=escape(thisCommand[curr+1]);
            curr+=2;
        } else if(regex_match(thisCommand[curr],ifContainOperator)){
            toBeHandleExpression+=escape(thisCommand[curr]);
            curr+=1;
        }
        
        //cout<<thisCommand[7]<<endl;
        //cout<<regex_match(thisCommand[7],pureOperator)<<endl;
        
        //到这里应该处理完了
        tobeHandledUpdates = multiSplitEscapeS(toBeHandleExpression,"+-*/=");
        
        if(tobeHandledUpdates.size()==2){
            uThisCondition.UpdateType = DIRECTASSIGN;
            uThisCondition.attr_name = escape(tobeHandledUpdates[0]);
            uThisCondition.UpdateValue = escape(tobeHandledUpdates[1]);
        }
        else{
            uThisCondition.attr_name = escape(tobeHandledUpdates[0]);

            int tag = -1;
            int thisAttrType = 0;
            for(int i = 0 ; i < table1.attr_count; i++){
                if( uThisCondition.attr_name == table1.attrs[i].attr_name){
                    thisAttrType = table1.attrs[i].attr_type;
                }
                
                if(table1.attrs[i].attr_name == escape(tobeHandledUpdates[1])){
                    tag = 2;
                } else if(table1.attrs[i].attr_name == escape(tobeHandledUpdates[2])){
                    tag = 1;
                }
            }
            
            if(!thisAttrType)printf("Dangerous: Unknown attribute name.");

            char oper = toBeHandleExpression[tobeHandledUpdates[0].size()+tobeHandledUpdates[1].size()+1];

            if(tag == -1){
                
                uThisCondition.UpdateType = DIRECTASSIGN;
                uThisCondition.UpdateValue = addByType(escape(tobeHandledUpdates[1]), escape(tobeHandledUpdates[2]),/*运算操作符*/oper,thisAttrType);
                
            }
            
            else{
                uThisCondition.UpdateType = CALCULATEASSIGN;
                uThisCondition.UpdateValue = escape(tobeHandledUpdates[tag]);
                uThisCondition.UpdateOperator = oper;
            }
            
        }
        
        ulist1.push_back(uThisCondition);
        
        //to be continued...
    }
    
    
    //接下来改处理where之后的部分
    curr+=1;
    
    //实际上这个方法不如我上面的这个方法简洁方便，但是目前我这里是直接复制粘贴自己在delete部分写的where
    while(curr<thisCommand.size()){
        //我们要处理很多种情况 : id>1   id > 1   id >1   id > 1 id> 1
        if(regex_match(thisCommand[curr], ifContainConditions)){
            smatch m;
            string division;
            
            if(regex_search(thisCommand[curr],m,allConditions)){
                //包含了符号
                
                division = m[0].str();
                
                Condition cond11;
                cond11.op_type = division;
                
                vector<string> thisConditionTemp = splitEscapeS(thisCommand[curr], division);
                if(thisConditionTemp.size() == 1){
                    
                    // id> 1
                    cond11.attr_name = escape(thisConditionTemp[0]);
                    curr++;
                    cond11.cmp_value = escape(thisCommand[curr]);
                    curr++;
                    
                } else if(thisConditionTemp.size() == 2){
                    
                    //id>1
                    cond11.attr_name = escape(thisConditionTemp[0]);
                    cond11.cmp_value = escape(thisConditionTemp[1]);
                    curr++;
                    
                } else {
                    //不再预期内的出错情况
                    printError(thisCommand[curr-2],thisCommand[curr-1],thisCommand[curr]);
                    return;
                }
                
                clist1.push_back(cond11);
            }
            else{
                //非预期情况
                printError(thisCommand[curr-2],thisCommand[curr-1],thisCommand[curr]);
                return;
            }
            
        } else {
            
            if(escape(thisCommand[curr])=="and") {
                curr++;
                continue;
            }
            
            Condition cond11;
            cond11.attr_name = escape(thisCommand[curr]);
            curr++;
            
            if(regex_match(thisCommand[curr], ifOnlyConditions)){
                // id > 1
                if(regex_match(thisCommand[curr], ifOnlyRightConditions)){
                    cond11.op_type = thisCommand[curr];
                    curr++;
                    cond11.cmp_value = escape(thisCommand[curr]);
                    curr++;
                    
                } else{
                    printError(thisCommand[curr-2],thisCommand[curr-1],thisCommand[curr]);
                    return;
                }
            } else{
                // id >1
                smatch m;
                string division;
                
                if(regex_search(thisCommand[curr],m,allConditions)){
                    
                    division = m[0].str();
                    cond11.op_type = division;
                    
                    vector<string> thisConditionTemp = splitEscapeS(thisCommand[curr], division);
                    
                    if(thisConditionTemp.size() == 1){
                        cond11.cmp_value = escape(thisConditionTemp[0]);
                        curr++;
                    } else {
                        //非预期情况
                        printError(thisCommand[curr-2],thisCommand[curr-1],thisCommand[curr]);
                        return;
                    }
                } else{
                    //非预期情况
                    printError(thisCommand[curr-2],thisCommand[curr-1],thisCommand[curr]);
                    return;
                }
            }
            
            clist1.push_back(cond11);
            
        }
    }
    
    
    API_Update(tableName1, clist1, ulist1);
    //讲道理应该写完update了
    
    /*
    int i = 0;
    for(;i<(commandNode->command).size();i++){
        cout<<(commandNode->command)[i]<<"--";
    }
    */
    cout<<endl;
}

string readFileIntoString(string filename)
{
    ifstream ifile(filename.c_str());
    //将文件读入到ostringstream对象buf中
    ostringstream buf;
    char ch;
    while(buf&&ifile.get(ch))
        buf.put(ch);
    //返回与流对象buf关联的字符串
    return buf.str();  
}  


void execEnter(CommandListNodes commandNode){
    
    //执行文件
    vector<string> thisCommand = commandNode->command;
    string fileName = thisCommand[1];
    
    string FileContent = readFileIntoString(fileName);
    
    //cout<<"FileContent:"<<FileContent<<endl;
    
    enterInter(FileContent);
    
    /*
    int i = 0;
    for(;i<(commandNode->command).size();i++){
        cout<<(commandNode->command)[i]<<"--";
    }
     */
    cout<<endl;
}

void quitEnter(CommandListNodes commandNode){
    //什么也不做就行
    cout<<BYE<<endl;
}

string addByType(string variable1,string variable2,char oper,int type){
    string result;
    
    if(type == CHAR){
        return variable1+variable2;
    } else if(type == INT){
        switch (oper) {
            case '+':
                result = to_string(atoi(variable1.c_str()) + atoi(variable2.c_str()));
                break;
            case '-':
                result = to_string(atoi(variable1.c_str()) - atoi(variable2.c_str()));
                break;
            case '*':
                result = to_string(atoi(variable1.c_str()) * atoi(variable2.c_str()));
                break;
            case '/':
                result = to_string(atoi(variable1.c_str()) / atoi(variable2.c_str()));
                break;
            default:
                break;
        }
    } else if(type == FLOAT){
        switch (oper) {
            case '+':
                result = to_string(atof(variable1.c_str()) + atof(variable2.c_str()));
                break;
            case '-':
                result = to_string(atof(variable1.c_str()) - atof(variable2.c_str()));
                break;
            case '*':
                result = to_string(atof(variable1.c_str()) * atof(variable2.c_str()));
                break;
            case '/':
                result = to_string(atof(variable1.c_str()) / atof(variable2.c_str()));
                break;
            default:
                break;
        }
    }
    
    return result;
}




