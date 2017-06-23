create table City (
  ID int,
  Name char(35) unique,
  CountryCode char(3),
  District char(20),
  Population int,
  primary key(ID)
);


insert into City values (1,'Kabul','AFG','Kabol',1780000);
insert into City values (2,'Qandahar','AFG','Qandahar',237500);
insert into City values (3,'Herat','AFG','Herat',186800);
insert into City values (4,'Mazar-e-Sharif','AFG','Balkh',127800);
insert into City values (5,'Amsterdam','NLD','Noord-Holland',731200);
insert into City values (6,'Rotterdam','NLD','Zuid-Holland',593321);
insert into City values (7,'Haag','NLD','Zuid-Holland',440900);
insert into City values (8,'Utrecht','NLD','Utrecht',234323);
insert into City values (9,'Eindhoven','NLD','Noord-Brabant',201843);
insert into City values (10,'Tilburg','NLD','Noord-Brabant',193238);
insert into City values (11,'Groningen','NLD','Groningen',172701);
insert into City values (12,'Breda','NLD','Noord-Brabant',160398);
insert into City values (13,'Apeldoorn','NLD','Gelderland',153491);
insert into City values (14,'Nijmegen','NLD','Gelderland',152463);
insert into City values (15,'Enschede','NLD','Overijssel',149544);


select * from City;
select Name, Population from City where ID=3;


delete from City where ID>50 and Population>200000;


select * from City where Name='Haag';
create index name_idx on City(Name);
select * from City where Name='Haag';

insert into City values (16,'Haarlem','NLD','Noord-Holland',148772);
insert into City values (17,'Almere','NLD','Flevoland',142465);
insert into City values (18,'Arnhem','NLD','Gelderland',138020);
insert into City values (19,'Zaanstad','NLD','Noord-Holland',135621);
insert into City values (20,'s-Hertogenbosch','NLD','Noord-Brabant',129170);

select * from City where Name='Almere';
drop index name_idx on City;
select * from City where Name='Almere';


drop table City;
