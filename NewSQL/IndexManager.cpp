//
//  Interpreter.cpp
//  NewSQL
//
//  Created by xiaotao Nie on 26/05/2017.
//  Copyright © 2017 xiaotao Nie. All rights reserved.
//


#include "IndexManager.hpp"

MBuffer m1;

extern int BTreeSize1,BTreeSize2;

//现在我对于b->record+4这种写法抱有一些疑问

//automatically create index on primary key
void Create_index(string index_name, int attr_type){
    Block* b;
    int n=attr_type;
    int firstplace=1;
    b=m1.GetBlock(index_name,0,1);
    //这里的index_name应该是表的名字
    //index_name应该是索引的名字经过一些改动作为文件名字了
    memcpy(b->record,&n,4);
    memcpy(b->record+4,&firstplace,4);
    
    //第一块就用来存root的位置了，没干别的...造成了资源浪费，虽然这种做法比较傻逼
    
    //这里有一点问题，我应该把这个初始化的过程时候第二个块作为第一个块的子节点
    //目前好像没有实现这个内容
    
    b=m1.GetBlock(index_name,1,1);
    int who=2;
    memcpy(b->record,&n,4);
    memcpy(b->record+4,&who,4);
    if(n!=2)
    {
        int k=12;
        int num=0x7fffffff;
        for(int i=0;i<500;i++)
        {
            memcpy(b->record+k,&num,4);
            k=k+8;
        }
    }
}

//CREATE INDEX NAME ON NAME '(' NAME ')'
void Create_index(vector<Value_offset> values, string index_name, int attr_type){
    Create_index(index_name, attr_type);
    for(int i=0;i<values.size();i++)
    {
        Insert_index(index_name, values[i].s, values[i].off);
    }
}

void Insert_index(string index_name, string value, int offset){
    Block* b=m1.GetBlock(index_name,0,0);
    int type=0;
    int root=0;
    memcpy(&type,b->record,4);
    memcpy(&root,b->record+4,4);
    root=insert(value,offset,root,index_name);//insert返回的是root代表的号块
    b=m1.GetBlock(index_name,0,0);
    memcpy(b->record+4,&root,4);
    return;
}

void Delete_index(string index_name, string value){
    Block* b=m1.GetBlock(index_name,0,0);
    int type=0;
    int root=0;
    memcpy(&type,b->record,4);
    memcpy(&root,b->record+4,4);
    root=deletenode(value,root,index_name);
    b=m1.GetBlock(index_name,0,0);
    memcpy(b->record+4,&root,4);
    return;
}

vector<int> Find_indices(string index_name,string op, string value){
    vector<int> ans;
    Block* b=m1.GetBlock(index_name,0,0);
    int root=0;
    memcpy(&root,b->record+4,4);
    //find1(root,value,index_name);
    if(op=="=")
    {
        int i=-1;
        i=find1(root,value,index_name);
        if(i!=-1)  ans.push_back(i);
    }
    else if(op==">=")
    {
        ans=findbigthan(root,value,index_name,1);
    }
    else if(op=="<=")
    {
        ans=findsmallthan(root,value,index_name,1);
    }
    else if(op==">")
    {
        ans=findbigthan(root,value,index_name,0);
    }
    else if(op=="<")
    {
        ans=findsmallthan(root,value,index_name,0);
    }
    else if(op=="<>")
    {
        ans=findnoeq(root,value,index_name);
    }
    return ans;
}
void dis(string filename)
{
    Block* b=m1.GetBlock(filename,0,0);
    int type=0;//?
    int root=0;
    memcpy(&type,b->record,4);
    memcpy(&root,b->record+4,4);
    cout<<"type: "<<type<<endl;
    cout<<"root: "<<root<<endl;
    node *p=getnode(root,filename);
    disre(p,0,filename);
}
void disre(node *p,int q,string filename)
{
    int N=(p->gettype()==2)?15:501;
    if(p->whoareyou()==2)
    {
        for(int i=0;i<N-1;i++)
        {
            for(int j=0;j<q;j++)
                cout<<"-";
            cout<<i<<" "<<p->getvalue(i)<<" "<<p->getOffset(i)<<endl;
        }
    }
    else
    {
        for(int i=0;i<N;i++)
        {
            if(p->getadree(i)!=0)
            {
                if(i!=N-1)
                {
                    for(int j=0;j<q;j++)
                        cout<<"-";
                    cout<<"In "<<i<<" "<<p->getvalue(i)<<endl;		
                }
                else
                {
                    for(int j=0;j<q;j++)
                        cout<<"-";
                    cout<<"In "<<i<<" NULL"<<endl;	
                }
                disre(getnode(p->getadree(i),filename),q+1,filename);
            }
        }
    }
}


