#include <stdlib.h>
#include <time.h>

#include "dana_rand.h"


//大拿随机数API接口， 厂商自行实现

static bool dana_srand = false;

extern int  dana_rand()
{
    if (dana_srand) {
        return rand();
    } else {
        srand((uint32_t)time(NULL));
        dana_srand = true;
        
        return rand();
    }
}
