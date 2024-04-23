#! /bin/sh
#  test toadb 
# create by senllang 2024/1/16
# mail : study@senllang.onaliyun.com
#
# Copyright (c) 2023-2024 senllang
# 
# toadb is licensed under Mulan PSL v2.
# You can use this software according to the terms and conditions of the Mulan PSL v2.
# You may obtain a copy of Mulan PSL v2 at:
# http://license.coscl.org.cn/MulanPSL2
# THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
# EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
# MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
# See the Mulan PSL v2 for more details.
#

dir=`pwd`
toadb_command="/home/senllang/Dev/toadb/src/tools/tsql/toadsql";
toadb_datadir="/home/senllang/toadbtest/"
result_out="/tmp/result.log"

toad_executor_command="${toadb_command} -D ${toadb_datadir} -C"

function excuteSql() 
{
    sql=$1
    echo "excute $sql" | tee -a ${result_out}
    sql_command="${toad_executor_command} \"${sql}\" "
    sh -c "${sql_command}" | tee -a ${result_out}
}

function createSimple()
{
    excuteSql "create table student(sid int, sname varchar);"
}

function testSimpleQuery()
{
    excuteSql "select sid from student;"
    excuteSql "select * from student;"
    excuteSql "select sname from student where sid > 1;"
    excuteSql "select * from student a ,student b;"
}

function testSimple() 
{    
    excuteSql "insert into student(sid,sname) values(1,'lilei'),(2,'wangming'),(3,'hanmeimei'),(4,'markhu');"
    #testSimpleQuery;
}

function restoreSimple()
{
    excuteSql "drop table student;"
}

function createMultiFromClause() 
{
    excuteSql "create table student(sid int, sname varchar);"
    excuteSql "create table teacher(tid int, tname varchar);"
    excuteSql "create table course(cid int, cname varchar);"
    excuteSql "create table timetable(tcid int, ttid int, tsid int);"
}

function restoreMultiFromClause() 
{
    excuteSql "drop table student;"
    excuteSql "drop table teacher;"
    excuteSql "drop table course;"
    excuteSql "drop table timetable;"
}

function testQueyqualClause()
{
    excuteSql "select * from student,teacher,course;"
    excuteSql "select tid,sid,cid from student,teacher,course;"
    excuteSql "select * from student,teacher,course,timetable where timetable.tsid=student.sid;"
    
    excuteSql "select * from student where id > 3;"    
    excuteSql "select * from student a,student b where a.sid > 3;"
    excuteSql "select * from teacher a,teacher b where a.sid=b.tid and a.sid >= a and b.tid <=4;"
    excuteSql "select * from teacher a,student b where a.id=b.id;"
}

function testMultiFromClause() 
{
    excuteSql "insert into student(sid,sname) values(1,'lilei'),(2,'wangming'),(3,'hanmeimei'),(4,'markhu');"
    excuteSql "insert into teacher  values(1,'w'),(2,'z'),(3,'s'),(4,'t');"
    excuteSql "insert into course values(1,'e'),(2,'h'),(3,'f'),(4,'o');"
    excuteSql "insert into timetable values(1,1,1),(2,3,2),(3,2,1),(4,3,4);"

   # testQueyqualClause;
}

function testStudent()
{
    excuteSql "update student set sname='lilei1' where sid=1;"
    excuteSql "update student set sname='wangmin' where sid=2;"
    excuteSql "update student set sname='hanmeimei301' where sid=3;"
    excuteSql "update student set sname='hanmeimei3001' where sid=3;"
    excuteSql "update student set sname='hanmeimei31' where sid=3;"
    excuteSql "select * from student;"
}

function testTearch()
{
    excuteSql "update teacher set tname='wangxiaohua11' where tid=1;"
    excuteSql "update teacher set tname='zhangduoduo22' where tid=2;"
    excuteSql "update teacher set tname='shixiaodong33' where tid=3;"
    excuteSql "update teacher set tname='shixiaodong333' where tid=3;"
    excuteSql "update teacher set tname='shixiaodong3' where tid=3;"
    excuteSql "select * from teacher;"
}

function testUpdateStmt()
{
    testStudent;
    testTearch;
}

function createAll()
{
    createSimple;
    createMultiFromClause;
}

function TestAll() 
{
    testSimple;
    testMultiFromClause;
    testUpdateStmt;
}

function restoreAll()
{
    restoreSimple;
    restoreMultiFromClause;
}

function excutorTest()
{
    #restoreAll;
    createAll;
    TestAll;
#    restoreAll;
}

excutorTest;

