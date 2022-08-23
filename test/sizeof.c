#include "test.h"

int main()
{
    // clang-format off
    { int x; ASSERT(4, sizeof(x)); }
    { int x; ASSERT(4, sizeof x); }
    { int* x; ASSERT(8, sizeof(x)); }
    { int x[4]; ASSERT(16, sizeof(x)); }
    { int x[3][4]; ASSERT(48, sizeof(x)); }
    { int x[3][4]; ASSERT(16, sizeof(*x)); }
    { int x[3][4]; ASSERT(4, sizeof(**x)); }
    { int x[3][4]; ASSERT(5, sizeof(**x) + 1); }
    { int x[3][4]; ASSERT(5, sizeof **x + 1); }
    { int x[3][4]; ASSERT(4, sizeof(**x + 1)); }
    { int x = 1; ASSERT(4, sizeof(x = 2)); }
    { int x = 1; sizeof(x = 2); ASSERT(1, x); }
    // clang-format on

    ASSERT(1, sizeof(char));
    ASSERT(2, sizeof(short));
    ASSERT(2, sizeof(short int));
    ASSERT(2, sizeof(int short));
    ASSERT(4, sizeof(int));
    ASSERT(8, sizeof(long));
    ASSERT(8, sizeof(long int));
    ASSERT(8, sizeof(long int));
    ASSERT(8, sizeof(char*));
    ASSERT(8, sizeof(int*));
    ASSERT(8, sizeof(long*));
    ASSERT(8, sizeof(int**));
    // ASSERT(8, sizeof(int (*)[4]));
    ASSERT(32, sizeof(int* [4]));
    ASSERT(16, sizeof(int[4]));
    ASSERT(48, sizeof(int[3][4]));
    ASSERT(8, sizeof(struct {int a; int b; }));

    ASSERT(8, sizeof(-10 + (long)5));
    ASSERT(8, sizeof(-10 - (long)5));
    ASSERT(8, sizeof(-10 * (long)5));
    ASSERT(8, sizeof(-10 / (long)5));
    ASSERT(8, sizeof((long)-10 + 5));
    ASSERT(8, sizeof((long)-10 - 5));
    ASSERT(8, sizeof((long)-10 * 5));
    ASSERT(8, sizeof((long)-10 / 5));

    {
        int i = 1;
        sizeof(i *= 8);
        ASSERT(1, i);
        ASSERT(4, sizeof(i *= 8));
    }
    {
        char i = 1;
        sizeof(++i);
        ASSERT(1, i);
        ASSERT(1, sizeof(++i));
        ASSERT(1, sizeof(i++));
        ASSERT(1, sizeof(--i));
        ASSERT(1, sizeof(i--));
    }

    printf("OK\n");
    return 0;
}
