#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>

pthread_t *cook_thread, *cashier_thread, *customer_thread;
int num_of_cooks, num_of_cashiers, num_of_customers, racksize;
int cashier_waiting_for_burger;
int customer_served; //the number of customers that has been served
int burgers_on_rack; //the number of available burgers 
pthread_mutex_t customer_count; //mutex for accessing `customer_served`
pthread_mutex_t cook_mutex; //only one cook can check rack(and then make one burger) at a time
pthread_mutex_t accessing_rack;
pthread_mutex_t cashier_mutex;
sem_t all_served;
sem_t rack_not_full;
sem_t rack_not_empty;

/**
 * pthread_cancel_handler - the handler function for signal(SIGUSER1, handler)
 * because pthread_cancel() is not supported in Android
 * So pthread_kill() is used instead, and this handler will
 * response to SIGUSER1 that pthread_kill() generates
 */
void pthread_cancel_handler(int num){
    pthread_exit(0);
}
void *cook(void* num){
    int cook_id = *(int *) num;
    while(true){
        pthread_mutex_lock(&cook_mutex);

        //* Step1 - check the rack
        pthread_mutex_lock(&accessing_rack);
        if(burgers_on_rack == racksize){ //rack is full
            pthread_mutex_unlock(&accessing_rack); //enable cashiers to fetch burgers from the rack
            sem_wait(&rack_not_full); //will be signaled when a cashier give out one burger(before which the rack is full)
            pthread_mutex_lock(&accessing_rack);
        }

        //* Step2 - make the burger and put it on the rack
        sleep(cook_id/3);
        burgers_on_rack++;
        printf("Cook [%d] makes a burger.\n", cook_id);

        //* Step3 - signal rack_not_empty if the burger just made is the first one in the rack
        if(burgers_on_rack == 1 && cashier_waiting_for_burger){
            /*! 
             * when there are no cashiers waiting, 
             * even though the cook makes the first burger, 
             * he/she should not signal(rack_not_empty) !!!!
             * Reason:
             * If one cook come and make the first burger and signal(rack_not_empty)
             * Then two cashiers come, the first one will take the only burger
             * But the second one will pass wait(rack_not_empty) and also takes a burger
             * even though there are no burgers.
             * */
            sem_post(&rack_not_empty);
            cashier_waiting_for_burger--;
        }
        pthread_mutex_unlock(&accessing_rack);
        pthread_mutex_unlock(&cook_mutex);
    }
}
void *cashier(void* num){
    int cashier_id = *(int *) num;
    while(true){
        pthread_mutex_lock(&cashier_mutex);
        //TODO: Step1 - check customer_waiting ?
         
        //* Step2 -  fetch burger and serve
        pthread_mutex_lock(&accessing_rack);
        if(burgers_on_rack == 0){
            cashier_waiting_for_burger++; //showing that there are cashiers waiting 
            pthread_mutex_unlock(&accessing_rack); //enable cooks to put burgers on the rack
            sem_wait(rack_not_empty);
            pthread_mutex_lock(&accessing_rack);
        }
        burgers_on_rack--;
        printf("Cashier [%d] takes a burger to customer.\n", cashier_id);

        //* Step3 - signal rack_not_full after fetching a burger from the full rack
        if(burgers_on_rack == racksize - 1){
            sem_post(&rack_not_full);
        }
        pthread_mutex_unlock(&accessing_rack);

        //* Step4 - change customer_served and check all_served ? 
        pthread_mutex_lock(&customer_count);
        ++customer_served;
        if(customer_served == num_of_customers){
            sem_post(&all_served);
        }
        pthread_mutex_unlock(&customer_count);

        pthread_mutex_unlock(&cashier_mutex);
    }
}
void *customer(void* num){
    int customer_id = *(int *) num;
    float random_time = rand() % 10 + 1; // 1 <= random_time <= 10
    sleep(random_time);

}
void *clean_up(void *num){
    //* when finish the service to all customers
    sem_wait(&all_served);
    int i = 0;
    for(i = 0; i < num_of_cooks; i++) pthread_kill(cook_thread[i], SIGUSR1); /*kill threads representing cooks*/
    for(i = 0; i < num_of_cashiers; i++) pthread_kill(cashier_thread[i], SIGUSR1); /*kill threads representing cashiers*/
    pthread_exit(0);
}
int main(int argc, char ** argv){
    signal(SIGUSR1, pthread_cancel_handler); /*pthread_cancel is not supported in Android bionics, so using pthread_kill instead.*/
    
    //* Step1 - processing input
    if(argc != 5){
        //!exception - wrong input
        printf("input format should be \"BBC #cook_threads #cashier_threads #customer_threads #RackSize\"\n");
        return -1;
    }
    num_of_cooks = atoi(argv[1]);
    num_of_cashiers = atoi(argv[2]);
    num_of_customers = atoi(argv[3]);
    racksize = atoi(argv[4]);
    if (!(num_of_cooks && num_of_cashiers && num_of_customers && racksize)){
        //!exception - wrong input (atoi failed)
        printf("\"#cook_threads #cashier_threads #customer_threads #RackSize\" should be positive numbers\n");
        return -1;
    }
    cook_thread = (pthread_t *)malloc(num_of_cooks * sizeof(pthread_t));
    cashier_thread  = (pthread_t *)malloc(num_of_cashiers * sizeof(pthread_t));
    customer_thread = (pthread_t *)malloc(num_of_customers * sizeof(pthread_t));
    printf("cook_threads [%d], cashier_threads [%d], customer_threads [%d]\n", num_of_cooks, num_of_cashiers, num_of_customers);
    printf("Begin run.\n");

    //* Step2 - init sem and mutex
    pthread_mutex_init(&customer_count, NULL);
    pthread_mutex_init(&cook_mutex, NULL);
    pthread_mutex_init(&accessing_rack, NULL);
    pthread_mutex_init(&cashier_mutex, NULL);
    sem_init(&all_served, 0, 0);
    sem_init(&rack_not_full, 0, 0); //rack_npt_full == 0 iff rack is full
    sem_init(&rack_not_empty, 0, 0); //rack_not_empty == 0 iff rack is empty

    //* Step3 - create threads
    int i = 0;
    //max_tmp if max{num_of_cooks, num_of_cashiers, num_of_customers}
    int max_tmp = (num_of_cooks > num_of_cashiers) ? num_of_cooks : num_of_cashiers;
    max_tmp = (max_tmp > num_of_customers) ? max_tmp : num_of_customers;
    int *max_num = (int *)malloc((max_tmp+1) * sizeof(int));
    for(i = 0; i < num_of_cooks; i++) pthread_create(&cook_thread[i], NULL, cook, &max_num[i + 1]);
    for(i = 0; i < num_of_cashiers; i++) pthread_create(&cashier_thread[i], NULL, cook, &max_num[i + 1]);
    for(i = 0; i < num_of_customers; i++) pthread_create(&customer_thread[i], NULL, cook, &max_num[i + 1]);
    
    //* Step4 - when finish, kill threads of cooks and cashiers because they are endless loop
    pthread_t clean_up_thread;
    pthread_create(&clean_up_thread, NULL, clean_up, NULL);
    pthread_join(clean_up, NULL);


    //* Step5 - Clean up 
    free(cook_thread);
    free(cashier_thread);
    free(customer_thread);
    pthread_mutex_destroy(&customer_count);
    pthread_mutex_destroy(&cook_mutex);
    pthread_mutex_destroy(&accessing_rack);
    pthread_mutex_destroy(&cashier_mutex);
    sem_destroy(&all_served);
    sem_destroy(&rack_not_full);
    sem_destroy(&rack_not_empty);
}