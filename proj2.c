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

/*--------------------Constants--------------------*/
#define FAIL 1
#define SUCCESS 0
#define MIN_MSEC 0
#define MAX_MSEC 2000
#define REQUIRED_ARGS 6

/*----------------Global variables-----------------*/
sem_t *semaphore = NULL;
FILE *output;



/*-------------------Functions--------------------*/
int errorCheck(int argc, char *argv[]);
int initialise();

/*------------------Semaphores--------------------*/



//deklarovat semafor
//namapovat do pamate
//pouzit
//nakoniec vsetky uzavriet

//chyby vypisovat na stderr, exit code 1
//tiez uvolnit vsetky alokovane zdroje

//xgulci00.ios.proj2.nazov

/**
* immigrants == PI
* timeImm == IG
* timeJudgeEnter == JG
* timeGetCertificate == IT
* timeIssueCertificate == JT
**/

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
  output = fopen("proj2.out", w);
  if(semaphore = sem_open("/xgulci00.ios.proj2.semaphore", O_CREAT | O_EXCL, 0666, 1)== SEM_FAILED) {
    fprintf(stderr, "error: initialisation of semaphore failed\n", );
    return FAIL;
  }
  return SUCCESS;
}

void cleanup() {
  //sem_close(sem);
  sem_unlink("xgulci00.ios.proj2.semaphore");
  if (output != NULL) {
    fclose(output);
  }
}
//
// void process_judge(delay,) {
//   exit(0);
// }
//
// void process_immigrant(delay, ) {
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
    // pid_t judge = fork();
    //
    // if(judge < 0) {
    //   fprintf(stderr, "error: invalid fork\n");
    //   return 1;
    // } else if (judge == 0) {
    //   //judge
    // } else {
    //   //gen_immigrants(pocet imigrantov, delay);
    // }
    //
    // pid_t judge = fork();
    //
    // if (first < 0) {
    //   fprintf(stdout, "Fork didnt work\n");
    // } else if (first == 0) {
    //   fprintf(stdout, "Ahoj ja som child process 1\n");
    // } else {
    //   fprintf(stdout, "Ahoj ja som parent process 1\n");
    //   pid_t second = fork();
    //   if(second < 0) {
    //     fprintf(stdout, "Chyba v druhom forku\n");
    //   } else if (second == 0) {
    //     fprintf(stdout, "AHoj ja som child process 2\n");
    //   } else {
    //     fprintf(stdout, "Ahoj ja som co ostalo\n");
    //   }
    // }
    //
    //prvni fork
    // pid_t immigrant = fork();
    //
    // if (immigrant < 0) {
    //     fprintf(stderr,"Vytvoreni child procesu selhalo(hacker)\n");
    //     // cleanUp(id,ptr);
    //     // exit(1);
    //     return 1;
    // }
    // else if (hacker == 0) {
    //     //child process #1
    //
    //     // processHack(hackTime,People,ptr,maxWaitTime,capacity,sailTime);
    // }
    // else
    // {//parent

  cleanup();
  exit(0);
  return SUCCESS;
}
