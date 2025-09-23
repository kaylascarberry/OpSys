#ifndef MORSE_H
#define MORSE_H

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include "twin.h"

typedef struct { char ch; const char *code; } MorseEntry;

static const MorseEntry MorseTable[] = {
    {'A', ".-"},    {'B', "-..."}, {'C', "-.-."}, {'D', "-.."},  {'E', "."},
    {'F', "..-."},  {'G', "--."},  {'H', "...."}, {'I', ".."},   {'J', ".---"},
    {'K', "-.-"},   {'L', ".-.."}, {'M', "--"},   {'N', "-."},   {'O', "---"},
    {'P', ".--."},  {'Q', "--.-"}, {'R', ".-."},  {'S', "..."},  {'T', "-"},
    {'U', "..-"},   {'V', "...-"}, {'W', ".--"},  {'X', "-..-"}, {'Y', "-.--"},
    {'Z', "--.."},
    {'0',"-----"}, {'1',".----"}, {'2',"..---"}, {'3',"...--"}, {'4',"....-"},
    {'5',"....."}, {'6',"-...."}, {'7',"--..."}, {'8',"---.."}, {'9',"----."},
    {'.',".-.-.-"}, {',',"--..--"}, {'?',"..--.."}, {'!',"-.-.--"}, {'/',"-..-."},
    {'(',"-.--."},  {')',"-.--.-"}, {'&',".-..."}, {':',"---..."}, {';',"-.-.-."},
    {'=',"-...-"},  {'+',".-.-."}, {'-',"-....-"}, {'_', "..--.-"},
    {'\"',".-..-."},{'$',"...-..-"},{'@',".--.-."}
};

static inline const char* morse_encode_char(char c) {
    if (c == ' ') return "/";                     // space between words
    c = (char)toupper((unsigned char)c);
    for (size_t i = 0; i < sizeof MorseTable/sizeof MorseTable[0]; ++i)
        if (MorseTable[i].ch == c) return MorseTable[i].code;
    return NULL;                                   // unknown char
}

static inline char morse_decode_code(const char *code) {
    if (strcmp(code, "/") == 0) return ' ';
    for (size_t i = 0; i < sizeof MorseTable/sizeof MorseTable[0]; ++i)
        if (strcmp(MorseTable[i].code, code) == 0) return MorseTable[i].ch;
    return '\0';                                   // unknown code
}

static inline void morse_encode_args(int argc, char **argv, int start) {
    int first = 1;
    for (int i = start; i < argc; ++i) {
        const char *w = argv[i];
        for (const char *p = w; *p; ++p) {
            const char *m = morse_encode_char(*p);
            if (!m) m = "?";
            if (!first) printf(" ");
            printf("%s", m);
            first = 0;
        }
        if (i < argc - 1) { printf(" /"); first = 0; } 
    }
    printf("\n");
}

static inline void morse_decode_tokens(int argc, char **argv, int start) {
    for (int i = start; i < argc; ++i) {
        char ch = morse_decode_code(argv[i]);
        if (ch == '\0') ch = '?';
        putchar(ch);
    }
    putchar('\n');
}

#endif