# 更新记录
## updated by 2023/11/15 
### 概述 
主要更新了解析树，增加了查询树和执行计划树的生成，这有利于带条件SQL和复杂SQL处理，同时节点化各个关系代数运算后，有利于执行计划优化的实现。与此同时，更新了执行器，可以通过输入的计划树来执行，不再像以前通过SQL解析结果。

### 更新内容 
下面将分别描述更新的内容：
* 解析树 
> 在词法分析、语法分析的过程中，会生成解析树，将用不同节点来表示各子句及存储它们传递的信息，这样有助于后续步骤的开展。在开发解析树时，尽可能与存储模型，数据库类型无关，希望做成一个通用的SQL解析器，它的输入就是SQL字符，输出就是解析树，不像现在每种类型的数据都有一个解析器。真正与数据库，存储模型相关的，是在下一步骤，也就是查询树。

* 查询树
> 查询树，也叫做逻辑执行计划，它的主要是把SQL用关系代数进行表达，不同关系代数用不同节点表示，树的层次来表示节点之间的关系。当然在转换的过程中，会对SQL中的数据进行检查，比如表是否存在，表的元数据是否一致等；同时还会对一些SQL子句进行展开和替换，比如对`sellect *`, 将它的结果目标列替换为所有的属性列，这就是常说的重写的过程。当然，在真实数据库中，重写阶段还会对视图，规则等进行处理。

* 计划树
> 计划树，也叫执行计划，或者叫做物理执行计划树，它是一个树状结构，每个节点的类型对应着真正要执行的动作。当然未来会在生成计划树之前，可以再增加一个优化器，对各种可能的SQL子句进行重新整理，提升子句，逻辑检查等，这将大大提升执行的效率。

* 执行器
> 之前只对简单的SQL执行全表查询，更新后通过输入的物理执行计划，只需要递归的遍历执行计划的节点，按对应节点的类型执行相应动作即可，然后返回target对应的元组，更新后执行器更加通用，更加灵活。
> 未来增加索引，优化节点执行动作，增加节点执行接口，这些将变成局部修改。

* 内存管理
> 之前对于动态内存申请并没有进行释放管理，本次增加了一个简单的内存管理，实现一种内存上下文的机制，在上下文使用完成后，会统一释放之前上下文中申请的内存。在程序退出时，会释放所有申请的内存。同时，为了方便调式，对于动态申请的内存，标定了内存的起始和结尾，这样可以方便探测到内存越界的情况。
> 这是一个公共组件，目前只是简单实现，后期与数据库的缓存管理一起进行模块级实现。

### 举例说明 
下面举几个例子说明一下变化情况。
* insert 语句

> 比如`insert into tablename(...) values(...),(...);` ,这条插入语句。

> 之前是通过遍历values子句，循环执行执行表的动作。

> 更新之后，values子句转换为一个虚似的表，在执行计划中，会有一个表扫描的节点，将返回的元组，再执行一个ModifyTable动作。这样就是一个通用的操作，未来很方便的支持`insert into ... select ...`这种形式，或者更复杂的形式。

* select 语句 
如上所述，之前只有一个表的全表扫描。现在对于表扫描，实现一种迭代器的扫描方式，每次可以返回下一个扫描到的元组。对于多表和带条件表达式时，增加控制节点，比如两个表的连接，会增加嵌套循环节点，先从外表中拿到一个元组，再从内表中依次扫描元组，如果两表都有元组，则输出结果，当内表遍历完时，从外表再扫描下一个元组，再从头遍历内表。

目前只有顺序扫描和嵌套循环两种实现路径。

### 支持情况 

目前toadb支持的SQL语法有：
* DDL 
> `create table tableName(columnName type,...);` 
> `drop table tableName;`
创建表和删除表的SQL支持；

* DML
> `insert into tablename(...) values(...),(...);`
向表中插入数据，可以是一个values，也可以带多组值；

* DQL
> `select columName,.. from tableName,.. where qual and qual or qual;`
可以查询单个或多个表，带有条件(目前需要带一个条件)，目前对条件并没有处理，只是简单的返回全表。当查询为多表时，会以积的形式返回结果，也就是两边非空的所有列。

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
[senllang@hatch src]$ ./toadb-0-01 -D ./toadbtest
Welcome to Toad Database Manage System.
cwd :/mnt/sda1/data/gitdata/hatch/Coder/operator/toadb/src
datadir: [./toadbtest]
toadb>

```
> -D 参数指定数据文件存放的目录，最后不需要加'/', 最好是绝对路径; 
> 之后创建的表，对应的表文件就会存放在此目录下 

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
> 注意： 
> 目前数据中不能带有空格，这是在词法分析中的问题。 比如'han meimei'这样就会解析错误。 
> 
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
## 0.5 版本开发
* 功能
  事务机制，保证事务的原子性，一致性；
   
* 限制

## 0.4 版本开发
* 功能 
  执行计划功能开发，在SQL解析后，重写查询树，生成执行计划树，执行器按照执行计划进行；

* 限制 
  
## 0.3 版本开发 
* 功能：
  完善SQL解析，支持select ... from ... where ... 简单语句；、

* 限制：
  暂不支持 table.column带 `.`的格式；

## 0.2版本开发
功能：行列混合的存储模式

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



