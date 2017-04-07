#include<stdio.h>
#include <stdlib.h>
#include <string.h>
#include "helpers.h"

void swapStrings(char** one, char** two){
    char* tmp = *one;
    *one = *two;
    *two = tmp;
}

void swapContacts(contact** one, contact** two){
    contact* tmp = *one;
    *one = *two;
    *two = tmp;
}

contact* createContact(char* name, char* surname, char* birthDate, char* email, char* phone, char* address ){
  contact* new = malloc(sizeof(contact));

  if(name != NULL){
    new->name = malloc(60);
    strncpy(new->name, name, 60);

    new->surname = malloc(60);
    strncpy(new->surname, surname, 60);

    new->birthDate = malloc(60);
    strncpy(new->birthDate, birthDate, 60);

    new->email = malloc(60);
    strncpy(new->email, email, 60);

    new->phone = malloc(15);
    strncpy(new->phone, phone, 15);

    new->address = malloc(60);
    strncpy(new->address, address, 60);
  }else{
    new->name = NULL;
    new->surname = NULL;
    new->birthDate = NULL;
    new->email = NULL;
    new->phone = NULL;
    new->address = NULL;
  }

      return new;
}

void freeContact(contact* deleting){
    if(deleting->name != NULL) free(deleting->name);
    if(deleting->surname != NULL) free(deleting->surname);
    if(deleting->birthDate != NULL) free(deleting->birthDate);
    if(deleting->email != NULL) free(deleting->email);
    if(deleting->phone != NULL) free(deleting->phone);
    if(deleting->address != NULL) free(deleting->address);

    free(deleting);
}

int nameCompare(contact* one, contact* two){
    return strcmp(one->name, two->name);
}
int surnameCompare(contact* one, contact* two){
    return strcmp(one->surname, two->surname);
}
int birthDateCompare(contact* one, contact* two){
    return strcmp(one->birthDate, two->birthDate);
}
int emailCompare(contact* one, contact* two){
    return strcmp(one->email, two->email);
}
int phoneCompare(contact* one, contact* two){
    return strcmp(one->phone, two->phone);
}
int addressCompare(contact* one, contact* two){
    return strcmp(one->address, two->address);
}

char* ranStr(char* s){
    int size = 26;
    const char* charset = "QWERTYUIOPASDFGHJKLZXCVBNM";

    char* one = malloc(sizeof(char)* 2);
    one[0] = charset[rand()%size];
    one[1] = '\0';

    char* two = malloc(sizeof(char)* 2);
    two[0] = charset[rand()%size];
    two[1] = '\0';

    char* prefix = concat(one, two);

    return concat(prefix, s);
}

char* ranStrNum(char* s){
    int size = 10;
    const char* charset = "0123456789";

    char* one = malloc(sizeof(char)* 2);
    one[0] = charset[rand()%size];
    one[1] = '\0';

    char* two = malloc(sizeof(char)* 2);
    two[0] = charset[rand()%size];
    two[1] = '\0';

    char* prefix = concat(one, two);

    return concat(prefix, s);
}

char* concat(char* one, char* two){
    char* new = malloc(sizeof(char) * (strlen(one) + strlen(two) + 1));
    strcpy(new, one);
    strcat(new, two);
    return new;
}
