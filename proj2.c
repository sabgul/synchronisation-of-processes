/* * * * * * * * * * * * * * *
*    Faneuil Hall Problem    *
*       IOS - project 2      *
*                            *
*     Sabina Gulcikova       *
* xgulci00@stud.fit.vutbr.cz *
*         06/04/20           *
* * * * * * * * * * * * * * */

//#include "proj2.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <pthread.h>
#include <semaphore.h>

/*--------------------Constants--------------------*/
#define FAIL 1
#define SUCCESS 0
#define MIN_MSEC 0
#define MAX_MSEC 2000
#define REQUIRED_ARGS 6

/*----------------Global variables-----------------*/
sem_t *semaphore;
FILE *output;
int *action;
int *immID;
int *insideWaiting; // == NE -> inside, not confirmed (entered)
int *registered; // == NC -> registered, not confirmed (checked)
int *immInside; // == NB -> inside the building
int *confirmed; // number of immigrants that were given confirmation
/*-----------------Shared memory------------------*/
int shmAction;
int shmIdentif;
int shmInside;
int shmRegistered;
int shmWaiting;
int shmConfirmed;
/*-------------------Functions--------------------*/
int errorCheck(int argc, char *argv[]);
int initialise();
void open_semaphores();
void memory_setup();
void close_semaphores();
void cleanup();
void mysleep(int delay);
void process_immigrant(int identificator, int getCertif/*int delay, int numberOfImmigrants, int timeToGetCertif*/);
void process_judge( int delay, int totalImm, int toEnter/*delay, timeToEnter, timeToConfirm*/ );
void gen_immigrants(int numOfImmigrants, int delay, int getCertif);
/*------------------Semaphores--------------------*/

sem_t *printMessage; /*avoids overlapping*/
sem_t *registration; /*immigrants have to registrate individually*/
sem_t *allRegistered; /*all immigrants to be registered NC == NE*/
sem_t *judgeInHouse; /*no one can enter / leave --> NE stays the same*/
sem_t *ongoingConfirmation; /*all registrations to be confirmed by judge NC => 0*/

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

/*Function prepares file for output, creates shared memory and loads semaphores*/
int initialise() {
 output = fopen("proj2.out", "w");
  //output = stdout;
  memory_setup();
  open_semaphores();
  return SUCCESS;
}

/*Function opens all semaphores*/
void open_semaphores() {
  printMessage = sem_open("/xgulci00.ios.proj2.print", O_CREAT | O_EXCL, 0644, 1);
  judgeInHouse = sem_open("/xgulci00.ios.proj2.judge", O_CREAT | O_EXCL, 0644, 1);
  allRegistered = sem_open("/xgulci00.ios.proj2.ready", O_CREAT | O_EXCL, 0644, 1);
  registration = sem_open("/xgulci00.ios.proj2.registration", O_CREAT | O_EXCL, 0644, 1);
  ongoingConfirmation = sem_open("/xgulci00.ios.proj2.confirmation", O_CREAT | O_EXCL, 0644, 0);

  if (printMessage == SEM_FAILED ||
      judgeInHouse == SEM_FAILED ||
      registration == SEM_FAILED ||
      allRegistered == SEM_FAILED ||
      ongoingConfirmation == SEM_FAILED) {
    fprintf(stderr, "error: loading of semaphores failed\n");
    cleanup();
    exit(FAIL);
  }
}

/*Function allocates shared memory to be used across all the processes*/
void memory_setup() {
    shmAction = shm_open("/xgulci00.action", O_CREAT | O_EXCL | O_RDWR, 0644);
    shmIdentif = shm_open("/xgulci00.identif", O_CREAT | O_EXCL | O_RDWR, 0644);
    shmInside = shm_open("/xgulci00.inside", O_CREAT | O_EXCL | O_RDWR, 0644);
    shmRegistered = shm_open("/xgulci00.registered", O_CREAT | O_EXCL | O_RDWR, 0644);
    shmWaiting = shm_open("/xgulci00.waiting", O_CREAT | O_EXCL | O_RDWR, 0644);
    shmConfirmed = shm_open("/xgulci00.confirmed", O_CREAT | O_EXCL | O_RDWR, 0644);

    ftruncate(shmAction,sizeof(int));
    ftruncate(shmIdentif,sizeof(int));
    ftruncate(shmInside,sizeof(int));
    ftruncate(shmRegistered, sizeof(int));
    ftruncate(shmWaiting, sizeof(int));
    ftruncate(shmConfirmed, sizeof(int));

    action = mmap(NULL,sizeof(int),PROT_READ|PROT_WRITE,MAP_SHARED,shmAction,0); //the number of action that has just happened
    immID = mmap(NULL,sizeof(int),PROT_READ|PROT_WRITE,MAP_SHARED,shmIdentif,0); //ID of immigrant
    immInside = mmap(NULL,sizeof(int),PROT_READ|PROT_WRITE,MAP_SHARED,shmInside,0); //number of immigrants within the building
    registered = mmap(NULL,sizeof(int),PROT_READ|PROT_WRITE,MAP_SHARED,shmRegistered,0); //number of registered immigrants within the building
    insideWaiting = mmap(NULL,sizeof(int),PROT_READ|PROT_WRITE,MAP_SHARED,shmWaiting,0);
    confirmed = mmap(NULL,sizeof(int),PROT_READ|PROT_WRITE,MAP_SHARED,shmConfirmed,0);

    if (action == MAP_FAILED ||
        immID == MAP_FAILED ||
        immInside == MAP_FAILED ||
        registered == MAP_FAILED ||
        insideWaiting == MAP_FAILED ||
        confirmed == MAP_FAILED) {
      fprintf(stderr, "error: mapping failed\n");
      cleanup();
      exit(FAIL);
    }
}

/*Function unlinks and closes all semaphores*/
void close_semaphores() {
  sem_close(printMessage);
  sem_close(allRegistered);
  sem_close(registration);
  sem_close(judgeInHouse);
  sem_close(ongoingConfirmation);

  sem_unlink("/xgulci00.ios.proj2.print");
  sem_unlink("/xgulci00.ios.proj2.judge");
  sem_unlink("/xgulci00.ios.proj2.registration");
  sem_unlink("/xgulci00.ios.proj2.ready");
  sem_unlink("/xgulci00.ios.proj2.confirmation");
}

void mysleep(int delay) {
  srand(time(NULL));
  usleep((rand()%delay+1)*1000);
}

/*Function closes the ouput file and deals with semaphores*/
void cleanup() {
  close_semaphores();

  if (output != NULL) {
    fclose(output);
  }

  munmap(action, sizeof(int));
  munmap(immID, sizeof(int));
  munmap(immInside, sizeof(int));
  munmap(registered, sizeof(int));
  munmap(insideWaiting, sizeof(int));
  munmap(confirmed, sizeof(int));

  shm_unlink("/xgulci00.action");
  shm_unlink("/xgulci00.identif");
  shm_unlink("/xgulci00.inside");
  shm_unlink("/xgulci00.registered");
  shm_unlink("/xgulci00.waiting");
  shm_unlink("/xgulci00.confirmed");
  //close(shmAction);

  close(shmAction);
  close(shmIdentif);
  close(shmInside);
  close(shmRegistered);
  close(shmWaiting);
  close(shmConfirmed);

}
/*------------------------------------------------*/

/*----------------Synchronisation-----------------*/
void process_judge( int delay, int totalImm, int toEnter/*delay, timeToEnter, timeToConfirm*/ ) {
while(*confirmed < totalImm) {
  if(toEnter != 0) {
    mysleep(toEnter);
  }
  sem_wait(printMessage);

  (*action)++; //so that we start from 1
  setbuf(output, NULL);
  fprintf(output, "%d     : JUDGE      : wants to enter\n", *action);
  fflush(output);
  sem_post(printMessage);


  sem_wait(judgeInHouse);
  sem_wait(printMessage);

  (*action)++; //so that we start from 1
  setbuf(output, NULL);
  fprintf(output, "%d     : JUDGE      : enters      : %d      : %d       : %d \n", *action, *insideWaiting, *registered, *immInside);
  fflush(output);
  sem_post(printMessage);

  if (*insideWaiting != *registered) {
    //sem_wait(registration);
    sem_wait(printMessage);

    (*action)++; //so that we start from 1
    setbuf(output, NULL);
    fprintf(output, "%d     : JUDGE      : waits for imm      : %d      : %d       : %d \n", *action, *insideWaiting, *registered, *immInside);
    fflush(output);
    sem_post(printMessage);
    //sem_post(registration);
    sem_wait(allRegistered);
    //while(*insideWaiting != *registered);
  }

  int toBeLet = 0;
  if (*insideWaiting == *registered) {
    sem_wait(printMessage);

    (*action)++; //so that we start from 1
    setbuf(output, NULL);
    fprintf(output, "%d     : JUDGE      : starts confirmation      : %d      : %d       : %d \n", *action, *insideWaiting, *registered, *immInside);
    fflush(output);
    sem_post(printMessage);

    //sem_wait(ongoingConfirmation);
    if(delay != 0) {
      mysleep(delay);
    }
    (*confirmed)+= *registered;
    //(*insideWaiting)-= registered;
    (*action)++; //so that we start from 1
    toBeLet = *registered;
    (*registered) = 0;
    (*insideWaiting) = 0;
    sem_wait(printMessage);
    setbuf(output, NULL);
    fprintf(output, "%d     : JUDGE      : ends confirmation      : %d      : %d       : %d \n", *action, *insideWaiting, *registered, *immInside);
    fflush(output);
    sem_post(printMessage);
    for(int i = 0; i < toBeLet; i++){
      sem_post(ongoingConfirmation);
    }
 }

  if(delay != 0) {
    mysleep(delay);
  }
  sem_wait(printMessage);
  (*action)++; //so that we start from 1
  setbuf(output, NULL);
  fprintf(output, "%d     : JUDGE      : leaves      : %d      : %d       : %d \n", *action, *insideWaiting, *registered, *immInside);
  fflush(output);
  sem_post(printMessage);

  sem_post(judgeInHouse);
}

  sem_wait(printMessage);
  (*action)++; //so that we start from 1
  setbuf(output, NULL);
  fprintf(output, "%d     : JUDGE      : finishes\n", *action);
  fflush(output);
  sem_post(printMessage);
}
//

void process_immigrant(int identificator, int getCertif) {
    sem_wait(judgeInHouse); //noJudge.wait()
    sem_wait(printMessage);
    //enter()
    (*action)++; //so that we start from 1
    (*immInside)++; //number of entered immigrants (entered++)
    (*insideWaiting)++;
    setbuf(output, NULL);
    fprintf(output, "%d     : IMM %d      : enters      : %d      : %d       : %d \n", *action, identificator, *insideWaiting, *registered, *immInside);
    fflush(output);
    sem_post(printMessage);
    sem_post(judgeInHouse);

    //registration
    sem_wait(registration); //mutex.wait
    sem_wait(printMessage);
    (*action)++;
    (*registered)++; //checked++
    setbuf(output, NULL);
    fprintf(output, "%d     : IMM %d      : checks      : %d      : %d       : %d \n", *action, identificator, *insideWaiting, *registered, *immInside);
    fflush(output);
    sem_post(printMessage);
    sem_post(registration);

    /*
    if judge = and entered == checked
    sem_post(allSignedIn)
    pass the mutex
    else:
    post_e*/

    if(*registered == *insideWaiting) {
      sem_post(allRegistered);
    }
    //caka na vydanie certifikatu sudcom

////////////////////////////////////////////////////////////////////////////////
    sem_wait(ongoingConfirmation);
    sem_wait(printMessage);

    (*action)++; //so that we start from 1
    setbuf(output, NULL);
    fprintf(output, "%d     : IMM %d      : wants certificate      : %d      : %d       : %d \n", *action, identificator, *insideWaiting, *registered, *immInside);
    fflush(output);
    sem_post(printMessage);
    if(getCertif != 0) {

      mysleep(getCertif);
    }

    sem_wait(printMessage);

    (*action)++; //so that we start from 1
    //NE a NC su znizene o pocet pristahovalcov, ktori uz dostali rozhodnutie
    //(*registered)--;
    //(*insideWaiting)--;
    setbuf(output, NULL);
    fprintf(output, "%d     : IMM %d      : got certificate      : %d      : %d       : %d \n", *action, identificator, *insideWaiting, *registered, *immInside);
    fflush(output);
    sem_post(printMessage);

    sem_wait(judgeInHouse);
    sem_wait(printMessage);
    (*action)++;
    (*immInside)--;
    setbuf(output, NULL);
    fprintf(output, "%d     : IMM %d      : leaves      : %d      : %d       : %d \n", *action, identificator, *insideWaiting, *registered, *immInside);
    fflush(output);
    sem_post(printMessage);
    sem_post(judgeInHouse);
  //  exit(SUCCESS);
}
  //

void gen_immigrants(int numOfImmigrants, int delay, int getCertif) {
    int status = 0;
    pid_t wpid;

    for(int i = 0; i < numOfImmigrants; i ++) {

      pid_t imm = fork();

      if(imm < 0) {
        fprintf(stderr, "error: invalid fork\n");
        cleanup();
        exit(FAIL);

      } else if (imm == 0) {

        sem_wait(printMessage);
        (*action)++;
        setbuf(output, NULL);
        fprintf(output, "%d     : IMM %d      : starts \n", *action, *immID + 1);
        sem_post(printMessage);

        (*immID) = i + 1;
        process_immigrant(*immID, getCertif);
        //cleanup();
        exit(SUCCESS);

      } else {
    //    exit(SUCCESS);
      }
      if(delay != 0) {
        mysleep(delay);
      }
    }
    while ((wpid = wait(&status)) > 0); //waits till child process ends
    //for(int i = 0; i<numOfImmigrants; i++) {wait(NULL);}
    exit(SUCCESS);
  }


/*------------------------------------------------*/
/*--------------------MAIN------------------------*/
/*------------------------------------------------*/

int main(int argc, char *argv[]) {
  if (errorCheck(argc, argv) == FAIL) {
    return FAIL;
  }

  if (initialise() == FAIL) {
    cleanup();
    return FAIL;
  }

/*------------------arguments----------------------*/
char *end;
// //number of immigrant processes to be generated
int PI = strtol(argv[1], &end, 10);
// //max time for generation of new process
int IG = strtol(argv[2], &end, 10);
// //max time for judges entrance since he last left
int JG = strtol(argv[3], &end, 10);
// //max time for obtaining of certificate
int IT = strtol(argv[4], &end, 10);
// //max time for issuance of a certificate
int JT = strtol(argv[5], &end, 10);
/*------------------------------------------------*/
int status = 0;
pid_t wpid;

//First division of process
    pid_t judge = fork();

    if (judge < 0) {
      fprintf(stderr, "error: fork failed\n");
      return FAIL;
    } else if (judge == 0) {
      //judge child process
      process_judge(JT, PI, JG);
      exit(SUCCESS);
    } else {
      //second division of process
      pid_t immigrant = fork();

      if (immigrant < 0) {
        fprintf(stderr, "error: fork failed\n");
        return FAIL;
      } else if (immigrant == 0) {
      //immgirant child process
        gen_immigrants(PI, IG, IT);
        exit(SUCCESS);
      } else {
       while ((wpid = wait(&status)) > 0); //waits till child process ends
      //wait(NULL);
      }
     while ((wpid = wait(&status)) > 0); //waits till child process ends
    //wait(NULL);
    }
  //exit(SUCCESS);
  //wait(NULL);
  //wait(NULL);
  cleanup();
  return SUCCESS;
}

/*------------------End of code--------------------*/
/*-------------------------------------------------*/
