#include <stdlib.h>

long randlong() {
	long ret = rand();
	ret |= rand() << sizeof(int);
	return abs(ret);
}

void swap_ll(long long *first, long long *second) {
	long long tmp = *first;
	*first = *second;
	*second = tmp;
}

int partition(long long *arr, int elem_cnt) {
	int pivot_idx = randlong() % elem_cnt;
	long long pivot = arr[pivot_idx];

	long long *l = &arr[0];
	long long *r = &arr[elem_cnt - 1];
	while (1) {
		while (*l <  pivot && l < r) {
			++l;
		}
		while (*r >= pivot && l < r) {
			--r;
		}

		swap_ll(l, r);

		if (l >= r) {
			break;
		}
	}

	return (int) (l - arr);
}

int main() {


	return 0;
}