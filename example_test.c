/* 
   example_test.c            - Chul-Woong Yang (cwyang@gmail.com)

   example test code to demonstrate c2unit
*/
#include <stdio.h>
#include "c2unit.h"

TEST(foo,"test function for foo()") 
{
        c2_assert(foo(1) == 1);
        c2_assert(foo(2) == 3);
        c2_assert(foo(3) == 3);
}
TEST_END(foo)
