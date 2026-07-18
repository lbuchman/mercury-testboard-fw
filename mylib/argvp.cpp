#include <argvp.h>
#include "Arduino.h"
#include <stdint.h>
#include <logger.hpp>


static char* splitArgv(char **str, char **word);

//https://stackoverflow.com/questions/1706551/parse-string-into-argv-argc
/**
 * Parse out the next non-space word from a string.
 * @note No nullptr protection
 * @param str  [IN]   Pointer to pointer to the string. Nested pointer to string will be changed.
 * @param word [OUT]  Pointer to pointer of next word. To be filled.
 * @return  pointer to string - current cursor. Check it for '\0' to stop calling this function
 */
static char* splitArgv(char **str, char **word) {
    constexpr char QUOTE = '\'';
    bool inquotes = false;

    // optimization
    if(**str == 0) {
        return NULL;
    }

    // Skip leading spaces.
    while(**str && isspace(**str)) {
        (*str)++;
    }

    if(**str == '\0') {
        return NULL;
    }

    // Phrase in quotes is one arg
    if(**str == QUOTE) {
        (*str)++;
        inquotes = true;
    }

    // Set phrase begining
    *word = *str;

    // Skip all chars if in quotes
    if(inquotes) {
        while(**str && **str != QUOTE) {
            (*str)++;
        }

        //if( **str!= QUOTE )
    }
    else {
        // Skip non-space characters.
        while(**str && !isspace(**str)) {
            (*str)++;
        }
    }

    // Null terminate the phrase and set `str` pointer to next symbol
    if(**str) {
        *(*str)++ = '\0';
    }

    return *str;
}

/*
    To support standart convetion last `argv[argc]` will be set to `NULL`
    \param[IN]  str : Input string. Will be changed - splitted to substrings
    \param[IN]  argc_MAX : Maximum a rgc, in other words size of input array \p argv
    \param[OUT] argc : Number of arguments to be filled
    \param[OUT] argv : Array of c-string pointers to be filled. All of these strings are substrings of \p str
    \return Pointer to the rest of string. Check if for '\0' and know if there is still something to parse. \
           If result !='\0' then \p argc_MAX is too small to parse all.
*/
char* parseStrToArgcArgvInsitu(char *str, const int argc_MAX, int *argc, char* argv[]) {
    *argc = 0;

    while(*argc < argc_MAX - 1  &&  splitArgv(&str, &argv[*argc])) {
        ++(*argc);

        if(*str == '\0') {
            break;
        }
    }

    argv[*argc] = nullptr;
    return str;
};

void parseAndPrintOneString(char *input) {
    constexpr size_t argc_MAX = 5;
    char* argv[argc_MAX] = {0};
    int argc = 0;

    char* rest = parseStrToArgcArgvInsitu(input, argc_MAX, &argc, argv);

    if(*rest != '\0') { // or more clear `strlen(rest)==0` but not efficient
        logger().error(logger().printHeader, (char*) __FILE__, __LINE__,  "There is still something to parse. argc_MAX is too small.");
    }

    Serial1.printf("argc : %d\n\r", argc);

    for(int i = 0; i < argc; i++) {
        Serial1.printf("argv[%d]: %s\n\r", i, argv[i]);
    }
}
