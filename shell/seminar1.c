//Twin Shell
#include "twin.h"
#include "sniff.h"

/*
WHAT IS A TOKENIZER?
splits the contents of the command line into unique arguments separated by delimiters
delim is the space between the strings in the cmd.
cmd: arg[0] arg[1]
argument 0 is always the command, argument 1 is the instruction
*/

// Tokenizer
static int split_args(char *line, char **argv, int max_args)
{
    int n = 0;
    for (char *tok = strtok(line, " \t\r\n"); tok && n < max_args - 1; tok = strtok(NULL, " \t\r\n")) {
        argv[n++] = tok;
    }
    argv[n] = NULL;
    return n;
}

//User prompt & read command line
char    *readLine(void)
{
    char    *buf;
    size_t  bufSize = 0;
    char    cwd[BUFSIZ];    //size of buffer used by setbuf
    buf = NULL;

    //prompt the user to input a command
    Getcwd(cwd,  sizeof(cwd));
    printf(C"\U0001F46F %s \U0001F46F "RST, cwd); 
    printf("twin>> ");
    fflush(stdout);

    //if error
    if (getline(&buf, &bufSize, stdin) == -1)   //stdin = stream
    {
        buf = NULL;         //in case of mistake, put a null inside of the buffer
        if (feof(stdin))    //tests for end of file indicator 
            printf(RED"[End of File]"RST"\n");
        else
            printf(RED"[getline Failed]"RST"\n");
    }
    return buf;
}

//Start of main function
int main(int argc, char** argv) //argc = arg. counter, argv = arg. vector
{
    printbanner();
    char    *line;
    //get command line

    while((line = readLine()))
    {        
        printf(">> %s\n", line);

        //tokenize input
        char *args[128];
        int n = split_args(line, args, 128);
        if (n == 0) { free(line); continue; }

        //Extra commands
        //help
        if (strcmp(args[0], "help") == 0) 
        {
            printf(G    "Twin Shell 'help'" RST "\n"
                        "  - Custom Commands:\n"
                        "      help   : Show help menu\n"
                        "      launch : Launchpad\n"
                        "      exit   : Quit the shell\n"
                        "      close  : Alias for exit\n"
                        "      sniff <iface> [count] [bpf] : capture packets until ctrl+c is pressed\n");
            free(line);
            continue;
        }
        //exit
        if (strcmp(args[0], "exit") == 0 || strcmp(args[0], "close") == 0) 
        {
            free(line);
            break;
        }
        //sniff for packets
        if (strcmp(args[0], "sniff") == 0) 
        {
            (void)cmd_sniff(n, args);   // returns when done
            free(line);
            continue;
        }
        //launch
        if (strcmp(args[0], "launch") == 0) 
        {
            const char *url = "https://www.youtube.com/watch?v=dQw4w9WgXcQ";
            printf(G "LAUNCHPAD: " RST "\n");
            printf("\033]8;;%s\a%s\033]8;;\a\n", url, "click here to see more information on Twin Terminal");

            free(line);
            continue;
        }

        pid_t pid = fork();
        if (pid == 0) 
        {
            execvp(args[0], args);            // returns only on error
            perror(RED "OOPS>>" RST);
            _exit(127);
        } 
        else if (pid < 0) 
        {
            perror(RED "fork" RST);
        } 
        else 
        {
            int status = 0;
            if (waitpid(pid, &status, 0) == -1)
                perror(RED "waitpid" RST);
        }
        free(line);
    }
    
    return 0;
}
