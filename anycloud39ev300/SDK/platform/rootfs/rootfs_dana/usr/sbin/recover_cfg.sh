#!/bin/sh
# File:				update.sh	
# Provides:         
# Description:      recover system configuration
# Author:			aj

play_recover_tip()
{
	ccli misc --tips "/usr/share/anyka_recover_device.mp3"
	sleep 3
}

#recover factory config ini
cp /usr/local/factory_cfg.ini /etc/jffs2/anyka_cfg.ini
sync

#recover isp config ini
rm -rf /etc/jffs2/isp*.conf

#if not uninstall the wifi driver.Would start wifi failed.
wifi_driver.sh uninstall

#after all done play tips
play_recover_tip
