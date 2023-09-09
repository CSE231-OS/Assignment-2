#include <stdio.h>
#include <stdlib.h>

int fib(int n){
    if (n < 2) return n;
    return fib(n - 1) + fib(n - 2);
}

int main(int argc, char **argv)
{
    if (argc != 2) {
        fprintf(stderr, "Usage: %s n\n", argv[0]);
        exit(0);
    }
    int n = atoi(argv[1]);
    printf("%d\n", fib(n));
}