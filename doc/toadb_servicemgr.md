toadb服务管理 
==================

[toc]

# 创建数据目录

首先需要创建一个数据存放的目录，在使用数据库过程中，产生的用户数据都会存放到此目录下。

```shell
# 在任意目录下
[senllang@hatch src]$ mkdir ~/toadbdir
```

# 启动服务 

数据库服务最新支持的模式为C/S模式，即客户端与服务端分离，之间通过TCP通信。

```shell
# 在toadb目录下
[senllang@hatch toadb]$ cd src/
[senllang@hatch src]$ ./toadb-0-01 -M 2 ~/toadbdir
Welcome to Toad Database Manage System.
datadir: [./toadbtest]
start toadb runMode 2 ...

```

**参数说明**
* `toadb-0-01` 数据库主程序；
* `-M` 指定运行模式，`2`即为C/S模式；
* `~/toadbdir`, 数据目录地址；

服务端启动默认端口为`6389`, 服务端的IP `any`。

启动之后，数据库服务运行在后台，可以通过ps命令查看。

```shell
[senllang@hatch src]$ ps -ef|grep toadb
senllang 1286719       1  0 16:25 pts/22   00:00:00 ./toadb-0-01 -M 2 /home/senllang/toadbdir
```

# 运行日志 

在数据库服务启动后，会生成一个.log为后缀的运行日志文件。

它是以启动时间来命名，所以每次启动都会生成一个新的日志文件。

```shell
[senllang@hatch src]$ ll *.log
-rw-r--r--. 1 senllang develops 5710 Aug 13 16:25 Toadb2024_08_13_08_25_13.log
```

# 停止服务 

通过ps 查看后台数据库服务进程ID，发送`SIGTERM`信号，可安全退出。

也可以使用`src/stop.sh`脚本进行停止，它是centos 8上编写，其它平台可能需要适配修改。

```
[senllang@hatch src]$ ./stop.sh
1286719
[senllang@hatch src]$ ps -ef|grep toadb
senllang 1289280 2782831  0 16:43 pts/22   00:00:00 grep --color=auto toadb
```

**注意**：数据库服务异常停止，或者使用`kill -9` 会引起数据库数据的事务不一致，最新数据会不可见。

