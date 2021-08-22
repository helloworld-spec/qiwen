#ifndef _INI_PLAT_H_
#define _INI_PLAT_H_

static inline void* open_file_w(const char *file_name)
{
	return (void *)fopen(file_name, "w+");
}

static inline void* open_file_r(const char *file_name)
{
	return (void *)fopen(file_name, "r");
}

static inline void write_file(char *file_buf, void *file_id)
{	
	fputs(file_buf, (FILE *)file_id);
}

static inline void* read_file(char *line_data, int line_data_len, void *file_id)
{	
   return fgets(line_data, line_data_len, (FILE *)file_id);
}

static inline void close_file(void *file_id)
{
	if(NULL != file_id ) 
		fclose((FILE *)file_id);
}

static inline int file_eof(void *file_id)
{
	return feof((FILE *)file_id);
}

#endif
