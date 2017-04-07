#include<stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tree.h"
#include "helpers.h"

treeBook* createTreeBook(contactComparator comparator){ // tworzy struct treeBook z domyslnymi wartosciami (sortedBy domyslnie na NAME)
    treeBook* new = malloc(sizeof(treeBook));
    new->root = NULL;
    new->comparator = comparator;
    return new;
}

static void deleteLeaf(tree* node){
    freeContact(node->contact);
    if(node->parent != NULL){
        if(node->parent->left == node) node->parent->left = NULL;
        else if(node->parent->right == node) node->parent->right = NULL;
    }
    free(node);
}

static void deleteTreeRec(tree* node){
    if(node == NULL) return;
    deleteTreeRec(node->left);
    deleteTreeRec(node->right);
    deleteLeaf(node);
}

void deleteTree(treeBook* book){ // usuwa zawartosc ksiazki
    deleteTreeRec(book->root);
    book->root = NULL;
}

static tree* createNode(contact* newContact){
    tree* new = malloc(sizeof(tree));
    new->contact = newContact;
    new->left = NULL;
    new->right = NULL;
    new->parent = NULL;
    return new;
}

static tree* treeInsert(tree* node, contactComparator comparator, contact* newContact){
    if(node == NULL){
        return createNode(newContact);
    }
    if((*comparator)(newContact, node->contact) > 0){
        node->right = treeInsert(node->right, comparator, newContact);
        node->right->parent = node;
    }else{
        node->left = treeInsert(node->left, comparator, newContact);
        node->left->parent = node;
    }

    return node;
}

void addNewToTree(treeBook* book, char* name, char*surname, char* birthDate, char* email, char* phone, char* address){
    contact* newContact = createContact(name, surname, birthDate, email, phone, address);
    book->root = treeInsert(book->root, book->comparator, newContact);
}

static tree* linearBSTSearch(tree* node, contactComparator comparator, contact* val){
    if(node == NULL) return NULL;
    if((*comparator)(node->contact, val) == 0) return node;

    tree* leftResult = linearBSTSearch(node->left, comparator, val);
    tree* rightResult = linearBSTSearch(node->right, comparator, val);

    if(leftResult == NULL && rightResult == NULL) return NULL;
    else if(leftResult != NULL) return leftResult;
    else return rightResult;
}

tree* findInTree(treeBook* book, contactComparator comparator, char* val){
    contact* tmp = createContact(val, val, val, val, val, val);
    tree* result =  linearBSTSearch(book->root, comparator, tmp); // tu moznaby sprawdzac, czy comparatory sie zgadzaja, i w razie mozliwosci uzywac szybszego wyszukiwania
    free(tmp); // bron Boze freeContact, bo sie sypie
    return result;
}

static tree* findMinNode(tree* node){ //assuming its called on non-empty tree
    if(node->left != NULL) return findMinNode(node->left);
    return node;
}

void deleteFromTree(treeBook* book, contactComparator comparator, char* val){ // najpierw znajdz element, potem odpale statyczne usuwanie(bedzie wymagalo nastepnika)
    tree* deleting = findInTree(book, comparator, val);
    if(deleting == NULL) return;

    if(deleting->left == NULL && deleting->right == NULL){
        if(deleting->parent == NULL){
            book->root = NULL;
        }
        return deleteLeaf(deleting);
    }

    if(deleting->left != NULL && deleting->right != NULL){
        tree* nextNode = findMinNode(deleting->right);
        swapContacts(&nextNode->contact, &deleting->contact);
        if(nextNode->right != NULL){ // nastepnik musi miec rodzica, wynika z warunkow wywolania

            if(nextNode->parent->left == nextNode){ // nastepnik moze byc prawym (poczatek) lub lewym dzieckiem rodzica
                nextNode->parent->left = nextNode->right;
                nextNode->right->parent = nextNode->parent;
            }else{
                nextNode->parent->right = nextNode->right;
                nextNode->right->parent = nextNode->parent;
            }
            return deleteLeaf(nextNode);

        }else{
            return deleteLeaf(nextNode);
        }

    }
    if(deleting->left != NULL){
        deleting->left->parent = deleting->parent;
        if(deleting->parent == NULL){
            book->root = deleting->left;
        }
        else if(deleting->parent->left == deleting){
            deleting->parent->left = deleting->left;
        }else if(deleting->parent->right == deleting){
            deleting->parent->right = deleting->left;
        }

        return deleteLeaf(deleting);
    }else{
        deleting->right->parent = deleting->parent;
        if(deleting->parent == NULL){
            book->root = deleting->right;
        }
        else if(deleting->parent->left == deleting){
            deleting->parent->left = deleting->right;
        }else if(deleting->parent->right == deleting){
            deleting->parent->right = deleting->right;
        }

        return deleteLeaf(deleting);
    }
}

static void showTreeRec(tree* root){
    if(root == NULL) return;

    showTreeRec(root->left);
    printf("---------------------------------------------------------------------------- \n");
    printf("Name: %s, Surname: %s, Birth Date: %s, \nEmail: %s, Phone: %s, Address: %s", root->contact->name, root->contact->surname, root->contact->birthDate,
		root->contact->email, root->contact->phone, root->contact->address);
    printf("---------------------------------------------------------------------------- \n");
    showTreeRec(root->right);
}

void showTree(treeBook* book){ // zwykly show
    printf("Showing the content of a tree: \n");
    showTreeRec(book->root);
}

static void rebuildTreeRec(tree* node, contactComparator comparator, tree** new){
    if(node == NULL) return;
    rebuildTreeRec(node->left, comparator, new);
    rebuildTreeRec(node->right, comparator, new);
    *new = treeInsert(*new, comparator, node->contact);
    free(node); // wystarczy ze usune node, jego contact ma zostac
}

static tree* rebuildTree(tree* node, contactComparator comparator){
    tree* new = NULL;
    rebuildTreeRec(node, comparator, &new);
    return new;
}
void sortTree(treeBook* book, contactComparator otherComparator){
    book->root = rebuildTree(book->root, otherComparator);
    book->comparator = otherComparator;
}




