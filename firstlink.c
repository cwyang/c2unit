/* 
   firstlink.c            - Chul-Woong Yang (cwyang@gmail.com)

   link this object file before any other object files or library files.
*/

#include <stdio.h>
#include "c2unit.h"

struct c2_si_ent c2_si_begin_marker __section(".c2") __aligned(c2_alignof(struct c2_si_ent));
