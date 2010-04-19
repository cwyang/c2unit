/* 
   c2unit.c            - Chul-Woong Yang (cwyang@gmail.com)

   Yet another C unit testing framework, ChulWoong's CUnit
*/

#include <stdint.h>
#include "c2unit.h"

#define DFL_HASH_SIZE 2011 // prime number

static struct c2_si_ent c2_si_end_marker __section(".c2") __aligned(c2_alignof(struct c2_si_ent));
static struct c2_list_head func_head, test_head;
static struct c2_list_head *func_hash, *test_hash;
static unsigned hash_size;

static unsigned int hashval(char *name, char *file, char *path) 
{
        const unsigned int shift = 6;
        const int mask = ~0U << (sizeof(int) * 8 - shift);
        unsigned int val;
        
        // we neglect file, path for now
        for (val = 0; *name != 0; ++name)
                val = (val & mask) ^ (val << shift) ^ *name;
        return val;
}

static void c2_func_ins(struct c2_si_ent *ent)
{
	struct c2_func_wrap *x = (struct c2_func_wrap *) ent->ptr;
	struct c2_func *f = x->begin;

	f->line = x->end_line - f->line; // function lines
        f->has_test = 0;
	init_c2_list_head(&f->link);
	init_c2_list_head(&f->hash_link);
	c2_list_add_tail(&f->link, &func_head);
	c2_list_add_tail(&f->hash_link, 
                         &func_hash[hashval(f->name,f->file,f->path) % 
                                    hash_size]);
}

/* if the base portion filename end with _test, cut it off */
#define TRIM_PATTERN "_test."
static void trim_filename(char *filename) 
{
        char *d, *s;
        if ((d = strstr(filename, TRIM_PATTERN)) == NULL)
                return;
        for (s = d + strlen(TRIM_PATTERN) - 1; *s != '\0'; s++, d++)
                *d = *s;
        *d = '\0';
}

static void c2_test_ins(struct c2_si_ent *ent)
{
	struct c2_test *f = (struct c2_test *) ent->ptr;

        f->file = strdup(f->file); // intentional leak
        trim_filename(f->file);
        f->has_func = 0;
	init_c2_list_head(&f->link);
	init_c2_list_head(&f->hash_link);
	c2_list_add_tail(&f->link, &test_head);
	c2_list_add_tail(&f->hash_link, 
                         &test_hash[hashval(f->name,f->file,f->path) % 
                                    hash_size]);
}

static int find_test(char *name, char *file, char *path) 
{
        int val = 0;
        struct c2_test *t;
        struct c2_list_head *le, *lh = &test_hash[hashval(name, file, path)
                                                  % hash_size ];

        c2_list_for_each(le, lh) {
                t = c2_list_entry(le, struct c2_test, hash_link);
                if (!strcmp(name,t->name) &&
                    !strcmp(file,t->file) &&
                    !strcmp(path,t->path))
                        val++;
        }
        return val;
}
static int find_func(char *name, char *file, char *path) 
{
        int val = 0;
        struct c2_func *f;
        struct c2_list_head *le, *lh = &func_hash[hashval(name, file, path)
                                                  % hash_size ];

        c2_list_for_each(le, lh) {
                f = c2_list_entry(le, struct c2_func, hash_link);
                if (!strcmp(name,f->name) &&
                    !strcmp(file,f->file) &&
                    !strcmp(path,f->path))
                        val++;
        }
        return val;
}

static void c2_match_test_func() 
{
	struct c2_func *f;
	struct c2_test *t;
	struct c2_list_head *le;
        
	c2_list_for_each(le, &func_head) {
		f = c2_list_entry(le, struct c2_func, link);
                f->has_test = find_test(f->name, f->file, f->path);
	}
	c2_list_for_each(le, &test_head) {
		t = c2_list_entry(le, struct c2_test, link);
                t->has_func = find_func(t->name, t->file, t->path);
	}
}

static void c2_env_build(struct c2_si_ent *begin, struct c2_si_ent *end)
{
	struct c2_si_ent *ent;

	for (ent = begin; ent != end; ent++) {
		if (ent->type == C2_FUNC)
			c2_func_ins(ent);
		else
			c2_test_ins(ent);
	}
        c2_match_test_func();
}

static void c2_dump(void)
{
	struct c2_func *f;
	struct c2_test *t;
	struct c2_list_head *le;

	printf("<func dump>\n");

	c2_list_for_each(le, &func_head) {
		f = c2_list_entry(le, struct c2_func, link);
		printf("name:%s, path:%s, file:%s, line:%d, level:%d has_test:%d\n",
                       f->name, f->path, f->file, f->line, f->level, f->has_test);
	}
	printf("<test dump>\n");

	c2_list_for_each(le, &test_head) {
		t = c2_list_entry(le, struct c2_test, link);
		printf("name:%s, desc:%s, path:%s, file:%s, pri:%d has_func:%d\n",
                       t->name, t->desc, t->path, t->file, t->pri, t->has_func);
	}
}

static void hash_init(void)
{
        int i;
        
        hash_size = DFL_HASH_SIZE;

        func_hash = malloc(hash_size * sizeof(struct c2_list_head));
        test_hash = malloc(hash_size * sizeof(struct c2_list_head));
        if (!func_hash || !test_hash) {
                c2_panic("failed to allocate initial hash table");
        }

        for (i = 0; i < hash_size; i++) {
                init_c2_list_head(&func_hash[i]);
                init_c2_list_head(&test_hash[i]);
        }
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
	c2_env_build(start_ent, end_ent);

	c2_dump();
}
    

void test_run(int argc, char *argv[]) 
{
    test_init();
}
