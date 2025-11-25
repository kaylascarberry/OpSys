/*
scheduler is part of the kernal code that decides who and when the next process starts
how is the kernal keeping track of processes?
    uses the structure mapping
where has structure used before?
    data structures
    what did we build in data structures?
        linked lists, data queues

The nicer you are, the less you get

fifo: first in first out
    
sjf: shortest jump first

rr: round robin
    enters q in order, and allows the process to run for a determined time unit (time slice) then
    runs the next in order.
    one of the best priorities for response time
    timer interrupt (IRQ) every x us
    simplest scheduler to play with
    
turnaround time: T_turnaround = T_completion - T_arrival
    a metric, something that is used to measure something else.
    in this case, measure the performance of the scheduler

response time: T_response = T_firstrun - T_arrival
    why are you interested in response time?
        because when you want low latency

besides turnaround and response time, look at efficiency of the scheduler
    time_unit = 4 us (microseconds)
    time_switch = 2 us
    eff_sched = (utilized time)/(total time)
    its good when the os is executing the processes as intended

there is a content switch happening between every process change
    this is when the OS decides what is executed next
    there needs to be some time to make this decision
        decide: keep the same process or switch?
    this is very intensive: remember registers, move q's, copy values to new registers

MLFQ: multi level feedback q
    multiple q's, to allow multiple priorities to be brought into the process
    seminar guidelines (check the rules from the slides):
    the newer processes become the higher priorities, and deprioritizes long processes
    all the rules happen at once, they are not sequential
    when a process is promoted to a higher priority, it is treated as a new process in that moment


Lottery: (simple)
    -allocate a number of tickets for each process. the higher the priority, the more tickets.
    -use the rand() function to randomly pick a ticket
    -temporary transfer allows a process to temporarily give its tickets to another process
    -sometimes a process will run out of tickets before finishing the process
        -re-issue tickets when everyone is out of tickets, like a reset rule

Stride Scheduling:
    -stride is a penalty, it reduces the chances for a process to run again
    -pass is the entire penalty based on user stride
    -stride does get updated after a process has its ticket used
        -but for this project, don't recalculate. keep userstride a constant.


CFS: Completely fair
    -closer to what a current linux system will run
    -vruntime replaces penalty inside of each process
        -how long the process has ran so far
        -"virtual run time"
    -latency: what should be the maximum time it takes for a process to show up again to run
        -if you want to achieve that latency, and you have 10 processes, you want to make sure that they all have
        that minimum latency. 
        -Time slice = max(sched_latency/# of processes, min_granularity)
            -time slice is an approximation because of the dead time used in process switching
            -the smaller the time slice, the less efficient your scheduler is

    

How to wake up a process from sleep:
    What does the OS need to do?
        update some registers in the CPU
        we care about 1 register in particular
*/
#include <stdio.h>
int main(void) {
int a, b, c;
//FIFO turnaround time
a = 4;
b = 10;
c = 11;

float avg1 = (a + b + c) / 3;

//sjf turnaround time
a = 5;
b = 11;
c = 1;

float avg2 = (a + b + c) / 3;

//RR response time
a = 0;
b = 1;
c = 2;

float avg3 = (a + b + c) / 3;


printf("Turnaround average fifo: %.2f \n", avg1);
printf("Turnaround average sjf: %.2f\n", avg2);
printf("Turnaround average sjf: %.2f\n", avg3);
return 0;
}