#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdint.h>

// Function Prototypes (Declarations) for functions used by others
int absolute(int a);

// Using standard subtraction for simplicity, as it's typically constant-time
// on CPUs and bitwise re-implementation is complex for general signed cases
// while preserving behavior identical to C's operator.
// The previous bitwise subtract had subtle borrow logic issues.
int subtract(int a, int b) {
    return a - b;
}

int multiply(int a, int b) {
    int sign = (((unsigned int)a >> 31) ^ ((unsigned int)b >> 31)) & 1;
    unsigned int ua = (unsigned int)absolute(a);
    unsigned int ub = (unsigned int)absolute(b);
    unsigned int result_unsigned = 0;

    for (int i = 0; i < 32; i++) {
        result_unsigned += (-((ub >> i) & 1) & (ua << i));
    }

    return (int)((result_unsigned ^ -sign) + sign);
}

// FIXED mpc_divide_unsigned
// This implements a classic bitwise long division algorithm more robustly.
uint32_t mpc_divide_unsigned(uint32_t numerator, uint32_t denominator) {
    if (denominator == 0) return 0; // Prevent division by zero
    if (numerator < denominator) return 0; // Early exit if numerator < denominator

    uint32_t quotient = 0;
    uint32_t remainder = numerator;

    // Find the highest bit position where denominator can be aligned with numerator
    int shift = 0;
    while ((denominator << shift) <= numerator && (denominator << shift) != 0 && shift < 32) {
        shift++;
    }
    shift--; // Move back to the last valid shift

    // Perform bitwise long division
    for (int i = shift; i >= 0; i--) {
        uint32_t shifted_divisor = denominator << i;
        if (remainder >= shifted_divisor) {
            remainder -= shifted_divisor;
            quotient |= (1U << i);
        }
    }

    return quotient;
}


int divide_signed(int a, int b) {
    if (b == 0) return 0;

    uint32_t ua = (uint32_t)absolute(a);
    uint32_t ub = (uint32_t)absolute(b);
    uint32_t result_unsigned = mpc_divide_unsigned(ua, ub);

    int result = (int)result_unsigned;
    if (((a < 0) ^ (b < 0)) != 0) {
        result = -result;
    }

    return result;
}

int absolute(int a) {
    int mask = a >> 31;
    return (a + mask) ^ mask;
}

bool greater_than(int a, int b) {
    int diff = subtract(a, b);
    int sign_bit = (diff >> 31) & 1;
    return !sign_bit && (diff != 0);
}

bool equal(int a, int b) {
    int diff = subtract(a, b);
    return diff == 0;
}

int ifelse(int a, int b, bool cond) {
    int cond_int = (int)cond;
    int mask = -cond_int;
    return (a & mask) | (b & ~mask);
}

int max(int a, int b) {
    bool cond = greater_than(a, b);
    return ifelse(a, b, cond);
}

int min(int a, int b) {
    bool cond = greater_than(b, a);
    return ifelse(a, b, cond);
}

typedef enum {
    NODE_VARIABLE, NODE_CONSTANT, NODE_OPERATOR, NODE_FUNCTION
} NodeType;

typedef enum {
    OP_ADD, OP_SUB, OP_MUL, OP_DIV
} OperatorType;

typedef enum {
    FUNC_MAX, FUNC_MIN, FUNC_EQUAL, FUNC_GREATER_THAN,
    FUNC_IFELSE, FUNC_ABSOLUTE
} FunctionType;

typedef struct ExprNode {
    NodeType type;
    union {
        char var_name;
        int constant;
        struct {
            OperatorType op;
            struct ExprNode* left;
            struct ExprNode* right;
        } operation;
        struct {
            FunctionType func;
            struct ExprNode* args[3];
            int argc;
        } function;
    };
} ExprNode;

typedef enum {
    TOKEN_VARIABLE, TOKEN_NUMBER, TOKEN_OPERATOR, TOKEN_LPAREN, TOKEN_RPAREN,
    TOKEN_FUNCTION, TOKEN_COMMA, TOKEN_EOF
} TokenType;

typedef struct {
    TokenType type;
    char value[32];
} Token;

typedef struct {
    Token* tokens;
    int count;
    int pos;
} Parser;

bool is_function_name(const char* word) {
    return strcmp(word, "max") == 0 || strcmp(word, "min") == 0 ||
           strcmp(word, "equal") == 0 || strcmp(word, "greater_than") == 0 ||
           strcmp(word, "ifelse") == 0 || strcmp(word, "absolute") == 0;
}

int tokenize(const char* expr, Token tokens[]) {
    int pos = 0, count = 0;
    
    while (expr[pos]) {
        if (isspace(expr[pos])) {
            pos++;
            continue;
        }
        
        if (isalpha(expr[pos])) {
            int start = pos;
            while (isalnum(expr[pos]) || expr[pos] == '_') pos++;
            int len = pos - start;
            strncpy(tokens[count].value, &expr[start], len);
            tokens[count].value[len] = '\0';
            tokens[count].type = is_function_name(tokens[count].value) ?
                                 TOKEN_FUNCTION : TOKEN_VARIABLE;
            count++;
        }
        else if (isdigit(expr[pos]) || (expr[pos] == '-' && isdigit(expr[pos + 1]))) {
            int start = pos;
            if (expr[pos] == '-') pos++;
            while (isdigit(expr[pos])) pos++;
            int len = pos - start;
            strncpy(tokens[count].value, &expr[start], len);
            tokens[count].value[len] = '\0';
            tokens[count++].type = TOKEN_NUMBER;
        }
        else {
            char ch = expr[pos++];
            if (ch == '+' || ch == '-' || ch == '*' || ch == '/') {
                tokens[count++] = (Token){TOKEN_OPERATOR, {ch, '\0'}};
            }
            else if (ch == '(') tokens[count++] = (Token){TOKEN_LPAREN, "("};
            else if (ch == ')') tokens[count++] = (Token){TOKEN_RPAREN, ")"};
            else if (ch == ',') tokens[count++] = (Token){TOKEN_COMMA, ","};
            else {
                printf("Error: Unexpected character '%c' in expression.\n", ch);
                exit(1);
            }
        }
    }
    
    tokens[count++] = (Token){TOKEN_EOF, ""};
    return count;
}

ExprNode* create_node_variable(char var_name) {
    ExprNode* node = malloc(sizeof(ExprNode));
    node->type = NODE_VARIABLE;
    node->var_name = var_name;
    return node;
}

ExprNode* create_node_constant(int value) {
    ExprNode* node = malloc(sizeof(ExprNode));
    node->type = NODE_CONSTANT;
    node->constant = value;
    return node;
}

ExprNode* create_node_operator(OperatorType op, ExprNode* left, ExprNode* right) {
    ExprNode* node = malloc(sizeof(ExprNode));
    node->type = NODE_OPERATOR;
    node->operation.op = op;
    node->operation.left = left;
    node->operation.right = right;
    return node;
}

ExprNode* create_node_function(FunctionType func, ExprNode** args, int argc) {
    ExprNode* node = malloc(sizeof(ExprNode));
    node->type = NODE_FUNCTION;
    node->function.func = func;
    node->function.argc = argc;
    for (int i = 0; i < argc; i++) {
        node->function.args[i] = args[i];
    }
    return node;
}

OperatorType op_type(char op) {
    switch (op) {
        case '+': return OP_ADD;
        case '-': return OP_SUB;
        case '*': return OP_MUL;
        case '/': return OP_DIV;
        default:
            printf("Error: Unknown operator: %c\n", op);
            exit(1);
    }
}

FunctionType func_type(const char* name) {
    if (strcmp(name, "max") == 0) return FUNC_MAX;
    if (strcmp(name, "min") == 0) return FUNC_MIN;
    if (strcmp(name, "equal") == 0) return FUNC_EQUAL;
    if (strcmp(name, "greater_than") == 0) return FUNC_GREATER_THAN;
    if (strcmp(name, "ifelse") == 0) return FUNC_IFELSE;
    if (strcmp(name, "absolute") == 0) return FUNC_ABSOLUTE;
    printf("Error: Unknown function: %s\n", name);
    exit(1);
}

ExprNode* parse_expression(Parser* p);
ExprNode* parse_term(Parser* p);
ExprNode* parse_factor(Parser* p);

Token current_token(Parser* p) {
    if (p->pos < p->count) return p->tokens[p->pos];
    return (Token){TOKEN_EOF, ""};
}

void advance_token(Parser* p) {
    if (p->pos < p->count) p->pos++;
}

ExprNode* parse_function(Parser* p) {
    Token func_token = current_token(p);
    advance_token(p);
    
    if (current_token(p).type != TOKEN_LPAREN) {
        printf("Error: Expected '(' after function name '%s'.\n", func_token.value);
        exit(1);
    }
    advance_token(p);
    
    ExprNode* args[3];
    int argc = 0;
    
    if (current_token(p).type != TOKEN_RPAREN) {
        args[argc++] = parse_expression(p);
        
        while (current_token(p).type == TOKEN_COMMA) {
            advance_token(p);
            if (argc >= 3) {
                printf("Error: Too many arguments for function '%s'. Max 3 arguments supported.\n", func_token.value);
                exit(1);
            }
            args[argc++] = parse_expression(p);
        }
    }

    if (strcmp(func_token.value, "max") == 0 || strcmp(func_token.value, "min") == 0 ||
        strcmp(func_token.value, "equal") == 0 || strcmp(func_token.value, "greater_than") == 0) {
        if (argc != 2) {
            printf("Error: Function '%s' expects 2 arguments, but got %d.\n", func_token.value, argc);
            exit(1);
        }
    } else if (strcmp(func_token.value, "ifelse") == 0) {
        if (argc != 3) {
            printf("Error: Function '%s' expects 3 arguments, but got %d.\n", func_token.value, argc);
            exit(1);
        }
    } else if (strcmp(func_token.value, "absolute") == 0) {
        if (argc != 1) {
            printf("Error: Function '%s' expects 1 argument, but got %d.\n", func_token.value, argc);
            exit(1);
        }
    }
    
    if (current_token(p).type != TOKEN_RPAREN) {
        printf("Error: Expected ')' after function arguments for function '%s'.\n", func_token.value);
        exit(1);
    }
    advance_token(p);
    
    return create_node_function(func_type(func_token.value), args, argc);
}

ExprNode* parse_factor(Parser* p) {
    Token token = current_token(p);
    
    switch (token.type) {
        case TOKEN_NUMBER:
            advance_token(p);
            return create_node_constant(atoi(token.value));
            
        case TOKEN_VARIABLE:
            advance_token(p);
            if (strlen(token.value) != 1) {
                printf("Error: Variables must be single characters (a, b, c, d). Invalid variable: '%s'.\n", token.value);
                exit(1);
            }
            return create_node_variable(token.value[0]);
            
        case TOKEN_FUNCTION:
            return parse_function(p);
            
        case TOKEN_LPAREN:
            advance_token(p);
            ExprNode* expr = parse_expression(p);
            if (current_token(p).type != TOKEN_RPAREN) {
                printf("Error: Expected ')' after sub-expression. Found '%s'.\n", current_token(p).value);
                exit(1);
            }
            advance_token(p);
            return expr;
            
        default:
            printf("Error: Unexpected token '%s' (type: %d) in factor.\n", token.value, token.type);
            exit(1);
    }
}

ExprNode* parse_term(Parser* p) {
    ExprNode* left = parse_factor(p);
    
    while (current_token(p).type == TOKEN_OPERATOR) {
        Token op_token = current_token(p);
        if (op_token.value[0] == '*' || op_token.value[0] == '/') {
            advance_token(p);
            ExprNode* right = parse_factor(p);
            left = create_node_operator(op_type(op_token.value[0]), left, right);
        } else {
            break;
        }
    }
    
    return left;
}

ExprNode* parse_expression(Parser* p) {
    ExprNode* left = parse_term(p);
    
    while (current_token(p).type == TOKEN_OPERATOR) {
        Token op_token = current_token(p);
        if (op_token.value[0] == '+' || op_token.value[0] == '-') {
            advance_token(p);
            ExprNode* right = parse_term(p);
            left = create_node_operator(op_type(op_token.value[0]), left, right);
        } else {
            break;
        }
    }
    
    return left;
}

ExprNode* parse(const char* expression) {
    Token tokens[100];
    int token_count = tokenize(expression, tokens);
    
    Parser parser = { tokens, token_count, 0 };
    ExprNode* ast = parse_expression(&parser);

    if (parser.pos < parser.count - 1) {
        printf("Error: Unconsumed tokens at end of expression: '%s'. Check for syntax errors.\n", current_token(&parser).value);
        exit(1);
    }
    return ast;
}

// --- Evaluation ---

int evaluate(ExprNode* node, int a, int b, int c, int d) {
    if (!node) return 0;
    
    switch (node->type) {
        case NODE_VARIABLE:
            switch (node->var_name) {
                case 'a': return a;
                case 'b': return b;
                case 'c': return c;
                case 'd': return d;
                default:
                    printf("Error: Unknown variable: '%c'.\n", node->var_name);
                    exit(1);
            }
            
        case NODE_CONSTANT:
            return node->constant;
            
        case NODE_OPERATOR: {
            int left_val = evaluate(node->operation.left, a, b, c, d);
            int right_val = evaluate(node->operation.right, a, b, c, d);
            
            switch (node->operation.op) {
                case OP_ADD: return left_val + right_val;
                case OP_SUB: return subtract(left_val, right_val);
                case OP_MUL: return multiply(left_val, right_val);
                case OP_DIV: 
                    if (right_val == 0) {
                        printf("Error: Division by zero.\n");
                        exit(1);
                    }
                    return divide_signed(left_val, right_val);
                default: return 0;
            }
        }
        
        case NODE_FUNCTION: {
            int args[3];
            for (int i = 0; i < node->function.argc; i++) {
                args[i] = evaluate(node->function.args[i], a, b, c, d);
            }
            
            switch (node->function.func) {
                case FUNC_MAX: 
                    return max(args[0], args[1]);
                case FUNC_MIN: 
                    return min(args[0], args[1]);
                case FUNC_EQUAL: 
                    return equal(args[0], args[1]);
                case FUNC_GREATER_THAN: 
                    return greater_than(args[0], args[1]);
                case FUNC_IFELSE: 
                    return ifelse(args[0], args[1], args[2] != 0);
                case FUNC_ABSOLUTE: 
                    return absolute(args[0]);
                default: return 0;
            }
        }
        default: return 0;
    }
}

// --- Memory Management ---

void free_tree(ExprNode* node) {
    if (!node) return;
    
    if (node->type == NODE_OPERATOR) {
        free_tree(node->operation.left);
        free_tree(node->operation.right);
    } else if (node->type == NODE_FUNCTION) {
        for (int i = 0; i < node->function.argc; i++) {
            free_tree(node->function.args[i]);
        }
    }
    
    free(node);
}

// --- Main Program ---

// Helper function to read an integer safely
int read_int_input(const char* prompt) {
    char buffer[100];
    int value;
    while (1) {
        printf("%s", prompt);
        if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
            printf("Error reading input. Exiting.\n");
            exit(1);
        }
        if (sscanf(buffer, "%d", &value) == 1) {
            return value;
        } else {
            printf("Invalid input. Please enter an integer.\n");
        }
    }
}

void print_usage() {
    printf("MPC Expression Interpreter\n");
    printf("Available variables: a, b, c, d (single character)\n");
    printf("Available functions: max(x, y), min(x, y), equal(x, y), greater_than(x, y), ifelse(condition, true_val, false_val), absolute(x)\n");
    printf("Available operators: +, -, *, /\n");
    printf("Example: max(a * b, c + 5)\n");
    printf("Enter 'quit' to exit\n\n");
}

int main() {
    print_usage();
    
    char input[256];
    int a, b, c, d;
    
    a = read_int_input("Enter value for a: ");
    b = read_int_input("Enter value for b: ");
    c = read_int_input("Enter value for c: ");
    d = read_int_input("Enter value for d: ");
    
    printf("\nVariables: a=%d, b=%d, c=%d, d=%d\n\n", a, b, c, d);
    
    while (1) {
        printf("Enter expression: ");
        if (fgets(input, sizeof(input), stdin) == NULL) {
            printf("Error reading input. Exiting.\n");
            break;
        }
        
        input[strcspn(input, "\n")] = 0;
        
        if (strcmp(input, "quit") == 0) {
            break;
        }
        
        if (input[0] == '\0') {
            printf("Empty expression. Please enter a valid expression or 'quit'.\n\n");
            continue;
        }

        ExprNode* ast = NULL;
        int result = 0;
        
        printf("Parsing and evaluating...\n");
        ast = parse(input);
        result = evaluate(ast, a, b, c, d);
        
        printf("Result: %d\n\n", result);
        
        free_tree(ast);
    }
    
    printf("Goodbye!\n");
    return 0;
}
