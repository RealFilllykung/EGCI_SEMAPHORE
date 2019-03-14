#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>

static int set_semvalue(int);
static void del_semvalue(int);
static int semaphore_p(int);
static int semaphore_v(int);

static int s1;
static int s2;

void f1(pid_t);
void f2(pid_t);

int main(void)
{
    pid_t child;
    
    s1 = semget((key_t) 1234, 1, 0666 | IPC_CREAT);
    s2 = semget((key_t) 1235, 1, 0666 | IPC_CREAT);
    
    if (!set_semvalue(s1) || !set_semvalue(s2))
    {
        fprintf(stderr,"Failed to initialize semaphore\n");
        exit(EXIT_FAILURE);
    }
    
    child = fork();
    
    if (child) f1(child);
    else f2(child);
    
    printf("%d Done!\n", getpid());
    
    if(child)
    {
        wait(NULL);
        del_semvalue(s2);
        del_semvalue(s1);
    }
    
    exit(EXIT_SUCCESS);
}

static int set_semvalue(int s)
{
    union semun sem_union;
    
    sem_union.val = 1;
    if(semctl(s,0,SETVAL,sem_union) == -1) return (0);
    
    printf("Successfully create semaphore\n");
    return (1);
}

static void del_semvalue(int s)
{
    union semun sem_union;
    
    if(semctl(s,0,IPC_RMID,sem_union) == -1)
    {
        fprintf(stderr, "Failed to delete semaphore\n");
    }
}

static int semaphore_p(int s)
{
    struct sembuf sem_b;
    
    sem_b.sem_num = 0;
    sem_b.sem_op = -1;
    sem_b.sem_flg = SEM_UNDO;
    if(semop(s,&sem_b,1) == -1)
    {
        fprintf(stderr,"semaphore_p failed\n");
        return(0);
    }
    return (1);
}

static int semaphore_v(int s)
{
    struct sembuf sem_b;
    
    sem_b.sem_num = 0;
    sem_b.sem_op = 1;
    sem_b.sem_flg = SEM_UNDO;
    if(semop(s,&sem_b,1) == -1)
    {
        fprintf(stderr,"semaphore_v failed\n");
        return (0);
    }
    return (1);
}

void f1(pid_t pid)
{
    if (!semaphore_p(s1)) exit(EXIT_FAILURE);
    printf("Enter f1\n");
    sleep(1);
    if(!semaphore_v(s1)) exit(EXIT_FAILURE);
}

void f2(pid_t pid)
{
    if (!semaphore_p(s2)) exit(EXIT_FAILURE);
    printf("Enter f2\n");
    sleep(1);
    if(!semaphore_v(s2)) exit(EXIT_FAILURE);
}
