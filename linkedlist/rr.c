/*
Questions:
1) does a structure have to have a _ at the start of its name?
2) what is an assignment operator?
*/
/*
Notes:
-structures are the processes
-typedef for alias
-'enter' is the normal word for adding something to a q
-in linux, functions dont normally return void
-can have multiple processes with the same name
-the starting entrypoint for a c program is the main function
-the code runs in the specific kernel
    -memory works differently in the kernel
-this program is a simulation of a scheduler
-processes always use memory segments
-we use 3 different memory segments in this program
    -stack (processes managed by the CPU)
    -data (global variables, static data, constants)
    -heap (managed by the user/programmer. it's dynamically managed)
    -text (pointer to a function)
-when a function starts running, the cpu creates a new stack frame
-the only time we do something with heap is when we use malloc or calloc functions
-when a function ends, there is a special procedure that ends the stack frame (destroyed/erased) and the code
    will continue on with the next line.

How to wake up a process from sleep:
    What does the OS need to do?
        update some registers in the CPU
        we care about 1 register in particular
            The instruction pointer that is at the next line of code

We never know what is in the heap. We can only see the Data and the stack.

-free is the opposite of malloc

-there is 1 more memory segment
    -.text
        -copy of your compile file 

*/

#include <stdlib.h> //to use malloc
#include <stdio.h>  //to use printf
#define UNITTIME 2
int next_pid = 1;   //global variable

typedef struct _Process{
    int pid;
    char name;
    int size;   //choose how long the process runs for

    struct _Process * next; //name of the structure * name of the variable
} Process;

struct _Queue{
    Process* first;
    int count;

};
typedef struct _Queue Queue;    //another method of aliasing

//create q function
Queue* create_queue(){
    Queue* result = malloc(sizeof(Queue));  //made a result local variable and assigned some memory to it
    if(result == NULL) 
        return NULL;
    result->count = 0;   //use arrow instead of . because its a pointer
    result->first = NULL;
    //(*result).count=0;  //use * to use the . syntax
    //can do this whole function as 'return calloc(sizeof(Queue))'
    return result;
}

//create process function
Process* create_process(char name, int size){
    Process* result;
    result = malloc(sizeof(Process));   //dont point * to the process because malloc needs to know how many bytes are needed for the structure
    if(result == NULL) 
        return NULL;
    result->pid = next_pid++; //global variable
    result->name = name;
    result->size = size;
    result->next = NULL;
    return result;
}


//adds process to end of q
int enqueue(Queue* q, Process* p){ //"enter into q" declared as int so if -1 occurs, we know it failed
    if(q == NULL) return -1;
    if(p == NULL) return 0;

    if(q->first == NULL)
    {
        q->first = p;   //know that q = 0
        q->count = 1;   //now the next position is at 1
        return 0;
    }
    Process* current = q->first;    //create new local variable called current
    while (current-> next != NULL)
    {
        current = current->next;
    }
    current->next = p;
    ++q->count; //incremement the count
    return 0;
}  

Process* dequeue(Queue* q){ //remove from q and return a process
    if(q == NULL) return -1;
    if(q->first == NULL) return NULL;   //theres at least 1 element

    --q->count; //decremement the count
    Process* result = q->first; //create a local variable called results
    q->first = q->first->next;  //makes the first element the one next to the first
    result->next = NULL;    //show there is nothing else after in the q
    return result;
}   

int rr(Queue* q){ //roundrobin scheduler
    if(q == NULL) return -1;
    Process* p;
    while(q->count > 0){    //while the first q is not empty
        p = dequeue(q);
        ////
        //printf("process is running\n");
        ////
        //need a condition to know when the process finishes
        p->size -= UNITTIME;
        printf(" %c ", p->name);
        if(p->size >0)
            enqueue(q,p);
        else
            free(p);    //very important, it releases this space of memory in the heap to be used by a new process
            //p == NULL;
        }
    return 0;
} 

//start of main function
//this where the cpu starts working.
int main(){ //main stack frame. this is where the return value is declared and reserved (int)
    Queue *rrq = create_queue();    //creates new rr queue //first local variable (a pointer) is defined and assigned to the create_q stack frame
    Process* pA = create_process('a', 4);
    Process* pB = create_process('b', 6);
    Process* pC = create_process('c', 1);

    /* could have done:
    enqueue(rrq, create_process('a',4));
    ...
    */
    enqueue(rrq, pA);
    enqueue(rrq, pB);
    enqueue(rrq, pC);

    printf("\n\nSTARTING THE SCHEDULER\n\n");
    rr(rrq);
    printf("\n\nSCHEDULER FINISHED\n\n");
    free(rrq);  //this is good practice, even tho its useless in this scenario

    return 0;
}//this last curly bracket gives this return value to the OS and then this main stack frame is destroyed
