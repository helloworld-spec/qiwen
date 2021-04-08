# linux连接无线网络之 wpa_cli, wpa_supplicant
    wpa_supplicant 软件包中包含客户端程序 wpa_cli, 通过它可以直接连接无线网络，
    不需要通常的无线网络配置文件的方式，这在某些特殊情况下是有用的
## 启动 wpa_supplicant
    以 daemon 方式启动 wpa_supplicant:
``` 
wpa_supplicant -B -i wlan0 -D wext -c /etc/wpa_supplicant.conf
-B daemon 模式
-i 无线网卡名
-D 驱动类型
-c 配置文件目录    
```
    其中的 wlan0 为系统内的无线网卡的设备名。配置文件是 /etc/wpa_supplicant.conf ,
    可以在文件中添加连接网络所需要的信息。以下是一个案例tuyav200
```
ctrl_interface=/var/run/wpa_supplicant
ap_scan=1
network={
    ssid="Hiwifi_2.4G"
    key_mgmt=WPA-PSK
    pairwise=CCMP TKIP
    group=CCMP TKIP WEP104 WEP40
    psk="licheng.chn"
    scan_ssid=1
}
```
    ssid wifi 名
    psk WiFi 密码

## 启动 wpa_cli
```
wpa_cli -i wlan0
```

## 增加网络，并设置网络参数
    可以先用 status 命令查询网络连接状态，应该返回
``` wpa_state=DISCONNECTED ```
    可以先扫描网络来查看当前所有的wifi名称
    scan 搜索无线网络
    scan_result 显示搜索结果
    用下面的命令增加网络，并设置相应的参数
``` add_network ```
    该命令会返回新增加的网络ID,一般是0.下面的命令的第一个参数就是网络的id
``` set_network 0 ssid "wyk" ```
    wyk 是无线网名称，要用引号围起来
``` set_network 0 psk "password" ```
    password 代表网络的密码，也要用引号围起来
### 断开网络号
``` disconnect 网络号 ```
### 删除网线
``` remove_network 网络号 ```

## 启用网络
``` enable_network 0 ```
    命令执行后，wpa_cli 会输出连接的过程信息，如果一切正常，最后输出
``` CTRL-EVENT-CONNECTED... 

wpa_supplicant -B -iwlan0 -Dwext -c /etc/jffs2/wpa_supplicant.conf


```

### 配置网关命令

```
sudo route add default gw [网关ip]
```

### 查看 网关命令 

```
route
```

