/****
 * Implementace překladače imperativního jazyka IFJ20.
 * Josef Kotoun - xkotou06
 *
 * Used from official example "simple interpret"
 *
 * library for operations with strings
 * */

#include <string.h>
#include <malloc.h>
#include "str.h"

 //default allocation size in bytes
 //every realloc, allocated memory is increased by this contant

int strInit(string* s)
{
   if ((s->str = (char*)malloc(STR_LEN_INC)) == NULL)
      return STR_ERROR;
   s->str[0] = '\0';
   s->length = 0;
   s->allocSize = STR_LEN_INC;
   return STR_SUCCESS;
}

void strFree(string* s)
{
   free(s->str);
}

void strClear(string* s)
{
   s->str[0] = '\0';
   s->length = 0;
}
//adds char to end of string
//if allocated memory is less than required, string is reallocated using realloc
int strAddChar(string* s1, char c)
{
   if (s1->length + 1 >= s1->allocSize)
   {
      if ((s1->str = (char*)realloc(s1->str, s1->length + STR_LEN_INC)) == NULL)
         return STR_ERROR;
      s1->allocSize = s1->length + STR_LEN_INC;
   }
   s1->str[s1->length] = c;
   s1->length++;
   s1->str[s1->length] = '\0';
   return STR_SUCCESS;
}
int strAddConstStr(string* str, char* source)
{
   int i = 0;
   while (source[i] != '\0')
   {
      if (strAddChar(str, source[i]) == STR_ERROR)      
{
         return STR_ERROR;
      }
      i++;
   }
   return STR_SUCCESS;
}

int strCopyString(string* s1, string* s2)
{
   int newLength = s2->length;
   if (newLength >= s1->allocSize)
   {
      if ((s1->str = (char*)realloc(s1->str, newLength + 1)) == NULL)
         return STR_ERROR;
      s1->allocSize = newLength + 1;
   }
   strcpy(s1->str, s2->str);
   s1->length = newLength;
   return STR_SUCCESS;
}
//returns 0 if strings are identical (same as strcmp)
int strCmpString(string* s1, string* s2)
{
   return strcmp(s1->str, s2->str);
}
//returns 0 if string is identical as given char litelar (same as strcmp)
int strCmpConstStr(string* s1, char* s2)
{
   return strcmp(s1->str, s2);
}
//return char literal from string structure
char* strGetStr(string* s)
{
   return s->str;
}

int strGetLength(string* s)
{
   return s->length;
}