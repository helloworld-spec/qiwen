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



向表中增加数据

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



### sqlite3的C函数接口

