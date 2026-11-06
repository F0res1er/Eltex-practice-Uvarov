#include "header.h"

#if defined(LIB_ADD)

char operation_symbol = '+';
int operation_priority = 1;

double operation(double a, double b)
{
    return a + b;
}

#elif defined(LIB_SUBTRACT)

char operation_symbol = '-';
int operation_priority = 1;

double operation(double a, double b)
{
    return a - b;
}

#elif defined(LIB_MULTIPLY)

char operation_symbol = '*';
int operation_priority = 2;

double operation(double a, double b)
{
    return a * b;
}

#elif defined(LIB_DIVIDE)

char operation_symbol = '/';
int operation_priority = 2;

double operation(double a, double b)
{
    return a / b;
}

#else

#include <ctype.h>
#include <dirent.h>
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void calc_init(Calculator* calc)
{
    calc->operations = NULL;
    calc->count = 0;
}

int calc_load(Calculator* calc, const char* directory)
{
    DIR* dir = opendir(directory);

    if (dir == NULL) return 0;

    struct dirent* entry;

    while ((entry = readdir(dir)) != NULL)
    {
        size_t len = strlen(entry->d_name);

        if (len < 3 || strcmp(entry->d_name + len - 3, ".so") != 0)
            continue;

        char path[512];
        snprintf(path, sizeof(path), "%s/%s", directory, entry->d_name);

        void* handle = dlopen(path, RTLD_NOW);

        if (handle == NULL) continue;

        OperationFn function = (OperationFn)dlsym(handle, "operation");
        char* symbol = (char*)dlsym(handle, "operation_symbol");
        int* priority = (int*)dlsym(handle, "operation_priority");

        if (function == NULL || symbol == NULL || priority == NULL)
        {
            dlclose(handle);
            continue;
        }

        Operation* updated = realloc(calc->operations, (calc->count + 1) * sizeof(Operation));

        if (updated == NULL)
        {
            dlclose(handle);
            closedir(dir);
            return 0;
        }

        calc->operations = updated;

        calc->operations[calc->count++] = (Operation){
            *symbol,
            *priority,
            function,
            handle
        };
    }

    closedir(dir);

    return calc->count > 0;
}

void calc_free(Calculator* calc)
{
    for (size_t i = 0; i < calc->count; ++i)
    {
        dlclose(calc->operations[i].handle);
    }

    free(calc->operations);
    calc->operations = NULL;
    calc->count = 0;
}

typedef struct {
    const Calculator* calc;
    const char* input;
    size_t pos;
    CalcResult result;
} Parser;

static void skip_spaces(Parser* p)
{
    while (isspace((unsigned char)p->input[p->pos]))
        ++p->pos;
}

static void syntax_error(Parser* p)
{
    if (p->result.error == CALC_OK)
    {
        p->result.error = CALC_SYNTAX_ERROR;
        p->result.error_position = p->pos;
    }
}

static const Operation* find_operation(const Calculator* calc, char symbol)
{
    for (size_t i = 0; i < calc->count; ++i)
    {
        if (calc->operations[i].symbol == symbol)
        {
            return &calc->operations[i];
        }
    }

    return NULL;
}

static double parse_expression(Parser* p, int min_priority);

static double parse_primary(Parser* p)
{
    skip_spaces(p);

    if (p->input[p->pos] == '(')
    {
        ++p->pos;

        double value = parse_expression(p, 0);

        if (p->result.error != CALC_OK) return 0.0;

        skip_spaces(p);

        if (p->input[p->pos] != ')')
        {
            syntax_error(p);
            return 0.0;
        }

        ++p->pos;
        return value;
    }

    char* end = NULL;
    double value = strtod(p->input + p->pos, &end);

    if (end == p->input + p->pos)
    {
        syntax_error(p);
        return 0.0;
    }

    p->pos = (size_t)(end - p->input);

    return value;
}

static double parse_unary(Parser* p)
{
    skip_spaces(p);

    if (p->input[p->pos] == '+' || p->input[p->pos] == '-')
    {
        char sign = p->input[p->pos++];
        double value = parse_unary(p);

        return sign == '-' ? -value : value;
    }

    return parse_primary(p);
}

static double parse_expression(Parser* p, int min_priority)
{
    double left = parse_unary(p);

    while (p->result.error == CALC_OK)
    {
        skip_spaces(p);

        const Operation* op = find_operation(
            p->calc,
            p->input[p->pos]
        );

        if (op == NULL || op->priority < min_priority)
            break;

        ++p->pos;

        double right = parse_expression(p, op->priority + 1);

        if (p->result.error != CALC_OK) return 0.0;

        if (op->symbol == '/' && right == 0.0)
        {
            p->result.division_by_zero = 1;
        }

        left = op->function(left, right);
    }

    return left;
}

CalcResult calc_evaluate(const Calculator* calc, const char* expression)
{
    Parser p = {
        calc,
        expression,
        0,
        { 0.0, CALC_OK, 0, 0 }
    };

    double value = parse_expression(&p, 0);

    if (p.result.error == CALC_OK)
    {
        skip_spaces(&p);

        if (p.input[p.pos] != '\0')
            syntax_error(&p);
    }

    p.result.value = value;

    return p.result;
}

#endif
