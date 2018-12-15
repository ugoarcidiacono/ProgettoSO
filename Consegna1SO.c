#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <dirent.h>
#include <string.h>
#define CPUCORES 4
#define N 1024

typedef struct l_dir {
    char directory[N];
    struct l_dir *next;
}nodo_directory, *lista_directory;

void carica_lista(lista_directory *list, char *file);
void carica_coda(char *file);
void stampa_lista(lista_directory lista);
void *thread_function(void *arg);
void cancella_directory(lista_directory *list, char *directory);
void cancella_elemento(char *elem);

pthread_mutex_t mutex;

lista_directory list = NULL;
lista_directory ultimo = NULL;

int main(){
    struct dirent *de;
    char dir[N];
    char buffer[N];
    char aux[N];
    pthread_t thread[CPUCORES];
    printf("Inserisci una directory: ");
    scanf("%s", dir);
    if(dir[strlen(dir)-1]!='/')
        strcat(dir, "/");
    strcpy(aux,dir);
    DIR *dr = opendir(dir);
    if(dr == NULL){
        printf("Directory non esistente!\n");
        return 0;
    }

    while ((de = readdir(dr)) != NULL){
        strcpy(buffer, (char *)de->d_name);
        if(buffer[0] != '.'){
        strcat(dir, (char *)de->d_name);
        carica_coda(dir);
        strcpy(dir,aux);
        }
    }
    closedir(dr);
    pthread_mutex_init(&mutex, NULL);
    for(int i=0; i<CPUCORES; i++){
        pthread_create(&thread[i], NULL, thread_function, NULL);
    }
    for(int i=0; i<CPUCORES; i++){
        pthread_join(thread[i], NULL);
    }
    pthread_mutex_destroy(&mutex);
    stampa_lista(list);
}

void carica_coda(char *file){
    lista_directory paux;
    paux=(lista_directory)malloc(sizeof(nodo_directory));
    if(paux == NULL){
        printf("Errore nell'allocazione della memoria!\n");
        return;
    }
    strcpy(paux->directory, file);
    paux->next = NULL;
    if(ultimo == NULL){
        list=paux;
        ultimo = paux;
    }
    else{
        ultimo->next = paux;
        ultimo = paux;
    }

}

void carica_lista(lista_directory *list, char *file){
    lista_directory paux;
    paux=(lista_directory)malloc(sizeof(nodo_directory));
    if(paux == NULL){
        printf("Errore nell'allocazione della memoria!\n");
        return;
    }
    strcpy(paux->directory, file);
    paux->next= *list;
    *list=paux;
}

void stampa_lista(lista_directory lista){
    if(lista==NULL){
        printf("La lista è vuota!\n");
        return;
    }
    while(lista!=NULL){
        printf("%s\n", lista->directory);
        lista=lista->next;       
    }
}

void *thread_function(void *arg){
    struct dirent *aux;
    char dir_aux[N];
    char nome_directory[N];
    lista_directory corr = list;
    lista_directory prec = list;
    DIR *dr;
    pthread_mutex_lock(&mutex);
    while(corr!=NULL){
        dr = opendir(corr->directory);
        if(dr != NULL){
            strcpy(dir_aux, corr->directory);
            strcpy(nome_directory, corr->directory); 
            cancella_elemento(corr->directory);
            while ((aux = readdir(dr)) != NULL){
                if(aux->d_name[0] != '.'){
                    if(dir_aux[strlen(dir_aux)-1] != '/')
                        strcat(dir_aux, "/");
                    strcat(dir_aux, (char *)aux->d_name);
                    carica_coda(dir_aux);
                    strcpy(dir_aux,nome_directory);
                }
                else {
                    if(strlen(aux->d_name)>1 && aux->d_name[1] != '.'){
                        if(dir_aux[strlen(dir_aux)-1] != '/')
                            strcat(dir_aux, "/");
                        strcat(dir_aux, (char *)aux->d_name);
                        carica_coda(dir_aux);
                        strcpy(dir_aux,nome_directory);
                    }
                }
            }
            closedir(dr);
            corr=prec; //il nodo corrente è stato eliminato dalla funzione cancella elemento
        }
        else{
            prec=corr;
            corr=corr->next;
        }
    }
    pthread_mutex_unlock(&mutex);
}

void cancella_elemento(char *elem){
    lista_directory prec, corr;
    if(list==NULL){
        printf("Lista vuota.\n");
        return;
    }
    corr=list;
    while(corr!=NULL){
        if(strcmp(corr->directory, elem)==0){
            if(corr==list){
                list=list->next;
                free(corr);
                return;
            }
            prec->next=corr->next;
            free(corr);
            return;
        }
        prec=corr;
        corr=corr->next;
    }
}