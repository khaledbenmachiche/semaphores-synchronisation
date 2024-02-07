#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>


#define scab 0
#define mutex 1
#define span 2


#define nc 2
#define np 5


#define N_NAGEUR 10

struct sembuf p_scab = {scab , -1,0};
struct sembuf v_scab = {scab,1,0};
struct sembuf p_mutex = {mutex, -1,0};
struct sembuf v_mutex = {mutex,1,0};
struct sembuf p_span = {span,-1,0};
struct sembuf v_span = {span,1,0};

int main(){
    union semun {
        int val;
        struct semid_ds *buf;
        unsigned short *array;
    }scab_init,mutex_init,span_init;
    // generation de la cle1
    key_t cle1 = ftok("main.c",1);
    if(cle1 == -1){
        printf("erreur dans la generation de la cle \n");
        exit(1);
    }
    printf("cle1 = %d\n",cle1);

    // creation des semaphores
    int sem = semget(cle1,3,IPC_CREAT|0666);
    if(sem == -1){
        printf("\nErreur de création des semaphores");
        exit(2);
    }
    printf("semaphore id = %d\n",sem);

    /* initialisation des semaphores*/
    // initialisation de semaphore scab
    scab_init.val = nc;
    semctl(sem,scab,SETVAL,scab_init);

    // initialisation de semaphore mutex
    mutex_init.val = 1;
    semctl(sem,mutex,SETVAL,mutex_init);
    // initialisation de semaphore span

    span_init.val = 0;
    semctl(sem,span,SETVAL,span_init);


    /**creations de segment memoire partagée**/

    //generation de la cle2

    key_t cle2 = ftok("main.c",2);
    if(cle2 == -1){
        printf("erreur dans la generation de la cle \n");
        exit(3);
    }

    printf("cle2 = %d\n",cle2);

    // création un segment de mémoire partagée
    int id_memoire_p = shmget(cle2,2 * sizeof(int),IPC_CREAT|0666);
    if(id_memoire_p == -1){
        printf("erreur dans la creation de segment de memoire partagee");
        exit(4);
    }
    int *ndp = (int *)shmat(id_memoire_p,0,0);
    if(ndp == NULL){
        printf(" \n erreur de l'attachement de la zone partage ");
        exit(5);
    }

    int *npo = ndp + 1;
    *ndp = 0;
    *npo = 0;

    shmdt(ndp);

    // creation des 10 processus Nageur

    pid_t p;
    for (int i = 0; i < N_NAGEUR; i++) {
        p = fork();
        if(p == -1){
            printf("\nerreur dans la création de processus nageur numero %d \n",i);
            exit(6);
        }
        if(p == 0){
            printf("idk");
            fflush(stdout);
            char i_char[3];
            sprintf(i_char,"%d",i);
            int err = execlp("./Pgme_nageur","Pgme_nageur",i_char,NULL);
            if(err != 0){
                printf("erreur dans exec numéro=%d\n",i);
                exit(err);
            }
            exit(7);
        }
    }

    while((p = wait(NULL)) != -1) printf("\nFils %d termine\n",p);

    /* Détruire les sémaphores */
    semctl(sem, 3, IPC_RMID, 0);

    /* Détruire le segments de memoire partagee */
    semctl(id_memoire_p,IPC_RMID,0);
    printf("\n======== Fin du processus pere ========\n");
    return 0;
}