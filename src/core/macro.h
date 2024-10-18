#pragma ocne

#define ENABLED(V...)       DO(ENA,&&,V)
#define __BV32(b)           (1UL << (b))
#define SBIT(n,b)           (n |= __BV32(b))
#define CBIT(n,b)           (n &= ~__BV32(b))
#define TBIT(n,b)           !!(n & __BV32(b))


