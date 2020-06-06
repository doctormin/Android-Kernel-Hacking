#include <stdio.h>
#include "test.h"


int main(){
	int i, j;
	extern struct Yimin_MMLimits Yimin_mm_limits;
	for(i = 0; i < 200; i++){
		for(j = 0; j < 2; j++){
			printf("%ld ", Yimin_mm_limits.mm_entries[i][j]);
		}
		printf("\n");
	}
	return 0;
}
