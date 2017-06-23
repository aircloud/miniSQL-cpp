#include "BPlus.hpp"
extern MBuffer m1;

//构造函数,根据type构造node并初始化value
//目测N代表树的叉数

extern int BTreeSize1,BTreeSize2;

node::node(int q)
{
    type=q;
    if(type==2) N=BTreeSize1;
    else N=BTreeSize2;
    value=new string[N-1];
    for(int i=0;i<N-1;i++)
    {
        value[i]="";
    }
}
//析构函数 释放value
node::~node()
{
    delete[] value;
}
//返回节点的type,type是自己构造的时候传入的
int node::gettype(){
    return type;
}
int node::Haveson()
{
    return 0;
}
//返回第i个value
string node::getvalue(int i){
    return value[i];
}
//设置第i个value
void node::setvalue(string k,int i){
    value[i]=k;
}
//根据i返回Offset,这里默认-1
int node::getOffset(int i){
    return -1;
}

void node::setOffset(int Off,int i){
}
int node::getadree(int i){
    return -1;
}
void node::setadree(int place,int i){
}
void node::clear(){
}
void node::setNext(int place){
}
int node::getNext(){
    return -1;
}
int node::whoareyou(){
    return 0;
}
//清空normalnode
void normalnode::clear(){
    for(int i=0;i<N;i++)
    {
        if(i!=N-1)
            value[i]="";
        P[i]=0;
    }
}
//判断是不是Haveson,只需要判断第一个字节点是不是空的即可
int normalnode::Haveson(){
    if(P[0]==0) return 0;
    else return 1;
}

//normalnode构造函数,注意一下这种写法
normalnode::normalnode(int q):node(q){
    P=new int[N];
    for(int i=0;i<N;i++)
    {
        P[i]=0;
    }
}
//normalnode析构函数
normalnode::~normalnode(){
    delete[] P;
}
int normalnode::getadree(int i){
    return P[i];
}
void normalnode::setadree(int place,int i){
    P[i]=place;
}
//whoareyou应该返回的是一种标志,比如0,1...这里是1
int normalnode::whoareyou(){
    return 1;
}

//leafnode的一些基础函数,leafnode的子节点自然是没有得
int leafnode::Haveson(){
    return 0;
}
//leafnode的构造函数,这里的N是和node的构造函数有关,具体推测请看node构造函数的注释
leafnode::leafnode(int q):node(q){
    Next=0;
    Offset=new int[N]; //偏移量
    for(int i=0;i<N;i++)
    {
        Offset[i]=0;
    }
}
//leafnode的析构函数
leafnode::~leafnode(){
    delete[] Offset;
}
//leafnode获得偏移量
//对于直接把偏移数组定义在leafnode上面这里我仍然有一点疑问
int leafnode::getOffset(int i){
    return Offset[i];
}
//leafnode设置偏移
void leafnode::setOffset(int Off,int i){
    Offset[i]=Off;
}
//清空leafnode
void leafnode::clear(){
    for(int i=0;i<N;i++)
    {
        if(i!=N-1)
            this->setvalue("",i);
        Offset[i]=0;
    }
}
//设置下一个的指向,这里的下一个是用int来表示的
void leafnode::setNext(int place){
    Next=place;
}
int leafnode::getNext(){
    return Next;
}
//返回节点类型,leafnode为2
int leafnode::whoareyou(){
    return 2;
}

//临时节点的构造函数
tempnode::tempnode(int q):leafnode(q){
    value2=new string[N];
    for(int i=0;i<N;i++)
    {
        value2[i]="";
        Offset[i]=0;
    }
}
//临时节点构造函数的另外一种方式
tempnode::tempnode(int q,node *p):leafnode(q){   //修改 /////////////////////////////////////
    value2=new string[N];
    for(int i=0;i<N-1;i++)
    {
        value2[i]=p->getvalue(i);
        Offset[i]=p->getOffset(i);
    }
}
//临时节点析构函数
tempnode::~tempnode(){
    delete[] value2;
}
//临时节点继承自leafnode节点,自然是没有son的
int tempnode::Haveson(){
    return 0;
}
//临时节点获得偏移和设置偏移
int tempnode::getOffset(int i){
    return Offset[i];
}
void tempnode::setOffset(int Off,int i){
    Offset[i]=Off;
}

//临时节点获得value和设置value
string tempnode::getvalue(int i){
    return value2[i];
}
void tempnode::setvalue(string k,int i){
    value2[i]=k;
}

//这个内存和以上是什么关系,我目前还没搞清楚
neicun::neicun(int q){
    type=q;
    if(q==2) N=BTreeSize1;
    else N=BTreeSize2;
    value=new string[N];
    p=new int[N+1];
    for(int i=0;i<N;i++)
    {
        value[i]="";
        p[i]=0;
    }
    p[N]=0;
}

neicun::neicun(int q,node *n){   //修改     //////////////////
    type=q;
    if(q==2) N=BTreeSize1;
    else N=BTreeSize2;
    value=new string[N];
    p=new int[N+1];
    for(int i=0;i<N;i++)
    {
        if(i!=N-1)
            value[i]=n->getvalue(i);
        p[i]=n->getadree(i);
    }
}
neicun::~neicun(){
    delete[] value;
    delete[] p;
}

//这里大概是相当于把一个node节点的数据信息通过某种组织形式存在一个string中.
char* getfromnode(node* p)
{
    int type=p->gettype();
    int k=0;
    int who=p->whoareyou();
    char* re = new char[4096];
    //0-4字节存储type,4-8字节存储who
    memcpy(re+k,&type,4); k+=4;
    memcpy(re+k,&who,4); k+=4;
    //这里根据当时定义节点的不同类型来操作,是比较合理的,具体意思自己还需要再看
    if(type==2)
    {
        int i=0;
        char c[256];
        int a;
        for(i=0;i<14;i++)
        {
            if(who==1)//表明是中间节点
                a=p->getadree(i);
            else
                a=p->getOffset(i);
            //然后再用四个字节存储地址或者偏移,然后再用256字节存储内容
            memcpy(re+k,&a,4); k+=4;
            strcpy(c,(p->getvalue(i)).c_str());
            memcpy(re+k,c,256); k+=256;
        }
        if(who==1)
            a=p->getadree(i);
        else
            a=p->getNext();
        memcpy(re+k,&a,4); k+=4;
    }
    else
    {
        int i=0;
        int a;
        for(i=0;i<500;i++)
        {
            if(who==1)
                a=p->getadree(i);
            else
                a=p->getOffset(i);
            memcpy(re+k,&a,4); k+=4;
            if(type==0)
            {
                int num;
                if(!(p->getvalue(i)).empty())
                    num=atoi((p->getvalue(i)).c_str());
                else
                    num=0x7fffffff;
                memcpy(re+k,&num,4); k+=4;
            }
            else
            {
                float num;
                if(!(p->getvalue(i)).empty())
                    num=atof((p->getvalue(i)).c_str());
                else
                    num=0x7fffffff;
                memcpy(re+k,&num,4); k+=4;
            }
            
        }
        if(who==1)
            a=p->getadree(i);
        else
            a=p->getNext();
        memcpy(re+k,&a,4); k+=4;
    }
    return re;
}

//这个相当于把一个字符串的信息存储到一个节点中变成节点形式
//是上一个函数的逆过程
node* getfromfile(char* re)//0 int, 1 float, 2 char*
{
    int type;
    int who;
    int k=0;
    node* p;
    memcpy(&type,re+k,4); k+=4;
    memcpy(&who,re+k,4); k+=4;
    if(who==1) p=new normalnode(type);
    else p=new leafnode(type);
    if(type==2)
    {
        int i;
        int a;
        char c[256];
        for(i=0;i<14;i++)
        {
            memcpy(&a,re+k,4); k+=4;
            if(who==1)
                p->setadree(a,i);
            else
                p->setOffset(a,i);
            memcpy(c,re+k,256); k+=256;
            string s(c);
            p->setvalue(s,i);
        }
        memcpy(&a,re+k,4); k+=4;
        if(who==1)
            p->setadree(a,i);
        else
            p->setNext(a);
    }
    else
    {
        int i,a;
        int t1;
        float t2;
        char temp[10];
        string s;
        for(i=0;i<500;i++)
        {
            memcpy(&a,re+k,4); k+=4;
            if(who==1)
                p->setadree(a,i);
            else
                p->setOffset(a,i);
            if(type==0)
            {
                memcpy(&t1,re+k,4); k+=4;
                if(t1!=0x7fffffff)
                {
                    sprintf(temp,"%d",t1);
                    s=temp;
                    p->setvalue(s,i);
                }
                else
                {
                    s="";
                    p->setvalue(s,i);
                }
            }
            else
            {
                memcpy(&t2,re+k,4); k+=4;
                if(t2!=0x7fffffff)
                {
                    sprintf(temp,"%f",t2);
                    s=temp;
                    p->setvalue(s,i);
                }
                else
                {
                    s="";
                    p->setvalue(s,i);
                }
            }
        }
        memcpy(&a,re+k,4); k+=4;
        if(who==1)
            p->setadree(a,i);
        else
            p->setNext(a);
    }
    return p;
}
//根据off和filename得到node的字符串表示
//getnode传入的第一个为偏移,这个具有唯一性
node* getnode(int off,string filename){
    if(off==0) return NULL;
    Block* b=m1.GetBlock(filename,off,0);
    node* tree=getfromfile(b->record);
    return tree;
}
//将节点node的信息写回buffer
void rewrite(int off,node *p,string filename){
    Block* b=m1.GetBlock(filename,off,0);
    char* c=getfromnode(p);
    memcpy(b->record,c,4096);
    delete p;
    delete[] c;
    return;
}

//由于s1和s2可能表示的是int或者float或者string,所以这里需要分开比较
//以下几个都是这个道理,分别比较小于\等于\小于等于
//这里传入node p的意义仅仅在于,获得这个B+树的文件格式
bool ltnode(string s1,string s2,node*p){
    if(p->gettype()==2)
        return (s1<s2);
    else if(p->gettype()==1)
        return (atof(s1.c_str())<atof(s2.c_str()));
    else
        return (atoi(s1.c_str())<atoi(s2.c_str()));
}
bool eqnode(string s1,string s2,node*p){
    if(p->gettype()==2)
        return (s1==s2);
    else if(p->gettype()==1)
        return (atof(s1.c_str())==atof(s2.c_str()));
    else
        return (atoi(s1.c_str())==atoi(s2.c_str()));
}
bool lenode(string s1,string s2,node*p){
    return (ltnode(s1,s2,p) || eqnode(s1,s2,p) ) ;
}

//文件名,根结点
//string s应该是最终要找的目标
int find1(int root,string s,string filename){
    node *tree=getnode(root,filename);
    //tree是得到的整棵树
    int i=-1;
    int N=(tree->gettype()==2)?BTreeSize1:BTreeSize2;
    while(tree->Haveson())
    {
        for(i=0;i<N-1;i++)//遍历该节点
        {
            if(tree->getvalue(i).empty())//空了
                break;
            if(lenode(s,tree->getvalue(i),tree)) //说明在左子树
                break;
        }
        if(i==N-1) //该节点不符合
        {
            node *temp=tree;
            tree=getnode(tree->getadree(i),filename);
            delete temp;
        }
        else//在该节点中
        {
            if(eqnode(s,tree->getvalue(i),tree))
            {
                node *temp=tree;
                tree=getnode(tree->getadree(i+1),filename);
                delete temp;
            }
            else
            {
                node *temp=tree;
                tree=getnode(tree->getadree(i),filename);
                delete temp;
            }
        }
    }
    for(i=0;i<N-1;i++)
    {
        if(eqnode(s,tree->getvalue(i),tree))
            break;
    }
    if(i==N-1)
    {
        delete tree;
        tree=NULL;
        return -1;
    }
    int q=tree->getOffset(i);
    delete tree;
    return q;
}

//返回第一个叶结点
int findfirst(int root,string filename){
    node *p=getnode(root,filename);
    if(p->whoareyou()==2)
    {
        delete p;
        return root;
    }
    else
    {
        //循环以找到第一个叶结点
        do
        {
            root=p->getadree(0);
            delete p;
            p=NULL;
            p=getnode(root,filename);
        }while(p->whoareyou()==1);
        delete p;
        return root;
    }
}

//找到小于等于的所有节点,我认为从这里开始往下的内容就比较麻烦了
//这个其实还好,这里利用了叶结点叶结点其实都相连接连接成一个链表的性质,直接一个一个加入就可以了
vector<int> findsmallthan(int root,string value,string filename,int equal){
    int first=findfirst(root,filename);
    vector<int> intvec;
    node *p=getnode(first,filename);
    int N=(p->gettype()==2)?BTreeSize1:BTreeSize2;
    int flag=0;
    while(1)
    {
        for(int i=0;i<N-1;i++)
        {
            if(equal)
            {
                if( !p->getvalue(i).empty() )
                {
                    if(lenode(p->getvalue(i),value,p))
                        intvec.push_back(p->getOffset(i));
                    else{
                        flag=1;
                        break;
                    }
                }
                else break;
            }
            else{
                if( !p->getvalue(i).empty() )
                {
                    if(ltnode(p->getvalue(i),value,p))
                        intvec.push_back(p->getOffset(i));
                    else{
                        flag=1;
                        break;
                    }
                }
                else break;
            }
        }
        if(flag) break;
        if(p->getNext()==0) break;
        int pla=p->getNext();
        delete p;
        p=getnode(pla,filename);
    }
    delete p;
    return intvec;
}
//这个我认为这样写并不是很好,是不是从后往前效率比较高
vector<int> findbigthan(int root,string value,string filename,int equal){
    int first=findfirst(root,filename);
    vector<int> intvec;
    node *p=getnode(first,filename);
    int N=(p->gettype()==2)?BTreeSize1:BTreeSize2;
    while(1)
    {
        for(int i=0;i<N-1;i++)
        {
            if(equal)
            {
                if( !p->getvalue(i).empty() )
                {
                    if(lenode(value,p->getvalue(i),p))
                        intvec.push_back(p->getOffset(i));
                }
                else break;
            }
            else{
                if( !p->getvalue(i).empty() )
                {
                    if(ltnode(value,p->getvalue(i),p))
                        intvec.push_back(p->getOffset(i));
                }
                else break;
            }
        }
        if(p->getNext()==0) break;
        int pla=p->getNext();
        delete p;
        p=getnode(pla,filename);
    }
    delete p;
    return intvec;
}

//找出不等的,这个和前两个套路类似
vector<int> findnoeq(int root,string value,string filename){
    int first=findfirst(root,filename);
    vector<int> intvec;
    node *p=getnode(first,filename);
    int N=(p->gettype()==2)?BTreeSize1:BTreeSize2;
    while(1)
    {
        for(int i=0;i<N-1;i++)
        {
            
            if( !p->getvalue(i).empty() )
            {
                if(!eqnode(value,p->getvalue(i),p))
                    intvec.push_back(p->getOffset(i));
            }
            else break;
        }
        if(p->getNext()==0) break;
        int pla=p->getNext();
        delete p;
        p=getnode(pla,filename);
    }
    delete p;
    return intvec;
}

//找到父节点 这个没什么特别的 就一点一点的找
int parent(int p,int root,string filename) //找p的父节点
{
    if(root==p) return 0;
    int j,i;
    node *r=getnode(root,filename);
    int N=(r->gettype()==2)?BTreeSize1:BTreeSize2;
    node *t=getnode(p,filename);
    string k=t->getvalue(0);
    delete t;
    while(root!=p)
    {
        delete r;
        r=getnode(root,filename);
        for(j=0;j<N;j++)
        {
            if(p == r->getadree(j))//存在value需要去的地方
                break;
        }
        if(j<N) break;//找到了该节点的父节点
        
        for(i=0;i<N-1;i++)//遍历该节点
        {
            if(r->getvalue(i).empty())//空了
                break;
            //if(k<= root->getvalue(i))//存在value需要去的地方
            if(lenode(k,r->getvalue(i),r))
                break;
        }
        if(i==N-1) //该节点不符合
            root=r->getadree(i);
        else//在该节点中
        {
            if(eqnode(k,r->getvalue(i),r))
                root=r->getadree(i+1);
            else
                root=r->getadree(i);
        }
    }
    delete r;
    return root;
}

//插入节点
//这个插入操作并不是最后的插入操作,只是一个中间函数
//这个是非常简单的插入节点
int insertleaf(string k,int off,int place,string filename) //place 就是 P
{
    int i=0;
    node *p=getnode(place,filename);
    //while(k>p->getvalue(i) && !p->getvalue(i).empty())
    while( ltnode(p->getvalue(i),k,p) && !p->getvalue(i).empty() )
        i++;
    
    //在这是不能重复的 如果有重复的就失败了...
    //这里有问题需要修改
    if(eqnode(k,p->getvalue(i),p)) //重复
    {
        delete p;
        return place;
    }
    
    if(p->getvalue(i).empty())//在最后一个
    {
        p->setvalue(k,i);
        p->setOffset(off,i);
        //	char* getfromnode(p);
        rewrite(place,p,filename);
        //插入之后需要重写 这里的重写只是写回磁盘,
        return place;
    }
    else//在中间
    {
        int c=i;
        while(!p->getvalue(i).empty())
            i++;
        for(;i>c;i--)
        {
            p->setvalue(p->getvalue(i-1),i);
            p->setOffset(p->getOffset(i-1),i);
        }
        p->setvalue(k,i);
        p->setOffset(off,i);
        rewrite(place,p,filename);//这个时候也需要重写
        
        return place;
    }
}

//这是一个比较完整的插入父节点的过程
//首先如果如果是给根结点插入父节点，是比较简单的(这个if段我认为有问题,目前保留疑问)
//如果是给普通节点插入父节点,这个时候：如果父节点有空位，可以直接插入到父节点中,如果父节点没有空位,这个时候又要分裂父节点,并且递归地执行父节点插入父节点

int insertpar(string k,int newp,int old,int root,string filename)
{
    int root2=root;
    node *r=getnode(root,filename);
    int N=(r->gettype()==2)?BTreeSize1:BTreeSize2;
    int type=r->gettype();
    //这个时候代表要分裂根结点
    if(old==root)//根结点
    {
        //node *newroot=new normalnode;
        int who=1;
        int newr=Block_num(filename);
        Block* b=m1.GetBlock(filename,newr,1);////////////////////////////////////////////////////////////////////////
        memcpy(b->record,&type,4);
        memcpy(b->record+4,&who,4);
        if(N==BTreeSize2)
        {
            int kk=12;
            int numm=0x7fffffff;
            for(int ii=0;ii<500;ii++)
            {
                memcpy(b->record+kk,&numm,4);
                kk=kk+8;
            }
        }
        node *newroot=getnode(newr,filename);
        newroot->setadree(old,0);
        newroot->setvalue(k,0);
        newroot->setadree(newp,1);
        rewrite(newr,newroot,filename);
        delete r;
        return newr;
    }
    else
    {
        int i=0,j=0;
        root=parent(old,root,filename);
        delete r;
        r=getnode(root,filename);
        for(j=0;j<N;j++)
        {
            if(old == r->getadree(j))//存在value需要去的地方
                break;
        }
        if(r->getvalue(N-2).empty())//父节点还有空位
        {
            for(i=N-1;i>j+1;i--)
            {
                r->setadree(r->getadree(i-1),i);
                r->setvalue(r->getvalue(i-2),i-1);
            }
            r->setadree(newp,j+1);
            r->setvalue(k,j);
            rewrite(root,r,filename);
            return root2;
        }
        else//没空位了，分裂父节点
        {
            neicun *nei=new neicun(type,r);
            for(i=N;i>j+1;i--)
            {
                nei->p[i]=nei->p[i-1];
                nei->value[i-1]=nei->value[i-2];
            }
            nei->p[j+1]=newp;
            nei->value[j]=k;
            r->clear();
            //node* second=new normalnode;
            int who=1;
            int sec=Block_num(filename);
            Block* b=m1.GetBlock(filename,sec,1);/////////////////////////////////////////////////////////////////////
            memcpy(b->record,&type,4);
            memcpy(b->record+4,&who,4);
            if(N==BTreeSize2)
            {
                int kk=12;
                int numm=0x7fffffff;
                for(int ii=0;ii<500;ii++)
                {
                    memcpy(b->record+kk,&numm,4);
                    kk=kk+8;
                }
            }
            node *second=getnode(sec,filename);
            
            int m=0,q=0;
            for(q=0;q<N/2;q++)
            {
                r->setvalue(nei->value[q],q);
                r->setadree(nei->p[q],q);
            }
            r->setadree(nei->p[q],q);
            q++;
            for(;q<N;q++)
            {
                second->setvalue(nei->value[q],m);
                second->setadree(nei->p[q],m);
                m++;
            }
            second->setadree(nei->p[q],m);
            delete nei; nei=NULL;
            
            rewrite(sec,second,filename);
            rewrite(root,r,filename);
            r=NULL;
            
            second=getnode(sec,filename);
            while(second->Haveson())
            {
                node *t=second;
                second=getnode(second->getadree(0),filename);
                delete t;
            }
            //递归处理
            int t2=insertpar(second->getvalue(0),sec,root,root2,filename);
            delete second;
            return t2;
        }
    }
}

//这个和上面的insertleaf相比,其实效果是差不多的,唯一的区别是这个不需要对磁盘进行操作
node* insertleafnode(string k,int off,node* p)
{
    int i=0;
    while(ltnode(p->getvalue(i),k,p) && !p->getvalue(i).empty())
        i++;
    if(eqnode(k,p->getvalue(i),p))
        return p;
    if(p->getvalue(i).empty())//在最后一个
    {
        p->setvalue(k,i);
        p->setOffset(off,i);
        return p;
    }
    else//在中间
    {
        int c=i;
        while(!p->getvalue(i).empty())
            i++;
        for(;i>c;i--)
        {
            p->setvalue(p->getvalue(i-1),i);
            p->setOffset(p->getOffset(i-1),i);
        }
        p->setvalue(k,i);
        p->setOffset(off,i);
        return p;
    }
}

//一个可以对外暴露的比较完整的insert
int insert(string k,int off,int place,string filename){
    //这里的p不一定是根结点node?
    //自己对这里保留疑问
    int t=place;
    node *p=getnode(place,filename);
    int type=p->gettype();
    int N=(p->gettype()==2)?BTreeSize1:BTreeSize2;
    if(!p->Haveson() && p->getvalue(N-2).empty())//p是没满的叶节点 或 空树
        //这里的N应该是叶节点的数量
        
    {
        delete p;
        return insertleaf(k,off,place,filename);
    }
    else //1.叶节点，但是满了 2.非叶节点
    {
        if(p->Haveson())//2.非叶节点
        {
            int i;
            while(p->Haveson())//找到K所在叶节点
            {
                for(i=0;i<N-1;i++)
                {
                    if(p->getvalue(i).empty())//空了
                        break;
                    if(lenode(k,p->getvalue(i),p))
                        break;
                }
                if(i==N-1)
                    place=p->getadree(i);
                else{
                    if(eqnode(k,p->getvalue(i),p))
                        place=p->getadree(i+1);
                    else
                        place=p->getadree(i);
                }
                delete p;
                p=getnode(place,filename);
            }
        }
        //满叶节点,或者找到的可能没满的叶节点
        if(p->getvalue(N-2).empty()) //没满  t or p
        {
            place=insertleaf(k,off,place,filename);
            delete p;
            return t;
        }
        else //满了 （1.一开始的满叶节点 2.找到所在的叶节点满了）
        {
            for(int q=0;q<N-2;q++)//如果重复了
            {
                if(eqnode(k,p->getvalue(q),p))
                {
                    delete p;
                    return t;
                }
            }
            //node *newtree=new leafnode;
            int who=2;
            int newp=Block_num(filename);
            Block* b=m1.GetBlock(filename,newp,1);//////////////////////////////////////////////////////////////////////
            memcpy(b->record,&type,4);
            memcpy(b->record+4,&who,4);
            if(N==BTreeSize2)
            {
                int kk=12;
                int numm=0x7fffffff;
                for(int ii=0;ii<500;ii++)
                {
                    memcpy(b->record+kk,&numm,4);
                    kk=kk+8;
                }
            }
            node *newtree=getnode(newp,filename);
            node* temp=new tempnode(type,p);
            insertleafnode(k,off,temp);
            p->clear(); //删除
            newtree->setNext(p->getNext());//next指针交换
            p->setNext(newp);
            int j,m=0;
            for(j=0;j<N/2;j++)
            {
                p->setvalue(temp->getvalue(j),j);
                p->setOffset(temp->getOffset(j),j);
            }
            for(;j<N;j++)
            {
                newtree->setvalue(temp->getvalue(j),m);
                newtree->setOffset(temp->getOffset(j),m);
                m++;
            }
            delete temp;
            rewrite(place,p,filename);
            rewrite(newp,newtree,filename);
            
            newtree=getnode(newp,filename);
            string k=newtree->getvalue(0);
            //新建一个叶结点后要插入根结点 插入根结点是一个递归的操作
            int ot=insertpar(k,newp,place,t,filename);
            delete newtree;
            return ot;
        }
    }
}

void changepar(int place,string k,int r,string k2,string filename){
    
    node* p=getnode(place,filename);
    int N=(p->gettype()==2)?BTreeSize1:BTreeSize2;
    if(place==r) {
        int i=0;
        while(i<N-1 && !(eqnode(k,p->getvalue(i),p)))
            i++;
        if(i<N-1)
        {
            p->setvalue(k2,i);
            rewrite(place,p,filename);
        }
        return;
    }
    //node *p_parent=parent(p,root);//如果p不是根节点,否则p_parent=NULL；
    int p_p=parent(place,r,filename);
    node *p_parent=getnode(p_p,filename);//如果p不是根节点,否则p_parent=NULL；
    int i=0;
    //	while(!(p_parent->getvalue(i)==k) && i<N-1)
    while(i<N-1 && !(eqnode(k,p_parent->getvalue(i),p_parent)) )
        i++;
    if(i<N-1)
    {
        p_parent->setvalue(k2,i);
        rewrite(p_p,p_parent,filename);
        delete p;
        return;
    }
    else {
        delete p;
        delete p_parent;
        changepar(p_p,k,r,k2,filename);
    }
}

//r代表root的意思
//
int deletereall(string k,int place,int r,string filename){
    //node* deletereall(string k,node* p,node* root){
    int i=0; //在p结点中删除off和value 注意：要判断是leafnode还是normalnode！！！！！
    int where=0;//记录哪里被删掉了
    string sss1="";
    node *p=getnode(place,filename);
    int N=(p->gettype()==2)?BTreeSize1:BTreeSize2;
    if(p->whoareyou()==2)//leafnode
    {
        
        //以下过程是删掉叶结点的这个一段,这个非常简单
        
        for(i=0;i<N-1;i++)
        {
            //if(k==p->getvalue(i))
            if(eqnode(k,p->getvalue(i),p))
                break;
        }
        where=i;
        for(;i<N-2;i++)
        {
            if(p->getvalue(i+1).empty()) break;
            p->setOffset(p->getOffset(i+1),i);
            p->setvalue(p->getvalue(i+1),i);
        }
        p->setOffset(0,i);
        p->setvalue("",i);
    }
    else//normalnode
        //删除普通节点
    {
        for(i=0;i<N-1;i++)
        {
            if(eqnode(k,p->getvalue(i),p))
                break;
        }
        where=i;
        //if(i==N-1) exit(-1);
        for(;i<N-2;i++)
        {
            if(p->getvalue(i+1).empty()) break;
            p->setadree(p->getadree(i+2),i+1);//注意：预定了normalnode的第一个指针无法修改
            p->setvalue(p->getvalue(i+1),i);
        }
        p->setadree(0,i+1);
        p->setvalue("",i);
    }
    
    
    ///上面的部分是普通删除,下面的这部分是对节点进行合并
    
    //这里的r是root
    //p->setvalue("",i);  i空了
    if(place==r)//p是根节点 1.刚建树开始的叶节点 2.normalnode的结点
    {
        if(p->whoareyou()==2) //1.刚建树开始的叶节点
        {
            rewrite(place,p,filename);
            return r;
        }
        else //2.normalnode
        {
            if(i==0)//根节点只有一个子节点了
            {
                int qqq=p->getadree(0);
                delete p;
                return qqq;
            }
            else
            {
                rewrite(place,p,filename);
                return r;
            }
        }
    }
    else//p不是根节点  1.有根的叶节点 2.normalnode的结点(树中间)
    {
        if((p->whoareyou()==1 && i+1>=N/2) || (p->whoareyou()==2 && i>=N/2)) //内容不少  (修改)/////////////
        {
            if(where==0 && p->whoareyou()==2) //要修改父节点
            {
                string kk=p->getvalue(0);
                changepar(place,k,r,kk,filename);
            }
            rewrite(place,p,filename);
            return r;
        }
        else//结点内容太少，1.合并 2.重新分布   是否考虑i=0？
        {
            int j=-1,num=0;
            string k1;
            //node *p_parent=parent(p,root);//如果p不是根节点,否则p_parent=NULL;
            int p_p=parent(place,r,filename); //找到父节点
            node* p_parent=getnode(p_p,filename); //获取父节点数据结构
            int p_b;
            node *p_bro=NULL;
            
            
            if(p_parent!=NULL)
            {
                for(j=0;j<N;j++)
                {
                    if(place == p_parent->getadree(j))//存在value需要去的地方
                        break;
                }
            }
            else //错误
            {
                cout<<"error(no parent.)"<<endl;
                //exit(-1);
            }
            //p_parent->getadree(j)=p;
            if(j!=0)
            {
                p_b=p_parent->getadree(j-1);//左边兄弟
                p_bro=getnode(p_b,filename);
                k1=p_parent->getvalue(j-1);
            }
            else //j=0的时候没有左边兄弟,这个时候考虑右边兄弟
            {
                p_b=p_parent->getadree(j+1); //右边兄弟
                p_bro=getnode(p_b,filename);
                k1=p_parent->getvalue(j);
            }
            while(!p_bro->getvalue(num).empty())//计算兄弟结点的key数  num个value
            {
                num++;
                if(num==N-1) break;
            }
            if((p->whoareyou()==2 && i+num<=N-1) || (p->whoareyou()==1 && i+num<=N-2))
                //可以合并  leafnode和normal可以合并的要求不一样
            {
                if(p->whoareyou()==2)//叶节点
                {
                    if(j!=0)//和左边兄弟合并
                    {
                        int m=0,q=num;
                        while(!p->getvalue(m).empty())
                        {
                            p_bro->setvalue(p->getvalue(m),q);
                            p_bro->setOffset(p->getOffset(m),q);
                            q++;m++;
                        }
                        p_bro->setNext(p->getNext());
                        rewrite(p_b,p_bro,filename);
                        delete p;
                        delete p_parent;
                        //rewrite(place,p,filename);
                        return deletereall(k1,p_p,r,filename); //deletp在哪？
                    }
                    else//和youbian兄弟合并
                    {
                        int m=0,q=i;
                        for(;m<num;q++,m++)
                        {
                            p->setvalue(p_bro->getvalue(m),q);
                            p->setOffset(p_bro->getOffset(m),q);
                        }
                        p->setNext(p_bro->getNext());
                        rewrite(place,p,filename);
                        delete p_bro;
                        delete p_parent;
                        return deletereall(k1,p_p,r,filename); //deletp在哪？
                    }
                }
                else //normalnode
                {
                    if(j!=0)//和左边兄弟合并
                    {
                        int m=0,q=num;
                        p_bro->setvalue(k1,q);
                        p_bro->setadree(p->getadree(0),q+1);
                        q++;
                        while(!p->getvalue(m).empty())
                        {
                            p_bro->setvalue(p->getvalue(m),q);
                            p_bro->setadree(p->getadree(m+1),q+1);
                            q++;
                            m++;
                        }
                        //delete(p);
                        rewrite(p_b,p_bro,filename);
                        delete p;
                        delete p_parent;
                        return deletereall(k1,p_p,r,filename); //deletp在哪？
                    }
                    else//和youbian兄弟合并
                    {
                        int m=0,q=i;
                        p->setvalue(k1,q);
                        p->setadree(p_bro->getadree(0),q+1);
                        q++;
                        for(;m<num;q++,m++)
                        {
                            p->setvalue(p_bro->getvalue(m),q);
                            p->setadree(p_bro->getadree(m+1),q+1);
                        }
                        rewrite(place,p,filename);
                        delete p_bro;
                        delete p_parent;
                        return deletereall(k1,p_p,r,filename); //deletp在哪？
                    }
                }
            }
            else//重新分配  记得i有可能等于0
            {
                if(j!=0)//和左边兄弟合并
                {
                    if(p->whoareyou()==2)//叶节点
                    {
                        int op=p_bro->getOffset(num-1);
                        string s=p_bro->getvalue(num-1);
                        p_bro->setOffset(0,num-1);
                        p_bro->setvalue("",num-1);
                        p=insertleafnode(s,op,p);
                        changepar(place,k1,r,s,filename);
                        rewrite(place,p,filename);
                        rewrite(p_b,p_bro,filename);
                        delete p_parent;
                        return r;
                    }
                    else//normalnode
                    {
                        //node* temp=p_bro->getadree(num);
                        int t=p_bro->getadree(num);
                        string s=p_bro->getvalue(num-1);
                        p_bro->setadree(0,num);
                        p_bro->setvalue("",num-1);
                        for(int m=N-1;m>=2;m--)
                        {
                            p->setadree(p->getadree(m-1),m);
                            p->setvalue(p->getvalue(m-2),m-1);
                        }
                        p->setadree(p->getadree(0),1);
                        p->setadree(t,0);
                        p->setvalue(k1,0);
                        rewrite(place,p,filename);
                        rewrite(p_b,p_bro,filename);
                        changepar(place,k1,r,s,filename);
                        delete p_parent;
                        return r;
                    }
                }
                
                else//和you边兄弟合并
                {
                    if(p->whoareyou()==2)//叶节点
                    {
                        int m=0;
                        int op=p_bro->getOffset(0);
                        string s=p_bro->getvalue(1);
                        string s1=p->getvalue(0);
                        p->setOffset(p_bro->getOffset(0),i);
                        p->setvalue(p_bro->getvalue(0),i);
                        for(m=0;m<N-2;m++)
                        {
                            p_bro->setOffset(p_bro->getOffset(m+1),m);
                            p_bro->setvalue(p_bro->getvalue(m+1),m);
                        }
                        p_bro->setOffset(0,m);
                        p_bro->setvalue("",m);
                        changepar(place,k1,r,s,filename);
                        if(where==0) changepar(place,k,r,s1,filename);
                        
                        rewrite(place,p,filename);
                        rewrite(p_b,p_bro,filename);
                        delete p_parent;
                        return r;
                    }
                    else//normalnode
                    {
                        int m=0;
                        int temp=p_bro->getadree(0);
                        string s=p_bro->getvalue(0);
                        p->setadree(temp,i+1);
                        p->setvalue(k1,i);
                        for(m=0;m<N-2;m++)
                        {
                            p_bro->setadree(p_bro->getadree(m+1),m);
                            p_bro->setvalue(p_bro->getvalue(m+1),m);
                        }
                        p_bro->setadree(p_bro->getadree(m+1),m);
                        p_bro->setadree(0,N-1);
                        p_bro->setvalue("",N-2);
                        changepar(place,k1,r,s,filename);
                        rewrite(place,p,filename);
                        rewrite(p_b,p_bro,filename);
                        delete p_parent;
                        return r;
                    }
                }
            }
        }
    }
}

int deletenode(string k,int place,string filename){
    //node* deletenode(string k,node* p){
    node *p=getnode(place,filename);
    int N=(p->gettype()==2)?BTreeSize1:BTreeSize2;
    int root=place;
    //node* root=p;
    int i=-1; //找到包含K的结点
    while(p->Haveson())
    {
        for(i=0;i<N-1;i++)//遍历该节点
        {
            if(p->getvalue(i).empty())//空了
                break;
            if(lenode(k,p->getvalue(i),p))
                break;
        }
        if(i==N-1) //该节点不符合
            place=p->getadree(i);//p=p->getadree(i);
        else//在该节点中
        {
            //if(k== p->getvalue(i))
            if(eqnode(k,p->getvalue(i),p))
                place=p->getadree(i+1);
            else
                place=p->getadree(i);
        }
        delete p;
        p=getnode(place,filename);
    }
    for(i=0;i<N-1;i++)
    {
        if(eqnode(k,p->getvalue(i),p))
            break;
    }
    delete p;
    if(i==N-1) return root; //没有该值
    return deletereall(k,place,root,filename);
}
