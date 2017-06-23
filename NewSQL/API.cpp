
#include "API.hpp"

using namespace std;

extern string delete_values[32];
extern vector<string> select_values;

extern int delete_num;
extern int select_num;

extern vector<int> select_offsets;

long beginTime = 0, endTime = 0;

/**************************************************************************************************/

/*	
 创建表时的内部调用
 这个函数做了这些工作:
    首先创建表,也就是把表在CatalogManager中写入;
    如果上一步成功,就创建索引,创建索引要分别创建索引文件和在这个表中写入
*/
void API_Create_Table(Table& table)
{
    //if create table succeed
    if (Create_table(table)) {
        
        //get primary key
        int id = table.getPrimaryKeyId();
        
        //automatically create index
        if (id != -1) {
            
            string index_name = table.table_name + "_" + table.attrs[id].attr_name;
            
            Index idx;
            idx.table_name = table.table_name;
            idx.attr_name = table.attrs[id].attr_name;
            idx.index_name = index_name;
            
            //如果索引名重复
            while (idx.index_name == Find_index_name(table.table_name, table.attrs[id].attr_name)) {
                //在前面加下划线
                idx.index_name = "_" + idx.index_name;
            }
            
            //catalog create index
            Create_index(idx);
            
            //index manager create index
            Create_index(idx.index_name + "_index.rec", table.attrs[id].attr_type - INT);
        }
        
        
        printf("Query OK, 0 rows affected(%lf sec)\n",calculate_time(get_current_time(),beginTime));
    }
    else {
        printf("error: Table '%s' already exists(%lf sec)\n", table.table_name.c_str(),calculate_time(get_current_time(),beginTime));
    }
}

/*	
 删除表时的内部调用
 删除表需要经过以下几个步骤：
    1. 查找是不是存在这个表,如果不存在,自然没有删除的必要。
    2. 对这个表查找所有的属性,遍历判断每一个属性是不是索引如果是索引的话就在索引表中删除
                                        (这里好像没有涉及到删除索引表的事情,我对这里的问题保留疑问)
    3. 处理完索引之后删除表
*/
void API_Drop_Table(string table_name)
{
    //寻找这个table上建立的所有索引
    Table tbl = Read_Table_Info(table_name);
    if (tbl.attr_count == 0) {
        printf("error: Unknown table '%s'(%lf sec)\n", table_name.c_str(),calculate_time(get_current_time(),beginTime));
        return;
    }
    
    for (int i = 0; i < tbl.attr_count; i++) {
        string index_name = Find_index_name(table_name, tbl.attrs[i].attr_name);
        //若该属性上存在索引
        if (index_name != "") {
            Drop_index(index_name);
        }
    }
    
    //if drop table succeed
    if (Drop_table(table_name)) {
        printf("Query OK, 0 rows affected(%lf sec)\n",calculate_time(get_current_time(),beginTime));
    }
    else {
        printf("error: Table '%s' does not exists(%lf sec)", table_name.c_str(),calculate_time(get_current_time(),beginTime));
    }
}




/**************************************************************************************************/

//	创建索引时的内部调用
void API_Create_Index(Index& index)
{
    if (Create_index(index)) {  //这里的Create_index调用的是CatalogManager的Create_index,返回布尔值
        //real create index(call IndexManager)
        
        //first select
        vector<int> offsetlist; //null
        Condition_list list;    //null
        
        //select * from table_name;
        //这里的条件list为null,其实相当于选择所有信息
        Select_tuple(offsetlist, Read_Table_Info(index.table_name), list);
        //调用完这个函数之后,相关返回结果已经自动存入select_values中了,下文直接拿来用(因为声明了extern)
        
        
        //获取表信息
        Table tbl = Read_Table_Info(index.table_name);
        
        //获取建立索引属性的id
        int id = tbl.searchAttrId(index.attr_name);
        
        vector<Value_offset> values;
        
        //对每一行，获取索引对应属性值+offset
        for (int i = 0; i < select_values.size(); i++) {
            
            Value_offset vo;
            vo.s = split(select_values[i], "\t", id);
            vo.off = select_offsets[i];
            
            values.push_back(vo);
        }
        //调用IndexManager
        Create_index(values, index.index_name + "_index.rec", tbl.attrs[id].attr_type - INT);
        
        printf("Query OK, 0 rows affected(%lf sec)\n",calculate_time(get_current_time(),beginTime));
    } else{
        printf("error: The index is not found(%lf sec)\n",calculate_time(get_current_time(),beginTime));
    }
    
}

//	删除索引时的内部调用
void API_Drop_Index(string index_name)
{
    //if drop table succeed
    //catalog drop index
    if (Drop_index(index_name)) {
        printf("Query OK. 0 rows affected(%lf sec)\n",calculate_time(get_current_time(),beginTime));
    }
    else {
        printf("error: Index '%s' not exists(%lf sec)\n", index_name.c_str(),calculate_time(get_current_time(),beginTime));
    }
}







/**************************************************************************************************/

//	插入纪录时的内部调用
//  这个插入记录，一次只能插入一条
void API_Insert(Tuple& tuple, bool ifoutput)
{
    //先补全所有CHAR类型的尾字节
    for (int i = 0; i < tuple.attr_count; ++i)
    {
        //调整char型数据，不足的末尾补全空格，超出的截断
        if (tuple.attrs[i].attr_type == CHAR)
        {
            int default_len = tuple.attrs[i].attr_len;
            int input_len = (int)tuple.attr_values[i].length();
            
            //长度不足，补全空格
            if (input_len < default_len)
            {
                for (int j = 0; j < default_len - input_len; ++j)
                    tuple.attr_values[i] += " ";
            }
            //长度超出，截断
            else if(input_len > default_len)
            {
                tuple.attr_values[i] = tuple.attr_values[i].substr(0, default_len);
            }
        }
    }
    
    //预检查，处理含有索引项
    for (int i = 0; i < tuple.attr_count; ++i)
    {
        string index_name;
        //检查每个属性，primary的去B+树查看
        if (tuple.attrs[i].attr_key_type == PRIMARY)
        {
            //获取primary key的索引名
            index_name = Find_index_name(tuple.table_name, tuple.attrs[i].attr_name);
            //搜索有无重值
            vector<int> v = Find_indices(index_name+"_index.rec", "=", tuple.attr_values[i]);
            //若重名
            if (!v.empty())
            {
                if(ifoutput)printf("error: Duplicate entry '%s' for key 'PRIMARY'\n", tuple.attr_values[i].c_str());
                return;
            }
        }
        else if (tuple.attrs[i].attr_key_type == UNIQUE)
        {
            //unqiue属性如果存在索引，去B+树查看，然后设置为not unique(EMPTY)
            index_name = Find_index_name(tuple.table_name, tuple.attrs[i].attr_name);
            if (index_name != "")
            {
                //搜索有无重名
                vector<int> v = Find_indices(index_name+"_index.rec", "=", tuple.attr_values[i]);
                //若重名
                if (!v.empty())
                {
                    if(ifoutput)printf("error: Duplicate entry '%s' for key 'unique'\n", tuple.attr_values[i].c_str());
                    return;
                }
                tuple.attrs[i].attr_key_type = EMPTY;
            }
        }
    }
    int offset = Insert(tuple);
    //insert成功

    if (offset != -1) {
        
        //将插入记录中涉及到index的内容插入B+树
        for (int i = 0; i < tuple.attr_count; ++i)
        {
            string index_name = Find_index_name(tuple.table_name, tuple.attrs[i].attr_name);
            if (index_name != "")
            {
                //void Insert_index(string index_name, string value, int offset);
                Insert_index(index_name+"_index.rec", tuple.attr_values[i], offset);
            }
        }
        
        if(ifoutput)printf("Query OK, 1 row affected(%lf sec)\n",calculate_time(get_current_time(),beginTime));
    }
    else {
        if(ifoutput)printf("error: Duplicate unique attribute exists(%lf sec)\n",calculate_time(get_current_time(),beginTime));
    }
}

/**************************************************************************************************/

//	选择纪录时的内部调用
void API_Select(string table_name, Condition_list clist, bool ifoutput)
{
    
    //这里先判断一下table_name是不是存在
    bool exist = Judge_table_exist(table_name);
    if(!exist)
    {
        printf("error: no such table.\n");
        return;
    }
    
    
    //create iterator
    Condition_list::iterator it;
    
    //divide all conditions into two parts: have index or not have index
    Condition_list have_index_list, no_index_list;
    
    Table tbl = Read_Table_Info(table_name);
    
    //记得补全char结尾的空格
    //先补全所有Condition_list里面的CHAR类型attr_name的尾字节 tbl
    for (it = clist.begin(); it != clist.end(); ++it)
    {
        //attr and tablename
        int id = tbl.searchAttrId(it->attr_name);
        //调整char型数据，不足的末尾补全空格，超出的截断
        if (tbl.attrs[id].attr_type == CHAR)
        {
            int default_len = tbl.attrs[id].attr_len;
            int input_len = (int)it->cmp_value.length();
            
            //长度不足，补全空格
            if (input_len < default_len)
            {
                for (int j = 0; j < default_len - input_len; ++j)
                    it->cmp_value += " ";
            }
            //长度超出，截断
            else if(input_len > default_len)
            {
                it->cmp_value = it->cmp_value.substr(0, default_len);
            }
        }
    }
    
    //traversal condition list 查看各个字段是不是索引
    for (it = clist.begin(); it != clist.end(); ++it)
    {
        //check whether this attr has index
        if (Find_index_name(table_name, it->attr_name) == "")
            no_index_list.push_back(*it);
        else
            have_index_list.push_back(*it);
    }
    
    //check have_index_list
    vector<int> v_d, v_a, v_b;
    vector<int>::iterator my_Iter;
    
    string index_name;
    
    //条件列表没有索引
    //没有索引的情况是比较简单的，但这个时候复杂度是m*n,m为条件个数,n为记录总数
    if (have_index_list.empty())
    {
        //Select_tuple(vector<int> offsetlist, Table table, Condition_list list)
        select_values.clear();
        select_num = 0;
        Select_tuple(v_d, tbl, clist);
        if (select_num == 0) {
            if(ifoutput)printf("Empty set\n");
            return;
        }
        else {
            if(ifoutput)API_Draw_result(tbl);
            if(ifoutput)printf("%d rows in set(%lf sec)\n", select_num,calculate_time(get_current_time(),beginTime));
        }
        return;
    }
    
    //处理有索引的情况
    
    //we always do this: vd <- va intersects vb, va <- vd
    it = have_index_list.begin();
    index_name = Find_index_name(table_name, it->attr_name);
    v_d = Find_indices(index_name+"_index.rec", it->op_type, it->cmp_value);
    
    //取完交集后一定要排序才能正确去重
    sort(v_d.begin(), v_d.end());
    v_d.erase( unique(v_d.begin(), v_d.end() ), v_d.end() );
    
    for (++it; it != have_index_list.end(); ++it) {
        index_name = Find_index_name(table_name, it->attr_name);
        v_b = Find_indices(index_name+"_index.rec", it->op_type, it->cmp_value);
        v_a = v_d;
        //intersection
        sort(v_a.begin(), v_a.end());
        sort(v_b.begin(), v_b.end());
        
        v_d.resize(v_a.size()+v_b.size());
        my_Iter = set_intersection(v_a.begin(), v_a.end(), v_b.begin(), v_b.end(), v_d.begin());
        v_d.resize(my_Iter - v_d.begin());
        //去除空格
        sort(v_d.begin(), v_d.end());
        v_d.erase( unique(v_d.begin(), v_d.end() ), v_d.end() );
    }
    
    //作完交集后若v_d为空，直接返回
    if (v_d.empty()) {
        if(ifoutput)printf("Empty set(%lf sec)\n",calculate_time(get_current_time(),beginTime));
        return;
    }
    
    //final have index result stores in vector<int> v_d
    //Select_tuple(vector<int> offsetlist, Table table, Condition_list list)
    select_values.clear();
    select_num = 0;
    Select_tuple(v_d, tbl, clist);
    //这个时候由于v_d不是空的,实际上走的流程是不一样的,
    //但是这里我认为有一个问题就是这里的clist实际上是可以精简从而提高效率的,但是没有精简
    
    
    if (select_num == 0) {
        if(ifoutput)printf("Empty set(%lf sec)\n",calculate_time(get_current_time(),beginTime));
        return;
    }
    else {
        if(ifoutput)API_Draw_result(tbl);
        if(ifoutput)printf("%d rows in set(%lf sec)\n", select_num,calculate_time(get_current_time(),beginTime));
    }
}

void API_Select(string table_name, Condition_list clist, map<string, int> columns, bool ifoutput)
{
    
    //create iterator
    Condition_list::iterator it;
    
    //divide all conditions into two parts: have index or not have index
    Condition_list have_index_list, no_index_list;
    
    Table tbl = Read_Table_Info(table_name);
    
    //记得补全char结尾的空格
    //先补全所有Condition_list里面的CHAR类型attr_name的尾字节 tbl
    for (it = clist.begin(); it != clist.end(); ++it)
    {
        //attr and tablename
        int id = tbl.searchAttrId(it->attr_name);
        //调整char型数据，不足的末尾补全空格，超出的截断
        if (tbl.attrs[id].attr_type == CHAR)
        {
            int default_len = tbl.attrs[id].attr_len;
            int input_len = (int)it->cmp_value.length();
            
            //长度不足，补全空格
            if (input_len < default_len)
            {
                for (int j = 0; j < default_len - input_len; ++j)
                    it->cmp_value += " ";
            }
            //长度超出，截断
            else if(input_len > default_len)
            {
                it->cmp_value = it->cmp_value.substr(0, default_len);
            }
        }
    }
    
    //traversal condition list 查看各个字段是不是索引
    for (it = clist.begin(); it != clist.end(); ++it)
    {
        //check whether this attr has index
        if (Find_index_name(table_name, it->attr_name) == "")
            no_index_list.push_back(*it);
        else
            have_index_list.push_back(*it);
    }
    
    //check have_index_list
    vector<int> v_d, v_a, v_b;
    vector<int>::iterator my_Iter;
    
    string index_name;
    
    //条件列表没有索引
    //没有索引的情况是比较简单的，但这个时候复杂度是m*n,m为条件个数,n为记录总数
    if (have_index_list.empty())
    {
        //Select_tuple(vector<int> offsetlist, Table table, Condition_list list)
        select_values.clear();
        select_num = 0;
        Select_tuple(v_d, tbl, clist, columns);
        if (select_num == 0) {
            if(ifoutput)printf("Empty set\n");
            return;
        }
        else {
            if(ifoutput)API_Draw_result(tbl,columns);
            if(ifoutput)printf("%d rows in set(%lf sec)\n", select_num,calculate_time(get_current_time(),beginTime));
        }
        return;
    }
    
    //处理有索引的情况
    
    //we always do this: vd <- va intersects vb, va <- vd
    it = have_index_list.begin();
    index_name = Find_index_name(table_name, it->attr_name);
    v_d = Find_indices(index_name+"_index.rec", it->op_type, it->cmp_value);
    
    //取完交集后一定要排序才能正确去重
    sort(v_d.begin(), v_d.end());
    v_d.erase( unique(v_d.begin(), v_d.end() ), v_d.end() );
    
    for (++it; it != have_index_list.end(); ++it) {
        index_name = Find_index_name(table_name, it->attr_name);
        v_b = Find_indices(index_name+"_index.rec", it->op_type, it->cmp_value);
        v_a = v_d;
        //intersection
        sort(v_a.begin(), v_a.end());
        sort(v_b.begin(), v_b.end());
        
        v_d.resize(v_a.size()+v_b.size());
        my_Iter = set_intersection(v_a.begin(), v_a.end(), v_b.begin(), v_b.end(), v_d.begin());
        v_d.resize(my_Iter - v_d.begin());
        //去除空格
        sort(v_d.begin(), v_d.end());
        v_d.erase( unique(v_d.begin(), v_d.end() ), v_d.end() );
    }
    
    //作完交集后若v_d为空，直接返回
    if (v_d.empty()) {
        if(ifoutput)printf("Empty set(%lf sec)\n",calculate_time(get_current_time(),beginTime));
        return;
    }
    
    //final have index result stores in vector<int> v_d
    //Select_tuple(vector<int> offsetlist, Table table, Condition_list list)
    select_values.clear();
    select_num = 0;
    Select_tuple(v_d, tbl, clist, columns);
    //这个时候由于v_d不是空的,实际上走的流程是不一样的,
    //但是这里我认为有一个问题就是这里的clist实际上是可以精简从而提高效率的,但是没有精简
    
    
    if (select_num == 0) {
        if(ifoutput)printf("Empty set(%lf sec)\n",calculate_time(get_current_time(),beginTime));
        return;
    }
    else {
        if(ifoutput)API_Draw_result(tbl,columns);
        if(ifoutput)printf("%d rows in set(%lf sec)\n", select_num,calculate_time(get_current_time(),beginTime));
    }
}


/**************************************************************************************************/
//	删除纪录时的内部调用
void API_Delete(string table_name, Condition_list clist, bool ifoutput)
{
    
    //create iterator
    Condition_list::iterator it;
    
    //divide all conditions into two parts: have index or not have index
    Condition_list have_index_list, no_index_list;
    
    
    Table tbl = Read_Table_Info(table_name);
    
    //先补全所有Condition_list里面的CHAR类型attr_name的尾字节 tbl
    for (it = clist.begin(); it != clist.end(); ++it)
    {
        //attr and tablename
        int id = tbl.searchAttrId(it->attr_name);
        //调整char型数据，不足的末尾补全空格，超出的截断
        if (tbl.attrs[id].attr_type == CHAR)
        {
            int default_len = tbl.attrs[id].attr_len;
            int input_len = it->cmp_value.length();
            
            //长度不足，补全空格
            if (input_len < default_len)
            {
                for (int j = 0; j < default_len - input_len; ++j)
                    it->cmp_value += " ";
            }
            //长度超出，截断
            else if(input_len > default_len)
            {
                it->cmp_value = it->cmp_value.substr(0, default_len);
            }
        }
    }
    
    
    //traversal condition list
    for (it = clist.begin(); it != clist.end(); ++it)
    {
        //check whether this attr has index
        if (Find_index_name(table_name, it->attr_name) == "")
            no_index_list.push_back(*it);
        else
            have_index_list.push_back(*it);
    }
    
    //check have_index_list
    vector<int> v_d, v_a, v_b;
    vector<int>::iterator my_Iter;
    
    string index_name;
    
    //条件列表没有索引, 同时处理了条件列表为空的情况, 因为此时no_index_list也为空, 筛选所有记录
    if (have_index_list.empty())
    {
        //extern string delete_values[32];
        
        //清空delete_values
        for (int i = 0; i < 32; i++) {
            delete_values[i] = "";
        }
        delete_num = 0;
        
        //调用RecordManager删除记录
        Delete_tuple(v_d, tbl, clist);
        
        //反馈删除的行数
        if(ifoutput)printf("Query OK, %d rows affected(%lf sec)\n", delete_num,calculate_time(get_current_time(),beginTime));
        
        //若未删除任何记录，直接返回，不用处理index
        if (delete_num == 0) {
            return;
        }
        
        //前往删除相应索引值
        goto Final_Delete;
        
    }
    
    //we always do this: vd <- va intersects vb, va <- vd
    it = have_index_list.begin();
    index_name = Find_index_name(table_name, it->attr_name);
    v_d = Find_indices(index_name+"_index.rec", it->op_type, it->cmp_value);
    
    //取完交集后一定要排序才能正确去重
    sort(v_d.begin(), v_d.end());
    v_d.erase( unique(v_d.begin(), v_d.end() ), v_d.end() );
    
    for (; it != have_index_list.end(); ++it) {
        index_name = Find_index_name(table_name, it->attr_name);
        v_b = Find_indices(index_name+"_index.rec", it->op_type, it->cmp_value);
        v_a = v_d;
        //intersection
        sort(v_a.begin(), v_a.end());
        sort(v_b.begin(), v_b.end());
        
        v_d.resize(v_a.size()+v_b.size());
        my_Iter = set_intersection(v_a.begin(), v_a.end(), v_b.begin(), v_b.end(), v_d.begin());
        v_d.resize(my_Iter - v_d.begin());
        
        //去除重复
        //取完交集后一定要排序才能正确去重
        sort(v_d.begin(), v_d.end());
        v_d.erase( unique(v_d.begin(), v_d.end() ), v_d.end() );
    }
    
    //作完交集后若v_d为空，说明没有符合条件的记录，直接返回
    if (v_d.empty()) {
        if(ifoutput)printf("Empty set(%lf sec)\n",calculate_time(get_current_time(),beginTime));
        return;
    }
    
    /*-----------------------------我是分割线----------------------------------*/
    /*
     上面的内容和select的过程很相似，都是处理有索引的情况，没有索引的情况，然后最后存入v_d,这个时候存入的是offset
     下文删除需要做的工作稍微多一些
    */
    /*-----------------------------我是分割线----------------------------------*/
    
    //final have index result stores in vector<int> v_d
    
    //先清空delete_values
    for (int i = 0; i < 32; i++) {
        delete_values[i] = "";
    }
    delete_num = 0;
    
    //调用RecordManager删除记录操作
    Delete_tuple(v_d, tbl, clist);
    
    //反馈删除行数
    if(ifoutput)printf("Query OK, %d rows affected(%lf sec)\n", delete_num,calculate_time(get_current_time(),beginTime));
    
    //若未删除任何记录，直接返回，不用处理index
    if (delete_num == 0) {
        if(ifoutput)printf("Empty set(%lf sec)\n",calculate_time(get_current_time(),beginTime));
        return;
    }
    
Final_Delete:
    //遍历该表的attribute，找value，找index_name，delete
    for (int i = 0; i < tbl.attr_count; i++) {
        
        //找条件列表中含索引项的属性的value和索引名
        
        //index_name
        string index_name = Find_index_name(table_name, tbl.attrs[i].attr_name);
        
        //先确定属性有无索引, 无索引则直接进入下一循环
        if (index_name == "") {
            continue;
        }
        
        //取出delete_values的第i列，即为要B+树删除的value
        vector<string> vs = split(delete_values[i], "\t");
        
        //将该列的value逐一删除
        for (int j = 0; j < vs.size(); j++) {
            Delete_index(index_name+"_index.rec", vs[j]);
        }
        
    }
    
}

/**************************************************************************************************/
//	更新纪录时的内部调用
void  API_Update(string table_name,Condition_list clist,Update_list ulist){
    
    Table table1 = Read_Table_Info(table_name);
    
    API_Select(table_name, clist, false);
    bool error = false;
    
    vector<Tuple> Tuples;
    vector<Tuple>::iterator ret;

    for (vector<string>::iterator it = select_values.begin(); it != select_values.end(); it++) {
        //分割字符串
        vector<string> ts = split(*it, "\t");
        
        //接下来想办法写入tuple,这个应该别的地方已经写好了
        Tuple tempTuple;
        
        for(int i=0;i<ts.size();i++){
            tempTuple.attr_values[i] = ts[i];
        }
        
        tempTuple.table_name = table1.table_name;
        tempTuple.attr_count = table1.attr_count;
        // 不能直接复值,所以用memcpy
        memcpy(tempTuple.attrs, table1.attrs, table1.attr_count * sizeof(Attribute));
        
        Tuples.push_back(tempTuple);
    }
    
    //然后更改
    
    for(int j = 0 ; j < ulist.size() ; j++) { //j 指的是第几个条件
        
        //对于每一个条件,遍历每一个tuple,在每一个tuple中找到这个条目,然后更改
        for(int k = 0; k < Tuples.size(); k++){ //k 指的是第几个tuple
            
            for(int m = 0;m < Tuples[k].attr_count; m++){ //m 指的是该tuple的第几个attr
                
                if(Tuples[k].attrs[m].attr_name == ulist[j].attr_name){
                    
                    if(DIRECTASSIGN == ulist[j].UpdateType){
                        Tuples[k].attr_values[m] = ulist[j].UpdateValue;
                    }
                    
                    else{
                        
                        if(Tuples[k].attrs[m].attr_type == CHAR){
                            
                            if(ulist[j].UpdateOperator != '+'){
                                error = true;
                                break;
                            } else{
                                Tuples[k].attr_values[m] = Tuples[k].attr_values[m] + ulist[j].UpdateValue;
                            }
                        } else if(Tuples[k].attrs[m].attr_type == INT){
                            switch (ulist[j].UpdateOperator) {
                                case '+':
                                    Tuples[k].attr_values[m] = to_string(atoi(Tuples[k].attr_values[m].c_str()) + atoi(ulist[j].UpdateValue.c_str()));
                                    break;
                                case '-':
                                    Tuples[k].attr_values[m] = to_string(atoi(Tuples[k].attr_values[m].c_str()) - atoi(ulist[j].UpdateValue.c_str()));
                                    break;
                                case '*':
                                    Tuples[k].attr_values[m] = to_string(atoi(Tuples[k].attr_values[m].c_str()) * atoi(ulist[j].UpdateValue.c_str()));
                                    break;
                                case '/':
                                    Tuples[k].attr_values[m] = to_string(atoi(Tuples[k].attr_values[m].c_str()) / atoi(ulist[j].UpdateValue.c_str()));
                                    break;
                                default:
                                    error = true;
                                    break;
                            }
                        } else if(Tuples[k].attrs[m].attr_type == FLOAT){
                            switch (ulist[j].UpdateOperator) {
                                case '+':
                                    Tuples[k].attr_values[m] = to_string(atof(Tuples[k].attr_values[m].c_str()) + atof(ulist[j].UpdateValue.c_str()));
                                    break;
                                case '-':
                                    Tuples[k].attr_values[m] = to_string(atof(Tuples[k].attr_values[m].c_str()) - atof(ulist[j].UpdateValue.c_str()));
                                    break;
                                case '*':
                                    Tuples[k].attr_values[m] = to_string(atof(Tuples[k].attr_values[m].c_str()) * atof(ulist[j].UpdateValue.c_str()));
                                    break;
                                case '/':
                                    Tuples[k].attr_values[m] = to_string(atof(Tuples[k].attr_values[m].c_str()) / atof(ulist[j].UpdateValue.c_str()));
                                    break;
                                default:
                                    error = true;
                                    break;
                            }
                        }
                    }
                }
                
            }
        }
        
        
    }
    
    if(!error){
        
        API_Delete(table_name,clist,false);

        for(int ind = 0 ; ind < Tuples.size(); ind++){
            API_Insert(Tuples[ind],false);
        }
        
        //插入成功
        printf("Query OK, %lu rows affected(%lf sec)\n", Tuples.size(),calculate_time(get_current_time(),beginTime));
        
    } else{
        printf("Failed,syntax error");
    }
    
}

/**************************************************************************************************/

/*
 *  下面的四个函数前两个是多值分割函数,后面两个是单值分割函数
    意思是前面两个分割是把pattern的每一个字符当作一个可分割项目进行分割,而后面的一个是把pattern当作一个整体
 */

//多值分割并且忽略纯粹的空格
vector<string> multiSplitEscapeS(string str, string pattern)
{
    regex blank ("^(\\s)*?$",regex::icase);
    string::size_type pos;
    vector<string> result;
    str+=pattern.substr(0,1);//扩展字符串以方便操作
    int size = (int)str.size();
    
    for(int i=0; i<size; i++)
    {
        pos = size+1;
        
        for(int j=0;j<pattern.size();j++){
            //cout<<"str.find(pattern.substr(j,1),i):"<<str.find(pattern.substr(j,1),i)<<endl;
            pos=(int)str.find(pattern.substr(j,1),i)<pos?(int)str.find(pattern.substr(j,1),i):pos;
        }

        if(pos<size)
        {
            std::string s=str.substr(i,pos-i);
            if(!regex_match(s,blank))
                result.push_back(s);
            i = ((int)pos+1) - 1;
            //多值分割和单值分割的不同之处
        }
    }
    return result;
}

//多值分割函数
vector<string> multiSplit(string str, string pattern)
{
    string::size_type pos;
    vector<string> result;
    str+=pattern.substr(0,1);//扩展字符串以方便操作
    int size = (int)str.size();
    
    for(int i=0; i<size; i++)
    {
        pos = size+1;
        
        for(int j=0;j<pattern.size();j++){
            pos=(int)str.find(pattern.substr(j,j+1),i)<pos?(int)str.find(pattern.substr(j,j+1),i):pos;
        }
        
        if(pos<size)
        {
            std::string s=str.substr(i,pos-i);
            result.push_back(s);
            i = ((int)pos+1) - 1;
            //多值分割和单值分割的不同之处
        }
    }
    return result;
}

//字符串分割函数
vector<string> splitEscapeS(string str, string pattern)
{
    regex blank ("^(\\s)*?$",regex::icase);
    string::size_type pos;
    vector<string> result;
    str+=pattern;//扩展字符串以方便操作
    int size = (int)str.size();
    
    for(int i=0; i<size; i++)
    {
        pos=str.find(pattern,i);
        if(pos<size)
        {
            std::string s=str.substr(i,pos-i);
            if(!regex_match(s,blank))
                result.push_back(s);
            i = ((int)pos+(int)pattern.size()) - 1;
        }
    }
    return result;
}

vector<string> split(string str, string pattern)
{
    string::size_type pos;
    vector<string> result;
    str+=pattern;//扩展字符串以方便操作
    int size = (int)str.size();
    
    for(int i=0; i<size; i++)
    {
        pos=str.find(pattern,i);
        if(pos<size)
        {
            std::string s=str.substr(i,pos-i);
            result.push_back(s);
            i = ((int)pos+(int)pattern.size()) - 1;
        }
    }
    return result;
}

string split(string str, string pattern, int id)
{
    vector<string> result = split(str, pattern);
    return result[id];
}

//extern vector<string> select_values;
//extern int select_num;

void API_Draw_result(Table& tbl)
{
    //确定列宽, 比该列字符串多两个字符
    int col_width[32];
    
    //initialize为2
    for (int i = 0; i < 32; i++) {
        col_width[i] = 0;
    }
    
    //扫描每一行每一列，调整列宽
    for (vector<string>::iterator it = select_values.begin(); it != select_values.end(); it++) {
        
        //每一行取出attr_count个字符串, 计算长度
        vector<string> vs = split(*it, "\t");
        for (int i = 0; i < tbl.attr_count; i++) {
            //调整列宽
            if (vs[i].length() > col_width[i]) {
                col_width[i] = vs[i].length();
            }
        }
    }
    //对属性名长度调整列宽
    for (int i = 0; i < tbl.attr_count; i++) {
        //调整列宽
        if (tbl.attrs[i].attr_name.length() > col_width[i]) {
            col_width[i] = tbl.attrs[i].attr_name.length();
        }
    }
    
    //最长字符串长度+2作为列宽
    for (int i = 0; i < tbl.attr_count; i++) {
        col_width[i] += 2;
    }
    
    //每行先画"+-----+-------+-------+", 再画"| id  | value | price |"
    
    //先确定分割线的字符串div_line
    string div_line = "+";
    for (int i = 0; i < tbl.attr_count; i++) {
        //
        for (int j = 0; j < col_width[i]; j++) {
            div_line += "-";
        }
        div_line += "+";
    }
    
    //先画分割线，属性名，分割线
    printf("%s\n", div_line.c_str());
    
    for (int i = 0; i < tbl.attr_count; i++) {
        
        //先画"| "
        printf("| ");
        
        //输出属性名
        cout << tbl.attrs[i].attr_name;
        
        //输出足够的填补空格
        for (int j = tbl.attrs[i].attr_name.length() + 1; j < col_width[i]; j++) {
            printf(" ");
        }
        
    }
    printf("|\n");
    
    printf("%s\n", div_line.c_str());
    
    //按行，画属性值
    for (vector<string>::iterator it = select_values.begin(); it != select_values.end(); it++) {
        
        //分割字符串
        vector<string> vs = split(*it, "\t");
        
        for (int i = 0; i < tbl.attr_count; i++) {
            
            //先画"| "
            printf("| ");
            
            //输出属性名
            printf("%s", vs[i].c_str());
            
            //输出足够的填补空格
            for (int j = vs[i].length() + 1; j < col_width[i]; j++) {
                printf(" ");
            }
            
        }
        printf("|\n");
    }
    
    printf("%s\n", div_line.c_str());
}

//这个绘制函数只会绘制选择出来的列,没有被选择的列不绘制
void API_Draw_result(Table& tbl,map<string, int> columns){
    
    int drawCount = 0;
    
    for(int i = 0; i< tbl.attr_count ; i++){
        if(columns[tbl.attrs[i].attr_name])
            drawCount+=1;
    }
    
    //确定列宽, 比该列字符串多两个字符
    int col_width[32];
    
    //initialize为2
    for (int i = 0; i < 32; i++) {
        col_width[i] = 0;
    }
    
    //扫描每一行每一列，调整列宽
    for (vector<string>::iterator it = select_values.begin(); it != select_values.end(); it++) {
        
        //每一行取出attr_count个字符串, 计算长度
        vector<string> vs = split(*it, "\t");
        for (int i = 0; i < drawCount; i++) {
            //调整列宽
            if (vs[i].length() > col_width[i]) {
                col_width[i] = vs[i].length();
            }
        }
    }
    //对属性名长度调整列宽
    for (int i = 0; i < drawCount; i++) {
        //调整列宽
        if (tbl.attrs[i].attr_name.length() > col_width[i]) {
            col_width[i] = tbl.attrs[i].attr_name.length();
        }
    }
    
    //最长字符串长度+2作为列宽
    for (int i = 0; i < drawCount; i++) {
        col_width[i] += 2;
    }
    
    //每行先画"+-----+-------+-------+", 再画"| id  | value | price |"
    
    //先确定分割线的字符串div_line
    string div_line = "+";
    for (int i = 0; i < drawCount; i++) {
        //
        for (int j = 0; j < col_width[i]; j++) {
            div_line += "-";
        }
        div_line += "+";
    }
    
    //先画分割线，属性名，分割线
    printf("%s\n", div_line.c_str());
    
    int tbl_index = 0;
    
    for (int i = 0; i < drawCount; i++) {
        
        //先画"| "
        printf("| ");
        
        //画属性名的这个地方,只画展示出来的属性名字,这个和重载之前的函数有所区别
        //输出属性名
        while(!columns[tbl.attrs[tbl_index].attr_name]){
            tbl_index++;
        }
        cout << tbl.attrs[tbl_index].attr_name;
        tbl_index++;
        
        
        //输出足够的填补空格
        for (int j = (int)tbl.attrs[i].attr_name.length() + 1; j < col_width[i]; j++) {
            printf(" ");
        }
        
    }
    printf("|\n");
    
    printf("%s\n", div_line.c_str());
    
    //按行，画属性值
    for (vector<string>::iterator it = select_values.begin(); it != select_values.end(); it++) {
        
        //分割字符串
        vector<string> vs = split(*it, "\t");
        
        for (int i = 0; i < drawCount; i++) {
            
            //先画"| "
            printf("| ");
            
            //输出属性名
            printf("%s", vs[i].c_str());
            
            //输出足够的填补空格
            for (int j = (int)vs[i].length() + 1; j < col_width[i]; j++) {
                printf(" ");
            }
            
        }
        printf("|\n");
    }
    
    printf("%s\n", div_line.c_str());
}
//测时函数
double calculate_time(long end, long start)
{
    return (double)(end - start)/1000000;
}

//获取当前从分钟开始的时间，因此这个函数可能会有一定的概率出错，虽然出错的概率非常低
long get_current_time()
{
    long result;
    
    struct timeval tv;
    gettimeofday(&tv,NULL);
    struct tm* pTime;
    pTime = localtime(&tv.tv_sec);
    
    result =  (long)pTime->tm_min * 60*1000000 + (long)pTime->tm_sec*1000000 + (long)tv.tv_usec;
    
    return result;
}




