/*
-point to a memory address with &
-virtual address space vs physical
    virtual is what is seen by the process, physical is the hardware allocation
-mmu: memory management unit
    -helps the OS, by transcribing the virtual memory to the physical memory
    -updates the instruction pointer too
    -each time an instruction says to access a point in memory, it automatically updates to a physical address
    -
-keep track of what memory is user accessible or not
-if virtual address > size, there is a memory exception :(
-split memory:
    -memory segments
physical address = base[index] + offset
if size [index] < offset, throw an exception

*/
