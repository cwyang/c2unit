/* 
   c2unit.c            - Chul-Woong Yang (cwyang@gmail.com)

   Yet another C unit testing framework, ChulWoong's CUnit
*/

#include <stdio.h>
#include <stdint.h>
#include "c2unit.h"

static struct c2_si_ent c2_si_end_marker __section(".c2") __aligned(c2_alignof(struct c2_si_ent));
static struct c2_list_head func_head, test_head;
static struct c2_list_head *func_hash, *test_hash;

static void __c2_func_ins(struct c2_si_ent *ent)
{
	struct c2_func_wrap *x = (struct c2_func_wrap *) ent->ptr;
	struct c2_func *f = x->begin;

	f->line = x->end_line - f->line; // function lines
	init_c2_list_head(&f->link);
	init_c2_list_head(&f->hash_link);
	c2_list_add_tail(&f->link, &func_head);
//	c2_list_add_tail(&f->hash_link, func_hash[hashval(f->name,f->file)]);
}

static void __c2_test_ins(struct c2_si_ent *ent)
{
}
static void __c2_env_build(struct c2_si_ent *begin, struct c2_si_ent *end)
{
	struct c2_si_ent *ent;
	for (ent = begin; ent != end; ent++) {
		if (ent->type == C2_FUNC)
			__c2_func_ins(ent);
		else
			__c2_test_ins(ent);
	}
}

static void __c2_func_dump(void)
{
	struct c2_func *f;
	struct c2_list_head *le;

	printf("<func dump>\n");

	c2_list_for_each(le, &func_head) {
		f = c2_list_entry(le, struct c2_func, link);
		printf("name:%s, path:%s file:%s, line:%d, level %d\n",
			f->name, f->path, f->file, f->line, f->level);
	}
}

static void hash_init(void)
{

}

static void test_init(void)
{
	extern struct c2_si_ent c2_si_begin_marker; // from firstlink.c
	struct c2_si_ent *start_ent, *end_ent;

	hash_init();
	start_ent = &c2_si_begin_marker + 1;
	end_ent = &c2_si_end_marker;

	init_c2_list_head(&func_head);
	init_c2_list_head(&test_head);
	__c2_env_build(start_ent, end_ent);

	__c2_func_dump();
}
    

void test_run(int argc, char *argv[]) 
{
    test_init();
}
