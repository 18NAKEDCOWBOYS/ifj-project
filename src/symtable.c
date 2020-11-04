/****
 * Implementace překladače imperativního jazyka IFJ20.
 * Josef Kotoun - xkotou06
 * Implementation of symtable
 * Some functions for tree data type are used from my implementation of IAL 2. project 
 * 
*/
#include "symtable.h"
#include "error_codes.h"
#include <string.h>
int init(node *rootptr)
{
    if (rootptr == NULL)
    {
        return INTERNAL_COMPILER_ERR;
    }
    rootptr = NULL;
    return OK;
}

bool search(node *rootptr, char *name, node *found_node)
{
    node *current = rootptr;
    while (current != NULL)
    {
        int cmp_result = strcmp(name, current->name);
        if (cmp_result > 0) //right subtree
        {
            current = current->r_ptr;
        }
        else if (cmp_result < 0) //left subtree
        {
            current = current->l_ptr;
        }
        else //found
        {
            found_node = current;
            return true;
        }
    }
    return false;
}
node **getNodeToInsert(node *rootptr, char *name)
{

    if (rootptr == NULL)
    {
        return &rootptr;
    }
    else
    {
        node *current = rootptr;
        do
        {
            int cmp_result = strcmp(name, current->name);
            if (cmp_result > 0)
            {
                //value is greater than node key and rptr is null => store new node into rptr
                if (current->r_ptr == NULL)
                {
                    return &(current->r_ptr);
                }
                current = current->r_ptr;
            }
            else if (cmp_result < 0)
            {
                //value is smaller than node key and lptr is null => store new node into lptr
                if (current->l_ptr == NULL)
                {
                    return &(current->l_ptr);
                }
                current = current->l_ptr;
            }
            else
            {
                //node is already in table - do nothing
                return NULL;
            }
        } while (current != NULL);
    }
}
int insert_node_func(node *rootptr, char *name, unsigned return_types_count, varType return_types[], unsigned params_count, varType params_types[], bool defined)
{
    if (rootptr == NULL)
    {
        return INTERNAL_COMPILER_ERR;
    }
    node **ptrToNodePtr = getNodeToInsert(rootptr, name);
    //symbol with name is already in tree - do nothing
    if (ptrToNodePtr == NULL)
    {
        return OK;
    }
    //allocation of  node
    (*ptrToNodePtr) = (node *)malloc(sizeof(node));
    if (*ptrToNodePtr == NULL)
    {
        return INTERNAL_COMPILER_ERR;
    }
    (*ptrToNodePtr)->l_ptr = rootptr->r_ptr = NULL;
    (*ptrToNodePtr)->name = name;
    (*ptrToNodePtr)->type = func;

    //allocation of structure with function data
    (*ptrToNodePtr)->data = malloc(sizeof(symbol_function));
    if ((*ptrToNodePtr)->data == NULL)
    {
        free(*ptrToNodePtr);
        return INTERNAL_COMPILER_ERR;
    }
    symbol_function *data_ptr = (symbol_function *)(*ptrToNodePtr)->data;
    data_ptr->defined = defined;
    data_ptr->par_count = params_count;

    //allocation of array of param types
    data_ptr->parameters = (varType *)malloc(sizeof(varType) * params_count);
    if (data_ptr->parameters == NULL)
    {
        free(*ptrToNodePtr);
        free((*ptrToNodePtr)->data);
        return INTERNAL_COMPILER_ERR;
    }
    for (int i = 0; i < params_count; i++)
    {
        data_ptr->parameters[i] = params_types[i];
    }
    data_ptr->return_types_count = return_types_count;

    //allocation of array of return types
    data_ptr->return_types = (varType *)malloc(sizeof(varType) * return_types_count);
    if (data_ptr->return_types == NULL)
    {
        free(*ptrToNodePtr);
        free((*ptrToNodePtr)->data);
        free(data_ptr->parameters);
        return INTERNAL_COMPILER_ERR;
    }
    for (int i = 0; i < return_types_count; i++)
    {
        data_ptr->return_types[i] = return_types[i];
    }
    return OK;
}

int insert_node_var(node *rootptr, char *name, varType var_type)
{
    if (rootptr == NULL)
    {
        return INTERNAL_COMPILER_ERR;
    }
    node **ptrToNodePtr = getNodeToInsert(rootptr, name);
    //symbol with name is already in tree - do nothing
    if (ptrToNodePtr == NULL)
    {
        return OK;
    }
    (*ptrToNodePtr) = (node*)malloc(sizeof(node));
    if(*ptrToNodePtr == NULL)
    {
        return INTERNAL_COMPILER_ERR;
    }
    (*ptrToNodePtr)->l_ptr = (*ptrToNodePtr)->r_ptr = NULL;
    (*ptrToNodePtr)->name = name;
    (*ptrToNodePtr)->type = var;
    (*ptrToNodePtr)->data = (symbol_variable*)malloc(sizeof(symbol_variable));
    if((*ptrToNodePtr)->data == NULL)
    {
        free(*ptrToNodePtr);
        return INTERNAL_COMPILER_ERR;
    }
    ((symbol_variable*)(*ptrToNodePtr)->data)->var_type = var_type;
    return OK;
}

int free_node_memory(node *nodeptr)
{
    if(nodeptr == NULL)
    {
        return INTERNAL_COMPILER_ERR;
    }
    if(nodeptr->type == func)
    {
        symbol_function* data = (symbol_function*)nodeptr->data;
        //free params and return types arrays for function
        free(data->parameters);
        free(data->return_types);
    }
    free(nodeptr->data);
    free(nodeptr);
    return OK;
}

int dispose_tree(node* rootptr)
{
	if(rootptr != NULL)
	{
		if(rootptr->l_ptr != NULL)
		{
			BSTDispose(rootptr->l_ptr);
		}
		if(rootptr->r_ptr!= NULL)
		{
			BSTDispose(rootptr->r_ptr);
		}
		free_node_memory(rootptr);
		rootptr = NULL;
	}
    else
    {
        return INTERNAL_COMPILER_ERR;
    }
}

