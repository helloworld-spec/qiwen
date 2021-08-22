#! /bin/sh

sed -e '1,/^Linker script/d' -e '/^LOAD/,$d' ../../kernel/maps.txt > ./maps_tmp1.txt
awk -f ./preproc.awk ./maps_tmp1.txt > ./maps_tmp2.txt
awk -f ./count_all.awk ./maps_tmp2.txt |sort -n -r -k 5 > ./maps_tmp3.txt

echo "MODULE_NAME                                 TEXT        RODATA      DATA        TOTAL\n" > maps_size.txt
echo "=======================================================================================\n" >> maps_size.txt
cat maps_tmp3.txt >> maps_size.txt

rm maps_tmp1.txt maps_tmp2.txt maps_tmp3.txt

