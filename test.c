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


TEST(foo,"test function for foo()") 
{
        assert(foo(1) == 1);
        assert(foo(2) == 3);
        assert(foo(3) == 3);
}

main(int argc, char *argv[]) 
{
        printf("cw's unit test program!\n");
        foo_test();
	test_run(argc, argv);
}

