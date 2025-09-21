#ifndef TWIN_H
#define TWIN_H

/*HEADER FILES*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <string.h>

/*COLORS*/
#define Y   "\033[1;33m"    //yellow
#define G   "\033[1;32m"    //green
#define C   "\033[1;36m"    //cyan
#define RED "\033[1;31m"    //red
#define RST "\033[0m"       //reset

/*WRAPPERS*/
char *Getcwd(char *buf, size_t size)
{
    if (NULL == getcwd(buf, size))
    {
        perror(RED"getcwd FAILED"RST);
    return buf;
    }
}

void printbanner(void)
{
    printf(C"Initializing Twin Terminal - Seminar 1 - Maria Aponte & Kayla Scarberry\n"RST);
    printf(Y" _____        _         _____                   _             _\n" 
            "|_   _|      (_)       |_   _|                 (_)           | |\n"
            "  | |_      ___ _ __     | | ___ _ __ _ __ ___  _ _ __   __ _| |\n"
            "  | \\ \\ /\\ / / | '_ \\    | |/ _ \\ '__| '_ ` _ \\| | '_ \\ / _` | |\n"
            "  | |\\ V  V /| | | | |   | |  __/ |  | | | | | | | | | | (_| | |\n"
            "  \\_/ \\_/\\_/ |_|_| |_|   \\_/\\___|_|  |_| |_| |_|_|_| |_|\\__,_|_|\n\n"RST);

}


#endif