select * from test_table715;
--begin test
select * from test_table715 where id > 45;
select * from test_table715 where id >45;
select * from test_table715 where id>45;
select * from test_table715 where id> 45;
create table City (  
        ID int, 
        Name char(35) unique,          
        CountryCode char(3), 
        Distinct char(20),
        Population int, 
        primary key(ID)
);
insert into City values(10,"Hangzhou","China","Zhejiang",1000000);
select * from City;
insert into City values(20,"Guangzhou","China","GuangDong",2000000);
insert into City values(30,"Shenzhen","Chi","Shanghai",3000000);
insert into City values(30,"Shenzhen","Chi","Shanghai",4000000);
update City set Distinct = "GuangDong" where Name = "Shenzhen";
select * from City;
select * from City where Name = Shenzhen;
select * from City where Name = "Shenzhen";
select * from City where Name = 'Shenzhen';
create index NameIndex on City(Name);
insert into City values(90,"Beijing","China","Beijing",9000000);
select * from City where ID = 90;
select * from City where ID > 10 and ID = 90;
select ID,Name from City;
select ID,Name from City where ID >= 10 and ID < 90;
delete * from City where Population < 3000000;	
drop index NameIndex on City;	
drop table City;
quit;