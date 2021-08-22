#ifndef _SSV_CHIP_H_
#define _SSV_CHIP_H_

#if 0

#ifdef SSV_6051Q
#define CHIP_IDENTITY_P1 0x00000000
#define CHIP_IDENTITY_P2 0x70000000
#define CHIP_IDENTITY 0x73000000
#endif

#ifdef SSV_6051Z
#define CHIP_IDENTITY 0x71000000
#endif

#ifdef SSV_6051P
#define CHIP_IDENTITY 0x75000000
#endif

#ifndef CHIP_IDENTITY
#define CHIP_IDENTITY 0x00000000
#endif

#endif

#endif

