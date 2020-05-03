/* * * * * * * * * * * * * * *
*    Faneuil Hall Problem    *
*       IOS - project 2      *
*                            *
*     Sabina Gulcikova       *
* xgulci00@stud.fit.vutbr.cz *
*         06/04/20           *
* * * * * * * * * * * * * * */

/**
* @file proj2.c
*
* @mainpage Operating systems - Project 2
*
* @brief Synchronisation of processes - Faneuil Hall Problem
*
* @author Sabina Gulcikova <xgulci00@stud.fit.vutbr.cz>
*
* @date 03/05/2020
**/

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

/*-----------Global / shared variables------------*/

sem_t *semaphore;
FILE *output;
int *action;        /** action counter */
int *immID;
int *insideWaiting; /** == NE -> immigrants inside, not confirmed (entered) */
int *registered;    /** == NC -> registered immigrants, not confirmed (checked) */
int *immInside;     /** == NB -> immigrants inside the building */
int *confirmed;     /** number of immigrants that were given confirmation */

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
void process_immigrant(int identificator, int getCertif);
void process_judge(int delay, int totalImm, int toEnter);
void gen_immigrants(int numOfImmigrants, int delay, int getCertif);

/*------------------Semaphores--------------------*/

sem_t *printMessage;  /** avoids overlapping of printed messages */
sem_t *registration;  /** ensures individual registration of immigrants */
sem_t *allRegistered; /** all immigrants to be registered NC == NE */
sem_t *judgeInHouse;  /** forbids entrance/leaving when judge is inside --> NE stays the same */
sem_t *ongoingConfirmation; /** all registrations to be confirmed by judge NC => 0 */

/*-------------------Functions--------------------*/
/*--------------Initial precautions---------------*/

/**
* Function checks whether valid number of arguments was entered,
* whether they satisfy specified data type, and fit within allowed range
*
* @brief checks for potential errors caused by invalid input
*
* @return SUCCESS if valid number and type of arguments was entered
*
* @return ERROR if invalid input was entered
*
* @param argc number of arguments
*
* @param argv entered arguments
*/
int errorCheck(int argc, char *argv[]) {
  /** Checks whether proper number of arguments was entered */
  if (argc != REQUIRED_ARGS) {
    fprintf(stderr, "error: invalid number of arguments.\n");
    return FAIL;
  }

  /** Checks whether valid arguments were entered */
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

  /** Checks whether the arguments belong to allowed range */
  if (immigrants < 1) {
    fprintf(stderr, "error: invalid argument (number of processes)\n");
    return FAIL;
  }

  for (int i = 2; i < argc; i++) {
    int time = strtol(argv[i], &end, 10);

    if (time < MIN_MSEC || time > MAX_MSEC) {
      fprintf(stderr, "error: invalid argument (max time for generation of new process)\n");
      return FAIL;
    }
  }

  return SUCCESS;
}

/**
* Function prepares file for output, creates shared memory and loads semaphores
*
* @brief prepares resources for synchronisation of processes
*
* @return SUCCESS if no problems occured
*
* @return ERROR if problems occured when preparing the resources
*/
int initialise() {
  output = fopen("proj2.out", "w");
  memory_setup();
  open_semaphores();
  return SUCCESS;
}

/**
* Function loads all semaphores, then checks whether it was successful
*
* @brief opens all semaphores
*/
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

/**
* Function allocates shared memory to be used across all the processes
*
* @brief allocates resources
*/
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

    action = mmap(NULL,sizeof(int),PROT_READ|PROT_WRITE,MAP_SHARED,shmAction,0); /** the number of action that has just happened */
    immID = mmap(NULL,sizeof(int),PROT_READ|PROT_WRITE,MAP_SHARED,shmIdentif,0); /** ID of immigrant */
    immInside = mmap(NULL,sizeof(int),PROT_READ|PROT_WRITE,MAP_SHARED,shmInside,0); /** number of immigrants within the building */
    registered = mmap(NULL,sizeof(int),PROT_READ|PROT_WRITE,MAP_SHARED,shmRegistered,0); /** number of registered immigrants within the building */
    insideWaiting = mmap(NULL,sizeof(int),PROT_READ|PROT_WRITE,MAP_SHARED,shmWaiting,0); /** number of immigrants inside who havent been given confirmation*/
    confirmed = mmap(NULL,sizeof(int),PROT_READ|PROT_WRITE,MAP_SHARED,shmConfirmed,0); /** total number of confirmed immigrants*/

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

/**
* Function unlinks and closes all semaphores
*
* @brief unloads all semaphores
*/
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

/**
* When called, function puts given process to sleep for random period out of <0, delay> range
*
* @brief puts calling process to sleep
*
* @param delay maximum of the range for random generator
*/
void mysleep(int delay) {
  srand(time(NULL));
  usleep((rand()%delay+1)*1000);
}

/**
* Function closes the output file and unloads the semaphores
*
* @brief provides the cleanup of resources
*/
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

  close(shmAction);
  close(shmIdentif);
  close(shmInside);
  close(shmRegistered);
  close(shmWaiting);
  close(shmConfirmed);

}
/*------------------------------------------------*/

/*----------------Synchronisation-----------------*/

/**
* Function manages the individual actions of the judge process
* according to the state of semaphores and values of shared variables
*
* @pre the fork for of the main process was successful
*
* @brief manages the behaviour of judge process
*
* @param delay
*
* @param totalImm
*
* @param toEnter maximum time for judge to enter the building again
*/
void process_judge( int delay, int totalImm, int toEnter) {
  while(*confirmed < totalImm) {
    if(toEnter != 0) {
      mysleep(toEnter);
    }

    sem_wait(printMessage);
    (*action)++;
    setbuf(output, NULL); //empties the buffer for writing into input file
    fprintf(output, "%d     : JUDGE      : wants to enter\n", *action);
    fflush(output);
    sem_post(printMessage);

    sem_wait(judgeInHouse);

    sem_wait(printMessage);
    (*action)++;
    setbuf(output, NULL); //empties the buffer for writing into input file
    fprintf(output, "%d     : JUDGE      : enters      : %d      : %d       : %d \n", *action, *insideWaiting, *registered, *immInside);
    fflush(output);
    sem_post(printMessage);

    if (*insideWaiting != *registered) {
      sem_wait(printMessage);
      (*action)++;
      setbuf(output, NULL); //empties the buffer for writing into input file
      fprintf(output, "%d     : JUDGE      : waits for imm      : %d      : %d       : %d \n", *action, *insideWaiting, *registered, *immInside);
      fflush(output);
      sem_post(printMessage);
      sem_wait(allRegistered);
    }

    int toBeLet = 0;
    if (*insideWaiting == *registered) {

      sem_wait(printMessage);
      (*action)++;
      setbuf(output, NULL); //empties the buffer for writing into input file
      fprintf(output, "%d     : JUDGE      : starts confirmation      : %d      : %d       : %d \n", *action, *insideWaiting, *registered, *immInside);
      fflush(output);
      sem_post(printMessage);

      //sem_wait(ongoingConfirmation);
      if(delay != 0) {
        mysleep(delay);
      }

      (*confirmed)+= *registered;
      toBeLet = *registered;
      (*registered) = 0;
      (*insideWaiting) = 0;

      sem_wait(printMessage);
      (*action)++;
      setbuf(output, NULL); //empties the buffer for writing into input file
      fprintf(output, "%d     : JUDGE      : ends confirmation      : %d      : %d       : %d \n", *action, *insideWaiting, *registered, *immInside);
      fflush(output);
      sem_post(printMessage);

      for(int i = 0; i < toBeLet; i++) {
        sem_post(ongoingConfirmation); //only confirmed immigrants can get certificate
      }
   }

    if(delay != 0) {
      mysleep(delay);
    }

    sem_wait(printMessage);
    (*action)++;
    setbuf(output, NULL); //empties the buffer for writing into input file
    fprintf(output, "%d     : JUDGE      : leaves      : %d      : %d       : %d \n", *action, *insideWaiting, *registered, *immInside);
    fflush(output);
    sem_post(printMessage);

    sem_post(judgeInHouse);
  }

  sem_wait(printMessage);
  (*action)++;
  setbuf(output, NULL); //empties the buffer for writing into input file
  fprintf(output, "%d     : JUDGE      : finishes\n", *action);
  fflush(output);
  sem_post(printMessage);
}

/**
* Function manages the individual actions of the immigrant process
* according to the state of semaphores and values of shared variables
*
* @pre the fork in the immigrant generator was successful
*
* @brief manages the behaviour of immigrant process
*
* @param identificator identificator of immigrant process
*
* @param getCertif maximum time for obtaining a certificate
*/
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
    setbuf(output, NULL); //empties the buffer for writing into input file
    fprintf(output, "%d     : IMM %d      : checks      : %d      : %d       : %d \n", *action, identificator, *insideWaiting, *registered, *immInside);
    fflush(output);
    sem_post(printMessage);
    sem_post(registration);

    if(*registered == *insideWaiting) {
      sem_post(allRegistered);
    }

    sem_wait(ongoingConfirmation); /** Waits for the judge to finish confirmation*/

    sem_wait(printMessage);
    (*action)++;
    setbuf(output, NULL); //empties the buffer for writing into input file
    fprintf(output, "%d     : IMM %d      : wants certificate      : %d      : %d       : %d \n", *action, identificator, *insideWaiting, *registered, *immInside);
    fflush(output);
    sem_post(printMessage);

    if(getCertif != 0) {
      mysleep(getCertif);
    }

    sem_wait(printMessage);
    (*action)++;
    setbuf(output, NULL); //empties the buffer for writing into input file
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

/**
* Function generates immigrant processes according to the desired input value
*
* @pre the fork of the process was successful
*
* @brief creates immigrant processes
*
* @param numOfImmigrants number of immigrant processes to be created
*
* @param delay maximum time for the creation of new immigrant process
*
* @param getCertif maximum time for immigrant to get certificate
*/
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
        exit(SUCCESS);

      } else {
          //while ((wpid = wait(&status)) > 0); //waits till child process ends
        //    exit(SUCCESS);
      }
      if(delay != 0) {
        mysleep(delay);
      }
    }
    while ((wpid = wait(&status)) > 0); //waits till child process ends
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

int status = 0;
pid_t wpid;

//First division of process
    pid_t judge = fork();

    if (judge < 0) { // fork did not work
      fprintf(stderr, "error: fork failed\n");
      return FAIL;
    } else if (judge == 0) {
      //judge child process
      process_judge(JT, PI, JG);
      exit(SUCCESS);
    } else { // judge parent process
      //second division of process
      pid_t immigrant = fork();

      if (immigrant < 0) { //fork did not work
        fprintf(stderr, "error: fork failed\n");
        return FAIL;
      } else if (immigrant == 0) {
      //immgirant child process
        gen_immigrants(PI, IG, IT);
        exit(SUCCESS);
      } else { // immigrant parent process
       while ((wpid = wait(&status)) > 0); //waits till child process ends
      }
     while ((wpid = wait(&status)) > 0); //waits till child process ends
    }

  cleanup();
  return SUCCESS;
}

/*------------------End of code--------------------*/
/*-------------------------------------------------*/
