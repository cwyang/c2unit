/* 
   example.c            - Chul-Woong Yang (cwyang@gmail.com)

   example code to demonstrate c2unit
*/
#include <stdio.h>
#include <unistd.h>

#define C2UNIT_TEST_PATH "sample"
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
        int c;

        // if -t then do unit tests
        while ((c = getopt(argc, argv, "t")) != -1)
                switch (c) {
                case 't': test_run(argc, argv);
                        exit(0);
                }

        // otherwise do normal tasks
        printf("Hello, world!\n");
        printf("Welcome to cwyang's first unit test program!\n");
}

#include "example_test.c"
