/**
 * @file expression_stack.h
 * @brief Expressions stack definition
 * @author Vít Tlustoš - xtlust05@stud.fit.vutbr.cz
 *
 * Represents expression stack for precedence analysis
 */
#pragma once

#include "expression.h"
#include "symtable.h"

/**
 * @enum Represents an expression symbol type
 */
typedef enum{
    plus,           /// +
    minus,          /// -
    multiply,       /// *    
    devide,         /// /
    lbrack,         /// (
    rbrack,         /// )
    eq,             /// =
    neq,            /// !=
    less,           /// <
    less_eq,        /// <=
    greater,        /// >
    greater_eq,     /// >=
    id,             /// id
    int_lit,        /// int literal
    string_lit,     /// string literal
    float64_lit,    /// float64 literal
    nt,             /// non-terminal
    reduce_br,      /// symbol that determinest stop of the reduction (<)
    dollar          /// $
} expression_symbol;

typedef enum{
    INT_VAL,                /// if value may be derived from int literals during compilation
    DECIMAL_VAL,            /// if value may be derived from float literals during compilation
    NOT_DETERMINED      /// if cannot value cannot be determined
} derived_value_state;

/**
 * @brief Represents derived value type used for 0 division check
 */
typedef struct {
    int64_t integer;            /// derived value of int literals only
    double decimal;             /// derived value of float literals only
    derived_value_state state;  /// state of value derivation
} derived_value;

/**
 * @brief Represents stack node type
 */
typedef struct expression_stack_node{
    expression_symbol symbol;           /// expression symbol type, e.g. id, $, ....
    varType type;                       /// represents type of the symbol (for type conversions and semantics check)
    struct expression_stack_node *next; /// ptr to next expression_stack_node
    derived_value value;                /// derived value used for 0 division check
} expression_stack_node;

/**
 * @brief Represents stack that is used when analysing expressions
 */
typedef struct expression_stack{
    expression_stack_node *top;
} expression_stack;

/**
 * @brief Initializes the stack
 */
void stack_init(expression_stack *stack);

/**
 * @brief Pushes symbol to the stack
 * @param symbol_type represents the actual "symbol"
 * @param type represents type of the symbol (for type conversions and semantics check)
 * @param value represents derived value (for 0 division check)
 * @return EXIT_CODE see error_codes.h
 */
int stack_push(expression_stack *stack,
    expression_symbol symbol,
    varType type,
    derived_value value);

/**
 * @brief Pushes symbol to the stack after the top terminal
 * Should be used when inserting < to the stack
 * @param symbol_type represents the actual "symbol"
 * @param type represents type of the symbol (for type conversions and semantics check)
 * @return EXIT_CODE see error_codes.h
 */
int stack_push_after_top_terminal(expression_stack *stack, 
    expression_symbol symbol,
    varType type);

/**
 * @brief Pops the stack
 * @param symbol_type represents the actual "symbol"
 * @param amount amount of pops, default = 1 more if multiple is required
 * @return EXIT_CODE see error_codes.h
 */
int stack_pop(expression_stack *stack, int amount);

/**
 * @brief Gets the top terminal from stack
 * @param stack represents the expression_stack
 * @param out_top_terminal represents the output top terminal
 * @return EXIT_CODE see error_codes.h
 */
int stack_top_terminal(expression_stack *stack, 
    expression_stack_node **out_top_terminal);

/**
 * @brief Detects all reduction element(s) appearing in some rule
 * @param out_reduce_element_0 returns the last or an only element appearing in the rule if only one element in the rule
 * @param out_reduce_element_1 returns the seccond element appearing in the rule if any
 * @param out_reduce_element_2 returns the first element appearing in the rule if any
 * @return EXIT_CODE see error_codes.h
 */
int stack_reduction_elements(expression_stack *stack,
    expression_stack_node **out_reduce_element_0, 
    expression_stack_node **out_reduce_element_1,
    expression_stack_node **out_reduce_element_2);

/**
 * @brief Frees all alocated resources
 */
void stack_dispose(expression_stack *stack);