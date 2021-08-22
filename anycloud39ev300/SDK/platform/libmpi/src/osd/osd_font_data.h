/** 
* brief :get Font data*
* Copyright (C) 2006 Anyka (GuangZhou) Software Technology Co., Ltd.* 
* date: 2016-10-26
* version 1.0
*/

#ifndef __AKFONT_LIB_H__
#define __AKFONT_LIB_H__

#ifdef __cplusplus
extern "C" {
#endif

unsigned char *get_one_font_data(unsigned short code);
int get_file_font_size(void);
void free_font_data(void);

#ifdef __cplusplus
}
#endif
#endif



