#include <unistd.h> //necessary for fork() and execl()
#include <sys/types.h>
#include <stdio.h>
#include <sys/wait.h>
int main()
{
	pid_t parent = getpid();
	pid_t child = fork();
	if (child > 0)//in the parent process
	{
	    printf("518030910188-Parent is %d\n", parent); 
	    wait(NULL); // wait for the child process to finish
	}
	else if (child == 0) //in the child process
	{
		printf("518030910188-Child is %d\n", getpid());
		if(execl("./ptree_test", "ptree_test", NULL) == -1){
            //!exception
            printf("calling ptree_test failed in child process!\n");
        }
        exit(0); //in case that execl failed
	}
    //!exception
    else if (child == -1)
    {
        printf("fork() failed!\n");
        return -1;
    }
	return 0;
}