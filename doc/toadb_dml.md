toadb DML操作
====================

[toc]

目前支持简单的select/insert/update/delete操作。

在操作之前确保数据库服务启动正常，以下操作在客户端toadsql中操作。

创建四张表，用户表，产品表，订单表，订单明细表。

```sql
toadb> create table users(user_id int, user_name varchar);
Create table result success
toadb> create table products(product_id int, product_name varchar, price float);
Create table result success
toadb> create table orders(order_id int, user_id int, order_date varchar, status bool);
Create table result success
toadb> create table orderdetails(orderdetail_id int, order_id int, product_id int, quanlity int);
Create table result success
```

# 插入数据

分别在四张表中插入一些数据。

插入操作支持单value和多value批量两种SQL形式，当一次插入多条时，建议使用多value方式，性能更快。

## 单条插入

```sql
toadb> insert into users values(1,'zhangqiang');
total 1 rows sucess
```

## 多条同时插入

```sql
toadb> insert into users values(2,'wangpei'),(3,'liuqi');
total 2 rows sucess

```

其它表的数据插入如下：

```sql
toadb> insert into products values(1,'pencil',10.2),(2,'t-shirt',299),(3,'toy-bear',129);
total 3 rows sucess

toadb> insert into orders values(1,1,'2022-10-1 10:20:00',1),(2,3,'2023-10-1 10:20:00',1),(3,2,'2022-12-1 12:20:00',1);
total 3 rows sucess

toadb> insert into orderdetails values(1,1,2,3),(2,1,1,2),(3,2,3,1),(4,3,1,1);
total 4 rows sucess

```

# 查询数据 

查询语句中，`select targetlist from rangtablelist where qual;`

各子句的支持情况如下：

* targetlist目标列子句，支持指定列名或者`*`代表所有列；
* rangtablelist目标表子句，可以是一个或多个表名，用逗号分隔；
* qual 条件表达式子句，使用比较运算 `>`,`>=`,`=`,`<`,`<=`构成单个条件，多个条件之间使用`and`,`or`逻辑运算符进行关联。

注意：

* 运算必须在同类型之间，目前不支持类型的自动转换。

## 单表查询

```sql
toadb> select * from users;
|user_id|user_name|
|1|                                                 zhangqiang|
|2|                                                 wangpei|
|3|                                                 liuqi|
total 3 rows sucess

```

## 单表带过滤条件

```sql
toadb> select * from products where price > 100.0;
|product_id|product_name|price|
|2|                                                 t-shirt|299.000000|
|3|                                                 toy-bear|129.000000|
total 2 rows sucess

```

## 单表带列筛选

```sql
toadb> select product_name from products where price <= 50.0;
|product_name|
|                                                 pencil|
total 1 rows sucess
```

## 多条件过滤

```sql
toadb>  select * from products where product_id > 2 or price > 100.0;
|product_id|product_name|price|
|2|                                                 t-shirt|299.000000|
|3|                                                 toy-bear|129.000000|
total 2 rows sucess

toadb>  select * from products where product_id > 2 and  price > 100.0;
|product_id|product_name|price|
|3|                                                 toy-bear|129.000000|
total 1 rows sucess

```

## 多表的联合查询

* 笛卡尔积

```sql
toadb> select * from users,products;
|user_id|user_name|product_id|product_name|price|
|1|                                                 zhangqiang|1|                                                 pencil|10.200000|
|1|                                                 zhangqiang|2|                                                 t-shirt|299.000000|
|1|                                                 zhangqiang|3|                                                 toy-bear|129.000000|
|2|                                                 wangpei|1|                                                 pencil|10.200000|
|2|                                                 wangpei|2|                                                 t-shirt|299.000000|
|2|                                                 wangpei|3|                                                 toy-bear|129.000000|
|3|                                                 liuqi|1|                                                 pencil|10.200000|
|3|                                                 liuqi|2|                                                 t-shirt|299.000000|
|3|                                                 liuqi|3|                                                 toy-bear|129.000000|
total 9 rows sucess

```

* 交集

两表的相同列`order_id`进行关联查询, 得到交集，类似于inner join。

```sql
toadb>  select * from orders,orderdetails where orders.order_id = orderdetails.order_id and orders.order_id = 1;
|order_id|user_id|order_date|status|orderdetail_id|order_id|product_id|quanlity|
|1|1|                                                 2022-10-1 10:20:00|T|1|1|2|3|
|1|1|                                                 2022-10-1 10:20:00|T|2|1|1|2|
total 2 rows sucess

```

# 更新数据 

```sql
toadb> select * from users;
|user_id|user_name|
|1|                                                 zhangqiang|
|2|                                                 wangpei|
|3|                                                 liuqi|
total 3 rows sucess

toadb> update users set user_name='zhangguilan' where user_id=1;
total 1 rows sucess

toadb> select * from users;
|user_id|user_name|
|1|                                                 zhangguilan|
|2|                                                 wangpei|
|3|                                                 liuqi|
total 3 rows sucess

```

# 删除数据 

```sql
toadb> select * from orderdetails;
|orderdetail_id|order_id|product_id|quanlity|
|1|1|2|3|
|2|1|1|2|
|3|2|3|1|
|4|3|1|1|
total 4 rows sucess

toadb> delete from orderdetails where orderdetail_id = 1;
total 1 rows sucess

toadb> select * from orderdetails;
|orderdetail_id|order_id|product_id|quanlity|
|2|1|1|2|
|3|2|3|1|
|4|3|1|1|
total 3 rows sucess

```
