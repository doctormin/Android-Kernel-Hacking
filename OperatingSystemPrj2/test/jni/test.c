#include <sys/syscall.h>
#include <stdio.h>
#define mySys 59

int main(){

    long number = syscall(mySys, 11000, 1000);

    return 0;
}


