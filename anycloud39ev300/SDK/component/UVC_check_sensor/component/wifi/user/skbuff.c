/*
 *	Routines having to do with the 'struct sk_buff' memory handlers.
 *
 *	Authors:	Alan Cox <alan@lxorguk.ukuu.org.uk>
 *			Florian La Roche <rzsfl@rz.uni-sb.de>
 *
 *	Fixes:
 *		Alan Cox	:	Fixed the worst of the load
 *					balancer bugs.
 *		Dave Platt	:	Interrupt stacking fix.
 *	Richard Kooijman	:	Timestamp fixes.
 *		Alan Cox	:	Changed buffer format.
 *		Alan Cox	:	destructor hook for AF_UNIX etc.
 *		Linus Torvalds	:	Better skb_clone.
 *		Alan Cox	:	Added skb_copy.
 *		Alan Cox	:	Added all the changed routines Linus
 *					only put in the headers
 *		Ray VanTassle	:	Fixed --skb->lock in free
 *		Alan Cox	:	skb_copy copy arp field
 *		Andi Kleen	:	slabified it.
 *		Robert Olsson	:	Removed skb_head_pool
 *
 *	NOTE:
 *		The __skb_ routines should be called with interrupts
 *	disabled, or you better be *real* sure that the operation is atomic
 *	with respect to whatever list is being frobbed (e.g. via lock_sock()
 *	or via disabling bottom half handlers, etc).
 *
 *	This program is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU General Public License
 *	as published by the Free Software Foundation; either version
 *	2 of the License, or (at your option) any later version.
 */

/*
 *	The functions in this file will not compile correctly with gcc 2.4.x
 */
//#define DEBUG
#include "usr_cfg.h"
#include "xrf_api.h"
#include "mlan.h"
#include "skbuff.h"
#include "netdevice.h"
#include "lwip/arch.h"

void get_page(struct page *page)
{
	p_dbg_enter;
	if (page->addr)
		p_err("addr != 0\n");

	page->addr = (char*)mem_malloc(4096);
	if (!page->addr)
		p_err("no mem\n");
} 

void put_page(struct page *page)
{
	p_dbg_enter;
	if (!page->addr)
	{
		p_err("addr == 0\n");
		return ;
	}
	mem_free(page->addr);
	page->addr = 0;
}

//static unsigned short from32to16(unsigned int x)
//{
//	/* add up 16-bit and 16-bit for 16+c bit */
//	x = (x &0xffff) + (x >> 16);
//	/* add up carry.. */
//	x = (x &0xffff) + (x >> 16);
//	return x;
//}
static void skb_dst_drop(struct sk_buff *skb)
{
	// 	p_err_miss;
	/*	if (skb->_skb_refdst) {
	refdst_drop(skb->_skb_refdst);
	skb->_skb_refdst = 0UL;
	}8/
	}

	static  void skb_dst_copy(struct sk_buff *nskb, const struct sk_buff *oskb)
	{
	p_err_miss;
	nskb->_skb_refdst = oskb->_skb_refdst;
	if (!(nskb->_skb_refdst & SKB_DST_NOREF))
	dst_clone(skb_dst(nskb));*/
} 


//应该用不到
//static void sg_set_buf(struct scatterlist *sg, const void *buf, unsigned int
//	buflen)
//	p_err_miss;
//	//g_set_page(sg, virt_to_page(buf), buflen, offset_in_page(buf));
//} 


/*
static int skb_pagelen(const struct sk_buff *skb)
{
	int i, len = 0;

	for (i = (int)skb_shinfo(skb)->nr_frags - 1; i >= 0; i--)
		len += skb_shinfo(skb)->frags[i].size;
	return len + skb_headlen(skb);
} */
/**
static void skb_fill_page_desc(struct sk_buff *skb, int i, struct page *page,
	int off, int size)
{
	skb_frag_t *frag = &skb_shinfo(skb)->frags[i];

	frag->page = page;
	frag->page_offset = off;
	frag->size = size;
	skb_shinfo(skb)->nr_frags = i + 1;
} */

//extern void skb_add_rx_frag(struct sk_buff *skb, int i, struct page *page, int
//	off, int size);

#define SKB_PAGE_ASSERT(skb) 	BUG_ON(skb_shinfo(skb)->nr_frags)
#define SKB_FRAG_ASSERT(skb) 	BUG_ON(skb_has_frag_list(skb))
#define SKB_LINEAR_ASSERT(skb)  BUG_ON(skb_is_nonlinear(skb))


/**
 *	skb_tailroom - bytes at buffer end
 *	@skb: buffer to check
 *
 *	Return the number of bytes of free space at the tail of an sk_buff
 */
int skb_tailroom(const struct sk_buff *skb)
{
	return skb_is_nonlinear(skb) ? 0 : skb->end - skb->tail;
} 
#pragma arm section code ="_video_server_"

/**
 *	skb_reserve - adjust headroom
 *	@skb: buffer to alter
 *	@len: bytes to move
 *
 *	Increase the headroom of an empty &sk_buff by reducing the tail
 *	room. This is only allowed for an empty buffer.
 */
void skb_reserve(struct sk_buff *skb, int len)
{
	skb->data += len;
	skb->tail += len;
} 

unsigned char *skb_mac_header(const struct sk_buff *skb)
{
	return skb->mac_header;
} 
#pragma arm section code 

static int skb_mac_header_was_set(const struct sk_buff *skb)
{
	return skb->mac_header != NULL;
} 

#pragma arm section code ="_video_server_"
void skb_reset_mac_header(struct sk_buff *skb)
{
	skb->mac_header = skb->data;
} 
#pragma arm section code 


/*
 * CPUs often take a performance hit when accessing unaligned memory
 * locations. The actual performance hit varies, it can be small if the
 * hardware handles it or large if we have to take an exception and fix it
 * in software.
 *
 * Since an ethernet header is 14 bytes network drivers often end up with
 * the IP header at an unaligned offset. The IP header can be aligned by
 * shifting the start of the packet by 2 bytes. Drivers should do this
 * with:
 *
 * skb_reserve(skb, NET_IP_ALIGN);
 *
 * The downside to this alignment of the IP header is that the DMA is now
 * unaligned. On some architectures the cost of an unaligned DMA is high
 * and this cost outweighs the gains made by aligning the IP header.
 *
 * Since this trade off varies between architectures, we allow NET_IP_ALIGN
 * to be overridden.
 */
#ifndef NET_IP_ALIGN
#define NET_IP_ALIGN	2
#endif 

/*
 * The networking layer reserves some headroom in skb data (via
 * dev_alloc_skb). This is used to avoid having to reallocate skb data when
 * the header has to grow. In the default case, if the header has to grow
 * 32 bytes or less we avoid the reallocation.
 *
 * Unfortunately this headroom changes the DMA alignment of the resulting
 * network packet. As for NET_IP_ALIGN, this unaligned DMA is expensive
 * on some architectures. An architecture can override this value,
 * perhaps setting it to a cacheline in size (since that will maintain
 * cacheline alignment of the DMA). It must be a power of 2.
 *
 * Various parts of the networking layer expect at least 32 bytes of
 * headroom, you should not reduce this.
 *
 * Using max(32, L1_CACHE_BYTES) makes sense (especially with RPS)
 * to reduce average number of cache lines per packet.
 * get_rps_cpus() for example only access one 64 bytes aligned block :
 * NET_IP_ALIGN(2) + ethernet_header(14) + IP_header(20/40) + ports(8)
 */
#ifndef NET_SKB_PAD
#define NET_SKB_PAD	max(32, 4)
#endif 

//extern int ___pskb_trim(struct sk_buff *skb, unsigned int len);
/*
static void __skb_trim(struct sk_buff *skb, unsigned int len)
{
	if (unlikely(skb_is_nonlinear(skb)))
	{
		WARN_ON(1);
		return ;
	} skb->len = len;
	skb_set_tail_pointer(skb, len);
}

extern void skb_trim(struct sk_buff *skb, unsigned int len);

static int __pskb_trim(struct sk_buff *skb, unsigned int len)
{
	if (skb->data_len)
		return ___pskb_trim(skb, len);
	__skb_trim(skb, len);
	return 0;
} 

static int pskb_trim(struct sk_buff *skb, unsigned int len)
{
	return (len < skb->len) ? __pskb_trim(skb, len): 0;
} 
*/
#pragma arm section code ="_video_server_"

struct sk_buff *__dev_alloc_skb(unsigned int length, gfp_t gfp_mask)
{
	struct sk_buff *skb = alloc_skb(length + NET_SKB_PAD, gfp_mask);
	if (likely(skb))
		skb_reserve(skb, NET_SKB_PAD);
	
#ifdef DEBUG
	if(length  + NET_SKB_PAD == 112)
		p_dbg("alloc:%x, %x", skb, skb->head);
	//else
	p_dbg("alloc1:%d,%x, %x", length + NET_SKB_PAD, skb, skb->head);
#endif
	return skb;
} 
#pragma arm section code 

extern struct sk_buff *dev_alloc_skb(unsigned int length);

/*
static int __skb_linearize(struct sk_buff *skb)
{
	return __pskb_pull_tail(skb, skb->data_len) ? 0 :  - ENOMEM;
} 
*/
/**
 *	skb_linearize - convert paged skb to linear one
 *	@skb: buffer to linarize
 *
 *	If there is no free memory -ENOMEM is returned, otherwise zero
 *	is returned and the old skb data released.
 */
//static int skb_linearize(struct sk_buff *skb)
//{
//	return skb_is_nonlinear(skb) ? __skb_linearize(skb): 0;
//} 

#define skb_queue_walk(queue, skb) \
for (skb = (queue)->next;					\
skb != (struct sk_buff *)(queue);				\
skb = skb->next)

#define skb_queue_walk_safe(queue, skb, tmp)					\
for (skb = (queue)->next, tmp = skb->next;			\
skb != (struct sk_buff *)(queue);				\
skb = tmp, tmp = skb->next)

#define skb_queue_walk_from(queue, skb)						\
for (; skb != (struct sk_buff *)(queue);			\
skb = skb->next)

#define skb_queue_walk_from_safe(queue, skb, tmp)				\
for (tmp = skb->next;						\
skb != (struct sk_buff *)(queue);				\
skb = tmp, tmp = skb->next)

#define skb_queue_reverse_walk(queue, skb) \
for (skb = (queue)->prev;					\
skb != (struct sk_buff *)(queue);				\
skb = skb->prev)

#define skb_queue_reverse_walk_safe(queue, skb, tmp)				\
for (skb = (queue)->prev, tmp = skb->prev;			\
skb != (struct sk_buff *)(queue);				\
skb = tmp, tmp = skb->prev)

#define skb_queue_reverse_walk_from_safe(queue, skb, tmp)			\
for (tmp = skb->prev;						\
skb != (struct sk_buff *)(queue);				\
skb = tmp, tmp = skb->prev)

#pragma arm section code ="_video_server_"

static bool skb_has_frag_list(const struct sk_buff *skb)
{
	return skb_shinfo(skb)->frag_list != NULL;
} 
#pragma arm section code 

/*
static void skb_frag_list_init(struct sk_buff *skb)
{
	skb_shinfo(skb)->frag_list = NULL;
} 

static void skb_frag_add_head(struct sk_buff *skb, struct sk_buff *frag)
{
	frag->next = skb_shinfo(skb)->frag_list;
	skb_shinfo(skb)->frag_list = frag;
} */

#define skb_walk_frags(skb, iter)	\
for (iter = skb_shinfo(skb)->frag_list; iter; iter = iter->next)

//extern struct sk_buff *__skb_recv_datagram(struct sock *sk, unsigned flags,
//					   int *peeked, int *err);
//extern struct sk_buff *skb_recv_datagram(struct sock *sk, unsigned flags,
//					 int noblock, int *err);
//extern unsigned int    datagram_poll(struct file *file, struct socket *sock,
//				     struct poll_table_struct *wait);
//extern int	       skb_copy_datagram_iovec(const struct sk_buff *from,
//					       int offset, struct iovec *to,
//					       int size);
//extern int	       skb_copy_and_csum_datagram_iovec(struct sk_buff *skb,
//							int hlen,
//							struct iovec *iov);
//extern int	       skb_copy_datagram_from_iovec(struct sk_buff *skb,
//						    int offset,
//						    const struct iovec *from,
//						    int from_offset,
//						    int len);
//extern int	       skb_copy_datagram_const_iovec(const struct sk_buff *from,
//						     int offset,
//						     const struct iovec *to,
//						     int to_offset,
//						     int size);
//extern void	       skb_free_datagram(struct sock *sk, struct sk_buff *skb);
//extern void	       skb_free_datagram_locked(struct sock *sk,
//						struct sk_buff *skb);
//extern int	       skb_kill_datagram(struct sock *sk, struct sk_buff *skb,
//					 unsigned int flags);
//extern __wsum	       skb_checksum(const struct sk_buff *skb, int offset,
//				    int len, __wsum csum);
int skb_copy_bits(const struct sk_buff *skb, int offset, void *to, int len);
//int skb_store_bits(struct sk_buff *skb, int offset, const void *from, int len);
//__wsum skb_copy_and_csum_bits(const struct sk_buff *skb, int offset, u8 *to,
//	int len, __wsum csum);

//int             skb_splice_bits(struct sk_buff *skb,
//						unsigned int offset,
//						struct pipe_inode_info *pipe,
//						unsigned int len,
//						unsigned int flags);

//void skb_copy_and_csum_dev(const struct sk_buff *skb, u8 *to);
//void skb_split(struct sk_buff *skb, struct sk_buff *skb1, const u32 len);
//int skb_shift(struct sk_buff *tgt, struct sk_buff *skb, int shiftlen);

//struct sk_buff *skb_segment(struct sk_buff *skb, u32 features);
/*
static void *skb_header_pointer(const struct sk_buff *skb, int offset, int len,
	void *buffer)
{
	int hlen = skb_headlen(skb);

	if (hlen - offset >= len)
		return skb->data + offset;

	if (skb_copy_bits(skb, offset, buffer, len) < 0)
		return NULL;

	return buffer;
} 

struct sk_buff *skb_dequeue(struct sk_buff_head *list)
{
	unsigned long flags;
	struct sk_buff *result;

	spin_lock_irqsave(&list->lock, flags);
	result = __skb_dequeue(list);
	spin_unlock_irqrestore(&list->lock, flags);
	return result;
}

struct sk_buff *skb_dequeue_tail(struct sk_buff_head *list)
{
	unsigned long flags;
	struct sk_buff *result;

	spin_lock_irqsave(&list->lock, flags);
	result = __skb_dequeue_tail(list);
	spin_unlock_irqrestore(&list->lock, flags);
	return result;
}

void skb_queue_purge(struct sk_buff_head *list)
{
	struct sk_buff *skb;
	while ((skb = skb_dequeue(list)) != NULL)
		kfree_skb(skb);
}

void skb_queue_head(struct sk_buff_head *list, struct sk_buff *newsk)
{
	unsigned long flags;

	spin_lock_irqsave(&list->lock, flags);
	__skb_queue_head(list, newsk);
	spin_unlock_irqrestore(&list->lock, flags);
}

void skb_queue_tail(struct sk_buff_head *list, struct sk_buff *newsk)
{
	unsigned long flags;

	spin_lock_irqsave(&list->lock, flags);
	__skb_queue_tail(list, newsk);
	spin_unlock_irqrestore(&list->lock, flags);
}*/
/*
void skb_unlink(struct sk_buff *skb, struct sk_buff_head *list)
{
	unsigned long flags;

	spin_lock_irqsave(&list->lock, flags);
	__skb_unlink(skb, list);
	spin_unlock_irqrestore(&list->lock, flags);
}

void skb_append(struct sk_buff *old, struct sk_buff *newsk, struct sk_buff_head *list)
{
	unsigned long flags;

	spin_lock_irqsave(&list->lock, flags);
	__skb_queue_after(list, old, newsk);
	spin_unlock_irqrestore(&list->lock, flags);
}
*/
void skb_copy_from_linear_data(const struct sk_buff *skb, void *to,
	const unsigned int len)
{
	memcpy(to, skb->data, len);
} 

void skb_copy_from_linear_data_offset(const struct sk_buff *skb, const
	int offset, void *to, const unsigned int len)
{
	memcpy(to, skb->data + offset, len);
} 

//extern void skb_timestamping_init(void);

#ifdef CONFIG_NETWORK_PHY_TIMESTAMPING

//extern void skb_clone_tx_timestamp(struct sk_buff *skb);
//extern bool skb_defer_rx_timestamp(struct sk_buff *skb);

#else /* CONFIG_NETWORK_PHY_TIMESTAMPING */

//static void skb_clone_tx_timestamp(struct sk_buff *skb){}

//static bool skb_defer_rx_timestamp(struct sk_buff *skb)
//{
//	return false;
//} 

#endif /* !CONFIG_NETWORK_PHY_TIMESTAMPING */

/**
 * skb_complete_tx_timestamp() - deliver cloned skb with tx timestamps
 *
 * @skb: clone of the the original outgoing packet
 * @hwtstamps: hardware time stamps
 *
 */
void skb_complete_tx_timestamp(struct sk_buff *skb, struct skb_shared_hwtstamps
	*hwtstamps);


#if defined(CONFIG_NF_CONNTRACK) || defined(CONFIG_NF_CONNTRACK_MODULE)
extern void nf_conntrack_destroy(struct nf_conntrack *nfct);
static void nf_conntrack_put(struct nf_conntrack *nfct)
{
	if (nfct && atomic_dec_and_test(&nfct->use))
		nf_conntrack_destroy(nfct);
} static void nf_conntrack_get(struct nf_conntrack *nfct)
{
	if (nfct)
		atomic_inc(&nfct->use);
} 
#endif 
#ifdef NET_SKBUFF_NF_DEFRAG_NEEDED
static void nf_conntrack_get_reasm(struct sk_buff *skb)
{
	if (skb)
		atomic_inc(&skb->users);
} static void nf_conntrack_put_reasm(struct sk_buff *skb)
{
	if (skb)
		kfree_skb(skb);
} 
#endif 
#ifdef CONFIG_BRIDGE_NETFILTER
static void nf_bridge_put(struct nf_bridge_info *nf_bridge)
{
	if (nf_bridge && atomic_dec_and_test(&nf_bridge->use)){
		 kfree(nf_bridge);
	}
	
} static void nf_bridge_get(struct nf_bridge_info *nf_bridge)
{
	if (nf_bridge)
		atomic_inc(&nf_bridge->use);
} 
#endif /* CONFIG_BRIDGE_NETFILTER */

/* Note: This doesn't put any conntrack and bridge info in dst. */
static void __nf_copy(struct sk_buff *dst, const struct sk_buff *src)
{
	#if defined(CONFIG_NF_CONNTRACK) || defined(CONFIG_NF_CONNTRACK_MODULE)
	dst->nfct = src->nfct;
	nf_conntrack_get(src->nfct);
	dst->nfctinfo = src->nfctinfo;
	#endif 
	#ifdef NET_SKBUFF_NF_DEFRAG_NEEDED
	dst->nfct_reasm = src->nfct_reasm;
	nf_conntrack_get_reasm(src->nfct_reasm);
	#endif 
	#ifdef CONFIG_BRIDGE_NETFILTER
	dst->nf_bridge = src->nf_bridge;
	nf_bridge_get(src->nf_bridge);
	#endif 
} 

#ifdef CONFIG_NETWORK_SECMARK
static void skb_copy_secmark(struct sk_buff *to, const struct sk_buff *from)
{
	to->secmark = from->secmark;
} 

static void skb_init_secmark(struct sk_buff *skb)
{
	skb->secmark = 0;
} 
#else 
static void skb_copy_secmark(struct sk_buff *to, const struct sk_buff *from){}

//static void skb_init_secmark(struct sk_buff *skb){}
#endif 
/*
static void skb_set_queue_mapping(struct sk_buff *skb, u16 queue_mapping)
{
	skb->queue_mapping = queue_mapping;
} 

static u16 skb_get_queue_mapping(const struct sk_buff *skb)
{
	return skb->queue_mapping;
} 
*/


static void skb_copy_queue_mapping(struct sk_buff *to, const struct sk_buff
	*from)
{
	to->queue_mapping = from->queue_mapping;
} 

static void skb_over_panic(struct sk_buff *skb, int sz, void *here)
{
	p_err("skb_over_panic: text:%p len:%d put:%d head:%p "
	"data:%p tail:%#lx end:%#lx dev:%s\n", here, skb->len, sz, skb->head, skb
	->data, (unsigned long)skb->tail, (unsigned long)skb->end, skb->dev ? skb
	->dev->name: "<NULL>");
	BUG();
} 


/**
 *	skb_under_panic	- 	private function
 *	@skb: buffer
 *	@sz: size
 *	@here: address
 *
 *	Out of line support code for skb_push(). Not user callable.
 */

static void skb_under_panic(struct sk_buff *skb, int sz, void *here)
{
	p_err("skb_under_panic: text:%p len:%d put:%d head:%p "
	"data:%p tail:%#lx end:%#lx dev:%s\n", here, skb->len, sz, skb->head, skb
	->data, (unsigned long)skb->tail, (unsigned long)skb->end, skb->dev ? skb
	->dev->name: "<NULL>");
	BUG();
} 

#pragma arm section code ="_video_server_"

/* 	Allocate a new skbuff. We do this ourselves so we can fill in a few
 *	'private' fields and also do memory statistics to find all the
 *	[BEEP] leaks.
 *
 */

/**
 *	__alloc_skb	-	allocate a network buffer
 *	@size: size to allocate
 *	@gfp_mask: allocation mask
 *	@fclone: allocate from fclone cache instead of head cache
 *		and allocate a cloned (child) skb
 *	@node: numa node to allocate memory on
 *
 *	Allocate a new &sk_buff. The returned buffer has no headroom and a
 *	tail room of size bytes. The object has a reference count of one.
 *	The return is the buffer. On a failure the return is %NULL.
 *
 *	Buffers may only be allocated from interrupts using a @gfp_mask of
 *	%GFP_ATOMIC.
 */
struct sk_buff *__alloc_skb(const char* name, unsigned int size, gfp_t gfp_mask, int fclone, int
	node)
{
//	uint32_t ret_size;
	//	struct kmem_cache *cache;
	struct skb_shared_info *shinfo;
	struct sk_buff *skb;
	u8 *data;
#if USE_MEM_DEBUG
	skb = mem_calloc_ex(name, sizeof(struct sk_buff), 1);
#else
	skb = mem_calloc(sizeof(struct sk_buff), 1);
#endif
	if (!skb)
		goto out;

	//get_mem_size(skb);
	
	size = SKB_DATA_ALIGN(size);
	data = mem_malloc(size + sizeof(struct skb_shared_info));

	if (!data)
		goto nodata;
	//	prefetchw(data + size);

	//get_mem_size(data);
	/*
	 * Only clear those fields we need to clear, not those that we will
	 * actually initialise below. Hence, don't put any more fields after
	 * the tail pointer in struct sk_buff!
	 */
	memset(skb, 0, offsetof(struct sk_buff, tail));
	skb->truesize = size + sizeof(struct sk_buff);
	atomic_set(&skb->users, 1);
	skb->head = data;
	skb->data = data;
	skb_reset_tail_pointer(skb);
	skb->end = skb->tail + size;
	#ifdef NET_SKBUFF_DATA_USES_OFFSET
	skb->mac_header = ~0U;
	#endif 
	//SET_MONITOR_ITEM_VALUE(_g_skb_alloc_size, g_skb_alloc_size);
	/* make sure we initialize shinfo sequentially */
	shinfo = skb_shinfo(skb);
	memset(shinfo, 0, /*offsetof(struct skb_shared_info, dataref)*/sizeof(struct skb_shared_info));
	atomic_set(&shinfo->dataref, 1);
	//	kmemcheck_annotate_variable(shinfo->destructor_arg);

	if (fclone)
	{
		p_err("fclone\n");
	} 
out: 
	return skb;
nodata: 
	kmem_cache_free(cache, skb);

skb = NULL;
	goto out;
}

//EXPORT_SYMBOL(__alloc_skb);
#pragma arm section code 


/**
 *	__netdev_alloc_skb - allocate an skbuff for rx on a specific device
 *	@dev: network device to receive on
 *	@length: length to allocate
 *	@gfp_mask: get_free_pages mask, passed to alloc_skb
 *
 *	Allocate a new &sk_buff and assign it a usage count of one. The
 *	buffer has unspecified headroom built in. Users should allocate
 *	the headroom they think they need without accounting for the
 *	built in space. The built in space is used for optimisations.
 *
 *	%NULL is returned if there is no free memory.
 */
struct sk_buff *__netdev_alloc_skb(struct net_device *dev, unsigned int length,
	gfp_t gfp_mask)
{
	struct sk_buff *skb;

	skb = __alloc_skb(__func__ , length + NET_SKB_PAD, gfp_mask, 0, NUMA_NO_NODE);
	if (likely(skb))
	{
		skb_reserve(skb, NET_SKB_PAD);
		skb->dev = dev;
	} return skb;
}

////EXPORT_SYMBOL(__netdev_alloc_skb);
/*
void skb_add_rx_frag(struct sk_buff *skb, int i, struct page *page, int off,
	int size)
{
	skb_fill_page_desc(skb, i, page, off, size);
	skb->len += size;
	skb->data_len += size;
	skb->truesize += size;
} //EXPORT_SYMBOL(skb_add_rx_frag);*/
#pragma arm section code ="_video_server_"

/**
 *	dev_alloc_skb - allocate an skbuff for receiving
 *	@length: length to allocate
 *
 *	Allocate a new &sk_buff and assign it a usage count of one. The
 *	buffer has unspecified headroom built in. Users should allocate
 *	the headroom they think they need without accounting for the
 *	built in space. The built in space is used for optimisations.
 *
 *	%NULL is returned if there is no free memory. Although this function
 *	allocates memory it can be called from an interrupt.
 */
struct sk_buff *dev_alloc_skb(unsigned int length)
{
	/*
	 * There is more code here than it seems:
	 * __dev_alloc_skb is an 
	 */
	return __dev_alloc_skb(length, GFP_ATOMIC);
} 
#pragma arm section code 

////EXPORT_SYMBOL(dev_alloc_skb);

static void skb_drop_list(struct sk_buff **listp)
{
	struct sk_buff *list =  *listp;

	*listp = NULL;

	do
	{
		struct sk_buff *this = list;
		list = list->next;
		kfree_skb(this);
	} 
	while (list);
}

static void skb_drop_fraglist(struct sk_buff *skb)
{
	skb_drop_list(&skb_shinfo(skb)->frag_list);
} 

static void skb_clone_fraglist(struct sk_buff *skb)
{
	struct sk_buff *list;

	skb_walk_frags(skb, list)
		skb_get(list);
} 
#pragma arm section code ="_video_server_"

static void skb_release_data(struct sk_buff *skb)
{
//	uint32_t ret_size;
	if (!skb->cloned || !atomic_sub_return(skb->nohdr ? (1 << SKB_DATAREF_SHIFT) +
	1: 1,  &skb_shinfo(skb)->dataref))
	{
		if (skb_shinfo(skb)->nr_frags)
		{
			int i;
			for (i = 0; i < skb_shinfo(skb)->nr_frags; i++)
				put_page(skb_shinfo(skb)->frags[i].page);
		} 

		if (skb_has_frag_list(skb))
			skb_drop_fraglist(skb);

		kfree(skb->head);
	}
}

/*
 *	Free an skbuff by memory without cleaning the state.
 */
static void kfree_skbmem(struct sk_buff *skb)
{
//	uint32_t ret_size;
	struct sk_buff *other;
	atomic_t *fclone_ref;

	switch (skb->fclone)
	{
		case SKB_FCLONE_UNAVAILABLE:
			
			kmem_cache_free(skbuff_head_cache, skb);
			break;

		case SKB_FCLONE_ORIG:
			fclone_ref = (atomic_t*)(skb + 2);
			if (atomic_dec_and_test(fclone_ref)){
				kmem_cache_free(skbuff_fclone_cache, skb);
			}
			break;

		case SKB_FCLONE_CLONE:
			fclone_ref = (atomic_t*)(skb + 1);
			other = skb - 1;

			/* The clone portion is available for
			 * fast-cloning again.
			 */
			skb->fclone = SKB_FCLONE_UNAVAILABLE;

			if (atomic_dec_and_test(fclone_ref)){
				kmem_cache_free(skbuff_fclone_cache, other);
			}
			break;
	}
}

static void skb_release_head_state(struct sk_buff *skb)
{
	skb_dst_drop(skb);
	#ifdef CONFIG_XFRM
	secpath_put(skb->sp);
	#endif 
	if (skb->destructor)
	{
		//	WARN_ON(in_irq());
		skb->destructor(skb);
	} 
	#if defined(CONFIG_NF_CONNTRACK) || defined(CONFIG_NF_CONNTRACK_MODULE)
	nf_conntrack_put(skb->nfct);
	#endif 
	#ifdef NET_SKBUFF_NF_DEFRAG_NEEDED
	nf_conntrack_put_reasm(skb->nfct_reasm);
	#endif 
	#ifdef CONFIG_BRIDGE_NETFILTER
	nf_bridge_put(skb->nf_bridge);
	#endif 
	/* XXX: IS this still necessary? - JHS */
	#ifdef CONFIG_NET_SCHED
	skb->tc_index = 0;
	#ifdef CONFIG_NET_CLS_ACT
	skb->tc_verd = 0;
	#endif 
	#endif 
}

/* Free everything but the sk_buff shell. */
static void skb_release_all(struct sk_buff *skb)
{
	skb_release_head_state(skb);
	skb_release_data(skb);
} 

/**
 *	__kfree_skb - private function
 *	@skb: buffer
 *
 *	Free an sk_buff. Release anything attached to the buffer.
 *	Clean the state. This is an internal helper function. Users should
 *	always call kfree_skb
 */

void __kfree_skb(struct sk_buff *skb)
{
	skb_release_all(skb);
	kfree_skbmem(skb);
	//SET_MONITOR_ITEM_VALUE(_g_skb_alloc_size, g_skb_alloc_size);
} 
//EXPORT_SYMBOL(__kfree_skb);
#pragma arm section code 


uint32_t get_global_skb_alloc()
{
	return 0;
}

#pragma arm section code ="_video_server_"

/**
 *	kfree_skb - free an sk_buff
 *	@skb: buffer to free
 *
 *	Drop a reference to the buffer and free it if the usage count has
 *	hit zero.
 */
void kfree_skb(struct sk_buff *skb)
{
	if (unlikely(!skb))
		return ;

	else if (likely(!atomic_dec_and_test(&skb->users)))
		return ;

	__kfree_skb(skb);
} //EXPORT_SYMBOL(kfree_skb);

/**
 *	consume_skb - free an skbuff
 *	@skb: buffer to free
 *
 *	Drop a ref to the buffer and free it if the usage count has hit zero
 *	Functions identically to kfree_skb, but kfree_skb assumes that the frame
 *	is being dropped after a failure and notes that
 */
void consume_skb(struct sk_buff *skb)
{
	if (unlikely(!skb))
		return ;

	else if (likely(!atomic_dec_and_test(&skb->users)))
		return ;

	__kfree_skb(skb);
} //EXPORT_SYMBOL(consume_skb);
#pragma arm section code 

/**
 *	skb_recycle_check - check if skb can be reused for receive
 *	@skb: buffer
 *	@skb_size: minimum receive buffer size
 *
 *	Checks that the skb passed in is not shared or cloned, and
 *	that it is linear and its head portion at least as large as
 *	skb_size so that it can be recycled as a receive buffer.
 *	If these conditions are met, this function does any necessary
 *	reference count dropping and cleans up the skbuff as if it
 *	just came from __alloc_skb().
 */
bool skb_recycle_check(struct sk_buff *skb, int skb_size)
{
	struct skb_shared_info *shinfo;

	//	if (irqs_disabled())
	//		return false;

	if (skb_is_nonlinear(skb) || skb->fclone != SKB_FCLONE_UNAVAILABLE)
		return false;

	skb_size = SKB_DATA_ALIGN(skb_size + NET_SKB_PAD);
	if (skb_end_pointer(skb) - skb->head < skb_size)
		return false;

	if (skb_shared(skb) || skb_cloned(skb))
		return false;

	skb_release_head_state(skb);

	shinfo = skb_shinfo(skb);
	memset(shinfo, 0, offsetof(struct skb_shared_info, dataref));
	atomic_set(&shinfo->dataref, 1);

	memset(skb, 0, offsetof(struct sk_buff, tail));
	skb->data = skb->head + NET_SKB_PAD;
	skb_reset_tail_pointer(skb);

	return true;
} //EXPORT_SYMBOL(skb_recycle_check);

static inline void skb_dst_copy(struct sk_buff *nskb, const struct sk_buff *oskb)
{
	nskb->_skb_refdst = oskb->_skb_refdst;
	//if (!(nskb->_skb_refdst & SKB_DST_NOREF))
	//	dst_clone(skb_dst(nskb));
}

static void __copy_skb_header(struct sk_buff *new, const struct sk_buff *old)
{
	new->tstamp = old->tstamp;
	new->dev = old->dev;
	new->transport_header = old->transport_header;
	new->network_header = old->network_header;
	new->mac_header = old->mac_header;
	skb_dst_copy(new, old);
	//p_err("skb_dst_copy\n");
	new->rxhash = old->rxhash;
	#ifdef CONFIG_XFRM
	new->sp = secpath_get(old->sp);
	#endif 
	memcpy(new->cb, old->cb, sizeof(old->cb));
	new->cs.csum = old->cs.csum;
	new->local_df = old->local_df;
	new->pkt_type = old->pkt_type;
	new->ip_summed = old->ip_summed;
	skb_copy_queue_mapping(new, old);
	new->priority = old->priority;
	#if defined(CONFIG_IP_VS) || defined(CONFIG_IP_VS_MODULE)
	new->ipvs_property = old->ipvs_property;
	#endif 
	new->protocol = old->protocol;
	new->mk.mark = old->mk.mark;
	new->skb_iif = old->skb_iif;
	__nf_copy(new, old);
	#if defined(CONFIG_NETFILTER_XT_TARGET_TRACE) || \
	defined(CONFIG_NETFILTER_XT_TARGET_TRACE_MODULE)
	new->nf_trace = old->nf_trace;
	#endif 
	#ifdef CONFIG_NET_SCHED
	new->tc_index = old->tc_index;
	#ifdef CONFIG_NET_CLS_ACT
	new->tc_verd = old->tc_verd;
	#endif 
	#endif 
	new->vlan_tci = old->vlan_tci;

	skb_copy_secmark(new, old);
} 

/*
 * You should not add any new code to this function.  Add it to
 * __copy_skb_header above instead.
 */
static struct sk_buff *__skb_clone(struct sk_buff *n, struct sk_buff *skb)
{
	#define C(x) n->x = skb->x

	n->next = n->prev = NULL;
	//n->sk = NULL;
	__copy_skb_header(n, skb);

	C(len);
	C(data_len);
	C(mac_len);
	n->hdr_len = skb->nohdr ? skb_headroom(skb): skb->hdr_len;
	n->cloned = 1;
	n->nohdr = 0;
	n->destructor = NULL;
	C(tail);
	C(end);
	C(head);
	C(data);
	C(truesize);
	atomic_set(&n->users, 1);

	atomic_inc(&(skb_shinfo(skb)->dataref));
	skb->cloned = 1;

	return n;
	#undef C
} 

/**
 *	skb_morph	-	morph one skb into another
 *	@dst: the skb to receive the contents
 *	@src: the skb to supply the contents
 *
 *	This is identical to skb_clone except that the target skb is
 *	supplied by the user.
 *
 *	The target skb is returned upon exit.
 */
struct sk_buff *skb_morph(struct sk_buff *dst, struct sk_buff *src)
{
	skb_release_all(dst);
	return __skb_clone(dst, src);
}

//EXPORT_SYMBOL(skb_morph);

/**
 *	skb_clone	-	duplicate an sk_buff
 *	@skb: buffer to clone
 *	@gfp_mask: allocation priority
 *
 *	Duplicate an &sk_buff. The new one is not owned by a socket. Both
 *	copies share the same packet data but not structure. The new
 *	buffer has a reference count of 1. If the allocation fails the
 *	function returns %NULL otherwise the new buffer is returned.
 *
 *	If this function is called from an interrupt gfp_mask() must be
 *	%GFP_ATOMIC.
 */

struct sk_buff *skb_clone(struct sk_buff *skb, gfp_t gfp_mask)
{
	struct sk_buff *n;

	n = skb + 1;
	if (skb->fclone == SKB_FCLONE_ORIG && n->fclone == SKB_FCLONE_UNAVAILABLE)
	{
		atomic_t *fclone_ref = (atomic_t*)(n + 1);
		n->fclone = SKB_FCLONE_CLONE;
		atomic_inc(fclone_ref);
	} 
	else
	{
		n = (struct sk_buff*)mem_calloc(1, sizeof(struct sk_buff));
		if (!n)
			return NULL;
		//get_mem_size(n);
		n->fclone = SKB_FCLONE_UNAVAILABLE;
	}

	return __skb_clone(n, skb);
}

//EXPORT_SYMBOL(skb_clone);

static void copy_skb_header(struct sk_buff *new, const struct sk_buff *old)
{
	#ifndef NET_SKBUFF_DATA_USES_OFFSET
	/*
	 *	Shift between the two data areas in bytes
	 */
	unsigned long offset = new->data - old->data;
	#endif 

	__copy_skb_header(new, old);

	#ifndef NET_SKBUFF_DATA_USES_OFFSET
	/* {transport,network,mac}_header are relative to skb->head */
	new->transport_header += offset;
	new->network_header += offset;
	if (skb_mac_header_was_set(new))
		new->mac_header += offset;
	#endif 
} 

/**
 *	skb_copy	-	create private copy of an sk_buff
 *	@skb: buffer to copy
 *	@gfp_mask: allocation priority
 *
 *	Make a copy of both an &sk_buff and its data. This is used when the
 *	caller wishes to modify the data and needs a private copy of the
 *	data to alter. Returns %NULL on failure or the pointer to the buffer
 *	on success. The returned buffer has a reference count of 1.
 *
 *	As by-product this function converts non-linear &sk_buff to linear
 *	one, so that &sk_buff becomes completely private and caller is allowed
 *	to modify all the data of returned buffer. This means that this
 *	function is not recommended for use in circumstances when only
 *	header is going to be modified. Use pskb_copy() instead.
 */

struct sk_buff *skb_copy(const struct sk_buff *skb, gfp_t gfp_mask)
{
	int headerlen = skb_headroom(skb);
	unsigned int size = (skb_end_pointer(skb) - skb->head) + skb->data_len;
	struct sk_buff *n = alloc_skb(size, gfp_mask);

	if (!n)
		return NULL;

	/* Set the data pointer */
	skb_reserve(n, headerlen);
	/* Set the tail pointer and length */
	skb_put(n, skb->len);

	if (skb_copy_bits(skb,  - headerlen, n->head, headerlen + skb->len))
		BUG();

	copy_skb_header(n, skb);
	return n;
} //EXPORT_SYMBOL(skb_copy);

/**
 *	pskb_copy	-	create copy of an sk_buff with private head.
 *	@skb: buffer to copy
 *	@gfp_mask: allocation priority
 *
 *	Make a copy of both an &sk_buff and part of its data, located
 *	in header. Fragmented data remain shared. This is used when
 *	the caller wishes to modify only header of &sk_buff and needs
 *	private copy of the header to alter. Returns %NULL on failure
 *	or the pointer to the buffer on success.
 *	The returned buffer has a reference count of 1.
 */

struct sk_buff *pskb_copy(struct sk_buff *skb, gfp_t gfp_mask)
{
	unsigned int size = skb_end_pointer(skb) - skb->head;
	struct sk_buff *n = alloc_skb(size, gfp_mask);

	if (!n)
		goto out;

	/* Set the data pointer */
	skb_reserve(n, skb_headroom(skb));
	/* Set the tail pointer and length */
	skb_put(n, skb_headlen(skb));
	/* Copy the bytes */
	skb_copy_from_linear_data(skb, n->data, n->len);

	n->truesize += skb->data_len;
	n->data_len = skb->data_len;
	n->len = skb->len;

	if (skb_shinfo(skb)->nr_frags)
	{
		int i;

		for (i = 0; i < skb_shinfo(skb)->nr_frags; i++)
		{
			skb_shinfo(n)->frags[i] = skb_shinfo(skb)->frags[i];
			get_page(skb_shinfo(n)->frags[i].page);
		} skb_shinfo(n)->nr_frags = i;
	}

	if (skb_has_frag_list(skb))
	{
		skb_shinfo(n)->frag_list = skb_shinfo(skb)->frag_list;
		skb_clone_fraglist(n);
	}

	copy_skb_header(n, skb);
	out: return n;
}

//EXPORT_SYMBOL(pskb_copy);

/**
 *	pskb_expand_head - reallocate header of &sk_buff
 *	@skb: buffer to reallocate
 *	@nhead: room to add at head
 *	@ntail: room to add at tail
 *	@gfp_mask: allocation priority
 *
 *	Expands (or creates identical copy, if &nhead and &ntail are zero)
 *	header of skb. &sk_buff itself is not changed. &sk_buff MUST have
 *	reference count of 1. Returns zero in the case of success or error,
 *	if expansion failed. In the last case, &sk_buff is not changed.
 *
 *	All the pointers pointing into skb header may change and must be
 *	reloaded after call to this function.
 */

int pskb_expand_head(struct sk_buff *skb, int nhead, int ntail, gfp_t gfp_mask)
{
	int i;
	u8 *data;
	int size = nhead + (skb_end_pointer(skb) - skb->head) + ntail;
	long off;
	bool fastpath;

	BUG_ON(nhead < 0);

	if (skb_shared(skb))
		BUG();

	size = SKB_DATA_ALIGN(size);

	/* Check if we can avoid taking references on fragments if we own
	 * the last reference on skb->head. (see skb_release_data())
	 */
	if (!skb->cloned)
		fastpath = true;
	else
	{
		int delta = skb->nohdr ? (1 << SKB_DATAREF_SHIFT) + 1: 1;

		fastpath = atomic_read(&skb_shinfo(skb)->dataref) == delta;
	} 

	if (fastpath /*&&
	 	    size + sizeof(struct skb_shared_info) <= ksize(skb->head)*/)
	{
		/*		memmove(skb->head + size, skb_shinfo(skb),
		offsetof(struct skb_shared_info,
		frags[skb_shinfo(skb)->nr_frags]));
		memmove(skb->head + nhead, skb->head,
		skb_tail_pointer(skb) - skb->head);
		off = nhead;
		goto adjust_others;*/
		p_err_miss;
	}

	data = kmalloc(size + sizeof(struct skb_shared_info), gfp_mask);
	if (!data)
		goto nodata;

	//get_mem_size(data);
	/* Copy only real data... and, alas, header. This should be
	 * optimized for the cases when header is void.
	 */
	memcpy(data + nhead, skb->head, skb_tail_pointer(skb) - skb->head);

	memcpy((struct skb_shared_info*)(data + size), skb_shinfo(skb), offsetof
	(struct skb_shared_info, frags[skb_shinfo(skb)->nr_frags]));

	if (fastpath)
	{
//		uint32_t ret_size;
		kfree(skb->head);
	} 
	else
	{
		for (i = 0; i < skb_shinfo(skb)->nr_frags; i++)
			get_page(skb_shinfo(skb)->frags[i].page);

		if (skb_has_frag_list(skb))
			skb_clone_fraglist(skb);

		skb_release_data(skb);
	}
	off = (data + nhead) - skb->head;

	skb->head = data;
//adjust_others: 
	skb->data += off;
	#ifdef NET_SKBUFF_DATA_USES_OFFSET
	skb->end = size;
	off = nhead;
	#else 
	skb->end = skb->head + size;
	#endif 
	/* {transport,network,mac}_header and tail are relative to skb->head */
	skb->tail += off;
	skb->transport_header += off;
	skb->network_header += off;
	if (skb_mac_header_was_set(skb))
		skb->mac_header += off;
	/* Only adjust this if it actually is csum_start rather than csum */
	if (skb->ip_summed == CHECKSUM_PARTIAL)
		skb->cs.cs_w.csum_start += nhead;
	skb->cloned = 0;
	skb->hdr_len = 0;
	skb->nohdr = 0;
	atomic_set(&skb_shinfo(skb)->dataref, 1);
	return 0;

	nodata: 
		return  - ENOMEM;
}

//EXPORT_SYMBOL(pskb_expand_head);

/* Make private copy of skb with writable head and some headroom */

struct sk_buff *skb_realloc_headroom(struct sk_buff *skb, unsigned int headroom)
{
	struct sk_buff *skb2;
	int delta = headroom - skb_headroom(skb);

	if (delta <= 0)
		skb2 = pskb_copy(skb, GFP_ATOMIC);
	else
	{
		skb2 = skb_clone(skb, GFP_ATOMIC);
		if (skb2 && pskb_expand_head(skb2, SKB_DATA_ALIGN(delta), 0, GFP_ATOMIC))
		{
			kfree_skb(skb2);
			skb2 = NULL;
		} 
	}
	return skb2;
}

//EXPORT_SYMBOL(skb_realloc_headroom);

/**
 *	skb_copy_expand	-	copy and expand sk_buff
 *	@skb: buffer to copy
 *	@newheadroom: new free bytes at head
 *	@newtailroom: new free bytes at tail
 *	@gfp_mask: allocation priority
 *
 *	Make a copy of both an &sk_buff and its data and while doing so
 *	allocate additional space.
 *
 *	This is used when the caller wishes to modify the data and needs a
 *	private copy of the data to alter as well as more space for new fields.
 *	Returns %NULL on failure or the pointer to the buffer
 *	on success. The returned buffer has a reference count of 1.
 *
 *	You must pass %GFP_ATOMIC as the allocation priority if this function
 *	is called from an interrupt.
 */
struct sk_buff *skb_copy_expand(const struct sk_buff *skb, int newheadroom, int
	newtailroom, gfp_t gfp_mask)
{
	/*
	 *	Allocate the copy buffer
	 */
	struct sk_buff *n = alloc_skb(newheadroom + skb->len + newtailroom, gfp_mask);
	int oldheadroom = skb_headroom(skb);
	int head_copy_len, head_copy_off;
	int off;

	if (!n)
		return NULL;

	skb_reserve(n, newheadroom);

	/* Set the tail pointer and length */
	skb_put(n, skb->len);

	head_copy_len = oldheadroom;
	head_copy_off = 0;
	if (newheadroom <= head_copy_len)
		head_copy_len = newheadroom;
	else
		head_copy_off = newheadroom - head_copy_len;

	/* Copy the linear header and data. */
	if (skb_copy_bits(skb,  - head_copy_len, n->head + head_copy_off, skb->len +
	head_copy_len))
		BUG();

	copy_skb_header(n, skb);

	off = newheadroom - oldheadroom;
	if (n->ip_summed == CHECKSUM_PARTIAL)
		n->cs.cs_w.csum_start += off;
	#ifdef NET_SKBUFF_DATA_USES_OFFSET
	n->transport_header += off;
	n->network_header += off;
	if (skb_mac_header_was_set(skb))
		n->mac_header += off;
	#endif 

	return n;
} //EXPORT_SYMBOL(skb_copy_expand);

/**
 *	skb_pad			-	zero pad the tail of an skb
 *	@skb: buffer to pad
 *	@pad: space to pad
 *
 *	Ensure that a buffer is followed by a padding area that is zero
 *	filled. Used by network drivers which may DMA or transfer data
 *	beyond the buffer end onto the wire.
 *
 *	May return error in out of memory cases. The skb is freed on error.
 */
#if 0
int skb_pad(struct sk_buff *skb, int pad)
{
	int err;
	int ntail;

	/* If the skbuff is non linear tailroom is always zero.. */
	if (!skb_cloned(skb) && skb_tailroom(skb) >= pad)
	{
		memset(skb->data + skb->len, 0, pad);
		return 0;
	} 

	ntail = skb->data_len + pad - (skb->end - skb->tail);
	if (likely(skb_cloned(skb) || ntail > 0))
	{
		err = pskb_expand_head(skb, 0, ntail, GFP_ATOMIC);
		if (unlikely(err))
			goto free_skb;
	}

	/* FIXME: The use of this function with non-linear skb's really needs
	 * to be audited.
	 */
	err = skb_linearize(skb);
	if (unlikely(err))
		goto free_skb;

	memset(skb->data + skb->len, 0, pad);
	return 0;

	free_skb: kfree_skb(skb);
	return err;
}

//EXPORT_SYMBOL(skb_pad);
#endif
#pragma arm section code ="_video_server_"

/**
 *	skb_put - add data to a buffer
 *	@skb: buffer to use
 *	@len: amount of data to add
 *
 *	This function extends the used data area of the buffer. If this would
 *	exceed the total buffer size the kernel will panic. A pointer to the
 *	first byte of the extra data is returned.
 */

unsigned char *skb_put(struct sk_buff *skb, unsigned int len)
{
	unsigned char *tmp = skb_tail_pointer(skb);
	//SKB_LINEAR_ASSERT(skb);
	skb->tail += len;
	skb->len += len;
	if (unlikely(skb->tail > skb->end))
		skb_over_panic(skb, len, 0 /*__builtin_return_address(0)*/);
	return tmp;
} //EXPORT_SYMBOL(skb_put);
#pragma arm section code 

/**
 *	skb_push - add data to the start of a buffer
 *	@skb: buffer to use
 *	@len: amount of data to add
 *
 *	This function extends the used data area of the buffer at the buffer
 *	start. If this would exceed the total buffer headroom the kernel will
 *	panic. A pointer to the first byte of the extra data is returned.
 */
unsigned char *skb_push(struct sk_buff *skb, unsigned int len)
{
	skb->data -= len;
	skb->len += len;
	if (unlikely(skb->data < skb->head))
		skb_under_panic(skb, len, 0 /*__builtin_return_address(0)*/);
	return skb->data;
} //EXPORT_SYMBOL(skb_push);

/**
 *	skb_pull - remove data from the start of a buffer
 *	@skb: buffer to use
 *	@len: amount of data to remove
 *
 *	This function removes data from the start of a buffer, returning
 *	the memory to the headroom. A pointer to the next data in the buffer
 *	is returned. Once the data has been pulled future pushes will overwrite
 *	the old data.
 */

/*
unsigned char *skb_pull(struct sk_buff *skb, unsigned int len)
{
	return skb_pull_inline(skb, len);
} //EXPORT_SYMBOL(skb_pull);
*/
/**
 *	skb_trim - remove end from a buffer
 *	@skb: buffer to alter
 *	@len: new length
 *
 *	Cut the length of a buffer down by removing data from the tail. If
 *	the buffer is already under the length specified it is not modified.
 *	The skb must be linear.
 */
 #if 0
void skb_trim(struct sk_buff *skb, unsigned int len)
{
	if (skb->len > len)
		__skb_trim(skb, len);
} //EXPORT_SYMBOL(skb_trim);

/* Trims skb to length len. It can change skb pointers.
 */

int ___pskb_trim(struct sk_buff *skb, unsigned int len)
{
	struct sk_buff **fragp;
	struct sk_buff *frag;
	int offset = skb_headlen(skb);
	int nfrags = skb_shinfo(skb)->nr_frags;
	int i;
	int err;

	if (skb_cloned(skb) && unlikely((err = pskb_expand_head(skb, 0, 0, GFP_ATOMIC))
	))
		return err;

	i = 0;
	if (offset >= len)
		goto drop_pages;

	for (; i < nfrags; i++)
	{
		int end = offset + skb_shinfo(skb)->frags[i].size;

		if (end < len)
		{
			offset = end;
			continue;
		} 

		skb_shinfo(skb)->frags[i++].size = len - offset;

		drop_pages: skb_shinfo(skb)->nr_frags = i;

		for (; i < nfrags; i++)
			put_page(skb_shinfo(skb)->frags[i].page);

		if (skb_has_frag_list(skb))
			skb_drop_fraglist(skb);
		goto done;
	}

	for (fragp = &skb_shinfo(skb)->frag_list; (frag =  *fragp); fragp = &frag
	->next)
	{
		int end = offset + frag->len;

		if (skb_shared(frag))
		{
			struct sk_buff *nfrag;

			nfrag = skb_clone(frag, GFP_ATOMIC);
			if (unlikely(!nfrag))
				return  - ENOMEM;

			nfrag->next = frag->next;
			kfree_skb(frag);
			frag = nfrag;
			*fragp = frag;
		}

		if (end < len)
		{
			offset = end;
			continue;
		} 

		if (end > len && unlikely((err = pskb_trim(frag, len - offset))))
			return err;

		if (frag->next)
			skb_drop_list(&frag->next);
		break;
	}

	done: if (len > skb_headlen(skb))
	{
		skb->data_len -= skb->len - len;
		skb->len = len;
	}
	else
	{
		skb->len = len;
		skb->data_len = 0;
		skb_set_tail_pointer(skb, len);
	}

	return 0;
}

//EXPORT_SYMBOL(___pskb_trim);
#endif
/**
 *	__pskb_pull_tail - advance tail of skb header
 *	@skb: buffer to reallocate
 *	@delta: number of bytes to advance tail
 *
 *	The function makes a sense only on a fragmented &sk_buff,
 *	it expands header moving its tail forward and copying necessary
 *	data from fragmented part.
 *
 *	&sk_buff MUST have reference count of 1.
 *
 *	Returns %NULL (and &sk_buff does not change) if pull failed
 *	or value of new tail of skb in the case of success.
 *
 *	All the pointers pointing into skb header may change and must be
 *	reloaded after call to this function.
 */

/* Moves tail of skb head forward, copying data from fragmented part,
 * when it is necessary.
 * 1. It may fail due to malloc failure.
 * 2. It may change skb pointers.
 *
 * It is pretty complicated. Luckily, it is called only in exceptional cases.
 */
unsigned char *__pskb_pull_tail(struct sk_buff *skb, int delta)
{
	/* If skb has not enough free space at tail, get new one
	 * plus 128 bytes for future expansions. If we have enough
	 * room at tail, reallocate without expansion only if skb is cloned.
	 */
	int i, k, eat = (skb->tail + delta) - skb->end;

	if (eat > 0 || skb_cloned(skb))
	{
		if (pskb_expand_head(skb, 0, eat > 0 ? eat + 128: 0, GFP_ATOMIC))
			return NULL;
	} 

	if (skb_copy_bits(skb, skb_headlen(skb), skb_tail_pointer(skb), delta))
		BUG();

	/* Optimization: no fragments, no reasons to preestimate
	 * size of pulled pages. Superb.
	 */
	if (!skb_has_frag_list(skb))
		goto pull_pages;

	/* Estimate size of pulled pages. */
	eat = delta;
	for (i = 0; i < skb_shinfo(skb)->nr_frags; i++)
	{
		if (skb_shinfo(skb)->frags[i].size >= eat)
			goto pull_pages;
		eat -= skb_shinfo(skb)->frags[i].size;
	}

	/* If we need update frag list, we are in troubles.
	 * Certainly, it possible to add an offset to skb data,
	 * but taking into account that pulling is expected to
	 * be very rare operation, it is worth to fight against
	 * further bloating skb head and crucify ourselves here instead.
	 * Pure masohism, indeed. 8)8)
	 */
	if (eat)
	{
		struct sk_buff *list = skb_shinfo(skb)->frag_list;
		struct sk_buff *clone = NULL;
		struct sk_buff *insp = NULL;

		do
		{
			BUG_ON(!list);

			if (list->len <= eat)
			{
				/* Eaten as whole. */
				eat -= list->len;
				list = list->next;
				insp = list;
			} 
			else
			{
				/* Eaten partially. */

				if (skb_shared(list))
				{
					/* Sucks! We need to fork list. :-( */
					clone = skb_clone(list, GFP_ATOMIC);
					if (!clone)
						return NULL;
					insp = list->next;
					list = clone;
				}
				else
				{
					/* This may be pulled without
					 * problems. */
					insp = list;
				}
				if (!pskb_pull(list, eat))
				{
					kfree_skb(clone);
					return NULL;
				}
				break;
			}
		}
		while (eat);

		/* Free pulled out fragments. */
		while ((list = skb_shinfo(skb)->frag_list) != insp)
		{
			skb_shinfo(skb)->frag_list = list->next;
			kfree_skb(list);
		}
		/* And insert new clone at head. */
		if (clone)
		{
			clone->next = list;
			skb_shinfo(skb)->frag_list = clone;
		}
	}
	/* Success! Now we may commit changes to skb data. */

	pull_pages: eat = delta;
	k = 0;
	for (i = 0; i < skb_shinfo(skb)->nr_frags; i++)
	{
		if (skb_shinfo(skb)->frags[i].size <= eat)
		{
			put_page(skb_shinfo(skb)->frags[i].page);
			eat -= skb_shinfo(skb)->frags[i].size;
		}
		else
		{
			skb_shinfo(skb)->frags[k] = skb_shinfo(skb)->frags[i];
			if (eat)
			{
				skb_shinfo(skb)->frags[k].page_offset += eat;
				skb_shinfo(skb)->frags[k].size -= eat;
				eat = 0;
			}
			k++;
		}
	}
	skb_shinfo(skb)->nr_frags = k;

	skb->tail += delta;
	skb->data_len -= delta;

	return skb_tail_pointer(skb);
}

//EXPORT_SYMBOL(__pskb_pull_tail);

/* Copy some data bits from skb to kernel buffer. */

int skb_copy_bits(const struct sk_buff *skb, int offset, void *to, int len)
{
	int start = skb_headlen(skb);
	struct sk_buff *frag_iter;
	int i, copy;

	if (offset > (int)skb->len - len)
		goto fault;

	/* Copy header. */
	if ((copy = start - offset) > 0)
	{
		if (copy > len)
			copy = len;
		skb_copy_from_linear_data_offset(skb, offset, to, copy);
		if ((len -= copy) == 0)
			return 0;
		offset += copy;
		//wmj-
		//(int)to += copy;
		to += copy;
	} 

	for (i = 0; i < skb_shinfo(skb)->nr_frags; i++)
	{
		int end;

		WARN_ON(start > offset + len);

		end = start + skb_shinfo(skb)->frags[i].size;
		if ((copy = end - offset) > 0)
		{
			u8 *vaddr;

			if (copy > len)
				copy = len;

			vaddr = (__u8*)kmap_skb_frag(&skb_shinfo(skb)->frags[i]);
			memcpy(to, vaddr + skb_shinfo(skb)->frags[i].page_offset + offset - start,
	copy);
			kunmap_skb_frag(vaddr);

			if ((len -= copy) == 0)
				return 0;
			offset += copy;
			//wmj
			//(int)to += copy;
			to += copy;
		}
		start = end;
	}

	skb_walk_frags(skb, frag_iter)
	{
		int end;

		WARN_ON(start > offset + len);

		end = start + frag_iter->len;
		if ((copy = end - offset) > 0)
		{
			if (copy > len)
				copy = len;
			if (skb_copy_bits(frag_iter, offset - start, to, copy))
				goto fault;
			if ((len -= copy) == 0)
				return 0;
			offset += copy;
			//wmj-
			//(int)to += copy;
			to += copy;
		}
		start = end;
	}
	if (!len)
		return 0;

	fault: return  - EFAULT;
}

