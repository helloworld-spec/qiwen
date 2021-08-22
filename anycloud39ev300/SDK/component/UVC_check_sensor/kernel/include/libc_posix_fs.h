#ifndef  __LIBC_POSIX_FS_H__
#define  __LIBC_POSIX_FS_H__

////////////////////libc 接口 begin ////////////
#include <stdio.h>

#define   fopen(a,b)    ak_fopen(a,b)
#define   fclose(a)        ak_fclose(a)
#define   fread(a,b,c,d)         ak_fread(a,b,c,d) 
//#define   fscanf(a,b,...)        ak_fscanf(a,b,...)
#undef    fgetc  
#define   fgetc(a)         ak_fgetc(a)
#undef    getc
#define   getc(a)          ak_getc(a)
//#define   fgets(a,b,c)         ak_fgets(a,b,c)
#define   fwrite(a,b,c,d)        ak_fwrite(a,b,c,d)
//#define   fprintf(a,b,...)       ak_fprintf(a,b,...) 
#undef    fputc  
#define   fputc(a,b)         ak_fputc(a,b)   
#undef    putc  
#define   putc(a,b)          ak_putc(a,b)    
#define   fputs(a,b)         ak_fputs(a,b)   
#define   fseek(a,b,c)         ak_fseek(a,b,c)   
#define   fseeko(a,b,c)         ak_fseek(a,b,c)   
//#define   fgetpos(a,b)       ak_fgetpos(a,b) 
//#define   fsetpos(a,b)       ak_fsetpos(a,b)
#undef    feof  
#define   feof(a)          ak_feof(a)
#define   fflush(a)       ak_fflush(a)
#define   ftell(a)         ak_ftell(a)
#define   remove(a)        ak_remove(a)
#define   rename(a,b)        ak_rename(a,b)
#undef    ferror
#define   ferror(a)        (-1) // return a const 
#define   strerror(a)      "unknown error\n"
#define   perror(a)      printk("unknown error\n")



FILE *ak_fopen( const char *fname, const char *mode );
int ak_fclose( FILE *stream );

size_t ak_fread( void *buffer, size_t size, size_t num, FILE *stream );
//int ak_fscanf( FILE *stream, const char *format, ... );
int ak_fgetc( FILE *stream );
int ak_getc( FILE *stream );
//char *ak_fgets( char *str, int num, FILE *stream );

int ak_fwrite( const void *buffer, size_t size, size_t count, FILE *stream );
//int ak_fprintf( FILE *stream, const char *format, ... );
int ak_fputc( int ch, FILE *stream );
int ak_putc( int ch, FILE *stream );
int ak_fputs( const char *str, FILE *stream );

int ak_fseek( FILE *stream, long offset, int origin );
//int ak_fgetpos( FILE *stream, fpos_t *position );
//int ak_fsetpos( FILE *stream, const fpos_t *position );
int ak_feof( FILE *stream );

int ak_fflush( FILE *stream );
long ak_ftell( FILE *stream );
int ak_remove( const char *fname );
int ak_rename( const char *oldfname, const char *newfname );
///////////////////libc 接口 end ////////////


////////////////////////posix begin////////////////////////
#include <sys/stat.h>
#include <sys/statfs.h>
#include <sys/types.h>
#include <dirent.h>

#define mkdir(a,b)        ak_mkdir(a,b)
#define statfs(a,b)       ak_statfs(a,b)
#define stat(a,b)         ak_stat(a,b)
#define opendir(a)        ak_opendir(a)
#define readdir(a)        ak_readdir(a) 
#define readdir_r(a,b,c)        ak_readdir_r(a,b,c)
#define closedir(a)          ak_closedir(a)
#define fileno(a)         ((int)a)
#define fdatasync(a)      ak_fflush((FILE*)a)

int ak_mkdir(const char *path, mode_t mode);
int ak_statfs(const char *path, struct statfs *buf);
int ak_stat( const char *path, struct stat *buffer );
DIR * ak_opendir(const char *name);
//struct dirent * ak_readdir(DIR *dirp);
int ak_readdir_r(DIR *dirp, struct dirent *entry, struct dirent **result);
int ak_closedir(DIR *dirp);
////////////////////////posix end/////////////////////////
#endif

