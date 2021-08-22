#!/bin/sh
# File:				recover_cfg.sh	
# Provides:         
# Description:      recover system config
# Author:			aj

#recover factory config ini
cp /usr/local/factory_cfg.ini /etc/jffs2/anyka_cfg.ini

#recover isp config ini
rm -rf /etc/jffs2/isp*.conf

#if no uninstall the wifi driver.May be a problem that wifi cannot be started.
wifi_driver.sh uninstall