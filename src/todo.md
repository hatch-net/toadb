Plan 
====================
* dictionary cache,and query interface 
* flex and grammer support table.column format
* concurrently 
* benchmark test tool
* 条件过滤算子 
* 增加PAX存储模式下的算子，执行计划的叶子节点是，获取某列的扫描，再上一层是获取某行的扫描；这样过滤条件可以放在列扫描上。未来索引也可以建到列级别，这样有效利用PAX存储，减少IO次数；
* 在服务线程中增加关闭线程取消；增加只取消当前执行，在客户端处理ctrl+c；

2024/7/x 

* 事务分发信息的持久化记录；
* 增加信号处理，对于退出信号响应；先退出各线程，由主线程处理退出任务，不需要锁；
* 增加事务查询接口；

## 事务处理
* 事务提交 
* 事务回滚

## 事务信息
* 事务号分发
* 事务号持久化
* 启动恢复事务号
* 事务号的回收

## 事务状态机
* 开启事务
* 结束事务
* 异常结束事务
* 事务中命令的执行


## MVCC机制
* 可见性判断；
* 事务状态记录；内存中和持久化记录；redo log;
* 版本链查找；当前读与快照读；select时找可见版本，回退查找；update/delete时，查找最新版本；
* 事务冲突等待
* 隔离级别，dirty read/read commited/read repeatability



## 过期版本清理 
* undo数据块回收
* 数据块中位置回收
* crash后的数据回收，同上

2024/7/22

# 事务控制

* 增加事务号的统一分发，作为一种时序；
* 每个数据行增加事务操作信息; 
* 增加快照处理；每条SQL执行前会获取快照；
* 基于快照的可见性判断；

# 文件存储管理并发控制 

打开或关闭文件时，会修改虚拟文件描述符的列表。在修改时和查找时需要加读写锁，保存并发修改的一致性。

2024/6/28
# 数据块缓冲区的并发控制

* 查找缓冲区 
* 查找空闲缓存区
* 替换缓存区

在这三种情况种，找到缓存区后，需要立即对缓冲区加锁，再pin住，然后进行二次检查，因为此时其它并发可能已经操作过，不满足时再次进行查找，可能原来不在缓存区时，已经在缓存区中了。

另外，hashtable中插入时，也需要先查找是否已经存在，如果存在时，则放弃当前缓存区的加载，使用已经存在的缓存区。保证相同tag的数据块在缓存中只有一份。

# 文件位置并发控制

在多线程架构下，对于打开的文件会共享文件描述符，此时读写的偏移也是共享的，在操作之前需要加锁，然后指定偏移，再进行读写，然后释放锁。

##  块级的并发访问控制策略 

* 读权限
* 写权限

数据块内容和数据块描述信息的读写，需要进行加读写锁，只读时加读锁。

修改内容，从文件加载内容需要加写锁，这样没有加载完成时，其它并发暂时不能访问。

保证同一数据块只有一个worker 在加载，其它worker等待加载完成时使用。

## 对于块描述信息并发访问策略
* 有tag标识
* pin不可替换标识
* lock加锁标识

2024/6/17
=======================
# 数据字典的并发访问

## 数据字典管理并发控制
* 拆解数据字典查找，将新建部分封为独立接口读写加锁

## 数据字典项并发访问策略
* 读权限，可以执行DML，会有字典项内容的修改；
* 写权限，DDL操作，如create table/delete table
* 字典项中的数据需要进一步拆分


2024/6/13
=======================
# 内存上下文的并发
* 修改内存上下文，每个线程有独立的topmemcontext；
* 不再使用freelist缓存内存块，每次内存块都会申请和释放；

2024/6/3
=======================
# 运行日志 
* 在并发执行时打印并发相关信息；

2024/6/3 
========================

# 数据块缓存池

* 增加hashtable 管理结构；
* 调整bufferpool中usedCnt初始值，避免刚使用的被替换的可能，同时避免遍历循环次数；

# 并发处理 

* 增加线程池，每个线程处理一个客户端的请求任务 
* 增加锁机制，内存修改使用spinLock；需要等待的使用rwLock，区分读写；
# 客户端与服务端调整为CS架构
* 客户端与服务端通过TCP网络交互
* 服务端每执行得到一个结果都会发送到客户端，由客户端进行输出前准备；

# 调整共享数据的并发访问

* 数据字典访问加锁
* 数据块缓冲池访问加锁
* 数据块的访问加锁 
* 内存上下文在各处理线程中进行分叉，各线程独立一个根上下文节点；共享数据在共同的上下文结点中；

# 代码量统计 

[senllang@hatch toadb]$ cloc ./
     158 text files.
     156 unique files.
      79 files ignored.

github.com/AlDanial/cloc v 1.98  T=0.07 s (2234.8 files/s, 421911.6 lines/s)
-------------------------------------------------------------------------------
Language                     files          blank        comment           code
-------------------------------------------------------------------------------
C                               61           3876           3020          15228
C/C++ Header                    70            911           1034           2258
yacc                             1             93             33            828
Bourne Shell                     9            150            135            509
Markdown                         4            123              0            414
JSON                             4              0              0            250
lex                              3             30             43            243
make                             4             62             85            127
-------------------------------------------------------------------------------
SUM:                           156           5245           4350          19857
-------------------------------------------------------------------------------


2024/1/8 
=====================
# 内存上下文的管理
通过内存上下文，对动态申请的内存进行自动管理，避免内存的泄漏，同时也增加了内存边界的校验。
在每个阶段使用独立的内存上下文，在退出阶段时销毁内存上下文，就可以释放此阶段所有动态申请的内存；

对于全局存在的动态内存，在顶层内存上下文中进行分配；

2023/6/27 
=====================
# 资源的释放清理  
设计资源管理，每次申请都加到全局list中，释放时从list中删除，最后退出时检查list中是否有未释放资源 
## 局部资源 

## 事务级资源 
* 解析树, 在每输query处理完后释放; 
* 执行状态 , 在每输query处理完后释放; 
* portal 资源 , 在每输query处理完后释放; 

## 系统级资源  
* 系统字典，在系统退出时释放； 

# 参数输入解析 
* 数据库目录参数 -D 
* 初始化参数 -i 此时新建数据库目录及其它标识文件，如果不带时必须目录存在 
* 帮助参数 -h 
* 版本参数 -v 

# 分层及接口 
## 系统字典接口 
* 初始化 系统字典缓存结构, 适合hash, (表名,id,数据库id)作为键值  
* 创建表 需要插入系统字典 
* 删除表 查找系统字典，先从系统字典中删除，再删除文件 
* 查找表 查找系统字典，缓存中没有时，从文件中加载 
* 销毁  在系统结束时清理资源 
* 同步 多任务时的同步处理 

* 物理存储 
* 数据缓存 

2023/10/26
======================
# insert 
* 支持多个values子句
* 支持带有部分列名的插入
* 对于不带列名时，values需要所有列的值


