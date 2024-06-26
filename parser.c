#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>

#include "lexer.h"
#include "ad.h"
#include "utils.h"
#include "parser.h"

Token *iTk;		// the iterator in the tokens list
Token *consumedTk;		// the last consumed token

Symbol *owner;

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
    printf(" => found %s\n",tkCodeName(iTk->code));
    return false;
}



//structDef: STRUCT ID LACC varDef* RACC SEMICOLON
bool structDef(){
    Token *ptrStart = iTk;
    if(consume(STRUCT)) {	
        if(consume(ID)) {
            Token *tkName = consumedTk;
            if(consume(LACC)) {
                Symbol *s=findSymbolInDomain(symTable,tkName->text);
                if(s)tkerr("symbol redefinition: %s",tkName->text);
                s=addSymbolToDomain(symTable,newSymbol(tkName->text,SK_STRUCT));
                s->type.tb=TB_STRUCT;
                s->type.s=s;
                s->type.n=-1;
                pushDomain();
                owner=s;
                for(;;) {
                    if(varDef()) {}
                    else break;
                }
               if(consume(RACC)) {
                    if(consume(SEMICOLON)) {
                        owner=NULL;
                        dropDomain();
                        return true;
                    }else tkerr("Missing ; after struct definition ");
                }else tkerr("Missing } from the struct definition");
            }
        }
    }
    iTk = ptrStart;
    return false;
}

//varDef: typeBase ID arrayDecl? SEMICOLON
bool varDef(){
    Type t;
	Token *ptrStart = iTk;
	if(typeBase(&t) )
		{
			if(consume(ID)){
                Token *tkName = consumedTk;
				if(arrayDecl(&t)){
                    if(t.n==0)tkerr("a vector variable must have a specified dimension");
                }
				if(consume(SEMICOLON)){
                    Symbol *var=findSymbolInDomain(symTable,tkName->text);
                    if(var)tkerr("symbol redefinition: %s",tkName->text);
                    var=newSymbol(tkName->text,SK_VAR);
                    var->type=t;
                    var->owner=owner;
                    addSymbolToDomain(symTable,var);
                    if(owner){
                        switch(owner->kind){
                            case SK_FN:
                                var->varIdx=symbolsLen(owner->fn.locals);
                                addSymbolToList(&owner->fn.locals,dupSymbol(var));
                                break;
                            case SK_STRUCT:
                                var->varIdx=typeSize(&owner->type);
                                addSymbolToList(&owner->structMembers,dupSymbol(var));
                                break;
                            case SK_VAR: break;
                            case SK_PARAM:break;
                        }
                    }else{
                    var->varMem=safeAlloc(typeSize(&t));
                    }
					return true;
				}else tkerr("Missing ; from the variable declaration");
			}else tkerr("The name of the variable is missing");
		}
	iTk = ptrStart;
	return false;

}


// typeBase: TYPE_INT | TYPE_DOUBLE | TYPE_CHAR | STRUCT ID
bool typeBase(Type *t){
    t->n=-1;
	Token *ptrStart = iTk;	
	if(consume(TYPE_INT)){
        t->tb=TB_INT;
		return true;
	}
	if(consume(TYPE_DOUBLE)){
        t->tb=TB_DOUBLE;
		return true;
	}
	if(consume(TYPE_CHAR)){
        t->tb=TB_CHAR;
		return true;
		}
	if(consume(STRUCT)){
		if(consume(ID)){
            Token *tkName = consumedTk;
            t->tb=TB_STRUCT;
            t->s=findSymbol(tkName->text);
            if(!t->s)tkerr("structura nedefinita: %s",tkName->text);
			return true;
		}tkerr("No struct name");
	}
	iTk = ptrStart;
	return false;
	}

//arrayDecl: LBRACKET INT? RBRACKET
bool arrayDecl(Type *t)
{
	Token *ptrStart = iTk;
	if(consume(LBRACKET)) {
        if(consume(INT)) {
            Token *tkSize=consumedTk;
            t->n=tkSize->i;
        }else {
            t->n=0;
        } 
        
        if(consume(RBRACKET)) {
            return true;
        } tkerr("missing ] from the array definition ");
    }
	iTk = ptrStart;
	return false;
}

				
//fnDef: ( typeBase | VOID ) ID LPAR ( fnParam ( COMMA fnParam )* )? RPAR stmCompound
bool fnDef(){

	Token *ptrStart = iTk;	
    Type t;

	if(consume(VOID))
	{ t.tb=TB_VOID;
			if (consume(ID)){
                Token *tkName = consumedTk;
				if (consume(LPAR))
				{
                    Symbol *fn=findSymbolInDomain(symTable,tkName->text);
                    if(fn)tkerr("symbol redefinition: %s",tkName->text);
                    fn=newSymbol(tkName->text,SK_FN);
                    fn->type=t;
                    addSymbolToDomain(symTable,fn);
                    owner=fn;
                    pushDomain();

					if (fnParam())
						{
							for(;;)
							{
								if (consume(COMMA)){
									if(fnParam())
										{}
									else{ tkerr("Missing parameter after , in the function definition");		
										break;
									}				
								}else break;
								
							}
						}
					if(consume(RPAR)){
                        if(stmCompound(false)){
                            dropDomain();
                            owner=NULL;
                            return true;
                        }
                    }
				}else tkerr("Missing ( in the function definition)");
			}else tkerr("Missing function name");
	}
	else if(typeBase(&t)){
			if (consume(ID)){
                Token *tkName = consumedTk;
				if (consume(LPAR)){
                    Symbol *fn=findSymbolInDomain(symTable,tkName->text);
                    if(fn)tkerr("symbol redefinition: %s",tkName->text);
                    fn=newSymbol(tkName->text,SK_FN);
                    fn->type=t;
                    addSymbolToDomain(symTable,fn);
                    owner=fn;
                    pushDomain(); 
					if (fnParam()){
						for(;;){
							if (consume(COMMA)){
								if(fnParam()){}
									else tkerr("Missing parameter after , in the function definition");						
								}else break;
								
							}
						}
					if(consume(RPAR))
						if(stmCompound(false))
                            {dropDomain();
                            owner=NULL;}
							return true;
				
				}
			}else tkerr("Missing function name");
		}
	iTk = ptrStart;
	return false;
}

//fnParam: typeBase ID arrayDecl?
bool fnParam(){
    Type t;
	Token *ptrStart = iTk;
	if(typeBase(&t)) {
        if(consume(ID)) {
            Token *tkName=consumedTk;
            if(arrayDecl(&t)) {
                t.n=0;
                return true;
            }
            Symbol *param=findSymbolInDomain(symTable,tkName->text);
            if(param)tkerr("symbol redefinition: %s",tkName->text);
            param=newSymbol(tkName->text,SK_PARAM);
            param->type=t;
            param->owner=owner;
            param->paramIdx=symbolsLen(owner->fn.params);
            // parametrul este adaugat atat la domeniul curent, cat si la parametrii fn
            addSymbolToDomain(symTable,param);
            addSymbolToList(&owner->fn.params,dupSymbol(param));
            return true;
        } tkerr("Missing the name of the parameter of the function");
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
bool stm() {
    
    Token *start = iTk;
    if(stmCompound(true)){
        return true;
    }

    if(consume(IF)) {
        if(consume(LPAR)) {
            if(expr()) {
                if(consume(RPAR)) {
                    if(stm()){
                        if(consume(ELSE)) {
                            if(stm()){
                                return true;
                            } else tkerr("statement missing after if clase");
                        }
                        return true;
                    } else tkerr("statement missing after if clase");
                } else tkerr("missing ) in if");
            } else tkerr("missing expressiom in if");
        } else tkerr("missing ( in if");
    }
    
    if(consume(WHILE)) {
        if (consume(LPAR)) {
            if(expr()){
                if(consume(RPAR)) {
                    if(stm()) {
                        return true;
                    } else tkerr("missing statement in while");
                }else tkerr("missing ) in while");
            } else tkerr("missing expression in while");
        } else tkerr("missing ( in while");
    }
  
    if(consume(RETURN)) {
        if(expr()){
            if(consume(SEMICOLON)){
                return true;
            } else tkerr("missing ; in return");
        }
        if(consume(SEMICOLON)) return true;
    }
    
    if(expr()) {
        if(consume(SEMICOLON)){
            return true;
        } else tkerr("missing ; in expresie");
    }
    else if(consume(SEMICOLON)) return true;

    iTk = start;
    return false;
}

 ///stmCompound: LACC ( varDef | stm )* RACC
bool stmCompound(bool newDomain){

	Token *ptrStart = iTk;
	if(consume(LACC)) {
        if(newDomain)pushDomain();
        for(;;) { 
            if(varDef()) {}
            else if(stm()) {}
            else break;
        }
        if(consume(RACC)) {if(newDomain)dropDomain();
            return true;
        } else tkerr("missing } in compound statement");
    }
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
	if(exprUnary()) {
        if(consume(ASSIGN)) {
            if(exprAssign()){
                return true;
            } else tkerr("missing expression after =");
        }
        iTk = ptrStart;  //in case of failure
    }
    

    if(exprOr()) {
        return true;
    }
	iTk = ptrStart;
	return false;
}


bool exprOrPrim() {
    Token *ptrStart = iTk;
    if(consume(OR)) {
        if(exprAnd()) {
            if(exprOrPrim()) {
                return true;
            }
        } else tkerr("Lipseste expresia dupa ||");
    }
    iTk = ptrStart;
    return true; // epsilon
}


//exprOr: exprOr OR exprAnd | exprAnd
bool exprOr() {
    Token *ptrStart = iTk;
    if(exprAnd()) {
        if(exprOrPrim()) {
            return true;
        }
    }
    iTk = ptrStart;
    return false;
}


bool exprAndPrim() {
    Token *ptrStart = iTk;
    if(consume(AND)) {
        if(exprEq()) {
            if(exprAndPrim()) {
                return true;
            }
        } else tkerr("missing expression after &&");
    }
    iTk = ptrStart;
    return true; 
}

//exprAnd = exprAnd AND exprEq | exprEq
bool exprAnd() {
   
    Token *ptrStart = iTk;
    if(exprEq()) {
        if(exprAndPrim()) {
            return true;
        }
    }
    iTk = ptrStart;
    return false;
}

////exprEq: exprEq ( EQUAL | NOTEQ ) exprRel | exprRel
bool exprEqPrim() {

    Token *ptrStart = iTk;
    if(consume(EQUAL)|| consume(NOTEQ)) {
        if(exprRel()) {
            if(exprEqPrim()) {
                return true;
            }
        } else tkerr("missing expression after checking equality ");
    }
    
    iTk = ptrStart;
    return true; 
}

bool exprEq() {
    
    Token *ptrStart = iTk;
    if(exprRel()) {
        if(exprEqPrim()) {
            return true;
        }
    }
    iTk = ptrStart;
    return false;
}

//exprRel: exprRel ( LESS | LESSEQ | GREATER | GREATEREQ ) exprAdd | exprAdd
bool exprRelPrim() {
    Token *ptrStart = iTk;
    if(consume(LESS)) {
        if(exprAdd()) {
            if(exprRelPrim()) {
                return true;
            }
        }else tkerr("Missing expression after <");
    }
    else if(consume(LESSEQ)) {
        if(exprAdd()) {
            if(exprRelPrim()) {
                return true;
            }
        } else tkerr("Missing expression after <=");
    }
    else if(consume(GREATER)) {
        if(exprAdd()) {
            if(exprRelPrim()) {
                return true;
            }
        } else tkerr("Missing expression after >");
    }
    else if(consume(GREATEREQ)) {
        if(exprAdd()) {
            if(exprRelPrim()) {
                return true;
            }
        } else tkerr("Missing expression after >=");
    }
    iTk = ptrStart;
    return true; 
}

bool exprRel() {
    Token *ptrStart = iTk;
    if(exprAdd()) {
        if(exprRelPrim()) {
            return true;
        }
    }
    iTk = ptrStart;
    return false;
}

//exprAdd = exprAdd ( ADD | SUB ) exprMul | exprMul
// recursivitate la stanga
// 16
bool exprAddPrim() {
    
    Token *ptrStart = iTk;
    if(consume(ADD)|consume(SUB)){
		if(exprMul()){
			if(exprAddPrim())
				return true;
		}else tkerr("lipseste termenn dupa + / -");
	}
	
    iTk = ptrStart;
    return true; 
}

bool exprAdd() {
    
    Token *ptrStart = iTk;
    if(exprMul()) {
        if(exprAddPrim()) {
            return true;
        }
    }
    iTk = ptrStart;
    return false;
}

///exprMul: exprMul ( MUL | DIV ) exprCast | exprCas
bool exprMulPrim() {
    Token *ptrStart = iTk;
    if(consume(MUL)) {
        if(exprCast()) {
            if(exprMulPrim()) {
                return true;
            }
        } else tkerr("missing expression after *");
    }
    else if(consume(DIV)) {
        if(exprCast()) {
            if(exprMulPrim()) {
                return true;
            }
        } else tkerr("missing expression after /");
    }
    iTk = ptrStart;
    return true; 
}

bool exprMul() {
    Token *ptrStart = iTk;
    if(exprCast()) {
        if(exprMulPrim()) {
            return true;
        }
    }
    iTk = ptrStart;
    return false;
}

//exprCast: LPAR typeBase arrayDecl? RPAR exprCast | exprUnary
bool exprCast()
{
    Token *ptrStart = iTk;
    if(consume(LPAR)) {
        Type t;
        if(typeBase(&t)) {
            if(arrayDecl(&t)) {
                if(consume(RPAR)) {
                    if(exprCast()) {
                        return true;
                    }
                } else tkerr("missing ) in cast expression");
            }
            
            if(consume(RPAR)) {
                if(exprCast()) {
                    return true;
                }
            }
        } else tkerr("missing or wrong type for the cast expression");
    }

    iTk = ptrStart; //in case of failure
    if(exprUnary()) {
        return true;
    }
    iTk = ptrStart;
    return false;
}

// exprUnary: ( SUB | NOT ) exprUnary | exprPostfix
bool exprUnary() {
    Token *ptrStart = iTk;
    if(consume(SUB)) {
        if(exprUnary()) {
            return true;
        } else tkerr("missing expression after -");
    }
    else if(consume(NOT)) {
        if(exprUnary()) {
            return true;
        } else tkerr("missing expressiong after!");
    }
    if(exprPostfix()) {
        return true;
    }
    iTk = ptrStart;
    return false;
}
/*
//exprPostfix: exprPostfix LBRACKET expr RBRACKET
| exprPostfix DOT ID
| exprPrimary
*/
bool exprPostfixPrim() {
    
    Token *ptrStart = iTk;
    if(consume(LBRACKET)) {
        if(expr()) {
            if(consume(RBRACKET)) {
                if(exprPostfixPrim()) {
                    return true;
                }
            } else tkerr("missing ] from the array");
        } else tkerr("missing expression to acces the array");
    }
    if(consume(DOT)) {
        if(consume(ID)) {
            if(exprPostfixPrim()) {
                return true;
            }
        } else tkerr("Missing identifier after . ");			
    }
    iTk = ptrStart;
    return true;
}

bool exprPostfix(){
	Token *ptrStart = iTk;
	if(exprPrimary())
		if(exprPostfixPrim())
			return true;
	iTk = ptrStart;
	return false;	
}

/*
exprPrimary: ID ( LPAR ( expr ( COMMA expr )* )? RPAR )?
            | INT | DOUBLE | CHAR | STRING | LPAR expr RPAR
*/
//exprPrimary: ID ( LPAR ( expr ( COMMA expr )* )? RPAR )? | INT | DOUBLE | CHAR | STRING | LPAR expr RPAR
bool exprPrimary(){
	Token *ptrStart = iTk;
    if(consume(ID)) {
        if(consume(LPAR)) {
            if(expr()) {
                for(;;) {
                    if(consume(COMMA)) {
                        if(expr()) {

                        } else {
                            tkerr("missing expression in the function call after , ");
                            break;
                        }
                    }
                    else break;
                }
            }
            if(consume(RPAR)) {
                return true;
            } else tkerr("missing ) in function call");
        }
        return true;
    }

    iTk = ptrStart;
    if (consume(INT)) {
        return true;
        return true;
    }

    else if (consume(DOUBLE)) {
        return true;
    }
    else if (consume(CHAR)) {
        return true;
    }
    else if (consume(STRING)) {
        return true;
    }
    if(consume(LPAR)) {
        if(expr()) {
            if(consume(RPAR)) {
                return true;
            } else tkerr("missing ) from function call");
        }
    }
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


