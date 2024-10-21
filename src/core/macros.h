#pragma ocne

#define ENABLED(V...)       DO(ENA,&&,V)
#define __BV32(b)           (1UL << (b))
#define SBIT(n,b)           (n |= __BV32(b))
#define CBIT(n,b)           (n &= ~__BV32(b))
#define TBIT(n,b)           !!(n & __BV32(b))
#define GPIO_CFG_NUN(a)     ((gpio_num_t)a)

#define _BV(n) (1<<(n))
#define TEST(n,b) (!!((n)&_BV(b)))
// Value helper macros
#define WITHIN(N,L,H)       ((N) >= (L) && (N) <= (H))
#define ISEOL(C)            ((C) == '\n' || (C) == '\r')
#define NUMERIC(a)          WITHIN(a, '0', '9')
#define DECIMAL(a)          (NUMERIC(a) || a == '.')
#define HEXCHR(a)           (NUMERIC(a) ? (a) - '0' : WITHIN(a, 'a', 'f') ? ((a) - 'a' + 10)  : WITHIN(a, 'A', 'F') ? ((a) - 'A' + 10) : -1)
#define NUMERIC_SIGNED(a)   (NUMERIC(a) || (a) == '-' || (a) == '+')
#define DECIMAL_SIGNED(a)   (DECIMAL(a) || (a) == '-' || (a) == '+')
