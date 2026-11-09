#include "header.h"
#include <stdlib.h>
#include <string.h>

static int compare_nodes(const ContactNode *a, const ContactNode *b)
{
    int result = strcoll(a->contact.surname, b->contact.surname);

    if (result == 0)
        result = strcoll(a->contact.name, b->contact.name);

    if (result == 0)
        result = strcoll(a->contact.patronymic, b->contact.patronymic);

    if (result == 0)
        result = (a->order > b->order) - (a->order < b->order);

    return result;
}

static int height(const ContactNode *node)
{
    return node ? node->height : 0;
}

static void update_height(ContactNode *node)
{
    int left = height(node->left);
    int right = height(node->right);

    node->height = (left > right ? left : right) + 1;
}

static ContactNode *rotate_right(ContactNode *node)
{
    ContactNode *left = node->left;

    node->left = left->right;
    left->right = node;

    update_height(node);
    update_height(left);

    return left;
}

static ContactNode *rotate_left(ContactNode *node)
{
    ContactNode *right = node->right;

    node->right = right->left;
    right->left = node;

    update_height(node);
    update_height(right);

    return right;
}

static ContactNode *balance(ContactNode *node)
{
    update_height(node);

    int difference = height(node->left) - height(node->right);

    if (difference > 1) 
    {
        if (height(node->left->left) < height(node->left->right))
            node->left = rotate_left(node->left);

        return rotate_right(node);
    }

    if (difference < -1) 
    {
        if (height(node->right->right) < height(node->right->left))
            node->right = rotate_right(node->right);

        return rotate_left(node);
    }

    return node;
}

static ContactNode *insert_node(ContactNode *root, ContactNode *node)
{
    if (root == NULL)
        return node;

    if (compare_nodes(node, root) < 0)
        root->left = insert_node(root->left, node);
    else
        root->right = insert_node(root->right, node);

    return balance(root);
}

static ContactNode *remove_node(ContactNode *root, const ContactNode *key)
{
    int result = compare_nodes(key, root);

    if (result < 0)
        root->left = remove_node(root->left, key);
    
    else if (result > 0)
        root->right = remove_node(root->right, key);
    
    else 
    {
        if (root->left == NULL || root->right == NULL) 
        {
            ContactNode *child = root->left ? root->left : root->right;

            free(root);
            return child;
        }

        ContactNode *next = root->right;

        while (next->left != NULL)
            next = next->left;

        root->contact = next->contact;
        root->order = next->order;

        root->right = remove_node(root->right, root);
    }

    return balance(root);
}

static const ContactNode *find_by_index(const ContactNode *root, size_t index, size_t *current)
{
    if (root == NULL)
        return NULL;

    const ContactNode *found = find_by_index(root->left, index, current);

    if (found != NULL)
        return found;

    if ((*current)++ == index)
        return root;

    return find_by_index(root->right, index, current);
}

const Contact *get_contact(const PhoneBook *book, size_t index)
{
    size_t current = 0;

    const ContactNode *node = find_by_index(book->root, index, &current);

    return node ? &node->contact : NULL;
}

int add_contact(PhoneBook *book, const Contact *contact)
{
    ContactNode *node = malloc(sizeof(*node));

    if (node == NULL)
        return 0;

    *node = (ContactNode){
        .contact = *contact,
        .order = book->next_order++,
        .height = 1
    };

    book->root = insert_node(book->root, node);
    book->count++;

    return 1;
}

int edit_contact(PhoneBook *book, size_t index, const Contact *contact)
{
    ContactNode *node = malloc(sizeof(*node));

    if (node == NULL)
        return 0;

    size_t current = 0;

    const ContactNode *old = find_by_index(book->root, index, &current);

    ContactNode key = {
        .contact = old->contact,
        .order = old->order
    };

    *node = (ContactNode){
        .contact = *contact,
        .order = old->order,
        .height = 1
    };

    book->root = remove_node(book->root, &key);
    book->root = insert_node(book->root, node);

    return 1;
}

void delete_contact(PhoneBook *book, size_t index)
{
    size_t current = 0;

    const ContactNode *old = find_by_index(book->root, index, &current);

    ContactNode key = {
        .contact = old->contact,
        .order = old->order
    };

    book->root = remove_node(book->root, &key);
    book->count--;
}

void free_contacts(ContactNode *root)
{
    if (root == NULL)
        return;

    free_contacts(root->left);
    free_contacts(root->right);

    free(root);
}