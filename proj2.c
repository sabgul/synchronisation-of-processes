/* * * * * * * * * * * * * * *
*    Faneuil Hall Problem    *
*       IOS - project 2      *
*                            *
*     Sabina Gulcikova       *
* xgulci00@stud.fit.vutbr.cz *
*         06/04/20           *
* * * * * * * * * * * * * * */

//#include "proj2.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include <semaphore.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <pthread.h>
/*--------------------Constants--------------------*/
#define FAIL 1
#define SUCCESS 0
#define MIN_MSEC 0
#define MAX_MSEC 2000
#define REQUIRED_ARGS 6

/*----------------Global variables-----------------*/
sem_t *semaphore = NULL;
FILE *output;
int action = 0;

/*-------------------Functions--------------------*/
int errorCheck(int argc, char *argv[]);
int initialise();
void cleanup();
void open_semaphores();
void close_semaphores();
/*------------------Semaphores--------------------*/

sem_t *printMessage;
sem_t *allRegistered;
sem_t *judgeInHouse;
sem_t *ongoingConfirmation;

//deklarovat semafor
//namapovat do pamate
//pouzit
//nakoniec vsetky uzavriet

/*-------------------Functions--------------------*/


/*Function checks for potential errors caused by invalid input*/
int errorCheck(int argc, char *argv[]) {
  /*Checks whether proper number of arguments was entered*/
  if (argc != REQUIRED_ARGS) {
    fprintf(stderr, "error: invalid number of arguments.\n");
    return FAIL;
  }

  /*Checks whether valid arguments were entered*/
  for (int i = 1; i < argc; i++) {
    char *argument = argv[i];
    int length = strlen(argument);

    for (int i = 0; i < length; i++) {
      if (isdigit(argument[i]) == 0) {
        fprintf(stderr, "error: invalid argument\n");
        return FAIL;
      }
    }
  }

  char *end;
  int immigrants = strtol(argv[1], &end, 10);

  if (immigrants < 1) {
    fprintf(stderr, "error: invalid argument (number of processes)\n");
    return FAIL;
  }

  /*Checks whether the arguments belong to allowed range*/
  for (int i = 2; i < argc; i++) {
    int time = strtol(argv[i], &end, 10);

    if (time < MIN_MSEC || time > MAX_MSEC) {
      fprintf(stderr, "error: invalid argument (max time for generation of new process)\n");
      return FAIL;
    }
  }

  return SUCCESS;
}

int initialise() {
  output = fopen("proj2.out", "w");
  // if(semaphore = sem_open("/xgulci00.ios.proj2.semaphore", O_CREAT | O_EXCL, 0666, 1)== SEM_FAILED) {
  //   fprintf(stderr, "error: initialisation of semaphore failed\n");
  //   return FAIL;
  // }
  open_semaphores();
  return SUCCESS;
}

void open_semaphores() {
  printMessage = sem_open("/xgulci00.ios.proj2.print", O_CREAT | O_EXCL, 0644, 1);
  //judgeInHouse = sem_open("/xgulci00.ios.proj2.judge", O_CREAT | O_EXCL, 0644, 0);
  //allRegistered = sem_open("/xgulci00.ios.proj2.registration", O_CREAT | O_EXCL, 0644, 0);
  //ongoingConfirmation = sem_open("/xgulci00.ios.proj2.confirmation", O_CREAT | O_EXCL, 0644, 0);
}

void close_semaphores() {
  sem_unlink("/xgulci00.ios.proj2.print");
  // sem_unlink("/xgulci00.ios.proj2.judge");
  // sem_unlink("/xgulci00.ios.proj2.registration");
  // sem_unlink("/xgulci00.ios.proj2.confirmation");
}

void cleanup() {
  close_semaphores();
  //sem_close(sem);
  if (output != NULL) {
    fclose(output);
  }
}

// void process_judge(delay, timeToEnter, timeToConfirm) {
//   exit(0);
// }
//
// void process_immigrant(delay, numberOfImmigrants, timeToGetCertif) {
//   exit(0);
// }
  //
  // void gen_immigrants(int numOfImmigrants, int delay){
  //   for(int i = 0; i < numOfImmigrants; i ++){
  //
  //     sleep(delay);
  //   }
  // }
  //

int main(int argc, char *argv[]) {
  if (errorCheck(argc, argv) == FAIL)
    return FAIL;

    if (initialise() == FAIL) {
      cleanup();
      return FAIL;
    }

/*----------------------redo-----------------------*/
char *end;
//number of immigrant processes to be generated
int PI = strtol(argv[1], &end, 10);
//max time for generation of new process
int IG = strtol(argv[2], &end, 10);
//max time for judges entrance since he last left
int JG = strtol(argv[3], &end, 10);
//max time for obtaining of certificate
int IT = strtol(argv[4], &end, 10);
//max time for issuance of a certificate
int JT = strtol(argv[5], &end, 10);
/*------------------------------------------------*/

//First division of process
    pid_t judge = fork();

    if (judge < 0) {
      fprintf(stderr, "error: fork failed\n");
      return FAIL;
    } else if (judge == 0) {
      //judge child process
      //process_judge();
      //exit(0);
    } else {
      //second division of process
      pid_t immigrant = fork();

      if (immigrant < 0) {
        fprintf(stderr, "error: fork failed\n");
        return FAIL;
      } else if (immigrant == 0) {
        //immgirant child process
        //gen_immigrants();
        //exit(0);
      } else {
        //waitpid();
      }
      //waitpid();
    }
  cleanup();
  //exit(0);
  return SUCCESS;
}
