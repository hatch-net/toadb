# 更新记录
## updated by 2023/8/17 
- 物理存储模式的改变
> * 原来传统的行存储模式，也叫NSM（N-Arr storage model），每一行的数据是连续存放在一起； 
> * 改变后的存储模式为行列混合存放模式 PAX(partition attributes across), 这结合了DSM(Decomposition storage model ) 和 NSM的优势； 
- 对四条SQL语句的适配  
> * 创建表时，不仅创建表数据文件，还要创建分组管理文件，这两者是一一对应的 
> * 在插入数据时，按分组查找空闲空间，如果没有时，先新建分组，再插入数据； 
> * 查询时，按分组来扫描数据；当前只支持 select * 配匹；如果按列时，那么只从磁盘加载对应列的数据即可，这就是PAX模式的优势； 
> * 删除表时，当然也是需要成对删除表数据文件和分组文件  

## updated by 2023/6/17 
- 支持四条SQL语句的执行，分别是

> create table tablename(columnname type, ...);  
> drop table tablename;  

> insert into tablename(columnname,...) values(val...);  
> select columnname,... from tablename;  

当前查找支持全部列表，不支持部分列名的过滤。  

- 支持的类型:   

> int/integer,  整型数字，如88，99等；  
> char, 单个字符，如'a','b'等，用‘’包括起来；  
> varchar, 字符串，也是用‘’包括；  
> bool，布尔值，用数字表示，0是false，非零为true，显示为F/T；  


## updated by 2023/5/26  
完成flex/bison语法解析的框架，能够解析几个关键词，字符串，数字。  

# 数据库SQL解析
分为两部分，
词法分析器scanner
语法分析器grammar

## 目前支持的SQL情况（tag: toadb_01）：
- 支持DDL:
create table name (column type,..);
drop table name;

- 支持DML:
select */ column, ... from tablename;
insert into tablename(column...) values(...);

# 编译执行
## 安装编译工具
- 需要安装gcc ，推荐8.5版本及以上
- 安装 flex 词法解析器， 推荐2.6.1版本及以上
- 安装 bsion 语法解析器，  推荐3.0.4版本及以上

## 编译词法与语法解析器
在toadb/src 当前目录下执行
> make parser
如果没有做修改的话，可以跳过这一步，因为库中已经包含编译后的文件

## 编译数据库
在toadb/src 目录下执行
> make

## 使用教程
在toadb/src 当前目录下执行

* 开始运行  
```
[senllang@localhost src]$ ./toadb-0-01
Welcome to Toad Database Manage System.
```

* 创建表   
```
toadb> create table student(sid integer, sname varchar, ssex char);
```

* 查询表   
```
toadb> select sid,sname,ssex from student;
return 0 rows
```

* 向表中插入数据     
```
toadb> insert into student(sid,sname,ssex) values(1,'lilei','M');
```

* 单行数据查询   
```
toadb> select sid,sname,ssex from student;
|sid-|sname|ssex|
|   1|lilei|   M|
return 1 rows
```

* 插入多行数据      
```
toadb> insert into student(sid,sname,ssex) values(2,'hanmeimei','F');
toadb> insert into student(sid,sname,ssex) values(3,'richel','F');
```

* 查询多行数据      
```
toadb>  select sid,sname,ssex from student;
|sid-|sname----|ssex|
|   1|    lilei|   M|
|   2|hanmeimei|   F|
|   3|   richel|   F|
return 3 rows
```

* 删除表      
```
toadb> drop table student;
```

* 退出toadb     
```
toadb> quit
```

最后输入quit 回车后，退出toadb client。

# roadmap

## 0.2版本开发


## 0.1版本开发

功能：支持创建表，删除表，插入数据，删除数据
数据库特性：客户端，词法/语法解析，SQL执行，数据字典，存储管理


## 开发指南
词法分析器和语法分析器使用了著名的开源工具flex和bison组合，flex能够按照正则表达式规则解析出关键字，标识符，以及其它字符串；而bison则通过它的语法推导规则，将各个部分解析成一个个表达式，每个表达式形成逆波兰式的抽象语法树。

### 词法分析器编写过程的注意事项：
词法分析器是向前看一个字符，所以对于规则在文件中的前后顺序就有要求。比如标识符字串如果放在操作符规则之后，那么在匹配时就会先匹配到操作符的分号，此时就会执行操作符的规则，而把前面的字符串会丢掉，把两者换个顺序就会优先匹配标识符，当下一个字符为分号时，先执行标识符的动作，再取分号进行分析。

### 语法分析器编写过程注意事项：
语法分析器在编写过程中遇到两个问题：
（1）语法分析器规则里要使用当前表达式的值时，需要给表达式定义类型，默认是yylval为整型，如果是字符串时，就需通过%union定义，然后在token里绑定类型；
（2）语法分析器第二部分规则定义部分，规则推导出的内容必须是词法分析器返回的内容，比如列名和类型中间是空格，但是空格在词法分析器中已经过滤掉了，就不能用空格来分隔了。

### 语法树生成


### 创建表的执行
执行过程涉及到执行模块和存储模块，还有数据字典。目前对于数据字典，简单存储在表文件中。
* 执行模块，主要对流程和输入的信息进行组合；
* 在存储模块，主要是文件的操作，如创建表文件，二进制信息的写入；

#### 数据库目录定义
在程序启动时指定，使用-D参数；
* 如果没有创建时，需要加init参数。
* 如果已经存在的目录，需要进行校验；

#### 表文件格式定义
文件以块为单位存储，第一个块是存储元数据信息，也就是数据字典内容；
* 先是表信息，包括块大小，当前块号，列（列名，列类型）列表，有效块的记录：块数量；



