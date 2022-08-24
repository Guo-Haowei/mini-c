#include "../src/generic/array.h"

#include <stdio.h>
#include <stdlib.h>

static void _expect(int ok, int line, char* msg)
{
    if (!ok) {
        printf("Assertion: %s failed on line %d\n", msg, line);
        exit(1);
    }
}

#define expect(expr) _expect(!!(expr), __LINE__, #expr)

static void print_array(Array* array)
{
    printf("[");
    for (int i = 0; i < array->len; ++i) {
        int a = *array_at(int, array, i);
        printf("%d,", a);
    }
    printf("]\n");
}

int array_test()
{
    printf("running array test...\n");
    Array _arr;
    Array* arr = &_arr;

    int target = 16;
    array_init(arr, sizeof(int), 8);

    for (int i = 0; i < target; ++i) {
        int a = target - i;
        array_push_back(int, arr, a);
    }

    expect(arr->capacity == target);
    expect(arr->len == target);
    expect(arr->eleSize == sizeof(int));
    for (int i = 0; i < target; ++i) {
        expect(target - i == *array_at(int, arr, i));
    }

    print_array(arr);
    array_clear(arr);
    printf("array test passed\n");
    return 0;
}

#ifndef NO_MAIN
int main()
{
    return array_test();
}
#endif
