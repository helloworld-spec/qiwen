## 安装
    ```sudo apt-get install samba samba-common```

## 设置共享的目录权限
    ```sudo chomd 777 /home/china/share```

## 配置samba
    1. 修改配置文件
    ```sudo vim /etc/samba/smb.conf```
    
    2. 跳转到261 - 272行
    ```
    [shhare]
    comment = share folder
    browseable = yes
    path = /home/china/share
    create mask = 0777
    directory mask = 0777
    valid users = china
    force user = china
    force group = china
    public = yes
    available = yes
    writable = yes
    ```
    
## 添加用户
    ```sudo smbpasswd -a china```
    同时会提示输入密码

## 重启samba 服务
    ```sudo /etc/init.d/smbd restart```

## windows端
    win + R打开运行窗口
    ``` \\192.168.10.231\share ```
    输入刚添加的用户名和密码就可以访问共享文件了
    
