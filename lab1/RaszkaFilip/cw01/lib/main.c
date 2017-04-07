#include "list.h"
#include "helpers.h"
#include "tree.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>

int main()
{
    int i;
    srand(time(NULL));
    int RANGE = 10;
    char* names[] = {"Adam", "Ben", "Ciri", "Dominic", "Edgar", "Felicity", "Garry", "Harold", "Ingrid", "Juliet"};
    char* surnames[] = {"Tashjian", "Leal", "Griffith", "Morita", "Da silva", "Raines", "Liou", "Fulco", "Lorenzini", "Elgan"};
    char* birthDates[] = {"2013-04-27","2011-04-24","1995-06-17","1990-04-12","2000-03-18","1995-07-28","2002-03-11","2003-06-18","2005-12-17","1912-03-18"};
    char* emails[] = {"@gmail.com", "@onet.pl", "@niepodam.pl", "@interia.pl","@example.com", "@goodemail.pl","@aaa.com", "@smth.pl","@random.com", "@random2.com"};
    char* phones[] = {"123456789", "365385729", "194756294","194856381","284659264","1243452557","128542466","85679634","942445351","943875943"};
    char* addresses[] = {"antoni 1/3", "burdun", "cekari", "downhill", "eversts", "falcons", "garryhill", "hunter", "indiands", "winkleys"};

    node_t* lBook = createListBook();
    treeBook* tBook = createTreeBook(surnameCompare);

    for(i=0; i<1000; i++){
        char* name = names[rand()%RANGE];
        char* surname = surnames[rand()%RANGE];
        char* birthDate = birthDates[rand()%RANGE];
        //char* email = concat(name, emails[rand()%RANGE]);
        char* email = emails[rand()%RANGE];
        char* phone = phones[rand()%RANGE];
        char* address = addresses[rand()%RANGE];

        addNewToList(lBook, name, surname, birthDate, email, phone, address);
        addNewToTree(tBook, name, surname, birthDate, email, phone, address);
    }

    sortList(lBook, nameCompare);

    findInList(lBook, nameCompare, "Adam");
    findInList(lBook, nameCompare, "Juliet");

    deleteFromList(lBook, nameCompare, "Adam");
    deleteFromList(lBook, nameCompare, "Juliet");

    showList(lBook);
    deleteList(lBook);

    ///////////////////////////////////////////////////

    sortTree(tBook, nameCompare);

    findInTree(tBook, nameCompare, "Edgar");
    findInTree(tBook, nameCompare, "Juliet");

    deleteFromTree(tBook, nameCompare, "Edgar");
    deleteFromTree(tBook, nameCompare, "Juliet");

    showTree(tBook);
    deleteTree(tBook);

    return 1;
}
