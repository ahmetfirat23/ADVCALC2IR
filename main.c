#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>


typedef enum {
    VAR,
    INT,
    OPEN_P,
    CLOSE_P,
    SUM,
    MULTI,
    DIV,
    MOD,
    MINUS,
    EQUAL,
    B_AND,
    B_OR,
    B_XOR,
    LS,
    RS,
    LR,
    RR,
    NOT,
    COMMA,
    EOL,
} token_type;

struct token {
    token_type token_type;
    char token_val[256];
    char register_name[16];
    struct token *next;
    struct token *prev;
};


//Reserved keywords and signs
char *KEYWORDS[] = {"xor", "ls", "rs", "lr", "rr", "not"};
char SIGNS[] = {'=', '+', '-', '*', '/', '%', '&', '|', '(', ')', ','};

/*
 * LOOKUP TABLE
 * VAR_KEYS holds variable names for the lookup table.
 * VARS holds variable values, and they share indices with VAR_KEYS
 * VAR_IDX holds next free index of the lookup table, must be updated when new var added
 * REG_IDX holds next free index of the LLVM register
 * LINE_IDX holds the current line index
 * */
char *VAR_KEYS[128];
long long VARS[128];
int VAR_IDX = 0;
int REG_IDX = 1;
int LINE_IDX = 1;
FILE *op;

/*
 * Check whether given char is a valid sign
 * Return 1 on sign, else 0
*/
int is_sign(char chr) {
    for (int i = 0; i < 11; i++) { //There are 11 signs
        if (chr == SIGNS[i]) {
            return 1;
        }
    }
    return 0;
}

/*
 * Check given string is a keyword
 * Return 1 on keyword, else 0
 */
int is_keyword(char *word) {
    for (int i = 0; i < 6; i++) { //There are 6 keywords
        if (strcmp(word, KEYWORDS[i]) == 0) {
            return 1;
        }
    }
    return 0;
}


/*
 * These functions will receive an iterator pointer when their corresponding type first encountered in lexer.
 * They will read from pointer until an irrelevant char been encountered, and then they will return the token or throw error.
 * Irrelevance is defined for each type since it differs which type can follow which type.
 * Pointer's current position is updated in these functions since it is used in the loop of lexer.
 * */

/*
 * Return token of type function or variable
 * */
struct token func_and_var_parser(char **exp) {
    struct token token;
    char str[256 + 1] = "";
    int idx = 0;
    while (isalpha(**exp)) {
        str[idx] = **exp;
        idx++;
        (*exp)++;
    }
    str[idx] = '\0';
    strcpy(token.token_val, str);
    if (is_keyword(str) == 1) {
        if (strcmp(str, "xor") == 0) {
            token.token_type = B_XOR;
        } else if (strcmp(str, "ls") == 0) {
            token.token_type = LS;
        } else if (strcmp(str, "rs") == 0) {
            token.token_type = RS;
        } else if (strcmp(str, "lr") == 0) {
            token.token_type = LR;
        } else if (strcmp(str, "rr") == 0) {
            token.token_type = RR;
        } else if (strcmp(str, "not") == 0) {
            token.token_type = NOT;
        }
    } else {
        token.token_type = VAR;
    }
    return token;
}

/*
 * Returns token of type integer
 * */
struct token int_parser(char **exp) {
    struct token token;
    char num[256 + 1] = "";
    int idx = 0;
    while (isdigit(**exp)) {
        num[idx] = **exp;
        idx++;
        (*exp)++;
    }
    num[idx] = '\0';
    strcpy(token.token_val, num);
    token.token_type = INT;
    return token;
}

/*
 * Returns token of type operator, parenthesis, comma or equal sign
 * */
struct token sign_parser(char **exp) {
    struct token token;
    if (**exp == '=') {
        token.token_type = EQUAL;
    } else if (**exp == '+') {
        token.token_type = SUM;
    } else if (**exp == '-') {
        token.token_type = MINUS;
    } else if (**exp == '*') {
        token.token_type = MULTI;
    } else if (**exp == '/') {
        token.token_type = DIV;
    } else if (**exp == '%') {
        token.token_type = MOD;
    } else if (**exp == '&') {
        token.token_type = B_AND;
    } else if (**exp == '|') {
        token.token_type = B_OR;
    } else if (**exp == '(') {
        token.token_type = OPEN_P;
    } else if (**exp == ')') {
        token.token_type = CLOSE_P;
    } else if (**exp == ',') {
        token.token_type = COMMA;
    }
    strcpy(token.token_val, (char[2]) {**exp, '\0'});
    (*exp)++;
    return token;
}

/*
 * Return end of line token
 * */
struct token eol_parser() {
    struct token token;
    token.token_type = EOL;
    strcpy(token.token_val, "<EOL>");
    return token;
}

/*
 * Perform lexical analysis
 * Loop through chars and call appropriate parser to tokenize until new line or comment char found
 * Assign first token to head of token linked list, eol token to tail
 * If equal sign found assign to p_equal pointer
 * Pointer params must be initialized to NULL before given to function
 * Return 0 on success, -1 on error
 * */
int lexer(char *p, int length, struct token **head, struct token **tail, struct token **p_equal) {
    int idx = 0;
    struct token *prev_token;
    for (int i = 0; i < length; i++) {
        struct token *token = malloc(sizeof(struct token));
        if (*p == '\n') {
            (*token) = eol_parser();
            (*tail) = token;
            if ((*head) == NULL) {
                (*head) = token;
                (*head)->next = NULL;
                (*head)->prev = NULL;
            } else {
                prev_token->next = (*tail);
                (*tail)->prev = prev_token;
            }
            break;
        } else if (isspace(*p)) {
            p += 1;
            continue;
        } else if (isalpha(*p)) {
            (*token) = func_and_var_parser(&p);
        } else if (isdigit(*p)) {
            (*token) = int_parser(&p);
        } else if (is_sign(*p)) {
            (*token) = sign_parser(&p);

            if (*(p - 1) == '=') {
                if ((*p_equal) == NULL) {
                    (*p_equal) = token;
                } else {
                    return -1;
                }
            }
        } else {
            return -1;
        }
        if ((*head) == NULL) {
            (*head) = token;
            (*head)->prev = NULL;
        } else {
            prev_token->next = token;
            (*token).prev = prev_token;
        }
        prev_token = token;
        idx++;
    }
    return 0;
}

/*
 * Check whether given expression is valid
 * Rules for each token explicitly defined in the function
 * Function tokens matched with comma's, open parentheses matched with closed ones
 * * Return 0 on valid, else -1
 */
int exp_syntax_checker(struct token *head) {
    struct token *iter = head;
    int func_count = 0;//Count of unmatched functions by commas
    int p_count = 0;//Count of unmatched open parentheses by closed parentheses
    int limiting_p_count = 0; //Count of unmatched parentheses when the current function seen. Unless the function exited p_count can not be less than or equal to this limit. When exiting the current function this number is one less than p_count
    int limiting_func_count = 0; //Count of unmatched functions when the current function seen. Current function is also counted towards this number. Unless the function exited func_count can not be less than this limit minus one. When the current function matched with a comma this value is one bigger than func_count

    int func_stack[512];//Current limiting elements and previous elements are hold
    int stack_idx = 0;//Points empty stack position


    while (iter->token_type != EOL) {
        if (p_count < 0 || func_count < 0) {
            return -1;
        }

        //Check commas match functions in recursive functions
        if(stack_idx>=2 && iter->token_type == CLOSE_P){ //Current limiting values are held at the top of the stack.
            if(limiting_p_count + 1 == p_count){
                if(limiting_func_count == func_count + 1){
                    if (stack_idx>=4){//If there is a  previous value in the stack then index must be >= 4
                        stack_idx-=2; //Pop the current element from stack
                        limiting_func_count = func_stack[--stack_idx]; //Update limiting values
                        limiting_p_count = func_stack[--stack_idx];
                        stack_idx+=2; //Point to empty slot
                    }
                    else{//If there is only current element in the stack, recursion is ended
                        stack_idx = 0; //Empty out the stack
                    }
                }
                else{
                    return -1;
                }
            }
        }

        token_type type = iter->token_type;
        token_type next_type = iter->next->token_type;
        if (iter->prev == NULL || iter->prev->token_type == EQUAL) {
            if (type == VAR || type == INT) {
                if (next_type == SUM || next_type == MULTI ||next_type == DIV ||next_type == MOD
                    || next_type == MINUS || next_type == B_AND || next_type == B_OR || next_type == EOL) {
                    iter = iter->next;
                    continue;
                } else {
                    return -1;
                }
            } else if (type == OPEN_P) {
                p_count++;
                if (next_type == VAR || next_type == INT || next_type == OPEN_P || next_type == LS
                    || next_type == RS || next_type == LR || next_type == RR || next_type == NOT) {
                    iter = iter->next;
                    continue;
                } else {
                    return -1;
                }
            } else if (type == B_XOR || type == LS || type == RS || type == LR || type == RR || type == NOT) {
                if (type != NOT) {
                    func_count++;
                    limiting_p_count = p_count;
                    limiting_func_count = func_count;
                    func_stack[stack_idx++] = limiting_p_count;
                    func_stack[stack_idx++] = limiting_func_count;
                }
                if (next_type == OPEN_P) {
                    iter = iter->next;
                    continue;
                } else {
                    return -1;
                }
            } else {
                return -1;
            }
        } else {
            if (type == VAR || type == INT) {
                if (next_type == CLOSE_P || next_type == SUM || next_type == MULTI || next_type == DIV
                    || next_type == MOD || next_type == MINUS || next_type == B_AND || next_type == B_OR
                    || next_type == COMMA || next_type == EOL) {
                    iter = iter->next;
                    continue;
                } else {
                    return -1;
                }
            } else if (type == OPEN_P) {
                p_count++;
                if (next_type == VAR || next_type == INT || next_type == OPEN_P || next_type == LS
                    || next_type == B_XOR || next_type == RS || next_type == LR || next_type == RR
                    || next_type == NOT) {
                    iter = iter->next;
                    continue;
                } else {
                    return -1;
                }
            } else if (type == CLOSE_P) {
                p_count--;
                if (next_type == CLOSE_P || next_type == SUM || next_type == MULTI ||next_type == DIV
                    || next_type == MOD || next_type == MINUS || next_type == B_AND || next_type == B_OR
                    || next_type == COMMA || next_type == EOL) {
                    iter = iter->next;
                    continue;
                } else {
                    return -1;
                }
            } else if (type == SUM || type == MULTI || type == DIV || type == MOD
                    || type == MINUS || type == B_AND || type == B_OR || type == COMMA) {
                if (type == COMMA) {
                    func_count--;
                    if(limiting_p_count + 1 != p_count){ //Seen different place than function scope
                        return -1;
                    }
                }
                if (next_type == VAR || next_type == INT || next_type == OPEN_P || next_type == B_XOR
                    || next_type == LS || next_type == RS || next_type == LR || next_type == RR
                    || next_type == NOT) {
                    iter = iter->next;
                    continue;
                } else {
                    return -1;
                }
            } else if (type == B_XOR || type == LS || type == RS || type == LR || type == RR || type == NOT) {
                if (type != NOT) {
                    func_count++;
                    limiting_p_count = p_count;
                    limiting_func_count = func_count;
                    func_stack[stack_idx++] = limiting_p_count;
                    func_stack[stack_idx++] = limiting_func_count;
                }
                if (next_type == OPEN_P) {
                    iter = iter->next;
                    continue;
                } else {
                    return -1;
                }
            } else {
                return -1;
            }
        }

    }
    if (p_count != 0 || func_count != 0) {
        return -1;
    }
    return 0;
}

/*
 * Check whether given assignment is valid
 * Rules for validity explicitly defined in function
 * Return 0 on valid, else -1
 * */
int assign_syntax_checker(struct token *head, struct token *p_equal) {
    if (head->token_type != VAR) {
        return -1;
    }
    if (head->next != p_equal) {
        return -1;
    }
    if (p_equal->next->token_type == EOL) {
        return -1;
    } else {
        return exp_syntax_checker(p_equal->next);
    }
}

/*
 * Change comma tokens into corresponding functions
 * Change variables into corresponding integers
 * Return 0 on success, else -1
 * */
int reformat_token_list(struct token **head) {
    int idx = 0;
    struct token *iter = *head;
    struct token *stack[256];
    while (iter->token_type != EOL) {
        token_type type = iter->token_type;
        if (type == B_XOR || type == LS || type == RS || type == LR || type == RR) {
            stack[idx] = iter;
            idx++;
        } else if (type == COMMA) {
            struct token *func = stack[--idx];
            struct token *tmp = func->prev;
            if (tmp != NULL) {
                func->prev->next = func->next;
            } else {
                *head = func->next;
            }
            func->next->prev = tmp;
            iter->token_type = func->token_type;
            strcpy(iter->token_val, func->token_val);
        } else if (type == VAR) {
            int i = 0;
            iter->token_type = INT;
            while (i < VAR_IDX) {
                if (strcmp(iter->token_val, VAR_KEYS[i]) == 0) {
                    sprintf(iter->token_val, "%lld", VARS[i]);
                    char reg_name[256];
                    sprintf(reg_name, "%%reg%d",REG_IDX); //Save register name for further operations
                    strcpy(iter->register_name, reg_name);
                    fprintf(op,"\t%s = load i32, i32* %%%s\n", reg_name, VAR_KEYS[i]);
                    REG_IDX++;
                    break;
                }
                i++;
            }
            if (i == VAR_IDX) {
                return -1;
            }
        }
        iter = iter->next;
    }
    if (idx != 0) {
        return -1;
    }
    return 0;
}

/*
 * Take pointer to operation and the type of the operation,
 * check for operation type and apply a proper process,
 * assign the result to a token and gets rid of operation. e.g. (5 + 2 * 3) => (5 + 6) => (11)
 * */


void calculate_opr(struct token *opr, token_type type) {
    struct token *left_side = opr->prev;
    struct token *right_side = opr->next;

    char *left_register_name = (strstr(left_side->register_name, "%reg"))? left_side->register_name : left_side->token_val;
    char *right_register_name = (strstr(right_side->register_name, "%reg"))? right_side->register_name : right_side->token_val;
    char new_register_name[16];
    char new_register_nameR[16];

    switch (type) {
        case MULTI:
            sprintf(new_register_name, "%%reg%d", REG_IDX);
            REG_IDX++;
            fprintf(op,"\t%s = mul i32 %s, %s\n", new_register_name, left_register_name, right_register_name);
            break;

        case DIV:
            sprintf(new_register_name, "%%reg%d", REG_IDX);
            REG_IDX++;
            fprintf(op,"\t%s = sdiv i32 %s, %s\n", new_register_name, left_register_name, right_register_name);
            break;

        case MOD:
            sprintf(new_register_name, "%%reg%d", REG_IDX);
            REG_IDX++;
            fprintf(op,"\t%s = srem i32 %s, %s\n", new_register_name, left_register_name, right_register_name);
            break;

        case SUM:
            sprintf(new_register_name, "%%reg%d", REG_IDX);
            REG_IDX++;
            fprintf(op,"\t%s = add i32 %s, %s\n", new_register_name, left_register_name, right_register_name);
            break;

        case MINUS:
            sprintf(new_register_name, "%%reg%d", REG_IDX);
            REG_IDX++;
            fprintf(op,"\t%s = sub i32 %s, %s\n", new_register_name, left_register_name, right_register_name);
            break;

        case B_AND:
            sprintf(new_register_name, "%%reg%d", REG_IDX);
            REG_IDX++;
            fprintf(op,"\t%s = and i32 %s, %s\n", new_register_name, left_register_name, right_register_name);
            break;

        case B_OR:
            sprintf(new_register_name, "%%reg%d", REG_IDX);
            REG_IDX++;
            fprintf(op, "\t%s = or i32 %s, %s\n", new_register_name, left_register_name, right_register_name);
            break;

        case B_XOR:
            sprintf(new_register_name, "%%reg%d", REG_IDX);
            REG_IDX++;
            fprintf(op, "\t%s = xor i32 %s, %s\n", new_register_name, left_register_name, right_register_name);
            break;

        case LS:
            sprintf(new_register_name, "%%reg%d", REG_IDX);
            REG_IDX++;
            fprintf(op, "\t%s = shl i32 %s, %s\n", new_register_name, left_register_name, right_register_name);
            break;

        case RS:
            sprintf(new_register_name, "%%reg%d", REG_IDX);
            REG_IDX++;
            fprintf(op,"\t%s = ashr i32 %s, %s\n", new_register_name, left_register_name, right_register_name);
            break;

        case LR:

            sprintf(new_register_nameR, "%%reg%d", REG_IDX);
            REG_IDX++;
            fprintf(op, "\t%s = shl i32 %s, %s\n", new_register_nameR, left_register_name, right_register_name);
            char new_register_name2[16];
            sprintf(new_register_name2, "%%reg%d", REG_IDX);
            REG_IDX++;
            fprintf(op,"\t%s = sub i32 32, %s\n", new_register_name2, right_register_name);
            char new_register_name3[16];
            sprintf(new_register_name3, "%%reg%d", REG_IDX);
            REG_IDX++;
            fprintf(op,"\t%s = lshr i32 %s, %s\n", new_register_name3, left_register_name, new_register_name2);
            sprintf(new_register_name, "%%reg%d", REG_IDX);
            REG_IDX++;
            fprintf(op, "\t%s = or i32 %s, %s\n", new_register_name, new_register_nameR, new_register_name3);
            break;

        case RR:

            sprintf(new_register_nameR, "%%reg%d", REG_IDX);
            REG_IDX++;
            fprintf(op, "\t%s = lshr i32 %s, %s\n", new_register_nameR, left_register_name, right_register_name);
            char new_register_name5[16];
            sprintf(new_register_name5, "%%reg%d", REG_IDX);
            REG_IDX++;
            fprintf(op,"\t%s = sub i32 32, %s\n", new_register_name5, right_register_name);
            char new_register_name6[16];
            sprintf(new_register_name6, "%%reg%d", REG_IDX);
            REG_IDX++;
            fprintf(op,"\t%s = shl i32 %s, %s\n", new_register_name6, left_register_name, new_register_name5);
            sprintf(new_register_name, "%%reg%d", REG_IDX);
            REG_IDX++;
            fprintf(op, "\t%s = or i32 %s, %s\n", new_register_name, new_register_nameR, new_register_name6);
            break;

        default:
            break;
    }

    left_side->next = right_side->next;
    right_side->next->prev = left_side;
    strcpy(left_side->register_name, new_register_name);
}

/*
 * Take the head of token list, traverse it to fulfill operations with respect to
 * precedence.
 * Parentheses expressions handled with recursive calls, and all other operations
 * ( * , + , -, &, |, ...)  handled with while loops in order.
 * Use calculate_opr() to handle operations.
 * */
void calculate(struct token *head) {
    if (head->token_type == NOT) {
        head = head->next;
    }
    struct token *temp_head = head;

    //Loop for parentheses
    while (temp_head->token_type != CLOSE_P && temp_head->token_type != EOL) {
        if (temp_head->token_type == OPEN_P) {
            calculate(temp_head->next);
        }
        temp_head = temp_head->next;
    }

    //Loop for multiplication, division, modulus
    temp_head = head;
    while (temp_head->token_type != CLOSE_P && temp_head->token_type != EOL) {
        if (temp_head->token_type == MULTI) {
            calculate_opr(temp_head, MULTI);
        }
        if (temp_head->token_type == DIV) {
            calculate_opr(temp_head, DIV);
        }
        if (temp_head->token_type == MOD) {
            calculate_opr(temp_head, MOD);
        }
        temp_head = temp_head->next;
    }
    //Loop for addition and subtraction
    temp_head = head;

    while (temp_head->token_type != CLOSE_P && temp_head->token_type != EOL) {
        if (temp_head->token_type == SUM) {
            calculate_opr(temp_head, SUM);
        }
        if (temp_head->token_type == MINUS) {
            calculate_opr(temp_head, MINUS);
        }
        temp_head = temp_head->next;
    }

    //Loop for bitwise and
    temp_head = head;
    while (temp_head->token_type != CLOSE_P && temp_head->token_type != EOL) {
        if (temp_head->token_type == B_AND) {
            calculate_opr(temp_head, B_AND);
        }
        temp_head = temp_head->next;
    }

    //Loop for bitwise or
    temp_head = head;
    while (temp_head->token_type != CLOSE_P && temp_head->token_type != EOL) {
        if (temp_head->token_type == B_OR) {
            calculate_opr(temp_head, B_OR);
        }
        temp_head = temp_head->next;
    }

    //Loop for binary functions
    temp_head = head;
    while (temp_head->token_type != CLOSE_P && temp_head->token_type != EOL) {
        if (temp_head->token_type == B_XOR) {
            calculate_opr(temp_head, B_XOR);
        }
        if (temp_head->token_type == LS) {
            calculate_opr(temp_head, LS);
        }
        if (temp_head->token_type == RS) {
            calculate_opr(temp_head, RS);
        }
        if (temp_head->token_type == LR) {
            calculate_opr(temp_head, LR);
        }
        if (temp_head->token_type == RR) {
            calculate_opr(temp_head, RR);
        }
        temp_head = temp_head->next;
    }

    //Detect close parenthesis and check for NOT function
    if (head->next->token_type == CLOSE_P) {
        if (head->prev->prev != NULL && head->prev->prev->token_type == NOT) {
            char *register_name = (strstr(head->register_name, "%reg"))? head->register_name : head->token_val;
            char new_register_name[16];
            sprintf(new_register_name, "%%reg%d", REG_IDX);
            REG_IDX++;
            fprintf(op,"\t%s = xor i32 -1, %s\n", new_register_name, register_name);
            strcpy(head->register_name, new_register_name);
            if (head->prev->prev->prev == NULL) {
                head->prev->prev = NULL;
            } else {
                head->prev->prev->prev->next = head->prev;
                head->prev->prev = head->prev->prev->prev;
            }
        }

        strcpy(head->prev->register_name, head->register_name);
        strcpy(head->prev->token_val, head->token_val);
        head->prev->token_type = INT;

        head->prev->next = head->next->next;
        head->next->next->prev = head->prev;
    }

}

/*
 * Print tokenized linked list for debugging
 * */
void print_debug(struct token *head) {
    struct token *iter = head;
    while (iter->token_type != EOL) {
        fprintf(op,"\t%s ", iter->token_val);
        iter = iter->next;
    }
}

/*
 * Free memory kept by linked list
 * */
void free_ll(struct token *head) {
    if(head==NULL){
        return;
    }
    struct token *iter = head;
    struct token *freed;
    while (iter->token_type != EOL) {
        freed = iter;
        iter = iter->next;
        free(freed);
    }
}

int main(int argc, char* argv[]) {

    char in_name[64];
    char out_name[64];
    char* postfix;
    //Extract file name
    strcpy(in_name, argv[1]);
    postfix = strrchr(in_name, '.');
    strncpy(out_name, in_name, postfix - in_name);
    strcat(out_name, ".ll");

    FILE *fp;
    fp = fopen("file.adv","r");
    op = fopen("file.ll","w");
    fprintf(op,"; ModuleID = 'advcalc2ir'\n");
    fprintf(op,"declare i32 @printf(i8*, ...)\n");
    fprintf(op,"@print.str = constant [4 x i8] c\"%%d\\0A\\00\"\n\n");
    fprintf(op,"define i32 @main() {\n");

    int error_code = 0;
    int exit_code = 0;
    char line[256 + 1] = "";

    while (fgets(line, sizeof(line), fp)) {
        error_code = 0;
        if (line[strlen(line)-1]!='\n') { //Safety check
            strcat(line,"\n");
        }

        char *p = line;
        struct token *head = NULL;
        struct token *tail = NULL;
        struct token *p_equal = NULL;
        error_code = lexer(p, strlen(p), &head, &tail, &p_equal);
        if (error_code == 0) {
            if (p_equal != NULL) {
                error_code = assign_syntax_checker(head, p_equal);
                if (error_code == 0) {
                    error_code = reformat_token_list(&p_equal);
                }
            } else {
                error_code = exp_syntax_checker(head);
                if (error_code == 0) {
                    error_code = reformat_token_list(&head);
                }
            }
            if (error_code == 0) {
                if (head->token_type != EOL) {
                    if (p_equal != NULL) {

                        calculate(p_equal->next);
                        char *var_name = calloc(256, sizeof(char));
                        strcpy(var_name, p_equal->prev->token_val);

                        int declared = 0;
                        for (int i = 0; i < VAR_IDX; i++) {
                            if (strcmp(VAR_KEYS[i], var_name) == 0) {
                                VARS[i] = 1;
                                declared = 1;
                                break;
                            }
                        }
                        if (declared == 0) {
                            fprintf(op,"\t%%%s = alloca i32\n", var_name);
                            VARS[VAR_IDX] = 1;
                            VAR_KEYS[VAR_IDX] = var_name;
                            VAR_IDX++;
                        }
                        struct token *ptr = (p_equal->next->token_type == NOT)? p_equal->next->next: p_equal->next;
                        char *result = (strstr(ptr->register_name, "%reg"))? ptr->register_name: ptr->token_val;
                        fprintf(op,"\tstore i32 %s, i32* %%%s\n", result, var_name);
                    } else {
                        calculate(head);
                        struct token *ptr = (head->token_type == NOT)? head->next: head;
                        char *result = (strstr(ptr->register_name, "%reg"))? ptr->register_name: ptr->token_val;
                        fprintf(op,"\tcall i32 (i8*, ...) @printf(i8* getelementptr ([4 x i8], [4 x i8]* @print.str, i32 0, i32 0), i32 %s)\n", result);
                    }
                }
            } else {
                printf("Error on line %d!\n", LINE_IDX);
                exit_code = 1;
            }
            free_ll(head);
        } else {
            printf("Error on line %d!\n", LINE_IDX);
            exit_code = 1;
        }

        LINE_IDX++;
    }
    if(exit_code==0) {
        fprintf(op, "\n\tret i32 0\n}");
    }
    else{
        remove(out_name);
    }
}
