//#define DEBUG

#include "sha1.h"
#include "defs.h"
#include "api.h"

int sha1_vector(size_t num_elem, const u8 *addr[], const size_t *len, u8 *mac);

#define REGISTER_ID ((char*)REGISTER_ID_ADDR)

void SHA1Init(SHA1_CTX* context)
{
	/* SHA1 initialization constants */
	context->state[0] = 0x67452301;
	context->state[1] = 0xEFCDAB89;
	context->state[2] = 0x98BADCFE;
	context->state[3] = 0x10325476;
	context->state[4] = 0xC3D2E1F0;
	context->count[0] = context->count[1] = 0;
}

void SHA1Transform(u32 state[5], const unsigned char buffer[64])
{
	u32 a, b, c, d, e;
	typedef union {
		unsigned char c[64];
		u32 l[16];
	} CHAR64LONG16;
	CHAR64LONG16* block;
#ifdef SHA1HANDSOFF
	CHAR64LONG16 workspace;
	block = &workspace;
	os_memcpy(block, buffer, 64);
#else
	block = (CHAR64LONG16 *) buffer;
#endif
	/* Copy context->state[] to working vars */
	a = state[0];
	b = state[1];
	c = state[2];
	d = state[3];
	e = state[4];
	/* 4 rounds of 20 operations each. Loop unrolled. */
	R0(a,b,c,d,e, 0); R0(e,a,b,c,d, 1); R0(d,e,a,b,c, 2); R0(c,d,e,a,b, 3);
	R0(b,c,d,e,a, 4); R0(a,b,c,d,e, 5); R0(e,a,b,c,d, 6); R0(d,e,a,b,c, 7);
	R0(c,d,e,a,b, 8); R0(b,c,d,e,a, 9); R0(a,b,c,d,e,10); R0(e,a,b,c,d,11);
	R0(d,e,a,b,c,12); R0(c,d,e,a,b,13); R0(b,c,d,e,a,14); R0(a,b,c,d,e,15);
	R1(e,a,b,c,d,16); R1(d,e,a,b,c,17); R1(c,d,e,a,b,18); R1(b,c,d,e,a,19);
	R2(a,b,c,d,e,20); R2(e,a,b,c,d,21); R2(d,e,a,b,c,22); R2(c,d,e,a,b,23);
	R2(b,c,d,e,a,24); R2(a,b,c,d,e,25); R2(e,a,b,c,d,26); R2(d,e,a,b,c,27);
	R2(c,d,e,a,b,28); R2(b,c,d,e,a,29); R2(a,b,c,d,e,30); R2(e,a,b,c,d,31);
	R2(d,e,a,b,c,32); R2(c,d,e,a,b,33); R2(b,c,d,e,a,34); R2(a,b,c,d,e,35);
	R2(e,a,b,c,d,36); R2(d,e,a,b,c,37); R2(c,d,e,a,b,38); R2(b,c,d,e,a,39);
	R3(a,b,c,d,e,40); R3(e,a,b,c,d,41); R3(d,e,a,b,c,42); R3(c,d,e,a,b,43);
	R3(b,c,d,e,a,44); R3(a,b,c,d,e,45); R3(e,a,b,c,d,46); R3(d,e,a,b,c,47);
	R3(c,d,e,a,b,48); R3(b,c,d,e,a,49); R3(a,b,c,d,e,50); R3(e,a,b,c,d,51);
	R3(d,e,a,b,c,52); R3(c,d,e,a,b,53); R3(b,c,d,e,a,54); R3(a,b,c,d,e,55);
	R3(e,a,b,c,d,56); R3(d,e,a,b,c,57); R3(c,d,e,a,b,58); R3(b,c,d,e,a,59);
	R4(a,b,c,d,e,60); R4(e,a,b,c,d,61); R4(d,e,a,b,c,62); R4(c,d,e,a,b,63);
	R4(b,c,d,e,a,64); R4(a,b,c,d,e,65); R4(e,a,b,c,d,66); R4(d,e,a,b,c,67);
	R4(c,d,e,a,b,68); R4(b,c,d,e,a,69); R4(a,b,c,d,e,70); R4(e,a,b,c,d,71);
	R4(d,e,a,b,c,72); R4(c,d,e,a,b,73); R4(b,c,d,e,a,74); R4(a,b,c,d,e,75);
	R4(e,a,b,c,d,76); R4(d,e,a,b,c,77); R4(c,d,e,a,b,78); R4(b,c,d,e,a,79);
	/* Add the working vars back into context.state[] */
	state[0] += a;
	state[1] += b;
	state[2] += c;
	state[3] += d;
	state[4] += e;
	/* Wipe variables */
	a = b = c = d = e = 0;
#ifdef SHA1HANDSOFF
	os_memset(block, 0, 64);
#endif
}

void SHA1Update(SHA1_CTX* context, const void *_data, u32 len)
{
	u32 i, j;
	const unsigned char *data = (const unsigned char *)_data;

	j = (context->count[0] >> 3) & 63;
	if ((context->count[0] += len << 3) < (len << 3))
		context->count[1]++;
	context->count[1] += (len >> 29);
	if ((j + len) > 63) {
		os_memcpy(&context->buffer[j], data, (i = 64-j));
		SHA1Transform(context->state, context->buffer);
		for ( ; i + 63 < len; i += 64) {
			SHA1Transform(context->state, &data[i]);
		}
		j = 0;
	}
	else i = 0;
	os_memcpy(&context->buffer[j], &data[i], len - i);
}

void SHA1Final(unsigned char digest[20], SHA1_CTX* context)
{
	u32 i;
	unsigned char finalcount[8];

	for (i = 0; i < 8; i++) {
		finalcount[i] = (unsigned char)
			((context->count[(i >= 4 ? 0 : 1)] >>
			  ((3-(i & 3)) * 8) ) & 255);  /* Endian independent */
	}
	SHA1Update(context, (unsigned char *) "\200", 1);
	while ((context->count[0] & 504) != 448) {
		SHA1Update(context, (unsigned char *) "\0", 1);
	}
	SHA1Update(context, finalcount, 8);  /* Should cause a SHA1Transform()
					      */
	for (i = 0; i < 20; i++) {
		digest[i] = (unsigned char)
			((context->state[i >> 2] >> ((3 - (i & 3)) * 8)) &
			 255);
	}
	/* Wipe variables */
	i = 0;
	os_memset(context->buffer, 0, 64);
	os_memset(context->state, 0, 20);
	os_memset(context->count, 0, 8);
	os_memset(finalcount, 0, 8);
}
/*
int sha1_vector(size_t num_elem, const u8 *addr[], const size_t *len, u8 *mac)
{
	SHA1_CTX ctx;
	size_t i;

	SHA1Init(&ctx);
	for (i = 0; i < num_elem; i++)
		SHA1Update(&ctx, addr[i], len[i]);
	SHA1Final(mac, &ctx);
	return 0;
}*/

int hmac_sha1_vector(const u8 *key, size_t key_len, size_t num_elem,
		     const u8 *addr[], const size_t *len, u8 *mac)
{
	unsigned char k_pad[64]; /* padding - key XORd with ipad/opad */
	unsigned char tk[20];
	const u8 *_addr[6];
	size_t _len[6], i;
	int ret;

	if (num_elem > 5) {
		/*
		 * Fixed limit on the number of fragments to avoid having to
		 * allocate memory (which could fail).
		 */
		return -1;
	}

        /* if key is longer than 64 bytes reset it to key = SHA1(key) */
        if (key_len > 64) {
		if (sha1_vector(1, &key, &key_len, tk))
			return -1;
		key = tk;
		key_len = 20;
        }

	/* the HMAC_SHA1 transform looks like:
	 *
	 * SHA1(K XOR opad, SHA1(K XOR ipad, text))
	 *
	 * where K is an n byte key
	 * ipad is the byte 0x36 repeated 64 times
	 * opad is the byte 0x5c repeated 64 times
	 * and text is the data being protected */

	/* start out by storing key in ipad */
	os_memset(k_pad, 0, sizeof(k_pad));
	os_memcpy(k_pad, key, key_len);
	/* XOR key with ipad values */
	for (i = 0; i < 64; i++)
		k_pad[i] ^= 0x36;

	/* perform inner SHA1 */
	_addr[0] = k_pad;
	_len[0] = 64;
	for (i = 0; i < num_elem; i++) {
		_addr[i + 1] = addr[i];
		_len[i + 1] = len[i];
	}
	if (sha1_vector(1 + num_elem, _addr, _len, mac))
		return -1;

	os_memset(k_pad, 0, sizeof(k_pad));
	os_memcpy(k_pad, key, key_len);
	/* XOR key with opad values */
	for (i = 0; i < 64; i++)
		k_pad[i] ^= 0x5c;

	/* perform outer SHA1 */
	_addr[0] = k_pad;
	_len[0] = 64;
	_addr[1] = mac;
	_len[1] = SHA1_MAC_LEN;

	ret = sha1_vector(2, _addr, _len, mac);

	return ret;
}

int hmac_sha1(const u8 *key, size_t key_len, const u8 *data, size_t data_len,
	       u8 *mac)
{
	return hmac_sha1_vector(key, key_len, 1, &data, &data_len, mac);
}


int sha1_prf(const u8 *key, size_t key_len, const char *label,
	     const u8 *data, size_t data_len, u8 *buf, size_t buf_len)
{
	u8 counter = 0;
	size_t pos, plen;
	u8 hash[SHA1_MAC_LEN];
	size_t label_len = os_strlen(label) + 1;
	const unsigned char *addr[3];
	size_t len[3];

	addr[0] = (u8 *) label;
	len[0] = label_len;
	addr[1] = data;
	len[1] = data_len;
	addr[2] = &counter;
	len[2] = 1;

	pos = 0;
	while (pos < buf_len) {
		plen = buf_len - pos;
		if (plen >= SHA1_MAC_LEN) {
			if (hmac_sha1_vector(key, key_len, 3, addr, len,
					     &buf[pos]))
				return -1;
			pos += SHA1_MAC_LEN;
		} else {
			if (hmac_sha1_vector(key, key_len, 3, addr, len,
					     hash))
				return -1;
			os_memcpy(&buf[pos], hash, plen);
			break;
		}
		counter++;
	}

	return 0;
}


static int pbkdf2_sha1_f(const char *passphrase, const char *ssid,
			 size_t ssid_len, int iterations, unsigned int count,
			 u8 *digest)
{
	unsigned char tmp[SHA1_MAC_LEN], tmp2[SHA1_MAC_LEN];
	int i, j;
	unsigned char count_buf[4];
	const u8 *addr[2];
	size_t len[2];
	size_t passphrase_len = os_strlen(passphrase);

	addr[0] = (u8 *) ssid;
	len[0] = ssid_len;
	addr[1] = count_buf;
	len[1] = 4;

	/* F(P, S, c, i) = U1 xor U2 xor ... Uc
	 * U1 = PRF(P, S || i)
	 * U2 = PRF(P, U1)
	 * Uc = PRF(P, Uc-1)
	 */

	count_buf[0] = (count >> 24) & 0xff;
	count_buf[1] = (count >> 16) & 0xff;
	count_buf[2] = (count >> 8) & 0xff;
	count_buf[3] = count & 0xff;
	if (hmac_sha1_vector((u8 *) passphrase, passphrase_len, 2, addr, len,
			     tmp))
		return -1;
	os_memcpy(digest, tmp, SHA1_MAC_LEN);

	for (i = 1; i < iterations; i++) {
		if (hmac_sha1((u8 *) passphrase, passphrase_len, tmp,
			      SHA1_MAC_LEN, tmp2))
			return -1;
		os_memcpy(tmp, tmp2, SHA1_MAC_LEN);
		for (j = 0; j < SHA1_MAC_LEN; j++)
			digest[j] ^= tmp2[j];
	}

	return 0;
}

void sha1_starts( sha1_context *ctx )
{ 
	ctx->total[0] = 0;  ctx->total[1] = 0;  
	ctx->state[0] = 0x67452301;  
	ctx->state[1] = 0xEFCDAB89;  
	ctx->state[2] = 0x98BADCFE;  
	ctx->state[3] = 0x10325476;  
	ctx->state[4] = 0xC3D2E1F0;
}

void sha1_process( sha1_context *ctx,const uint8 data[64] )
{  
	uint32 temp, W[16], A, B, C, D, E;  
	GET_UINT32( W[0], data, 0 );  
	GET_UINT32( W[1], data, 4 );  
	GET_UINT32( W[2], data, 8 );  
	GET_UINT32( W[3], data, 12 );  
	GET_UINT32( W[4], data, 16 );  
	GET_UINT32( W[5], data, 20 );  
	GET_UINT32( W[6], data, 24 );  
	GET_UINT32( W[7], data, 28 );  
	GET_UINT32( W[8], data, 32 );  
	GET_UINT32( W[9], data, 36 );  
	GET_UINT32( W[10], data, 40 );  
	GET_UINT32( W[11], data, 44 );  
	GET_UINT32( W[12], data, 48 );  
	GET_UINT32( W[13], data, 52 );  
	GET_UINT32( W[14], data, 56 );  
	GET_UINT32( W[15], data, 60 );
#define S(x,n) ((x << n) | ((x & 0xFFFFFFFF) >> (32 - n)))

#define R(t)                             \
	(                                     \
	temp = W[(t - 3) & 0x0F] ^ W[(t - 8) & 0x0F] ^   \
		W[(t - 14) & 0x0F] ^ W[ t     & 0x0F],     \
		( W[t & 0x0F] = S(temp,1) )                 \
	)

#define P(a,b,c,d,e,x)                       \
	{                                     \
	e += S(a,5) + F(b,c,d) + K + x; b = S(b,30);     \
	}

	A = ctx->state[0];  
	B = ctx->state[1];  
	C = ctx->state[2];  
	D = ctx->state[3];  
	E = ctx->state[4];
#define F(x,y,z) (z ^ (x & (y ^ z)))

#define K 0x5A827999  
	P( A, B, C, D, E, W[0] );  P( E, A, B, C, D, W[1] );  P( D, E, A, B, C, W[2] );  P( C, D, E, A, B, W[3] );  P( B, C, D, E, A, W[4] );  P( A, B, C, D, E, W[5] );  P( E, A, B, C, D, W[6] );  P( D, E, A, B, C, W[7] );  P( C, D, E, A, B, W[8] );  P( B, C, D, E, A, W[9] );  P( A, B, C, D, E, W[10] );  P( E, A, B, C, D, W[11] );  P( D, E, A, B, C, W[12] );  P( C, D, E, A, B, W[13] );  P( B, C, D, E, A, W[14] );  P( A, B, C, D, E, W[15] );  P( E, A, B, C, D, R(16) );  P( D, E, A, B, C, R(17) );  P( C, D, E, A, B, R(18) );  P( B, C, D, E, A, R(19) );
#undef K
#undef F
#define F(x,y,z) (x ^ y ^ z)	
#define K 0x6ED9EBA1	
	P( A, B, C, D, E, R(20) );  P( E, A, B, C, D, R(21) );  P( D, E, A, B, C, R(22) );  P( C, D, E, A, B, R(23) );  P( B, C, D, E, A, R(24) );  P( A, B, C, D, E, R(25) );  P( E, A, B, C, D, R(26) );  P( D, E, A, B, C, R(27) );  P( C, D, E, A, B, R(28) );  P( B, C, D, E, A, R(29) );  P( A, B, C, D, E, R(30) );  P( E, A, B, C, D, R(31) );  P( D, E, A, B, C, R(32) );  P( C, D, E, A, B, R(33) );  P( B, C, D, E, A, R(34) );  P( A, B, C, D, E, R(35) );  P( E, A, B, C, D, R(36) );  P( D, E, A, B, C, R(37) );  P( C, D, E, A, B, R(38) );  P( B, C, D, E, A, R(39) );
#undef K
#undef F	
#define F(x,y,z) ((x & y) | (z & (x | y)))	
#define K 0x8F1BBCDC
	P( A, B, C, D, E, R(40) );  P( E, A, B, C, D, R(41) );  P( D, E, A, B, C, R(42) );  P( C, D, E, A, B, R(43) );  P( B, C, D, E, A, R(44) );  P( A, B, C, D, E, R(45) );  P( E, A, B, C, D, R(46) );  P( D, E, A, B, C, R(47) );  P( C, D, E, A, B, R(48) );  P( B, C, D, E, A, R(49) );  P( A, B, C, D, E, R(50) );  P( E, A, B, C, D, R(51) );  P( D, E, A, B, C, R(52) );  P( C, D, E, A, B, R(53) );  P( B, C, D, E, A, R(54) );  P( A, B, C, D, E, R(55) );  P( E, A, B, C, D, R(56) );  P( D, E, A, B, C, R(57) );  P( C, D, E, A, B, R(58) );  P( B, C, D, E, A, R(59) );
#undef K
#undef F
	
#define F(x,y,z) (x ^ y ^ z)
#define K 0xCA62C1D6
P( A, B, C, D, E, R(60) );  P( E, A, B, C, D, R(61) );  P( D, E, A, B, C, R(62) );  P( C, D, E, A, B, R(63) );  P( B, C, D, E, A, R(64) );  P( A, B, C, D, E, R(65) );  P( E, A, B, C, D, R(66) );  P( D, E, A, B, C, R(67) );  P( C, D, E, A, B, R(68) );  P( B, C, D, E, A, R(69) );  P( A, B, C, D, E, R(70) );  P( E, A, B, C, D, R(71) );  P( D, E, A, B, C, R(72) );  P( C, D, E, A, B, R(73) );  P( B, C, D, E, A, R(74) );  P( A, B, C, D, E, R(75) );  P( E, A, B, C, D, R(76) );  P( D, E, A, B, C, R(77) );  P( C, D, E, A, B, R(78) );  P( B, C, D, E, A, R(79) );
#undef K
#undef F
ctx->state[0] += A;  ctx->state[1] += B;  ctx->state[2] += C;  ctx->state[3] += D;  ctx->state[4] += E;
}


void sha1_update( sha1_context *ctx,const uint8 *input, uint32 length )
{  
	uint32 left, fill;  
	if( ! length ) 
		return;  
	left = ctx->total[0] & 0x3F;  
	fill = 64 - left;  
	ctx->total[0] += length;  
	ctx->total[0] &= 0xFFFFFFFF;  
	if( ctx->total[0] < length )    
		ctx->total[1]++;  
	if( left && length >= fill )  
	{    
		memcpy( (void *) (ctx->buffer + left),          
			(void *) input, fill );    
		sha1_process( ctx, ctx->buffer );    
		length -= fill;    
		input += fill;    
		left = 0;  
	}  
	while( length >= 64 )  
	{    
		sha1_process( ctx, input );    
		length -= 64;    
		input += 64;  
	}  
	if( length )  
	{    
		memcpy( (void *) (ctx->buffer + left),          (void *) input, length );  
	}
}

const uint8 sha1_padding[64] ={0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

void sha1_finish( sha1_context *ctx, uint8 digest[20] )
{  
	uint32 last, padn;  
	uint32 high, low;  
	uint8 msglen[8];  
	high = ( ctx->total[0] >> 29 )      | ( ctx->total[1] << 3 );  
	low = ( ctx->total[0] << 3 );  
	PUT_UINT32( high, msglen, 0 );  
	PUT_UINT32( low, msglen, 4 );  
	last = ctx->total[0] & 0x3F;  
	padn = ( last < 56 ) ? ( 56 - last ) : ( 120 - last );  
	sha1_update( ctx, sha1_padding, padn );  
	sha1_update( ctx, msglen, 8 );  
	PUT_UINT32( ctx->state[0], digest, 0 );  
	PUT_UINT32( ctx->state[1], digest, 4 );  
	PUT_UINT32( ctx->state[2], digest, 8 );  
	PUT_UINT32( ctx->state[3], digest, 12 );  
	PUT_UINT32( ctx->state[4], digest, 16 );
}

int sha1_vector(size_t num_elem, const u8 *addr[], const size_t *len, u8 *mac)
{
	size_t i;
	sha1_context ctx;
	sha1_starts(&ctx);
	for (i = 0; i < num_elem; i++)
		sha1_update(&ctx, (unsigned char*)addr[i], len[i]);
	sha1_finish(&ctx, mac);

//	p_hex("openssl_digest_vector:",mac,20);
	
	return 0;
}

const unsigned char PADDING[64] = {
0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

/* F, G, H and I are basic MD5 functions.
*/
#define F(x, y, z) (((x) & (y)) | ((~x) & (z)))
#define G(x, y, z) (((x) & (z)) | ((y) & (~z)))
#define H(x, y, z) ((x) ^ (y) ^ (z))
#define I(x, y, z) ((y) ^ ((x) | (~z)))

/* ROTATE_LEFT rotates x left n bits.
*/
#define ROTATE_LEFT(x, n) (((x) << (n)) | ((x) >> (32-(n))))

/* FF, GG, HH, and II transformations for rounds 1, 2, 3, and 4.
Rotation is separate from addition to prevent recomputation.
*/
#define FF(a, b, c, d, x, s, ac) { \
(a) += F((b), (c), (d)) + (x) + (UINT4)(ac); \
(a) = ROTATE_LEFT ((a), (s)); \
(a) += (b); \
}
#define GG(a, b, c, d, x, s, ac) { \
(a) += G ((b), (c), (d)) + (x) + (UINT4)(ac); \
(a) = ROTATE_LEFT ((a), (s)); \
(a) += (b); \
}
#define HH(a, b, c, d, x, s, ac) { \
(a) += H ((b), (c), (d)) + (x) + (UINT4)(ac); \
(a) = ROTATE_LEFT ((a), (s)); \
(a) += (b); \
}
#define II(a, b, c, d, x, s, ac) { \
(a) += I ((b), (c), (d)) + (x) + (UINT4)(ac); \
(a) = ROTATE_LEFT ((a), (s)); \
(a) += (b); \
}


void Encode (unsigned char *output,UINT4 *input,unsigned int len)
{
  unsigned int i, j;

  for (i = 0, j = 0; j < len; i++, j += 4) 
  {
 output[j] = (unsigned char)(input[i] & 0xff);
 output[j+1] = (unsigned char)((input[i] >> 8) & 0xff);
 output[j+2] = (unsigned char)((input[i] >> 16) & 0xff);
 output[j+3] = (unsigned char)((input[i] >> 24) & 0xff);
  }
}


/* Decodes input (unsigned char) into output (UINT4). Assumes len is
  a multiple of 4.
 */
void Decode (UINT4 *output,const unsigned char *input,unsigned int len)
{
  unsigned int i, j;

  for (i = 0, j = 0; j < len; i++, j += 4)
 output[i] = ((UINT4)input[j]) | (((UINT4)input[j+1]) << 8) | (((UINT4)input[j+2]) << 16) | (((UINT4)input[j+3]) << 24);
}

/* Note: Replace "for loop" with standard memcpy if possible. */
void MD5_memcpy (POINTER output,POINTER input,unsigned int len)
{
  unsigned int i;

  for (i = 0; i < len; i++)
   output[i] = input[i];
}

/* Note: Replace "for loop" with standard memset if possible. */
void MD5_memset (POINTER output,int value,unsigned int len)
{
  unsigned int i;

  for (i = 0; i < len; i++)
  ((char *)output)[i] = (char)value;
}

#if 1

void MD5Init (MD5_CTX *context)    /* context */
{
  context->count[0] = context->count[1] = 0;
  context->state[0] = 0x67452301;    /* Load magic initialization constants.*/
  context->state[1] = 0xefcdab89;
  context->state[2] = 0x98badcfe;
  context->state[3] = 0x10325476;
}

void MD5Transform (UINT4 state[4],const unsigned char block[64])
{
 UINT4 a = state[0], b = state[1], c = state[2], d = state[3], x[16];

 Decode (x, block, 64);
  /* Round 1 */
  FF (a, b, c, d, x[ 0], S11, 0xd76aa478); /* 1 */
  FF (d, a, b, c, x[ 1], S12, 0xe8c7b756); /* 2 */
  FF (c, d, a, b, x[ 2], S13, 0x242070db); /* 3 */
  FF (b, c, d, a, x[ 3], S14, 0xc1bdceee); /* 4 */
  FF (a, b, c, d, x[ 4], S11, 0xf57c0faf); /* 5 */
  FF (d, a, b, c, x[ 5], S12, 0x4787c62a); /* 6 */
  FF (c, d, a, b, x[ 6], S13, 0xa8304613); /* 7 */
  FF (b, c, d, a, x[ 7], S14, 0xfd469501); /* 8 */
  FF (a, b, c, d, x[ 8], S11, 0x698098d8); /* 9 */
  FF (d, a, b, c, x[ 9], S12, 0x8b44f7af); /* 10 */
  FF (c, d, a, b, x[10], S13, 0xffff5bb1); /* 11 */
  FF (b, c, d, a, x[11], S14, 0x895cd7be); /* 12 */
  FF (a, b, c, d, x[12], S11, 0x6b901122); /* 13 */
  FF (d, a, b, c, x[13], S12, 0xfd987193); /* 14 */
  FF (c, d, a, b, x[14], S13, 0xa679438e); /* 15 */
  FF (b, c, d, a, x[15], S14, 0x49b40821); /* 16 */
  /* Round 2 */
  GG (a, b, c, d, x[ 1], S21, 0xf61e2562); /* 17 */
  GG (d, a, b, c, x[ 6], S22, 0xc040b340); /* 18 */
  GG (c, d, a, b, x[11], S23, 0x265e5a51); /* 19 */
  GG (b, c, d, a, x[ 0], S24, 0xe9b6c7aa); /* 20 */
  GG (a, b, c, d, x[ 5], S21, 0xd62f105d); /* 21 */
  GG (d, a, b, c, x[10], S22,  0x2441453); /* 22 */
  GG (c, d, a, b, x[15], S23, 0xd8a1e681); /* 23 */
  GG (b, c, d, a, x[ 4], S24, 0xe7d3fbc8); /* 24 */
  GG (a, b, c, d, x[ 9], S21, 0x21e1cde6); /* 25 */
  GG (d, a, b, c, x[14], S22, 0xc33707d6); /* 26 */
  GG (c, d, a, b, x[ 3], S23, 0xf4d50d87); /* 27 */
  GG (b, c, d, a, x[ 8], S24, 0x455a14ed); /* 28 */
  GG (a, b, c, d, x[13], S21, 0xa9e3e905); /* 29 */
  GG (d, a, b, c, x[ 2], S22, 0xfcefa3f8); /* 30 */
  GG (c, d, a, b, x[ 7], S23, 0x676f02d9); /* 31 */
  GG (b, c, d, a, x[12], S24, 0x8d2a4c8a); /* 32 */
  /* Round 3 */
  HH (a, b, c, d, x[ 5], S31, 0xfffa3942); /* 33 */
  HH (d, a, b, c, x[ 8], S32, 0x8771f681); /* 34 */
  HH (c, d, a, b, x[11], S33, 0x6d9d6122); /* 35 */
  HH (b, c, d, a, x[14], S34, 0xfde5380c); /* 36 */
  HH (a, b, c, d, x[ 1], S31, 0xa4beea44); /* 37 */
  HH (d, a, b, c, x[ 4], S32, 0x4bdecfa9); /* 38 */
  HH (c, d, a, b, x[ 7], S33, 0xf6bb4b60); /* 39 */
  HH (b, c, d, a, x[10], S34, 0xbebfbc70); /* 40 */
  HH (a, b, c, d, x[13], S31, 0x289b7ec6); /* 41 */
  HH (d, a, b, c, x[ 0], S32, 0xeaa127fa); /* 42 */
  HH (c, d, a, b, x[ 3], S33, 0xd4ef3085); /* 43 */
  HH (b, c, d, a, x[ 6], S34,  0x4881d05); /* 44 */
  HH (a, b, c, d, x[ 9], S31, 0xd9d4d039); /* 45 */
  HH (d, a, b, c, x[12], S32, 0xe6db99e5); /* 46 */
  HH (c, d, a, b, x[15], S33, 0x1fa27cf8); /* 47 */
  HH (b, c, d, a, x[ 2], S34, 0xc4ac5665); /* 48 */
  /* Round 4 */
  II (a, b, c, d, x[ 0], S41, 0xf4292244); /* 49 */
  II (d, a, b, c, x[ 7], S42, 0x432aff97); /* 50 */
  II (c, d, a, b, x[14], S43, 0xab9423a7); /* 51 */
  II (b, c, d, a, x[ 5], S44, 0xfc93a039); /* 52 */
  II (a, b, c, d, x[12], S41, 0x655b59c3); /* 53 */
  II (d, a, b, c, x[ 3], S42, 0x8f0ccc92); /* 54 */
  II (c, d, a, b, x[10], S43, 0xffeff47d); /* 55 */
  II (b, c, d, a, x[ 1], S44, 0x85845dd1); /* 56 */
  II (a, b, c, d, x[ 8], S41, 0x6fa87e4f); /* 57 */
  II (d, a, b, c, x[15], S42, 0xfe2ce6e0); /* 58 */
  II (c, d, a, b, x[ 6], S43, 0xa3014314); /* 59 */
  II (b, c, d, a, x[13], S44, 0x4e0811a1); /* 60 */
  II (a, b, c, d, x[ 4], S41, 0xf7537e82); /* 61 */
  II (d, a, b, c, x[11], S42, 0xbd3af235); /* 62 */
  II (c, d, a, b, x[ 2], S43, 0x2ad7d2bb); /* 63 */
  II (b, c, d, a, x[ 9], S44, 0xeb86d391); /* 64 */

  state[0] += a;
  state[1] += b;
  state[2] += c;
  state[3] += d;

  /* Zeroize sensitive information.*/
  MD5_memset ((POINTER)x, 0, sizeof (x));
}

void MD5Update (MD5_CTX *context, const unsigned char *input,unsigned int inputLen  )
{
  unsigned int i, index, partLen;
  index = (unsigned int)((context->count[0] >> 3) & 0x3F);   /* Compute number of bytes mod 64 */

  if ( (context->count[0] += ( (UINT4)inputLen << 3) ) < ( (UINT4)inputLen << 3 ) )
  context->count[1]++;
  
  context->count[1] += ((UINT4)inputLen >> 29);
  partLen = 64 - index;

  /* Transform as many times as possible.*/
  if (inputLen >= partLen) 
  {
 MD5_memcpy( (POINTER)&context->buffer[index], (POINTER)input, partLen);
 MD5Transform (context->state, context->buffer);

 for (i = partLen; i + 63 < inputLen; i += 64)
   MD5Transform (context->state, &input[i]);

 index = 0;
  }
  else
  i = 0;
  /* Buffer remaining input */
  MD5_memcpy( (POINTER)&context->buffer[index], (POINTER)&input[i],inputLen-i );
}

void MD5Final (unsigned char digest[16], MD5_CTX *context)
                /* message digest */      /* context */
{
  unsigned char bits[8];
  unsigned int index, padLen;
  
  Encode (bits, context->count, 8); /* Save number of bits */

  /* Pad out to 56 mod 64.*/
  index = (unsigned int)((context->count[0] >> 3) & 0x3f);
  padLen = (index < 56) ? (56 - index) : (120 - index);
  MD5Update (context,(const unsigned char*) PADDING, padLen);

  MD5Update (context, bits, 8);   /* Append length (before padding) */
  Encode (digest, context->state, 16); /* Store state in digest */

  /* Zeroize sensitive information.*/
  MD5_memset ((POINTER)context, 0, sizeof (*context));
} 

#endif


int pbkdf2_sha1(const char *passphrase, const char *ssid, size_t ssid_len,
		int iterations, u8 *buf, size_t buflen)
{
	unsigned int count = 0;
	unsigned char *pos = buf;
	size_t left = buflen, plen;
	unsigned char digest[SHA1_MAC_LEN];

	while (left > 0) {
		count++;
		if (pbkdf2_sha1_f(passphrase, ssid, ssid_len, iterations,
				  count, digest))
			return -1;
		plen = left > SHA1_MAC_LEN ? SHA1_MAC_LEN : left;
		os_memcpy(pos, digest, plen);
		pos += plen;
		left -= plen;
	}

	return 0;
}

int md5_vector(size_t num_elem, const u8 *addr[], const size_t *len, u8 *mac)
{
	MD5_CTX ctx;
	size_t i;

	MD5Init(&ctx);
	for (i = 0; i < num_elem; i++)
		MD5Update(&ctx, (unsigned char*)addr[i], len[i]);
	MD5Final(mac, &ctx);
	return 0;
}



