#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ioctl.h>

#include "list.h"
#include "ini_plat.h"
#include "internal_error.h"

#include "ak_common.h"
#include "ak_thread.h"
#include "ak_ini.h"

#define CONFIG_LINE_BUF_SIZE    512
#define CONFIG_ITEM_SIZE    	50

struct item_info{
	char name[CONFIG_ITEM_SIZE];
	char value[CONFIG_ITEM_SIZE];
	struct list_head list;
};

struct title_info{
	char name[CONFIG_ITEM_SIZE];
	struct list_head item_head;
	struct list_head list;
};

struct config_info {
	int update;
	int ref;
	ak_mutex_t  lock;
	char cfg_file[100];
	struct list_head title_head;
	struct list_head list;
};

static const char *ini_version = "libapp_ini V2.0.01";
static struct list_head  g_cfg_head ;
static int init_flg = 0;
static ak_mutex_t g_config_lock;

#if SHOW_CONFIG_INFO
void ak_ini_dump_config(void *config_handle, char *title)
{
	struct title_info *title_entry = NULL;
	struct item_info *item_entry = NULL;
	struct config_info *config = ((struct config_info *)config_handle);

	ak_print_normal("\n--- dump [%s] info ---\n", title);
	list_for_each_entry(title_entry, &config->title_head, list) {
		if(0 == strcmp(title, title_entry->name)) {
			list_for_each_entry(item_entry, &title_entry->item_head, list){
				ak_print_normal("%s = %s\n", item_entry->name, item_entry->value);
			}
		}

	}
    ak_print_normal("--- dump [%s] info End ---\n\n", title);
}
#endif

/**
 * filter_space - filter space and tab in the head & tail of the string
 * @data: current line data info
 * @data_len: len of line data info
 * return: real data start index
 */
static int filter_space(char *data, int data_len)
{
	int index = 0;

    while((' '==data[index]) || ('\t' == data[index])){
        ++index;
    }

    while((' ' == data[data_len-1]) || ('\t' == data[data_len-1])
    	|| ('\r' == data[data_len-1]) || ('\n' == data[data_len-1])){
    	data[data_len-1] = 0x00;
        --data_len;
    }

    return index;
}

/**
 * filter_comment: filter comment
 * @data[in]: current line data info
 * return: 0 no config info; otherwize config info len
 */
static int filter_comment(const char *data)
{
	char *ptr = strstr(data, "####");
	if(NULL != ptr){
		*ptr = 0x00;
	}

	ptr = strchr(data, ';');
	if(NULL != ptr){
		*ptr = 0x00;
	}

	return strlen(data);
}

static struct config_info* add_new_cfg( const char *cfg_path )
{
	struct config_info *p_cfg = NULL;
	p_cfg = (struct config_info *)calloc(1,sizeof(struct config_info ));
	if(p_cfg == NULL){
		ak_print_error_ex("it fails to calloc\n");
		return NULL;
	}
	ak_thread_mutex_init(& p_cfg->lock);
	strncpy(p_cfg->cfg_file,cfg_path,100);
	INIT_LIST_HEAD(&p_cfg->title_head);
	p_cfg->ref ++;
	list_add_tail( &p_cfg->list, &g_cfg_head);

	return p_cfg;
}

static int cfg_save(const void *handle)
{
	int ret = 0;
	struct config_info *p_cfg = (struct config_info *)handle;
	struct title_info *p_title = NULL;
	struct item_info *p_item = NULL;

	if(p_cfg->update)
	{
		FILE *file_id = open_file_w(p_cfg->cfg_file);
		char *file_buf;
		unsigned int size = 0;
		unsigned int filelen = 0;

		if(file_id == NULL){
			ak_print_error_ex("config file don't open:%s\n", p_cfg->cfg_file);
			ret = -1;
		}

		file_buf = (char *)calloc(1, 512);
		if(file_buf == NULL){
			ak_print_error_ex("it fails to calloc\n");
			close_file(file_id);
			ret = -1;
		}
		if( ret < 0) return ret;
		ak_thread_mutex_lock(&p_cfg->lock);
		list_for_each_entry(p_title,&p_cfg->title_head,list){
			sprintf(file_buf,"\n[%s]\n", p_title->name);
			write_file(file_buf, file_id);
			size += strlen(p_title->name) + 4;
			list_for_each_entry(p_item,&p_title->item_head,list){
				sprintf(file_buf,"%s\t\t\t= %s\n", p_item->name, p_item->value);
				write_file(file_buf, file_id);
				size += strlen(p_item->name) + strlen(p_item->value) + 6;
			}
		}
		ak_thread_mutex_unlock(&p_cfg->lock);

		free(file_buf);
		close_file(file_id);
#if 1 //debug
		struct stat statbuff;
		filelen = (stat(p_cfg->cfg_file, &statbuff) < 0)? 0 : statbuff.st_size;
		ak_print_info_ex("anyka_config_save, size = %d, filelen = %d\n",size, filelen);
#endif
	}

	return ret;
}

static int cfg_destroy( void *handle)
{
	int ret = 0;
	struct config_info *p_cfg = (struct config_info *)handle;
	struct title_info *title_entry = NULL;
	struct title_info *title_ptr = NULL;
	struct item_info *item_entry = NULL;
	struct item_info *item_ptr = NULL;

	if(p_cfg->update) {
		ret = cfg_save((const void *)p_cfg);
	}

	ak_thread_mutex_lock(&p_cfg->lock);
	list_for_each_entry_safe(title_entry, title_ptr, &p_cfg->title_head, list){
		list_for_each_entry_safe(item_entry, item_ptr, 
			&title_entry->item_head, list){
			list_del(&item_entry->list);
			free(item_entry);
		}
		list_del(&title_entry->list);
		free(title_entry);
	}
	ak_thread_mutex_unlock(&p_cfg->lock);

	free(p_cfg);
	return ret;

}

/**
 * ak_ini_init: init one configure file .
 * @file[IN]: configure file path
 * return: handle of the configure file or null
 */
void* ak_ini_init(const char *file)
{
	if(NULL == file){
		set_error_no(ERROR_TYPE_INVALID_ARG);
		ak_print_error_ex("Invalid argument.\n");
		return NULL;
	}
	if(0 == init_flg){
		ak_thread_mutex_init(&g_config_lock);
		INIT_LIST_HEAD(&g_cfg_head);
		init_flg = 1;
	}

	struct config_info *p_cfg = NULL;

	ak_thread_mutex_lock(&g_config_lock);
	/* if already open, return struct config_info */
	list_for_each_entry(p_cfg, &g_cfg_head, list){
		if(strcmp(file, p_cfg->cfg_file) == 0){
			p_cfg->ref++;
			ak_thread_mutex_unlock(&g_config_lock);
			return (void *)p_cfg;
		}
	}

	FILE *file_id = open_file_r(file);
	if (!file_id) {
		ak_print_error_ex("fopen: %s, file: %s\n", strerror(errno), file);
		ak_thread_mutex_unlock(&g_config_lock);
		return NULL;
	}

	char line_data[CONFIG_LINE_BUF_SIZE] = {0};
	char *find = NULL;
	int index = 0;
	struct title_info *p_title = NULL;
	struct title_info *p_title2 = NULL;
	struct item_info *p_item = NULL;
	struct item_info *p_item2 = NULL;

	p_cfg = add_new_cfg(file);
	while(!file_eof(file_id)){
		memset(line_data, 0x00, sizeof(line_data));
		if(NULL == read_file(line_data, sizeof(line_data), file_id)){
            break;
        }

		if (strlen(line_data) > 0) {
			/* empty line */
			if ((1 == strlen(line_data)) && (0x0A == line_data[0])) {
				continue;
			}
			if (line_data[0] > 0x80) {
				ak_print_warning_ex("find exception char in config file\n");				
			}
		}
		
		index = filter_space(line_data, strlen(line_data));
		if(0x00 == filter_comment(&line_data[index])){
        	continue;
        }

        /* title */
        if('[' == line_data[index]){
			p_title = (struct title_info *)calloc(1, sizeof(struct title_info));
			if(NULL == p_title){
				goto anyka_config_init_err_tbl;
			}

			find = strchr(&line_data[index], ']');
            if(NULL == find){
            	ak_print_info_ex("unknow title: %s\n", &line_data[index]);
            	continue;
            }else{
            	*find = 0x00;
            }

            strcpy(p_title->name, &line_data[index+1]);
			INIT_LIST_HEAD(&p_title->item_head);
			list_add_tail( &p_title->list, &p_cfg->title_head);
			continue;
		}

		/* check items */
		find = strchr(&line_data[index], '=');
		if(NULL == find){
        	ak_print_info_ex("unknow item: %s\n", &line_data[index]);
        	continue;
        }

		if(NULL == p_title){
			ak_print_info_ex(":we find item before title\n");
			continue;
		}

		/* item data */
		p_item = (struct item_info *)calloc(1, sizeof(struct item_info));
		if(p_item == NULL){
			ak_print_error_ex("it fails to calloc\n");
			goto anyka_config_init_err_tbl;
		}

		*find = 0;
		find ++;

		/* item name */
		index += filter_space(&line_data[index], strlen(&line_data[index]));
        strcpy(p_item->name, &line_data[index]);

        /* item value */
        index = filter_space(find, strlen(find));
        strcpy(p_item->value, &find[index]);

		list_add_tail( &p_item->list, &p_title->item_head);
	}

	close_file(file_id);
	ak_thread_mutex_unlock(&g_config_lock);
	return (void *)p_cfg;

anyka_config_init_err_tbl:
	if(file_id){
		close_file(file_id);
	}

	list_del(&p_cfg->list);

	list_for_each_entry_safe(p_title, p_title2, &p_cfg->title_head, list){
		list_for_each_entry_safe(p_item, p_item2, &p_title->item_head, list){
			list_del(&p_item->list);
			free(p_item);
		}
		list_del(&p_title->list);
		free(p_title);
	}
	free(p_cfg);
	ak_thread_mutex_unlock(&g_config_lock);

	return NULL;
}

/**
 * ak_ini_add_item: set item value. if title or item is not exist,creat it .
 * @handle[IN]: configure file handle
 * @title[IN]:  title name
 * @item[IN]:  item name
 * @value[IN]: value of item, strlen < 50
 * return: 0 - success; otherwise -1;
 */
int ak_ini_set_item_value(const void *handle, const char *title,
		const char *item, const char *value)
{
	struct config_info *p_cfg = ((struct config_info *)handle);
	struct title_info *p_title = NULL;
	struct item_info *p_item = NULL;
	const char *title_name = title;
	const char *item_name = item;
	int ret = 0;

	if((!handle) || (!title) || (!item) || (!value)){
		set_error_no(ERROR_TYPE_INVALID_ARG);
		ak_print_error_ex("Invalid argument.\n");
		return AK_FAILED;
	}
	ak_thread_mutex_lock(&p_cfg->lock);
	list_for_each_entry(p_title, &p_cfg->title_head, list){
		if(strcmp(title_name, p_title->name) == 0){
			list_for_each_entry(p_item, &p_title->item_head, list){
				if(strcmp(item_name, p_item->name) == 0){
					strcpy(p_item->value, value);
					p_cfg->update = 1;
					goto set_item_end;
				}
			}
			
			p_item = (struct item_info *)calloc(1, sizeof(struct item_info));
			if(!p_item){
				ak_print_error_ex("calloc p_item failed\n");
				ret =  -1;
				goto set_item_end;
			}
			
			strcpy(p_item->name, item_name);
			strcpy(p_item->value, value);

			list_add_tail( &p_item->list, &p_title->item_head);
			p_cfg->update = 1;
			goto set_item_end;
		}
	}   

	ak_print_info_ex("config file don't find title:%s\n", title_name);
	p_title = (struct title_info *)calloc(1, sizeof(struct title_info));
	if(!p_title){
		ak_print_error_ex("calloc p_title failed\n");
		ret =  -1;
		goto set_item_end;
	}
	
	p_item = (struct item_info *)calloc(1, sizeof(struct item_info));
	if(!p_item ){
		ak_print_error_ex("calloc p_item failed\n");
		free(p_title);
		ret =  -1;
		goto set_item_end;
	}

	strcpy(p_title->name, title_name);
	INIT_LIST_HEAD(&p_title->item_head);
	list_add_tail( &p_title->list, &p_cfg->title_head);

	strcpy(p_item->name, item_name);
	strcpy(p_item->value, value);
	list_add_tail( &p_item->list, &p_title->item_head);

	p_cfg->update = 1;
	
set_item_end:
	ak_thread_mutex_unlock(&p_cfg->lock);

	return ret;
}

/**
 * ak_ini_get_item_value: get value of one item .
 * @handle[IN]: configure file handle
 * @title[IN]:   title name
 * @item[IN]:  item name
 * @value[OUT]: value of item
 * return: 0 - success; otherwise -1;
 */
int ak_ini_get_item_value(const void *handle, const char *title,
		const char *item, char *value)
{
	int ret = -1;
	struct config_info *p_cfg = ((struct config_info *)handle);
	struct title_info *p_title = NULL;
	struct item_info *p_item = NULL;
	const char *title_name = title;
	const char *item_name = item;

	if((!handle) || (!title) || (!item) || (!value)){
		set_error_no(ERROR_TYPE_INVALID_ARG);
		ak_print_error_ex("Invalid argument.\n");
		return AK_FAILED;
	}
	ak_thread_mutex_lock(&p_cfg->lock);
	list_for_each_entry(p_title,&p_cfg->title_head,list){
		if(strcmp(title_name, p_title->name) == 0){
			list_for_each_entry(p_item,&p_title->item_head,list){
				if(strcmp(item_name, p_item->name) == 0){
					strcpy(value, p_item->value);
					ret = 0;
					break;
				}
			}
		}
	}
	ak_thread_mutex_unlock(&p_cfg->lock);

	return ret;
}

/**
 * ak_ini_del_item: delete one item  .
 * @handle[IN]: configure file handle
 * @title[IN]:  title name
 * @item[IN]: item name
 * return: 0 - success; otherwise -1;
 */
int ak_ini_del_item(const void *handle, const char *title, const char *item)
{
	int ret = AK_FAILED;
	struct config_info *p_cfg = (struct config_info *)handle;
	struct title_info *title_entry = NULL;
	struct title_info *title_ptr = NULL;
	struct item_info *item_entry = NULL;
	struct item_info *item_ptr = NULL;
	const char *title_name = title;
	const char *item_name = item;

	if((!handle) || (!title) || (!item)){
		set_error_no(ERROR_TYPE_INVALID_ARG);
		ak_print_error_ex("Invalid argument.\n");
		return AK_FAILED;
	}
	ak_thread_mutex_lock(&p_cfg->lock);
	list_for_each_entry_safe(title_entry, title_ptr, &p_cfg->title_head, list){
		if(strcmp(title_name, title_entry->name) == 0){
			list_for_each_entry_safe(item_entry, item_ptr, 
				&title_entry->item_head,list){
				if(strcmp(item_name, item_entry->name) == 0){
					list_del(&item_entry->list);
					free(item_entry);
					ret = AK_SUCCESS;
					p_cfg->update = 1;
					break;
				}
			}
		}
	}
	ak_thread_mutex_unlock(&p_cfg->lock);

	return ret;
}

/**
 * ak_ini_del_title: delete one title ,the items attach to this title delete also.
 * @handle[IN]: configure file handle
 * @title[IN]:  title name
 * return: 0 - success; otherwise -1;
 */
int ak_ini_del_title(const void *handle, const char *title)
{
	int ret = AK_FAILED;
	struct config_info *p_cfg = (struct config_info *)handle;
	struct title_info *title_entry = NULL;
	struct title_info *title_ptr = NULL;
	struct item_info *item_entry = NULL;
	struct item_info *item_ptr = NULL;
	const char *title_name = title;

	if((!handle) || (!title)){
		set_error_no(ERROR_TYPE_INVALID_ARG);
		ak_print_error_ex("Invalid argument.\n");
		return AK_FAILED;
	}
	ak_thread_mutex_lock(&p_cfg->lock);
	list_for_each_entry_safe(title_entry, title_ptr, &p_cfg->title_head, list){
		if(strcmp(title_name, title_entry->name) == 0){
			list_for_each_entry_safe(item_entry, item_ptr, &title_entry->item_head, list){
				list_del(&item_entry->list);
				free(item_entry);
			}

			p_cfg->update = 1;
			list_del(&title_entry->list);
			free(title_entry);
			ret = AK_SUCCESS;
			break;
		}
	}
	ak_thread_mutex_unlock(&p_cfg->lock);

	return ret;
}

/**
 * ak_ini_flush_data: flush data of  handle from memory to file .
 * @handle[IN]: configure file handle
 * return: 0 - success; otherwise -1;
 */
int ak_ini_flush_data(const void *handle)
{
	if(!handle){
		set_error_no(ERROR_TYPE_INVALID_ARG);
		ak_print_error_ex("Invalid argument.\n");
		return AK_FAILED;
	}
	 return cfg_save(handle);
}

/**
 * ak_ini_destroy: close one config handle .
 * @handle[IN]: configure file handle
 * return: 0 - success; otherwise -1;
 */
int ak_ini_destroy(void *handle)
{
	int ret = 0;
	struct config_info *cfg_now = (struct config_info *)handle;
	struct config_info *p_cfg ;

	if(!handle){
		set_error_no(ERROR_TYPE_INVALID_ARG);
		ak_print_error_ex("Invalid argument.\n");
		return AK_FAILED;
	}
	if(cfg_now->ref > 0) 
		cfg_now->ref--;
		
	ak_thread_mutex_lock(&g_config_lock);
	/*find it in list*/
	list_for_each_entry(p_cfg,&g_cfg_head,list){
		if(p_cfg == cfg_now){
			if( 0 == cfg_now->ref){
				/*remove from list*/
				ak_print_debug_ex("remove from list:%s.\n",cfg_now->cfg_file);
				list_del(&cfg_now->list);
			}
			break;
		}
	}
	ak_thread_mutex_unlock(&g_config_lock);

	if( 0 == cfg_now->ref){
		ret = cfg_destroy((void *)cfg_now);
	}

	return ret;
}

const char* ak_ini_get_version(void)
{
	return ini_version;
}