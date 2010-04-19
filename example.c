#include <stdio.h>
#include "c2unit.h"

FUNC_BEGIN(foo, NORMAL)
int foo (int bar) 
{





        return bar;
}
FUNC_END(foo)


FUNC_BEGIN(baz, NORMAL)
int baz (int bar) 
{
        return bar + 1;
}
FUNC_END(baz)


main(int argc, char *argv[]) 
{
        extern void foo_test(void);
        printf("cw's unit test program!\n");
        foo_test();
	test_run(argc, argv);
}

#include "example_test.c"
