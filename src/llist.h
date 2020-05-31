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

#define llist_init(head)    \
{                \
    (head)->next = head;    \
    (head)->prev = head;    \
}

#define llist_insert_tail(head, elem)    \
{                    \
    (elem)->next = (head);        \
    (elem)->prev = (head)->prev;    \
    (head)->prev->next = (elem);    \
    (head)->prev = (elem);        \
}

#define llist_insert_head(head, elem)    \
{                    \
    (elem)->next = (head)->next;    \
    (elem)->prev = (head);        \
    (head)->next->prev = (elem);    \
    (head)->next = (elem);        \
}

#define llist_remove(elem)            \
{                        \
    (elem)->next->prev = (elem)->prev;    \
    (elem)->prev->next = (elem)->next;    \
    (elem)->next = (elem);            \
    (elem)->prev = (elem);            \
}

#define llist_empty(head) ((head)->next == (head))

#define llist_entry(head, type, member)    \
    container_of(head, type, member)

#define llist_first_entry(head, type, member) \
    llist_entry((head)->next, type, member)

#define llist_next_entry(elem, member)    \
    llist_entry((elem)->member.next, typeof(*(elem)), member)

#define llist_last_entry(head, type, member) \
    llist_entry((head)->prev, type, member)

#define llist_foreach(head, elem, member)                 \
    for ((elem) = llist_first_entry(head, typeof(*(elem)), member);    \
         &((elem)->member) != (head);                \
         (elem) = llist_next_entry(elem, member))

#define llist_foreach_safe(head, elem, tmp, member)            \
    for ((elem) = llist_first_entry(head, typeof(*(elem)), member),    \
         (tmp) = llist_next_entry(elem, member);            \
         &((elem)->member) != (head);                \
         (elem) = (tmp), (tmp) = llist_next_entry(tmp, member))

#endif
