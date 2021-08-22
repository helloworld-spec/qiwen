#ifndef _SHA1_H
#define _SHA1_H

#include "type.h"

#define SHA1_MAC_LEN 20

#define SHA1HANDSOFF

typedef unsigned char u8;
typedef unsigned int u32;
typedef unsigned int size_t;

struct SHA1Context {
	u32 state[5];
	u32 count[2];
	unsigned char buffer[64];
};
typedef struct SHA1Context SHA1_CTX;

#define MD5_MAC_LEN 16

#define PROTOTYPES 0
typedef unsigned char *POINTER;

/* UINT2 defines a two byte word */
typedef unsigned short int UINT2;

/* UINT4 defines a four byte word */
typedef unsigned long int UINT4;

#define PROTO_LIST(list) ()
#define S11 7
#define S12 12
#define S13 17
#define S14 22
#define S21 5
#define S22 9
#define S23 14
#define S24 20
#define S31 4
#define S32 11
#define S33 16
#define S34 23
#define S41 6
#define S42 10
#define S43 15
#define S44 21

#if 1 
typedef struct {
UINT4 state[4]; /* state (ABCD) */
UINT4 count[2]; /* number of bits, modulo 2^64 (lsb first) */
unsigned char buffer[64]; /* input buffer */
} MD5_CTX;
#endif

typedef struct{  
	uint32 total[2];  
	uint32 state[5];  
	uint8 buffer[64];
}sha1_context;

#define GET_UINT32(n,b,i)       \
{                               \
	(n) = ( (uint32) (b)[(i)] << 24 )     \
	| ( (uint32) (b)[(i) + 1] << 16 )     \
	| ( (uint32) (b)[(i) + 2] << 8 )     \
	| ( (uint32) (b)[(i) + 3]     );     \
}

#define PUT_UINT32(n,b,i)               \
{                               \
	(b)[(i)] = (uint8) ( (n) >> 24 );     \
	(b)[(i) + 1] = (uint8) ( (n) >> 16 );     \
	(b)[(i) + 2] = (uint8) ( (n) >> 8 );     \
	(b)[(i) + 3] = (uint8) ( (n)     );     \
}


#define rol(value, bits) (((value) << (bits)) | ((value) >> (32 - (bits))))

/* blk0() and blk() perform the initial expand. */
/* I got the idea of expanding during the round function from SSLeay */
#define blk0(i) block->l[i]

#define blk(i) (block->l[i & 15] = rol(block->l[(i + 13) & 15] ^ \
	block->l[(i + 8) & 15] ^ block->l[(i + 2) & 15] ^ block->l[i & 15], 1))

/* (R0+R1), R2, R3, R4 are the different operations used in SHA1 */
#define R0(v,w,x,y,z,i) \
	z += ((w & (x ^ y)) ^ y) + blk0(i) + 0x5A827999 + rol(v, 5); \
	w = rol(w, 30);
#define R1(v,w,x,y,z,i) \
	z += ((w & (x ^ y)) ^ y) + blk(i) + 0x5A827999 + rol(v, 5); \
	w = rol(w, 30);
#define R2(v,w,x,y,z,i) \
	z += (w ^ x ^ y) + blk(i) + 0x6ED9EBA1 + rol(v, 5); w = rol(w, 30);
#define R3(v,w,x,y,z,i) \
	z += (((w | x) & y) | (w & x)) + blk(i) + 0x8F1BBCDC + rol(v, 5); \
	w = rol(w, 30);
#define R4(v,w,x,y,z,i) \
	z += (w ^ x ^ y) + blk(i) + 0xCA62C1D6 + rol(v, 5); \
	w=rol(w, 30);

int pbkdf2_sha1(const char *passphrase, const char *ssid, size_t ssid_len,
		int iterations, u8 *buf, size_t buflen);

int md5_vector(size_t num_elem, const u8 *addr[], const size_t *len, u8 *mac);

int sha1_prf(const u8 *key, size_t key_len, const char *label,
	     const u8 *data, size_t data_len, u8 *buf, size_t buf_len);


int hmac_sha1(const u8 *key, size_t key_len, const u8 *data, size_t data_len,
	       u8 *mac);

#endif
