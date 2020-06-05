#include <sys/syscall.h>
#include <stdio.h>
#define mySys 59

int main(){
    long number = syscall(mySys, 100, 100000);
    printf("%d", number);
}


