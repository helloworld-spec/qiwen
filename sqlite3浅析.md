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

sqlite3的命令输入没有大小写区别，且以分号结尾

打开一个数据库，没有则创建

```
sqlite3 [数据库名.db]
eg:
sqlite3 department.db
```

数据创建一个表

```
creat table [表名]
(	
	[列名1] [数据类型] ["约束"]，
	[列名2] [数据类型] ["约束"]，
	...
	[列名n] [数据类型] ["约束"]
);
eg:

```

查看表

```
.table	
```

查看表的结构

```
.schema	//查看所有表的结构
.schema	[表名]	//查看某个表的结构
```

向表插入新行

```

```

