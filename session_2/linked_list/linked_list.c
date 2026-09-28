#include <stdio.h>
#include <stdlib.h>

typedef struct node
{
    int data;
    struct node *next;
} node;

node *insert_head_node(node *head, int value)
{
    node *new_node = malloc(sizeof(node));
    if (new_node == NULL)
    {
        return head;
    }
    new_node->data = value;
    new_node->next = head;
    return new_node;
}

node *insert_tail_node(node *head, int value)
{
    node *new_node = malloc(sizeof(node));
    if (new_node == NULL)
    {
        return head;
    }
    new_node->data = value;
    new_node->next = NULL;

    if (head == NULL)
    {
        return new_node;
    }

    node *current = head;

    while (current->next != NULL)
    {
        current = current->next;
    }
    current->next = new_node;

    return head;
}

node *insert_node(node *head, int value, int index)
{
    node *new_node = malloc(sizeof(node));
    if (new_node == NULL)
    {
        return head;
    }
    new_node->data = value;
    new_node->next = NULL;

    if (head == NULL)
    {
        return new_node;
    }

    node *current = head;
    int dem = 0;
    while (current != NULL)
    {
        if (dem == (index - 1))
        {
            new_node->next = current->next;
            current->next = new_node;
            return head;
        }
        dem++;
        current = current->next;
    }

    free(new_node);
    return head;
}

node *delete_head_node(node *head)
{
    if (head == NULL)
    {
        return head;
    }
    node *p = head;
    head = head->next;
    free(p);
    return head;
}

node *delete_tail_node(node *head)
{
    if (head == NULL)
    {
        return head;
    }

    if(head->next == NULL)
    {
        free(head);
        return NULL;
    }

    node *current = head;

    while (current->next->next != NULL)
    {
        current = current->next;
    }

    node *p = current->next;
    current->next = NULL;

    free(p);
    return head;
}

node *delete_node(node *head, int value)
{
    /* no node */
    if (head == NULL)
    {
        return head;
    }

    /* head node */
    if(head->data == value)
    {
        return delete_head_node(head);
    }

    /* more than 1 node */
    node *current = head;
    while (current->next != NULL)
    {
        if (current->next->data == value)
        {
            node *q = current->next;
            current->next = q->next;
            free(q);
            return head;
        }
        current = current->next;
    }
    return head;
}

node *delete_list(node *head)
{
    while (head != NULL)
    {
        node *current = head;
        head = head->next;
        free(current);
    }
    return NULL;
}

node *find_node(node *head, int value)
{
    node *current = head;
    while (current != NULL)
    {
        if(current->data == value)
        {
            return current;
        }
        current = current->next;
    }
    
    return current;
}

node *copy_list(node *head)
{
    if(head == NULL)
    {
        return NULL;
    }

    node *copy_head = NULL;
    node *copy_tail = NULL;

    while (head != NULL)
    {
        node *new_node = malloc(sizeof(node));
        if(new_node == NULL)
        {
            return NULL;
        }

        new_node->data = head->data;
        new_node->next = NULL;

        if(copy_head == NULL)
        {
            copy_head = new_node;
            copy_tail = new_node;
        }
        else
        {
            copy_tail->next = new_node;
            copy_tail = new_node;
        }

        head = head->next;
    }
    return copy_head;
}

void print_value_of_list(node *head)
{
    node *p = head;
    while (p != NULL)
    {
        printf("%d\n", p->data);
        p = p->next;
    }
}

int main(void)
{
    /* create an empty list */
    node *head = malloc(sizeof(node));

    if (head != NULL)
    {
        head->data = 10;
        head->next = NULL;

        /* add a node to the list */
        head = insert_head_node(head, 10);
        head = insert_head_node(head, 20);
        head = insert_head_node(head, 30);
        head = insert_head_node(head, 40);
        /* print the nodes in the list */
        print_value_of_list(head);
        printf("==================\n");

        /* delete head node */
        head = delete_head_node(head);
        print_value_of_list(head);
        printf("==================\n");

        /* add tail node */
        head = insert_tail_node(head, 50);
        print_value_of_list(head);
        printf("==================\n");

        /* delete tail node */
        head = delete_tail_node(head);
        print_value_of_list(head);
        printf("==================\n");

        /* insert node */
        head = insert_node(head, 70, 2);
        print_value_of_list(head);
        printf("==================\n");

        /* delete node */
        head = delete_node(head, 30);
        print_value_of_list(head);
        printf("==================\n");

        /* delete list */
        node *copy = NULL;
        copy = copy_list(head);
        print_value_of_list(copy);
        printf("==================\n");

        free(copy);
    }

    free(head);
    return 0;
}
