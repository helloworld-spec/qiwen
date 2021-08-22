# !/bin/bash

elf_file=$1

if [ -z $1 ]
then
	echo -e "\033[1;31mformat: ./do_posix_check.sh file.elf\033[m"
else
    echo "generating objdump and symbol files...."
    arm-anykav200-linux-uclibcgnueabi-objdump  -h -D -Mreg-names-raw --show-raw-insn $elf_file > sky39ev200.txt
    arm-anykav200-linux-uclibcgnueabi-nm -n $elf_file > symbols
    
    echo "check invalid posix call ...."
    echo "result:"
    ./find_used_symbols.sh ./sky39ev200.txt ./symbols ./filter
fi
