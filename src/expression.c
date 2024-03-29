/**
 * @file expression.c
 * @brief Expressions processing implementation
 * @author Vít Tlustoš - xtlust05@stud.fit.vutbr.cz
 *
 * Processing of expressions by precedent analysis
 */

#include "expression.h"
#include "scanner.h"
#include "expression_stack.h"
#include "symtable.h"
#include "code_gen.h"
#include "error_codes.h"
#include "parser_helpers.h"

/**
 * @brief Represents the precedence table
 * rows represent stack terminals, colums represent input terminals
 */
int precedence_table[7][7] = {
//  plus_minus_gr | mul_div_gr | lbrack_gr | rbrack_gr | rel_op_gr | id_lit_gr | dollar_gr
    { R, S, S, R, R, S, R },    // plus_minus_gr
    { R, R, S, R, R, S, R },    // mul_div_gr
    { S, S, S, Eq, S, S, Er },  // lbrack_gr
    { R, R, Er, R, R, Er, R },  // rbrack_gr
    { S, S, S, R, Er, S, R },   // rel_op_gr
    { R, R, Er, R, R, Er, R },  // id_lit_gr
    { S, S, S, Er, S, S, Er}    // dollar_gr
};

int convert_token_to_expression_symbol(token *token, 
    expression_symbol *out_expression_symbol)
{
    switch(token->type){
        // Variable
        case ID_TOKEN:
            *out_expression_symbol = id;
            break;

        // Literals
        case STRING_LITERAL_TOKEN:
            *out_expression_symbol = string_lit;
            break;

        case DECIMAL_LITERAL_TOKEN:
            *out_expression_symbol = float64_lit;
            break;

        case INTEGER_LITERAL_TOKEN:
            *out_expression_symbol = int_lit;
            break;

        // Brackets
        case LEFT_BRACKET_TOKEN:
            *out_expression_symbol = lbrack;
            break;
        case RIGHT_BRACKET_TOKEN:
            *out_expression_symbol = rbrack;
            break;

        // Arithmetic operators
        case PLUS_TOKEN:
            *out_expression_symbol = plus;
            break;
        case MINUS_TOKEN:
            *out_expression_symbol = minus;
            break;
        case MULTIPLICATION_TOKEN:
            *out_expression_symbol = multiply;
            break;
        case DIVISON_TOKEN:
            *out_expression_symbol = devide;
            break;

        // Relational operators
        case EQUALS_TOKEN:
            *out_expression_symbol = eq;
            break;
        case NOT_EQUALS_TOKEN:
            *out_expression_symbol = neq;
            break;
        case LESS_TOKEN:
            *out_expression_symbol = less;
            break;
        case LESS_EQUAL_TOKEN:
            *out_expression_symbol = less_eq;
            break;
        case GREATER_TOKEN:
            *out_expression_symbol = greater;
            break;
        case GREATER_EQUAL_TOKEN:
            *out_expression_symbol = greater_eq;
            break;

        // Unexpected token
        default:
            return SYNTAX_ERR;
    }
    return OK;
}

int convert_expression_symbol_to_terminal_group(expression_symbol expression_symbol,
    terminal_group *out_terminal_group)
{
    switch(expression_symbol){
        // Variable and literals
        case id:
        case string_lit:
        case float64_lit:
        case int_lit:
            *out_terminal_group = id_lit_gr;
            break;

        // Brackets
        case lbrack:
            *out_terminal_group = lbrack_gr;
            break;
        case rbrack:
            *out_terminal_group = rbrack_gr;
            break;

        // Arithmetic operators
        case plus:
        case minus:
            *out_terminal_group = plus_minus_gr;
            break;
        case multiply:
        case devide:
            *out_terminal_group = mul_div_gr;
            break;

        // Relational operators
        case eq:
        case neq:
        case less:
        case less_eq:
        case greater:
        case greater_eq:
            *out_terminal_group = rel_op_gr;
            break;
        
        // Dollar - end symbol
        case dollar:
            *out_terminal_group = dollar_gr;
            break;
            
        // Unexpected token
        default:
            return SYNTAX_ERR;
    }
    return OK;
}

int get_reduction_rule(expression_stack_node *reduce_element_0, 
    expression_stack_node *reduce_element_1,
    expression_stack_node *reduce_element_2,
    reduction_rule *out_reduction_rule)
{
    if(reduce_element_0 != NULL 
        && reduce_element_1 != NULL 
        && reduce_element_2 != NULL)
    {        
        // Rule with 3 elements
        if(reduce_element_0->symbol == nt 
            && reduce_element_2->symbol == nt)
        {
            // E -> E operation E
            switch(reduce_element_1->symbol){
                case plus:
                    // E -> E + E
                    *out_reduction_rule = nt_plus_nt;
                    break;
                case minus:
                    // E -> E - E
                    *out_reduction_rule = nt_minus_nt;
                    break;
                case multiply:
                    // E -> E * E
                    *out_reduction_rule = nt_mul_nt;
                    break;
                case devide:
                    // E -> E / E
                    *out_reduction_rule = nt_div_nt;
                    break;
                case eq:
                    // E -> E == E
                    *out_reduction_rule = nt_eq_nt;
                    break;
                case neq:
                    // E -> E != E
                    *out_reduction_rule = nt_neq_nt;
                    break;
                case less:
                    // E -> E < E
                    *out_reduction_rule = nt_less_nt;
                    break;
                case less_eq:
                    // E -> E <= E
                    *out_reduction_rule = nt_less_eq_nt;
                    break;
                case greater:
                    // E -> E > E
                    *out_reduction_rule = nt_more_nt;
                    break;
                case greater_eq:
                    // E -> E >= E
                    *out_reduction_rule = nt_more_eq_nt;
                    break;              
                default:
                    return SYNTAX_ERR;
            }
            return OK;
        }
        else if(reduce_element_0->symbol == rbrack 
            && reduce_element_1->symbol == nt
            && reduce_element_2->symbol == lbrack)
        {
            // E -> (E)
            *out_reduction_rule = lbrack_nt_rbrack;   
            return OK;
        }
    }
    else if(reduce_element_0 != NULL)
    {
        // Rule with 1 element
        if (reduce_element_0->symbol == id 
            || reduce_element_0->symbol == int_lit 
            || reduce_element_0->symbol == string_lit 
            || reduce_element_0->symbol == float64_lit){
            // E -> i
            *out_reduction_rule = operand;
            return OK;
        }
        // Invalid one operand rule
        return SYNTAX_ERR;
    }
    // Unknown rule
    return SYNTAX_ERR;
}

derived_value get_reduced_const_value(expression_stack_node *reduce_element_0,
    expression_stack_node *reduce_element_1,
    expression_stack_node *reduce_element_2,
    reduction_rule rule)
{
    if(reduce_element_0->value.state == reduce_element_2->value.state 
        && reduce_element_0->value.state != NOT_DETERMINED){
        if(reduce_element_0->value.state == INT_VAL){
            switch (rule){
                case nt_plus_nt:
                    {
                        derived_value out = {
                            .integer = reduce_element_2->value.integer 
                                + reduce_element_0->value.integer,
                            .state = INT_VAL
                        };
                        return out;
                    }
                case nt_minus_nt:
                    {
                        derived_value out = {
                            .integer = reduce_element_2->value.integer 
                                - reduce_element_0->value.integer,
                            .state = INT_VAL
                        };
                        return out;
                    }
                case nt_mul_nt:
                    {
                        derived_value out = {
                            .integer = reduce_element_2->value.integer 
                                * reduce_element_0->value.integer,
                            .state = INT_VAL
                        };
                        return out;
                    }
                case nt_div_nt:
                    {
                        derived_value out = {
                            .integer = reduce_element_2->value.integer 
                                / reduce_element_0->value.integer,
                            .state = INT_VAL
                        };
                        return out;
                    }
                default:
                    break;
            }
        }
        else if(reduce_element_0->value.state == DECIMAL_VAL){
            switch (rule){
                case nt_plus_nt:
                    {
                        derived_value out = {
                            .decimal = reduce_element_2->value.decimal 
                                + reduce_element_0->value.decimal,
                            .state = DECIMAL_VAL
                        };
                        return out;
                    }
                case nt_minus_nt:
                    {
                        derived_value out = {
                            .decimal = reduce_element_2->value.decimal 
                                - reduce_element_0->value.decimal,
                            .state = DECIMAL_VAL
                        };
                        return out;
                    }
                case nt_mul_nt:
                    {
                        derived_value out = {
                            .decimal = reduce_element_2->value.decimal 
                                * reduce_element_0->value.decimal,
                            .state = DECIMAL_VAL
                        };
                        return out;
                    }
                case nt_div_nt:
                    {
                        derived_value out = {
                            .decimal = reduce_element_2->value.decimal 
                                / reduce_element_0->value.decimal,
                            .state = DECIMAL_VAL
                        };
                        return out;
                    }
                default:
                    break;
            }
        }
    }
    derived_value not_det_val = {
        .state = NOT_DETERMINED
    };
    return not_det_val;
}

instruction_type convert_reduction_rule_to_relation(reduction_rule rule){
    switch(rule)
    {
        case nt_eq_nt:
            return EQS;
        case nt_neq_nt:
            return NEQS;
        case nt_less_nt:
            return LTS;
        case nt_less_eq_nt:
            return LSES;
        case nt_more_nt:
            return GTS;
        case nt_more_eq_nt:
            return GTES;
        default:
            return CLEARS; // TO DO error handle - clears is symbolizing error (při překladu to házelo warning, že tady není default)
    }
}

int parse_expression_inner(tDLList *scoped_symtables,
    token *token_arr, 
    int token_count,
    varType *out_type,
    instrumented_node **out_instrumented_arr)
{    
    // Undefined derived_val
    derived_value not_determined_val = {
        .state = NOT_DETERMINED
    };

    // Out var type default
    *out_type = UNDEFINED;

    // Initializes the stack and pushes stop element ($) to the stack
    expression_stack stack;
    stack_init(&stack);
    int push_code_0 = stack_push(&stack, dollar, UNDEFINED, not_determined_val);
    if(push_code_0 != OK){
        stack_dispose(&stack);
        return push_code_0;
    }

    token *input_token = token_arr;
    token_count -= 1;
    while(true){
        // Converts input token to the symbol
        expression_symbol input_symbol;
        if(input_token != NULL){
            int convert_code = convert_token_to_expression_symbol(input_token, &input_symbol);
            if(convert_code != OK){
                stack_dispose(&stack);
                return convert_code;
            }
        }
        else{
            input_symbol = dollar;
        }

        // Gets top terminal from the stack
        expression_stack_node *top_stack_terminal = NULL;
        int top_terminal_code = stack_top_terminal(&stack, &top_stack_terminal);
        if(top_terminal_code != OK){
            stack_dispose(&stack);
            return top_terminal_code;
        }

        // Converts symbols to the terminal groups
        terminal_group input_terminal_group;
        terminal_group top_stack_terminal_group;
        int conversion_code_0 = convert_expression_symbol_to_terminal_group(input_symbol, 
            &input_terminal_group);
        if(conversion_code_0 != OK){
            stack_dispose(&stack);
            return conversion_code_0;
        }
        int conversion_code_1 = convert_expression_symbol_to_terminal_group(top_stack_terminal->symbol,
            &top_stack_terminal_group);
        if(conversion_code_1 != OK){
            stack_dispose(&stack);
            return conversion_code_1;
        }

        // Gets the precedence from the table and perform suitable operation
        precedence precedence_evaluation = (precedence)precedence_table[top_stack_terminal_group][input_terminal_group];
        switch(precedence_evaluation){
            case Eq:
                {
                    // ---------------------------------- EQUAL - = --------------------------------- //
                    // May happen only if top_stack_terminal_group = '(' and input_terminal_group = ')'
                    // NO_CODE_GENERATION and NO_SEMANTIC_CHECK needed = (nt)
                    int push_code_1 = stack_push(&stack, input_symbol, UNDEFINED, not_determined_val);
                    if(push_code_1 != OK){
                        stack_dispose(&stack);
                        return push_code_1;
                    }

                    // Increments token ptr
                    if(token_count > 0){
                        input_token += 1;
                        token_count -= 1;
                    }
                    else{
                        input_token = NULL;
                    }
                }
                break;
                
            case S:
                {
                    // ---------------------------------- SHIFT - < --------------------------------- //    

                    // PUSHES the reduce_br
                    int push_code_1 = stack_push_after_top_terminal(&stack, reduce_br, UNDEFINED);
                    if(push_code_1 != OK){
                        stack_dispose(&stack);
                        return push_code_1;
                    }

                    // PUSHES the input_symbol
                    switch(input_symbol){
                        case id:
                            {
                                varType var_type;
                                int var_scope=get_varType_from_symtable(scoped_symtables, input_token->str, &var_type);
                                if(var_scope != -1){
                                    int push_code_2 = stack_push(&stack, input_symbol, var_type, not_determined_val);
                                    if(push_code_2 != OK){
                                        stack_dispose(&stack);
                                        return push_code_2;
                                    }
                                    generate_add_var_to_stack(var_scope,input_token->str->str);
                                }
                                else{
                                    stack_dispose(&stack);
                                    return VAR_DEFINITION_ERR;
                                }
                            }
                            break;
                        case int_lit:
                            {
                                varType literal_type = get_varType_from_literal(input_token->type);
                                derived_value derived_val = {
                                    .integer = input_token->integer,
                                    .state = INT_VAL
                                };
                                int push_code_2 = stack_push(&stack, input_symbol, literal_type, derived_val);
                                if(push_code_2 != OK){
                                    stack_dispose(&stack);
                                    return push_code_2;
                                }
                                generate_add_int_to_stack(derived_val.integer);
                            }
                            break;
                        case float64_lit:
                            {
                                derived_value derived_val = {
                                    .decimal = input_token->decimal,
                                    .state = DECIMAL_VAL
                                };
                                varType literal_type = get_varType_from_literal(input_token->type);
                                int push_code_2 = stack_push(&stack, input_symbol, literal_type, derived_val);
                                if(push_code_2 != OK){
                                    stack_dispose(&stack);
                                    return push_code_2;
                                }
                                generate_add_float_to_stack(derived_val.decimal);
                            }
                            break;
                        case string_lit:
                            {
                                varType literal_type = get_varType_from_literal(input_token->type);
                                int push_code_2 = stack_push(&stack, input_symbol, literal_type, not_determined_val);
                                if(push_code_2 != OK){
                                    stack_dispose(&stack);
                                    return push_code_2;
                                }
                                generate_add_string_to_stack(input_token->str->str);
                            }
                            break;
                        default:
                            {
                                // other possible input (e.g. +, -, ...)
                                int push_code_2 = stack_push(&stack, input_symbol, UNDEFINED, not_determined_val);
                                if(push_code_2 != OK){
                                    stack_dispose(&stack);
                                    return push_code_2;
                                }
                            }
                            break;
                    }
                   
                    // Increments token ptr
                    if(token_count > 0){
                        input_token += 1;
                        token_count -= 1;
                    }
                    else{
                        input_token = NULL;
                    }
                }
                break;               

            case R:
                {
                    // ---------------------------------- REDUCE - > --------------------------------- //
                    // RULE IDENTIFICATION AND REDUCTION ELEMENTS
                    reduction_rule reduction_rule;
                    expression_stack_node *reduce_element_0 = NULL;
                    expression_stack_node *reduce_element_1 = NULL;
                    expression_stack_node *reduce_element_2 = NULL;
                    int reduction_elements_code = stack_reduction_elements(&stack, &reduce_element_0, 
                        &reduce_element_1, &reduce_element_2);
                    if(reduction_elements_code != OK){
                        stack_dispose(&stack);
                        return reduction_elements_code;
                    }
                    int reduction_rule_code = get_reduction_rule(reduce_element_0, reduce_element_1,
                        reduce_element_2, &reduction_rule);
                    if(reduction_rule_code != OK){
                        stack_dispose(&stack);
                        return reduction_rule_code;
                    }
                    
                    // CODE GENERATION AND SEMANTIC CHECK
                    varType reduced_type = UNDEFINED;
                    derived_value reduced_val = {
                        .state = NOT_DETERMINED
                    };
                    switch(reduction_rule){
                        case operand:
                            // NO_CODE_GENERATION - value has already been pushed to the stack
                            if(reduce_element_0->type == UNDEFINED){
                                stack_dispose(&stack);
                                return VAR_DEFINITION_ERR;
                            }
                            reduced_val = reduce_element_0->value; 
                            reduced_type = reduce_element_0->type;
                            break;
                        case lbrack_nt_rbrack:
                            // NO_CODE_GENERATION - value has already been pushed to the stack
                            if(reduce_element_1->type == UNDEFINED){
                                stack_dispose(&stack);
                                return VAR_DEFINITION_ERR;
                            }
                            reduced_val = reduce_element_1->value; 
                            reduced_type = reduce_element_1->type;
                            break;
                        case nt_plus_nt:
                            if(reduce_element_0->type == STRING && reduce_element_2->type == STRING){
                                generate_add_concat_to_stack();
                                reduced_type = STRING;
                            }
                            else if(reduce_element_0->type == reduce_element_2->type
                                && reduce_element_0->type != BOOL
                                && reduce_element_0->type != UNDEFINED){
                                generate_stack_operation(ADDS);
                                reduced_val = get_reduced_const_value(reduce_element_0, reduce_element_1, 
                                    reduce_element_2, reduction_rule);
                                reduced_type = reduce_element_0->type;
                            }
                            else{
                                // Mismatching data types
                                stack_dispose(&stack);
                                return DATATYPE_COMPATIBILITY_ERR;
                            }
                            break;
                        case nt_minus_nt:
                            if(reduce_element_0->type == reduce_element_2->type
                                && reduce_element_0->type != BOOL 
                                && reduce_element_0->type != STRING
                                && reduce_element_0->type != UNDEFINED){
                                generate_stack_operation(SUBS);
                                reduced_val = get_reduced_const_value(reduce_element_0, reduce_element_1, 
                                    reduce_element_2, reduction_rule);
                                reduced_type = reduce_element_0->type;
                            }
                            else{
                                // Mismatching data types
                                stack_dispose(&stack);
                                return DATATYPE_COMPATIBILITY_ERR;
                            }
                            break;
                        case nt_mul_nt:
                            if(reduce_element_0->type == reduce_element_2->type
                                && reduce_element_0->type != BOOL 
                                && reduce_element_0->type != STRING
                                && reduce_element_0->type != UNDEFINED){
                                generate_stack_operation(MULS);
                                reduced_val = get_reduced_const_value(reduce_element_0, reduce_element_1, 
                                    reduce_element_2, reduction_rule);
                                reduced_type = reduce_element_0->type;
                            }
                            else{
                                // Mismatching data types
                                stack_dispose(&stack);
                                return DATATYPE_COMPATIBILITY_ERR;
                            }
                            break;
                        case nt_div_nt:
                            if(reduce_element_0->type == reduce_element_2->type
                                && reduce_element_0->type == INT){         
                                if(reduce_element_0->value.state != NOT_DETERMINED
                                    && reduce_element_0->value.integer == 0){
                                    return DIVIDE_ZERO_ERR;
                                }                       
                                else{
                                    generate_stack_operation(IDIVS);
                                    reduced_val = get_reduced_const_value(reduce_element_0, reduce_element_1, 
                                        reduce_element_2, reduction_rule);
                                    reduced_type = INT;
                                }
                            }
                            else if(reduce_element_0->type == reduce_element_2->type
                                && reduce_element_0->type == FLOAT){
                                if(reduce_element_0->value.state != NOT_DETERMINED
                                    && reduce_element_0->value.decimal == 0.f){
                                    return DIVIDE_ZERO_ERR;
                                }
                                else{
                                    generate_stack_operation(DIVS);
                                    reduced_val = get_reduced_const_value(reduce_element_0, reduce_element_1, 
                                        reduce_element_2, reduction_rule);
                                    reduced_type = FLOAT;
                                }
                            }
                            else{
                                // Mismatching data types
                                stack_dispose(&stack);
                                return DATATYPE_COMPATIBILITY_ERR;
                            }
                            break;
                        case nt_eq_nt:
                        case nt_neq_nt:
                        case nt_less_nt:
                        case nt_less_eq_nt:
                        case nt_more_nt:
                        case nt_more_eq_nt:
                            if(reduce_element_0->type == reduce_element_2->type
                                && reduce_element_0->type != BOOL
                                && reduce_element_0->type != UNDEFINED){
                                generate_relation(convert_reduction_rule_to_relation(reduction_rule));
                                reduced_type = BOOL;
                            }
                            else{
                                // Mismatching data types
                                stack_dispose(&stack);
                                return DATATYPE_COMPATIBILITY_ERR;
                            }
                            break;                    
                    }

                    // INSTRUMENTATION
                    if(out_instrumented_arr != NULL){
                        instrumented_node *node = *out_instrumented_arr;
                        instrumented_node *new_node = (instrumented_node*)malloc(sizeof(instrumented_node));
                        new_node->rule = reduction_rule;
                        new_node->type = reduced_type;
                        new_node->next = NULL;
                        if (node == NULL){
                            *out_instrumented_arr = new_node;
                        }
                        else{
                            while (node->next != NULL){
                                node = node->next;
                            }
                            node->next = new_node;
                        }
                    }             

                    // STACK UPDATE
                    // Pop - if rule had 3 elements (e.g. E -> E ... E) pops 4x (<E+E) else E -> i pops 2x (<i)
                    int pop_code_0 = stack_pop(&stack, reduce_element_2 != NULL ? 4 : 2);
                    if(pop_code_0 != OK){
                        stack_dispose(&stack);
                        return pop_code_0;
                    }
                    // Pushes reduced non-terminal to the stack
                    int push_code_1 = stack_push(&stack, nt, reduced_type, reduced_val);
                    if(push_code_1 != OK){
                        stack_dispose(&stack);
                        return push_code_1;
                    }
                }
                break;

            case Er:
                // --------------------------------- ERROR OR END --------------------------------- //
                // May happen only if the input is invalid or when the end of the expression is reached
                
                if (input_symbol == dollar && top_stack_terminal->symbol == dollar){
                    // Expression has been successfuly parsed 
                    *out_type = stack.top->type;
                    stack_dispose(&stack);
                    return OK;
                }
                else{
                    // Precedence error
                    stack_dispose(&stack);
                    return SYNTAX_ERR;
                }
                break;
        }
    }

    // Reached only if expression is not valid
    stack_dispose(&stack);
    return SYNTAX_ERR;
}

int parse_expression(tDLList *scoped_symtables,
    token *token_arr, 
    int token_count,
    varType *out_type){
    return parse_expression_inner(scoped_symtables, token_arr, token_count,
        out_type, NULL);
}

int parse_instrumented_expression(tDLList *scoped_symtables,
    token *token_arr, 
    int token_count,
    varType *out_type,
    instrumented_node **out_instrumented_arr){
    return parse_expression_inner(scoped_symtables, token_arr, token_count,
        out_type, out_instrumented_arr);
}