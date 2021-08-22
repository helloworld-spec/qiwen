#!/bin/sh
insmod /root/mlan.ko
insmod /root/sd87xx.ko drv_mode=2
iwpriv uap0 apcfg "ASCII_CMD=AP_CFG,SSID=TEST_uAP,SEC=WPA2-PSK,KEY=123456789"
ifconfig uap0 192.168.0.1 netmask 255.255.255.0 up
udhcpd /etc/udhcpd.conf
iwpriv uap0 start
iwpriv uap0  bssstart
/usr/bin/anyka_ipc &
