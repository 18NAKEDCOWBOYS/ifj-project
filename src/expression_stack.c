/**
 * @file expression_stack.c
 * @brief Expressions stack implementation
 * @author Vít Tlustoš - xtlust05@stud.fit.vutbr.cz
 *
 * Represents expression stack for precedence analysis
 */

#include "expression.h"
#include "symtable.h"
#include "expression_stack.h"
#include "error_codes.h"

void stack_init(expression_stack *stack){
    stack->top = NULL;
}

int stack_push(expression_stack *stack, 
    expression_symbol symbol,
    varType type,
    derived_value value)
{
    expression_stack_node* push_node = (expression_stack_node*)malloc(sizeof(expression_stack_node));
    if(push_node != NULL){
        push_node->symbol = symbol;
        push_node->type = type;
        push_node->next = stack->top;
        push_node->value = value;
        stack->top = push_node;
        return OK;
    }
    return INTERNAL_COMPILER_ERR;
}

int stack_push_after_top_terminal(expression_stack *stack, 
    expression_symbol symbol,
    varType type)
{
    expression_stack_node *prev_node = NULL;
    expression_stack_node* iterable_node = stack->top;
	while(iterable_node != NULL){
		if (iterable_node->symbol != reduce_br && iterable_node->symbol != nt)
		{
            expression_stack_node *push_node = (expression_stack_node*)malloc(sizeof(expression_stack_node));
            if(push_node != NULL){
                push_node->symbol = symbol;
                push_node->type = type;
                if (prev_node != NULL){
                    push_node->next = prev_node->next;
                    prev_node->next = push_node;
                }
                else{
                    push_node->next = stack->top;
                    stack->top = push_node;
                }
                return OK;    
            }
            else{
                return INTERNAL_COMPILER_ERR;
            }
		}
		prev_node = iterable_node;
        iterable_node = iterable_node->next;
	}
    return INTERNAL_COMPILER_ERR;    
}

int stack_pop(expression_stack *stack, 
    int amount)
{
    while(amount != 0){
        if(stack->top != NULL){
            expression_stack_node *pop_node = stack->top;
            stack->top = pop_node->next;
            free(pop_node);
        }
        else{
            // Poping empty stack
            return INTERNAL_COMPILER_ERR;
        }
        amount -= 1;
    }
    return OK;
}

int stack_top_terminal(expression_stack *stack, 
    expression_stack_node **out_top_terminal)
{
    // Find the top terminal
    expression_stack_node *top_terminal = stack->top;
    while(top_terminal != NULL && (top_terminal->symbol == nt 
        || top_terminal->symbol == reduce_br))
    {
        top_terminal = top_terminal->next;
    }

    // Output result
    if(top_terminal != NULL){
        *out_top_terminal = top_terminal;
        return OK;
    }
    else{
        // Error no terminal has been found
        return INTERNAL_COMPILER_ERR;
    }
}

int stack_reduction_elements(expression_stack *stack,
    expression_stack_node **out_reduce_element_0, 
    expression_stack_node **out_reduce_element_1,
    expression_stack_node **out_reduce_element_2)
{
    // Gets elements that should be reduced
    *out_reduce_element_0 = stack->top;
    if(*out_reduce_element_0 != NULL 
        && (*out_reduce_element_0)->next != NULL
        && (*out_reduce_element_0)->next->symbol != reduce_br)
    {
        *out_reduce_element_1 = (*out_reduce_element_0)->next;
    }
    if(*out_reduce_element_1 != NULL 
        && (*out_reduce_element_1)->next != NULL
        && (*out_reduce_element_1)->next->symbol != reduce_br)
    {
        *out_reduce_element_2 = (*out_reduce_element_1)->next;
    }

    if(
        // !result - if error -> true, calls handler
        !(
            // check if stack state format is <|*|*|*
            (*out_reduce_element_0 != NULL 
                && *out_reduce_element_1 != NULL
                && *out_reduce_element_2 != NULL 
                && (*out_reduce_element_2)->next != NULL 
                && (*out_reduce_element_2)->next->symbol == reduce_br)
            ||
            // check if stack state format is <|*
            (*out_reduce_element_0 != NULL 
                && (*out_reduce_element_0)->next != NULL 
                && (*out_reduce_element_0)->next->symbol == reduce_br) 
        )
    )
    {  
        // Invalid stack state - not a suitable rule
        return SYNTAX_ERR;
    }
    return OK;
}
    
void stack_dispose(expression_stack *stack){
    while(stack->top != NULL){
        expression_stack_node* next_node = stack->top->next;
        free(stack->top);
        stack->top = next_node;
    }
}