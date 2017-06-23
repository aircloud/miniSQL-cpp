//
//  Interpreter.cpp
//  NewSQL
//
//  Created by xiaotao Nie on 24/05/2017.
//  Copyright © 2017 xiaotao Nie. All rights reserved.
//

#include "BufferManager.hpp"

int BTreeSize1 = 15,BTreeSize2 = 501;

//初始化一个block,和磁盘读写无关
Block* MBuffer::GBlock()
{
    Block* Tmp;
    Tmp = new Block;
    Tmp->record = new char[BlockSize];
    memset(Tmp->record,0,BlockSize);
    Tmp->tablename = "";
    Tmp->offset = 0;
    Tmp->accessed = false;
    Tmp->written = false;
    return Tmp;
}

//初始化一个MBuffer,一个MBuffer包含64个Block,全都初始化
bool MBuffer::Init()
{
    int i;
    for(i=0;i<64;i++)
    {
        Buffer[i] = GBlock();
        if(Buffer[i] == NULL || Buffer[i]->record == NULL)
            return false;
    }
    return true;
}
//得到一个块,tablename实际上就是filename,然后指定偏移地址和flag,选择不同的Schedule的方式
Block* MBuffer::GetBlock(string tablename, int offset, int flag)
{
    Block* Target;
    int i;
    //得到相应的块,看是不是已经写入缓存
    for(i=0;i<64;i++)
    {
        Target = Buffer[i];
        if(!Target){
            continue;
        }
        //printf("%s %s\n", Target->tablename.c_str(), tablename.c_str());
        if(Target->tablename == tablename && Target->offset == offset)
        {
            Target->accessed = true;
            return Target;
        }
    }
    if(flag == 0)
    {
        if(Sche1(tablename,offset))
        {
            for(i=0;i<64;i++)
            {
                Target = Buffer[i];
                if(Target->tablename == tablename && Target->offset == offset)
                {
                    Target->accessed = true;
                    return Target;
                }
            }
        }
    }
    else
    {
        if(Sche2(tablename,offset))
        {
            for(i=0;i<64;i++)
            {
                Target = Buffer[i];
                if(Target->tablename == tablename && Target->offset == offset)
                {
                    Target->accessed = true;
                    return Target;
                }
            }
        }
    }
    return NULL;
}
//
bool MBuffer::Sche1(string tablename, int offset)
{
    int i = 0;
    for(i=0;i<64;i++)
    {
        if(Buffer[i]->tablename == "")
        {
            break;
        }
    }
    if(i==64)
    {
        //这里采用操作系统的置换算法并不是LRU,而是一种近似 LRU 算法
        i = 0;
        while(1)
        {
            if(Buffer[i]->accessed == false)
                break;
            else
                Buffer[i]->accessed = false;
            i++;
            if(i == 64)
                i = 0;
        }
    }
    Exchange(tablename, offset, Buffer[i]);
    if(Buffer[i] != NULL)
        return true;
    else
        return false;
}
//这个置换算法相比于上面的置换算法,多初始化了一些内容，实际上是将写入的buffer有一个初始化为空的过程，第一次访问这个块的时候，需要这样做
bool MBuffer::Sche2(string tablename, int offset)
{
    int i = 0;
    for(i=0;i<64;i++)
    {
        if(Buffer[i]->tablename == "")
        {
            break;
        }
    }
    //如果有空buffer就不采用置换算法
    if(i==64)
    {
        i = 0;
        while(1)
        {
            if(Buffer[i]->accessed == false)
                break;
            else
                Buffer[i]->accessed = false;
            i++;
            if(i == 64)
                i = 0;
        }
    }
    Exchange("",-1,Buffer[i]);
    Buffer[i]->tablename = tablename;
    Buffer[i]->accessed = true;
    Buffer[i]->written = true;
    Buffer[i]->offset = offset;
    memset(Buffer[i]->record,0,BlockSize);//内容置为空
    Exchange("",-1,Buffer[i]);//保存内容
    if(Buffer[i] != NULL)
        return true;
    else
        return false;
}
//drop是根据tablename也就是filename来drop的,目前看来没有同步磁盘,所以是一个危险操作,应该是被调用的内置函数不是直接调用函数。
bool MBuffer::Drop(string tablename)
{
    Block* Target;
    int i;
    for(i=0;i<64;i++)
    {
        Target = Buffer[i];
        if(Target->tablename == tablename)
        {
            Buffer[i] = GBlock();
            if(Buffer[i] == NULL || Buffer[i]->record == NULL)
                return false;
            delete Target->record;
            delete Target;
        }
    }
    return true;
}
//这是一个重点的方法,这里的意思是置换,置换就是先换出(写入磁盘)再换入(从磁盘读到内存)
bool  MBuffer::Exchange(string tablename, int offset, Block* Replaced)
{
    
    //Exchange("",-1,Target)这种出现过很多次
    //这里我们需要注意一下,如果我们只是同步到磁盘中而不是再从磁盘中读取什么了,也就是说只存而不取新的，通常这样做
    FILE* fp;
    int i;
    char Filename[64];
    memset(Filename,0,sizeof(Filename));
    int length = 0;
    length = (int)Replaced->tablename.size();
    //如果我们传入了文件名,这里的操作就是要先把block的内容写入磁盘
   
    if(length!=0)
    {
        //虽然一个表可以由很多块来存放，但是一个表的确就是对应一个文件(文件是写在磁盘上,要先放在内存中操作)
        for(i=0;i<length;i++)
        {
            Filename[i] = Replaced->tablename.at(i);
        }
        if((fp = fopen(Filename,"rb+"))==NULL)
        {
            //cout<<"Writing data error!"<<endl;
            return false;
        }
        fseek(fp,BlockSize*Replaced->offset,SEEK_SET);
        fwrite(Replaced->record,BlockSize,1,fp);
        fclose(fp);
    }
    // 清空文件名
    memset(Filename,0,sizeof(Filename));
    
    //如果offset不等于-1,这个时候要从磁盘中把新内容读入,我估计这个会在index过程被调用
    if(offset!=-1)
    {
        length = (int)tablename.size();
        for(i=0;i<length;i++)
        {
            Filename[i] = tablename.at(i);
        }
        if((fp = fopen(Filename,"rb"))==NULL)
        {
            cout<<"Reading data error!"<<endl;
            return false;
        }
        fseek(fp,BlockSize*offset,SEEK_SET);
        fread(Replaced->record,BlockSize,1,fp);
        fclose(fp);
        Replaced->offset = offset;
        Replaced->accessed = true;
        Replaced->tablename = tablename;
        return true;
    }
    else
        return true;
}
//析构函数,在结束操作之前,我们要把相关的内容同步到磁盘,但是这里我们注意,如果我们强行终止掉程序,析构函数很可能不会运行的
MBuffer::~MBuffer()
{
    Block* Target;
    int i;
    for(i=0;i<64;i++)
    {
        Target=Buffer[i];
        
        if(Target!=NULL)
        {
            if(Target->tablename != "")
                Exchange("",-1,Target);
            delete Target->record;
            delete Target;
        }
    }
}
//这个函数的作用是clearBuffer,请注意和上面的函数做区分
void MBuffer::clearBuffer()
{
    Block* Target;
    int i;
    for(i=0;i<64;i++)
    {
        Target=Buffer[i];
        
        if(Target!=NULL)
        {
            if(Target->tablename != "")
                Exchange("",-1,Target);
        }
        
        //real clear buffer
        memset(Buffer[i]->record,0,BlockSize);
        Buffer[i]->tablename = "";
        Buffer[i]->offset = 0;
        Buffer[i]->accessed = false;
        Buffer[i]->written = false;
    }
}
//返回一个文件的Block数量
int Block_num(string file_name)
{
    int num = 0;
    FILE *fp;
    fp = fopen(file_name.c_str(),"r");
    fseek(fp, 0, 2);  
    num = (int)ftell(fp)/BlockSize;
    fclose(fp);
    return num;
}
