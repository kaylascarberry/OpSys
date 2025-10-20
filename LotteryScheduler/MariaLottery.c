#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// ------- Simple, functional lottery scheduler -------
// - Linked list of processes
// - Random ticket draw printed each round
// - Winner runs for WORK_PER_TURN and loses 1 ticket

#define WORK_PER_TURN 1  // fixed "quantum" per winner

typedef struct Process {
    int pid;
    int tickets;
    int work;
    struct Process* next;
} Process;

typedef struct {
    Process* head;
    int totalTickets;
    int count;
} RunQueue;

// ---------- Minimal helpers ----------
static Process* make_proc(int pid, int tickets, int work) {
    Process* p = (Process*)malloc(sizeof(Process));
    p->pid = pid; p->tickets = tickets; p->work = work; p->next = NULL;
    return p;
}

static void push_back(RunQueue* rq, Process* p) {
    if (!rq->head) rq->head = p;
    else { Process* c = rq->head; while (c->next) c = c->next; c->next = p; }
    rq->totalTickets += p->tickets; rq->count++;
}

// Remove node using pointer-to-pointer (no separate prev variable needed)
static void remove_node(RunQueue* rq, Process* target) {
    for (Process** pp = &rq->head; *pp; pp = &(*pp)->next) {
        if (*pp == target) {
            rq->totalTickets -= target->tickets;
            rq->count--;
            *pp = target->next;
            free(target);
            return;
        }
    }
}

static void print_queue(const RunQueue* rq) {
    printf("\nPID  Tickets  Work\n");
    printf("------------------\n");
    for (const Process* p = rq->head; p; p = p->next)
        printf("P%-3d %-7d %-4d\n", p->pid, p->tickets, p->work);
    printf("Total tickets: %d | Procs: %d\n", rq->totalTickets, rq->count);
}

// ---------- Core lottery ----------
static Process* pick_winner(const RunQueue* rq, int* drawn) {
    if (rq->totalTickets <= 0) return NULL;
    int r = (rand() % rq->totalTickets) + 1;   // [1..totalTickets]
    if (drawn) *drawn = r;

    int acc = 0;
    for (Process* p = rq->head; p; p = p->next) {
        acc += p->tickets;
        if (acc >= r) return p;
    }
    return NULL; // unreachable if totals are consistent
}

static void run_one_round(RunQueue* rq, int round) {
    if (rq->count == 0 || rq->totalTickets <= 0) return;

    int ticket = -1;
    Process* w = pick_winner(rq, &ticket);
    if (!w) return;

    printf("\n-- Round %d --\n", round);
    printf("🎟  Drawn ticket: %d (of %d)\n", ticket, rq->totalTickets);
    printf("🎯 Winner: P%d (tickets=%d, work=%d)\n", w->pid, w->tickets, w->work);

    // Run winner for fixed work-per-turn
    if (w->work > WORK_PER_TURN) w->work -= WORK_PER_TURN; else w->work = 0;

    // Priority decay: winner loses one ticket
    if (w->tickets > 0) { w->tickets--; rq->totalTickets--; }

    // Remove if finished or out of tickets
    if (w->work == 0 || w->tickets == 0) {
        printf("✅ Removing P%d (%s).\n", w->pid,
               (w->work == 0) ? "finished work" : "no tickets left");
        remove_node(rq, w);
    }
}

// ---------- Main ----------
int main(void) {
    srand((unsigned)time(NULL));

    RunQueue rq = {0};
    int n;
    printf("Enter Number of processes: ");
    if (scanf("%d", &n) != 1 || n <= 0) { fprintf(stderr, "Invalid.\n"); return 1; }

    for (int i = 1; i <= n; ++i) {
        int t, w;
        printf("Process %d - number of tickets: ", i);  if (scanf("%d", &t) != 1 || t <= 0) return 1;
        printf("Process %d - work units:    ", i);  if (scanf("%d", &w) != 1 || w <= 0) return 1;
        push_back(&rq, make_proc(i, t, w));
    }

    printf("\n=== Lottery start (WORK_PER_TURN = %d) ===\n", WORK_PER_TURN);
    print_queue(&rq);

    for (int round = 1; rq.count > 0; ++round) {
        run_one_round(&rq, round);
        print_queue(&rq);
    }

    printf("\n🎉 All processes completed.\n");
    return 0;
}
