#include "helpers.h"
#include "list.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

node_t* createListBook(){
	node_t* guardian = malloc(sizeof(node_t));
	guardian->contact = createContact(NULL, NULL, NULL, NULL, NULL, NULL);
	guardian->next = NULL;
	guardian->prev = NULL;
	return guardian;
}

static void deleteSingleFromList(node_t* deleting){
    freeContact(deleting->contact);
    if(deleting->prev != NULL){
        deleting->prev->next = deleting->next;
    }
    if(deleting->next != NULL){
        deleting->next->prev = deleting->prev;
    }
    free(deleting);
}

void deleteList(node_t* head){
    node_t* tmp = head->next;

    deleteSingleFromList(head);

    if(tmp == NULL){
        return;
    } return deleteList(tmp);
}

void addNewToList(node_t* head, char* name, char*surname, char* birthDate, char* email, char* phone, char* address){
	node_t* new = malloc(sizeof(node_t));
	new->contact = createContact(name, surname, birthDate, email, phone, address);
	new->prev = head;
	new->next = head->next;

	head->next = new;
	if(new->next != NULL){
        new->next->prev = new;
	}
}

node_t* findInList(node_t* head, contactComparator comparator, char* val){
    node_t* i;

    contact* tmp = malloc(sizeof(contact)); // uzywam tego dla uproszczenia implementacji dowolnego szukania - koszt jest staly i nieduzy
    tmp->address = val; tmp->birthDate = val; tmp->email = val;
    tmp->name = val; tmp->phone = val; tmp->surname = val;

    for(i = head; i->next != NULL; i = i->next){
        if((*comparator)(i->next->contact, tmp) == 0){
            free(tmp);
            return i->next;
        }
    }
    free(tmp);
    return NULL;
}

void deleteFromList(node_t* head, contactComparator comparator, char* val){
    node_t* deleting = findInList(head, comparator, val);
    if(deleting == NULL) return;
    return deleteSingleFromList(deleting);
}

void showList(node_t* head){
	while(head->next != NULL){
		node_t* p = head->next;
		printf("---------------------------------------------------------------------------- \n");
		printf("Name: %s, Surname: %s, Birth Date: %s, \nEmail: %s, Phone: %s, Address: %s", p->contact->name, p->contact->surname, p->contact->birthDate,
		p->contact->email, p->contact->phone, p->contact->address);
		printf("\n---------------------------------------------------------------------------- \n");

		head = head->next;
	}
}

static node_t* findLastNode(node_t* head){
    while(head->next != NULL){
        head = head->next;
    } return head;
}

static node_t* nodePartition(contactComparator comparator, node_t* first, node_t* last){
    node_t* q = first->next; // zakladam, ze wywolano dla co najmniej 2 elementow
    node_t* i = first->next;

    while(1){
        if((*comparator)(i->contact, first->contact) <= 0 ){
            swapContacts(&q->contact, &i->contact);
            q = q->next;
        }

        if(i == last) break;
        i = i->next;
    }

    if(q != NULL){
        swapContacts(&first->contact, &q->prev->contact);
        return q->prev;
    }else{
        swapContacts(&first->contact, &last->contact);
        return last;
    }
}

static void quickerSort(contactComparator comparator, node_t* first, node_t* last){
    node_t* fixed = nodePartition(comparator, first, last);

    if((fixed != last) && (fixed->next != last)){ // at least two nodes to sort
        quickerSort(comparator, fixed->next, last);
    }

    if((fixed != first) && (fixed->prev != first)){
        quickerSort(comparator, first, fixed->prev);
    }
}

void sortList(node_t* head, contactComparator comparator){
    node_t* last = findLastNode(head);
    if((head == last) || (head->next == last)) return; // at least two nodes to sort
    quickerSort(comparator, head->next, last);
}
