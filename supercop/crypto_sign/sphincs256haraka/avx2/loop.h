/////////////////
// FOR LOOPING //
/////////////////
#define TIMES1(n) TIMES0((n)) TIMES0(((n)+1))
#define TIMES2(n) TIMES1((n)) TIMES1(((n)+2))
#define TIMES3(n) TIMES2((n)) TIMES2(((n)+4))
#define TIMES4(n) TIMES3((n)) TIMES3(((n)+8))
#define TIMES5(n) TIMES4((n)) TIMES4(((n)+16))
#define TIMES6(n) TIMES5((n)) TIMES5(((n)+32))
#define TIMES7(n) TIMES6((n)) TIMES6(((n)+64))
#define TIMES8(n) TIMES7((n)) TIMES7(((n)+128))

#define X1bit(n,start) if ((n)&1) { TIMES0((start)) }
#define X2bit(n,start) if ((n)&2) { TIMES1((start)) } X1bit((n)&1, (start)+((n)&2))
#define X3bit(n,start) if ((n)&4) { TIMES2((start)) } X2bit((n)&3, (start)+((n)&4))
#define X4bit(n,start) if ((n)&8) { TIMES3((start)) } X3bit((n)&7, (start)+((n)&8))
/* NOTE: 0 <= n < 32  as of now */
#define REPEAT_THIS(n) if ((n)&16) { TIMES4(0) } X4bit((n)&0xf, (n)&16)
