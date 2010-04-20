/* 
   example_test.c            - Chul-Woong Yang (cwyang@gmail.com)

   example test code to demonstrate c2unit
*/

TEST(foo, 1, "test function for foo()")  // name, no, desc
{
        c2_assert(foo(1) == 1);
        c2_assert(foo(2) == 2);
        c2_assert(foo(3) == 3);
}
TEST_END(foo, 1)

TEST(foo, 2, "another test function for foo()")
{
        c2_assert(foo(4) == 4);
}
TEST_END(foo, 2)

TEST_FUNC(baz, 1,"test function for baz()", 2)  // name, no, dex, pri
{
        c2_assert(baz(1) == 2);
        c2_assert(baz(2) == 3);
        c2_assert(baz(3) == 3);
}
TEST_END(baz, 1)
