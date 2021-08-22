#/bin/bash

CUR_DIR=`pwd`
echo "${CUR_DIR}"

cd ${CUR_DIR}/../..
SSV_DIR=`pwd`
echo "${SSV_DIR}"

echo "================================================"
echo "==== Step 1: build driver & enable debug HW ===="
echo "================================================"
cd ${SSV_DIR}
cp config_common.mak config_common.mak.old
echo "ccflags-y += -DCONFIG_SSV6XXX_HW_DEBUG" >> config_common.mak
sleep 1
make; make install
sleep 10

echo "================================================"
echo "==== Step 2: build verification tools       ===="
echo "================================================"
cd ${CUR_DIR}
make
sleep 10

echo "================================================"
echo "==== Step 3: load ssv driver                ===="
echo "================================================"
cd ${SSV_DIR}
./load.sh
sleep 10

echo "================================================"
echo "==== Step 4: load verification module       ===="
echo "================================================"
cd ${SSV_DIR}
insmod ./umac/ssv6xxx_umac_core.ko attach_device_name="SSV6006" 
sleep 10

echo "================================================"
echo "==== Step 5: Verification Server Start...   ===="
echo "================================================"
cd ${CUR_DIR}
./icomm_osif_hal_main &
sleep 3


echo "================================================"
echo "==== Step 6: Verification Test Start...     ===="
echo "================================================"

echo "================================================"
echo "==== Step 6.1: DMA Test ...                 ===="
echo "================================================"
./icomm_osif_hal_util sdio_dma 10000 1000
echo "================================================"
echo "==== Step 6.2: HCI Loopback Test...         ===="
echo "================================================"
./icomm_osif_hal_util hci_lpbk normal 1400 1000
echo "================================================"
echo "==== Step 6.3: Pattern Test...              ===="
echo "================================================"
./icomm_osif_hal_util pattern_test pattern/HCI_RX.bin
./icomm_osif_hal_util pattern_test pattern/HCI_TX_8WSID.bin
./icomm_osif_hal_util pattern_test pattern/HCI_TX_M0_80211.bin
./icomm_osif_hal_util pattern_test pattern/HCI_TX_M0_8023.bin
./icomm_osif_hal_util pattern_test pattern/HCI_TX_M1_80211.bin
./icomm_osif_hal_util pattern_test pattern/HCI_TX_M1_8023.bin
./icomm_osif_hal_util pattern_test pattern/HWSEC_V2_AMPDU_V2.bin
./icomm_osif_hal_util pattern_test pattern/HWSEC_V2_AMPDU_V2_FIX_PN.bin
./icomm_osif_hal_util pattern_test pattern/HWSEC_V2_CCMP_FIX_PN.bin
./icomm_osif_hal_util pattern_test pattern/HWSEC_V2_GROUP.bin
./icomm_osif_hal_util pattern_test pattern/HWSEC_V2_TEST.bin
./icomm_osif_hal_util pattern_test pattern/MRX_ACK_GEN.bin
./icomm_osif_hal_util pattern_test pattern/MRX_AMPDU_RX.bin
./icomm_osif_hal_util pattern_test pattern/MRX_DECISION_FLOW.bin
./icomm_osif_hal_util pattern_test pattern/MRX_FILTER_TABLE.bin
./icomm_osif_hal_util pattern_test pattern/MRX_MCAST_TABLE.bin
./icomm_osif_hal_util pattern_test pattern/MRX_MIB_COUNTER.bin
./icomm_osif_hal_util pattern_test pattern/MRX_TRAP_REASON.bin
./icomm_osif_hal_util pattern_test pattern/MRX_BA_SCORE_BOARD.bin
./icomm_osif_hal_util pattern_test pattern/MRX_TIMESTAMP.bin
./icomm_osif_hal_util pattern_test pattern/MTX_AMPDU_RETRY_CNT.bin
./icomm_osif_hal_util pattern_test pattern/MTX_AMPDU_V2.bin
./icomm_osif_hal_util pattern_test pattern/MTX_AMPDU_V3.bin
./icomm_osif_hal_util pattern_test pattern/MTX_RATE_MPDU.bin
./icomm_osif_hal_util pattern_test pattern/MTX_RETRY_CNT.bin
./icomm_osif_hal_util pattern_test pattern/MTX_RETRY_CNT_CTS2SELF.bin
#./icomm_osif_hal_util pattern_test pattern/MTX_TXQ_ON_OFF.bin
# for mp
./icomm_osif_hal_util pattern_test pattern/MTX_TXQ_ON_OFF_MP.bin

# remove verification tools
cd ${CUR_DIR}
killall icomm_osif_hal_main
make clean

# remove ssv driver
cd ${SSV_DIR}
rmmod ssv6xxx_umac_core
sleep 3
./unload.sh
make clean
mv config_common.mak.old config_common.mak
