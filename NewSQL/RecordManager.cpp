//
//  RecordManager.cpp
//  NewSQL
//
//  Created by xiaotao Nie on 24/05/2017.
//  Copyright © 2017 xiaotao Nie. All rights reserved.
//

#include "RecordManager.hpp"

//目前delete_values设置成32是一个比较有问题的事情
//这个最好是和selet_values一样设置成vector...
string delete_values[32];
vector<string> select_values;
MBuffer mybuffer;
int delete_num;
int select_num;


std::vector<int> select_offsets;

//这里相当于把一个tuple中的内容变成一个char*字符串
char* Convertvalue(Tuple &tuple)
{
    string value = "";
    
    int tupleLength = (int)tuple.length();
    //std::cout<<tuple.length()+2<<std::endl;

    char *p = (char*)malloc((tupleLength+2)*sizeof(char));
//    char *p = new char[tupleLength+2];
    memset(p, 0, tuple.length()+2);
    char *v = p;
    if(v==NULL)
    std::cout<<"V is empty"<<endl;
    if(p==NULL)
    std::cout<<"P is empty"<<endl;

    int ivalue;
    float fvalue;
    
    for(int i=0;i<tuple.attr_count;i++)
    {
        if(tuple.attrs[i].attr_type == CHAR)
        {
//            std::cout<<tuple.attr_values[i].data()<<endl;
            strcpy(v,tuple.attr_values[i].data());
//            v += tuple.attr_values[i].size();
            v += tuple.attrs[i].attr_len;
        }
        if(tuple.attrs[i].attr_type == INT)
        {
            ivalue = atoi(tuple.attr_values[i].c_str());
            
            memcpy(v, &ivalue, 4);
            v += 4;
        }
        if(tuple.attrs[i].attr_type == FLOAT)
        {
            fvalue = (float) atof(tuple.attr_values[i].c_str());
            memcpy(v,&fvalue,4);
            v += 4;
        }
    }
    //这个地方虽然读取有问题但是实际上应该是没有问题的
    //std::cout<<*(p+4+tuple.attrs[1].attr_len)<<std::endl;
    return p;
}

//insert插入,这个是非常重要的函数
//insert之前,实际上我们都是用
int Insert(Tuple &tuple)
{
    int i,j;
    int offtest;
    char *values;
    int dp;
    int ip;
    char *t;
    int compi;
    float compf;
    bool flag = false;
    string comps;
    char *target = new char[1];
    int comi;
    float comf;
    values = Convertvalue(tuple);
    
    //std::cout<<*(values+4)<<std::endl;

    values[tuple.length()]='1';
    //这大概算是一个标志位?
    
    values[tuple.length()+1]=0;
    string file_name = tuple.table_name + "_table.rec";
    //调用BufferManager的Block_num函数
    int offset = Block_num(file_name)-1;
    if(offset<0)
    {
        //根据这里的意思,如果offset小于0那么就直接插入到第一块中,这里好像是在第一次插入才有的情况
        //但是我感觉这里有点不对的地方,目前先保留看法
        Block* newblock = mybuffer.GetBlock(file_name,0,1);
        memcpy(newblock->record,values,(tuple.length()+1));
        
        delete[] values;
        delete[] target;
        return 0;
    }
    else
    {
        Block* Target;
        for(i=0;i<tuple.attr_count;i++)
        {
            //一个属性一个属性的遍历一下
            //这里大概的意思是,如果是UNIQUE,就要全部都遍历下,有重复的就不能插入成功了
            if(tuple.attrs[i].attr_key_type==UNIQUE)
            {
                ip=0;
                for(j=0;j<i;j++)
                {
                    ip+=tuple.attrs[j].attr_len;//这里是我修改过了
                }
                //ip代表到目前为止的长度
                
                //offset是1.2.3...这样,因为最后还要乘BlockSize,所以这里是一个一个的加
                for(offtest = 0;offtest<=offset;offtest++)
                {
                    Target = mybuffer.GetBlock(file_name,offtest,0);
                    
                    //一个内容块一个内容块的遍历
                    for(dp=0;dp<BlockSize-tuple.length()-1;dp+=tuple.length()+1)
                    {
                        //这个地方我暂时保留疑问
                        //这个应该是和刚才 values[tuple.length()]='1'; 设定的标志位有关系
                        memcpy(target,Target->record+dp+tuple.length(),1);
                        //这个memcpy是干嘛用的还是没有特别看懂
                        if(*target=='1')
                        {
                            if(tuple.attrs[i].attr_type==CHAR)
                            {
                                t = new char[tuple.attrs[i].attr_len+1];
                                memcpy(t,Target->record+dp+ip,tuple.attrs[i].attr_len);
                                t[tuple.attrs[i].attr_len]='\0';
                                comps=t;
                                if(comps==tuple.attr_values[i])
                                {
                                    flag = true;
                                    break;
                                }
                                delete t;
                            }
                            if(tuple.attrs[i].attr_type==INT)
                            {
                                memcpy(&compi,Target->record+dp+ip,4);
                                comi=atoi(tuple.attr_values[i].c_str());
                                if(compi==comi)
                                {
                                    flag = true;
                                    break;
                                }
                            }
                            if(tuple.attrs[i].attr_type==FLOAT)
                            {
                                memcpy(&compf,Target->record+dp+ip,4);
                                comf=atof(tuple.attr_values[i].c_str());
                                if(compf==comf)
                                {
                                    flag = true;
                                    break;
                                }
                            }
                        }
                    }
                    if(flag == true)
                        break;
                }
                if(flag == true)
                    break;
            }
        }
        //直接在后面找地方插入
        if(flag != true)
        {
            Target = mybuffer.GetBlock(file_name,offset,0);
            for(i=tuple.length();i<BlockSize;i+=tuple.length()+1)
            {
                if(Target->record[i] == 0)
                    break;
            }
            //如果可以在当前块插入
            if(i<BlockSize)
            {
                memcpy(Target->record+i-tuple.length(),values,tuple.length()+1);
                Target->accessed = true;
                Target->written = true;
                delete[] values;
                delete[] target;
                return offset;
            }
            //如果当前块不能插入需要开辟新块
            else
            {
                Block* newblock = mybuffer.GetBlock(file_name,offset+1,1);
                memcpy(newblock->record,values,tuple.length()+1);
                delete[] values;
                delete[] target;
                return offset+1;
            }
        }
    }
    
    delete[] values;
    delete[] target;
    return -1;
    //返回-1代表没有插入成功
}

//删除某一个内容
bool Delete_tuple(vector<int> offsetlist, Table table, Condition_list list)
{
    Block * Target;
    int compi;
    delete_num = 0;
    float compf;
    string comps;
    char *target = new char[1];
    int comi;
    int comf;
    int i,j;
    int vi=0;
    int ip;
    string value;
    int dp;
    int offset=0;
    Condition_list::iterator li;
    int Blocknum = 0;
    char* t;
    string file_name = table.table_name + "_table.rec";
    for(int i=0;i<32;i++)
    {
        delete_values[i]="";
    }
    //offsetlist是否为空
    
    //个人感觉offsetlist应该是从B+树中拿来的
    //但是offsetlist这个东西可能为空,如果为空,那就要全都遍历了,如果不为空,那么只遍历指定的offset即可
    
    if(offsetlist.empty()==1)
    {
        Blocknum = Block_num(file_name);
    }
    if(Blocknum != 0 || offsetlist.empty() != 1)
    {
        while(1)
        {
            if(offsetlist.empty()!=1)
            {
                offset = offsetlist[vi];
            }
            if(offset>=Block_num(file_name))
                break;
            Target = mybuffer.GetBlock(file_name,offset,0);
            //一个数据条目一个数据条目的遍历
            for(dp=0;dp<BlockSize-table.length()-1;dp+=(table.length()+1))
            {
                memcpy(target,Target->record+dp+table.length(),1);
                if(*target == '1')
                {
                    if(list.empty()!=1)
                    {
                        //对每一个数据条目,一个条件一个条件的遍历过去,
                        for(li=list.begin();li!=list.end();li++)
                        {
                            //
                            for(i=0;i<table.attr_count;i++)
                            {
                                if(table.attrs[i].attr_name == li->attr_name)
                                    break;
                            }
                            ip = 0;
                            for(j=0;j<i;j++)
                            {
                                ip+=table.attrs[j].attr_len;
                            }
                            if(table.attrs[i].attr_type == CHAR)
                            {
                                t = new char[table.attrs[i].attr_len+1];
                                memcpy(t,Target->record+dp+ip,table.attrs[i].attr_len);
                                t[table.attrs[i].attr_len]='\0';
                                comps = t;
                                delete t;
                                if(li->op_type == "<>")
                                {
                                    if(li->cmp_value == comps)
                                        break;
                                }
                                else if(li->op_type == "=")
                                {
                                    if(li->cmp_value != comps)
                                        break;
                                }
                                else if(li->op_type == ">")
                                {
                                    if(li->cmp_value >= comps)
                                        break;
                                }
                                else if(li->op_type == ">=")
                                {
                                    if(li->cmp_value > comps)
                                        break;
                                }
                                else if(li->op_type == "<")
                                {
                                    if(li->cmp_value <= comps)
                                        break;
                                }
                                else if(li->op_type == "<=")
                                {
                                    if(li->cmp_value < comps)
                                        break;
                                }
                            }
                            else if(table.attrs[i].attr_type == INT)
                            {
                                memcpy(&compi, Target->record+dp+ip, 4);
                                comi = atoi(li->cmp_value.c_str());
                                if(li->op_type == "<>")
                                {
                                    if(comi == compi)
                                        break;
                                }
                                else if(li->op_type == "=")
                                {
                                    if(comi != compi)
                                        break;
                                }
                                else if(li->op_type == ">")
                                {
                                    if(comi>= compi)
                                        break;
                                }
                                else if(li->op_type == ">=")
                                {
                                    if(comi > compi)
                                        break;
                                }
                                else if(li->op_type == "<")
                                {
                                    if(comi <= compi)
                                        break;
                                }
                                else if(li->op_type == "<=")
                                {
                                    if(comi < compi)
                                        break;
                                }
                            }
                            else if(table.attrs[i].attr_type==FLOAT)
                            {
                                memcpy(&compf,Target->record+dp+ip,4);
                                comf = atof(li->cmp_value.c_str());
                                if(li->op_type == "<>")
                                {
                                    if(comf == compf)
                                        break;
                                }
                                else if(li->op_type == "=")
                                {
                                    if(comf != compf)
                                        break;
                                }
                                else if(li->op_type == ">")
                                {
                                    if(comf >= compf)
                                        break;
                                }
                                else if(li->op_type == ">=")
                                {
                                    if(comf > compf)
                                        break;
                                }
                                else if(li->op_type == "<")
                                {
                                    if(comf <= compf)
                                        break;
                                }
                                else if(li->op_type == "<=")
                                {
                                    if(comf < compf)
                                        break;
                                }
                            }
                        }
                    }
                    //如果没有什么条件或者说全部条件都遍历完了(中途没有break,也就是说所有条件都满足)
                    
                    //怎么样把这个删除呢?就直接标志位置0...虽然这种做法不是真正的删除
                    
                    //然后这个deleteList还要给B树删除用,
                    if(li == list.end()||list.empty()==1)
                    {
                        delete_num++;
                        *target = '0';
                        memcpy(Target->record+dp+table.length(),target,1);
                        for(int i=0;i<table.attr_count;i++)
                        {
                            if(table.attrs[i].attr_key_type!=EMPTY)
                            {
                                ip = 0;
                                for(j=0;j<i;j++)
                                {
                                    ip+=table.attrs[j].attr_len;
                                }
                                value = "";
                                if(table.attrs[i].attr_type==CHAR)
                                {
                                    t = new char[table.attrs[i].attr_len+1];
                                    memcpy(t,Target->record+dp+ip,table.attrs[i].attr_len);
                                    t[table.attrs[i].attr_len]='\0';
                                    value = t;
                                    delete_values[i]+=value+'\t';
                                    delete t;
                                }
                                if(table.attrs[i].attr_type==INT)
                                {
                                    t = new char[100];
                                    memcpy(&compi,Target->record+dp+ip,4);
                                    sprintf(t,"%d",compi);
                                    value = t;
                                    delete_values[i]+=value+'\t';
                                    delete t;
                                }
                                if(table.attrs[i].attr_type==FLOAT)
                                {
                                    t = new char[100];
                                    memcpy(&compf,Target->record+dp+ip,4);
                                    sprintf(t,"%f",compf);
                                    value = t;
                                    delete_values[i]+=value+'\t';
                                    delete t;
                                }
                            }
                        }
                    }
                }
            }
            if(offsetlist.empty()==1)
            {
                offset++;
                if(offset==Blocknum)
                    break;
            }
            if(offsetlist.empty()!=1)
            {
                vi++;
                if(vi == offsetlist.size())
                    break;
            }
        }
    }
    delete[] target;
    return true;
}
//选择合适的内容
//offset应该存储的是块偏移
bool Select_tuple(vector<int> offsetlist, Table table, Condition_list list)
{
    Block * Target;
    int compi;
    select_num = 0;
    float compf;
    string comps;
    char *target = new char[1];
    int comi;
    int comf;
    int i,j;
    int vi=0;
    int ip;
    string value;
    int dp;
    int offset=0;
    Condition_list::iterator li;
    int Blocknum = 0;
    char* t;
    string tuplevalue;
    string file_name = table.table_name + "_table.rec";
    select_values.clear();
    select_offsets.clear();
    if(offsetlist.empty()==1)
    {
        Blocknum = Block_num(file_name);
    }
    if(Blocknum != 0 || offsetlist.empty() != 1)
    {
        while(1)
        {
            if(offsetlist.empty()!=1)
            {
                offset = offsetlist[vi];
            }
            if(offset>=Block_num(file_name))
                break;
            Target = mybuffer.GetBlock(file_name,offset,0);
            for(dp=0;dp<BlockSize-table.length()-1;dp+=(table.length()+1))
            {
                memcpy(target,Target->record+dp+table.length(),1);
                if(*target == '1')
                {
                    if(list.empty()!=1)
                    {
                        for(li=list.begin();li!=list.end();li++)
                        {
                            for(i=0;i<table.attr_count;i++)
                            {
                                if(table.attrs[i].attr_name == li->attr_name)
                                    break;
                            }
                            ip = 0;
                            for(j=0;j<i;j++)
                            {
                                ip+=table.attrs[j].attr_len;
                            }
                            if(table.attrs[i].attr_type==CHAR)
                            {
                                t = new char[table.attrs[i].attr_len+1];
                                memcpy(t,Target->record+dp+ip,table.attrs[i].attr_len);
                                t[table.attrs[i].attr_len]='\0';
                                comps = t;
                                delete t;
                                if(li->op_type == "<>")
                                {
                                    if(li->cmp_value == comps)
                                        break;
                                }
                                else if(li->op_type == "=")
                                {
                                    if(li->cmp_value != comps)
                                        break;
                                }
                                else if(li->op_type == ">")
                                {
                                    if(li->cmp_value >= comps)
                                        break;
                                }
                                else if(li->op_type == ">=")
                                {
                                    if(li->cmp_value > comps)
                                        break;
                                }
                                else if(li->op_type == "<")
                                {
                                    if(li->cmp_value <= comps)
                                        break;
                                }
                                else if(li->op_type == "<=")
                                {
                                    if(li->cmp_value < comps)
                                        break;
                                }
                            }
                            else if(table.attrs[i].attr_type==INT)
                            {
                                memcpy(&compi,Target->record+dp+ip,4);
                                comi = atoi(li->cmp_value.c_str());
                                if(li->op_type == "<>")
                                {
                                    if(comi == compi)
                                        break;
                                }
                                else if(li->op_type == "=")
                                {
                                    if(comi != compi)
                                        break;
                                }
                                else if(li->op_type == ">")
                                {
                                    if(comi>= compi)
                                        break;
                                }
                                else if(li->op_type == ">=")
                                {
                                    if(comi > compi)
                                        break;
                                }
                                else if(li->op_type == "<")
                                {
                                    if(comi <= compi)
                                        break;
                                }
                                else if(li->op_type == "<=")
                                {
                                    if(comi < compi)
                                        break;
                                }
                            }
                            else if(table.attrs[i].attr_type==FLOAT)
                            {
                                memcpy(&compf,Target->record+dp+ip,4);
                                comf = atof(li->cmp_value.c_str());
                                if(li->op_type == "<>")
                                {
                                    if(comf == compf)
                                        break;
                                }
                                else if(li->op_type == "=")
                                {
                                    if(comf != compf)
                                        break;
                                }
                                else if(li->op_type == ">")
                                {
                                    if(comf >= compf)
                                        break;
                                }
                                else if(li->op_type == ">=")
                                {
                                    if(comf > compf)
                                        break;
                                }
                                else if(li->op_type == "<")
                                {
                                    if(comf <= compf)
                                        break;
                                }
                                else if(li->op_type == "<=")
                                {
                                    if(comf < compf)
                                        break;
                                }
                            }
                        }
                    }
                    if(li == list.end()||list.empty())
                    {
                        select_num++;
                        tuplevalue = "";
                        for(int i=0;i<table.attr_count;i++)
                        {
                            ip = 0;
                            for(j=0;j<i;j++)
                            {
                                //这边是不是不对
                                if(table.attrs[j].attr_type==CHAR)
                                    ip+=table.attrs[j].attr_len;
                                else  ip+=4;
                            }
                            value = "";
                            if(table.attrs[i].attr_type==CHAR)
                            {
                                int tableAttriAttrlen = table.attrs[i].attr_len;
                                t = new char[tableAttriAttrlen+1];
                                memcpy(t,Target->record+dp+ip,tableAttriAttrlen);
                                t[table.attrs[i].attr_len]='\0';
                                value = t;
                                tuplevalue+=value+'\t';
                                delete t;
                            }
                            if(table.attrs[i].attr_type==INT)
                            {
                                t = new char[100];
                                memcpy(&compi,Target->record+dp+ip,4);
                                sprintf(t,"%d",compi);
                                value = t;
                                tuplevalue+=value+'\t';
                                delete t;
                            }
                            if(table.attrs[i].attr_type==FLOAT)
                            {
                                t = new char[100];
                                memcpy(&compf,Target->record+dp+ip,4);
                                sprintf(t,"%f",compf);
                                value = t;
                                tuplevalue+=value+'\t';
                                delete t;
                            }
                        }
                        select_values.push_back(tuplevalue);
                        select_offsets.push_back(offset);
                    }
                }
            }
            if(offsetlist.empty()==1)
            {
                offset++;
                if(offset==Blocknum)
                    break;
            }
            if(offsetlist.empty()!=1)
            {
                vi++;
                if(vi == offsetlist.size())
                    break;
            }
        }
    }
    delete[] target;
    return true;
}

//这个Select是选择出特定的列
bool Select_tuple(vector<int> offsetlist, Table table, Condition_list list, map<string,int>columns)
{
    Block * Target;
    int compi;
    select_num = 0;
    float compf;
    string comps;
    char *target = new char[1];
    int comi;
    int comf;
    int i,j;
    int vi=0;
    int ip;
    string value;
    int dp;
    int offset=0;
    Condition_list::iterator li;
    int Blocknum = 0;
    char* t;
    string tuplevalue;
    string file_name = table.table_name + "_table.rec";
    select_values.clear();
    select_offsets.clear();
    if(offsetlist.empty()==1)
    {
        Blocknum = Block_num(file_name);
    }
    if(Blocknum != 0 || offsetlist.empty() != 1)
    {
        while(1)
        {
            if(offsetlist.empty()!=1)
            {
                offset = offsetlist[vi];
            }
            if(offset>=Block_num(file_name))
                break;
            Target = mybuffer.GetBlock(file_name,offset,0);
            
            //一个块一个块的循环
            for(dp=0;dp<BlockSize-table.length()-1;dp+=(table.length()+1))
            {
                
                memcpy(target,Target->record+dp+table.length(),1);
                //如果这个块被写入
                if(*target == '1')
                {
                    //如果条件不为空
                    if(list.empty()!=1)
                    {
                        //遍历条件
                        for(li=list.begin();li!=list.end();li++)
                        {
                            for(i=0;i<table.attr_count;i++)
                            {
                                if(table.attrs[i].attr_name == li->attr_name)
                                    break;
                            }
                            ip = 0;
                            for(j=0;j<i;j++)
                            {
                                ip+=table.attrs[j].attr_len;
                            }
                            if(table.attrs[i].attr_type==CHAR)
                            {
                                t = new char[table.attrs[i].attr_len+1];
                                memcpy(t,Target->record+dp+ip,table.attrs[i].attr_len);
                                t[table.attrs[i].attr_len]='\0';
                                comps = t;
                                delete t;
                                if(li->op_type == "<>")
                                {
                                    if(li->cmp_value == comps)
                                        break;
                                }
                                else if(li->op_type == "=")
                                {
                                    if(li->cmp_value != comps)
                                        break;
                                }
                                else if(li->op_type == ">")
                                {
                                    if(li->cmp_value >= comps)
                                        break;
                                }
                                else if(li->op_type == ">=")
                                {
                                    if(li->cmp_value > comps)
                                        break;
                                }
                                else if(li->op_type == "<")
                                {
                                    if(li->cmp_value <= comps)
                                        break;
                                }
                                else if(li->op_type == "<=")
                                {
                                    if(li->cmp_value < comps)
                                        break;
                                }
                            }
                            else if(table.attrs[i].attr_type==INT)
                            {
                                memcpy(&compi,Target->record+dp+ip,4);
                                comi = atoi(li->cmp_value.c_str());
                                if(li->op_type == "<>")
                                {
                                    if(comi == compi)
                                        break;
                                }
                                else if(li->op_type == "=")
                                {
                                    if(comi != compi)
                                        break;
                                }
                                else if(li->op_type == ">")
                                {
                                    if(comi>= compi)
                                        break;
                                }
                                else if(li->op_type == ">=")
                                {
                                    if(comi > compi)
                                        break;
                                }
                                else if(li->op_type == "<")
                                {
                                    if(comi <= compi)
                                        break;
                                }
                                else if(li->op_type == "<=")
                                {
                                    if(comi < compi)
                                        break;
                                }
                            }
                            else if(table.attrs[i].attr_type==FLOAT)
                            {
                                memcpy(&compf,Target->record+dp+ip,4);
                                comf = atof(li->cmp_value.c_str());
                                if(li->op_type == "<>")
                                {
                                    if(comf == compf)
                                        break;
                                }
                                else if(li->op_type == "=")
                                {
                                    if(comf != compf)
                                        break;
                                }
                                else if(li->op_type == ">")
                                {
                                    if(comf >= compf)
                                        break;
                                }
                                else if(li->op_type == ">=")
                                {
                                    if(comf > compf)
                                        break;
                                }
                                else if(li->op_type == "<")
                                {
                                    if(comf <= compf)
                                        break;
                                }
                                else if(li->op_type == "<=")
                                {
                                    if(comf < compf)
                                        break;
                                }
                            }
                        }
                    }
                    //条件遍历结束或者为空,这个时候是需要写入的
                    if(li == list.end()||list.empty())
                    {
                        select_num++;
                        tuplevalue = "";
                        for(int i=0;i<table.attr_count;i++)
                        {
                            
                            //我们在这里添加跳过
                            //注意这个重载函数和原函数的区别主要在这里
                            //这里跳过了不被选择的属性
                            
                            if(!columns[table.attrs[i].attr_name]){
                                continue;
                            }
                            
                            ip = 0;
                            for(j=0;j<i;j++)
                            {
                                //这边是不是不对
                                if(table.attrs[j].attr_type==CHAR)
                                    ip+=table.attrs[j].attr_len;
                                else  ip+=4;
                            }
                            value = "";
                            if(table.attrs[i].attr_type==CHAR)
                            {
                                int tableAttriAttrlen = table.attrs[i].attr_len;
                                t = new char[tableAttriAttrlen+1];
                                memcpy(t,Target->record+dp+ip,tableAttriAttrlen);
                                t[table.attrs[i].attr_len]='\0';
                                value = t;
                                tuplevalue+=value+'\t';
                                delete t;
                            }
                            if(table.attrs[i].attr_type==INT)
                            {
                                t = new char[100];
                                memcpy(&compi,Target->record+dp+ip,4);
                                sprintf(t,"%d",compi);
                                value = t;
                                tuplevalue+=value+'\t';
                                delete t;
                            }
                            if(table.attrs[i].attr_type==FLOAT)
                            {
                                t = new char[100];
                                memcpy(&compf,Target->record+dp+ip,4);
                                sprintf(t,"%f",compf);
                                value = t;
                                tuplevalue+=value+'\t';
                                delete t;
                            }
                        }
                        select_values.push_back(tuplevalue);
                        select_offsets.push_back(offset);
                    }
                }
            }
            if(offsetlist.empty()==1)
            {
                offset++;
                if(offset==Blocknum)
                    break;
            }
            if(offsetlist.empty()!=1)
            {
                vi++;
                if(vi == offsetlist.size())
                    break;
            }
        }
    }
    delete[] target;
    return true;
}

