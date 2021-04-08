### ifi无线网卡访问互联网和作为个人热点

嵌入式设备想要联网，有有线和无线两种方式

有线就是插上网卡，接入网线

无线网卡有两种：usb wifi、sdio wifi

usb wifi使用usb接口( Universal Serial Bus),是连接计算机系统与外部设备的一种串口总线 标准 

sdio wifi使用sdio接口，是在sd内存卡接口基础上发展的接口 

### wifi 网卡有两种工作模式：

### 无线终端模式(STA) :通过该模式连接网络上网

### 无线热点模式(AP)：通过这个模式产生热点给其他设备上网

### 无线网络的安全性由两部分组成：认证和加密

认证：使得只有运行的设备才能连接到无线网络

加密：确保数据的保密性和完整性，确保数据在传输中不会被篡改

| 安全策略 |   认证方式    |       加密方式       |          备注          |
| :------: | :-----------: | :------------------: | :--------------------: |
|   open   |     open      |         open         |  开放wifi,无任何加密   |
|   open   |      WEP      | 开放wifi,仅数据加密  |                        |
|   WEP    |      WEP      |         WEP          | 共享密钥认证，容易破解 |
|   WAP    |   8002.11x    |       TKIP/WEP       |   比较安全，用于企业   |
|   PSK    |   TKIP/WEP    |  比较安全，用于个人  |                        |
|   WAP2   |    802.11X    |    CCMP/TKIP/WEP     |  目前最安全，用于个人  |
|   PSK    | CCMP/TKIP/WEP | 目前最安全，用于个人 |                        |

​		连入超市等公共场合wifi的时候，不需要输入密码，但要通过网页输入手机号，使用验证码验证，也就是采用802.11x进行验证，然后通过服务器完成的验证

​		使用手机开启个人热点时候，可以选择open、wep、wap、wap2不同的安全等级

### 想要使用无线网卡，需要用到的命令

iw:可用于open、wep两种“认证/加密”，以及扫描WiFi热点,可取代 iwconfig

wpa_supplicant:可用于前面4中“认证/加密”，是一个连接、配置wifi的工具

hostapd:能够使得无线网卡切换为ap模式

dhcp:STA模式使wifi动态获取ip，ap模式分配ip

ifconfig:配置网卡信息

iwconfig:用于系统配置无线网络设备或显示无线网络设备信息

iwlist:对/proc/net/wireless文件进行分析，得出无线网卡相关信息

route:

### ifconfig

简单分析

```
[root@localhost ~]# ifconfig eth0
 
// UP：表示“接口已启用”。
// BROADCAST ：表示“主机支持广播”。
// RUNNING：表示“接口在工作中”。
// MULTICAST：表示“主机支持多播”。
// MTU:1500（最大传输单元）：1500字节
eth0: flags=4163<UP,BROADCAST,RUNNING,MULTICAST> mtu 1500
 
 
// inet ：网卡的IP地址。
// netmask ：网络掩码。
// broadcast ：广播地址。
inet 192.168.1.135 netmask 255.255.255.0 broadcast 192.168.1.255
 
 
// 网卡的IPv6地址
inet6 fe80::2aa:bbff:fecc:ddee prefixlen 64 scopeid 0x20<link>
 
// 连接类型：Ethernet (以太网) HWaddr (硬件mac地址)
// txqueuelen (网卡设置的传送队列长度)
ether 00:aa:bb:cc:dd:ee txqueuelen 1000 (Ethernet)
 
 
// RX packets 接收时，正确的数据包数。
// RX bytes 接收的数据量。
// RX errors 接收时，产生错误的数据包数。
// RX dropped 接收时，丢弃的数据包数。
// RX overruns 接收时，由于速度过快而丢失的数据包数。
// RX frame 接收时，发生frame错误而丢失的数据包数。
RX packets 2825 bytes 218511 (213.3 KiB)
RX errors 0 dropped 0 overruns 0 frame 0
 
 
 
// TX packets 发送时，正确的数据包数。
// TX bytes 发送的数据量。
// TX errors 发送时，产生错误的数据包数。
// TX dropped 发送时，丢弃的数据包数。
// TX overruns 发送时，由于速度过快而丢失的数据包数。
// TX carrier 发送时，发生carrier错误而丢失的数据包数。
// collisions 冲突信息包的数目。
TX packets 1077 bytes 145236 (141.8 KiB)
TX errors 0 dropped 0 overruns 0 carrier 0 collisions 0
```

简单用例

显示激活的网卡信息

```
ifconfig
```

显示所有网卡信息

```
ifconfig -a
```

启动/停止     有线/无线  网卡

```
ifconfig wlan0/eth0 up/down
```

配置ip、子网掩码 

```
//配置ip地址
ifconfig  eth0 192.168.1.100

//配置ip地址和子网掩码
ifconfig eth0  192.168.1.100 netmask 255.255.255.0
```

### iwconfig

用于系统配置无线网络设备或显示无线网络设备信息，iwconfig命令类似与于ifconfig命令，但它的配置对象是无线网卡

```
auto	自动模式 
essid	设置essid
nwid	设置网络id
freq	设置无线网络信道
chanel	设置无线网络信道
mode	设置无线网络设备的通信设备
```

### iwlist

简单用例

搜索当前无线网络

```
iwlist wlan0 scanning
```

显示频道信息 

```
iwlist wlan0 frequen
```

显示连接速度

```
iwlist wlan0 rate
```

显示热点信息

```
iwlist wlan0 ap
```

### iw

iw是一种新的基于 nl80211 的用于无线设备的ctl配置实用程序。它支持最近已添加到内核所有新的驱动程序 

简单用例 

列出wifi网卡的性能

```
iw list
```

扫描wifi热点

```
iw dev wlan0 scan
iw dev wlan0 scan | grep SSID
```

连接到开放ap

```
iw wlan0 connect hceng
```

查看连接状态

```
iw wlan0 link
```

断开wifi连接

```
iw wlan0 disconnect
```

### wpa_supplicant

wpa_supplicant 主要包含wpa_supplicant(命令行模式)和wpa_cli(交互模式)两个程序 

简单用例：连接开放网络

向/etc/wpa_supplicant.conf加入 ：

```
network={
ssid="hceng"
key_mgmt=NONE
}
```

初始化wpa_supplicant,执行 ：

```
wpa_supplicant -B -d -i wlan0 -c /etc/wpa_supplicant.conf
```

查看连接状态：

```
wpa_cli -i wlan0 status
```

断开连接

```
wpa_cli -i wlan0 disconnect
killall wpa_supplicant
```

重新连接

```
wpa_cli -i wlan0 reconnect
```

### dhclient

使用实例

自动获取分配IP，并设置 

```
dhclient wlan0
```

### ap模式产生热点

```

1）、ifconfig wlan0 up
2）、hostapd -B /etc/jffs2/hostapd.conf
3）、udhcpd /etc/jffs2/udhcpd.conf
4）、tcpsvd 0 21 ftpd -w /data &
```

