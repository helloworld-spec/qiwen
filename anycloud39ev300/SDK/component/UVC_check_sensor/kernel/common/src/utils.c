//#include "anyka_types.h"
#include "command.h"
#include "string.h"
#include "libc_mem.h"

void cmd_utils_loglevel();
void cmd_utils_free();
void cmd_utils_idle();
void cmd_utils_ps();
void cmd_utils_regread();
void cmd_utils_regwrite();
void cmd_utils_irq();


/* set log level */
void cmd_utils_loglevel(int argc, char **args) 
{
    int level;

	if (argc != 1)
	{
		printk("usage: loglevel [0-5]\n");
		return;
	}
		
    level = atoi(args[0]);
	printk("set log level to %d\n", level);
    
    //TODO
    //ak_print_set_level(level);
}

/* show free memory */
void cmd_utils_free(int argc, char **args) 
{
    printk("mem total: %d kB\n", Fwl_GetTotalRamSize()/1024);
    printk("mem free : %d kB\n", Fwl_GetRemainRamSize()/1024);
}

/* show idle cpu */
void cmd_utils_idle(int argc, char **args) 
{	
    printk("cpu idle: %d%%\r\n", idle_get_cpu_idle());
}

/* show background tasks */
void cmd_utils_jobs(int argc, char **args) 
{
    cmd_show_jobs();
}

/* kill background tasks */
void cmd_utils_kill(int argc, char **args) 
{
    int id;

    if(argc != 1)
    {
        printk("usage: kill [job_id]\n");
        return;
    }
    
    id = atoi(args[0]);
    cmd_kill_job(id);
}

/* show runing tasks */
void cmd_utils_ps(int argc, char **args) 
{
	int prio_format=0, time_format=1;
	
    //prio_format: 0, normal prio; 1, ak_thread prio
    //time_format: 0, ms unit; 1, second unit
	if (argc >= 1)
	{
		prio_format = atoi(args[0]);
	}
	if (argc >= 2)
	{
		time_format = atoi(args[1]);
	}

	AK_List_Task(prio_format, time_format);
}

/* read register */
void cmd_utils_regread(int argc, char **args)
{
    unsigned long regaddr;

	if (argc != 1)
	{
		printk("format: regread [addr]\n");
		return;
	}

	if (*(args[0]+1) != 'x')
	{
		printk("data must be hex format\n");
		return;
	}
	sscanf(args[0], "0x%x", &regaddr);
	
    printk("reg(0x%x)=0x%x\r\n", regaddr, *(volatile unsigned long *)regaddr);
}

/* write register */
void cmd_utils_regwrite(int argc, char **args)
{
    unsigned long regaddr;
    unsigned long regvalue;

	if (argc != 2)
	{
		printk("format: regwrite [addr] [value]\n");
		return;
	}

	if (*(args[0]+1) != 'x' || *(args[1]+1) != 'x')
	{
		printk("data must be hex format\n");
		return;
	}

	sscanf(args[0], "0x%x", &regaddr);
	sscanf(args[1], "0x%x", &regvalue);
    
    *(volatile unsigned long *)regaddr = regvalue;
    
    printk("reg(0x%x)=0x%x\r\n", regaddr, *(volatile unsigned long *)regaddr);

}

void cmd_utils_irq(int argc, char **args)
{
    irq_show();
}

void cmd_utils_uptime(int argc, char **args)
{
	unsigned long ts;
	int day, hour, min, sec;

	ts = get_tick_count();

	sec = ts / 1000;
	
	day = sec / (60*60*24);
	if (day > 0)
		sec -= day*60*60*24;

	hour = sec / (60*60);
	if (hour > 0)
		sec -= hour*60*60;
	
	min = sec / 60;
	if (min > 0)
	{
		sec -= min*60;
	}

	printk("up ");
	if (day)
		printk("%d day ", day);
	if (hour)
		printk("%d hour ", hour);
	if (min)
		printk("%d min ", min);
	if (sec)
		printk("%d sec ", sec);
	printk("total in %lu ms\r\n", ts);	
}

static char *help_free[]={ 
	"show free mem size",
	""
};
static char *help_idle[]={
	"show idle cpu",
	""
};
static char *help_regread[]={
	"read register",
	""
};
static char *help_regwrite[]={
	"write register",
	""
};
static char *help_uptime[]={
	"show running time",
	""
};
static char *help_ps[]={
	"list running tasks",
	""
};
static char *help_irq[]={
	"list irq status",
	""
};
static char *help_loglevel[]={
	"set log level",
	""
};
static char *help_jobs[]={
	"show jobs",
	""
};
static char *help_kill[]={
	"kill job",
	""
};

int cmd_utils_reg(void)
{
    cmd_register("free", cmd_utils_free, help_free);
    cmd_register("idle", cmd_utils_idle, help_idle);
    cmd_register("regread", cmd_utils_regread, help_regread);
    cmd_register("regwrite", cmd_utils_regwrite, help_regwrite);
    cmd_register("up", cmd_utils_uptime, help_uptime);
    cmd_register("ps", cmd_utils_ps, help_ps);
    cmd_register("irq", cmd_utils_irq, help_irq);
    cmd_register("loglevel", cmd_utils_loglevel, help_loglevel);
    cmd_register("jobs", cmd_utils_jobs, help_jobs);
    cmd_register("kill", cmd_utils_kill, help_kill);
	
    return 0;
}

//cmd_module_init(cmd_utils_reg)
