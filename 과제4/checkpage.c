#include "types.h"
#include "stat.h"
#include "user.h"

int
main(int argc, char *argv[]){
    printf(1, "%d is free page num\n", getNumFreePages());
    int pid;
    for(int i=0;i<3;i++){
        pid = fork();
        if(pid == 0){
            printf(1, "%d is free page num | I am CHILD PROCESS\n", getNumFreePages());
        }
        else if(pid > 0){
            wait();
            printf(1, "%d is free page num | I am PARENT PROCESS\n", getNumFreePages());
            exit();
        }
    }

        
    exit();
}

