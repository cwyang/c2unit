#include <stdio.h>
#include "c2unit.h"

TEST(foo,"test function for foo()") 
{
        assert(foo(1) == 1);
        assert(foo(2) == 3);
        assert(foo(3) == 3);
}
