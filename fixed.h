#ifndef FIXED_H__
#define FIXED_H__

#include <stddef.h>

typedef int32_t fixed_t;

#define FIXED_SHIFT_BITS    16
#define FIXED_UNIT          (1<<FIXED_SHIFT_BITS)

fixed_t	FixedMul(fixed_t a, fixed_t b);
fixed_t	FixedDiv(fixed_t a, fixed_t b);
#define FixedMul2(c,a,b) \
       __asm volatile ( \
            "dmuls.l %1, %2\n\t" \
            "sts mach, r1\n\t" \
            "sts macl, r0\n\t" \
            "xtrct r1, r0\n\t" \
            "mov r0, %0\n\t" \
            : "=r" (c) \
            : "r" (a), "r" (b) \
            : "r0", "r1", "mach", "macl")
fixed_t IDiv(fixed_t a, fixed_t b);

#endif
