// Caiden Downey
// CS-311 
// Lab 3
// 12/8/21
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <stdbool.h>

#define N 5
#define THINKING 0
#define HUNGRY 1
#define EATING 2
#define LEFT  (pnum % N)
#define RIGHT (pnum + 1) % N

int ret,forks, count;
int state[N] = {0,0,0,0,0}; // STATE OF PHIL
int meals[N] = {0,0,0,0,0}; // COUNT # OF MEALS EACH PHIL HAS EATEN
long fName = 2, cName = 3, tName = 4; // NAME FOR INTIALIZING SEMAPHORES
struct sembuf  forksSem, wait, signal; // SEMAPHORES
bool done = 0; // SETS EQUAL TO TRUE WHEN DONE

void check(int pnum){ // CHECKS TO SEE IF PHIL CAN EAT
    if (state[pnum] == HUNGRY && state[LEFT] != EATING && state[RIGHT] != EATING) {
        state[pnum] = EATING;
        printf("Philosopher %d is Eating\n", pnum);
        fflush(stdout);
        sleep(rand() % (10 + 1 - 5) + 5);
        srand(pnum + count);
        printf("Philosopher %d ate for %d seconds\n", pnum, (rand() % (10 + 1 - 5) + 5));
        fflush(stdout);
        meals[pnum]++;
        ret = semctl(count,0,GETVAL,(void *)0);
        if (ret < 0){
             perror("Count GETVAL");
             exit(1);
            }
        if (meals[pnum] == 3){
          if (ret == 1){ // DELETES SEMAPHORES ON LAST PHIL
            printf("Philosopher %d deleting semaphores \n", pnum);
            fflush(stdout);
            ret = semctl(forks,5,IPC_RMID,(void *)0);
            if (ret < 0){
             perror("Forks delete");
             exit(1);
            }
            ret = semctl(count,1,IPC_RMID,(void *)0);
            if (ret < 0){
             perror("Count delete");
             exit(1);
            }
            done = 1;
           }
          else { // DECREMENTS COUNT
	     ret = semop(count,&wait,1); // DECREMENTS LEFT FORK
        	if (ret < 0){
          	   perror("Count");
          	   exit(1);
        	}
          }
        }
       }
}

void take_fork(int pnum){
    state[pnum] = HUNGRY;
    printf("Philosopher %d is Hungry\n", pnum);
    fflush(stdout);
    // TAKE LEFT FORK
     forksSem.sem_op = -1;
     forksSem.sem_flg = 0;
     forksSem.sem_num = LEFT;
     ret = semop(forks,&forksSem,1); // DECREMENTS LEFT FORK
        if (ret < 0){
          perror("Forks");
          exit(1);
        }
      printf("Philosopher %d took  fork #%d\n", pnum, LEFT);
      fflush(stdout);
      // TAKE RIGHT FORK
     forksSem.sem_num = RIGHT;
     ret = semop(forks,&forksSem,1); // DECREMENTS RIGHT FORK
        if (ret < 0){
          perror("Forks");
          exit(1);
        }
      printf("Philosopher %d took  fork #%d\n", pnum, RIGHT);
      fflush(stdout);
      check(pnum); // CHECK TO SEE IF CAN EAT
}
void drop_fork(int pnum){
     if (done == 1){ // IF NO MORE PHILS RETURN
          printf("ALL DONE PRESS ENTER TO KILL PROCESSES\n");
          fflush(stdout);
          return;
     }
     // DROP LEFT FORK
     forksSem.sem_op = 1;
     forksSem.sem_num = LEFT;
     ret = semop(forks,&forksSem,1); // INCREMENT LEFT FORK
        if (ret < 0){
          perror("forksSem");
          exit(1);
        }
    printf("Philosopher %d dropped fork#%d\n", pnum, LEFT);
    fflush(stdout);
    // DROP RIGHT FORK
    forksSem.sem_num = RIGHT;
    ret = semop(forks,&forksSem,1); // INCREMENT RIGHT FORK
        if (ret < 0){
          perror("forksSem");
          exit(1);
        }
    printf("Philosopher %d dropped fork#%d\n", pnum, RIGHT);
    fflush(stdout);
    state[pnum] = THINKING;
    printf("Philosopher %d is thinking\n", pnum);
    fflush(stdout);
}
void philosopher(int pnum) {
    printf("Philosopher %d Waiting\n",pnum);
    fflush(stdout);
    sleep(rand() % (90 + 1 - 60) + 60);
    srand(pnum + count);
    printf("Philosopher %d is thinking\n", pnum);
    fflush(stdout);
    ret = semop(count,&wait,1); // WAIT FOR ALL 5 PHILS TO ENTER
        if (ret < 0){
          perror("Wait");
          exit(1);
        }
    sleep((rand() % (20 + 1 - 10) + 10));
    srand(pnum + count);
    printf("Philosopher %d was Thinking for %d seconds \n", pnum, (rand() % (20 + 1 - 10) + 10) );
    fflush(stdout);
    ret = semop(count,&signal,1); // SET COUNT BACK TO 5
        if (ret < 0){
          perror("Signal");
          exit(1);
        }
    for(int i = 0; i < 3; i++) { // EAT THREE TIMES
        take_fork(pnum);
        drop_fork(pnum);
    }
}
void main(int argc, char *argv[]){
    int value = 1; // INITIAL VALUE OF FORKS
    int pnum = atoi(argv[1]); // PHIL #
    wait.sem_flg = 0;
    wait.sem_num = 0;
    wait.sem_op = -1;
    signal.sem_flg = 0;
    signal.sem_num = 0;
    signal.sem_op = 1;
    forks = semget(fName,N,IPC_CREAT|0600); // CREATES FORKS SEMAPHORE
        if (forks < 0){
            perror("forks");
            exit(1);
        }
    count = semget(cName,1,IPC_CREAT|0600); // CREATES COUNT SEMAPHORE
        if (count < 0){
            perror("count");
            exit(1);
        }
    if (pnum == 0){ // ONLY FOR PHIL #0
        ret = semctl(count, 0, SETVAL,N); // INITIALIZES COUNT TO 5
        
        for (int k = 0; k < N; k++){
            ret = semctl(forks, k, SETVAL,value); // INITIALIZES FORKS AT K TO 1
        }
    }
    philosopher(pnum);
}