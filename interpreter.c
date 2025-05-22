#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdint.h>

// --- MPC-Safe Arithmetic Functions ---

int subtract(int a, int b) {
    unsigned int ua = (unsigned int)a;
    unsigned int ub = (unsigned int)b;
    unsigned int result = 0;
    unsigned int borrow = 0;

    for (int i = 0; i < 32; i++) {
        unsigned int ai = (ua >> i) & 1;
        unsigned int bi = (ub >> i) & 1;
        unsigned int di = ai ^ bi ^ borrow;
        result |= (di << i);
        borrow = ((~ai & bi) | (~(ai ^ bi) & borrow)) & 1;
    }
    
    return (int)result;
}

int multiply(int a, int b) {
    unsigned int ua = (unsigned int)(a < 0 ? -a : a);
    unsigned int ub = (unsigned int)(b < 0 ? -b : b);
    unsigned int result = 0;

    for (int i = 0; i < 32; i++) {
        result += (-((ub >> i) & 1) & (ua << i));
    }

    int sign = ((a >> 31) ^ (b >> 31));
    return (result ^ -sign) + sign;
}

uint32_t mpc_divide_unsigned(uint32_t numerator, uint32_t denominator) {
    if (denominator == 0) return 0; // Handle division by zero
    
    uint32_t result = 0;
    uint32_t remainder = numerator;

    for (int i = 31; i >= 0; i--) {
        uint32_t shifted_denom = denominator << i;
        int32_t diff = (int32_t)(remainder - shifted_denom);
        uint32_t ge = ((diff >> 31) & 1) ^ 1;
        remainder = remainder - (shifted_denom * ge);
        result = result | (ge << i);
    }

    return result;
}

int divide_signed(int a, int b) {
    if (b == 0) return 0;
    
    uint32_t ua = (uint32_t)(a < 0 ? -a : a);
    uint32_t ub = (uint32_t)(b < 0 ? -b : b);
    uint32_t result = mpc_divide_unsigned(ua, ub);
    
    int sign = ((a >> 31) ^ (b >> 31));
    return ((int)result ^ -sign) + sign;
}

int absolute(int a) {
    int mask = a >> 31;
    return (a + mask) ^ mask;
}

bool greater_than(int a, int b) {
    int diff = subtract(a, b);
    int sign_bit = (diff >> 31) & 1;
    int neg_diff = ~diff + 1;
    int non_zero = ((diff | neg_diff) >> 31) & 1;
    int result = (~sign_bit) & non_zero;
    return result != 0;
}

bool equal(int a, int b) {
    int diff = subtract(a, b);
    int neg_diff = ~diff + 1;
    int non_zero = ((diff | neg_diff) >> 31) & 1;
    return non_zero == 0;
}

int ifelse(int a, int b, bool cond) {
    return multiply(a, cond) + multiply(b, subtract(1, cond));
}

int max(int a, int b) {
    bool cond = greater_than(a, b);
    return ifelse(a, b, cond);
}

int min(int a, int b) {
    bool cond = greater_than(b, a);
    return ifelse(a, b, cond);
}

// --- Expression Tree Definitions ---

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

// --- Tokenizer ---

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
            if (expr[pos] == '-') pos++; // Handle negative numbers
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
        }
    }
    
    tokens[count++] = (Token){TOKEN_EOF, ""};
    return count;
}

// --- Node Creation Functions ---

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

// --- Parser Functions ---

OperatorType op_type(char op) {
    switch (op) {
        case '+': return OP_ADD;
        case '-': return OP_SUB;
        case '*': return OP_MUL;
        case '/': return OP_DIV;
        default: 
            printf("Unknown operator: %c\n", op);
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
    printf("Unknown function: %s\n", name);
    exit(1);
}

// Forward declarations
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
    advance_token(p); // consume function name
    
    if (current_token(p).type != TOKEN_LPAREN) {
        printf("Expected '(' after function name\n");
        exit(1);
    }
    advance_token(p); // consume '('
    
    ExprNode* args[3];
    int argc = 0;
    
    if (current_token(p).type != TOKEN_RPAREN) {
        args[argc++] = parse_expression(p);
        
        while (current_token(p).type == TOKEN_COMMA) {
            advance_token(p); // consume ','
            if (argc >= 3) {
                printf("Too many function arguments\n");
                exit(1);
            }
            args[argc++] = parse_expression(p);
        }
    }
    
    if (current_token(p).type != TOKEN_RPAREN) {
        printf("Expected ')' after function arguments\n");
        exit(1);
    }
    advance_token(p); // consume ')'
    
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
                printf("Variables must be single characters. Invalid variable: %s\n", token.value);
                exit(1);
            }
            return create_node_variable(token.value[0]);
            
        case TOKEN_FUNCTION:
            return parse_function(p);
            
        case TOKEN_LPAREN:
            advance_token(p); // consume '('
            ExprNode* expr = parse_expression(p);
            if (current_token(p).type != TOKEN_RPAREN) {
                printf("Expected ')'\n");
                exit(1);
            }
            advance_token(p); // consume ')'
            return expr;
            
        default:
            printf("Unexpected token in factor: %s (type: %d)\n", token.value, token.type);
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
    return parse_expression(&parser);
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
                    printf("Unknown variable: %c\n", node->var_name);
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
                case OP_DIV: return divide_signed(left_val, right_val);
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
                    return ifelse(args[0], args[1], args[2]);
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

void print_usage() {
    printf("MPC Expression Interpreter\n");
    printf("Available variables: a, b, c, d\n");
    printf("Available functions: max, min, equal, greater_than, ifelse, absolute\n");
    printf("Available operators: +, -, *, /\n");
    printf("Example: max(a * b, c + 5)\n");
    printf("Enter 'quit' to exit\n\n");
}

int main() {
    print_usage();
    
    char input[256];
    int a = 0, b = 0, c = 0, d = 0;
    
    // Get variable values
    printf("Enter value for a: ");
    scanf("%d", &a);
    printf("Enter value for b: ");
    scanf("%d", &b);
    printf("Enter value for c: ");
    scanf("%d", &c);
    printf("Enter value for d: ");
    scanf("%d", &d);
    
    printf("\nVariables: a=%d, b=%d, c=%d, d=%d\n\n", a, b, c, d);
    
    while (1) {
        printf("Enter expression: ");
        fgetc(stdin); // consume newline from previous scanf
        if (fgets(input, sizeof(input), stdin) == NULL) break;
        
        // Remove newline if present
        input[strcspn(input, "\n")] = 0;
        
        if (strcmp(input, "quit") == 0) break;
        
        ExprNode* ast = parse(input);
        int result = evaluate(ast, a, b, c, d);
        
        printf("Result: %d\n\n", result);
        
        free_tree(ast);
    }
    
    printf("Goodbye!\n");
    return 0;
}
