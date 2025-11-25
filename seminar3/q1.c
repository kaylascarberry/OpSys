#include <stdio.h>

typedef struct _Node{
    int count;
    char value;
    struct _Node *next;
}Node;

typedef struct _List{
    Node *first;
}List;

//create the node for each letter
Node* createNode(char c){
    Node* result = (Node*)malloc(sizeof(Node));
    result->next=NULL;
    result->value=c;
    result->count=1;    //if you find a new character, count it
}

//create a list
List* createList(){
    List* result = malloc(sizeof(List));
    result->first = NULL;
}
