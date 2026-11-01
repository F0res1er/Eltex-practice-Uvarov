#include "header.h"
#include <ctype.h>
#include <stdlib.h>

double add(double a, double b)
{
    return a + b;
}

double subtract(double a, double b)
{
    return a - b;
}

double multiply(double a, double b)
{
    return a * b;
}

double divide(double a, double b)
{
    return a / b;
}

void calc_init(Calculator* calc)
{
    calc->operations = NULL;
    calc->count = 0;
}

int calc_register(Calculator* calc, char symbol, int priority, OperationFn function)
{
    Operation* new_operations = realloc(calc->operations, (calc->count + 1) * sizeof(Operation));

    if (new_operations == NULL) return 0;

    calc->operations = new_operations;
    calc->operations[calc->count++] = (Operation){ symbol, priority, function };

    return 1;
}

void calc_free(Calculator* calc)
{
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
    while (isspace((unsigned char)p->input[p->pos])) ++p->pos;
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

        const Operation* op = find_operation(p->calc, p->input[p->pos]);

        if (op == NULL || op->priority < min_priority) break;

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

        if (p.input[p.pos] != '\0') syntax_error(&p);
    }

    p.result.value = value;

    return p.result;
}