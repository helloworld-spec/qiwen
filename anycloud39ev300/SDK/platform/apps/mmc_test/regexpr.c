#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <regex.h>
#include <string.h>
#include <stdarg.h>

#include "regexpr.h"
#include "printcolor.h"

int init_regloop( struct regloop *p_regloop , char *pc_pattern , int i_flags )
{
	char ac_error[ LEN_ERROR ];
	int i_status = 0;

	memset( p_regloop , 0 , sizeof( struct regloop ) );
	memset( ac_error , 0 , LEN_ERROR );

	if ( ( pc_pattern != NULL ) &&
	     ( ( i_status = regcomp( &p_regloop->regex_t_use , pc_pattern , i_flags ) ) == REG_NOERROR ) ) {
		p_regloop->pc_pattern = pc_pattern;
	}
	else {
		if ( pc_pattern != NULL ) {
			regerror( i_status , &p_regloop->regex_t_use , ac_error , LEN_ERROR ) ;
		}
		DEBUG_PRINT( COLOR_MODE_NORMAL , COLOR_BACK_BLACK , COLOR_FRONT_RED ,
		             "i_status= %d pc_pattern= '%s' ac_error= '%s'" ,
		             i_status , pc_pattern , ac_error )
		return AK_FALSE;
	}
	return AK_TRUE;
}

int free_regloop( struct regloop *p_regloop )
{
	if( p_regloop->pc_pattern != NULL ) {
		regfree( &p_regloop->regex_t_use );
	}
	return 0;
}

int regexpr_match_loop( struct regloop *p_regloop , char *pc_buff , char *pc_res , int i_len_res )
{
	int i_len_match = 0, i_len_cpy = 0;
	regmatch_t regmatch_t_res ;
	char *pc_now = pc_buff + p_regloop->i_offset ;

	pc_res[ 0 ] = 0;
	if ( regexec( &p_regloop->regex_t_use , pc_now , 1 , &regmatch_t_res , 0 ) == 0 ) {
		if ( ( i_len_match = regmatch_t_res.rm_eo - regmatch_t_res.rm_so ) <= 0 ) {                 //匹配到的字符串长度为0
			DEBUG_HINT( COLOR_MODE_NORMAL , COLOR_BACK_BLACK , COLOR_FRONT_RED )
			return 0;
		}

		if ( i_len_res > i_len_match ) {
			i_len_cpy = i_len_match;
		}
		else {
			i_len_cpy = i_len_res - 1;
		}
		memcpy( pc_res , pc_now + regmatch_t_res.rm_so , i_len_cpy ) ;
		pc_res[ i_len_cpy ] = 0 ;
		p_regloop->i_offset += regmatch_t_res.rm_eo;
	}
	return i_len_cpy ;
}


                                                                                                    //获取全部匹配正则表达式的字符串
int regexpr_match( regex_t *p_regex_t_in , char *pc_pattern , int i_flags , char *pc_buff , char *pc_res , int i_len_res , char *pc_split , int i_split_option , int i_exec_option )
{
	int i_status , i_len_match = 0 , i_len_return = 0 , i_len_split = 0 ;
	regex_t regex_t_pattern , *p_regex_t_now = NULL ;
	char ac_error[ LEN_ERROR ] , *pc_now = pc_buff , c_buff_full = AK_FALSE ;
	regmatch_t regmatch_t_res ;

	if ( p_regex_t_in != NULL ) {
		p_regex_t_now = p_regex_t_in ;
	}
	else if ( ( i_status = regcomp( &regex_t_pattern , pc_pattern , i_flags ) ) == REG_NOERROR ) {
		p_regex_t_now = &regex_t_pattern ;
	}
	else {
		regerror( i_status , &regex_t_pattern , ac_error , LEN_ERROR ) ;
		DEBUG_PRINT( COLOR_MODE_NORMAL , COLOR_BACK_BLACK , COLOR_FRONT_RED ,
		             "i_status= %d pc_pattern= '%s' ac_error= '%s'" ,
		             i_status , pc_pattern , ac_error )
		return 0 ;
	}

	if ( i_split_option != SPLIT_NONE ) {
		i_len_split = strlen( pc_split ) ;
		if ( i_len_split <= 0 ) {
			i_split_option = SPLIT_NONE ;
		}
	}

	i_len_return = 0 ;                                                                              //返回的字符串长度
	while ( ( i_status = regexec( p_regex_t_now , pc_now , 1 , &regmatch_t_res , 0 ) ) == 0 ) {
		if ( ( i_len_match = regmatch_t_res.rm_eo - regmatch_t_res.rm_so ) <= 0 ) {                 //匹配到的字符串长度为0
			DEBUG_HINT( COLOR_MODE_NORMAL , COLOR_BACK_BLACK , COLOR_FRONT_RED )
			break ;
		}

		switch( i_split_option ) {
			case SPLIT_FULL :                                                                       //分隔符前后都有
				if ( ( i_len_return + i_len_split ) < i_len_res ) {                                 //目标缓冲区未满
					if ( i_len_return == 0 ) {                                                      //判断是否存在数据,没有有数据才拷贝分隔符
						memcpy( pc_res + i_len_return , pc_split , i_len_split ) ;                  //拷贝分隔符
						i_len_return += i_len_split;                                                //设置返回的字符串长度
					}
				}
				else {
					c_buff_full = AK_TRUE ;
				}
				break ;
			case SPLIT_FRONT :                                                                      //分隔符在前
				if ( ( i_len_return + i_len_split ) < i_len_res ) {                                 //目标缓冲区未满
					memcpy( pc_res + i_len_return , pc_split , i_len_split ) ;                      //拷贝分隔符
					i_len_return += i_len_split;                                                    //设置返回的字符串长度
				}
				else {
					c_buff_full = AK_TRUE ;
				}
				break ;
			case SPLIT_MIDDLE :                                                                     //分隔符在结果之间
				if ( ( i_len_return + i_len_split ) < i_len_res ) {                                 //目标缓冲区未满
					if ( i_len_return != 0 ) {                                                      //判断是否存在数据,有数据才拷贝分隔符
						memcpy( pc_res + i_len_return , pc_split , i_len_split ) ;                  //拷贝分隔符
						i_len_return += i_len_split;                                                //设置返回的字符串长度
					}
				}
				else {
					c_buff_full = AK_TRUE ;
				}
				break ;
		}

		if( c_buff_full != AK_TRUE ) {                                                              //判断缓冲区是否已满,未满则拷贝数据
			if ( ( i_len_return + i_len_match ) < i_len_res ) {                                     //目标缓冲区未满
				memcpy( pc_res + i_len_return , pc_now + regmatch_t_res.rm_so , i_len_match ) ;
				i_len_return += i_len_match ;                                                       //设置返回的字符串长度
				pc_now += regmatch_t_res.rm_so + i_len_match ;                                      //跳转到替换结果后的地址
			}
			else {                                                                                  //目标缓冲区已满,部分拷贝
				c_buff_full = AK_TRUE ;
				memcpy( pc_res + i_len_return , pc_now + regmatch_t_res.rm_so , i_len_res - 1 - i_len_return ) ;
				i_len_return = i_len_res - 1 ;
			}
		}

		if( c_buff_full != AK_TRUE ) {
			switch( i_split_option ) {
				case SPLIT_TAIL :                                                                   //分隔符在后
				case SPLIT_FULL :                                                                   //分隔符前后都有
					if ( ( i_len_return + i_len_split ) < i_len_res ) {                             //目标缓冲区未满
						memcpy( pc_res + i_len_return , pc_split , i_len_split ) ;                  //拷贝分隔符
						i_len_return += i_len_split;                                                //设置返回的字符串长度
					}
					else {
						c_buff_full = AK_TRUE ;
					}
					break ;
			}
		}

		pc_res[ i_len_return ] = 0x0 ;
		if ( ( i_exec_option == EXEC_ONE ) || ( c_buff_full == AK_TRUE ) ) {
			break ;
		}
	}
	if( p_regex_t_now == &regex_t_pattern ) {
		regfree( &regex_t_pattern ) ;
	}
	return i_len_return ;
}
                                                                                                    //根据正则表达式替换字符串
int regexpr_replace( regex_t *p_regex_t_in , char *pc_pattern , int i_flags , char *pc_buff , char *pc_res , int i_len_res , char *pc_replace , int i_exec_option )
{
	int i_status , i_len_replace = 0 , i_len_match = 0 , i_len_now = 0 , i_len_return = 0 , i_len_move = 0 , i_len_limit = 0 , i_len_cpy = 0 ;;
	regex_t regex_t_pattern , *p_regex_t_now = NULL ;
	char ac_error[ LEN_ERROR ] , *pc_now = pc_res , ac_match_tmp[ LEN_ERROR ] ;
	regmatch_t regmatch_t_res ;

	if ( p_regex_t_in != NULL ) {
		p_regex_t_now = p_regex_t_in ;
	}
	else if ( ( i_status = regcomp( &regex_t_pattern , pc_pattern , i_flags ) ) == REG_NOERROR ) {
		p_regex_t_now = &regex_t_pattern ;
	}
	else {
		regerror( i_status , &regex_t_pattern , ac_error , LEN_ERROR ) ;
		DEBUG_PRINT( COLOR_MODE_NORMAL , COLOR_BACK_BLACK , COLOR_FRONT_RED ,
		             "i_status= %d pc_pattern= '%s' ac_error= '%s'" ,
		             i_status , pc_pattern , ac_error )
		return 0 ;
	}
	if( pc_replace != NULL ) {                                                                      //判断是否有replace的字符串
		i_len_replace = strlen( pc_replace ) ;                                                      //获取替换字符串的长度
	}

	strncpy( pc_res , pc_buff , i_len_res - 1 ) ;                                                   //将源字符串先拷贝到返回结果中去
	pc_res[ i_len_res - 1 ] = 0x0 ;
	i_len_now = strlen( pc_res ) ;                                                                  //pc_now指向地址后的字符串长度
	i_len_return = i_len_now ;                                                                      //返回的字符串长度

	while ( ( i_len_now > 0 ) && ( i_status = regexec( p_regex_t_now , pc_now , 1 , &regmatch_t_res , 0 ) ) == 0 ) {
		if ( ( i_len_match = regmatch_t_res.rm_eo - regmatch_t_res.rm_so ) <= 0 ) {                 //匹配到的字符串长度为0
			DEBUG_HINT( COLOR_MODE_NORMAL , COLOR_BACK_BLACK , COLOR_FRONT_RED )
			break ;
		}

		memcpy( ac_match_tmp , pc_now + regmatch_t_res.rm_so , i_len_match ) ;
		ac_match_tmp[ i_len_match ] = 0 ;
		/*
		DEBUG_PRINT( COLOR_MODE_NORMAL , COLOR_BACK_BLACK , COLOR_FRONT_RED , "i_len_res= %d so= %d eo= %d i_len_now= %d i_len_replace= %d ac_match_tmp= '%s'" ,
		             i_len_res , regmatch_t_res.rm_so , regmatch_t_res.rm_eo , i_len_now , i_len_replace , ac_match_tmp )
		*/
		i_len_return = i_len_return - i_len_match + i_len_replace ;                                 //设置返回的字符串长度
		if( i_len_return < i_len_res ) {                                                            //当前返回长度小于于缓冲区长度
			memmove( pc_now + regmatch_t_res.rm_so + i_len_replace  , pc_now + regmatch_t_res.rm_eo , i_len_now - regmatch_t_res.rm_eo ) ;
			if ( i_len_replace != 0 ) {                                                             //有替换字符串
				memcpy( pc_now + regmatch_t_res.rm_so , pc_replace , i_len_replace ) ;
			}
			//DEBUG_PRINT( COLOR_MODE_NORMAL , COLOR_BACK_BLACK , COLOR_FRONT_RED , "pc_now + regmatch_t_res.rm_so= '%s'" , pc_now + regmatch_t_res.rm_so )
			pc_now += regmatch_t_res.rm_so + i_len_replace ;                                        //跳转到替换结果后的地址
			i_len_now -= regmatch_t_res.rm_eo ;                                                     //剩下没检索的长度
		}
		else {                                                                                      //返回结果缓冲区已满
			i_len_move = i_len_now - regmatch_t_res.rm_eo - ( i_len_return + 1 - i_len_res ) ;      //计算可以移动的长度
			if ( i_len_move > 0 ) {                                                                 //移动字符串
				memmove( pc_now + regmatch_t_res.rm_so + i_len_replace  , pc_now + regmatch_t_res.rm_eo , i_len_move ) ;
			}
			i_len_limit = i_len_res - 1 - ( pc_now + regmatch_t_res.rm_so - pc_res ) ;              //剩余缓冲区长度
			if( i_len_limit >= i_len_replace ) {                                                    //获取拷贝的替换字符串的长度
				i_len_cpy = i_len_replace ;
			}
			else {
				i_len_cpy = i_len_limit ;
			}
			memcpy( pc_now + regmatch_t_res.rm_so , pc_replace , i_len_cpy ) ;                      //拷贝替换字符串
			pc_now += regmatch_t_res.rm_so + i_len_cpy ;                                            //跳转到替换结果后的地址
			i_len_now = i_len_res - 1 - ( pc_now - pc_res ) ;                                       //剩下没检索的长度
			i_len_return = i_len_res - 1 ;
			DEBUG_PRINT( COLOR_MODE_NORMAL , COLOR_BACK_BLACK , COLOR_FRONT_RED , "%s" , ERGEX_HING_BUFFFULL )
		}
		pc_res[ i_len_return ] = 0x0 ;
		if( i_exec_option == EXEC_ONE ) {
			break ;
		}
	}

	if( p_regex_t_now == &regex_t_pattern ) {
		regfree( &regex_t_pattern ) ;
	}
	return i_len_return ;
}

int ak_regexpr_compare( char *pc_pattern , int i_flags , char *pc_buff )                            //对获取结果进行比较
{
	int i_status;
	regex_t regex_t_pattern;
	char ac_error[ LEN_ERROR ] ;

	if ( ( i_status = regcomp( &regex_t_pattern , pc_pattern , i_flags ) ) != REG_NOERROR ) {
		regerror( i_status , &regex_t_pattern , ac_error , LEN_ERROR ) ;
		DEBUG_PRINT( COLOR_MODE_NORMAL , COLOR_BACK_BLACK , COLOR_FRONT_RED ,
		             "i_status= %d pc_pattern= '%s' ac_error= '%s'" ,
		             i_status , pc_pattern , ac_error )
		return REG_NOMATCH ;
	}
	i_status = regexec( &regex_t_pattern , pc_buff , (size_t)0 , NULL , 0 ) ;
	regfree( &regex_t_pattern ) ;

	return i_status ;
}

int ak_regexpr_get( char *pc_buff , char *pc_res_in , int i_len_res , int i_regex_num , ... )          //通过多个正则表达式获取最终的字符串结果
{
	va_list va_list_regex ;
	char *pc_pattern = NULL , *pc_split = NULL , *pc_replace = NULL , *pc_res_tmp = NULL , *pc_source = NULL , *pc_target = NULL ;
	int i , i_len = 0 , i_action , i_regex_option , i_split_option ;
	regex_t *p_regex_t ;

	if ( i_len_res > 0 ) {
		pc_res_tmp = ( char * )calloc( 1 , i_len_res ) ;
	}
	else {
		return 0 ;
	}
	pc_res_in[ 0 ] = 0 ;
	va_start( va_list_regex, i_regex_num ) ;

	for( i = 0 ; i < i_regex_num ; i ++ ) {
		if ( ( pc_source == NULL ) || ( pc_target == NULL ) ) {
			pc_source = pc_buff ;
			pc_target = pc_res_in ;
		}
		else {
			if( pc_target == pc_res_in ) {
				pc_target = pc_res_tmp ;
				pc_source = pc_res_in ;
			}
			else {
				pc_target = pc_res_in ;
				pc_source = pc_res_tmp ;
			}
		}
		i_action = ( int )va_arg( va_list_regex , int ) ;
		switch( i_action ) {
			case ACTION_PTRN_MATCH_ALL :                                                            //提取全部符合的字符串
				pc_pattern = ( char * )va_arg( va_list_regex , char * ) ;
				i_regex_option = ( int )va_arg( va_list_regex , int ) ;
				pc_split = ( char * )va_arg( va_list_regex , char * ) ;
				i_split_option = ( int )va_arg( va_list_regex , int ) ;
				i_len = regexpr_match( NULL , pc_pattern , i_regex_option , pc_source , pc_target , i_len_res , pc_split , i_split_option , EXEC_ALL ) ;
				break ;
			case ACTION_PTRN_MATCH_ONE :                                                            //提取一个字符串
				pc_pattern = ( char * )va_arg( va_list_regex , char * ) ;
				i_regex_option = ( int )va_arg( va_list_regex , int ) ;
				i_len = regexpr_match( NULL , pc_pattern , i_regex_option , pc_source , pc_target , i_len_res , NULL , SPLIT_NONE , EXEC_ONE ) ;
				break ;
			case ACTION_PTRN_DELETE_ALL :                                                           //删除全部字符串
				pc_pattern = ( char * )va_arg( va_list_regex , char * ) ;
				i_regex_option = ( int )va_arg( va_list_regex , int ) ;
				i_len = regexpr_replace( NULL , pc_pattern , i_regex_option , pc_source , pc_target , i_len_res , NULL , EXEC_ALL ) ;
				break ;
			case ACTION_PTRN_DELETE_ONE :                                                           //删除一个字符串
				pc_pattern = ( char * )va_arg( va_list_regex , char * ) ;
				i_regex_option = ( int )va_arg( va_list_regex , int ) ;
				i_len = regexpr_replace( NULL , pc_pattern , i_regex_option , pc_source , pc_target , i_len_res , NULL , EXEC_ONE ) ;
				break ;
			case ACTION_PTRN_REPLACE_ALL :                                                          //替换全部字符串
				pc_pattern = ( char * )va_arg( va_list_regex , char * ) ;
				i_regex_option = ( int )va_arg( va_list_regex , int ) ;
				pc_replace = ( char * )va_arg( va_list_regex , char * ) ;
				i_len = regexpr_replace( NULL , pc_pattern , i_regex_option , pc_source , pc_target , i_len_res , pc_replace , EXEC_ALL) ;
				break ;
			case ACTION_PTRN_REPLACE_ONE :                                                          //替换一个字符串
				pc_pattern = ( char * )va_arg( va_list_regex , char * ) ;
				i_regex_option = ( int )va_arg( va_list_regex , int ) ;
				pc_replace = ( char * )va_arg( va_list_regex , char * ) ;
				i_len = regexpr_replace( NULL , pc_pattern , i_regex_option , pc_source , pc_target , i_len_res , pc_replace , EXEC_ONE ) ;
				break ;

			case ACTION_COMP_MATCH_ALL :                                                            //提取全部符合的字符串
				p_regex_t = ( regex_t * )va_arg( va_list_regex , regex_t * ) ;
				pc_split = ( char * )va_arg( va_list_regex , char * ) ;
				i_split_option = ( int )va_arg( va_list_regex , int ) ;
				i_len = regexpr_match( p_regex_t , NULL , 0  , pc_source , pc_target , i_len_res , pc_split , i_split_option , EXEC_ALL ) ;
				break ;
			case ACTION_COMP_MATCH_ONE :                                                            //提取一个字符串
				p_regex_t = ( regex_t * )va_arg( va_list_regex , regex_t * ) ;
				i_len = regexpr_match( p_regex_t , NULL , 0  , pc_source , pc_target , i_len_res , NULL , SPLIT_NONE , EXEC_ONE ) ;
				break ;
			case ACTION_COMP_DELETE_ALL :                                                           //删除全部字符串
				p_regex_t = ( regex_t * )va_arg( va_list_regex , regex_t * ) ;
				i_len = regexpr_replace( p_regex_t , NULL , 0  , pc_source , pc_target , i_len_res , NULL , EXEC_ALL ) ;
				break ;
			case ACTION_COMP_DELETE_ONE :                                                           //删除一个字符串
				p_regex_t = ( regex_t * )va_arg( va_list_regex , regex_t * ) ;
				i_len = regexpr_replace( p_regex_t , NULL , 0  , pc_source , pc_target , i_len_res , NULL , EXEC_ONE ) ;
				break ;
			case ACTION_COMP_REPLACE_ALL :                                                          //替换全部字符串
				p_regex_t = ( regex_t * )va_arg( va_list_regex , regex_t * ) ;
				pc_replace = ( char * )va_arg( va_list_regex , char * ) ;
				i_len = regexpr_replace( p_regex_t , NULL , 0  , pc_source , pc_target , i_len_res , pc_replace , EXEC_ALL) ;
				break ;
			case ACTION_COMP_REPLACE_ONE :                                                          //替换一个字符串
				p_regex_t = ( regex_t * )va_arg( va_list_regex , regex_t * ) ;
				pc_replace = ( char * )va_arg( va_list_regex , char * ) ;
				i_len = regexpr_replace( p_regex_t , NULL , 0  , pc_source , pc_target , i_len_res , pc_replace , EXEC_ONE ) ;
				break ;
			default :
				DEBUG_HINT( COLOR_MODE_NORMAL , COLOR_BACK_BLACK , COLOR_FRONT_RED )
		}
		//DEBUG_PRINT( COLOR_MODE_NORMAL , COLOR_BACK_BLACK , COLOR_FRONT_BLUE , "i_len_res= %d pc_pattern= '%s' pc_action[ %d ]= '%c' %s" , i_len_res , pc_pattern , i , pc_action[ i ] , pc_res_in ) ;

		if ( i_len <= 0 ) {
			break ;
		}
	}
	va_end( va_list_regex ) ;
	if( pc_target != pc_res_in ) {
		memcpy( pc_res_in , pc_target , i_len_res ) ;
	}
	FREE_POINT( pc_res_tmp ) ;
	return i_len ;
}

/*
int ak_regexpr_set( char *pc_buff , char *pc_res , int i_len_res , int i_times , ... )              //通过多个正则表达式进行替换相应符合正则表达式的字符串
{
	va_list va_list_regex ;
	char *pc_pattern = NULL , *pc_replace = NULL , c_res = AK_FALSE , *pc_res_tmp = NULL ;
	int i , i_len = 0 ;

	if ( i_len_res > 0 ) {
		pc_res_tmp = ( char * )calloc( 1 , i_len_res ) ;
	}
	pc_res[ 0 ] = 0 ;

	va_start( va_list_regex, i_times ) ;
	for( i = 0 ; i < i_times ; i ++ ) {
		pc_pattern = ( char * )va_arg( va_list_regex , char * ) ;
		pc_replace = ( char * )va_arg( va_list_regex , char * ) ;

		if( pc_pattern == NULL ) {
			break ;
		}
		if( c_res == AK_FALSE ) {                                                                   //处理数据存储在pc_buff
			i_len = regexpr_replace( pc_pattern , pc_buff , pc_res , i_len_res , pc_replace ) ;
			c_res = AK_TRUE ;
		}
		else {                                                                                      //处理数据存储在pc_res
			memcpy( pc_res_tmp , pc_res , i_len ) ;
			pc_res_tmp[ i_len ] = 0 ;
			i_len = regexpr_replace( pc_pattern , pc_res_tmp , pc_res , i_len_res , pc_replace ) ;
		}
		DEBUG_PRINT( COLOR_MODE_NORMAL , COLOR_BACK_BLACK , COLOR_FRONT_BLUE , "i_len= %d pc_res= '%s'" , i_len , pc_res )
		if ( i_len <= 0 ) {
			break ;
		}
	}
	va_end( va_list_regex ) ;
	if( pc_res_tmp != NULL ) {
		free( pc_res_tmp ) ;
	}
	return i_len ;
}


int regexpr_match(char *pc_pattern , char *pc_buff , char *pc_res , int i_len_res )                 //获取一个匹配正则表达式的字符串
{
	int i_status , i_len = 0 ;
	regex_t regex_t_pattern ;
	char ac_error[ LEN_ERROR ] ;
	regmatch_t regmatch_t_res ;

	if ( ( i_status = regcomp( &regex_t_pattern , pc_pattern , REG_EXTENDED | REG_NEWLINE ) ) != REG_NOERROR ) {
		regerror( i_status , &regex_t_pattern , ac_error , LEN_ERROR ) ;
		DEBUG_PRINT( COLOR_MODE_NORMAL , COLOR_BACK_BLACK , COLOR_FRONT_RED , "i_status= %d pc_pattern= '%s' ac_error= '%s'" , i_status , pc_pattern , ac_error )
		return 0 ;
	}

	if ( ( i_status = regexec( &regex_t_pattern , pc_buff , 1 , &regmatch_t_res , 0 ) ) == 0 ) {
		i_len = regmatch_t_res.rm_eo - regmatch_t_res.rm_so ;
		if( i_len >= i_len_res ) {
			i_len = i_len_res - 1 ;
		}
		memcpy( pc_res , pc_buff + regmatch_t_res.rm_so , i_len ) ;
		pc_res[ i_len ] = 0 ;
	}
	regfree( &regex_t_pattern ) ;
	DEBUG_PRINT( COLOR_MODE_NORMAL , COLOR_BACK_BLACK , COLOR_FRONT_GREEN , "pc_pattern= '%s' pc_buff= '%s' pc_res= '%s' %d i_len= %d" , pc_pattern , pc_buff , pc_res , strlen( pc_res ) , i_len )
	return i_len ;
}

int regexpr_replace(char *pc_pattern , char *pc_buff , char *pc_res , int i_len_res , char *pc_replace )  //根据正则表达式替换字符串
{
	int i_status , i_len_replace = 0 , i_len_match = 0 , i_len_now = 0 , i_len_return = 0 ;
	regex_t regex_t_pattern ;
	char ac_error[ LEN_ERROR ] , *pc_now = pc_res ;
	regmatch_t regmatch_t_res ;

	//DEBUG_PRINT( COLOR_MODE_NORMAL , COLOR_BACK_BLACK , COLOR_FRONT_BLUE , "pc_pattern= '%s' %d pc_buff= '%s' %d" , pc_pattern , strlen( pc_pattern ) , pc_buff , strlen( pc_buff ) )
	if ( ( i_status = regcomp( &regex_t_pattern , pc_pattern , REG_EXTENDED | REG_NEWLINE ) ) != REG_NOERROR ) {
		regerror( i_status , &regex_t_pattern , ac_error , LEN_ERROR ) ;
		DEBUG_PRINT( COLOR_MODE_NORMAL , COLOR_BACK_BLACK , COLOR_FRONT_RED , "i_status= %d ac_error= '%s'" , i_status , ac_error )
		return 0 ;
	}

	if( pc_replace != NULL ) {                                                                      //判断是否有replace的字符串
		i_len_replace = strlen( pc_replace ) ;
		DEBUG_PRINT( COLOR_MODE_NORMAL , COLOR_BACK_BLACK , COLOR_FRONT_BLUE , "i_len_replace= %d pc_replace= '%s'" , i_len_replace , pc_replace )
	}

	strncpy( pc_res , pc_buff , i_len_res ) ;                                                       //将源字符串先拷贝到返回结果中去
	i_len_now = strlen( pc_res ) ;
	i_len_return = i_len_now ;
	while ( ( i_status = regexec( &regex_t_pattern , pc_now , 1 , &regmatch_t_res , 0 ) ) == 0 ) {
		if ( ( i_len_match = regmatch_t_res.rm_eo - regmatch_t_res.rm_so ) <= 0 ) {
			break ;
		}
		i_len_return = i_len_return - i_len_match + i_len_replace ;                                    //设置返回的字符串长度
		//DEBUG_PRINT( COLOR_MODE_NORMAL , COLOR_BACK_BLACK , COLOR_FRONT_BLUE , "i_len_match= %d rm_so= %d rm_eo= %d" , i_len_match , regmatch_t_res.rm_so , regmatch_t_res.rm_eo )
		memmove( pc_now + regmatch_t_res.rm_so + i_len_replace  , pc_now + regmatch_t_res.rm_eo , i_len_now - regmatch_t_res.rm_eo ) ;
		if ( i_len_replace != 0 ) {
			memcpy( pc_now + regmatch_t_res.rm_so , pc_replace , i_len_replace ) ;
		}
		pc_now[ i_len_now - i_len_match + i_len_replace ] = 0 ;                                     //设置当前的字符串的终结位置
		pc_now += regmatch_t_res.rm_so + i_len_replace ;
		i_len_now -= regmatch_t_res.rm_eo ;                                                         //剩下没检索的长度
	}
	regfree( &regex_t_pattern ) ;
	//DEBUG_PRINT( COLOR_MODE_NORMAL , COLOR_BACK_BLACK , COLOR_FRONT_GREEN , "pc_pattern= '%s' pc_buff= '%s' pc_res= '%s' %d i_len_now= %d" , pc_pattern , pc_buff , pc_res , strlen( pc_res ) , i_len_now )
	return i_len_return ;
}

int regexpr_compare( regex_t *p_regex_t , char *pc_pattern , char *pc_buff )
{
	int i_status;
	regex_t regex_t_pattern;
	char ac_error[ LEN_ERROR ] ;

	if ( p_regex_t == NULL ) {
		if ( ( i_status = regcomp( &regex_t_pattern , pc_pattern , REG_EXTENDED | REG_NOSUB ) ) != REG_NOERROR ) {
			regerror( i_status , &regex_t_pattern , ac_error , LEN_ERROR ) ;
			DEBUG_PRINT( COLOR_MODE_NORMAL , COLOR_BACK_BLACK , COLOR_FRONT_RED , "i_status= %d ac_error= '%s'" , i_status , ac_error )
			return REG_NOMATCH ;
		}
		i_status = regexec( &regex_t_pattern , pc_buff , (size_t)0 , NULL , 0 ) ;
		regfree( &regex_t_pattern ) ;
	}
	else {
		i_status = regexec( p_regex_t , pc_buff , (size_t)0 , NULL , 0 ) ;
	}
	return i_status ;
}

int regexpr_match_once( regex_t *p_regex_t , char *pc_pattern , char *pc_buff , char *pc_res , int i_len_res )
{
	int i_status , i_num = 0 , i_len = 0 ;
	regex_t regex_t_pattern , *p_regex_t_now ;
	char ac_error[ LEN_ERROR ] ;
	regmatch_t regmatch_t_res ;

	if ( p_regex_t == NULL ) {
		if ( ( i_status = regcomp( &regex_t_pattern , pc_pattern , REG_EXTENDED | REG_NEWLINE ) ) != REG_NOERROR ) {
			regerror( i_status , &regex_t_pattern , ac_error , LEN_ERROR ) ;
			DEBUG_PRINT( COLOR_MODE_NORMAL , COLOR_BACK_BLACK , COLOR_FRONT_RED , "i_status= %d pc_pattern= '%s' ac_error= '%s'" , i_status , pc_pattern , ac_error )
			return 0 ;
		}
		p_regex_t_now = &regex_t_pattern ;
	}
	else {
		p_regex_t_now = p_regex_t ;
	}
	if ( ( i_status = regexec( p_regex_t_now , pc_buff , 1 , &regmatch_t_res , 0 ) ) == 0 ) {
		i_len = regmatch_t_res.rm_eo - regmatch_t_res.rm_so ;
		if( i_len >= i_len_res ) {
			i_len = i_len_res - 1 ;
		}
		memcpy( pc_res , pc_buff + regmatch_t_res.rm_so , i_len ) ;
		pc_res[ i_len ] = 0 ;
	}
	if( p_regex_t_now != p_regex_t ) {
		regfree( p_regex_t_now ) ;
	}
	return i_len ;
}

struct regres *regexpr_list( regex_t *p_regex_t , char *pc_pattern , char *pc_buff , int *pi_res_num )
{
	int i_status , i_len ;
	regex_t regex_t_pattern , *p_regex_t_now ;
	char ac_error[ LEN_ERROR ] , *pc_now = pc_buff , c_init = PROG_FALSE ;
	regmatch_t regmatch_t_res ;
	struct regres *p_regres_now = NULL , *p_regres_head = NULL , *p_regres_prev = NULL ;

	*pi_res_num = 0 ;
	if ( p_regex_t == NULL ) {
		if ( ( i_status = regcomp( &regex_t_pattern , pc_pattern , REG_EXTENDED ) ) != REG_NOERROR ) {
			regerror( i_status , &regex_t_pattern , ac_error , LEN_ERROR ) ;
			DEBUG_PRINT( COLOR_MODE_NORMAL , COLOR_BACK_BLACK , COLOR_FRONT_RED , "i_status= %d ac_error= '%s'" , i_status , ac_error )
			return NULL ;
		}
		p_regex_t_now = &regex_t_pattern ;
	}
	else {
		p_regex_t_now = p_regex_t ;
	}
	while ( ( i_status = regexec( p_regex_t_now , pc_now , 1 , &regmatch_t_res , 0 ) ) == 0 ) {
		i_len = regmatch_t_res.rm_eo - regmatch_t_res.rm_so ;
		p_regres_now = ( struct regres * )calloc( 1 , sizeof( struct regres ) ) ;
		p_regres_now->pc_data = ( char * )calloc( 1 , i_len + 1 ) ;
		memcpy( p_regres_now->pc_data , pc_now + regmatch_t_res.rm_so , i_len ) ;

		DEBUG_PRINT( COLOR_MODE_NORMAL , COLOR_BACK_BLACK , COLOR_FRONT_BLUE ,
		             "i_len= %d regmatch_t_res.rm_so= %d regmatch_t_res.rm_eo= %d regmatch_t_res= '%s' p_regres_now->pc_data= '%s'" ,
		             i_len , regmatch_t_res.rm_so , regmatch_t_res.rm_eo , pc_now + regmatch_t_res.rm_so , p_regres_now->pc_data )

		if( c_init == PROG_FALSE ) {
			p_regres_head = p_regres_now ;
			p_regres_prev = p_regres_now ;
			c_init = PROG_TRUE ;
		}
		else {
			p_regres_prev->p_regres_next = p_regres_now ;
			p_regres_prev = p_regres_now ;
		}
		pc_now += regmatch_t_res.rm_eo ;
		*pi_res_num += 1 ;
	}
	if( p_regex_t_now != p_regex_t ) {
		regfree( p_regex_t_now ) ;
	}
	return p_regres_head ;
}

*/