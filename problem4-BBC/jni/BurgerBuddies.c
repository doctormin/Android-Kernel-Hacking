#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

int main(int argc, char ** argv){
    int cooks, cashiers, customers, racksize;
    if(argc != 5){
        //!exception - wrong input
        printf("input format should be \"BBC #Cooks #Cashiers #Customers #RackSize\"\n");
        return -1;
    }
    cooks = atoi(argv[1]);
    cashiers = atoi(argv[2]);
    customers = atoi(argv[3]);
    racksize = atoi(argv[4]);
    if (!(cooks && cashiers && customers && racksize)){
        //!exception - wrong input (atoi failed)
        printf("please ensure that \"#Cooks #Cashiers #Customers #RackSize\" are all positive numbers\n");
        return -1;
    }
    printf("Cooks [%d], Cashiers [%d], Customers [%d]\n", cooks, cashiers, customers);
    printf("Begin run.\n");

}