#include "libc_posix_fs.h"
#include "file.h"
#include "print.h"
#include "fs.h"
#include "libc_mem.h"

////////////////////libc 接口 begin ////////////

FILE *ak_fopen( const char *fname, const char *mode )
{
	//模式进行映射
	unsigned long ak_mode;
	unsigned long ak_handle;
	int create_flag=0;

	if (0 == strcmp(mode , "r") || 0 == strcmp(mode,"rb"))
		ak_mode = FILE_MODE_READ;
	else if (0 == strcmp(mode , "w") || 0 == strcmp(mode,"wb"))
	{
		ak_mode = FILE_MODE_OVERLAY;
		create_flag = 1;
	}
	else if (0 == strcmp(mode , "a") || 0 == strcmp(mode,"ab"))
	{
		ak_mode = FILE_MODE_APPEND;
	}
	else if (0 == strcmp(mode , "r+") || 0 == strcmp(mode,"rb+"))
		ak_mode = FILE_MODE_OVERLAY  ;
	else if (0 == strcmp(mode , "w+") || 0 == strcmp(mode,"wb+"))
	{
		ak_mode =  FILE_MODE_OVERLAY ;
		create_flag = 1;
	}
	else if (0 == strcmp(mode , "a+") || 0 == strcmp(mode,"ab+"))
	{
		ak_mode = FILE_MODE_APPEND ;
	}else
	    return NULL;

//有文件创建标识，则如果文件不存在就需要创建文件
	if (create_flag)
	{
		ak_handle = File_OpenAsc( 0 , fname, FILE_MODE_CREATE);
		File_Close(ak_handle);

		/*
		ak_handle = File_OpenAsc( 0 , fname, FILE_MODE_READ);
		if (!File_Exist(ak_handle))
		{
			File_Close(ak_handle);
			ak_handle = File_OpenAsc( 0 , fname, FILE_MODE_CREATE);
			File_Close(ak_handle);
			
		}else
		{	File_Truncate(ak_handle , 0, 0);
			File_Close(ak_handle);
		}
		*/
			
	}
//这里再根据需要打开文件
    ak_mode |= FILE_MODE_ASYN ;
        
	ak_handle = File_OpenAsc( 0 , fname, ak_mode);

    //judge if file exist
    if (0 !=ak_handle)
    {
        if (!File_Exist(ak_handle)) 
        {
            File_Close(ak_handle);
            ak_handle = 0;
        }
    }
	return (FILE *)ak_handle;		
	
}

int ak_fclose( FILE *stream )
{
	File_Close((unsigned long) stream);
	return 0;
}

size_t ak_fread( void *buffer, size_t size, size_t num, FILE *stream )
{
	return File_Read((unsigned long)stream, buffer, size * num);
}
/*
int ak_fscanf( FILE *stream, const char *format, ... )
{
	return 0;
}
*/
int ak_fgetc( FILE *stream )
{
	char ch;
	int ret; 

	ret = File_Read((unsigned long)stream ,  &ch, 1);
	if (ret ==0)
		return EOF;
	else
		return ch;
	
}
int ak_getc( FILE *stream )
{
	return ak_fgetc(stream);
}

/*
char *ak_fgets( char *str, int num, FILE *stream )
{
	return NULL;
}
*/


int ak_fwrite( const void *buffer, size_t size, size_t count, FILE *stream )
{

    int used,tol;
    int unit = 64 *1024;
    int cur_size = size * count;
    int good_size;

    
    if (cur_size % unit ==0)
        good_size = cur_size ;
    else
        good_size = (cur_size /unit +1)* unit;

    FS_GetAsynBufInfo(&used,&tol);
    if (good_size > tol)
        good_size =tol;
        
    if(used+cur_size > tol)
    {
//        printk("buffer full\n");
        change_fs_thread_priority(127);
        while (used+good_size > tol)
        {
            AK_Sleep(2); //waiting async flush thread to run
            FS_GetAsynBufInfo(&used,&tol);
        }
        change_fs_thread_priority(150);
//        printk("buffer sub,used=%d\n",used);
        
    }
    
	return File_Write((unsigned long)stream, (void *)buffer, size * count);		
}
/*
int ak_fprintf( FILE *stream, const char *format, ... )
{
	return 0;
}
*/
int ak_fputc( int ch, FILE *stream )
{
	unsigned long ret;
	char data = (char) ch;
	
	ret = File_Write((unsigned long)stream , &data, 1);
	if (0 == ret)
		return EOF;
	else
		return ch;
		
}
int ak_putc( int ch, FILE *stream )
{
	return ak_fputc(ch, stream);
}
int ak_fputs( const char *str, FILE *stream )
{
	unsigned long ret;
	
	ret = File_Write((unsigned long)stream , (void *)str, strlen(str));
	if (0 == ret)
		return EOF;
	else
		return ret;
	
}


int ak_fseek( FILE *stream, long offset, int origin )
{
	unsigned short ak_origin;	
	unsigned long ret;
	
	switch (origin)
	{
		case SEEK_SET:
			ak_origin = FILE_SEEK_SET ;
			break;
		case SEEK_CUR:
			ak_origin = FILE_SEEK_CUR ;
			break;
		case SEEK_END:
			ak_origin = FILE_SEEK_END ;
			break;
		default:
			return -1;
			break;
	}
	ret = File_SetFilePtr((unsigned long)stream, offset, ak_origin);
	if (ret == (unsigned long)(-1))
		return -1;
	else
		return 0;
	
	
}
/*
int ak_fgetpos( FILE *stream, fpos_t *position )
{
	return 0;	
}
int ak_fsetpos( FILE *stream, const fpos_t *position )
{
	return 0;
}
*/

//暂只考虑小于4G -1 长度的文件
int ak_feof( FILE *stream )
{
	unsigned long len;
	unsigned long pos;
	unsigned long tmp;
	

	len = File_GetLength((unsigned long)stream , &tmp);
	//长度为０，可能是出错，也可能是空文件，都认为文件结束。
	if (len ==0)
		return 1; 

	pos = File_GetFilePtr((unsigned long)stream , &tmp);
	//如果获取文件当前位置出错，则返回4G-1, 此时认为文件结束
	if (pos ==(unsigned long)(-1) )
		return 1;
		
	if (len == pos)
		return 1;

	return 0;
	
	
}


int ak_fflush( FILE *stream )
{
	File_Flush((unsigned long)stream);
	return 0;
}
//此函数的取值范围不能超过2G
long ak_ftell( FILE *stream )
{
	unsigned long high;
	
	unsigned long ret = File_GetFilePtr((unsigned long)stream, &high);

	return (signed long)ret;

}

int ak_remove( const char *fname )
{
	if(File_DelAsc((unsigned char *)fname))
		return 0;
	else
		return -1;
}

int ak_rename( const char *oldfname, const char *newfname )
{
 	unsigned long handle_src , handle_dst;
 	int ret=0;

	handle_src = File_OpenAsc( 0 , (unsigned char *)oldfname, FILE_MODE_READ);
	if (0 == handle_src)	
		return -1;


	handle_dst = File_OpenAsc( 0 , (unsigned char *)newfname, FILE_MODE_READ);
	if (0 == handle_dst)	
	{
		File_Close(handle_src);
		return -1;
	}
		
	if (!File_RenameTo(handle_src,handle_dst))
	    ret = -1;

	File_Close(handle_src);
	File_Close(handle_dst);
    
	return ret;

}


////////////////////libc 接口 end ////////////


////////////////////////posix begin////////////////////////

int ak_mkdir(const char *path, mode_t mode)
{
    if (File_MkdirsAsc(path))
        return 0;
    else
        return -1;
}
          
           
int ak_statfs(const char *path, struct statfs *buf)
{
    struct statfs fs_info;
    DRIVER_INFO drv_info;
    unsigned long free_high=0, free_low=0;

    if (NULL == buf)
        return -1;
        
    memset(&fs_info, 0 , sizeof(fs_info));

    if (!FS_GetDriver(&drv_info, 0)) //first disk in t  card
    {
        return -1;
    }

    fs_info.f_bsize =  drv_info.nBlkSize ;
    fs_info.f_blocks = drv_info.nBlkCnt;
    
    free_low = FS_GetDriverFreeSize(0 , &free_high);
    //get block amount by size 
    //blksize = 512 * n , free_high << 32 is too big, so need /512
    fs_info.f_bfree = free_low /fs_info.f_bsize + (free_high <<(32-9)) / (fs_info.f_bsize >>9) ; 
    fs_info.f_bavail = fs_info.f_bfree;

    *buf = fs_info;
    
    return 0;
}


int ak_stat( const char *path, struct stat *buffer )
{
    struct stat fs_stat;
    unsigned long high;
    DRIVER_INFO drv_info;
    unsigned long ak_handle ;

    if (NULL == buffer)
        return -1;
        
    if (!FS_GetDriver(&drv_info, 0)) //first disk in t  card
    {
        return -1;
    }
    
    ak_handle = File_OpenAsc( 0 , path, FILE_MODE_READ);
    if (ak_handle ==0)
        return -1;

    //文件长度信息暂时<=4G-1
    memset(&fs_stat, 0, sizeof(fs_stat));        
    fs_stat.st_size = File_GetLength(ak_handle,& high); 
    fs_stat.st_blksize = drv_info.nBlkSize ;
    fs_stat.st_blocks = File_GetOccupiedSpace(ak_handle, &high) / drv_info.nBlkSize;

    //文件时间信息
    File_Close(ak_handle);

    *buffer = fs_stat;
                
    return 0;
}


static int g_find_last=0;
static int g_find_open=0;

DIR * ak_opendir(const char *name)
{
    unsigned long ak_findhandle;
    T_FINDBUFCTRL   find_ctrl;
    short           pattern[4]={'*','.','*',0};
    short * file_name;
    int name_len,i;
    

    if (g_find_open)
    {
        printk("error!ak_opendir have done!you must do ak_closedir. \n");
        return NULL;
        
    }else
    {
        g_find_open =1;
        g_find_last =0;       
    }
    
    find_ctrl.NodeCnt = 1;
    find_ctrl.pattern = pattern;
    find_ctrl.type = FILTER_NOTITERATE;
    find_ctrl.patternLen = 3;

    name_len = strlen(name);
    file_name = malloc(2*(name_len+1));
    if (file_name ==NULL)
    {
        
        g_find_open =0;
        return NULL;
    }
        
    //transfer ascii to unicode
    for(i=0 ;i< name_len+1;i++)
    {
        file_name[i] = name[i];
    }
    

    ak_findhandle = File_FindFirst(file_name, &find_ctrl);
    free(file_name);
    if (0 == ak_findhandle)
    {
        printk("findhandle is null\n");
        g_find_open =0;
        return NULL;
    }
    else
        return (DIR *)ak_findhandle;

        
}

struct dirent * ak_readdir(DIR *dirp)
{
    static struct dirent entry ;
    struct dirent *result;

    if (0 == ak_readdir_r(dirp,&entry,&result))
    {
        if (NULL == result)
            return NULL;
        else            
            return &entry;
    }
    else
        return NULL;
}


int ak_readdir_r(DIR *dirp, struct dirent *entry, struct dirent **result)
{
    unsigned long ak_filehandle;
    struct dirent  dir;
    T_PFILEINFO info ;
    int i , len;
    unsigned long   file_cnt,fold_cnt;  
    
    info = File_FindInfo((unsigned long)dirp , 0, &file_cnt, &fold_cnt);
    if (NULL == info)
    {
        printk("have no record\n");
        *result =NULL;
        return 0;
    }
    
//    if (File_Find_IsDirsEnd(dirp))
    if (g_find_last)
    {
        printk("last record\n");
        *result =NULL;
        return 0;
        
    }
    
    memset(&dir, 0 ,sizeof(dir));
    //filename , now only support ascii
    len = info->NameLen;
    if (len > 255)
        len = 255;
        
    for(i=0 ;i< len;i++)
    {
        dir.d_name[i] = (char)info->LongName[i];
    }
    dir.d_name[len]=0;

    //filetype
    if (info->attr & 0x10 )// is  a  directory
        dir.d_type = DT_DIR;        
    else
        dir.d_type = DT_REG;

    *entry = dir;
    *result =entry;

    //move to next
    if (0 == File_FindNext((unsigned long)dirp,1))
        g_find_last = 1;

    return 0;
        
}

int ak_closedir(DIR *dirp)
{
    g_find_open =0 ;
    File_FindCloseWithHandle((unsigned long)dirp);
    return 0;    
}



////////////////////////posix end/////////////////////////



