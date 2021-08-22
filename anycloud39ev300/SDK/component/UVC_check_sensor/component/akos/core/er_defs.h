
#ifndef ER_DEFS_H
#define ER_DEFS_H

#ifdef NU_DEBUG_MEMORY

/* NU_DEBUG_MEMORY can only service one memory pool each time it is
   compiled.  It will examine the memory pool NU_DEBUG_POOL points to.*/
#define NU_DEBUG_POOL System_Memory

typedef struct ER_DEBUG_ALLOCATION_STRUCT
{

    /* prev is the link needed to maintain a linked list of all the 
       ER_DEBUG_ALLOCATION structures.  The head of the list is the global
       variable ERD_RecentAllocation. */
    struct ER_DEBUG_ALLOCATION_STRUCT *prev;
    /* size is the number of bytes used for the users memory allocation */
    unsigned int size;
    /* Assignes each allocation an unique ID */
    unsigned long AllocSequenceCounter;
    /* line and file refer to the place in the code where the call to the
       allocation is made in the application.  These variables are filled
       in with compiler specific macros. */
    unsigned long line;
    const char * file; 
    /* head and foot contain the non-null terminated strings "HEAD" and 
       "FOOT" so this module can spot some instances where pointers write
       to memory locations beyond thier bounds. data is the user's data
       which the allocation call is intended.  */
    unsigned char head[4];
    unsigned char data[1];

} ER_DEBUG_ALLOCATION;

#endif /* NU_DEBUG_MEMORY */

#endif /* ER_DEFS_H */

