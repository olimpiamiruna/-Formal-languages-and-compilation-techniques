#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>

#include "lexer.h"
#include "utils.h"

Token *tokens;	// single linked list of tokens
Token *lastTk;		// the last token in list

int line=1;		// the current line in the input file

// adds a token to the end of the tokens list and returns it
// sets its code and line

Token *addTk(int code){
	Token *tk=safeAlloc(sizeof(Token));
	tk->code=code;
	tk->line=line;
	tk->next=NULL;
	if(lastTk){
		lastTk->next=tk;
		}else{
		tokens=tk;
		}
	lastTk=tk;
	return tk;
	}

char *extract(const char *begin,const char *end){
    
 //begin = inceputul subsirului     end = adresa primului caracter de dupa subsir
	if (begin == NULL || end == NULL)
        err("NULL");

  	//alocare dinamica pt sir
    size_t size = end - begin;
    char *text= (char *)malloc((size + 1) * sizeof(char));


    if (text == NULL)
       err("malloc failed pt functia extract \n");

    strncpy(text, begin, size); //copiaza cel mult n caractere din sirul begin in sirul text.
    text[size] = '\0';  //terminator sir

    return text;
}


Token *tokenize(const char *pch){
	const char *start;
	Token *tk = (Token *)malloc(sizeof(Token));
	if (tk == NULL) 
    	err("Malloc failed pt functia tokenize");
	for(;;){
		
		switch(*pch){
			case ' ':case '\t':pch++;break;
			case '\r':		// handles different kinds of newlines (Windows: \r\n, Linux: \n, MacOS, OS X: \r or \n)
				if(pch[1]=='\n')pch++; //\r muta cursorul la inceputul liniei curente
				// fallthrough to \n
			case '\n':
				line++; //the current line in the input file + 1
				pch++;
				break;
						
			case '\0':addTk(END);return tokens; //se iese din bucla si se returneaza lista de atomi
			case ',':addTk(COMMA); pch++; break;
			case '=':
				if(pch[1]=='='){
					addTk(EQUAL);
					pch+=2;
					}else{
					addTk(ASSIGN);
					pch++;
					}
				break;

			/*
			,COMMA,END,SEMICOLON,LPAR,RPAR,LBRACKET,
			RBRACKET,LACC,RACC
			*/
            case ';' : addTk(SEMICOLON);pch++;break;
            case '(' : addTk(LPAR);pch++;break;
            case ')' : addTk(RPAR);pch++;break;
            case '[' : addTk(LBRACKET);pch++;break;
            case ']' : addTk(RBRACKET);pch++;break;
            case '{' : addTk(LACC);pch++;break;
            case '}' : addTk(RACC);pch++;break;
			case '"' : { 
					const char *start = ++pch; 
					while(pch[0]!='"' && pch[0] != '\0')
					{	
						pch++;
						
					}
					if(pch[0] == '\0')
						err("Ghilimeaua nu a fost gasita");
					
					
					char *text=extract(start,pch);
				
					tk=addTk(STRING);
					tk->text=text;
					pch++;
					break;
			}

			case '\'' : { 
				const char *start = pch; 	

				pch++; // primul apostrof
			
				pch++; // al doilea aprostrof
				if(pch[0]!='\'') //daca nu urmeaza un apostrof dupa acel caracter
					err(" ' nu a fost gasita"); //eroare
				char *text=extract(start,pch);
				tk=addTk(CHAR);
				tk->c = text[1];
				//printf("%c",tk->c);
				pch++;
				break;
			}
		

            /*
            ,ASSIGN,EQUAL, ADD,SUB,MUL,DIV,DOT
            ,AND,OR,NOT,NOTEQ,LESS,LESSEQ,
            GREATER,GREATEREQ
            */

            case '+' : addTk(ADD);pch++;break;
            case '-' : addTk(SUB);pch++;break;
            case '*' : addTk(MUL);pch++;break;
            case '/' :  if (pch[1] == '/') {pch++;pch++;
       						while (pch[0] != '\n' && pch[0] != '\0') 
            					pch++;}
						else {
							addTk(DIV);
							pch++;
						}
						break;
            case '.' :	if(!isdigit(*(pch++))) addTk(DOT);pch++;break;
            case '&' : if(pch[1]=='&') 
							{addTk(AND);pch+=2;} 
						else err("eroare1");
						break;
            case '|' : if(pch[1]=='|') 
							{addTk(OR);pch+=2;}
						else err("eroare2");
						 break;
            case '!' : if(pch[1]=='=') 
							{addTk(NOTEQ);pch+=2;}
                        else {addTk(NOT);pch++;}break;
            case '<' : if(pch[1]=='=')
                        {addTk(LESSEQ);pch+=2;}
                        else {addTk(LESS);pch++;}break;
            case '>' : if(pch[1]=='=')
                        {addTk(GREATEREQ);pch+=2;}
                        else {addTk(GREATER);pch++;}break;
            /*
            ,TYPE_CHAR,TYPE_DOUBLE,ELSE,IF,
            TYPE_INT,RETURN,STRUCT,VOID,WHILE

            */


			default: //testeaza daca atomul incepe cu mai multe caractere
				if(isalpha(*pch)||*pch=='_')
				{
					
					for(start=pch++;isalpha(*pch)||*pch=='_'; pch++){}
					
					char *text=extract(start,pch);
				
					//se verifica daca ID-ul nu este de fapt un cuvant cheie
					if(strcmp(text,"char")==0)addTk(TYPE_CHAR);else
					if(strcmp(text,"if")==0)addTk(IF); else
					if(strcmp(text,"double")==0)addTk(TYPE_DOUBLE);else
					if(strcmp(text,"else")==0)addTk(ELSE);else
					if(strcmp(text,"int")==0)addTk(TYPE_INT);else
					if(strcmp(text,"return")==0)addTk(RETURN); else
					if(strcmp(text,"void")==0)addTk(VOID);else
					if(strcmp(text,"struct")==0)addTk(STRUCT);else
					if(strcmp(text,"while")==0)addTk(WHILE);
					//se adauga ca ID
					else{
						
						tk=addTk(ID);
						tk->text=text;break;
						}
				} else if(isdigit(*pch)) {
						
                        for(start=pch++;isdigit(*pch)||(*pch=='.' && isdigit((*(++pch))))||*pch=='E'||*pch=='e'||*pch=='+'||*pch=='-'; pch++){}
                        
						char *text=extract(start,pch);
						if(strchr(text,'e')!=NULL || strchr(text,'E')!=NULL )
						{
							tk = addTk(DOUBLE);
							tk->d = atof(text); //atof : string -> doubl
							break;
						} else
						if (strchr(text,'.')!=NULL)
							{
								char *afterDot = strchr(text, '.') + 1;  //pointer la urmatorul caracter de dupa punct
								if (!isdigit(*afterDot)) 
									err("dupa punct, trebuie sa urmeze o cifra!");
								else {	
										
										tk = addTk(DOUBLE);       
										tk->d = atof(text); //atof : string -> double
										break;
										}
							}
						
                        else {
                            tk = addTk(INT);
                            tk->i = atoi(text); //atoi : string -> integer
					
							break;
                        }
					} else err("invalid char: %c (%d)",*pch,*pch);
                
				
		}
	}
}


void showTokens(const Token *tokens){
	for(const Token *tk=tokens;tk;tk=tk->next){
		//printf("%d",tk->code);
		switch(tk->code){
			//CUVINTE CHEIE
			case TYPE_CHAR : printf("TYPE_CHAR\n");break;
			case TYPE_DOUBLE : printf("TYPE_DOUBLE\n");break;
			case TYPE_INT : printf("TYPE_INT\n");break;
			case RETURN : printf("RETURN\n");break;
			case STRUCT : printf("STRUCT\n");break;
			case VOID : printf("VOID\n");break;
			case WHILE : printf("WHILE\n");break;
			case IF : printf("IF\n");break;
			case ELSE : printf("ELSE\n");break;

			//constante
			case ID : printf("ID: %s\n", tk->text);break;
			case INT : printf("INT: %d\n", tk->i);break;
			case DOUBLE : printf("DOUBLE: %.5g\n", tk->d);break;
			case STRING : printf("STRING :%s\n",tk->text);break;
			case CHAR : printf("CHAR: %c\n",tk->c);break;

			//delimitatori
			case COMMA : printf("COMMA\n");break;
			case SEMICOLON : printf("SEMICOLON\n");break;
			case LPAR : printf("LPAR\n");break;
			case RPAR : printf("RPAR\n");break;
			case LBRACKET : printf("LBRACKET\n");break;
			case RBRACKET : printf("RBRACKET\n");break;
			case LACC : printf("LACC\n");break;
			case RACC : printf("RACC\n");break;
			case END : printf("END\n");break; 
			
			//operatori
			case DIV : printf("DIV\n");break; 
			case ADD : printf("ADD\n");break; 
			case SUB : printf("SUB\n");break; 
			case MUL : printf("MUL\n");break;
			case AND : printf("AND\n");break; 
			case OR : printf("OR\n");break;  
			case NOT : printf("NOT\n");break;
			case ASSIGN : printf("ASSIGN\n");break;
			case EQUAL : printf("EQUAL\n");break;
			case NOTEQ : printf("NOTEQ\n");break; 
			case LESS : printf("LESS\n");break; 
			case LESSEQ : printf("LESSEQ\n");break; 
			case GREATER : printf("GREATER\n");break; 
			case GREATEREQ : printf("GREATEREQ\n");break; 
			
		}
		
	}
}
