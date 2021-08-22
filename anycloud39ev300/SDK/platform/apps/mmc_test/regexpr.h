
/*
	REG_EXTENDED - 扩展方式匹配
	REG_ICASE    - 忽略大小写
	REG_NOSUB    - 不存储匹配后的结果
	REG_NEWLINE  - 识别换行符
*/
/*
#define REGEXPR_TRUE     1
#define REGEXPR_FALSE    0

#define REGEXPR_SUCCESS  0
#define REGEXPR_FAIL    -1
*/

#define LEN_ERROR        1024

#define ERGEX_HING_BUFFFULL "Target buffer is FULL."
#ifndef AK_TRUE
#define AK_TRUE  1
#define AK_FALSE 0
#endif

enum REGEX_ACTION {
	ACTION_PTRN_MATCH_ALL ,                                                                         //pc_pattern , i_regex_option , pc_split , i_split_option
	ACTION_PTRN_MATCH_ONE ,                                                                         //pc_pattern , i_regex_option
	ACTION_PTRN_DELETE_ALL ,                                                                        //pc_pattern , i_regex_option
	ACTION_PTRN_DELETE_ONE ,                                                                        //pc_pattern , i_regex_option
	ACTION_PTRN_REPLACE_ALL ,                                                                       //pc_pattern , i_regex_option , pc_replace
	ACTION_PTRN_REPLACE_ONE ,                                                                       //pc_pattern , i_regex_option , pc_replace
	ACTION_COMP_MATCH_ALL ,                                                                         //p_regex_t , pc_split , i_split_option
	ACTION_COMP_MATCH_ONE ,                                                                         //p_regex_t
	ACTION_COMP_DELETE_ALL ,                                                                        //p_regex_t
	ACTION_COMP_DELETE_ONE ,                                                                        //p_regex_t
	ACTION_COMP_REPLACE_ALL ,                                                                       //p_regex_t , pc_replace
	ACTION_COMP_REPLACE_ONE ,                                                                       //p_regex_t , pc_replace
};

enum SPLIT_OPTION {
	SPLIT_NONE ,
	SPLIT_FRONT ,
	SPLIT_TAIL ,
	SPLIT_MIDDLE ,
	SPLIT_FULL ,
};

enum EXEC_OPTION {
	EXEC_ALL ,
	EXEC_ONE ,
};

/*
struct regres {
	char *pc_data ;
	struct regres *p_regres_next ;
};
*/

struct regloop {                                                                                    //用以循环方式获取并处理正则结果的结构
	regex_t regex_t_use ;
	char *pc_pattern;
	int i_flags;

	int i_offset ;
} ;

int ak_regexpr_compare( char *pc_pattern , int i_flags ,char *pc_buff ) ;
int ak_regexpr_get( char *pc_buff , char *pc_res_in , int i_len_res , int i_regex_num , ... ) ;
int regexpr_match( regex_t *p_regex_t_in , char *pc_pattern , int i_flags , char *pc_buff , char *pc_res , int i_len_res , char *pc_split , int i_split_option , int i_exec_option ) ;
int regexpr_replace( regex_t *p_regex_t_in , char *pc_pattern , int i_flags , char *pc_buff , char *pc_res , int i_len_res , char *pc_replace , int i_exec_option ) ;

int init_regloop( struct regloop *p_regloop , char *pc_pattern , int i_flags );
int free_regloop( struct regloop *p_regloop );
int regexpr_match_loop( struct regloop *p_regloop , char *pc_buff , char *pc_res , int i_len_res );