// Kernel-style double-linked list implementation.
// Inspired from research code I wrote during my PhD thesis.
// https://github.com/target0/thesis-data/blob/master/sdnres-src/lib/llist.h

#ifndef _LLIST_H
#define _LLIST_H

#include <stddef.h>
#include <stdbool.h>

struct llist_head {
    struct llist_head *prev;
    struct llist_head *next;
};

#define container_of(ptr, type, member) ({            \
    const typeof( ((type *)0)->member ) *__mptr = (ptr);    \
    (type *)( (char *)__mptr - offsetof(type, member) ); })

#define LLIST_INIT(head)    \
{                \
    (head)->next = head;    \
    (head)->prev = head;    \
}

#define LLIST_INSERT_TAIL(head, elem)    \
{                    \
    (elem)->next = (head);        \
    (elem)->prev = (head)->prev;    \
    (head)->prev->next = (elem);    \
    (head)->prev = (elem);        \
}

#define LLIST_INsERT_HEAD(head, elem)    \
{                    \
    (elem)->next = (head)->next;    \
    (elem)->prev = (head);        \
    (head)->next->prev = (elem);    \
    (head)->next = (elem);        \
}

#define LLIST_REMOVE(elem)            \
{                        \
    (elem)->next->prev = (elem)->prev;    \
    (elem)->prev->next = (elem)->next;    \
    (elem)->next = (elem);            \
    (elem)->prev = (elem);            \
}

#define LLIST_EMPTY(head) ((head)->next == (head))

#define LLIST_ENTRY(head, type, member)    \
    container_of(head, type, member)

#define LLIST_FIRST_ENTRY(head, type, member) \
    LLIST_ENTRY((head)->next, type, member)

#define LLIST_NEXT_ENTRY(elem, member)    \
    LLIST_ENTRY((elem)->member.next, typeof(*(elem)), member)

#define LLIST_LAST_ENTRY(head, type, member) \
    LLIST_ENTRY((head)->prev, type, member)

#define LLIST_FOREACH_ENTRY(head, elem, member)                 \
    for ((elem) = LLIST_FIRST_ENTRY(head, typeof(*(elem)), member);    \
         &((elem)->member) != (head);                \
         (elem) = LLIST_NEXT_ENTRY(elem, member))

#define LLIST_FOREACH_ENTRY_EXT(head, elem, member, ext)                 \
    for ((elem) = LLIST_FIRST_ENTRY(head, typeof(*(elem)), member), ext;    \
         &((elem)->member) != (head);                \
         (elem) = LLIST_NEXT_ENTRY(elem, member), ext)

#define LLIST_FOREACH_ENTRY_SAFE(head, elem, tmp, member)            \
    for ((elem) = LLIST_FIRST_ENTRY(head, typeof(*(elem)), member),    \
         (tmp) = LLIST_NEXT_ENTRY(elem, member);            \
         &((elem)->member) != (head);                \
         (elem) = (tmp), (tmp) = LLIST_NEXT_ENTRY(tmp, member))

#define LLIST_FOREACH_ENTRY_SAFE_EXT(head, elem, tmp, member, ext)            \
    for ((elem) = LLIST_FIRST_ENTRY(head, typeof(*(elem)), member),    \
         (tmp) = LLIST_NEXT_ENTRY(elem, member), ext;            \
         &((elem)->member) != (head);                \
         (elem) = (tmp), (tmp) = LLIST_NEXT_ENTRY(tmp, member), ext)

#endif
