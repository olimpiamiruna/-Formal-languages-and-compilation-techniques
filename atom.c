#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lexer.h"
#include "utils.h"
#include "parser.h"
#include "ad.h"

int main()
{
  
    char *inbuf = loadFile("tests/testad.c");
    
    Token *tokens = tokenize(inbuf);      
                  
    free(inbuf);
    showTokens(tokens);
    pushDomain();
    parse(tokens);
    

    showDomain(symTable,"global");
    dropDomain();
    
    return 0;
}
