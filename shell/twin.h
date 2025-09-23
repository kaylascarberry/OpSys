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
#include <limits.h>

#if defined(__has_include)
#  if __has_include(<readline/readline.h>) && __has_include(<readline/history.h>)
#    define TWIN_HAVE_READLINE 1
#  endif
#endif

#ifdef TWIN_HAVE_READLINE
  #include <readline/readline.h>
  #include <readline/history.h>
#endif

/*COLORS*/
#define Y   "\033[1;33m"    //yellow
#define G   "\033[1;32m"    //green
#define C   "\033[1;36m"    //cyan
#define RED "\033[1;31m"    //red
#define RST "\033[0m"       //reset

/*WRAPPERS*/
//get current working directory
char *Getcwd(char *buf, size_t size)
{
    if (NULL == getcwd(buf, size))
    {
        perror(RED"getcwd FAILED"RST);
    return buf;
    }
}

//prompt user input
    static inline void twin_build_prompt(char *out, size_t outsz) 
    {
    char cwd[BUFSIZ];
    if (!Getcwd(cwd, sizeof(cwd))) { cwd[0] = '?'; cwd[1] = '\0'; }
    /* 👯 = U+1F46F encoded as UTF-8 bytes to avoid wide-char fuss */
    snprintf(out, outsz, C "\xF0\x9F\x91\xAF %s \xF0\x9F\x91\xAF " RST "twin>> ", cwd);
    }

//ASCII banner for terminal
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

#ifdef TWIN_HAVE_READLINE
static inline const char *history_path(void) {
    static char histpath[PATH_MAX];
    if (histpath[0]) return histpath;
    const char *home = getenv("HOME");
    if (home) {
        snprintf(histpath, sizeof(histpath), "%s/.twinterminal_history", home);
    } else {
        snprintf(histpath, sizeof(histpath), ".twinterminal_history");
    }
    return histpath;
}

static inline void twin_history_init(void) {
    using_history();
    stifle_history(1000);               //keep last 1000 commands
    (void)read_history(history_path()); //ignore if missing
}

static inline void twin_history_save(void) {
    (void)write_history(history_path());
}

static inline char *twin_readline(const char *prompt) {
    char *s = readline(prompt ? prompt : "");
    if (s && *s) add_history(s);
    return s;  
}
#else

static inline void twin_history_init(void)  {}
static inline void twin_history_save(void)  {}
static inline char *twin_readline(const char *prompt) {
    (void)prompt;
    char *buf = NULL; size_t n = 0;
    if (getline(&buf, &n, stdin) == -1) { free(buf); return NULL; }
    size_t len = strlen(buf);
    if (len && buf[len-1] == '\n') buf[len-1] = '\0';
    return buf; 
}
#endif


#endif