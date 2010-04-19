#include <stdio.h>
#include "c2unit.h"

TEST(foo,"test function for foo()") 
{
        c2_assert(foo(1) == 1);
        c2_assert(foo(2) == 3);
        c2_assert(foo(3) == 3);
}
