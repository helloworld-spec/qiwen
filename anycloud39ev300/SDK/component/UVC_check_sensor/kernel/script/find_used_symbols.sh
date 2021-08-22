# !/bin/bash
# 根据符号文件在汇编文件中查找符号使用情况
#
# 使用：./find_used_symbols.sh 反汇编文件 符号文件 [排除符号文件]
# 命令参数1 ：反汇编文件
# 命令参数2 ：符号文件
# 命令参数3 ：排除符号文件
assembler_file=$1
symbols_file=$2
except_file=$3


# tmp_file 需要查找的符号，存放的临时文件
tmp_file=tmpfile
tmp_file1=tmpfile1
tmp_file2=tmpfile2
# only_define=only_define
output_file=output


# start_symbol 从start_symbol符号开始查找
start_symbol=__GI_modf

## 从符号文件中过滤出需要查找的符号，输出到tmp_file
function find_used_symbol {
	awk -v start_symbol=$start_symbol -v awk_is_start=0 -v awk_output_file=$tmp_file '{

		if( awk_is_start == 1)
		{
			# 从start_symbol 符号开始连续查找
			if ($2 == "r" || $2 == "R")
			{
				#如果找到符号类型为r,停止查找
				awk_is_start = 2
			}
			else if( awk_is_start == 2)
			{
				# do nothing
			}
			else
			{
				 print $0 >> awk_output_file
			}
		}
		else if( $3 == start_symbol)
		{
			# 查找开始符号
			awk_is_start = 1;
			print $0 > awk_output_file
		}
	}' $symbols_file
}
# 处理汇编文件，查找汇编文件时只查询开始符号定义以前的内容
function process_assembler_file {
	# 找到开始符号的地址
	start_address=`grep $start_symbol $tmp_file | awk '{ print $1}'`

	# 根据开始符号地址找在汇编文件中找到开始行
	start_line=`grep $start_address $assembler_file | awk -v start_address=$start_address '{ 
			if ($1 == start_address) 
			{
				print $0
			}
		
	
	}'`

	# 删除开始行以后的文字
	sed '/'"$start_line"'/,$d' $assembler_file >> $tmp_file2

	assembler_file=$tmp_file2
#	echo $start_line
}

# 得到需要查找符号，删除except_file中包含的符号
function process_tmpfile {

	if [ -z $except_file  ]
	then
		echo 没有用到排除文件 
		return
	fi

	for symbol in `cat $except_file`
		do
#			echo $symbol
			sed '/'"$symbol"'/d ' $tmp_file > $tmp_file1
#			echo $tmp_file1

		    var_tmp=$tmp_file
	   		tmp_file=$tmp_file1
	 		tmp_file1=$var_tmp		
		done

}

address_num=
address_symbol=
is_one=1
function symbol_used_info {

	# clean output_file
	echo "" > $output_file
#	echo "" > "only_define"
	# 设置段落分隔符，否则空格、制表、换行都认为是。
	IFS=$'\n'

	for symbol in `cat $tmp_file`
	do
		# 得到一行数据,解析出第一个编号
		# data : 8011dd94 T _etext
		address_num=`echo $symbol| awk '{print $1}'`
		address_symbol=`echo $symbol| awk '{print $3}'`
#		echo $address_num

		# 根据编号在反汇编文件中查找使用情况
		result=`grep $address_num $assembler_file | awk -v i=0 -v awk_num=$address_num '{
		
		# length用来过滤信息描述行提到地址
		if(i==0 && length($1) == 9)
			{
				# 只要一行中包含此符号地址就代表使用，
				# if($4 == awk_num)
				if( index($0, awk_num) > 0 )
				{
					print 1
					i++
#					print $0 >> output_file
				}
				else if($1 == awk_num)
				{
#					print $1 >> only_define
#					print $0 >> only_define
					# 如果是定义，停止寻找
					i++
					print 0
				}
			}
			
		}'`
#		echo "=$result="
		if [ -n "$result" ]
		then
			if [ $result = $is_one ]
			then
				#echo $address_symbol
				echo -e "\033[1;31m$address_symbol\033[m"
				echo $address_symbol >> $output_file
			fi
		fi
	done
}

# 验证参数 
if [ $# -lt 2 ]
then
#	echo $#
	echo 使用：./find_used_symbols.sh 反汇编文件 符号文件 [排除符号文件]
else
	find_used_symbol
	process_assembler_file
	process_tmpfile
	symbol_used_info

	echo 匹配的符号已经打印在 $output_file 文件中
	rm -f $tmp_file $tmp_file1 $tmp_file2
fi
