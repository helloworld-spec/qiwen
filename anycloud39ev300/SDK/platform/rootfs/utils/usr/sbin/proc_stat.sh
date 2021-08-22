#!/bin/sh

pid=$1
if [ "$pid" = "" ]
then 
        echo "usage: proc_stat.sh pid"
        exit 1        
fi 
        
b=1
stat=`cat /proc/$pid/stat`
while [ $b -le  48 ]
do
	case $b in
		1)
			xx1=`echo $stat |awk '{print $1}'`
			echo "pid		$xx1"
			;;
		2)
			xx2=`echo $stat |awk '{print $2}'`
			echo "tcomm		$xx2"
			;;
		3)
			xx3=`echo $stat |awk '{print $3}'`
			echo "state		$xx3"
			;;
		4)
			xx4=`echo $stat |awk '{print $4}'`
			echo "ppid		$xx4"
			;;
		5)
			xx5=`echo $stat |awk '{print $5}'`
			echo "pgrp		$xx5"
			;;
                6)
			xx6=`echo $stat |awk '{print $6}'`
			echo "sid		$xx6"
			;;
                7)
			xx7=`echo $stat |awk '{print $7}'`
			echo "tty_nr		$xx7"
			;;
                 8)
			xx8=`echo $stat |awk '{print $8}'`
			echo "tty_pgrp	$xx8"
			;;
             	9)
			xx9=`echo $stat |awk '{print $9}'`
			echo "flags		$xx9"
			;;
		10)
			xx10=`echo $stat |awk '{print $10}'`
			echo "min_fit		$xx10"
			;;
		11)
			xx11=`echo $stat |awk '{print $11}'`
			echo "cmin_flt	$xx11"
			;;	
	        12)
			xx12=`echo $stat |awk '{print $12}'`
			echo "maj_flt		$xx12"
			;;

	13)
			xx13=`echo $stat |awk '{print $13}'`
			echo "cmaj_flt	$xx13"
			;;

	14)
			xx14=`echo $stat |awk '{print $14}'`
			echo "utime		$xx14"
			;;
	15)
			xx15=`echo $stat |awk '{print $15}'`
			echo "stime		$xx15"
			;;
	16)
			xx16=`echo $stat |awk '{print $16}'`
			echo "cuttime		$xx16"
			;;

	17)
			xx17=`echo $stat |awk '{print $17}'`
			echo "cstime		$xx17"
			;;
	18)
			xx18=`echo $stat |awk '{print $18}'`
			echo "priority	$xx18"
			;;

	19)
			xx19=`echo $stat |awk '{print $19}'`
			echo "nice		$xx19"
			;;
	20)
			xx20=`echo $stat |awk '{print $20}'`
			echo "num_thread	$xx20"
			;;

	21)
			xx21=`echo $stat |awk '{print $21}'`
			echo "start_time	$xx21"
			;;
        22)
			xx22=`echo $stat |awk '{print $22}'`
			echo "vsize		$xx22"
			;;

	23)
			xx23=`echo $stat |awk '{print $23}'`
			echo "rss		$xx23"
			;;
	24)
			xx24=`echo $stat |awk '{print $24}'`
			echo "rsslim		$xx24"
			;;

	25)
			xx25=`echo $stat |awk '{print $25}'`
			echo "start_stack	$xx25"
			;;

	26)
			xx26=`echo $stat |awk '{print $26}'`
			echo "end_code	$xx26"
			;;
	27)
			xx27=`echo $stat |awk '{print $27}'`
			echo "start_stack	$xx27"
			;;
	28)
			xx28=`echo $stat |awk '{print $28}'`
			echo "esp		$xx28"
			;;


	29)
			xx29=`echo $stat |awk '{print $29}'`
			echo "eip		$xx29"
			;;
	30)
			xx30=`echo $stat |awk '{print $30}'`
			echo "pending		$xx30"
			;;
	31)
			xx31=`echo $stat |awk '{print $31}'`
			echo "blocked		$xx31"
			;;
	32)
			xx32=`echo $stat |awk '{print $32}'`
			echo "sigigm		$xx32"
			;;
		33)
			xx33=`echo $stat |awk '{print $33}'`
			echo "sigcatch	$xx33"
			;;

	34)
			xx34=`echo $stat |awk '{print $34}'`
			echo "wchan		$xx34"
			;;
	35)
			xx35=`echo $stat |awk '{print $35}'`
			echo "0		$xx35"
			;;
	36)
			xx36=`echo $stat |awk '{print $36}'`
			echo "0		$xx36"
			;;
	37)
			xx37=`echo $stat |awk '{print $37}'`
			echo "exit_signal	$xx37"
			;;
	38)
			xx38=`echo $stat |awk '{print $38}'`
			echo "task_cpu	$xx38"
			;;
	39)
			xx39=`echo $stat |awk '{print $39}'`
			echo "rt_priority	$xx39"
			;;
	40)
			xx40=`echo $stat |awk '{print $40}'`
			echo "policy		$xx40"
			;;
	41)
			xx41=`echo $stat |awk '{print $41}'`
			echo "bikio_ticks	$xx41"
			;;

	42)
			xx42=`echo $stat |awk '{print $42}'`
			echo "gtime		$xx42"
			;;

	43)
			xx43=`echo $stat |awk '{print $43}'`
			echo "cgtime		$xx43"
			;;
	44)
			xx44=`echo $stat |awk '{print $44}'`
			echo "start_date	$xx44"
			;;
	45)
			xx45=`echo $stat |awk '{print $45}'`
			echo "end_data	$xx45"
			;;
		46)
			xx46=`echo $stat |awk '{print $46}'`
			echo "start_brk	$xx46"
			;;

		
	47)
			xx47=`echo $stat |awk '{print $48}'`
			echo "		$xx47"
			;;
	48)
			xx48=`echo $stat |awk '{print $48}'`
			echo "		$xx48"
	               ;;

		*)
		echo "no more"
		;;
         	esac
         	b=$(($b+1))
done




