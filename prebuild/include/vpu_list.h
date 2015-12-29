#ifndef __VPU_LIST_H__
#define __VPU_LIST_H__

#ifdef __cplusplus
extern "C" {
#endif
struct list_head
{

    struct list_head *next, *prev;
};

#define LIST_HEAD_INIT(name) { &(name), &(name) }

#define LIST_HEAD(name) \
    struct list_head name = LIST_HEAD_INIT(name)

#define INIT_LIST_HEAD(ptr) do { \
        (ptr)->next = (ptr); (ptr)->prev = (ptr); \
    } while (0)

#define list_for_each_safe(pos, n, head) \
    for (pos = (head)->next, n = pos->next; pos != (head); \
            pos = n, n = pos->next)

#define list_entry(ptr, type, member) \
    ((type *)((char *)(ptr)-(unsigned long)(&((type *)0)->member)))

#define list_for_each_entry(pos, head, member)              \
    for (pos = list_entry((head)->next, typeof(*pos), member);  \
         &pos->member != (head);    \
         pos = list_entry(pos->member.next, typeof(*pos), member))
#define list_for_each_entry_safe(pos, n, head, member)          \
    for (pos = list_entry((head)->next, typeof(*pos), member),  \
        n = list_entry(pos->member.next, typeof(*pos), member); \
         &pos->member != (head);                    \
         pos = n, n = list_entry(n->member.next, typeof(*n), member))
static __inline__ void __list_add(struct list_head * _new,
                                  struct list_head * prev,
                                  struct list_head * next)
{
    next->prev = _new;
    _new->next = next;
    _new->prev = prev;
    prev->next = _new;
}

static inline void list_add(struct list_head *_new, struct list_head *head)
{
    __list_add(_new, head, head->next);
}

static __inline__ void list_add_tail(struct list_head *_new, struct list_head *head)
{
    __list_add(_new, head->prev, head);
}

static __inline__ void __list_del(struct list_head * prev,
                                  struct list_head * next)
{
    next->prev = prev;
    prev->next = next;
}

static __inline__ void list_del_init(struct list_head *entry)
{
    __list_del(entry->prev, entry->next);

    INIT_LIST_HEAD(entry);
}

static inline int list_is_last(const struct list_head *list, const struct list_head *head)
{
    return list->next == head;
}

static __inline__ int list_empty(struct list_head *head)
{
    return head->next == head;
}

#ifdef __cplusplus
}
#endif
#endif /* __VPU_LIST_H__ */
