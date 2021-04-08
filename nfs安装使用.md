# 全称：Network File System
# 作用：让同一网络上两个主机共享文件，像 ubuntu 和主机共享主机一样便利
## 服务端上安装 nfs：
    1. ```sudo apt-get install nfs-kernel-server```
    2. ```sudo apt-get install nfs-common```
        如果不能安装，需要换源
    3. 编写配置文件
        ```sudo vim /etc/exports```
        配置内容如下：
        ```/home/chenqiwen/nfs *(insecure,rw,sync,no_root_squash)```
    4. 创建共享文件夹，路径要和配置文件的一样
    5. 重启nfs服务
        ```sudo service nfs-kernel-server restart```

## 客户端上安装nfs              
1. 安装驱动
```
sudo apt-get install nfs-common
```
2. 创建本地挂载目录
```
mkdir /tmp/nfs
```
3. 挂载贡献目录
```
mount -t nfs -o nolock 172.16.5.170:/home/chenqiwen/nfs /tmp/nfs
ulimit -c unlimited
echo "/tmp/nfs/%e-%p-%t.coredmp" > /proc/sys/kernel/core_pattern
```

## 开发板配置nfs
~~~c
开发板系统配置支持nfs,运行 nfs_start.sh 脚本启动 nfs 功能，
启动nfs服务后挂载
```
cd ~
nfs_start.sh 
umount /tmp/nfs
rm -rf /tmp/nfs
mkdir /tmp/nfs
mount -t nfs -o nolock 172.16.5.170 :/home/chenqiwen/nfs /tmp/nfs
cd /tmp/nfs
ls
```
以上命令可以写入脚本文件，开机运行脚本。

这时候，在开发板的/tmp/nfs 目录下就可以直接访问 172.16.5.170:/home/chenqiwen/nfs 的内容
~~~


