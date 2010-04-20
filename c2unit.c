/* 
   c2unit.c            - Chul-Woong Yang (cwyang@gmail.com)

   Yet another C unit testing framework, ChulWoong's CUnit
*/

#ifdef __MCT
#include <mct.h>
#else
#include <stdint.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#endif
#include "c2unit.h"

#define DFL_HASH_SIZE 2011 // prime number

static struct c2_si_ent c2_end_marker __section(".c2") __aligned(c2_alignof(struct c2_si_ent));
static struct c2_list_head func_head, test_head;
static struct c2_list_head *func_hash, *test_hash;
static unsigned hash_size;
struct c2_stat __c2_prog_stat;

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
        __c2_prog_stat.nr_func++;
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
        struct c2_test_wrap *x = (struct c2_test_wrap *) ent->ptr;
	struct c2_test *f = x->begin;
        
        f->testfn = x->testfn;
        f->file = strdup(f->file); // intentional leak
        trim_filename(f->file);
        f->has_func = 0;
	init_c2_list_head(&f->link);
	init_c2_list_head(&f->hash_link);
	c2_list_add_tail(&f->link, &test_head);
	c2_list_add_tail(&f->hash_link, 
                         &test_hash[hashval(f->name,f->file,f->path) % 
                                    hash_size]);
        __c2_prog_stat.nr_test++;
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

static void c2_match_test_func(void) 
{
	struct c2_func *f;
	struct c2_test *t;
	struct c2_list_head *le;
        
	c2_list_for_each(le, &func_head) {
		f = c2_list_entry(le, struct c2_func, link);
                f->has_test = find_test(f->name, f->file, f->path);
                if (f->has_test)
                        __c2_prog_stat.nr_tested_func++;
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

// cwyang's heuristics
// Code safety is proportional to the square of code lines.
// Safe code is 2 time more reliable than normal code, and
// dangerous code is 5 time more dangerous than normal code.
//
// returns 0 ~ 1000000

static void calc_score(void) 
{
        int total, tested, s;
	struct c2_func *f;
	struct c2_list_head *le;
        
        total = tested = 0;
	c2_list_for_each(le, &func_head) {
		f = c2_list_entry(le, struct c2_func, link);
                s = f->line * f->line;
                total += s;
                if (f->has_test == 0)
                        continue;
                switch(f->level) {
                case DANGER: s /= 5;
                case NORMAL: s /= 2;
                case SAFE: break;
                default: s = 0;
                }
                tested += s;
        }
	if (total != 0)
            __c2_prog_stat.prog_score = tested * 1000000 / total;
	else
            __c2_prog_stat.prog_score = 0;
}

static void c2_dump(void)
{
	struct c2_func *f;
	struct c2_test *t;
	struct c2_list_head *le;

	printf("<FUNC DUMP>\n");

	c2_list_for_each(le, &func_head) {
		f = c2_list_entry(le, struct c2_func, link);
		printf("name:%-20s (%s:%-10s)  line:%4lu  level:%d  has_test:%d\n",
                       f->name, f->path, f->file, f->line, f->level, f->has_test);
	}
	printf("<TEST DUMP>\n");

	c2_list_for_each(le, &test_head) {
		t = c2_list_entry(le, struct c2_test, link);
		printf("name:%-20s (%s:%-10s)  pri:%d  has_func:%d  desc:%s\n",
                       t->name, t->path, t->file, t->pri, t->has_func, t->desc);
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

static void stat_init(void)
{
        __c2_prog_stat.nr_func = 0;
        __c2_prog_stat.nr_test = 0;
        __c2_prog_stat.nr_tested_func = 0;
        __c2_prog_stat.nr_assert = 0;
        __c2_prog_stat.pass_assert = 0;
        __c2_prog_stat.pass_test = 0;
        __c2_prog_stat.prog_score = 0;
        __c2_prog_stat.test_pri = 1;
        __c2_prog_stat.test_dump_core = 0;
        __c2_prog_stat.test_dump_info = 0;
        __c2_prog_stat.test_verbose = 0;
        __c2_prog_stat.test_path = "";
}

static void test_usage(void)
{
        const char *help =
"Usage: exec_prog <options>                                             \n"
"                                                                       \n"
" The options are:                                                      \n"
//"  -P <test_path>          test path (no support yet)                   \n"
"  -p <test_priority>      test priority (1~3)                          \n"
"  -d                      dump test and function information and exit  \n"
"  -c                      dump core when assert fails                  \n"
"  -v                      be verbose                                   \n"
"                                                                       \n";
        fprintf(stderr, help);
}
static void parse_arg(int argc, char *argv[]) 
{
        struct c2_stat *p = &__c2_prog_stat;
        int c;
        
        while ((c = getopt(argc, argv, "P:p:dcvt")) != -1)
                switch (c) {
                case 'P': p->test_path = optarg; break;
                case 'p': p->test_pri  = atoi(optarg); break;
                case 'c': p->test_dump_core = 1; break;
                case 'd': p->test_dump_info = 1; break;
                case 'v': p->test_verbose = 1;          break;
                case 't': break;
                        
                default:
                        test_usage();
                        __c2_exit(0);
                }

}

static void test_init(int argc, char *argv[])
{
	extern struct c2_si_ent c2_beg_marker; // from firstlink.c
	struct c2_si_ent *start_ent, *end_ent;

	hash_init();
        stat_init();

        parse_arg(argc, argv);

	start_ent = &c2_beg_marker + 1;
	end_ent = &c2_end_marker;

	init_c2_list_head(&func_head);
	init_c2_list_head(&test_head);
	c2_env_build(start_ent, end_ent);
        calc_score();
}

// remove trailing zero. i.e. 2100 -> 21
static int trim_int(int n) 
{
        while (n != 0 && (n / 10) * 10 == n)
                n = n / 10;
        return n;
}

static void print_stat(void) 
{
        struct c2_stat *p = &__c2_prog_stat;
        printf("\n<STATISTICS>\n");
        printf("Test priority is %d, test path is [%s].\n",
               p->test_pri, p->test_path);
        printf("%d functions out of %d are test-covered.\n",
               p->nr_tested_func, p->nr_func);
        printf("%d tests are passed from total %d tests.\n",
               p->pass_test, p->nr_test);
        printf("%d asserts are passed from total %d asserts.\n",
               p->pass_assert, p->nr_assert);
        printf("Total duration is %d seconds, took %d seconds per a test on average.\n", 0, 0);
        printf("The longest test took %d seconds, %s/%s/%s (%s).\n",
               0, "name", "file", "path", "desc");
        printf("Program score is %d.%d.\n\n",
               p->prog_score / 10000, trim_int(p->prog_score % 10000));
}

void test_run(int argc, char *argv[]) 
{
	struct c2_test *t;
	struct c2_list_head *le;
        struct c2_stat *p = &__c2_prog_stat;
        int no = 1;
        

        test_init(argc, argv);
        if (__c2_prog_stat.test_dump_info) {
                c2_dump();
                __c2_exit(0);
        }

	c2_list_for_each(le, &test_head) {
		t = c2_list_entry(le, struct c2_test, link);
                if (p->test_verbose)
                        fprintf(stderr, "[%03d] %s() in %s [%s:%s]...",
                                no, t->name, t->file, t->path, t->desc);
                t->testfn();
                if (p->test_verbose)
                        fprintf(stderr, "OK\n");
                p->pass_test++;
                no++;
	}
        print_stat();
}

void __c2_exit(int rc) 
{
        struct c2_stat *p = &__c2_prog_stat;

        if (p->test_dump_core == 0 || rc == 0) {
#ifdef __MCT
		die(0);
#else
		exit(rc);
#endif
        }
#ifdef __MCT 
		die(1);
#else
        kill(getpid(), SIGABRT);
#endif
}
