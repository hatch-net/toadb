客户端使用
=============
[toc]

# 概述

在Client-Server模式下，客户端是独立的一个工具 `toadsql`。

# 客户端路径

客户端的编译后的目录在`src/tools/tsql/toadsql`

# 客户端参数

使用`--help`查看帮助信息。

```shell
[senllang@hatch toadb]$ src/tools/tsql/toadsql --help
Welcome to toadb client - toadsql.
toadsql argments list: -D datapath , enter toadb command client.
-C "sqlstring" , execute sql once only.

Running Mode Select:
--i , Single client Mode, Server/Client is the same process.
--d , Single sql Mode, Server/Client is the same process.
--s , Server running with deamon mode, only server start.
default, Server running deamon mode, client/server communicate with network.
--v , show version.
--h , show help.

```

`-C` 可以带SQL字符串，在命令行调用时使用。

默认是C/S模式启动客户端；

# 启动客户端

在命令行通过路径启动客户端。

```shell
[senllang@hatch toadb]$ ./src/tools/tsql/toadsql
toadb>
```

客户端启动后，进入SQL输入模式当中。