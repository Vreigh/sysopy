#ifndef LIST_H
#define LIST_H
#include "helpers.h"

typedef struct node {
    struct contact* contact;
    struct node* next;
    struct node* prev;
} node_t;

node_t* createListBook();

void deleteList(node_t* head);

void addNewToList(node_t* head, char* name, char*surname, char* birthDate, char* email, char* phone, char* address);

node_t* findInList(node_t* head, contactComparator comparator, char* val);

void deleteFromList(node_t* head, contactComparator comparator, char* val);

void showList(node_t* head);

void sortList(node_t* head, contactComparator comparator);


#endif // LIST_H





