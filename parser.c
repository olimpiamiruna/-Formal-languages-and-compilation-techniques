#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>

#include "parser.h"

Token *iTk;		// the iterator in the tokens list
Token *consumedTk;		// the last consumed token



void tkerr(const char *fmt,...){
	fprintf(stderr,"error in line %d: ",iTk->line);
	va_list va;
	va_start(va,fmt);
	vfprintf(stderr,fmt,va);
	va_end(va);
	fprintf(stderr,"\n");
	exit(EXIT_FAILURE);
	}

char *tkCodeName(int code) {
    switch(code) {
        case ID:
            return "ID";
        case TYPE_INT:
            return "TYPE_INT";
        case TYPE_CHAR:
            return "TYPE_CHAR";
        case TYPE_DOUBLE:
            return "TYPE_DOUBLE";
        case ELSE:
            return "ELSE";
        case IF:
            return "IF";
        case RETURN:
            return "RETURN";
        case STRUCT:
            return "STRUCT";
        case VOID:
            return "VOID";
        case WHILE:
            return "WHILE";
        case COMMA:
            return "COMMA";
        case SEMICOLON:
            return "SEMICOLON";
        case LPAR:
            return "LPAR";
        case RPAR:
            return "RPAR";
        case LBRACKET:
            return "LBRACKET";
        case RBRACKET:
            return "RBRACKET";
        case LACC:
            return "LACC";
        case RACC:
            return "RACC";
        case END:
            return "END";
        case ADD:
            return "ADD";
        case MUL:
            return "MUL";
        case DIV:
            return "DIV";
        case DOT:
            return "DOT";
        case AND:
            return "AND";
        case OR:
            return "OR";
        case NOT:
            return "NOT";
        case NOTEQ:
            return "NOTEQ";
        case LESS:
            return "LESS";
        case LESSEQ:
            return "LESSEQ";
        case GREATER:
            return "GREATER";
        case GREATEREQ:
            return "GREATEREQ";
        case ASSIGN:
            return "ASSIGN";
        case EQUAL:
            return "EQUAL";
        case SUB:
            return "SUB";
        case INT:
            return "INT";
        case DOUBLE:
            return "DOUBLE";
        case CHAR:
            return "CHAR";
        case STRING:
            return "STRING";
        default:
            return "N\\A";
    }
}

bool consume(int code)
{
    printf("consume(%s)",tkCodeName(code));
    if(iTk->code==code)
    {
        consumedTk=iTk;
        iTk=iTk->next;
        printf(" => consumed\n");
        return true;
    }
    printf(" => found %d\n",iTk->line);
    return false;
}



//structDef: STRUCT ID LACC varDef* RACC SEMICOLON
bool structDef(){
    Token *start = iTk;
    if(consume(STRUCT)) {
		
        if(consume(ID)) {
            if(consume(LACC)) {
                for(;;) {
                    if(varDef()) {
                    }
                    else break;
                }
                if(consume(RACC)) {
                    if(consume(SEMICOLON)) {
						
                        return true;
                    }else tkerr("Lipseste ; dupa definirea structurii");
                }else tkerr("Lipseste } din structura");
            }else tkerr("Lipseste { din structura");
        }else tkerr("Varibila fara nume");
    }else tkerr("nu a fost gasit cuvantul cheie STRUCT");
    iTk = start;
    return false;
}

//varDef: typeBase ID arrayDecl? SEMICOLON
bool varDef(){
	Token *ptrStart = iTk;
	if(typeBase() )
		{
			if(consume(ID)){
				if((arrayDecl()))
					{}
				if(consume(SEMICOLON)){
					printf("\n returnu simocolon");
					return true;
				}else tkerr("Lipseste ; dupa definirea structurii");
			}else tkerr("Variabila fara ID");
		}
	iTk = ptrStart;
	return false;

}


// typeBase: TYPE_INT | TYPE_DOUBLE | TYPE_CHAR | STRUCT ID
bool typeBase(){
	Token *ptrStart = iTk;	
	if(consume(TYPE_INT)){
		return true;
	}
	if(consume(TYPE_DOUBLE)){
		return true;
	}
	if(consume(TYPE_CHAR)){
		return true;
		}
	if(consume(STRUCT)){
		if(consume(ID)){
			return true;
		}tkerr("Variabila nu are nume intr-un struct");
	}
	iTk = ptrStart;
	return false;
	}

//arrayDecl: LBRACKET INT? RBRACKET
bool arrayDecl()
{
	Token *ptrStart = iTk;

	
		if(consume(LBRACKET)){
			{
				if (consume(INT)){}
				if (consume(RBRACKET)) 
					return true;
				else tkerr("Lipseste ] din definitia vectorului ");
			} 
		}else tkerr("lipseste [ din definitia vectorului");
	iTk = ptrStart;
	return false;
}

//fnDef: ( typeBase | VOID ) ID LPAR ( fnParam ( COMMA fnParam )* )? RPAR stmCompound
bool fnDef(){

	Token *ptrStart = iTk;	

	if(typeBase())
		{
			if (consume(ID))
				if (consume(LPAR))
				{
					if (fnParam())
						{
							for(;;)
							{
								if (consume(COMMA)){
									if(fnParam())
										{}
									else tkerr("lipseste parametru dupa ,");						
								}else break;
								
							}
						}
					if(consume(RPAR))
						if(stmCompound())
							return true;
				}else tkerr("lipseste ( in definirea functiei");
			else tkerr("Lipseste numele functiei");
		}

	if(consume(VOID))
		{
			if (consume(ID))
				if (consume(LPAR))
				{
					if (fnParam())
						{
							for(;;)
							{
								if (consume(COMMA)){
									if(fnParam())
										{}
									else tkerr("lipseste parametru dupa ,");						
								}else break;
								
							}
						}
					if(consume(RPAR))
						if(stmCompound())
							return true;
				}else tkerr("lipseste ( in definirea functiei");
			else tkerr("Lipseste numele functiei");
		}
	iTk = ptrStart;
	return false;
}

//fnParam: typeBase ID arrayDecl?
bool fnParam(){

	Token *ptrStart = iTk;
	if(typeBase()){
		if (consume(ID)){
			if(arrayDecl())
				{}
			return true;
		}else printf("lipseste numele parametrului");
	}
	iTk = ptrStart;
	return false;
}



/*
//stm: stmCompound
| IF LPAR expr RPAR stm ( ELSE stm )?
| WHILE LPAR expr RPAR stm
| RETURN expr? SEMICOLON
| expr? SEMICOLON
*/
bool stm(){
	Token *ptrStart = iTk;
	if(stmCompound())
		return true;
	else 
		if(consume(IF)){
			if(consume(LPAR)){
				if(expr()){
					if(consume(RPAR)){
						if(stm()){
							if(consume(ELSE))
								if(stm())
									return true;
						} else return true;
					}else tkerr("lipseste )");
				}
			}else tkerr("lipseste (");

		}else
		 if(consume(WHILE)){
		 	if(consume(LPAR)){
				if(expr()){
					if(consume(RPAR)){
						if(stm())
							return true;
					}else tkerr("lipseste )");
				}
			}else tkerr("lipseste (");
		}
		
		else
		if(consume(RETURN)){
			if(expr()){
				if(consume(SEMICOLON))
					return true;
				else tkerr("lipseste ;");
			}
		}else{
			if(expr()){
				if(consume(SEMICOLON))				
					return true;
				else tkerr("lipseste ;");
			}
		}
		 
	
	iTk = ptrStart;
	return false;
}

 ///stmCompound: LACC ( varDef | stm )* RACC
bool stmCompound(){

	Token *ptrStart = iTk;
	if(consume(LACC))
	{
		for(;;){
			if(varDef()){}
			else if(stm()){}
				else break;
		}
		if(consume(RACC))
				return true;
		else tkerr("lipseste }");
	}else tkerr("lipseste { 42");
	
	iTk = ptrStart;
	return false;
}



//expr: exprAssign
bool expr()
{
	Token *ptrStart = iTk;

	if(exprAssign())
		return true;

	iTk = ptrStart;
	return false;
}

// exprAssign: exprUnary ASSIGN exprAssign | exprOr
bool exprAssign(){
	Token *ptrStart = iTk;

	if(exprUnary())
		if (consume(ASSIGN)){
			if(exprAssign())
				return true;
			if(exprOr())
				return true;
			
		} 
	iTk = ptrStart;
	return false;
}


bool exprOrPrim(){
	Token *ptrStart = iTk;
	if(consume(OR))
		if(exprAnd()){
			if (exprOrPrim())
				return true;
		}	
	
	iTk = ptrStart;
	return false;
}

//exprOr: exprOr OR exprAnd | exprAnd
bool exprOr()
{
	Token *ptrStart = iTk;
	if(exprAnd())
		if(exprOrPrim())
			return true;
	iTk = ptrStart;
	return false;

}
bool exprAndPrim(){
	Token *ptrStart = iTk;

	if(consume(OR))
		if(exprAnd()){
			if (exprOrPrim())
				return true;
			
		}
	iTk = ptrStart;
	return false;
}


///exprAnd: exprAnd AND exprEq | exprEq
bool exprAnd()
{
	Token *ptrStart = iTk;

	if(exprAnd())
		if(exprAndPrim())
			return true;

	iTk = ptrStart;
	return false;

}

////exprEq: exprEq ( EQUAL | NOTEQ ) exprRel | exprRel
bool exprEqPrim()
{
	Token *ptrStart = iTk;
	if(consume(EQUAL)|consume(NOTEQ)){
		if(exprRel()){
			if(exprEqPrim()){
				return true;
			}else tkerr("lipseste expresia in exprEq");
		}
	}
	
	iTk = ptrStart;
	return false;
}
bool exprEq(){
	Token *ptrStart = iTk;

	if(exprRel())
		if(exprEqPrim())
			return true;
	iTk = ptrStart;
	return false;
}


//exprRel: exprRel ( LESS | LESSEQ | GREATER | GREATEREQ ) exprAdd | exprAdd
bool exprRelPrim(){
	Token *ptrStart = iTk;
	if(consume(LESS)|consume(LESSEQ)|consume(GREATER)|consume(GREATEREQ)){
		if(exprAdd()){
			if(exprRelPrim())
				return true;
		}else tkerr("lipseste elementul relatiei");
	}
	;
	iTk = ptrStart;
	return false;
}
bool exprRel(){
	Token *ptrStart = iTk;

	if(exprAdd())
		if(exprRelPrim())
			return true;
	iTk = ptrStart;
	return false;
}

///exprAdd: exprAdd ( ADD | SUB ) exprMul | exprMul
bool exprAddPrim(){
	Token *ptrStart = iTk;
	if(consume(ADD)|consume(SUB)){
		if(exprMul()){
			if(exprAddPrim())
				return true;
		}else tkerr("lipseste termenn dupa + / -");
	}
	
	iTk = ptrStart;
	return false;
}
bool exprAdd(){
	Token *ptrStart = iTk;

	if(exprMul())
		if(exprAddPrim())
			return true;
	iTk = ptrStart;
	return false;
}


///exprMul: exprMul ( MUL | DIV ) exprCast | exprCas
bool exprMulPrim(){
	Token *ptrStart = iTk;
	if(consume(MUL)|consume(DIV)){
		if(exprCast()){
			if(exprMulPrim())
				return true;
			
		}else tkerr("lipseste termenn dupa * / /");
	}
	
	iTk = ptrStart;
	return false;
}
bool exprMul(){
	Token *ptrStart = iTk;

	if(exprCast())
		if(exprMulPrim())
			return true;
	
	iTk = ptrStart;
	return false;
}

//exprCast: LPAR typeBase arrayDecl? RPAR exprCast | exprUnary
bool exprCast()
{
	Token *ptrStart = iTk;

	if(consume(LPAR)){
		if(typeBase()){
			if(arrayDecl()){}
			if(consume(RPAR)){
				if(exprCast())
					return true;
			}else tkerr("lipseste )");
		}
	}else tkerr("lipseste ( ");

	if(exprUnary())
		return true;
	
	iTk = ptrStart;
	return false;

}

// exprUnary: ( SUB | NOT ) exprUnary | exprPostfix
bool exprUnary(){
	
	Token *ptrStart = iTk;

	if(consume(SUB)){
		if(exprUnary())
			return true;
		else  tkerr("lipseste termenn dupa -");
	}
	if(consume(NOT))
		if(exprUnary())
			return true;
	if(exprPostfix())
		return true;

	
	iTk = ptrStart;
	return false;
}


/*
//exprPostfix: exprPostfix LBRACKET expr RBRACKET
| exprPostfix DOT ID
| exprPrimary
*/
bool exprPostfixPrim(){
	Token *ptrStart = iTk;
	if(consume(LBRACKET)){
		if(expr()){
			if(consume(RBRACKET)){
				if(exprPostfixPrim())
					return true;
			}else tkerr("lipseste }");
		}else
			if(consume(DOT)){
				if(consume(ID)){
					if(exprPostfixPrim())
						return true;
				}else tkerr("Variabila fara nume");
			}else tkerr("eroare in exprPostFix");	
	}else tkerr("Lipseste {");
	
	iTk = ptrStart;
	return false;	
}
bool exprPostfix(){
	Token *ptrStart = iTk;
	if(exprPrimary())
		if(exprPostfixPrim())
			return true;
	iTk = ptrStart;
	return false;	
}

//exprPrimary: ID ( LPAR ( expr ( COMMA expr )* )? RPAR )? | INT | DOUBLE | CHAR | STRING | LPAR expr RPAR
bool exprPrimary(){

	Token *ptrStart = iTk;

	if(consume(ID)){
		if(consume(LPAR)){
			if(expr()){
				if(consume(COMMA))
					if(expr())
						{}
						
			}
			if (consume(RPAR))
				return true;
		}
		if(consume(INT)|consume(DOUBLE)|consume(CHAR)|consume(STRING)|consume(LPAR))
			if(expr())
				if(consume(RPAR))
					return true;
	}else tkerr("lipseste numele varibilei");
	tkerr("eroare in exprPrimary");
	iTk = ptrStart;
	return false;
}

// unit: ( structDef | fnDef | varDef )* END
bool unit()
{   
    Token *start = iTk;
    for(;;)
    {
        if(structDef()) {}
        else if(fnDef()) {}
        else if(varDef()) {}
        else break;
    }
    if(consume(END))
    {
        return true;
    }
    iTk = start;
    return false;
}

void parse(Token *tokens)
{
    iTk=tokens;
    if(!unit())tkerr("syntax error");
}