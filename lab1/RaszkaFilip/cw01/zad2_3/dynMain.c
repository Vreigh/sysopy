#include "../lib/list.h"
#include "../lib/tree.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/times.h>
#include <sys/resource.h>
#include <dlfcn.h>

void tStart();
void tStop();

struct tms usTime;
clock_t rTime;

int main(void)
{
    void* lib = dlopen("libAddressSh.so", RTLD_LAZY);

    node_t* (*createListBook)();
    createListBook = dlsym(lib, "createListBook");

    void (*deleteList)(node_t* head);
    deleteList = dlsym(lib, "deleteList");

    void (*addNewToList)(node_t* head, char* name, char*surname, char* birthDate, char* email, char* phone, char* address);
    addNewToList = dlsym(lib, "addNewToList");

    node_t* (*findInList)(node_t* head, contactComparator comparator, char* val);
    findInList = dlsym(lib, "findInList");

    void (*deleteFromList)(node_t* head, contactComparator comparator, char* val);
    deleteFromList = dlsym(lib, "deleteFromList");

    void (*showList)(node_t* head);
    showList = dlsym(lib, "showList");

    void (*sortList)(node_t* head, contactComparator comparator);
    sortList = dlsym(lib, "sortList");

    treeBook* (*createTreeBook)(contactComparator);
    createTreeBook = dlsym(lib, "createTreeBook");

    void (*deleteTree)(treeBook* book);
    deleteTree = dlsym(lib, "deleteTree");

    void (*addNewToTree)(treeBook* book, char* name, char*surname, char* birthDate, char* email, char* phone, char* address);
    addNewToTree = dlsym(lib, "addNewToTree");

    tree* (*findInTree)(treeBook* book, contactComparator, char* val);
    findInTree = dlsym(lib, "findInTree");

    void (*deleteFromTree)(treeBook* book, contactComparator, char* val);
    deleteFromTree = dlsym(lib, "deleteFromTree");

    void (*showTree)(treeBook* book);
    showTree = dlsym(lib, "showTree");

    void (*sortTree)(treeBook*, contactComparator);
    sortTree = dlsym(lib, "sortTree");

    char* (*ranStr)(char*);
    ranStr = dlsym(lib, "ranStr");

    char* (*ranStrNum)(char*);
    ranStrNum = dlsym(lib, "ranStrNum");

    char* (*concat)(char *s1, char *s2);
    concat = dlsym(lib, "concat");

    int (*nameCompare)(contact* one, contact* two);
    nameCompare = dlsym(lib, "nameCompare");
    int (*surnameCompare)(contact* one, contact* two);
    surnameCompare = dlsym(lib, "surnameCompare");
    int (*birthDateCompare)(contact* one, contact* two);
    birthDateCompare = dlsym(lib, "birthDateCompare");
    int (*emailCompare)(contact* one, contact* two);
    emailCompare = dlsym(lib, "emailCompare");
    int (*phoneCompare)(contact* one, contact* two);
    phoneCompare = dlsym(lib, "phoneCompare");
    int (*addressCompare)(contact* one, contact* two);
    addressCompare = dlsym(lib, "addressCompare");

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
