#ifndef TREE_H
#define TREE_H
#include "helpers.h"

//enum SORTED_BY {NAME, SURNAME, ADDRESS, EMAIL, BIRTH, PHONE};

typedef struct tree {
    struct contact* contact;
    struct tree* left;
    struct tree* right;
    struct tree* parent;
} tree;

typedef struct treeBook{
    tree* root;
    contactComparator comparator;
} treeBook;

treeBook* createTreeBook(contactComparator); // zwraca struct treeBook

void deleteTree(treeBook* book); // rekurencyjnie uwalniaj

void addNewToTree(treeBook* book, char* name, char*surname, char* birthDate, char* email, char* phone, char* address);

tree* findInTree(treeBook* book, contactComparator, char* val);

void deleteFromTree(treeBook* book, contactComparator, char* val); // interfejsowa funkcja

void showTree(treeBook* book);

void sortTree(treeBook*, contactComparator); // funkcja sama wybierze comparator

#endif // TREE_H



