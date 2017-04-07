#ifndef HELPERS_H
#define HELPERS_H
typedef struct contact{
    char* name;
    char* surname;
    char* birthDate;
    char* email;
    char* phone;
    char* address;
} contact;

typedef int (*contactComparator)(contact*, contact*);

void swapStrings(char** one, char** two);

void swapContacts(contact** one, contact** two);

contact* createContact(char* name, char* surname, char* birthDate, char* email, char* phone, char* address );

void freeContact(contact* deleting);

int nameCompare(contact* one, contact* two);
int surnameCompare(contact* one, contact* two);
int birthDateCompare(contact* one, contact* two);
int emailCompare(contact* one, contact* two);
int phoneCompare(contact* one, contact* two);
int addressCompare(contact* one, contact* two);

char* ranStr(char*);
char* ranStrNum(char*);
char* concat(char *s1, char *s2);

#endif // HELPERS_H
