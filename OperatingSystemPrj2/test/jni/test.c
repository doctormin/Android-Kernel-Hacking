#include <sys/syscall.h>
#include <stdio.h>
#define mySys 59

int main(){
    long number = syscall(mySys, 100, 100000);

    //number = syscall(mySys, 10, 1000);
    //number = syscall(mySys, 99, 1023200);
    //number = syscall(mySys, 100, 102300);
    return 0;
}


