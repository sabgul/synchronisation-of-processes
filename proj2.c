/* * * * * * * * * * * * * * *
*    Faneuil Hall Problem    *
*       IOS - project 2      *
*                            *
*     Sabina Gulcikova       *
* xgulci00@stud.fit.vutbr.cz *
*         06/04/20           *
* * * * * * * * * * * * * * */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define SUCCESS 0
#define FAIL 1
#define REQUIRED_ARGS 6

//chyby vypisovat na stderr, exit code 1
//tiez uvolnit vsetky alokovane zdroje
//vstup neodpoveda formatu, alebo je moc maly/velky

/**
* immigrants == PI
* timeImm == IG
* timeJudgeEnter == JG
* timeGetCertificate == IT
* timeIssueCertificate == JT
**/

int main(int argc, char *argv[]) {
  char *end;
  int immigrants = strtol(argv[1], &end, 10);
  int timeImm = strtol(argv[2], &end, 10);
  int timeJudgeEnter = strtol(argv[3], &end, 10);
  int timeGetCertificate = strtol(argv[4], &end, 10);
  int timeIssueCertificate = strtol(argv[5], &end, 10);

  if (argc != REQUIRED_ARGS) {
    fprintf(stderr, "error: invalid number of arguments.\n");
    return FAIL;
  }

  if (immigrants < 1) {
    fprintf(stderr, "error: invalid argument (number of processes)\n");
    return FAIL;
  }

  if (timeImm < 0 || timeImm > 2000) {
    fprintf(stderr, "error: invalid argument (max time for generation of new process)\n");
    return FAIL;
  }

  if (timeJudgeEnter < 0 || timeJudgeEnter > 2000) {
    fprintf(stderr, "error: invalid argument (max time for generation of new process)\n");
    return FAIL;
  }

  if (timeGetCertificate < 0 || timeJudgeEnter > 2000) {
    fprintf(stderr, "error: invalid argument (max time for generation of new process)\n");
    return FAIL;
  }

  if (timeIssueCertificate < 0 || timeIssueCertificate > 2000) {
    fprintf(stderr, "error: invalid argument (max time for generation of new process)\n");
    return FAIL;
  }

  return SUCCESS;
}
