#ifndef HEADER_H
#define HEADER_H

// Арифметические операции
double add(double a, double b);
double subtract(double a, double b);
double multiply(double a, double b);
double divide(double a, double b);

// Пользовательский интерфейс
void show_menu();
int get_choice();
int get_number(const char* message, double* number);
void show_operation(int choice);
void run_calculator();

#endif