

cd /tmp
echo "cd /tmp"
./test_ptz_cmd 6 /var/stat &
echo "./test_ptz_cmd 6 /var/stat &"
./test_wifi 3 /var/stat &
echo "./test_wifi 3 /var/stat &"
./test_mmc /var/stat &
echo "./test_mmc /var/stat &"
