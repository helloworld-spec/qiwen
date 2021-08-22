/*
 * @(#)Cluster.h
 * @date 2005/12/25
 * @version 1.0
 * @author Zhou Shangpin.
 * Copyright 2005 Anyka corporation, Inc. All rights reserved.
 * ANYKA PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef        _CLUSTER_H_
#define        _CLUSTER_H_

typedef struct tag_Cluster
{ 
    T_OBJECT    obj;         // Inheriting the base class.
    T_U16   EndFlag;        // all cluster have been loaded
    T_U16   FirstWirteFlag; //the flag of the first wirting
    T_U32   LastPtr;        // Address of the last fragment's address in cluster link.
    T_U32   MaxVcn;         // Max virtual ID of cluster in cluster link.
    T_U32   LastCluster;    // Physical ID of last cluster in cluster link.
    T_U32   MinCluster;     // Minimal physical ID of cluster in cluster link.
    T_U32   MaxCluster;     // Maximal physical ID of cluster in cluster link.
    T_U32   BufLen;         // Data buffer's length of cluster link.
    T_U32   NewVCN;           //it is the first new vcn which wirte file after opening file 
    T_U32   UpdateVCN;        //it have updated vcn cluster 
    T_U32   UseVCN;       //the file have used total clusters
    T_U8*   data;           // Data buffer to save cluster link.
}T_CLUSTER, *T_PCLUSTER;

T_VOID    Cluster_Destroy(T_PCLUSTER obj);
T_PCLUSTER Cluster_Initial(T_U32 BufLen);
T_BOOL Cluster_Add(T_PCLUSTER obj, T_U32 cluster, T_U32 count);
T_U32 Cluster_Find(T_PCLUSTER obj, T_U32 cluster);
T_U32 Cluster_Seek(T_PCLUSTER obj, T_U32 cnid, T_U32 *StartAddr, T_U32 *pos);
T_BOOL Cluster_SortedDel(T_PCLUSTER obj, T_PCLUSTER sub);
T_BOOL Cluster_SortedAdd(T_PCLUSTER obj, T_U32 cluster, T_U32 count);
T_VOID Cluster_Clear(T_PCLUSTER obj);
T_VOID Cluster_Copy(T_PCLUSTER dst, T_PCLUSTER src);
T_U32 Cluster_GetCount(T_PCLUSTER obj, T_U32 vcn, T_U32 *RetCount);
T_BOOL  Cluster_Split(T_PCLUSTER obj, T_U32 vcn, T_PCLUSTER sub);
T_U8 Cluster_GetBlock(const T_U8 *data, T_U32* count, T_U32* RealAddr, T_U32* relative, T_BOOL* normal);
T_BOOL Cluster_CheckSerious(T_PCLUSTER obj);
T_BOOL Cluster_SortedCutBlock(T_PCLUSTER obj, T_U32 cluster, T_U32 len);
#endif


