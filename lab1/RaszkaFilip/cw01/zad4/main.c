#include "../lib/list.h"
#include "../lib/tree.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/times.h>
#include <sys/resource.h>

void tStart();
void tStop();

struct tms usTime;
clock_t rTime;

int main(void)
{
    srand(time(NULL));
    int RANGE = 10;
    char* names[] = {"Adam", "Ben", "Ciri", "Dominic", "Edgar", "Felicity", "Garry", "Harold", "Ingrid", "Juliet"};
    char* surnames[] = {"Tashjian", "Leal", "Griffith", "Morita", "Da silva", "Raines", "Liou", "Fulco", "Lorenzini", "Elgan"};
    char* birthDates[] = {"2013-04-27","2011-04-24","1995-06-17","1990-04-12","2000-03-18","1995-07-28","2002-03-11","2003-06-18","2005-12-17","1912-03-18"};
    char* emails[] = {"@gmail.com", "@onet.pl", "@niepodam.pl", "@interia.pl","@example.com", "@goodemail.pl","@aaa.com", "@smth.pl","@random.com", "@random2.com"};
    char* phones[] = {"123456789", "365385729", "194756294","194856381","284659264","1243452557","128542466","85679634","942445351","943875943"};
    char* addresses[] = {"antoni 1/3", "burdun", "cekari", "downhill", "eversts", "falcons", "garryhill", "hunter", "indiands", "winkleys"};

    printf("-------------------------------LIST------------------------------------\n \n");
    printf("-------------------------------LIST------------------------------------\n \n");

    printf("Creating 10000-element list Book: \n");
    node_t* lBook = createListBook();

    tStart();
    for(int i=0; i<10000; i++){
        char* name = ranStr(names[rand()%RANGE]);
        char* surname = ranStr(surnames[rand()%RANGE]);
        char* birthDate = ranStr(birthDates[rand()%RANGE]);
        char* email = ranStr(concat(name, emails[rand()%RANGE]));
        char* phone = ranStrNum(phones[rand()%RANGE]);
        char* address = ranStr(addresses[rand()%RANGE]);

        addNewToList(lBook, name, surname, birthDate, email, phone, address);
    }
    tStop();

    printf("Adding two contacts to the full list \n ");
    tStart();
    addNewToList(lBook, "A", "A", "A", "A", "A", "A");
    addNewToList(lBook, "z", "z", "z", "z", "z", "z");
    tStop();

    printf("Sorting list by names: \n");
    tStart();
    sortList(lBook, nameCompare);
    tStop();

    printf("Optimistic search: \n");
    tStart();
    findInList(lBook, nameCompare, "A");
    tStop();

    printf("Pesimistic search: \n");
    tStart();
    findInList(lBook, nameCompare, "z");
    tStop();

    printf("Optimistic delete: \n");
    tStart();
    deleteFromList(lBook, nameCompare, "A");
    tStop();

    printf("Pesimistic delete: \n");
    tStart();
    deleteFromList(lBook, nameCompare, "z");
    tStop();

    printf("Deleting entire list: \n");
    tStart();
    deleteList(lBook);
    tStop();

    ///////////////////////////////////////////////////
    printf("-------------------------------TREE------------------------------------\n \n");
    printf("-------------------------------TREE------------------------------------\n \n");


    printf("Creating 10000-element tree Book: \n");
    tStart();
    treeBook* tBook = createTreeBook(surnameCompare);
    for(int i=0; i<10000; i++){
        char* name = ranStr(names[rand()%RANGE]);
        char* surname = ranStr(surnames[rand()%RANGE]);
        char* birthDate = ranStr(birthDates[rand()%RANGE]);
        char* email = ranStr(concat(name, emails[rand()%RANGE]));
        char* phone = ranStrNum(phones[rand()%RANGE]);
        char* address = ranStr(addresses[rand()%RANGE]);

        addNewToTree(tBook, name, surname, birthDate, email, phone, address);
    }
    tStop();

    printf("Adding two contacts to the full tree: \n");
    tStart();
    addNewToTree(tBook, "FD", "FD", "FD", "FD", "FD", "FD");
    addNewToTree(tBook, "A", "A", "A", "A", "A", "A");
    tStop();

    printf("Rebuilding the tree by names: \n");
    tStart();
    sortTree(tBook, nameCompare);
    tStop();

    printf("Optimistic search: \n");
    tStart();
    findInTree(tBook, nameCompare, "A");
    tStop();

    printf("Pesimistic search: \n");
    tStart();
    findInTree(tBook, nameCompare, "FD");
    tStop();

    printf("Optimistic delete: \n");
    tStart();
    deleteFromTree(tBook, nameCompare, "A");
    tStop();

    printf("Pesimistic delete: \n");
    tStart();
    deleteFromTree(tBook, nameCompare, "FD");
    tStop();

    printf("Deleting entire tree: \n");
    tStart();
    deleteTree(tBook);
    tStop();

    printf("Its going to crush %s", findInTree(tBook, nameCompare, "FD")->contact->surname);

    printf("\n \n");
}


#define CLK sysconf(_SC_CLK_TCK)

void tStart(){
  rTime = clock();
  times(&usTime);
}

void tStop(){
  clock_t rNow = clock();
  struct tms usNow;
  times(&usNow);

  double difReal = ((double)(rNow - rTime)) / CLOCKS_PER_SEC;
  double difUser = ((double) (usNow.tms_utime - usTime.tms_utime)) / CLK;
  double difSys = ((double)(usNow.tms_stime - usTime.tms_stime)) / CLK;

  printf("\n Time of operation:\tReal: %.6f\tUser: %.6f\tSystem: %.6f \n \n", difReal, difUser, difSys);
}
