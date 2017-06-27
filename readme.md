### 一个c++实现的mini数据库

本项目是浙江大学数据库系统课程项目设计作业，已经顺利通过所有验收。

>注意：建议使用macOS的Xcode进行操作，可以直接打开NewSQL.xcodeproj文件(内建虚拟目录)。windows下没有尝试，并不保证完全成功。

总共实现的内容如下：

| 功能	| 样例 |
| ------ | ------ |
| create | create table	"create table City (ID int,Name char(35) unique,CountryCode char(3),District char(20),Population int,primary key(ID));"|
| insert	| insert into ChinaInfo values(1,北京,东城区,NONE,800000,CHINA);|
| delete	| delete from City where ID=3;delete from City;|
| select	| select * from City where Name>='Morn' and name<='Tbessa'; select ID,Name from City where ID >= 10 and ID < 90; |
| update	| update City set Distinct = "GuangDong" where Name = "Shenzhen"; |
| drop table	| drop table city;|
| create index	| create index NameIndex on City(Name);|
| drop index |	drop index NameIndex on City; |
| 注释	| --begin test |


### 提示1

如果你正在上“数据库系统”相关课程，并且课程要求实现一个c++版本的mini数据库，可以采用这个项目作为参考，但不建议全盘照搬，个人认为最有效的提升是在我的基础上进行重构，实现诸如不定长存储(包括索引)，跨buffer存储等内容，并且可以在效率上有所考虑。

(如果你这样做了，欢迎在[issue](https://github.com/aircloud/miniSQL-cpp/issues)里面交流)

### 提示2

目前这个系统虽然通过了课程测试，但由于是我自己写的，还有很多不完善的地方，还有很多地方自己设计的比较蠢，目前并不适合用在生产环境中。