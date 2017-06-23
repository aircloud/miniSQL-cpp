select * from City;
select Name, Population from City where ID=3;
select ID, District from City where Name='Herat';
select * from City where ID>50 and Population>200000;

select Language from CountryLanguage;
select * from CountryLanguage where Percentage>=70.0;