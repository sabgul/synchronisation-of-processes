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
#include <sys/mman.h>

/*--------------------Constants--------------------*/
#define FAIL 1
#define SUCCESS 0
#define MIN_MSEC 0
#define MAX_MSEC 2000
#define REQUIRED_ARGS 6

/*----------------Global variables-----------------*/
sem_t *semaphore = NULL;
FILE *output;
int *action = 0;

/*-------------------Functions--------------------*/
int errorCheck(int argc, char *argv[]);
int initialise();
void cleanup();
void open_semaphores();
void close_semaphores();

/*------------------Semaphores--------------------*/

sem_t *printMessage; /*avoids the overlapping*/
sem_t *allRegistered; /*all immigrants to be registered NC == NE*/
sem_t *judgeInHouse; /*no one can enter / leave --> NE stays the same*/
sem_t *ongoingConfirmation; /*all registrations to be confirmed by judge NC => 0*/
//sem_t *completeImmigrants; /*all immigrants were given naturalisation, judge can terminate*/

//deklarovat semafor
//namapovat do pamate
//pouzit
//nakoniec vsetky uzavriet

/*-------------------Functions--------------------*/


/*--------------Initial precautions---------------*/
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

/*Function prepares file for output and loads semaphores*/
int initialise() {
  output = fopen("proj2.out", "w");
  open_semaphores();
  return SUCCESS;
}

/*Function opens all semaphores*/
void open_semaphores() {
  printMessage = sem_open("/xgulci00.ios.proj2.print", O_CREAT | O_EXCL, 0644, 1);
  judgeInHouse = sem_open("/xgulci00.ios.proj2.judge", O_CREAT | O_EXCL, 0644, 0);
  allRegistered = sem_open("/xgulci00.ios.proj2.registration", O_CREAT | O_EXCL, 0644, 0);
  ongoingConfirmation = sem_open("/xgulci00.ios.proj2.confirmation", O_CREAT | O_EXCL, 0644, 0);

  if (printMessage == SEM_FAILED ||
      judgeInHouse == SEM_FAILED ||
      allRegistered == SEM_FAILED ||
      ongoingConfirmation == SEM_FAILED) {
    fprintf(stderr, "error: loading of semaphores failed\n");
    cleanup();
    exit(FAIL);
  }


}

/*Function unlinks and closes all semaphores*/
void close_semaphores() {
  sem_close(printMessage);
  sem_close(allRegistered);
  sem_close(judgeInHouse);
  sem_close(ongoingConfirmation);

  sem_unlink("/xgulci00.ios.proj2.print");
  sem_unlink("/xgulci00.ios.proj2.judge");
  sem_unlink("/xgulci00.ios.proj2.registration");
  sem_unlink("/xgulci00.ios.proj2.confirmation");
}

/*Function closes the ouput file and deals with semaphores*/
void cleanup() {
  close_semaphores();
  //sem_close(sem);
  if (output != NULL) {
    fclose(output);
  }
}
/*------------------------------------------------*/

/*----------------Synchronisation-----------------*/
// void process_judge(delay, timeToEnter, timeToConfirm) {
//   exit(0);
// }
//
// void process_immigrant(delay, numberOfImmigrants, timeToGetCertif) {
//   exit(0);
// }
  //
  void gen_immigrants(int numOfImmigrants, int delay) {
    srand(time(NULL));
    for(int i = 0; i < numOfImmigrants; i ++) {
      pid_t imm = fork();
      if(imm < 0) {
        fprintf(stderr, "error: invalid fork\n");
        exit(FAIL);
      } else if (imm == 0) {
        (*action)++;
        fprintf(stdout, "%ls. I am child and I should do something\n", action);
        exit(SUCCESS);
      } else {
        sem_wait(printMessage);
        (*action)++;
        fprintf(stdout, "%ls. I am parent\n", action);
        sem_post(printMessage);
        exit(SUCCESS);
      }
      usleep((rand()%delay+1)*1000);
    }
    exit(SUCCESS);
  }


int main(int argc, char *argv[]) {
  if (errorCheck(argc, argv) == FAIL) {
    return FAIL;
  }
    if (initialise() == FAIL) {
      cleanup();
      return FAIL;
    }

/*----------------------redo-----------------------*/
char *end;
// //number of immigrant processes to be generated
int PI = strtol(argv[1], &end, 10);
// //max time for generation of new process
int IG = strtol(argv[2], &end, 10);
// //max time for judges entrance since he last left
// int JG = strtol(argv[3], &end, 10);
// //max time for obtaining of certificate
// int IT = strtol(argv[4], &end, 10);
// //max time for issuance of a certificate
// int JT = strtol(argv[5], &end, 10);
/*------------------------------------------------*/

//First division of process
    pid_t judge = fork();

    if (judge < 0) {
      fprintf(stderr, "error: fork failed\n");
      return FAIL;
    } else if (judge == 0) {
      //judge child process

      //process_judge();
      //exit(SUCCESS);
    } else {
      //second division of process
      pid_t immigrant = fork();

      if (immigrant < 0) {
        fprintf(stderr, "error: fork failed\n");
        return FAIL;
      } else if (immigrant == 0) {
        //immgirant child process
        gen_immigrants(PI, IG);
        exit(SUCCESS);
      } else {
        //waitpid();
      }
      //waitpid();
    }
  cleanup();
  //exit(SUCCESS);
  return SUCCESS;
}

/*------------------End of code--------------------*/
/*------------------------------------------------*/
