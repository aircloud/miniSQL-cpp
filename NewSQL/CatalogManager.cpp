
#include "CatalogManager.hpp"
#include "BufferManager.hpp"

extern MBuffer m1;
extern MBuffer mybuffer;

//这个文件只是存储这些元信息,并没有直接存储数据

//目前表信息都没有用buffer来存储,直接读写文件的,我看看能不能把这个部分改掉
bool Create_table(Table table)
{
    ofstream out;
    int i;
    
    //Create_table首先判断table是不是存在,如果已经存在的话肯定是不行的
    bool exist = Judge_table_exist(table.table_name);
    if(exist)
    {
        return false;
    }
    
    //Tablelist为存储表名的一个txt
    //ios::app 从结尾处输入
    out.open("Tablelist.txt",ios::app);
    
    out<<table.table_name<<endl;
    out.close();
    
    //table.info也是一个表名字 _table.info存储着这个表的属性相关信息,并没有存储具体信息
    string table_info = table.table_name + "_table.info";
    //c_str:一个将string转换为const* char的函数;
    out.open(table_info.c_str(),ios::app);
    for(i=0;i<table.attr_count;i++)
    {
        
        out<<table.attrs[i].attr_name<<" ";
        out<<table.attrs[i].attr_key_type<<" ";
        out<<table.attrs[i].attr_type<<" ";
        out<<table.attrs[i].attr_len<<" ";
        out<<table.attrs[i].attr_id<<endl;
    }
    out.close();
    
    //创建一个_table.rec结尾的文件
    string table_rec = table.table_name + "_table.rec";
    out.open(table_rec.c_str(),ios::app);
    out.close();
    return true;
}

//建立索引
bool Create_index(Index index)
{
    
    //创建索引,需要进行若干判断,先判断是佛已经存在这个索引了,再判断表是不是存在,再判断这个被当作索引的属性是不是存在
    
    bool exist = Judge_index_exist(index.index_name);
    if(exist)
    {
        printf("error: Index '%s' exists\n", index.index_name.c_str());
        return false;
    }
    exist = Judge_table_exist(index.table_name);
    if(!exist)
    {
        printf("error: Table '%s' doesn't exist\n", index.table_name.c_str());
        return false;
    }
    exist = Judge_attr_exist(index.table_name, index.attr_name);
    if(!exist)
    {
        printf("error: attribute '%s' does not exists in the table", index.attr_name.c_str());
        return false;
    }
    
    //其实我感觉这部分元信息存储的画风有点不对
    
    //在Indexlist.txt写入这个索引的信息
    ofstream out;
    out.open("Indexlist.txt",ios::app);
    out<<index.index_name<<" "<<index.table_name<<" "<<index.attr_name<<endl;
    out.close();
    //创建_index.rec结尾的文件,这个文件会被当作索引文件
    string index_rec = index.index_name + "_index.rec";
    out.open(index_rec.c_str(),ios::app);
    out.close();
    return true;
}

//删除表
bool Drop_table(string table_name)
{
    bool exist = Judge_table_exist(table_name);
    if(!exist)
    {
        return false;
    }
    //remove函数用于删除指定的文件
    string table_rec = table_name + "_table.rec";
    remove(table_rec.c_str());
    string table_info = table_name + "_table.info";
    remove(table_info.c_str());
    
    //m1和mybuffer分别代表索引buffer和数据buffer
    m1.clearBuffer();
    mybuffer.clearBuffer();
    
    //这里是在Tablelist.txt这个文件中查找到这个表名字然后删除这一行,我觉得这个部分做的有点麻烦
    string table_list = "";
    char Table_name[32];
    string tablename;
    ifstream in;
    in.open("Tablelist.txt",ios::in);
    while(!in.eof())
    {
        in.getline(Table_name,32);
        tablename = Table_name;
        if(tablename == "" && in.eof())
        {
            break;
        }
        if(tablename != "" && tablename != table_name)
        {
            table_list += tablename + '\n';
        }
    }
    in.close();
    
    remove("Tablelist.txt");
    ofstream out;
    out.open("Tablelist.txt", ios::app);
    out<<table_list;
    out.close();
    return true;
}

//删除索引
//删除索引和删除表的过程比较相似,进行参考即可。
bool Drop_index(string index_name)
{
    bool exist = Judge_index_exist(index_name);
    if(!exist)
    {
        return false;
    }
    string index_rec = index_name + "_index.rec";
    remove(index_rec.c_str());
    
    //-----------------------------
    m1.clearBuffer();
    mybuffer.clearBuffer();
    
    string index_list = "";
    char Index_name[128];
    string indexname;
    ifstream in;
    in.open("Indexlist.txt",ios::in);
    int i;
    while(!in.eof())
    {
        in.getline(Index_name,128);
        indexname = Index_name;
        if(indexname!="")
        {
            i=0;
            while(indexname.at(i)!=' ')
            {
                i++;
            }
            indexname = indexname.substr(0,i);
        }
        if(indexname == "" && in.eof())
        {
            break;
        }
        if(indexname != "" && indexname != index_name)
        {
            indexname = Index_name;
            index_list += indexname + '\n';
        }
    }
    in.close();
    
    remove("Indexlist.txt");
    ofstream out;
    out.open("Indexlist.txt",ios::app);
    out<<index_list;
    out.close();
    return true;
}

bool Judge_attr_primary_unique(string table_name, string attr_name)
{
    fstream in;
    int i = 0;
    string table_info = table_name + "_table.info";
    in.open(table_info.c_str(),ios::in);
    char attr[128];
    string attrname;
    char attr_key = '0';
    while(!in.eof())
    {
        in.getline(attr,128);
        attrname = attr;
        
        //找到该行存的attrname名
        if(attrname!="")
        {
            i=0;
            while(attrname.at(i)!=' ')
            {
                i++;
            }
            attrname = attrname.substr(0,i);
        }
        
        //获取到 attr_key_type  这个目前自己还不是那么确定怎么生成的,但是如果这个不为0 就是说明是唯一的,有点蛋疼·
        if(attrname == attr_name)
        {
            attr_key = attr[i+1];
            break;
        }
    }
    in.close();
    if(attr_key!='0')
        return true;
    else
        return false;
}
//判断table是否在Tablelist.txt中存在
bool Judge_table_exist(string table_name)
{
    ifstream in;
    bool flag = false;
    char Table_name[32];
    string tablename;
    
    in.open("Tablelist.txt", ios::in);
    
    if (!in) {
        ofstream out;
        out.open("Tablelist.txt", ios::out);
        return false;
    }
    
    while(!in.eof())
    {
        in.getline(Table_name,32);
        
        tablename = Table_name;
        
        if(tablename == table_name)
        {
            flag = true;
            break;
        }
    }
    in.close();
    return flag;
}
//判断index是否在Indexlist.txt中存在
bool Judge_index_exist(string index_name)
{
    ifstream in;
    int i;
    bool flag = false;
    char index[128];
    string indexname;
    in.open("Indexlist.txt",ios::in);
    
    if (!in) {
        ofstream out;
        out.open("Indexlist.txt", ios::out);
        return false;
    }
    
    while(!in.eof())
    {
        in.getline(index,128);
        indexname = index;
        if(indexname!="")
        {
            i=0;
            while(indexname.at(i)!=' ')
            {
                i++;
            }
            indexname = indexname.substr(0,i);
        }
        if(indexname == index_name)
        {
            flag = true;
            break;
        }
    }
    in.close();
    return flag;
}
//判断这个attr_name是不是存在,这个在插入index的时候是需要用的
bool Judge_attr_exist(string table_name,string attr_name)
{
    ifstream in;
    bool flag = false;
    int i;
    string table_info = table_name + "_table.info";
    
    in.open(table_info.c_str(),ios::in);
    
    char attr[128];
    string attrname;
    
    while(!in.eof())
    {
        in.getline(attr,128);
        attrname = attr;
        if(attrname!="")
        {
            i=0;
            while(attrname.at(i)!=' ')
            {
                i++;
            }
            attrname = attrname.substr(0,i);
        }
        if(attrname == attr_name)
        {
            flag = true;
            break;
        }
    }
    in.close();
    return flag;
}

//读取table info到相应的数据结构中
Table Read_Table_Info(string table_name)
{
    Table table;
    table.table_name = table_name;
    table.attr_count = 0;
    string file_name = table_name + "_table.info";
    ifstream in;
    in.open(file_name.c_str(),ios::in);
    
    if(!in)
    {
        return table;
    }
    
    string info;
    int k=0;
    
    while(!in.eof())
    {
        in >> table.attrs[k].attr_name;		
        in >> table.attrs[k].attr_key_type;
        in >> table.attrs[k].attr_type;
        in >> table.attrs[k].attr_len;
        in >> table.attrs[k].attr_id;
        
        k++; 		
    }
    table.attr_count = k-1;
    in.close();
    return table;	
}

//根据表名和属性名查找这个对应的IndexName
//由于某些原因,IndexName 和attrname并不相同
//可以参考 out<<index.index_name<<" "<<index.table_name<<" "<<index.attr_name<<endl;
string Find_index_name(string table_name,string attr_name)
{
    ifstream in;
    int i,j,k;
    
    char Index[128];
    string index;
    string indexname = "";
    string tablename;
    string attrname;
    in.open("Indexlist.txt",ios::in);
    if(!in)
    {
        cout<<"No such info file!"<<endl;
        return "";
    }
    while(!in.eof())
    {	
        in.getline(Index,128);
        index = Index;
        if(index!="")
        {		
            index += '\n';
            i=0;
            while(index.at(i)!=' ')
            {
                i++;
            }
            indexname = index.substr(0,i);
            j=i+1;
            while(index.at(j)!=' ')
            {
                j++;
            }
            tablename = index.substr(i+1,j-i-1);
            k=j+1;
            while(index.at(k)!='\n')
            {
                k++;
            }
            attrname = index.substr(j+1,k-j-1);
            
            if(tablename == table_name && attrname == attr_name)
                break;
            indexname = "";
        }
    }
    in.close();
    return indexname; 
}






