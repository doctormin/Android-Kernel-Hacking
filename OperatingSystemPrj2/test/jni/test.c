#include <sys/syscall.h>
#include <stdio.h>
#define HelloWorld 59

int main(){
    long number = syscall(HelloWorld);
    printf("%d", number);
}


