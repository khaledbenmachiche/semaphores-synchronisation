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

struct sembuf p_scab = {scab , -1,0};
struct sembuf v_scab = {scab,1,0};
struct sembuf p_mutex = {mutex, -1,0};
struct sembuf v_mutex = {mutex,1,0};
struct sembuf p_span = {span,-1,0};
struct sembuf v_span = {span,1,0};


int sem;// identifiant de l'ensemble des semaphores
int *ndp;
int *npo;

void demander_cabine(){
    semop(sem,&p_scab,1);
}

void librerer_cabine(){
    semop(sem,&v_scab,1);
}

void demander_panier(int *nump){
    semop(sem,&p_mutex,1);
    (*ndp)++;
    if((*npo) == np){
        semop(sem,&v_mutex,1);
        semop(sem,&p_span,1);
    }
    (*ndp)--;
    (*npo)++;
    *nump = *npo;
    semop(sem,&v_mutex,1);
}

void liberer_panier(int i, int *nump){
    semop(sem,&p_mutex,1);
    (*npo)--;
    *nump = *npo;
    if(*ndp >0){
        printf("\nNageur ,%d,: va libérer un panier et il y a %d demandes en attente\n",i,*ndp);
        semop(sem,&v_span,1);
    }else{
        semop(sem,&v_mutex,1);
    }
}


int main(int argc, char **argv){
    if (argc != 2) {
        printf("utilisation: %s <numero-du-negeur>\n", argv[0]);
        exit(1);
    }
    int i = atoi(argv[1]);
    printf("%d",i);

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

    // creation des semaphores
    sem = semget(cle1,3,IPC_CREAT|0666);
    if(sem == -1){
        printf("\nErreur de création des semaphores");
        exit(2);
    }
    printf("semaphore id = %d",sem);

    /**creations de segment memoire partagée**/
    //generation de la cle2

    key_t cle2 = ftok("main.c",2);
    if(cle2 == -1){
        printf("erreur dans la generation de la cle \n");
        exit(3);
    }

    // création un segment de mémoire partagée
    int id_memoire_p = shmget(cle2,2 * sizeof(int),IPC_CREAT|0666);
    if(id_memoire_p == -1){
        printf("erreur dans la creation de segment de memoire partagee");
        exit(4);
    }
    ndp = (int *)shmat(id_memoire_p,0,0);
    if(ndp == NULL){
        printf(" \n erreur de l'attachement de la zone partage ");
        exit(5);
    }
    npo = ndp + 1;

    int *npo = ndp + 1;

    // le programme Nageur(i);
    int nump;
    demander_panier(&nump);
    demander_cabine();
    // se changer et ranger ses vetements dans le panier (4 secondes);
    sleep(4);

    librerer_cabine();

    // se baigner (7 seconde);
    sleep(7);

    printf("Je suis le nageur numéro = %d, j'occupe le panier %d",i,nump);
    demander_cabine();
    // se changer et ranger ses vetements dans le panier (4 secondes);
    librerer_cabine();
    liberer_panier(i,&nump);
    printf("Je suis le nargeur : %d, j'ai libéré un panier, il reste %d paniers libres",i,np - nump);

    shmdt(ndp);
    exit(0);
}