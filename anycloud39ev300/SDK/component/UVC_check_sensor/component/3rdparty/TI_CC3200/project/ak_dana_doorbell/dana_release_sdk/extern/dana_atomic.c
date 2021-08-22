#include <stdint.h>
#include <stdlib.h>

#include <time.h>
#include "osi.h"


#include "dana_base.h"


extern int32_t dana_atomic(int *ptr, int val)
{
  //TODO
     *ptr +=val;
     return *ptr;
}

extern void dana_sync()
{
   // __sync_synchronize();
}

extern OsiLockObj_t atomic_lock;

static bool trans_id_init = false; 
static int32_t trans_id = 0;
extern int32_t dana_seq_auto()
{
    if (!trans_id_init) {
            srand((uint32_t)time(NULL));
            trans_id = rand()%65535;
            trans_id_init = true;
        }
    
	osi_LockObjLock(&atomic_lock , OSI_WAIT_FOREVER); 
        trans_id++;
	osi_LockObjUnlock(&atomic_lock );
	
    return trans_id;
}


