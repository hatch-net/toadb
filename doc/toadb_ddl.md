toadb DDL操作
====================

[toc]

# 支持的DDL
toadb目前支持的DDL只有两种：

* 创建表
* 删除表 

# 支持的类型 

支持的数据类型有：

* int, integer， 整型数字类型
* varchar, string, 字符串类型，需要用单引号包括`’string'`
* float, double, 浮点数类型 
* char , 单字符类型
* bool, 布尔类型 , 用 `1` ,`0`数字，或者用字符串 `'true'`，`'false'`表示；

toadb对于输入的SQL字符串的大小写不敏感，可以采用大写，也可以小写，或者混合都视为一样。

类型具体定义详见 `src/sqlcore/dictionary/dataTypes/dataTypes.h` 。

# 创建表 

```sql
toadb> create table book(id integer, name varchar, price float, amount int, isSales bool);
Create table result success

```

# 删除表 

```sql
toadb> drop table book;
Drop table result success

```





