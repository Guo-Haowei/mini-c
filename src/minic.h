#ifndef __MINIC_H__
#define __MINIC_H__
#include <assert.h>

#include "list.h"

typedef int bool;

#ifndef false
#define false 0
#endif

#ifndef true
#define true 1
#endif

#ifndef nullptr
#define nullptr ((void*)0)
#endif

#define unreachable() assert(0)

// utilities
#define ARRAY_COUNTER(arr) (sizeof(arr) / sizeof(*(arr)))
#define STATIC_ASSERT(COND) typedef char _static_assertion_[(COND) ? 1 : -1]
#define assertindex(a, bound) assert(((int)a >= 0) && ((int)a < (int)bound))

typedef enum token_kind_t {
    TK_IDENT, // Identifiers
    TK_PUNCT, // Punctuators
    TK_NUM,   // Numeric literals
    TK_EOF,   // End-of-file markers
} TokenKind;

typedef enum node_kind_t {
#define DEFINE_NODE(NAME) NAME,
#include "node.inl"
#undef DEFINE_NODE
    ND_COUNT,
} NodeKind;

typedef struct source_info_t {
    char const* file;
    char const* start;
    char const* end;
    int len;
} SourceInfo;

typedef struct token_t {
    TokenKind eTokenKind;
    int line;
    int col;
    char const* start;
    char const* end;
    int len;

    SourceInfo const* sourceInfo;
} Token;

typedef struct node_t {
    NodeKind eNodeKind;
    struct node_t* next;
    struct node_t* lhs;
    struct node_t* rhs;
    char name; // Used if kind == ND_VAR
    int val;   // Used if kind == ND_NUM

    // flags
    int isBinary;
    int isUnary;
} Node;

typedef struct lexer_t {
    SourceInfo const* sourceInfo;
    char const* p;
    int line;
    int col;
} Lexer;

int token_as_int(Token const* tok);

List* lex(SourceInfo const* sourceInfo);
Node* parse(List* toks);
void gen(Node const* node);

void error_at_lexer(Lexer const* lexer, char const* const fmt, ...);
void error_at_token(Token const* token, char const* const fmt, ...);

// DEBUG
char const* token_kind_to_string(TokenKind eTokenKind);
char const* node_kind_to_string(NodeKind eNodeKind);

void debug_print_token(Token const* tok);
void debug_print_tokens(List const* toks);
void debug_print_node(Node const* node);

// @TODO: implement
void debug_validate_node(Node const* node);

#endif
