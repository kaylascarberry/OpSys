//Twin Shell
/*COMPILE WITH:
gcc -D_DEFAULT_SOURCE -D_POSIX_C_SOURCE=200809L -Wall -Wextra -std=gnu11 \
  seminar1.c -lreadline -lhistory -ltinfo -lpcap -o twinterminal
*/
#include "twin.h"
#include "sniff.h"
#include "morsecode.h"

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

    #ifdef TWIN_HAVE_READLINE
    twin_history_init();
#endif

    for (;;) {
        char *line = NULL;

#ifdef TWIN_HAVE_READLINE
        char prompt[256];
        twin_build_prompt(prompt, sizeof(prompt));
        line = twin_readline(prompt);       // malloc'd; NULL on EOF
#else
        line = readLine();                   // your getline() fallback
#endif
        if (!line) break;

        printf(">> %s\n", line);

        /* If the command line contains a pipeline, delegate to /bin/sh */
        if (strchr(line, '|')) {
            pid_t pid = fork();
            if (pid == 0) {
                execl("/bin/sh", "sh", "-c", line, (char *)NULL);
                perror(RED "exec sh -c" RST);
                _exit(127);
            } else if (pid < 0) {
                perror(RED "fork" RST);
            } else {
                int st = 0;
                if (waitpid(pid, &st, 0) == -1)
                    perror(RED "waitpid" RST);
            }
        free(line);
        continue;
        }

        // tokenize
        char *args[128];
        int n = split_args(line, args, 128);
        if (n == 0) { free(line); continue; }

        // built-ins
        if (strcmp(args[0], "help") == 0) {
            printf(G "Twin Shell 'help'" RST "\n"
                   "  - Custom Commands:\n"
                   "      help   : Show help menu\n"
                   "      launch : Launchpad\n"
                   "      exit   : Quit the shell\n"
                   "      close  : Alias for exit\n"
                   "      sniff <iface> [count] [bpf] : Capture packets\n"
                   "      morse  : Translate to Morse Code\n"
                   "      morse -d : Decode from Morse Code\n");
            free(line);
            continue;
        }

        if (strcmp(args[0], "exit") == 0 || strcmp(args[0], "close") == 0) {
            free(line);
            break;
        }

        if (strcmp(args[0], "sniff") == 0) {
            (void)cmd_sniff(n, args);
            free(line);
            continue;
        }

        if (strcmp(args[0], "launch") == 0) {
            const char *url = "https://www.youtube.com/watch?v=dQw4w9WgXcQ";
            printf(G "LAUNCHPAD:" RST " \033]8;;%s\a%s\033]8;;\a\n",
                   url, "click here to see more information on Twin Terminal");
            free(line);
            continue;
        }

        if (strcmp(args[0], "morse") == 0) {
            if (n == 1) {                      // no args, do nothing
                free(line);
                continue;
            }
        int decode = 0;
        int start = 1;
            if (strcmp(args[1], "-d") == 0 || strcmp(args[1], "--decode") == 0) {
                decode = 1;
                start = 2;
                if (n <= start) {              // "morse -d" without tokens → do nothing
                    free(line);
                    continue;
                }
            }
            if (decode)  morse_decode_tokens(n, args, start);
            else         morse_encode_args(n, args, start);

            free(line);
            continue;
        }
        // external command
        pid_t pid = fork();
        if (pid == 0) {
            execvp(args[0], args);
            perror(RED "OOPS>>" RST);
            _exit(127);
        } else if (pid < 0) {
            perror(RED "fork" RST);
        } else {
            int status = 0;
            if (waitpid(pid, &status, 0) == -1)
                perror(RED "waitpid" RST);
        }

        free(line);
    }

#ifdef TWIN_HAVE_READLINE
    twin_history_save();
#endif

    return EXIT_SUCCESS;
}