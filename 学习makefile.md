# 变量赋值
    makefile没有类型，都是字符串
## 简单赋值 :=
    A:=123
    B:=$(A)
    C:=A   
    ==>>
    $A 123
    $B 123
    $C A
    
## 递归赋值 =  
    C=$(D)
    D=123
    ===>>
    $C 123
    $D 123
    
## 追加赋值 +=
    A:=123
    A+=456
    ===>>
    $A 123456
    
## 条件赋值 ?=
    当变量没有定义或者没有赋值时，才会给它赋值
    A?=hello
        因为之前A没有定义，$A hello
    A:=hello
    A?=xxxx
    ===>>
    $A hello
    
## 自动变量
### 目标文件的完整名字 $@
### 所依赖的文件 $^
### 第一个依赖文件的名字 $<
### 所有时间戳比目标文件晚的依赖文件 $?
    如：工程中有main.c a.c b.c,要生成一个可执行文件main
    TARGET:=main
    $(TARGET):main.o a.o b.o
        gcc $^ -o $@
    main.o:main.c
        gcc $^ -o $@
    a.o:a.c
        gcc $^ -o $@
    b.o:b.c
        gcc $^ -o $@
    clean:
        rm %.o
        
## makefile的通配规则
### %表示一个任意的文件名(不包括扩展名)
### %.xxx 表示所有扩展名是xxx的文件
    
# makefile中的函数
## 调用内置函数
$([函数名] [函数参数列表])
## 调用shell命令 或者 得到函数返回结果

$(shell [shell命令] [命令参数]) $([函数名] [函数参数])   

### 文件名展开函数 wildcard
展开成所有符合 wildcard 的参数描述的文件名，文件名之间以空格隔开   
CSRCS:=$(wildcard *.c)  
将当前目录下查找所有以 .c 结尾的文件名，然后赋值给CSRCS

### 模式字符串替换函数 patsubst 
CSRCS:=$(wildcard *.c)  
===>>   
    $CSRCS a.c b.c c.c  
OBJS:=$(patsubst %.c,%.c $(CSRCS))  
    把 CSRCS 中的文件名，变成同名的 .o 文件，赋值给 OBJS
    
### 加前缀函数 addprefix
$(addprefix [prefix], [name1 name2])    
把 prefix 加到 name 序列中的每一个元素前面    
res = $(addprefix %., c cpp)    
===>>   
    $res %.c %.cpp
    
### foreach函数
$(foreach var, list, text)
把参数list中的单词逐一取出来放到 var 所指的变量中，然后再执行 text 所包含的表达式。   
每一次 text 会返回一个字符串，循环过程中，text 所返回的每一个字符串以空格隔开。最后  
结束循环时，text 所返回的每个字符串所组成的整个字符将会是 foreach 函数的返回值
name := test main log caffe     
filesd := $(foreach n, $(name), $(n).o)    
===>>       
$filesd test.o main.o log.o caffe.o
   
    
    
    
    
    