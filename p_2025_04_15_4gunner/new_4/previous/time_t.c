#include <stdio.h>
#include <time.h>

int main()
{
    time_t timer = time(NULL);
    struct tm* t = localtime(&timer);
    printf("%ld",timer);

    return 0;
}