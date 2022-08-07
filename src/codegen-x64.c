#include "minic.h"

#include <stdio.h>

// @TODO: refactor
static int s_depth = 0;
static char* s_argreg[] = { "%rcx", "%rdx", "%r8", "%r9" };
static Function* s_current_fn;

static void push()
{
    printf("  push %%rax\n");
    s_depth++;
}

static void pop(char const* arg)
{
    printf("  pop %s\n", arg);
    s_depth--;
}

static void gen_expr(Node const* node);

// Compute the absolute address of a given node.
// It's an error if a given node does not reside in memory.
static void gen_addr(Node const* node)
{
    switch (node->eNodeKind) {
    case ND_VAR:
        printf("  lea %d(%%rbp), %%rax\n", node->var->offset);
        return;
    case ND_DEREF:
        gen_expr(node->lhs);
        return;
    default:
        break;
    }

    error_tok(node->tok, "not an lvalue");
}

static void gen_cmp_expr(NodeKind eNodeKind)
{
    static char const* const s_cmds[] = {
        "sete",
        "setne",
        "setl",
        "setle",
        "setg",
        "setge",
    };

    STATIC_ASSERT(ARRAY_COUNTER(s_cmds) == (ND_GE - ND_EQ + 1));

    int const index = eNodeKind - ND_EQ;
    assertindex(index, ARRAY_COUNTER(s_cmds));

    printf("  cmp %%rdi, %%rax\n");
    printf("  %s %%al\n", s_cmds[index]);
    printf("  movzb %%al, %%rax\n");
}

static void gen_expr(Node const* node)
{
    switch (node->eNodeKind) {
    case ND_NUM:
        printf("  mov $%d, %%rax\n", node->val);
        return;
    case ND_NEG:
        gen_expr(node->lhs);
        printf("  neg %%rax\n");
        return;
    case ND_VAR:
        gen_addr(node);
        printf("  mov (%%rax), %%rax\n");
        return;
    case ND_DEREF:
        gen_expr(node->lhs);
        printf("  mov (%%rax), %%rax\n");
        return;
    case ND_ADDR:
        gen_addr(node->lhs);
        return;
    case ND_ASSIGN:
        gen_addr(node->lhs);
        push();
        gen_expr(node->rhs);
        pop("%rdi");
        printf("  mov %%rax, (%%rdi)\n");
        return;
    case ND_FUNCCALL:
        Node* arg = node->args;
        for (int i = 0; i < node->argc; ++i, arg = arg->next) {
            gen_expr(arg);
            push();
        }
        assert(arg == nullptr);

        for (int i = node->argc - 1; i >= 0; --i) {
            pop(s_argreg[i]);
        }

        printf("  mov $0, %%rax\n");
        printf("  call %s\n", node->funcname);
        return;
    default:
        break;
    }

    gen_expr(node->rhs);
    push();
    gen_expr(node->lhs);
    pop("%rdi");

    switch (node->eNodeKind) {
    case ND_ADD:
        printf("  add %%rdi, %%rax\n");
        return;
    case ND_SUB:
        printf("  sub %%rdi, %%rax\n");
        return;
    case ND_MUL:
        printf("  imul %%rdi, %%rax\n");
        return;
    case ND_DIV:
        printf("  cqo\n");
        printf("  idiv %%rdi\n");
        return;
    case ND_EQ:
    case ND_NE:
    case ND_LT:
    case ND_LE:
    case ND_GT:
    case ND_GE:
        gen_cmp_expr(node->eNodeKind);
        return;
    default:
        break;
    }

    error_tok(node->tok, "invalid statement");
}

// TODO: refactor
static int label_counter()
{
    static int i = 1;
    return i++;
}

static void gen_stmt(Node const* node)
{
    switch (node->eNodeKind) {
    case ND_IF: {
        int const c = label_counter();
        gen_expr(node->cond);
        printf("  cmp $0, %%rax\n");
        printf("  je  .L.else.%d\n", c);
        gen_stmt(node->then);
        printf("  jmp .L.end.%d\n", c);
        printf(".L.else.%d:\n", c);
        if (node->els) {
            gen_stmt(node->els);
        }
        printf(".L.end.%d:\n", c);
        return;
    }
    case ND_EXPR_STMT:
        gen_expr(node->lhs);
        return;
    case ND_RETURN:
        gen_expr(node->lhs);
        printf("  jmp .L.return.%s\n", s_current_fn->name);
        return;
    case ND_BLOCK:
        for (Node* n = node->body; n; n = n->next) {
            gen_stmt(n);
        }
        return;
    case ND_FOR: {
        int const c = label_counter();
        if (node->init) {
            gen_stmt(node->init);
        }

        printf(".L.begin.%d:\n", c);

        if (node->cond) {
            gen_expr(node->cond);
            printf("  cmp $0, %%rax\n");
            printf("  je  .L.end.%d\n", c);
        }

        gen_stmt(node->then);

        if (node->inc) {
            gen_expr(node->inc);
        }
        printf("  jmp .L.begin.%d\n", c);
        printf(".L.end.%d:\n", c);
        return;
    }
    default:
        break;
    }

    error_tok(node->tok, "invalid statement");
}

// Assign offsets to local variables.
static void assign_lvar_offsets(Function* prog)
{
    for (Function* fn = prog; fn; fn = fn->next) {
        int offset = 0;
        for (Obj* var = fn->locals; var; var = var->next) {
            offset += 8;
            var->offset = -offset;
        }
        fn->stackSize = align_to(offset, 16);
    }
}

void gen(Function* prog)
{
    assign_lvar_offsets(prog);

    printf("  .text\n");

    for (Function* fn = prog; fn; fn = fn->next) {
        printf("  .globl %s\n", fn->name);
        printf("%s:\n", fn->name);
        s_current_fn = fn;
        // Prologue
        printf("  push %%rbp\n");
        printf("  mov %%rsp, %%rbp\n");
        printf("  sub $%d, %%rsp\n", fn->stackSize);
        // Emit code

        // Save passed-by-register arguments to the stack
        int i = 0;
        for (Obj* var = fn->params; var; var = var->next) {
            printf("  mov %s, %d(%%rbp)\n", s_argreg[i++], var->offset);
        }

        gen_stmt(fn->body);
        assert(s_depth == 0);
        // Epilogue
        printf(".L.return.%s:\n", fn->name);
        printf("  mov %%rbp, %%rsp\n");
        printf("  pop %%rbp\n");
        printf("  ret\n");
    }
}
