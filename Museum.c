#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>
#include <time.h>

//Na, Nb - capacity of hall a and b; N - total number of people
int Na;
int Nb;
int N;

//semaphores for hall a and b
sem_t hallA;
sem_t hallB;

//mutexes for use later
pthread_mutex_t inside;
pthread_mutex_t entrance;
pthread_mutex_t MControl;
pthread_mutex_t AB;
pthread_mutex_t BA;

//other variables that help the programme
int NhallA;
int NhallB;
int waitAB;
int waitBA;
int EndControl;
int IfControl1;
int IfControl2;

//struct containing info for all visitors
typedef struct _Visitor
{
    int Id; // id of visitor (0-N)
    int Hall; // id of Hall (0-outside; 1-A; 2-B)

} Visitor;

Visitor* Person;

//control function that eliminates the deadlock for the border between hall a and b
void* Control()
{
    while(EndControl==0)
    {
        if(waitAB>0 && waitBA>0)
        {
            pthread_mutex_lock(&MControl);
            pthread_mutex_trylock(&AB);
            pthread_mutex_trylock(&BA);
            sem_post(&hallA);
            sem_post(&hallB);
            waitAB--;
            waitBA--;
            IfControl2=1;
            IfControl1=1;
            pthread_mutex_unlock(&BA);
            pthread_mutex_unlock(&AB);
            pthread_mutex_unlock(&MControl);
        }
    }
    pthread_exit(NULL); // end the thread

}

//function detailing how a visitor behaves
void* Visit(void* index)
{
    int i = Person[(long) index].Id;
    double watch;
    int decision, qBreak=0;
    //waiting outside
    printf("Person %d wants to enter hall A.\n", i);
    while(1)
    {
        pthread_mutex_lock(&entrance);
        pthread_mutex_lock(&BA);
        if(waitBA==0)
        {
            if(sem_trywait(&hallA)==0)
            {
                pthread_mutex_lock(&inside);
                qBreak=1;
                NhallA++;
                Person[i].Hall=1;
                printf("Person %d enters hall A. (A:%d B:%d)\n", i, NhallA, NhallB);
                pthread_mutex_unlock(&inside);
            }
        }
        pthread_mutex_unlock(&BA);
        pthread_mutex_unlock(&entrance);
        if(qBreak==1) break;
    }
    //is inside
    //watch the exhibition for 1 to 10 seconds (random)
    watch=rand()%91+10;
    watch/=10.0; 
    sleep(watch);
    
    //decide if go to B(1) or exit(0)
    decision=rand()%2;

    //procede in accordance with the decision
    if(decision==0) //EXIT
    {
        pthread_mutex_lock(&inside);
        sem_post(&hallA);

        Person[i].Hall=0;
        NhallA--;
        printf("Person %d exits hall A. (A:%d B:%d)\n", i, NhallA, NhallB);

        pthread_mutex_unlock(&inside);
    }
    else if(decision==1) //ENTER HALL B
    {
        printf("Person %d wants to enter hall B.\n", i);
        
        pthread_mutex_lock(&AB);
        waitAB++;
        pthread_mutex_unlock(&AB);

        sem_wait(&hallB);

        pthread_mutex_lock(&AB);
        if(IfControl1==0)
        {
            waitAB--;
        }
        pthread_mutex_unlock(&AB);


        pthread_mutex_lock(&inside);
        pthread_mutex_lock(&MControl);
        if(IfControl2==0)
        {
            sem_post(&hallA);
        }
        pthread_mutex_unlock(&MControl);


        NhallB++;
        NhallA--;

        printf("Person %d exits hall A and enters hall B. (A:%d B:%d)\n", i, NhallA, NhallB);

        pthread_mutex_unlock(&inside);

        //watch the exhibition for 1 to 5 seconds (random)
        watch=rand()%41+10;
        watch/=10.0; 
        sleep(watch);

        //EXIT
        printf("Person %d wants to exit hall B.\n", i);
        
        pthread_mutex_lock(&BA);
        waitBA++;
        pthread_mutex_unlock(&BA);

        sem_wait(&hallA);

        pthread_mutex_lock(&BA);
        if(IfControl1==0)
        {
            waitBA--;
        }
        else IfControl1=0;
        pthread_mutex_unlock(&BA);


        pthread_mutex_lock(&inside);
        pthread_mutex_lock(&MControl);
        if(IfControl2==0)
        {
            sem_post(&hallB);
        }
        else IfControl2=0;
        pthread_mutex_unlock(&MControl);
        NhallB--;
        NhallA++;
        printf("Person %d exits hall B and enters hall A. (A:%d B:%d)\n", i, NhallA, NhallB);
        sem_post(&hallA);
        NhallA--;
        printf("Person %d exits hall A. (A:%d B:%d)\n", i, NhallA, NhallB);
        pthread_mutex_unlock(&inside);

    }
    EndControl++;

    pthread_exit(NULL); // end the thread

}

int main(int argc, char* argv[])
{
    if(argc==4)
    {
        N=atoi(argv[1]);
        Na=atoi(argv[2]);
        Nb=atoi(argv[3]);
    }
    else
    {
        N=20;
        Na=4;
        Nb=3;
    }
    printf("N=%d; Na=%d; Nb=%d\n", N, Na, Nb);
    //Initialize stuff
    srand(time(NULL));
    int i=0, n=N;
    int result;
    void* ThreadRet;
    pthread_t PersonThread[n];
    pthread_t ControlThread;
    NhallA=0;
    NhallB=0;
    EndControl=0;
    waitAB=0;
    waitBA=0;
    IfControl1=0;
    IfControl2=0;

    Person=malloc(n*sizeof(Visitor));
    for(i=0; i<n; i++)
    {
        Person[i].Id=i;
        Person[i].Hall=0;
    }
    //////////////////////////////////////////////

    //Initialize semaphores
    result=sem_init(&hallA, 0, Na);
    if(result!=0)
    {
        printf("ERROR-semaphore A\n");
        return 1;
    }
    result=sem_init(&hallB, 0, Nb);
    if(result!=0)
    {
        printf("ERROR-semaphore B\n");
        return 1;
    }
    //////////////////////////////////////////////

    //Initialize mutexes
    result=pthread_mutex_init(&inside, NULL);
    if(result!=0)
    {
        printf("ERROR-mutex inside\n");
        return 1;
    }
    result=pthread_mutex_init(&entrance, NULL);
    if(result!=0)
    {
        printf("ERROR-mutex entrance\n");
        return 1;
    }
    result=pthread_mutex_init(&AB, NULL);
    if(result!=0)
    {
        printf("ERROR-mutex AB B\n");
        return 1;
    }
    result=pthread_mutex_init(&BA, NULL);
    if(result!=0)
    {
        printf("ERROR-mutex BA B\n");
        return 1;
    }
    result=pthread_mutex_init(&MControl, NULL);
    if(result!=0)
    {
        printf("ERROR-mutex control B\n");
        return 1;
    }
    //////////////////////////////////////////////

    //Initialize threads
    result=pthread_create(&ControlThread, NULL, Control, NULL);
    if(result!=0)
    {
        printf("ERROR-thread creation (control)\n");
        return 1;
    }
    for(long j=0; j<n; j++)
    {
        result=pthread_create(&PersonThread[j], NULL, Visit, (void*)j);
        if(result!=0)
        {
            printf("ERROR-thread creation (%ld)\n", j);
            return 1;
        }
    }
    //////////////////////////////////////////////

    //Wait for threads to finish
    for(i=0; i<n; i++)
    {
        result=pthread_join(PersonThread[i], &ThreadRet);
        if(result!=0)
        {
            printf("ERROR-thread join (%d)\n", i);
            return 1;
        }
    }
    EndControl=1;
    result=pthread_join(ControlThread, &ThreadRet);
    if(result!=0)
    {
        printf("ERROR-thread join (control)\n");
        return 1;
    }
    //////////////////////////////////////////////

    sem_destroy(&hallA);
    sem_destroy(&hallB);
    pthread_mutex_destroy(&inside);
    pthread_mutex_destroy(&entrance);
    pthread_mutex_destroy(&MControl);
    pthread_mutex_destroy(&AB);
    pthread_mutex_destroy(&BA);

    return 0;
}