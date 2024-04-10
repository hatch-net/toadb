toadb性能测试
=======================================================
# 概要说明 

软件的迭代开发过程中，它的功能，稳定性，性能，易用性能各方面都在发生着变化，随着软件的庞大，已经不能通过人工手段进行觉察，所以需要配套测试工具，由自动化来验证原有功能的完整性，和增加功能带来的变化效果。

在toadb起始，我们准备了两套自动化测试工具集，一套是功能测试，一套是性能兼稳定性测试，它们都是通过shell脚本来编写，调用客户端工具tsql来执行SQL。

# 功能测试 

功能测试自动化脚本位于，toadb/src/test/下，使用的前提是，在toadb/src下执行`make client`，将tsql工具编译出来。

然后在命令行运行`./src/test/startTest.sh`，它会将结果输出到当前终端和当前目录下的.log文件中。

# 性能测试工具 

在源码根目录下toadb/test/benchmark/，模拟TPC-B标准实现了四张表的增删改查。
需要两步来执行：

* 创建测试表与数据 
这一步是由`benchmarkBuildData.sh`来完成，同时需要输入scale因子，它会将同名表删除，再创建新的表，同时插入新的数据。数据量根据scale 乘以每张表的初始数量。

测试会创建以四张表，`toad_history`、`toad_branches`、`toad_tellers`、`toad_accounts`。

* 测试
测试是由`benchmarkRun.sh`脚本来完成，需要输入 scale因子，测试模式，运行时间，输出间隔时间。其中scale因子需要和创建的测试数据一致，运行时间以秒为单位，输出间隔默认为10s打印一次结果。

测试模式用数字表式，有三种模式可选：
> * 1 是`TPC-B`模式, 混合读写模式； 
> * 2 是`update`模式， 只执行udpate SQL语句；
> * 3 是`select` 模式， 也就是只读模式；

# 性能测试数据 

## toadb-06 测试 
* 测试配置
测试机配置 1CPU, 8*2 cores ,2.5GHZ 
磁盘 SSD

* 测试数据量 
scale = 10 
最大表10*100000条数据(一百万条)

* 加载数据时间 
用时12h2m2s 

```shell
[senllang@hatch benchmark]$ ./benchmarkBuildData.sh 10
... 

insert into toad_accounts aid:1000000
load toad_accounts endTime: 2024-01-26 02:35:09
begin time:2024-01-25 14:33:07
end time  :2024-01-26 02:35:09
---------------------------------------------------------
load data finish.
total elapse time: 12h2m2s
create table elapse time:
total of load table data elapse time: 12h2m2s
total of load toad_branches data elapse time:
total of load toad_tellers data elapse time:
total of load toad_accounts data elapse time: 12h2m2s
```


* 加载数据大小 
```shell
[senllang@hatch toadbtest]$ ll -h
-rw-r--r--. 1 senllang develops 732K Jan 26 02:35 grp_toad_accounts
-rw-r--r--. 1 senllang develops 8.0K Jan 25 14:33 grp_toad_branches
-rw-r--r--. 1 senllang develops 4.0K Jan 25 14:33 grp_toad_history
-rw-r--r--. 1 senllang develops 8.0K Jan 25 14:33 grp_toad_tellers
-rw-r--r--. 1 senllang develops 143M Jan 26 02:35 toad_accounts
-rw-r--r--. 1 senllang develops  16K Jan 25 14:33 toad_branches
-rw-r--r--. 1 senllang develops 4.0K Jan 25 14:33 toad_history
-rw-r--r--. 1 senllang develops  20K Jan 25 14:33 toad_tellers
```

* 只读测试
```shell
[senllang@hatch benchmark]$ ./benchmarkRun.sh 10 3 600 5
scale=10 runmode=3
command infomation:/home/senllang/Dev/toadb/src/tools/tsql/toadsql -D /home/senllang/toadbtest/ -C
datadir infomation:/home/senllang/toadbtest/
pwd : /home/senllang/Dev/toadb/test/benchmark
nbranches :10
ntellers :100
naccounts :1000000


Test process elapse(s) 570 tps=136.68
Test process elapse(s) 580 tps=136.70
Test process elapse(s) 590 tps=136.70
Test process elapse(s) 600 tps=136.69
---------------------------------------------------------
test end.............
begin time: 2024-01-26 07:50:25
  end time: 2024-01-26 08:00:26
Test process total runtimes(s)  :10m1s
Test process total transactions :82156
Test process average tps=136.69

```

