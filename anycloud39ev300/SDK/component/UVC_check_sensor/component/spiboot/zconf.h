/* zconf.h -- configuration of the zlib compression library
 * Copyright (C) 1995-1998 Jean-loup Gailly.
 * For conditions of distribution and use, see copyright notice in zlib.h 
 */

/* @(#) $Id$ */

#ifndef _ZCONF_H
#define _ZCONF_H

#ifndef Z_PREFIX
    #define Z_PREFIX
#endif

/*
 * If you *really* need a unique prefix for all types and library functions,
 * compile with -DZ_PREFIX. The "standard" zlib should be compiled without it.
 */
#ifdef Z_PREFIX
#if 1//IMAGELIB_PNG_ENCODE
#  define deflateInit_	spring_anyka_deflateInit_
#  define deflate	    spring_anyka_deflate
#  define deflateEnd	spring_anyka_deflateEnd

#  define deflateInit2_	spring_anyka_deflateInit2_
#  define deflateSetDictionary spring_anyka_deflateSetDictionary
#  define deflateCopy	spring_anyka_deflateCopy
#  define deflateReset	spring_anyka_deflateReset
#  define deflateParams	spring_anyka_deflateParams

#  define deflate_copyright spring_anyka_deflate_copyright

#  define deflateSetHeader  spring_anyka_deflateSetHeader
#  define deflatePrime      spring_anyka_deflatePrime
#  define deflateTune       spring_anyka_deflateTune
#  define deflateBound      spring_anyka_deflateBound
#endif // IMAGELIB_PNG_ENCODE

#  define inflateInit_ 	spring_anyka_inflateInit_
#  define inflateInit2_	spring_anyka_inflateInit2_
#  define inflate	    spring_anyka_inflate
#  define inflateEnd	spring_anyka_inflateEnd
#  define inflateSetDictionary spring_anyka_inflateSetDictionary
#  define inflateSync	spring_anyka_inflateSync
#  define inflateSyncPoint spring_anyka_inflateSyncPoint
#  define inflateReset	spring_anyka_inflateReset
#  define compress	    spring_anyka_compress
#  define compress2	    spring_anyka_compress2
#  define uncompress	spring_anyka_uncompress
#  define adler32	    spring_anyka_adler32
#  define crc32		    spring_anyka_crc32
#  define get_crc_table spring_anyka_get_crc_table

#  define zcalloc       spring_anyka_zcalloc
#  define zcfree        spring_anyka_zfree
#  define z_errmsg      spring_anyka_z_errmsg
#  define zError        spring_anyka_zError
#  define zlibVersion   spring_anyka_zlibVersion
#  define inflate_copyright spring_anyka_inflate_copyright

#  define compressBound     spring_anyka_compressBound
#  define crc32_combine     spring_anyka_crc32_combine
#  define inflate_fast      spring_anyka_inflate_fast
#  define _tr_init          spring_anyka__tr_init
#  define _tr_align         spring_anyka__tr_align
#  define _tr_stored_block  spring_anyka__tr_stored_block
#  define _tr_flush_block   spring_anyka__tr_flush_block
#  define _tr_tally         spring_anyka__tr_tally
#  define _length_code      spring_anyka__length_code
#  define _dist_code        spring_anyka__dist_code

#  define Byte		spring_anyka_Byte
#  define uInt		spring_anyka_uInt
#  define uLong		spring_anyka_uLong
#  define Bytef	    spring_anyka_Bytef
#  define charf		spring_anyka_charf
#  define intf		spring_anyka_intf
#  define uIntf		spring_anyka_uIntf
#  define uLongf	spring_anyka_uLongf
#  define voidpf	spring_anyka_voidpf
#  define voidp		spring_anyka_voidp
#endif

#if (defined(_WIN32) || defined(__WIN32__)) && !defined(WIN32)
#  define WIN32
#endif
#if defined(__GNUC__) || defined(__CC_ARM)|| defined(WIN32) || defined(__386__) || defined(i386)
#  ifndef __32BIT__
#    define __32BIT__
#  endif
#endif
#if defined(__MSDOS__) && !defined(MSDOS)
#  define MSDOS
#endif

/*
 * Compile with -DMAXSEG_64K if the alloc function cannot allocate more
 * than 64k bytes at a time (needed on systems with 16-bit int).
 */
#if defined(MSDOS) && !defined(__32BIT__)
#  define MAXSEG_64K
#endif
#ifdef MSDOS
#  define UNALIGNED_OK
#endif

#if (defined(MSDOS) || defined(_WINDOWS) || defined(WIN32))  && !defined(STDC)
#  define STDC
#endif
#if defined(__STDC__) || defined(__cplusplus) || defined(__OS2__)
#  ifndef STDC
#    define STDC
#  endif
#endif

#ifndef STDC
#  ifndef const /* cannot use !defined(STDC) && !defined(const) on Mac */
#    define const
#  endif
#endif

/* Some Mac compilers merge all .h files incorrectly: */
#if defined(__MWERKS__) || defined(applec) ||defined(THINK_C) ||defined(__SC__)
#  define NO_DUMMY_DECL
#endif

/* Old Borland C incorrectly complains about missing returns: */
#if defined(__BORLANDC__) && (__BORLANDC__ < 0x500)
#  define NEED_DUMMY_RETURN
#endif

/* Maximum value for memLevel in deflateInit2 */
#ifndef MAX_MEM_LEVEL
#  ifdef MAXSEG_64K
#    define MAX_MEM_LEVEL 8
#  else
#    define MAX_MEM_LEVEL 9
#  endif
#endif

/* Maximum value for windowBits in deflateInit2 and inflateInit2.
 * WARNING: reducing MAX_WBITS makes minigzip unable to extract .gz files
 * created by gzip. (Files created by minigzip can still be extracted by
 * gzip.)
 */
#ifndef MAX_WBITS
#  define MAX_WBITS   15 /* 32K LZ77 window */
#endif

/* The memory requirements for deflate are (in bytes):
            (1 << (windowBits+2)) +  (1 << (memLevel+9))
 that is: 128K for windowBits=15  +  128K for memLevel = 8  (default values)
 plus a few kilobytes for small objects. For example, if you want to reduce
 the default memory requirements from 256K to 128K, compile with
     make CFLAGS="-O -DMAX_WBITS=14 -DMAX_MEM_LEVEL=7"
 Of course this will generally degrade compression (there's no free lunch).

   The memory requirements for inflate are (in bytes) 1 << windowBits
 that is, 32K for windowBits=15 (default value) plus a few kilobytes
 for small objects.
*/

                        /* Type declarations */

#ifndef OF /* function prototypes */
#  ifdef STDC
#    define OF(args)  args
#  else
#    define OF(args)  args
#  endif
#endif


/* The following definitions for FAR are needed only for MSDOS mixed
 * model programming (small or medium model with some far allocations).
 * This was tested only with MSC; for other MSDOS compilers you may have
 * to define NO_MEMCPY in zutil.h.  If you don't need the mixed model,
 * just define FAR to be empty.
 */
#if (defined(M_I86SM) || defined(M_I86MM)) && !defined(__32BIT__)
   /* MSC small or medium model */
#  define SMALL_MEDIUM
#  ifdef _MSC_VER
#    define FAR _far
#  else
#    define FAR far
#  endif
#endif
#if defined(__BORLANDC__) && (defined(__SMALL__) || defined(__MEDIUM__))
#  ifndef __32BIT__
#    define SMALL_MEDIUM
#    define FAR _far
#  endif
#endif

/* Compile with -DZLIB_DLL for Windows DLL support */
#if defined(ZLIB_DLL)
#  if defined(_WINDOWS) || defined(WINDOWS)
#    ifdef FAR
#      undef FAR
#    endif
#    include <windows.h>
#    define ZEXPORT  WINAPI
#    ifdef WIN32
#      define ZEXPORTVA  WINAPIV
#    else
#      define ZEXPORTVA  FAR _cdecl _export
#    endif
#  endif
#  if defined (__BORLANDC__)
#    if (__BORLANDC__ >= 0x0500) && defined (WIN32)
#      include <windows.h>
#      define ZEXPORT __declspec(dllexport) WINAPI
#      define ZEXPORTRVA __declspec(dllexport) WINAPIV
#    else
#      if defined (_Windows) && defined (__DLL__)
#        define ZEXPORT _export
#        define ZEXPORTVA _export
#      endif
#    endif
#  endif
#endif

#if defined (__BEOS__)
#  if defined (ZLIB_DLL)
#    define ZEXTERN extern __declspec(dllexport)
#  else
#    define ZEXTERN extern __declspec(dllimport)
#  endif
#endif

#ifndef ZEXPORT
#  define ZEXPORT
#endif
#ifndef ZEXPORTVA
#  define ZEXPORTVA
#endif
#ifndef ZEXTERN
#  define ZEXTERN extern
#endif

#ifndef FAR
#   define FAR
#endif

#if !defined(MACOS) && !defined(TARGET_OS_MAC)
typedef unsigned char  Byte;  /* 8 bits */
#endif
typedef unsigned int   uInt;  /* 16 bits or more */
typedef unsigned long  uLong; /* 32 bits or more */

#ifdef SMALL_MEDIUM
   /* Borland C/C++ and some old MSC versions ignore FAR inside typedef */
#  define Bytef Byte FAR
#else
   typedef Byte  FAR Bytef;
#endif
typedef char  FAR charf;
typedef int   FAR intf;
typedef uInt  FAR uIntf;
typedef uLong FAR uLongf;

#ifdef STDC
   typedef void FAR *voidpf;
   typedef void     *voidp;
#else
   typedef Byte FAR *voidpf;
   typedef Byte     *voidp;
#endif

#ifdef HAVE_UNISTD_H
#  include <sys/types.h> /* for off_t */
#  include <unistd.h>    /* for SEEK_* and off_t */
#  define z_off_t  off_t
#endif
#ifndef SEEK_SET
#  define SEEK_SET        0       /* Seek from beginning of file.  */
#  define SEEK_CUR        1       /* Seek from current position.  */
#  define SEEK_END        2       /* Set file pointer to EOF plus "offset" */
#endif
#ifndef z_off_t
#  define  z_off_t long
#endif

/* MVS linker does not support external names larger than 8 bytes */
#if defined(__MVS__)
#if 1//IMAGELIB_PNG_ENCODE
#   pragma map(deflateInit_,"DEIN")
#   pragma map(deflateInit2_,"DEIN2")
#   pragma map(deflateEnd,"DEEND")
#endif // IMAGELIB_PNG_ENCODE
#   pragma map(inflateInit_,"ININ")
#   pragma map(inflateInit2_,"ININ2")
#   pragma map(inflateEnd,"INEND")
#   pragma map(inflateSync,"INSY")
#   pragma map(inflateSetDictionary,"INSEDI")
#   pragma map(inflate_blocks,"INBL")
#   pragma map(inflate_blocks_new,"INBLNE")
#   pragma map(inflate_blocks_free,"INBLFR")
#   pragma map(inflate_blocks_reset,"INBLRE")
#   pragma map(inflate_codes_free,"INCOFR")
#   pragma map(inflate_codes,"INCO")
#   pragma map(inflate_fast,"INFA")
#   pragma map(inflate_flush,"INFLU")
#   pragma map(inflate_mask,"INMA")
#   pragma map(inflate_set_dictionary,"INSEDI2")
#   pragma map(inflate_copyright,"INCOPY")
#   pragma map(inflate_trees_bits,"INTRBI")
#   pragma map(inflate_trees_dynamic,"INTRDY")
#   pragma map(inflate_trees_fixed,"INTRFI")
#   pragma map(inflate_trees_free,"INTRFR")
#endif

#endif /* _ZCONF_H */
