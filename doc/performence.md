toadb性能测试
=======================================================
# 概要说明 


# 性能测试数据 

## 2024/1/26 测试 
scale = 10 

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

