/* 
   c2unit.h            - Chul-Woong Yang (cwyang@gmail.com)

   Yet another C unit testing framework, ChulWoong's CUnit

   Much stuffs are borrowed from the code of Tejun Heo. Many Thanks.
*/

#ifndef _C2UNIT_H_
#define _C2UNIT_H_
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* list stuffs by */

#define c2_prefetch(x)	((void)0)
struct c2_list_head {
	struct c2_list_head *next, *prev;
};
#define C2_LIST_HEAD_INIT(name) { &(name), &(name) }
#define C2_LIST_HEAD(name) \
	struct c2_list_head name = C2_LIST_HEAD_INIT(name)
static inline struct c2_list_head * init_c2_list_head(struct c2_list_head *head)
{
	head->next = head;
	head->prev = head;
	return head;
}
static __inline__ void __c2_list_add(struct c2_list_head * new,
	struct c2_list_head * prev,
	struct c2_list_head * next)
{
	next->prev = new;
	new->next = next;
	new->prev = prev;
	prev->next = new;
}
static __inline__ void c2_list_add(struct c2_list_head *new, struct c2_list_head *head)
{
	__c2_list_add(new, head, head->next);
}
static __inline__ void c2_list_add_tail(struct c2_list_head *new, struct c2_list_head *head)
{
	__c2_list_add(new, head->prev, head);
}
static __inline__ void __c2_list_del(struct c2_list_head * prev,
				  struct c2_list_head * next)
{
	next->prev = prev;
	prev->next = next;
}
static __inline__ void c2_list_del(struct c2_list_head *entry)
{
	__c2_list_del(entry->prev, entry->next);
}
static __inline__ void c2_list_del_init(struct c2_list_head *entry)
{
	__c2_list_del(entry->prev, entry->next);
	init_c2_list_head(entry); 
}
static __inline__ int c2_list_empty(struct c2_list_head *head)
{
	return head->next == head;
}
static __inline__ void c2_list_splice(struct c2_list_head *list, struct c2_list_head *head)
{
	struct c2_list_head *first = list->next;

	if (first != list) {
		struct c2_list_head *last = list->prev;
		struct c2_list_head *at = head->next;

		first->prev = head;
		head->next = first;

		last->next = at;
		at->prev = last;
	}
}
#define c2_list_entry(ptr, type, member) \
	((type *)((char *)(ptr)-(unsigned long)(&((type *)0)->member)))
#define c2_list_for_each(pos, head) \
	for (pos = (head)->next, c2_prefetch(pos->next); pos != (head); \
        	pos = pos->next, c2_prefetch(pos->next))
#define c2_list_for_each_safe(pos, n, head) \
	for (pos = (head)->next, n = pos->next; pos != (head); \
		pos = n, n = pos->next)
#define c2_list_for_each_bw(pos, head) \
	for (pos = (head)->prev, c2_prefetch(pos->prev); pos != (head); \
		pos = pos->prev, c2_prefetch(pos->prev))
#define c2_list_for_each_bw_safe(pos, p, head) \
	for (pos = (head)->prev, p = pos->prev; pos != (head); \
		pos = p, p = pos->prev)

/* gcc stuffs */
#define __packed                        __attribute__((packed))
#define __aligned(align)                __attribute__((aligned(align)))
#define __section(sect)                 __attribute__((section(sect)))
#define __printf_format(s, f)           __attribute__((format(printf,s,f)))
#define __scanf_format(s, f)            __attribute__((format(scanf,s,f)))
#define __noprof                        __attribute__((no_instrument_function))
#define __noret                         __attribute__((noreturn))
#define __used                          __attribute__((used))
#define __weak                          __attribute__((weak))
#define __asmlinkage                    __attribute__((regparm(0)))
#define c2_alignof(x)                   __alignof__(x)

/* util stuffs */
#define c2_stringify(s) c2_stringfy1(s)
#define c2_stringfy1(s) #s
#define c2_unique_id(prefix)         c2_unique_id_1(prefix, __LINE__)
#define c2_unique_id_1(prefix, line) c2_unique_id_2(prefix, line)
#define c2_unique_id_2(prefix, line) prefix##_##line

/* main stuffs */
enum c2_node_type {
    C2_FUNC = 1,
    C2_TEST = 2
};
struct c2_si_ent 
{
        enum c2_node_type          type;
        void *              ptr;
};
#define __C2_SI_INIT(Init, InitArg) { (Init), (InitArg) }
#define __C2_SYS_INIT(Init, InitArg) \
        static struct c2_si_ent c2_unique_id(c2_si_ent) \
        __section(".c2") __aligned(c2_alignof(struct c2_si_ent)) __used = \
        __C2_SI_INIT(Init, InitArg)

#define __C2_FUNC_ID          c2_unique_id(__c2_func)
#define __C2_FUNC_BEGIN_ID(f) __c2_func_##f
#define __C2_FUNC_BEGIN_INIT(Name, Path, Level, Line)              \
        { .name = (Name), .file = __FILE__, .path = (Path), .level = (Level), .line = (Line) }
#define __C2_FUNC_REGISTER \
        __C2_SYS_INIT(C2_FUNC, &__C2_FUNC_ID)

enum c2_func_level {
        SAFE = 1,
        NORMAL = 2,
        DANGER = 3,
};

#define FUNC_BEGIN(f,x) \
        static struct c2_func __C2_FUNC_BEGIN_ID(f) =                      \
                __C2_FUNC_BEGIN_INIT(#f,(C2UNIT_TEST_PATH),(x),__LINE__);
#define FUNC_END(f)                                                  \
        static struct c2_func_wrap __C2_FUNC_ID =                      \
		{ .begin = &__C2_FUNC_BEGIN_ID(f), .end_line = __LINE__ }; __C2_FUNC_REGISTER;

struct c2_func
{
        char *name;
        char *file;
        char *path;
        enum c2_func_level level;
        size_t line;
        int has_test;
	struct c2_list_head link;
	struct c2_list_head hash_link;
};
struct c2_func_wrap
{
        struct c2_func *begin;
        size_t end_line;
};

struct c2_test
{
        char *name;
        char *desc;
        char *file;
        char *path;
        int pri;
        int has_func;
	struct c2_list_head link;
	struct c2_list_head hash_link;
};
#define __C2_TEST_ID          c2_unique_id(__c2_test)
#define __C2_TEST_INIT(Name, Desc, Path, Pri)                           \
        { .name = (Name), .file = __FILE__, .desc = (Desc), .path = (Path), .pri = (Pri) }
#define __C2_TEST_REGISTER \
        __C2_SYS_INIT(C2_TEST, &__C2_TEST_ID)

#define TEST(f,desc) TEST_FUNC(f,1,desc)
#define TEST_FUNC(f,pri,desc)                                               \
        static struct c2_test __C2_TEST_ID =                            \
                __C2_TEST_INIT(#f,desc, (C2UNIT_TEST_PATH),(pri));      \
        __C2_TEST_REGISTER;                                             \
        void (f##_test) (void)

#define c2_assert(x) do {                                               \
                if (!(x)) {                                             \
                        fprintf(stderr, "Assertion failure @ %s:%d\n\"%s\" does not hold\n", \
                                __FILE__, __LINE__, #x);                \
                }                                                       \
        } while (0)
#define c2_panic(x) do {                                                \
                fprintf(stderr, "*** PANIC *** %s:%d <%s>\n",           \
                        __FILE__, __LINE__, x);                         \
                exit(1);                                                \
        } while (0)

typedef void (*c2_testfn_t)(void);

#ifdef C2UNIT_TEST_PATH
#undef C2UNIT_TEST_PATH
#endif
#define C2UNIT_TEST_PATH "" // no use yet
#endif
