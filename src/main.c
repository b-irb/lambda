#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>

enum expr_type_t { VAR, FUNC, APP };

struct expr_func_t {
    char name;
    struct expr_t* body;
};

struct expr_app_t {
    struct expr_t* m;
    struct expr_t* n;
};

struct expr_t {
    enum expr_type_t type;
    union {
        char var;
        struct expr_func_t func;
        struct expr_app_t app;
    } value;
};

struct parser_ctx {
    char* buffer;
    size_t size;
    size_t offset;
};

void print_expr(struct expr_t*);
struct expr_t* parse_expr(struct parser_ctx*);
int parse_name(struct parser_ctx*, char*);
struct expr_t* parse_func(struct parser_ctx*);
struct expr_t* reduce_expr(struct expr_t*);
struct expr_t* apply(struct expr_t*);

char parser_advance(struct parser_ctx* ctx) { ++ctx->offset; }

int parser_read(struct parser_ctx* ctx, char* out) {
    char byte;

    while (1) {
        if (ctx->offset > ctx->size) {
            return -1;
        }
        byte = ctx->buffer[ctx->offset];
        switch (byte) {
            case ' ':
            case '\t':
            case '\n':
            case '\r':
                parser_advance(ctx);
            default:
                goto retn;
        }
    }

retn:
    *out = byte;
    return 0;
}

bool parser_match(struct parser_ctx* ctx, char p) {
    char byte;

    if (parser_read(ctx, &byte) < 0) {
        return false;
    }
    if (byte == p) {
        parser_advance(ctx);
        return true;
    }
    return false;
}

struct expr_t* create_var(char name) {
    struct expr_t* var;

    if (!(var = malloc(sizeof(*var)))) {
        return NULL;
    }

    var->type = VAR;
    var->value.var = name;
    return var;
}

struct expr_t* create_func(char name, struct expr_t* body) {
    struct expr_t* func;

    if (!(func = malloc(sizeof(*func)))) {
        return NULL;
    }

    func->type = FUNC;
    func->value.func.name = name;
    func->value.func.body = body;
    return func;
}

struct expr_t* create_app(struct expr_t* m, struct expr_t* n) {
    struct expr_t* app;

    if (!(app = malloc(sizeof(*app)))) {
        return NULL;
    }

    app->type = APP;
    app->value.app.m = m;
    app->value.app.n = n;
    return app;
}

void destroy_expr(struct expr_t* expr) {
    switch (expr->type) {
        case VAR:
            free(expr);
            break;
        case FUNC:
            destroy_expr(expr->value.func.body);
            free(expr);
            break;
        case APP:
            destroy_expr(expr->value.app.m);
            destroy_expr(expr->value.app.n);
            free(expr);
            break;
    }
}

void print_expr(struct expr_t* expr) {
    switch (expr->type) {
        case VAR:
            printf("%c", expr->value.var);
            break;
        case FUNC:
            printf("(%c.", expr->value.func.name);
            print_expr(expr->value.func.body);
            printf(")");
            break;
        case APP:
            print_expr(expr->value.app.m);
            print_expr(expr->value.app.n);
            break;
    }
}

char* read_file(char* fname, size_t* size) {
    int fd;
    char* buf;
    struct stat s;

    if ((fd = open(fname, O_RDONLY)) < 0) {
        perror("failed to open file");
        return NULL;
    }

    fstat(fd, &s);
    if ((buf = mmap(NULL, s.st_size, PROT_READ, MAP_PRIVATE, fd, 0)) ==
        MAP_FAILED) {
        perror("unable to map file into memory");
        return NULL;
    }
    *size = s.st_size;
    return buf;
}

int parse_name(struct parser_ctx* ctx, char* name) {
    char byte;

    if (parser_read(ctx, &byte) < 0 || !isalpha(byte)) {
        return -1;
    }
    parser_advance(ctx);
    *name = byte;
    return 0;
}

struct expr_t* parse_var(struct parser_ctx* ctx) {
    char name;
    char* err_str;

    if (parse_name(ctx, &name) < 0) {
        err_str = "failed to parse name";
        goto error;
    }

    return create_var(name);
error:
    fprintf(stderr, "[var] %s\n", err_str);
    return NULL;
}

struct expr_t* parse_func(struct parser_ctx* ctx) {
    char byte;
    char name;
    char* err_str;
    struct expr_t* body;

    if (!parser_match(ctx, '(')) {
        err_str = "failed to match '('";
        goto error;
    }

    if (parse_name(ctx, &name) < 0) {
        err_str = "failed to parse name";
        goto error;
    }

    if (!parser_match(ctx, '.')) {
        err_str = "failed to match'.'";
        goto error;
    }

    if (!(body = parse_expr(ctx))) {
        err_str = "failed to parse body";
        goto error;
    }

    if (!parser_match(ctx, ')')) {
        err_str = "failed to match ')'";
        goto error;
    }

    return create_func(name, body);
error:
    fprintf(stderr, "[func] %s\n", err_str);
    return NULL;
}

struct expr_t* parse_expr_internal(struct parser_ctx* ctx) {
    char byte;
    char* err_str;
    struct expr_t* expr;

    if (parser_read(ctx, &byte) < 0) {
        err_str = "at end of byffer";
        goto error;
    }

#ifdef DEBUG
    fprintf(stderr, "[expr] consuming '%c'\n", byte);
#endif

    switch (byte) {
        case '(':
            if (!(expr = parse_func(ctx))) {
                err_str = "failed to parse function";
                goto error;
            }
            break;
        case ')':
            return NULL;
        default:
            if (!(expr = parse_var(ctx))) {
                err_str = "failed to parse variable";
                goto error;
            }
            break;
    }

    return expr;
error:
    fprintf(stderr, "[expr] %s\n", err_str);
    return NULL;
}

struct expr_t* parse_expr(struct parser_ctx* ctx) {
    char* err_str;
    struct expr_t* m;
    struct expr_t* n;

    if (!(m = parse_expr_internal(ctx))) {
        err_str = "unspecified";
        goto error;
    }

    if (!(n = parse_expr_internal(ctx))) {
        return m;
        err_str = "failed to parse second expr";
        goto error;
    }

    if (!(m = create_app(m, n))) {
        err_str = "failed to build basis";
        goto error;
    }

    while ((n = parse_expr_internal(ctx))) {
        if (!(m = create_app(m, n))) {
            err_str = "creating app failed";
            goto error;
        }
    }

    return m;
error:
    fprintf(stderr, "[app] %s\n", err_str);
    return NULL;
}

struct expr_t* parse_buffer(char* buf, size_t size) {
    /* Grammar
     * expr = [var] | [func] | [app]
     * var  = [a-Z]
     * app  = [expr] [expr]
     * func = ( [var] . [expr] )
     *
     * funcapp = [func] [func]
     * code = [funcapp]
     */
    struct parser_ctx ctx = {.buffer = buf, .size = size, .offset = 0};
    return parse_expr(&ctx);
}

struct expr_t* substitute(struct expr_t* expr, char target, struct expr_t* x) {
    struct expr_t* tmp;

    switch (expr->type) {
        case VAR:
            if (expr->value.var == target) {
                expr = x;
            }
            break;
        case FUNC:
            tmp = substitute(expr->value.func.body, target, x);
            if (expr->value.func.name == target) {
                expr = tmp;
            } else {
                expr->value.func.body = tmp;
            }
            break;
        case APP:
            expr->value.app.m = substitute(expr->value.app.m, target, x);
            expr->value.app.n = substitute(expr->value.app.n, target, x);
            break;
    }

    return expr;
}

struct expr_t* beta_reduce(struct expr_t* m, struct expr_t* n) {
    char target;

    target = m->value.func.name;

    print_expr(m);
    printf("\n");

    m = substitute(m->value.func.body, target, n);

    return m;
}

struct expr_t* apply(struct expr_t* expr) {
    struct expr_t* m = expr->value.app.m;
    struct expr_t* n = expr->value.app.n;

    if (m->type == APP) {
        m = apply(m);
    }

    return beta_reduce(m, n);
}

struct expr_t* reduce_expr(struct expr_t* expr) {
    switch (expr->type) {
        case VAR:
            break;
        case FUNC:
            expr->value.func.body = reduce_expr(expr->value.func.body);
            break;
        case APP:
            /* perform alpha subsitution and beta reduction */
            expr = reduce_expr(apply(expr));
            break;
    }
    fprintf(stderr, "reduction step\n");
    return expr;
}

int main(int argc, char** argv) {
    char* buffer;
    size_t size;
    struct expr_t* expr;

    if (argc < 2) {
        fprintf(stderr, "usage: %s <filename>\n", argv[0]);
        return EXIT_FAILURE;
    }

    if (!(buffer = read_file(argv[1], &size))) {
        perror("unable to read file");
        return EXIT_FAILURE;
    }

    if (!(expr = parse_buffer(buffer, size))) {
        fprintf(stderr, "unable to parse buffer\n");
        return EXIT_FAILURE;
    }

    print_expr(expr);
    printf("\n");

    expr = reduce_expr(expr);

    print_expr(expr);
    printf("\n");

    return 0;
}
