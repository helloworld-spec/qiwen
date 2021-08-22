#include <string.h>
#include "command.h"
#include "ak_common.h"


static char *help[] = {
	"rtc module demo",
	"usage:datedemo [day/time] [number]\n"
	"      datedemo [read]\n"
	"      example:datedemo day 2014-10-21\n"
	"      example:datedemo time 23:6:5\n"
	
};

static void cmd_rtc_get_time()
{
    struct ak_date systime ;
    ak_get_localdate(&systime);

    ak_print_normal("%04d-%02d-%02d %02d:%02d:%02d\n", systime.year, systime.month, 
		systime.day, systime.hour, systime.minute, systime.second);
}

static void cmd_rtc_set_time(struct ak_date systime)
{
  
	ak_set_localdate(&systime);

    mini_delay(20);

    cmd_rtc_get_time();
}

#if 0

static void rtc_cb(void)
{
    printf("alarm!\n");
    cmd_rtc_get_time();
}

void cmd_rtc_set_alarm()
{
    T_SYSTIME systime;

    rtc_set_callback(rtc_cb);
    
    systime = rtc_get_systime();

    systime.minute += 1;

    rtc_set_AlarmBySystime(&systime);

    printf("set alarm in %04d_%02d_%02d %02d:%02d:%02d\n", systime.year, systime.month, systime.day, 
        systime.hour, systime.minute, systime.second, systime.week);
}

#endif


static void cmd_rtc_test(int argc, char **args)
{
	int j = 0;
	char * command = (args[0]);
	if((strcmp(command,"day") != 0) && (strcmp(command,"time") != 0)\
		&& (strcmp(command,"read") != 0))
	{
		ak_print_error_ex("Input command error\n");
    	for(j = 0; j<2 ;j++)
    	{
    		ak_print_normal("%s\n",help[j]);
    	}
		return ;
	}

	if((strcmp(command,"day") == 0))
	{
		int i = 0;
		int j = 0;
		unsigned char month_std_day[13]  = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
        unsigned char month_leap_day[13] = {0, 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
		struct ak_date systime1,systime2;
		char * number = (args[1]);
		systime1.year = atoi(number);
		
		while(number[i] != '-')
		{
			i++;
		}
		i += 1;
		number += (i);
		
		systime1.month = atoi(number);

		i = 0;
		while(number[i] != '-')
		{
			i++;
		}
		i += 1;
		number += (i);
		systime1.day = atoi(number);

	    
    	if((systime1.year % 4 == 0 && systime1.year % 100 != 0) || (systime1.year % 400 == 0))
	    {
			
		    if ((systime1.year < BASE_YEAR) || (systime1.year > 2099) || (systime1.month < 1) || \
				(systime1.month > 12) || (systime1.day < 1) || \
				(systime1.day >( month_leap_day[(systime1.month)])))
		    {
		        ak_print_error_ex("Input command error\n");
		    	for(j = 0; j < 2 ;j++)
		    	{
		    		ak_print_normal("%s\n",help[j]);
		    	}
				return ;
		    }
		}
		else
		{
			if ((systime1.year < BASE_YEAR) || (systime1.year > 2099) || (systime1.month < 1) || \
				(systime1.month > 12) || (systime1.day < 1) || \
				(systime1.day > ( month_std_day[(systime1.month)])))
		    {
		        ak_print_error_ex("Input command error\n");
		    	for(j = 0; j < 2 ;j++)
		    	{
		    		ak_print_normal("%s\n",help[j]);
		    	}
				return ;
		    }
		}
		ak_get_localdate(&systime2);

		
		//ak_print_normal("ak_get_localdate 1 %04d-%02d-%02d %02d:%02d:%02d\n", systime2.year, systime2.month, 
				//systime2.day, systime2.hour, systime2.minute, systime2.second);
		
		
		systime1.hour = systime2.hour;
		systime1.minute = systime2.minute;
	    systime1.second = systime2.second;

		//ak_print_normal("set time to %04d-%02d-%02d %02d:%02d:%02d\n", systime1.year, systime1.month, 
				//systime1.day, systime1.hour, systime1.minute, systime1.second);
		

		cmd_rtc_set_time(systime1);
	}
	else
	{
		if((strcmp(command,"time") == 0))
		{
			int i = 0;
			int j = 0;
			struct ak_date systime1,systime2;
			char * number = (args[1]);
			systime1.hour = atoi(number);
			while(number[i] != ':')
			{
				i++;
			}
			i += 1;
			number += (i);
			
			systime1.minute = atoi(number);

			i = 0;
			while(number[i] != ':')
			{
				i++;
			}
			i += 1;
			number += (i);
			systime1.second = atoi(number);
			if((systime1.hour > 23) || (systime1.minute > 59) || (systime1.second > 59))
			{
				ak_print_error_ex("Input command error\n");
		    	for(j = 0; j<2 ;j++)
		    	{
		    		ak_print_normal("%s\n",help[j]);
		    	}
				return ;
		    }
			ak_get_localdate(&systime2);

			//ak_print_normal("ak_get_localdate %04d-%02d-%02d %02d:%02d:%02d\n", systime2.year, systime2.month, 
		//systime2.day, systime2.hour, systime2.minute, systime2.second);

			systime1.year = systime2.year;
			systime1.month = systime2.month;
		    systime1.day = systime2.day ;

			cmd_rtc_set_time(systime1);
		}
		else
		{
			cmd_rtc_get_time();
		}

	}

}


static int cmd_rtc_reg(void)
{
    cmd_register("datedemo", cmd_rtc_test, help);
    return 0;
}

cmd_module_init(cmd_rtc_reg)

