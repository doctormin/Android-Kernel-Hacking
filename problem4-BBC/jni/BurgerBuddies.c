#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>
#include <signal.h>

pthread_t *cook_thread, *cashier_thread, *customer_thread;
int *Queue; //This is a cycular queue for customers
int Queue_tail, Queue_head;
int num_of_cooks, num_of_cashiers, num_of_customers, racksize;
int cashier_waiting_for_burger; //1 or 0
int cashier_waiting_for_customer;
int cook_waiting_for_rack;
int customer_served; //the number of customers that has been served
int burgers_on_rack; //the number of available burgers 
pthread_mutex_t customer_count; //mutex for accessing `customer_served`
pthread_mutex_t cook_mutex; //only one cook can check rack(and then make one burger) at a time
pthread_mutex_t accessing_rack;
pthread_mutex_t accessing_Queue;

sem_t all_served;
sem_t rack_not_full;
sem_t rack_not_empty;
sem_t Queue_not_empty;

/**
 * pthread_cancel_handler - the handler function for signal(SIGUSER1, handler)
 * because pthread_cancel() is not supported in Android
 * So pthread_kill() is used instead, and this handler will
 * response to SIGUSER1 that pthread_kill() generates
 */
void pthread_cancel_handler(int num){
    pthread_exit(0);
}
void enQueue(int num){
    Queue[Queue_tail] = num;
    Queue_tail = (Queue_tail + 1) % (num_of_customers + 1);
}
int deQueue(){
    int tmp = Queue[Queue_head];
    Queue_head = (Queue_head + 1) % (num_of_customers + 1);
    return tmp;
}

int Queue_length(){
    if(Queue_tail >= Queue_head){
        return (Queue_tail - Queue_head);
    }
    else{
        return (num_of_customers + 1 - (Queue_head - Queue_tail));
    }
}
void *cook(void* num){
    int cook_id = *(int *) num;
    while(1){
        //* Step1 - make the burger 
        sleep(cook_id <= 3 ? cook_id : cook_id/3);
        //* Step2 - check the rack and wait or put the burger on it
        pthread_mutex_lock(&cook_mutex);
        pthread_mutex_lock(&accessing_rack);
        if(burgers_on_rack == racksize){ //rack is full
            cook_waiting_for_rack++;
            printf("=========rack is full !=========\n");
            pthread_mutex_unlock(&accessing_rack); //enable cashiers to fetch burgers from the rack
            sem_wait(&rack_not_full); //will be signaled when a cashier give out one burger(before which the rack is full)
            pthread_mutex_lock(&accessing_rack);
        }
        burgers_on_rack++;
        printf("Cook [%d] makes a burger.\n", cook_id);
        printf("There are %d burgers on the rack\n", burgers_on_rack);
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
    while(1){
        //* Step1 - waiting for and serve a customer
        pthread_mutex_lock(&accessing_Queue);
        if(Queue_length() == 0){//no customers in the Queue now
            ++cashier_waiting_for_customer;
            pthread_mutex_unlock(&accessing_Queue);
            sem_wait(&Queue_not_empty); //waiting for the signal ---- "Queue is not empty now!"
            pthread_mutex_lock(&accessing_Queue);
        }
        int customer_id = deQueue();
        printf("Cashier [%d] accepts the order of customer [%d].\n", cashier_id, customer_id);
        pthread_mutex_unlock(&accessing_Queue);

        //* Step2 -  fetch burger and serve
        pthread_mutex_lock(&accessing_rack);
        if(burgers_on_rack == 0){
            ++cashier_waiting_for_burger; //showing that there are cashiers waiting 
            pthread_mutex_unlock(&accessing_rack); //enable cooks to put burgers on the rack
            sem_wait(&rack_not_empty);
            pthread_mutex_lock(&accessing_rack);
        }
        burgers_on_rack--;
        printf("Cashier [%d] takes a burger to customer [%d].\n", cashier_id, customer_id);
        printf("burgers_on_rack is %d\n", burgers_on_rack);
        //* Step3 - signal rack_not_full after fetching a burger from the full rack
        if(burgers_on_rack == racksize - 1 && cook_waiting_for_rack){
            sem_post(&rack_not_full);
            cook_waiting_for_rack--;
        }
        pthread_mutex_unlock(&accessing_rack);

        //* Step4 - change customer_served and check all_served ? 
        pthread_mutex_lock(&customer_count);
        ++customer_served;
        if(customer_served == num_of_customers){
            sem_post(&all_served);
        }
        pthread_mutex_unlock(&customer_count);
    }
}
void *customer(void* num){
    int customer_id = *(int *) num;
    float random_time = rand() % 10 + 1; // 1 <= random_time <= 10
    sleep(random_time);
    pthread_mutex_lock(&accessing_Queue);
    enQueue(customer_id);
    if(Queue_length() == 1 && cashier_waiting_for_customer){
        sem_post(&Queue_not_empty);
        cashier_waiting_for_customer--;
    }
    printf("Customer [%d] comes.\n", customer_id);
    pthread_mutex_unlock(&accessing_Queue);
    pthread_exit(0);
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

    //* Step2 - init sem and mutex and Queue
    Queue = (int *) malloc ((num_of_customers + 1)* sizeof(int));
    Queue_tail = 0;
    Queue_head = 0;
    pthread_mutex_init(&customer_count, NULL);
    pthread_mutex_init(&cook_mutex, NULL);
    pthread_mutex_init(&accessing_rack, NULL);
    pthread_mutex_init(&accessing_Queue, NULL);
    sem_init(&all_served, 0, 0);
    sem_init(&rack_not_full, 0, 0); //rack_npt_full == 0 iff rack is full
    sem_init(&rack_not_empty, 0, 0); //rack_not_empty == 0 iff rack is empty
    sem_init(&Queue_not_empty, 0, 0);

    //* Step3 - create threads
    int i = 0;
    //max_tmp if max{num_of_cooks, num_of_cashiers, num_of_customers}
    int max_tmp = (num_of_cooks > num_of_cashiers) ? num_of_cooks : num_of_cashiers;
    max_tmp = (max_tmp > num_of_customers) ? max_tmp : num_of_customers;
    int *max_num = (int *)malloc((max_tmp+1) * sizeof(int));
    for(i = 0; i < max_tmp + 1; i++){
        max_num[i] = i;
    }
    for(i = 0; i < num_of_cooks; i++) pthread_create(&cook_thread[i], NULL, cook, &max_num[i + 1]);
    for(i = 0; i < num_of_cashiers; i++) pthread_create(&cashier_thread[i], NULL, cashier, &max_num[i + 1]);
    for(i = 0; i < num_of_customers; i++) pthread_create(&customer_thread[i], NULL, customer, &max_num[i + 1]);
    
    //* Step4 - when finish, kill threads of cooks and cashiers because they are endless loop
    pthread_t clean_up_thread;
    pthread_create(&clean_up_thread, NULL, clean_up, NULL);
    pthread_join(clean_up_thread, NULL);


    //* Step5 - Clean up 
    free(cook_thread);
    free(cashier_thread);
    free(customer_thread);
    free(Queue);
    pthread_mutex_destroy(&customer_count);
    pthread_mutex_destroy(&cook_mutex);
    pthread_mutex_destroy(&accessing_rack);
    pthread_mutex_destroy(&accessing_Queue);
    sem_destroy(&all_served);
    sem_destroy(&rack_not_full);
    sem_destroy(&rack_not_empty);
    sem_destroy(&Queue_not_empty);
}