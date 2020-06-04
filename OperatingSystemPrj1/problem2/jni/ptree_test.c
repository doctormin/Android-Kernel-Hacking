#include <sys/types.h> 
#include <sys/syscall.h>
#include <stdlib.h>
#include <stdio.h>
#define MAXBUFFER 3000
#define Yimin_ptree 356


struct prinfo {
  pid_t parent_pid;        // process id of parent
  pid_t pid;               // process id
  pid_t first_child_pid;   // pid of youngest child (earliest created child)
  pid_t next_sibling_pid;  // pid of older sibling (later created sibling)
  long state;              // current state of process
  long uid;                // user id of process owner
  char comm[64];           // name of program executed
};

int find_parent(struct prinfo *buf, int index){
    int i;
    for(i = index; i >= 0; i--){
        if(buf[index].parent_pid == buf[i].pid)
            return i;
    }
    return -1;
}

/**
 * print_tree - print out the process tree in DFS order
 * @buf: an array containing the info of processes in DFS order
 * @nr: the length of buf
 */
void print_tree(struct prinfo *buf, int nr){
    int num_of_tab[nr]; //num_of_tab[index] = the number of tabs which needs to be printed for process[index]
    num_of_tab[0] = 0; //ini_task is generation 0 with no tab during output
    int parent_index;
    int i, j;
    //get generation number(=num_of_tab) for each process
    for(i = 1; i < nr; i++){
        parent_index = find_parent(buf, i);
        if(parent_index == -1){
            printf("%d,%s, %s", buf[i].pid, buf[i].comm, "find parent failed!");
            return;
        }
        num_of_tab[i] = num_of_tab[parent_index] + 1;
    }
    //output
    for(i = 0; i < nr; i++){
        for(j = 0; j < num_of_tab[i]; j++)
            printf("%s", "\t");
        printf("%s,%d,%ld,%d,%d,%d,%ld\n", buf[i].comm, buf[i].pid, buf[i].state, buf[i].parent_pid, \
                buf[i].first_child_pid, buf[i].next_sibling_pid, buf[i].uid);
    }
};

int main(){
    struct prinfo *buf = (struct prinfo*) malloc (sizeof(struct prinfo) * MAXBUFFER);
    int nr = 0;
    long number = syscall(Yimin_ptree, buf, &nr);
    print_tree(buf, nr);
    printf("process number = %ld\n", number);
    printf("entries number = %d\n", nr);
    free(buf);
}


