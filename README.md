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


## 开发指南
词法分析器和语法分析器使用了著名的开源工具flex和bison组合，flex能够按照正则表达式规则解析出关键字，标识符，以及其它字符串；而bison则通过它的语法推导规则，将各个部分解析成一个个表达式，每个表达式形成逆波兰式的抽象语法树。

### 词法分析器编写过程的注意事项：
词法分析器是向前看一个字符，所以对于规则在文件中的前后顺序就有要求。比如标识符字串如果放在操作符规则之后，那么在匹配时就会先匹配到操作符的分号，此时就会执行操作符的规则，而把前面的字符串会丢掉，把两者换个顺序就会优先匹配标识符，当下一个字符为分号时，先执行标识符的动作，再取分号进行分析。

### 语法分析器编写过程注意事项：
语法分析器在编写过程中遇到两个问题：
（1）语法分析器规则里要使用当前表达式的值时，需要给表达式定义类型，默认是yylval为整型，如果是字符串时，就需通过%union定义，然后在token里绑定类型；
（2）语法分析器第二部分规则定义部分，规则推导出的内容必须是词法分析器返回的内容，比如列名和类型中间是空格，但是空格在词法分析器中已经过滤掉了，就不能用空格来分隔了。




# 编译执行
## 安装编译工具
- 需要安装gcc ，推荐8.5版本及以上
- 安装 flex 词法解析器， 推荐2.6.1版本及以上
- 安装 bsion 语法解析器，  推荐3.0.4版本及以上

## 编译词法与语法解析器
在toadb 当前目录下执行
> make parser

## 编译数据库
在toadb 当前目录下执行
> make

## 运行
在toadb 当前目录下执行
./toadb-0-01
create table abc (int a, int c);
ident :abc
ident :int
ident :a
attrdef
ident :int
ident :c
attrdef
sql: create
create stmt
drop table ata;
ident :ata
sql drop
drop stmt
^C

最后是ctrl+c结束，输入SQL回车后，打印解析内容

# roadmap

## 0.1版本开发

功能：支持创建表，删除表，插入数据，删除数据
数据库特性：客户端，词法/语法解析，SQL执行，数据字典，存储管理