/*
 *	数据类型声明符的再定义.
 * 	NOTE	these typedef come from file Gbl_MacroDef.h
 */

#ifndef _AKTYPES_H_
#define _AKTYPES_H_

/* preliminary type definition for global area */
typedef	unsigned char			T_U8;		/* unsigned 8 bit integer */
typedef	unsigned short			T_U16;		/* unsigned 16 bit integer */
typedef	unsigned long			T_U32;		/* unsigned 32 bit integer */
typedef	signed char			    T_S8;		/* signed 8 bit integer */
typedef	signed short			T_S16;		/* signed 16 bit integer */
typedef	signed long 			T_S32;		/* signed 32 bit integer */
typedef void					T_VOID;		/* void */

/* basal type definition for global area */
typedef T_S8					T_CHR;		/* char */
typedef T_U8					T_BOOL;		/* BOOL type */

typedef T_VOID *				T_pVOID;	/* pointer of void data */
typedef const T_VOID *			T_pCVOID;	/* const pointer of void data */

typedef T_S8 *				T_pSTR;		/* pointer of string */
typedef const T_S8 *			T_pCSTR;	/* const pointer of string */

typedef T_U8 *				T_pDATA;	/* pointer of data */
typedef const T_U8 *			T_pCDATA;	/* const pointer of data */

typedef T_U32					T_COLOR;

//typedef HANDLE					T_HANDLE;			/* a handle */
//typedef LPTHREAD_START_ROUTINE	T_LPTHREAD_START;	/* handle for thread */
typedef T_U32					T_HANDLE;			/* a handle */
typedef	T_pVOID					T_LPTHREAD_START;	/* handle for thread */

#define	AK_FALSE				0
#define	AK_TRUE				1
#define 	AK_NULL				(0)

#ifndef OS_WIN32

#define		VOID 					void
#define		FALSE					0
#define		TRUE					1
#define 	NULL					0

/*								*/
/* Common byte, 16 bit and 32 bit types				*/
/*								*/
	typedef signed char					BYTE;
	typedef short						WORD;
	typedef long						DWORD;

	typedef long						LONG;
	typedef short						SHORT;
	typedef int							COUNT;
	typedef int							BOOL;

	typedef unsigned char				UBYTE;
	typedef unsigned int				UWORD;
	typedef unsigned int				BITS;				/* for use in bit fields(!)	*/
	typedef unsigned int				UCOUNT;
	typedef unsigned long				ULONG;
#endif

typedef unsigned char bool;


#endif	//  _AKTYPES_H_

