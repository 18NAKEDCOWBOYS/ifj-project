/****
 * Implementace překladače imperativního jazyka IFJ20.
 * Josef Kotoun - xkotou06
 * Lexical analysis (scanner)
*/
#include <stdio.h>
#include <ctype.h>
#include "str.h"
#include "scanner.h"
#include "error_codes.h"
#include <stdlib.h>
//#include "error_codes.h"


static keyword_str_pair all_keywords_pairs[] = 
{
    {INT_KEYWORD, "int"},
    {ELSE_KEYWORD, "else"},
    {FLOAT64_KEYWORD, "float64"},
    {FOR_KEYWORD, "for"},
    {FUNC_KEYWORD, "func"},
    {IF_KEYWORD, "if"},
    {PACKAGE_KEYWORD, "package"},
    {RETURN_KEYWORD, "return"},
    {STRING_KEYWORD, "string"}
};
//iterates over keywords pairs strings and if its same, returns true and passes enum code of keyword to parameter *keyword
bool str_is_keyword(string *str, keyword *keyword)
{
    for(int i = 0;i<((int)(sizeof(all_keywords_pairs) / sizeof(keyword_str_pair)));i++)
    {
        //string is same as some of keyword string representation
        if(!strCmpConstStr(str,all_keywords_pairs[i].keyword_str))
        {
            *keyword = all_keywords_pairs[i].keyword;
            return true;
        }
    }
    return false;
}

int get_token(token *token)
{
    int c;
    strClear(token->str);
    if(token == NULL)
    {
        return INTERNAL_COMPILER_ERR;
    }
    state scanner_state = INITIAL_STATE;
    while(1)
    {   
        c = getc(stdin);
        switch(scanner_state)
        {
            case INITIAL_STATE:
                if(c == '\n')
                {
                    token->type = EOL_TOKEN;
                    return OK;
                }
                else if (c == ';') 
                { 
                    token->type = SEMICOLON_TOKEN;
                    return OK; 
                }
                else if (c == ',') 
                { 
                    token->type = COMMA_TOKEN;
                    return OK; 
                }
                else if (c == '+') 
                { 
                    token->type = PLUS_TOKEN; 
                    return OK;
                }
                else if (c == '=') 
                { 
                    scanner_state = EQUALS_ONCE_STATE;
                }
                else if (c == '-') 
                { 
                    token->type = MINUS_TOKEN; 
                    return OK;
                }
                else if(c == '{')
                {
                    token->type = CURLY_BRACKET_LEFT_TOKEN;
                    return OK;
                }
                else if(c == '}')
                {
                    token->type = CURLY_BRACKET_RIGHT_TOKEN;
                    return OK;
                }
                else if(c == ')')
                {
                    token->type = RIGHT_BRACKET_TOKEN;
                    return OK;
                }
                else if(c == '(')
                {
                    token->type = LEFT_BRACKET_TOKEN;
                    return OK;
                }
                else if (c == '*') 
                { 
                    token->type = MULTIPLICATION_TOKEN; 
                    return OK;
                }
                else if (c == ':')
                {
                    scanner_state = COLON_STATE;
                }
                else if (c == '!')
                {
                    scanner_state = EXCLAMATION_MARK_STATE;
                }
                else if(c == '<')
                {
                    scanner_state = LESS_STATE;
                }
                else if(c == '>')
                {
                    scanner_state = GREATER_STATE;
                }
                else if(isalpha(c) || c == '_')
                {
                    scanner_state = ID_KEYWORD_STATE;
                    strAddChar(token->str,c);
                }
                else if(isdigit(c))
                {
                    scanner_state = NUM_STATE;
                    strAddChar(token->str,c);
                }
                else if(c == '/')
                {
                    scanner_state = SLASH_STATE;
                }
                else if(c == '"')
                {
                    scanner_state = STRING_STATE; 
                }
                else if(c == EOF)
                {
                    token->type = EOF_TOKEN;
                    return OK;  
                }
                else if(isspace(c))
                {
                    scanner_state = INITIAL_STATE;
                }     
                else
                {
                    return LEX_ERR;
                }
                break;
            case COLON_STATE:
                if(c == '=')
                {
                   token->type = SHORT_VAR_DECLARATION_TOKEN;
                   return OK;
                }
                else
                {
                    return LEX_ERR;
                }
                break;
            case EQUALS_ONCE_STATE:
                if(c == '=')
                {
                    token->type = EQUALS_TOKEN;
                }
                else
                {
                    ungetc(c,stdin);
                    token->type = ASSIGNMENT_TOKEN;
                }
                return OK;
                break;
            case EXCLAMATION_MARK_STATE:
                if(c == '=')
                {
                    token->type = NOT_EQUALS_TOKEN;
                    return OK;
                }
                else
                {
                    return LEX_ERR;
                }
                break;
            case LESS_STATE:
                if(c == '=')
                {
                    token->type = LESS_EQUAL_TOKEN;
                }
                else
                {
                    ungetc(c, stdin);
                    token->type = LESS_TOKEN;
                }
                return OK;
                break;
            case GREATER_STATE:
                if(c == '=')
                {
                    token->type = GREATER_EQUAL_TOKEN;
                }
                else
                {
                    ungetc(c, stdin);
                    token->type = GREATER_TOKEN;
                }
                return OK;
                break;
            case SLASH_STATE:
                if(c == '/')
                {
                    scanner_state = ONE_LINE_COMMENT_STATE;
                }
                else if(c == '*')
                {
                    scanner_state = BLOCK_COMMENT_STATE;
                }
                else
                {
                    ungetc(c,stdin);
                    token->type = DIVISON_TOKEN;
                    return OK;
                }
                break;
            case ID_KEYWORD_STATE:
                if(isalnum(c))
                {
                    strAddChar(token->str,c);
                }
                else
                {
                    ungetc(c,stdin);
                    keyword keyword_from_str;
                    if(str_is_keyword(token->str,&keyword_from_str))
                    {
                        strClear(token->str);
                        token->type = KEYWORD_TOKEN;
                        token->keyword = keyword_from_str;
                    }
                    else
                    {
                        token->type = ID_TOKEN;
                    }
                    return OK;
                }
                break;
            //decimal numbers automaton
            case NUM_STATE:
                if(isdigit(c))
                {
                    strAddChar(token->str,c);
                }
                else if(c=='e' || c == 'E')
                {
                    scanner_state = EXP_DEC_STATE;
                    strAddChar(token->str, c);
                }
                else if(c == '.')
                {
                    scanner_state = DECIMAL_POINT_STATE;
                    strAddChar(token->str, c);
                }
                else
                {
                    ungetc(c,stdin);
                    token->type = INTEGER_LITERAL_TOKEN;
                    //convert string to int and clear string
                    token->integer = atoi(strGetStr(token->str));
                    strClear(token->str);
                    return OK;
                }
                break;
            case DECIMAL_POINT_STATE:
                if(isdigit(c))
                {
                    strAddChar(token->str, c);
                    scanner_state = DECIMAL_NO_EXP_STATE;
                }
                else
                {
                    strClear(token->str);
                    return LEX_ERR;
                }
                break;
            case DECIMAL_NO_EXP_STATE:
                if(isdigit(c))
                {
                    strAddChar(token->str, c);
                }
                else if(c == 'E' || c == 'e')
                {
                    strAddChar(token->str, c);
                    scanner_state = EXP_DEC_STATE;
                }
                else
                {
                    ungetc(c,stdin);
                    token->decimal = strtod(strGetStr(token->str), NULL);
                    strClear(token->str);
                    return DECIMAL_LITERAL_TOKEN;
                }
                break;
            case EXP_DEC_STATE:
                if(isdigit(c))
                {
                    scanner_state = DECIMAL_WITH_EXP_STATE;
                    strAddChar(token->str, c);
                }
                else if(c == '-' || c == '+')
                {
                    int last_char =strGetStr(token->str)[token->str->length-1]; 
                    if(last_char != '-' && last_char != '+' )
                    {
                        strAddChar(token->str, c);
                    }
                    else
                    {
                        strClear(token->str);
                        return LEX_ERR;
                    }
                }
                else
                {
                    strClear(token->str);
                    return LEX_ERR;
                }
                break;
            case DECIMAL_WITH_EXP_STATE:
                if(isdigit(c))
                {
                    strAddChar(token->str,c);
                }
                else
                {
                    ungetc(c, stdin);
                    token->type = DECIMAL_LITERAL_TOKEN;
                    token->decimal = strtod(strGetStr(token->str), NULL);
                    strClear(token->str);
                    return OK;
                }
                break;
            case ONE_LINE_COMMENT_STATE:
                if(c == '\n')
                {
                    scanner_state = INITIAL_STATE;
                }
                break;
            case BLOCK_COMMENT_STATE:
                if (c == '*')
                {
                    scanner_state = BLOCK_COMMENT_STAR_STATE;
                }
                break;
            case BLOCK_COMMENT_STAR_STATE:
                if(c == '/')
                {
                    scanner_state = INITIAL_STATE;
                }
                else if (c != '*')
                {
                    scanner_state = BLOCK_COMMENT_STATE;
                }
                break;
            case STRING_STATE:
                if(c == '"')
                {
                    token->type = STRING_LITERAL_TOKEN;
                    return OK;
                }
                else if(c == '\\')
                {
                    scanner_state = ESCAPE_STRING_STATE;
                }
                else if(c<=31)
                {
                    strClear(token->str);
                    return LEX_ERR;
                }
                else
                {
                    strAddChar(token->str, c);
                }
                break;
            case ESCAPE_STRING_STATE:
                if(c<=31)
                {
                    strClear(token->str);
                    return LEX_ERR;
                }
                else if(c == '\\' || c == '"')
                {
                    strAddChar(token->str, c);
                    scanner_state = STRING_STATE;
                }
                else if(c == 'n')
                {
                    strAddChar(token->str, '\n');
                    scanner_state = STRING_STATE;
                }
                else if(c == 't')
                {
                    strAddChar(token->str, '\t');
                    scanner_state = STRING_STATE;
                }
                else if(c == 'x')
                {
                    scanner_state = HEXA_ESCAPE_FIRST_STATE;
                }
                else
                {
                    strClear(token->str);
                    return LEX_ERR;
                }
                break;
            case HEXA_ESCAPE_FIRST_STATE:
                if(isxdigit(c))
                {
                    //read second char to escape sequence
                    int hexa2 = getc(stdin);
                    if(isxdigit(hexa2))
                    {
                        //init char array for int conversion by strtol 
                        char hexa_literal[] = {c, hexa2};
                        int hexa_char =(int)strtol(hexa_literal,NULL,16);
                        strAddChar(token->str, hexa_char);
                        scanner_state = STRING_STATE;
                    }
                    else
                    {
                        strClear(token->str);
                        return LEX_ERR;
                    }
                }
                else
                {
                    strClear(token->str);
                    return LEX_ERR;
                }
                break;  
        }
    }
}