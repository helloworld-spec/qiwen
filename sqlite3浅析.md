### 概述

SQlite是一个轻量级、开源、嵌入式关系数据库，

可移植性好、使用简单、很小、高效、不需要网络

### 软件环境

Ubuntu 18.04 、sqlite3 

### 下载安装

安装

```
sudo apt-get install sqlite3
```

查看版本信息

```
sqlite3 -version
```

### 数据库字段类型

### Ubuntu命令行使用sqlite3

sqlite库包含一个sqlite3的命令行，可以让用户手工输入并执行sqlite数据库SQL命令.

打开一个数据库，没有则创建

```
sqlite3 [数据库名.db]
eg:
sqlite3 department.db
```

数据库创建一个表

```c
creat table [表名]
(	
	[列名1] [数据类型] ["约束"]，
	[列名2] [数据类型] ["约束"]，
	...
	[列名n] [数据类型] ["约束"]
);
eg:
CREATE TABLE library(
number int "primary key",
name varchar "not null",
price real "default",
type text "not null",
amount int "not null"
);
```

删除一个表

```c
drop table [表名];
eg:
drop table library;
```

修改一个表的结构

```
修改表的名字：
ALTER TABLE [表名] RENAME TO [新名字]
eg:
sqlite> .table
library
sqlite> ALTER TABLE library RENAME TO library2;
sqlite> .table
library2
增加字段：
ALTER TABLE [表名] ADD [字段名] [字段类型]；
sqlite> .schema
CREATE TABLE IF NOT EXISTS "library"(
number int "primary key",
name varchar "not null",
price real "default",
type text "not null",
amount int "not null"
);

sqlite> ALTER TABLE library ADD remark text;

CREATE TABLE IF NOT EXISTS "library"(
number int "primary key",
name varchar "not null",
price real "default",
type text "not null",
amount int "not null"
, remark text);

sqlite> INSERT INTO library VALUES (2,"English",23.5,"langue",562,"note");

sqlite> SELECT * FROM library ;
1|chinese|23.5|langue|236|
2|English|23.5|langue|562|note
```

查看表的结构

```c
.schema	//查看所有表的结构
.schema	[表名]	//查看某个表的结构
sqlite> .schema library
CREATE TABLE library(
number int "primary key",
name varchar "not null",
price real "default",
type text "not null",
amount int "not null"
);
```

查看表

```
.table	
```



向表中增加数据N

```c
insert into [表名] values([值1], [值2]...);
eg:
INSERT INTO library VALUES (
01, "chinese", 23.5, "langue", 52);
```

数据库的运算符

```
= 等于
<> 不等于
> 大于
< 小于
如果有多个条件：
and 并且
or 或者
```

从表中删除数据

```c
DELETE FROM [表名] WHERE [列] [运算符] [值];
eg:
DELETE FROM library WHERE name="English";
```

修改表中数据

```c
UPDATA [表名] SET [列名1]=[新值]，[列名2]=[新值]... WHERE [列名] [运算符] [值]；
sqlite> select * from library ;
1|chinese|23.5|langue|52
sqlite> UPDATE library SET amount=236 WHERE name = "chinese";
sqlite> select * from library ;
1|chinese|23.5|langue|236
```

查找表中数据

```c
select [列名1], [列名2], ... from [表名] where [列] [运算符] [值];
sqlite> SELECT * FROM library ;c
1|chinese|23.5|langue|236
```

### sqlite3C程序编译环境准备

1、需要准备sqlite3的源码

sqlite-autoconf-3110100

2、代码编译

```c
tar -zxvf sqlite-autoconf-3110100.tar.gz
cd sqlite-autoconf-3110100
./configure -prefix=[绝对路径] #-prefix用来指定sqlite目标路径
make
make install
/*顺利执行后就会在指定路劲下生成
-bin:存放可执行文件
-include:头文件
-lib：库文件路径*/
```

3、移植

```
- 将bin目录下的文件拷贝到 /usr/local/bin
- 将lib目录下的文件拷贝到 /usr/local/lib 拷贝过程需要特别注意软连接
- include –> /usr/local/include
- share –> /usr/local/share
```

### sqlite3的C函数接口

**1、sqlite3_open: 用来打开或创建一个sqlite3数据库引擎的连接**
在sqlite3数据库引擎中，用结构体sqlite3来表示 sqlite3的数据库引擎 的连接。
我们在调用sqlite3_open这个函数时，它会给我们创建一个sqlite3数据库引擎的连接。

```c
int sqlite3_open(
            const char *filename, //database fiename, 你要打开或创建的数据库的文c件名
            sqlite3 **ppdb;  //sqlite3这个结构体的二级指针
        );
返回值:
    成功返回 SQLITE_OK, 并且ppdb指向新创建的sqlite3数据库引擎的连接
    其他值，表示失败。
```

#### 2、sqlite3_exec:操作一个SQL引擎的数据库系统，实际上就是在这个数据库引擎上执行SQL语句

```c
int sqlite3_exec(
        sqlite * db; //指向sqlite3数据库系统引擎的连接
        const char *sql; //你要执行的SQL语句的字符串
        int (*callback)(void *, int, char **, char **), //函数指针，指向回调函数
        void *, //将作为callback的第一个参数，传给回调函数
        char **errmsg //指向错误字符串
        );
    返回值:
        //成功返回0
        //失败返回其他值，错误信息在errmsg
        
        int (*callback)(void *, int, char **, char **), //函数指针，指向回调函数
        callback主要是在SQL语句为SELECT时用，SELECT返回的结果是一个二维表， 在
        sqlite3_exec实现查询语句，每查到一条记录，就会把结果返回， 每查到一条符合
        条件的记录，就调用callback指向的函数。
            int (*callback)(void *,  //
                            int,  	//结果中多少列
                            char **, //char* column_value[], 指针数组，每列的值
                            char **, //char* column_name[], 指针数组，每列的字段名
                            )
```

#### 3、关闭数据库连接

```
int sqlite3_close(sqlite3 *ppDb);
```

### 使用sqlite3_exec()函数完成增删改查

增：

```c
sqlite> select * from library;	
1|English|0.5|language|83
2|Chinese|22.3|language|88
****************after insert****************
china@ubuntu:/mnt/hgfs/WmShare$ gcc insert.c -lsqlite3 -o insert
china@ubuntu:/mnt/hgfs/WmShare$ ./insert
china@ubuntu:/mnt/hgfs/WmShare$ sqlite3 test.db 
SQLite version 3.23.0 2018-03-27 15:13:43
Enter ".help" for usage hints.
sqlite> select * from	library;
1|English|0.5|language|83
2|Chinese|22.3|language|88
3|math|34.0|langue|112
```

删、改：略

查：

```c
sqlite> select * from library;
1|English|0.5|language|83
2|Chinese|22.3|language|88
3|math|34.0|langue|112
*************after select*****************
china@ubuntu:/mnt/hgfs/WmShare$ vim select.c 
china@ubuntu:/mnt/hgfs/WmShare$ gcc select.c -o select -lsqlite3
china@ubuntu:/mnt/hgfs/WmShare$ ./select
number	name	price	type				amount	
1		English		0.5			language	83	
2		Chinese		22.3		language	88	
3		math		34.0		langue		112	
```

