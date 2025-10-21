/*
Seminar 2: Team Twin Terminal
Lottery Scheduler
*/

/*Necessary Libraries*/
#include <stdio.h>      //used for printf and stderr
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
    int init_tickets;   //originally declared # of tickets so they can be re-issued
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
    q->count = 0;       //initialize count to 0
    q->totalTickets = 0;//initialize totalTickets to 0
    return q;
}

static Process* create_process(int tickets, int size) {
    //dont point * to the process because malloc needs to know how many bytes 
    // are needed for the structure
    Process* p = (Process*)malloc(sizeof(Process));
    if(!p) return NULL;
    p->pid = next_pid++;    //global variable
    p->tickets = tickets;
    p->init_tickets = tickets;  //remember the initial user declared amount of tickets
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
        c->next = p;                      //assign process to c->next
    }
    p->next = NULL;
    q->count++; //increment the count
    q->totalTickets += p->tickets;  //add totalTickets to p->tickets  and assign the sum to q->totalTickets
    return 0;                             
}

//removes the first process from queue and return a process
static Process* dequeue(Queue* q) {
    //if(q == NULL) return -1;
    if (!q || !q->first) return NULL;   //there's at least 1 element in the q
    Process* p = q->first;              //create a local variable p to hold the result
    q->first = p->next;                 //make the first element the one next to the first
    p->next = NULL;                     //show that there is nothing else after in the q
    q->count--;                         //decrement the count
    q->totalTickets -= p->tickets;      //decrement the number of lottery tickets after used
    return p;                           //return the result
}

//Remove any finished target process from queue and free it (updates count/totalTickets). 
static void remove_and_free(Queue* q, Process* target) {
    if (!q || !target) return;  //return nothing if there is no Q or process
    //point to a pointer (pointing to the current node, pp)
    //when target = first node, pp=&q->first
    //when target is in between the first and last node, pp = &(prev->next)
    for (Process** pp = &q->first; *pp; pp = &(*pp)->next){
        if (*pp == target){ //when target is found:
            q->totalTickets -= target->tickets; //keep the ticket total
            q->count--;         //decrement the process count
            *pp = target->next; //unlink the target
            free(target);       //free the memory
            return;
        }
    }
}

//Print the Q with the process ID, # of tickets, and the size of the process
static void print_queue(const Queue* q) {
    printf("\nPID  Tickets  Size\n");
    printf("------------------\n");
    for (const Process* p = q->first; p; p = p->next)
        printf("P%-3d %-7d %-4d\n", p->pid, p->tickets, p->size); //%-nd where n is the number of character spaces
    printf("Total tickets: %d | Processes: %d\n", q->totalTickets, q->count);
}

//pick winning lottery ticket
static Process* pick_winner(const Queue* q, int* drawn) {
    //if q is NULL or there are no tickets assigned, there is nothing
    //to pick from, so NULL winners will be chosen
    if (!q || q->totalTickets <= 0) return NULL;
    //acquire a random integer, limit the range between the
    //totalTickets.
    int r = (rand() % q->totalTickets) + 1;
    //if drawn is not NULL, store r as the drawn ticket
    if (drawn) *drawn = r;

    int acc = 0;    //initialize an accumulator
    //begin at the first process in the Q, and loop until p = NULL
    for (Process* p = q->first; p; p = p->next) {
        acc += p->tickets;  //every process contributes its tickets to the total
        if (acc >= r) return p; //each process has its range of tickets that it is assigned.
    }
    return NULL;
}

//function to re-issue tickets when every process current ticket
//count is depleted
static void reissue_all_tickets(Queue* q) {
    int total = 0;  //initialize total to 0
    for (Process* p = q->first; p; p = p->next) {
        p->tickets = p->init_tickets;  // restore declared amount
        total += p->tickets;
    }
    q->totalTickets = total;    //accumulate process tickets to totalTickets
}

static void redistribute_tickets(Queue* q, Process* p_removed) {
    if (!q || q->count == 0 || p_removed->tickets <= 0) return;

    // Find the process with the lowest priority (lowest # of tickets)
    Process* lowest = q->first;
    for (Process* p = q->first; p; p = p->next) {
        if (p->tickets < lowest->tickets) {
            lowest = p;
        }
    }

    // Give all the tickets to the lowest priority process
    lowest->tickets += p_removed->tickets;
    q->totalTickets += p_removed->tickets;
    remove_and_free(q, p_removed); //remove the original process that has ran out of work

    if (q->count == 0) {
        printf("All processes completed after redistribution. Clearing remaining %d tickets.\n", q->totalTickets);
        q->totalTickets = 0;
        return;
    }
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

    /* Run winner for one unit */
    if (w->size > UNITTIME) w->size -= UNITTIME;
    else                    w->size = 0;

    /* Ticket decay for the winner */
    if (w->tickets > 0) { w->tickets--; rq->totalTickets--; }

    /* 1) If finished, remove it. */
    if (w->size == 0 && w->tickets ==0) {
        printf("Removing P%d (finished work).\n", w->pid);
        remove_and_free(rq, w);
        return;
    }

    /* 2) If out of tickets but still has work, start a new epoch (reissue). */
    if (w->tickets == 0 && w->size >0) {
        printf("P%d used all tickets but still has work; Reissuing initial %d tickets to all processes.\n", w->pid, w->init_tickets);
        reissue_all_tickets(rq);
        /* Do NOT remove the process. It stays in the queue with restored tickets. */
        return;
    }

    if (w->tickets > 0 && w->size == 0) {
        printf("Removing P%d (finished work). Redistributing remaining %d tickets to lowest priority pid. \n", w->pid, w->tickets);
        redistribute_tickets(rq, w);
        return;
    }
    /* 3) Otherwise, nothing else to do this round. */
}


// Start of main function, this is where the CPU starts working
//main stack frame. This is where the return value is declared and reserved as int
int main(void) {
    srand((unsigned)time(NULL));

    Queue q = {0};
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

    printf("\n=== Lottery start (UNITTIME = %d) ===\n", UNITTIME);
    print_queue(&q);

    for (int round = 1; q.count > 0; ++round) {
        lottery(&q, round);
        print_queue(&q);
    }

    printf("\nAll processes completed.\n");
    return 0;
}
