#include <stdion.h>

int main() {
    int x;
    int a[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    scanf("%d", &x);
    a[x] = -10;
    printf("a[%d] = %d\n", x, a[x]);

}
