#pragma ocne

#define ENABLED(V...)       DO(ENA,&&,V)
#define __BV32(b)           (1UL << (b))
#define SBIT(n,b)           (n |= __BV32(b))
#define CBIT(n,b)           (n &= ~__BV32(b))
#define TBIT(n,b)           !!(n & __BV32(b))
#define GPIO_CFG_NUN(a)     ((gpio_num_t)a)

#define _BV(n) (1<<(n))
#define TEST(n,b) (!!((n)&_BV(b)))