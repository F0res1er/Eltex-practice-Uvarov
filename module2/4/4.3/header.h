#ifndef HEADER_H
#define HEADER_H

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
    size_t order;
    int height;

    struct ContactNode *left;
    struct ContactNode *right;
} ContactNode;

typedef struct {
    ContactNode *root;
    size_t count;
    size_t next_order;
} PhoneBook;

int add_contact(PhoneBook *book, const Contact *contact);
int edit_contact(PhoneBook *book, size_t index, const Contact *contact);
void delete_contact(PhoneBook *book, size_t index);

const Contact *get_contact(const PhoneBook *book, size_t index);
void free_contacts(ContactNode *root);

void run_menu(PhoneBook *book);

#endif
