#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

typedef enum {
    INT,
    KEYWORD,
    OPERATOR,
    SEPARATOR,
    STRING,
    COMMENT, // New token type for comments
} TokenType;

typedef struct {
    TokenType type;
    char *value;
} Token;

void print_token(Token token) {
    printf("Token Value: %s\n", token.value);
    switch (token.type) {
        case INT:
            printf("Type: INT\n");
            break;
        case KEYWORD:
            printf("Type: KEYWORD\n");
            break;
        case OPERATOR:
            printf("Type: OPERATOR\n");
            break;
        case SEPARATOR:
            printf("Type: SEPARATOR\n");
            break;
        case STRING:
            printf("Type: STRING\n");
            break;
        case COMMENT:
            printf("Type: COMMENT\n");
            break;
    }
}

// Function to generate number tokens.
Token *generate_number(char *current, int *current_index) {
    Token *token = malloc(sizeof(Token));
    token->type = INT;
    char *value = malloc(sizeof(char) * 8); // Assuming a number won't exceed 7 digits.
    int value_index = 0;

    // Parse digits.
    while (isdigit(current[*current_index]) && current[*current_index] != '\0') {
        value[value_index++] = current[*current_index];
        (*current_index)++; // Increment the dereferenced index pointer
    }

    value[value_index] = '\0'; // Null-terminate the string
    token->value = value;
    return token;
}

// Function to generate keyword tokens.
Token *generate_keyword(char *character, int *char_index) {
    Token *token = malloc(sizeof(Token));
    char *keyword = malloc(sizeof(char) * 100); // Increased size for longer keywords
    int keyword_index = 0;

    // Parse alphabetic characters.
    while (isalpha(character[*char_index]) && character[*char_index] != '\0') {
        keyword[keyword_index++] = character[*char_index];
        (*char_index)++; // Increment the dereferenced index pointer
    }

    keyword[keyword_index] = '\0'; // Null-terminate the string
    token->type = KEYWORD;
    token->value = strdup(keyword); // Copy the keyword
    free(keyword); // Free the allocated memory for the keyword
    return token;
}

// Function to generate string tokens.
Token *generate_string(char *current, int *current_index) {
    Token *token = malloc(sizeof(Token));
    token->type = STRING;
    char *value = malloc(sizeof(char) * 256); // Allocate memory for the string
    int value_index = 0;

    (*current_index)++; // Skip the opening quote

    // Parse characters until the closing quote or end of string.
    while (current[*current_index] != '"' && current[*current_index] != '\0') {
        if (current[*current_index] == '\\') { // Handle escape sequences
            (*current_index)++; // Skip the backslash
            if (current[*current_index] == 'n') {
                value[value_index++] = '\n';
            } else if (current[*current_index] == 't') {
                value[value_index++] = '\t';
            } else {
                value[value_index++] = current[*current_index]; // Copy the escaped character
            }
        } else {
            value[value_index++] = current[*current_index]; // Normal character
        }
        (*current_index)++;
    }

    if (current[*current_index] == '"') {
        (*current_index)++; // Skip the closing quote
    }

    value[value_index] = '\0'; // Null-terminate the string
    token->value = value;
    return token;
}

// Function to generate comment tokens.
Token *generate_comment(char *current, int *current_index) {
    Token *token = malloc(sizeof(Token));
    token->type = COMMENT;
    char *value = malloc(sizeof(char) * 256); // Allocate memory for the comment
    int value_index = 0;

    if (current[*current_index] == '/') {
        (*current_index)++; // Skip the first '/'
        if (current[*current_index] == '/') { // Single-line comment
            (*current_index)++; // Skip the second '/'
            while (current[*current_index] != '\n' && current[*current_index] != '\0') {
                value[value_index++] = current[*current_index++];
            }
            value[value_index] = '\0'; // Null-terminate the comment
        } else if (current[*current_index] == '*') { // Multi-line comment
            (*current_index)++; // Skip the '*'
            while (!(current[*current_index] == '*' && current[*current_index + 1] == '/') && current[*current_index] != '\0') {
                value[value_index++] = current[*current_index++];
            }
            if (current[*current_index] == '*') {
                current_index++; // Skip the '*'
                if (current[*current_index] == '/') {
                    current_index++; // Skip the '/'
                }
            }
            value[value_index] = '\0'; // Null-terminate the comment
        }
    }

    token->value = value;
    return token;
}

// Function to check if a string is in an array.
int is_in_array(const char *str, const char *array[], int size) {
    for (int i = 0; i < size; i++) {
        if (strcmp(str, array[i]) == 0) {
            return 1; // Found in array
        }
    }
    return 0; // Not found
}

// Lexer function to tokenize the input file.
Token *lexer(FILE *file, size_t *token_count) {
    int length;
    char *buffer = 0;
    fseek(file, 0, SEEK_END);
    length = ftell(file);
    fseek(file, 0, SEEK_SET);
    buffer = malloc(sizeof(char) * (length + 1)); // Allocate memory for the buffer
    fread(buffer, 1, length, file);
    fclose(file);
    buffer[length] = '\0'; // Null-terminate the string
    char *current = buffer;
    int current_index = 0;

    size_t capacity = 12; // Initial capacity
    Token *tokens = malloc(sizeof(Token) * capacity);
    *token_count = 0;

    // Define arrays of keywords, operators, and separators
    const char *keywords[] = {"exit", "if", "else", "for", "while", "printf"};
    const char *operators[] = {"+", "-", "*", "/", "=", "==", "!="};
    const char *separators[] = {";", "(", ")", "{", "}"};

    // Loop through the buffer.
    while (current[current_index] != '\0') {
        // Skip white spaces
        if (isspace(current[current_index])) {
            current_index++;
            continue; // Go to the next character
        }

        if (*token_count >= capacity) {
            capacity *= 2; // Double the capacity
            tokens = realloc(tokens, sizeof(Token) * capacity);
        }

        // Check for comments
        if (current[current_index] == '/') {
            Token *comment_token = generate_comment(current, &current_index);
            tokens[(*token_count)++] = *comment_token;
            print_token(*comment_token);
            
            continue;
        }

        // Check for string literals
        if (current[current_index] == '"') {
            Token *string_token = generate_string(current, &current_index);
            tokens[(*token_count)++] = *string_token;
            print_token(*string_token);
            
        } else if (strchr("();{}", current[current_index])) {
            char separator[2] = {current[current_index], '\0'};
            Token *separator_token = malloc(sizeof(Token));
            separator_token->value = strdup(separator);
            separator_token->type = SEPARATOR;
            tokens[(*token_count)++] = *separator_token;
            print_token(*separator_token);
            
        } else if (isdigit(current[current_index])) {
            Token *number_token = generate_number(current, &current_index);
            tokens[(*token_count)++] = *number_token;
            print_token(*number_token);
            
            continue; // Skip incrementing current_index here to avoid skipping characters
        } else if (isalpha(current[current_index])) {
            Token *keyword_token = generate_keyword(current, &current_index);
            if (is_in_array(keyword_token->value, keywords, sizeof(keywords) / sizeof(keywords[0]))) {
                tokens[(*token_count)++] = *keyword_token;
                print_token(*keyword_token);
            } else {
                // Handle unknown keywords
                printf("Found Unknown Keyword: %s\n", keyword_token->value);
                tokens[(*token_count)++] = *keyword_token;
                print_token(*keyword_token);
            }
            
            continue; // Skip incrementing current_index here to avoid skipping characters
        } else if (strchr("+-*/=!", current[current_index])) {
            // Check for operators
            char operator[3] = {current[current_index], '\0', '\0'};
            if (current[current_index + 1] == '=') {
                operator[1] = '=';
                current_index++;
            }
            Token *operator_token = malloc(sizeof(Token));
            operator_token->value = strdup(operator);
            operator_token->type = OPERATOR;
            tokens[(*token_count)++] = *operator_token;
            print_token(*operator_token);
            
        } else {
            printf("Found Unknown Character: %c\n", current[current_index]);
        }

        current_index++;
    }

    free(buffer); // Free the allocated memory for the buffer
    return tokens;
}

// Main function.
int main() {
    FILE *file = fopen("test.unn", "r");
    if (file == NULL) {
        perror("Error opening file");
        return EXIT_FAILURE;
    }

    size_t token_count;
    Token *tokens = lexer(file, &token_count);
    
    // Clean up allocated tokens
    for (size_t i = 0; i < token_count; i++) {
        free(tokens[i].value); // Free each token's value
    }
    free(tokens); // Free the allocated token array

    return 0;
}
