#ifndef HEADER_H
#define HEADER_H

#define _CRT_SECURE_NO_WARNINGS

#include <stddef.h>

#define MAX_STR 100
#define MAX_PHONES 3
#define MAX_EMAILS 3

typedef struct {
    char telegram[MAX_STR];
    char vk[MAX_STR];
    char other[MAX_STR];
} SocialMedia;

typedef struct {
    char surname[MAX_STR];
    char name[MAX_STR];
    char patronymic[MAX_STR];
    char workplace[MAX_STR];
    char position[MAX_STR];

    char phones[MAX_PHONES][MAX_STR];
    char emails[MAX_EMAILS][MAX_STR];

    SocialMedia social;
} Contact;

typedef struct ContactNode {
    Contact contact;
    struct ContactNode* prev;
    struct ContactNode* next;
} ContactNode;

typedef struct {
    ContactNode* head;
    size_t count;
} PhoneBook;

int add_contact(PhoneBook* book, const Contact* contact);
void edit_contact(PhoneBook* book, ContactNode* node, const Contact* contact);
void delete_contact(PhoneBook* book, ContactNode* node);

void run_menu(PhoneBook* book);

#endif
