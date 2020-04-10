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

#define FAIL 1
#define SUCCESS 0
#define MIN_MSEC 0
#define MAX_MSEC 2000
#define REQUIRED_ARGS 6


int errorCheck(int argc, char *argv[]);

//chyby vypisovat na stderr, exit code 1
//tiez uvolnit vsetky alokovane zdroje

/**
* immigrants == PI
* timeImm == IG
* timeJudgeEnter == JG
* timeGetCertificate == IT
* timeIssueCertificate == JT
**/

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

  // if (timeImm < MIN_MSEC || timeImm > MAX_MSEC) {
  //   fprintf(stderr, "error: invalid argument (max time for generation of new process)\n");
  //   return FAIL;
  // }
  //
  // if (timeJudgeEnter < MIN_MSEC || timeJudgeEnter > MAX_MSEC) {
  //   fprintf(stderr, "error: invalid argument (max time for generation of new process)\n");
  //   return FAIL;
  // }
  //
  // if (timeGetCertificate < MIN_MSEC || timeJudgeEnter > MAX_MSEC) {
  //   fprintf(stderr, "error: invalid argument (max time for generation of new process)\n");
  //   return FAIL;
  // }
  //
  // if (timeIssueCertificate < MIN_MSEC || timeIssueCertificate > MAX_MSEC) {
  //   fprintf(stderr, "error: invalid argument (max time for generation of new process)\n");
  //   return FAIL;
  // }

  return SUCCESS;
}


int main(int argc, char *argv[]) {
  if (errorCheck(argc, argv) == FAIL)
    return FAIL;

  return SUCCESS;
}
