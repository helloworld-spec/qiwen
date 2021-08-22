/*
 * Copyright (c) 2014 South Silicon Valley Microelectronics Inc.
 * Copyright (c) 2015 iComm Semiconductor Ltd.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef _LIST_H
#define _LIST_H

#define offsetof(type, field) ((size_t) &((type *)0)->field)

#define container_of(ptr, type, field) ({ \
        const typeof( ((type *)0)->field ) *__mlptr = (ptr); \
        (type *)( (char *)__mlptr - offsetof(type,field) );})


#define LIST_POS1  ((void *) 0x00300300)
#define LIST_POS2  ((void *) 0x00500500)

struct list_head {
	struct list_head *prev, *next;
};

#define LIST_HEAD_INIT(name) { &(name), &(name) }

#define LIST_HEAD(name) \
	struct list_head name = LIST_HEAD_INIT(name)

#define INIT_LIST_HEAD(ptr) do { (ptr)->prev = (ptr); (ptr)->next = (ptr); } while (0)

static inline int list_empty(const struct list_head *head)
{
	return head->next == head;
}

static inline void __list_splice(struct list_head *list, struct list_head *head)
{
	struct list_head *last = list->prev;
	struct list_head *first = list->next;
	struct list_head *at = head->next;

	first->prev = head;
	head->next = first;

	last->next = at;
	at->prev = last;
}

static inline void list_splice(struct list_head *list, struct list_head *head)
{
	if(!list_empty(list))
		__list_splice(list, head);
}

static inline void __list_add(struct list_head *new, struct list_head *prev, struct list_head *next)
{
	next->prev = new;
	new->prev = prev;
	new->next = next;
	prev->next = new;
}

static inline void list_add(struct list_head *new, struct list_head *head)
{
	__list_add(new, head, head->next);
}

static inline void list_add_tail(struct list_head *new, struct list_head *head)
{
	__list_add(new, head->prev, head);
}

static inline void __list_del(struct list_head *prev, struct list_head *next)
{
	next->prev = prev;
	prev->next = next;
}

static inline void list_del_init(struct list_head *entry)
{
	__list_del(entry->prev, entry->next);
	INIT_LIST_HEAD(entry);
}

static inline void list_del(struct list_head *entry)
{
	__list_del(entry->prev, entry->next);
	entry->prev = LIST_POS1;
	entry->next = LIST_POS2;
}

static inline void list_move(struct list_head *list, struct list_head *head)
{
	__list_del(list->prev, list->next);
	list_add(list, head);
}

static inline void list_move_tail(struct list_head *list, struct list_head *head)
{
	__list_del(list->prev, list->next);
	list_add_tail(list, head);
}

static inline void list_splice_init(struct list_head *list, struct list_head *head)
{
	if(!list_empty(list)) {
		__list_splice(list, head);
		INIT_LIST_HEAD(list);
	}
}

#define list_entry(ptr, type, member) container_of(ptr, type, member)

#define list_for_each(loc, head) \
  for (loc = (head)->next; loc != (head); loc = loc->next)

#define __list_for_each(loc, head) \
	for (loc = (head)->next; loc != (head); loc = loc->next)

#define list_for_each_prev(loc, head) \
	for (loc = (head)->prev; prefetch(loc->prev), loc != (head); loc = loc->prev)

#define list_for_each_safe(loc, n, head) \
	for (loc = (head)->next, n = loc->next; loc != (head); loc = n, n = loc->next)

#define list_for_each_entry_reverse(loc, head, member)			\
	for (loc = list_entry((head)->prev, typeof(*loc), member);	\
	     &loc->member != (head); 	\
	     loc = list_entry(loc->member.prev, typeof(*loc), member))

#define list_for_each_entry(loc, head, member)				\
	for (loc = list_entry((head)->next, typeof(*loc), member);	\
	     &loc->member != (head);					\
	     loc = list_entry(loc->member.next, typeof(*loc), member))

#define list_prepare_entry(loc, head, member) \
	((loc) ? : list_entry(head, typeof(*loc), member))

#define list_for_each_entry_safe(loc, n, head, member)			\
	for (loc = list_entry((head)->next, typeof(*loc), member),	\
		n = list_entry(loc->member.next, typeof(*loc), member);	\
	     &loc->member != (head); 					\
	     loc = n, n = list_entry(n->member.next, typeof(*n), member))

#endif
