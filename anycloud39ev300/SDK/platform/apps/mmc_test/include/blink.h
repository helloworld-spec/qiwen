/*
 * @(#)Blink.h
 * @date 2005/12/25
 * @version 1.0
 * @author Zhou Shangpin.
 * Copyright 2005 Anyka corporation, Inc. All rights reserved.
 * ANYKA PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef        _BLINK_H_
#define        _BLINK_H_

typedef struct BLinkItem *T_PBLINKITEM;
typedef struct BLinkItem  T_BLINKITEM;
struct BLinkItem
{
    T_U8 *data;
    T_PBLINKITEM prev;
    T_PBLINKITEM next;
};

typedef struct tag_BLink
{
    T_OBJECT        object;
    T_U32           ItemSize;
    T_PBLINKITEM    head;
    T_PBLINKITEM    tail;
    T_PBLINKITEM    ptr;
}T_BLINK, *T_PBLINK;

//destroy a link.
T_VOID BLink_Destroy(T_PBLINK obj);

//initial object' item.
T_VOID BLink_Initial(T_PBLINK obj, T_U32 ItemSize);

//seek to first.
T_VOID BLink_SetToHead(T_PBLINK obj);

T_VOID BLink_SetToTail(T_PBLINK obj);

//get next item.
T_BOOL    BLink_Next(T_PBLINK obj, T_pVOID item);

T_U8* BLink_NextNoCopy(T_PBLINK obj);

T_U8* BLink_PreNoCopy(T_PBLINK obj);

T_U8* BLink_GetCurrent(T_PBLINK obj);

T_U8* BLink_GetHead(T_PBLINK obj);

T_U8* BLink_GetTail(T_PBLINK obj);

T_U8* BLink_GetNodeData(T_PBLINK obj, T_U32 Position);

T_VOID BLink_ReSetByHead(T_PBLINK obj);

T_VOID BLink_ReSetByTail(T_PBLINK obj);


//T_U8* BLink_GetNodeData(T_PBLINK obj, T_U32 Position);

//get previous item.
T_BOOL    BLink_Prev(T_PBLINK obj, T_pVOID item);

T_BOOL BLink_Delete(T_PBLINK obj);
//insert a item.
T_BOOL    BLink_Insert(T_PBLINK obj, T_pCVOID item);

/* Insert a BLINK into the dest blink. */
T_BOOL BLink_InsertBlink(T_PBLINK blink, T_PBLINK subLink);

T_BOOL BLink_InsertBlinkItem(T_PBLINK obj, T_PBLINKITEM blinkitem);

#endif

