/**
 * @filename randomizer.c
 * @brief  the arithmetic  to randomize or derandomize the data
 * Copyright (C) 2006 Anyka (Guangzhou) Software Technology Co., LTD
 * @author yangyiming
 * @date 2012-05-15
 * @version 1.0
 */

#include "anyka_types.h"
#include "randomizer.h"

#define RANDOMIZER_SECTION_SIZE  1024    //1024B as a ecc & randomized section in one page
#define RANDOMIZER_SEED_NUM     8
#define RANDOMIZER_ARR_LEN     2048

static unsigned char *s_rand_array = NULL; 

static unsigned short random_seed_generator(const unsigned short RandomSeed, unsigned long n);

/**
*@brief     initial the randomizer
*
*@author    yangyiming
*@date       2012-05-15
*@param    PageSize[in]  the pagesize of  the nand
*@return    bool true for success, false for failure
*/
bool randomizer_init(unsigned long PageSize)
{
    unsigned long F, B, i;
    //seed#0 2046 clocks,seed#1 4094 clocks---seed#7 16382 clocks lapse form all 1;    
    unsigned short seed[RANDOMIZER_SEED_NUM] = 
            {0x3989, 0x38e4, 0xe9c, 0x12d5, 0x3bd5, 0x2d30, 0xa29, 0x2cd7};
    unsigned short SeedNext[RANDOMIZER_SEED_NUM];
    unsigned char SeedLsb;
    unsigned char result_value;
    unsigned long Frame_num;

    Frame_num = PageSize / RANDOMIZER_SECTION_SIZE;

    s_rand_array = (unsigned char *)MALLOC(RANDOMIZER_ARR_LEN * Frame_num);
    
    if (NULL == s_rand_array) 
    {
        return false;
    }
    
    for (F = 0; F < Frame_num; F++)
    {
        for (i = 0; i < RANDOMIZER_SEED_NUM; i++)
        {
            SeedNext[i] = seed[i];
        }
        
        for (B = 0; B < RANDOMIZER_ARR_LEN; B++)
        {
            result_value = 0;
            for(i = 0; i < RANDOMIZER_SEED_NUM; i++)
            {
                SeedNext[i] = random_seed_generator(SeedNext[i], 1);
                SeedLsb = SeedNext[i] & 0x1;
                result_value |= SeedLsb << ((i + F) % RANDOMIZER_SEED_NUM);
            }
            s_rand_array[F * RANDOMIZER_ARR_LEN + B] = result_value;
        }
    }
    
    return true;
}

/**
*@brief     randomize the data or derandomize the randomized data
*
*@author    yangyiming
*@date       2012-05-15
*@param    pageIndex[in]   the offset of the page where the data got from /will be stored in the nand
*@param    EccFrameIndex[in] which ecc frame in a page
*@param    destBuff[out]  the  buffer to store the data to be converted 
*@param    srcBuff[in]      the buffer stored the data to be converted
*@param    datalen[in]      the data length ,unit byte
*@return    void
*/
void randomize(unsigned long pageIndex, unsigned long EccFrameIndex, unsigned char *destBuff, unsigned char *srcBuff, unsigned long datalen)
{
    unsigned long i;
    unsigned char *pArray = s_rand_array + (pageIndex << 2) + EccFrameIndex * RANDOMIZER_ARR_LEN;//multi pageIndex by 4 for 4bytes aligned
    unsigned long res = ((unsigned long)srcBuff & 0x3) || ((unsigned long)destBuff & 0x3);

    if(0 != res)
    {
        for(i = 0; i < datalen; i++)
        {
            destBuff[i] = srcBuff[i] ^ pArray[i];
        }
    }
    else
    {
        for(i = 0; i < datalen ; i += 4)
        {
            *((unsigned long *)(destBuff + i)) = *((unsigned long *)(srcBuff + i)) ^ *((unsigned long *)(pArray + i));
        }
    }
}

//generator polynomial : X^14 + X^11 + X^9 + X^6 + X^5 + X^2 + 1
static unsigned short random_seed_generator(const unsigned short RandomSeed, unsigned long n)
{
    unsigned long i;
    unsigned short Seed = RandomSeed;
    unsigned char SeedBit;
    
    for (i = 0; i < n; i++)
    {
        SeedBit = Seed & 0x1;
        
        Seed = Seed ^ (SeedBit << 12);
        Seed = Seed ^ (SeedBit << 9);
        Seed = Seed ^ (SeedBit << 8);
        Seed = Seed ^ (SeedBit << 5);
        Seed = Seed ^ (SeedBit << 3);

        Seed = ((Seed >> 1)|(SeedBit << 13));
    }
    
    return Seed;
}

