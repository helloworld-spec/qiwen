#include "command.h"
#include "string.h"
#include "ak_common.h"
#include "kernel.h"

#include "ak_drv_detector.h"


static char *help_mount[]={
	"mount file system",
	"usage:mount <auto>\n"
};

static void cmd_sd_mount(int argc, char **args)
{
	int tmp = 0;

	int *fd;
	
	if(argc > 1 )
	{
		ak_print_error("%s",help_mount[1]);
		return;
	}
	
	if(1 ==  argc)
	{
		if(strcmp(args[0], "auto") == 0)
		{
			tmp = 1;
		}
		else
		{
			ak_print_error("%s",help_mount[1]);
			return;
		}
	}

	if(tmp)
	{
		fd = ak_drv_detector_open(SD_DETECTOR); //open SD detect
		if(NULL == fd)
		{
			ak_print_error("open detector fail\n");
		}
	}
	else
	{
		if (0 ==ak_mount_fs(DEV_MMCBLOCK, 0, ""))
			ak_print_normal("mount sd ok!\n");
		else
			ak_print_error("mount sd fail!\n");
	}
	
		
}

static void cmd_sd_unmount(int argc, char **args)
{
	if (0 ==ak_unmount_fs(DEV_MMCBLOCK, 0, ""))
		ak_print_normal("unmount sd ok!\n");
	else
		ak_print_error("unmount sd fail!\n");
	
}

static void cmd_file_demo(int argc, char **args)
{
	char wData[100];
	char rData[100];
	int ret;
	int i;
	long pos;

    if (0 !=ak_mount_fs(DEV_MMCBLOCK, 0, ""))
    {
		ak_print_error("sd card mount failed!\n");
        return ;
    }

	//open  a file for write	
	FILE * handle = fopen( "a:/test.txt", "w" );
	if (handle ==NULL)
	{
		ak_print_error("open file failed!\n");
		return ;
	}
	else
		ak_print_normal("open file success!\n");

	strcpy(wData, "hello");
	ret = fwrite(wData, strlen(wData),1,handle );
	ak_print_normal("write %d size data!\n", ret);

	fputc(' ', handle);
	ak_fputs("tom",handle);
	fclose(handle);
	
	//open  a file for read	
	handle = fopen( "a:/test.txt", "r" );
	if (handle ==NULL)
	{
		ak_print_error("open file failed!\n");
		return ;
	}
	else
		ak_print_normal("open file success!\n");
	

	//read hello
	memset(rData, 0 ,sizeof(rData));
	ret = fread(rData, strlen(wData),1,handle );
	ak_print_normal("read data=%s\n", rData);


	pos = ftell(handle);
	ak_print_normal("cur file pos=%d\n", pos);
	//read tom
	fseek(handle, 1, SEEK_CUR);//skip ' ' 

	while(1)
	{
		if (!feof(handle))
		{
			ak_print_normal("%c", fgetc(handle));
		}else
			break;
	}
	ak_print_normal("\n");

	fclose(handle);

	//file speed test
	//open  a file for write	
    ak_print_normal("\nfile write speed test:\n");
	handle = fopen( "a:/test.txt", "w" );
	if (handle ==NULL)
	{
		ak_print_error("open file failed!\n");
		return ;
	}
	else
		ak_print_normal("open file success!\n");

    int num= 50 * 1024  /64;
    int t1 = get_tick_count();
    for(i=0 ;i < num;i++)
    {
	    fwrite(wData, 64*1024,1,handle );
	    
    }
    fclose(handle);

    int t=get_tick_count() - t1;
    
    ak_print_normal("write time=%d(ms),speed=%d(byte/s)\n\n", t,num * 64* 1024/(t/1000));
    ak_unmount_fs(DEV_MMCBLOCK, 0, "");
    return ;
	
    ak_unmount_fs(DEV_MMCBLOCK, 0, "");


}

static char *help_unmount[]={
	"unmount file system",
	"usage:unmount \n"
};
static char *help_filerw[]={
	"file read/write demo",
	""
};

static int cmd_file_reg(void)
{
    cmd_register("mount", cmd_sd_mount, help_mount);
    cmd_register("unmount", cmd_sd_unmount, help_unmount);
    cmd_register("filerw", cmd_file_demo, help_filerw);
    return 0;
}

cmd_module_init(cmd_file_reg)

