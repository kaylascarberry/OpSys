/*
Seminar 2: Team Twin Terminal
Lottery Scheduler
*/

/*Necessary Libraries*/
#include <stdio.h>      //used for printf
#include <stdlib.h>     //used for malloc
#include <time.h>       //used for rand()

/*Global Definitions*/
// Length of time the process runs, in this program, this is how much of the process's size 
// is reduced when its ticket is picked
#define UNITTIME 1  
int next_pid = 1;

typedef struct _Process {
    int pid;        //name of the process P[n]
    int tickets;    //number of tickets
    int size;       //work remaining, how long the process runs for
    struct _Process * next; // <structure name> * <variable name>
} Process;

typedef struct _Queue{
    Process* first;     //
    int totalTickets;   // total tickets in the queue
    int count;          // number of processes
} Queue;

/*Function Declaration and Prototyping All Before Main*/
static Queue* create_queue(void) {
    //make a result variable and allocate some memory to it:
    Queue* q = (Queue*)malloc(sizeof(Queue));
    if (!q) return NULL;
    q->first = NULL;    //use -> instead . because it is a pointer
    q->count = 0;
    q->totalTickets = 0;
    return q;
}

static Process* create_process(int tickets, int size) {
    //dont point * to the process because malloc needs to know how many bytes 
    // are needed for the structure
    Process* p = (Process*)malloc(sizeof(Process));
    if(!p) return NULL;
    p->pid = next_pid++;    //global variable
    p->tickets = tickets;
    p->size = size;
    p->next = NULL;
    return p;
}

//adds process to end of q
static int enqueue(Queue* q, Process* p) {
    //enqueue is declared as int so if -1 occurs, we know that it failed
    if (!q || !p) return -1;

    if (!q->first) {
        q->first = p;   //know that q = 0
    } else {
        Process* c = q->first;            //create a new local variable 
        while (c->next) c = c->next;      //c is current
        c->next = p;                      //
    }
    p->next = NULL;
    q->count++; //increment the count
    q->totalTickets += p->tickets;
    return 0;                             
}

//remove from queue and return a process
static Process* dequeue(Queue* q) {
    if(q == NULL) return -1;
    if (!q || !q->first) return NULL;   //there's at least 1 element in the q
    Process* p = q->first;              //create a local variable p to hold the result
    q->first = p->next;                 //make the first element the one next to the first
    p->next = NULL;                     //show that there is nothing else after in the q
    q->count--;                         //decrement the count
    q->totalTickets -= p->tickets;      //decrement the number of lottery tickets after used
    return p;                           //return the result
}

/* Remove target from queue and free it (updates count/totalTickets). */
//NEED HELP COMMENTING THIS
static void remove_and_free(Queue* q, Process* target) {
    if (!q || !target) return;
    for (Process** pp = &q->first; *pp; pp = &(*pp)->next){
        if (*pp == target){
            q->totalTickets -= target->tickets;
            q->count--;
            *pp = target->next;
            free(target);
            return;
        }
    }
}

//Print the Q with the process ID, # of tickets, and the size of the process
static void print_queue(const Queue* q) {
    printf("\nPID  Tickets  Size\n");
    printf("------------------\n");
    for (const Process* p = q->first; p; p = p->next)
        printf("P%-3d %-7d %-4d\n", p->pid, p->tickets, p->size);
    printf("Total tickets: %d | Processes: %d\n", q->totalTickets, q->count);
}

//NEED HELP COMMENTING THIS
static Process* pick_winner(const Queue* q, int* drawn) {
    if (!q || q->totalTickets <= 0) return NULL;
    int r = (rand() % q->totalTickets) + 1;   //
    if (drawn) *drawn = r;

    int acc = 0;
    for (Process* p = q->first; p; p = p->next) {
        acc += p->tickets;
        if (acc >= r) return p;
    }
    return NULL;
}

//Lottery Scheduler
static void lottery(Queue* rq, int round) {
    if (rq->count == 0 || rq->totalTickets <= 0) return;

    int ticket = -1;
    Process* w = pick_winner(rq, &ticket);
    if (!w) return;

    printf("\n-- Round %d --\n", round);
    printf("Drawn ticket: %d (of %d)\n", ticket, rq->totalTickets);
    printf("Winner: P%d (tickets=%d, size=%d)\n", w->pid, w->tickets, w->size);

    // Run winner for fixed work-per-turn
    if (w->size > UNITTIME
) w->size -= UNITTIME
; else w->size = 0;

    // Priority decay: winner loses one ticket
    if (w->tickets > 0) { w->tickets--; rq->totalTickets--; }

    // Remove if finished or out of tickets
    if (w->size == 0 || w->tickets == 0) {
        printf("Removing P%d (%s).\n", w->pid,
               (w->size == 0) ? "finished work" : "no tickets left");
        remove_and_free(rq, w);
    }
}

// Start of main function, this is where the CPU starts working
//main stack frame. This is where the return value is declared and reserved as int
int main(void) {    
    srand((unsigned)time(NULL)); //

    Queue q = {0};  //create new Q
    int n;
    printf("Enter Number of processes: ");
    if (scanf("%d", &n) != 1 || n <= 0) { fprintf(stderr, "Invalid.\n"); return 1; }

    for (int i = 1; i <= n; ++i) {
        int t, w;
        printf("Process %d - number of tickets: ", i);
        if (scanf("%d", &t) != 1 || t <= 0) return 1;
        printf("Process %d - work units:    ", i);
        if (scanf("%d", &w) != 1 || w <= 0) return 1;
        if (enqueue(&q, create_process(t, w)) != 0) {
            fprintf(stderr, "enqueue failed.\n");
            return 1;
        }
    }

    printf("\n=== Lottery start (UNITTIME     = %d) ===\n", UNITTIME);
    print_queue(&q);

    for (int round = 1; q.count > 0; ++round) {
        lottery(&q, round);
        print_queue(&q);
    }

    printf("\nAll processes completed.\n");
    return 0;
}
