
/*
MODELNAME:memapi.c
DISCRIPTION:simple memery manage
AUTHOR:WangXi
DATE:2011-02-15
*/


#include "memapi.h"



//==========================================================
#define MEM_ACTIVE		   1
#define MEM_INACTIVE	   0
#define MEM_BLK_SIZE       32
#define MEM_BLK_CNT_MAX    (9*1024)

#ifdef DEBUG_OUTPUT
#define MEM_INFO  printf
#else
#define MEM_INFO
#endif

typedef struct index_Stack
{
	T_U16	stack_Array[MEM_BLK_CNT_MAX];	
	T_U16	top;						
}Index_Stack;

typedef struct mEM_Node_New
{	
	T_VOID*	ptr;				
	T_U32	size;				
	T_U16	Index_nextNode;	
	T_U16	Index_nextfreeNode;
}MEM_Node_New;

typedef struct mEM_System
{	
	MEM_Node_New 	mem_Node_New[MEM_BLK_CNT_MAX];
	Index_Stack		index_Stack;	
	T_U16			Index_freeHead;
	T_U32			mem_using;		
	T_U16			memBlock_num;	
} MEM_System;

static MEM_System *pmem_System = AK_NULL;
static MEM_System  mem_System;
static T_U32  mem_blockcnt = 0;

#if 0
extern T_U8 *_ftext;
extern T_U8 *_etext;
extern T_U8 *_fdata;
extern T_U8 *_edata;
extern T_U8 *_fbss;
extern T_U8 *_bss_start;
extern T_U8 *__end__;
T_VOID address_Test(T_VOID)
{
	
	T_U32 *pData1 = AK_NULL;
	T_U32 *pData2 = AK_NULL;
	T_U32 *pData3 = AK_NULL;
	T_U32 *pData4 = AK_NULL;
	T_U32 *pData5 = AK_NULL;
	T_U32 *pData6 = AK_NULL;
	T_U32 *pData7 = AK_NULL;
	

	pData1 = &_ftext;
	pData2 = &_etext;
	pData3 = &_fdata;
	pData4 = &_edata;
	pData5 = &_fbss;
	pData6 = &_bss_start;
	pData7 = &__end__;
	
	printf("_ftext = %08x %08x \n",    pData1,_ftext);
	printf("_etext = %08x %08x \n",    pData2,_etext);
	printf("_fdata = %08x %08x \n",    pData3,_fdata);
	printf("_edata = %08x %08x \n",    pData4,_edata);
	printf("_fbss = %08x %08x \n",     pData5,_fbss);
	printf("_bss_start = %08x %08x \n",pData6,_bss_start);
	printf("__end__ = %08x %08x \n",   pData7,__end__);
	//while(1);
}
#endif


//==================================================================
T_VOID	Init_MallocMem(T_U32 MallocAddr_Start,T_U32 MallocAddr_End)
{	
	//address_Test();
	//printf("Set Malloc 0x%08x - 0x%08x, size = %d Bytes\n",MallocAddr_Start,MallocAddr_End,MallocAddr_End - MallocAddr_Start);
	//MallocAddr_Start = MallocAddr_Start & (~3);
	//MallocAddr_End   = MallocAddr_End & (~3);
	//printf("Align Malloc 0x%08x - 0x%08x, size = %d Bytes\n",MallocAddr_Start,MallocAddr_End,MallocAddr_End - MallocAddr_Start);
	
	//memset((T_VOID*)MallocAddr_Start,0xff,MallocAddr_End - MallocAddr_Start);
	pmem_System = &mem_System; 
	
	if(AK_NULL == pmem_System)
	{
		MEM_INFO("malloc fail!\n");
		return;
	}
	pmem_System->mem_Node_New[0].ptr                = (T_VOID *)MallocAddr_Start;//+sizeof(MEM_System)+MEM_BLK_SIZE-((T_U32)STR_ADDR+sizeof(MEM_System))%MEM_BLK_SIZE;
	pmem_System->mem_Node_New[0].size               = 0;
	pmem_System->mem_Node_New[0].Index_nextNode     = 1;
	pmem_System->mem_Node_New[0].Index_nextfreeNode = 1;

	pmem_System->mem_Node_New[1].ptr                = (T_VOID *)MallocAddr_End;
	pmem_System->mem_Node_New[1].size               = 0;
	pmem_System->mem_Node_New[1].Index_nextNode     = 0;
	pmem_System->mem_Node_New[1].Index_nextfreeNode = 0;

	pmem_System->Index_freeHead  = 0;
	pmem_System->index_Stack.top = 0;
	pmem_System->mem_using       = 0;
	pmem_System->memBlock_num    = 0;
	
	while(pmem_System->index_Stack.top < MEM_BLK_CNT_MAX-1)
	{
		pmem_System->index_Stack.stack_Array[pmem_System->index_Stack.top] = MEM_BLK_CNT_MAX - pmem_System->index_Stack.top;
		(pmem_System->index_Stack.top)++;
	}
#if 0
	T_U32 i;
	for(i=0;i<2;i++)
	{
		MemTest();
		if(pmem_System->Index_freeHead!=0)
		{
			printf("Test ERROR!Index_freeHead\n");
			break;
		}
		if(pmem_System->mem_Node_New[0].Index_nextNode!=1)
		{
			printf("Test ERROR!Index_nextNode\n");
			break;
		}
		if(pmem_System->mem_Node_New[0].Index_nextfreeNode!=1)
		{
			printf("Test ERROR!Index_nextfreeNode\n");
			break;
		}
		printf("LOOP %d SUCCESS\n",i+1);
	}
	if(2 == i)
	{
		printf("Test SUCESS.\n");
	}
#endif
}

/**
 * @brief  Malloc one memory block from global heap memory
 *
 * @author  WangXi
 * @date	    2010-12-18
 *
 * @param	T_U32 size : want memory size 
 *
 * @return  T_pVOID : memory address for success, AK_NULL for failure 
 *
 */
T_VOID *AllocMem(T_U32 size)
{
	T_U16			Index,Index_tempPre;
	T_BOOL			flagFirstloop;
	MEM_Node_New	*temp = AK_NULL; 
	MEM_Node_New	*temp0 = AK_NULL, *found = AK_NULL;
	T_U32 loop;
	T_U32	freeSize;


	if(0 == size)
	{
		return AK_NULL;
	}
	
	if(0 != (size%MEM_BLK_SIZE))
	{
		size+=(MEM_BLK_SIZE-size%MEM_BLK_SIZE);
	}
	
	loop = 0;
	flagFirstloop = AK_TRUE;
	temp = &(pmem_System->mem_Node_New[pmem_System->Index_freeHead]);
	Index_tempPre=pmem_System->Index_freeHead;
	do
	{
		temp0 = &(pmem_System->mem_Node_New[temp->Index_nextNode]);
		freeSize=((T_U32)temp0->ptr&0x7fffffff) - ((T_U32)temp->ptr&0x7fffffff) - temp->size;
		if(freeSize>= size)
		{
			
			Index = pmem_System->index_Stack.stack_Array[--(pmem_System->index_Stack.top)];
			found = &(pmem_System->mem_Node_New[Index]);
			
			found->ptr = (T_VOID*)((T_U32)(temp->ptr)+temp->size);
			found->size=size;
			
			found->Index_nextNode=temp->Index_nextNode;			
			temp->Index_nextNode=Index;

			pmem_System->memBlock_num++;
			mem_blockcnt++;
			pmem_System->mem_using+=size;

			if(freeSize!=size)
			{
				found->Index_nextfreeNode=temp->Index_nextfreeNode;
				if(flagFirstloop)
				{
					pmem_System->Index_freeHead=Index;
				}
				else
				{
					pmem_System->mem_Node_New[Index_tempPre].Index_nextfreeNode=Index;
				}	
			}
			else
			{
				found->Index_nextfreeNode=0;
				if(flagFirstloop)
				{
					pmem_System->Index_freeHead=temp->Index_nextfreeNode;
				}
				else
				{
					pmem_System->mem_Node_New[Index_tempPre].Index_nextfreeNode=temp->Index_nextfreeNode;
				}
			}
			temp->Index_nextfreeNode=0;
			return	found->ptr;
		}
		
		if(!flagFirstloop)
		{
			Index_tempPre=pmem_System->mem_Node_New[Index_tempPre].Index_nextfreeNode;//a
		}
		else
		{
			flagFirstloop=AK_FALSE;
		}
		temp=&(pmem_System->mem_Node_New[temp->Index_nextfreeNode]);
		if(loop++>pmem_System->memBlock_num)
		{
			MEM_INFO("MEM List ERROR!\n");
			return AK_NULL;
		}
	}while(0 != temp->Index_nextfreeNode);

	return AK_NULL;
}

/**
 * @brief  Free one memory block to global heap memory
 *
 * @author  WangXi
 * @date	    2010-12-18
 *
 * @param	T_pVOID ptr : memory start address alloced before 
 *
 * @return  T_VOID 
 *
 */
T_VOID FreeMem(T_VOID* ptr)
{
	MEM_Node_New	*temp;
	MEM_Node_New	*temp0;
	T_U16			loop,Index_temp;
	T_U16			Index_tempPre;
	T_BOOL			flagBefor;

	if(AK_NULL == ptr)
	{
		return;
	}
	flagBefor  = AK_FALSE;
	Index_temp = 0;
	loop = 0;
	Index_tempPre = pmem_System->Index_freeHead;
	temp = &(pmem_System->mem_Node_New[0]);
	do
	{
		temp0=&(pmem_System->mem_Node_New[temp->Index_nextNode]);

		if(Index_temp==pmem_System->Index_freeHead)
		{
			flagBefor = AK_TRUE;
		}
		
		if(temp0->ptr == ptr)
		{
			if(temp->Index_nextfreeNode!=0)
			{
				if(temp0->Index_nextfreeNode!=0)
				{
					temp->Index_nextfreeNode=temp0->Index_nextfreeNode;
				}
			}
			else
			{
				if(temp0->Index_nextfreeNode!=0)
				{
					if(!flagBefor)
					{
						pmem_System->Index_freeHead=Index_temp;
					}
					else
					{
						pmem_System->mem_Node_New[Index_tempPre].Index_nextfreeNode=Index_temp;
					}
					temp->Index_nextfreeNode=temp0->Index_nextfreeNode;
				}
				else
				{
					if(!flagBefor)
					{
						temp->Index_nextfreeNode=Index_tempPre;
						pmem_System->Index_freeHead=Index_temp;
					}
					else
					{
						temp->Index_nextfreeNode = pmem_System->mem_Node_New[Index_tempPre].Index_nextfreeNode;
						pmem_System->mem_Node_New[Index_tempPre].Index_nextfreeNode=Index_temp;
					}
				}
			}

			pmem_System->index_Stack.stack_Array[pmem_System->index_Stack.top++] = temp->Index_nextNode;
			temp->Index_nextNode=temp0->Index_nextNode;
			pmem_System->memBlock_num--;
			pmem_System->mem_using-=temp0->size;
			mem_blockcnt--;

			return;
		}


		if(temp->Index_nextfreeNode!=0)
		{
			Index_tempPre=Index_temp;
		}
		Index_temp=temp->Index_nextNode;
		temp=temp0;

		if(loop++>pmem_System->memBlock_num)
		{
			return;
		}
	}while(0 != temp->Index_nextNode);
	
	MEM_INFO("FreeMem ERROR:The pointer to free can't be found!\n");
}


