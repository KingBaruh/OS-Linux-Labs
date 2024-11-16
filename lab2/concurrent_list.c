#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include "concurrent_list.h"

struct node
{
    int value; /**< integer value of the node */
    node *p_next; /**< pointer to next list node */
    pthread_mutex_t mutex; /**< mutex for fine-grain thread-safety */
};

struct list
{
    node *p_head; /**< head node of the list, NULL if empty */
    pthread_mutex_t mutex; /**< mutex guard on list head in case of head update */
};

void print_node(node* node)
{
  // DO NOT DELETE
  if(node)
  {
    printf("%d ", node->value);
  }
}

// Function to create a new list
list* create_list()
{
    int result;
    /* Allocate list structure */
    list *new_list = (list *)malloc(sizeof(list));
    if(new_list)
    {
        new_list->p_head = NULL;
    }
    /* Initilalize head mutex with default attributes,
     * in real program some attributes would be considrerd.
     */
    result = pthread_mutex_init(&(new_list->mutex), NULL);
    if(result)
    {
        free(new_list);
        return NULL;
    }
    return new_list;
}

// Function to delete the list
void delete_list(list* list)
{
    node *p_head;
    node *tmp;

    if(NULL == list) /* sanity check */
        return;

    /* lock head mutex, save head pointer and nullify the original pointer */
    pthread_mutex_lock(&list->mutex);
    p_head = list->p_head;
    list->p_head = NULL;
    pthread_mutex_unlock(&list->mutex);
    pthread_mutex_destroy(&list->mutex);
    free(list);

    /* traverse whole list and destroy nodes */
    while(p_head)
    {
        pthread_mutex_lock(&p_head->mutex);
        tmp = p_head;
        p_head = p_head->p_next;
        pthread_mutex_unlock(&tmp->mutex);
        pthread_mutex_destroy(&tmp->mutex);
        free(tmp);
    }
}

// Function to insert a value into the list in ascending order
void insert_value(list* list, int value)
{
    node *p_head;
    if(NULL == list) /* sanity check */
        return;


    int result;
    node *p = (node *)malloc(sizeof(node));
    if(p)
    {
        p->value = value;
        p->p_next = NULL;
        /* Init mutex with default attributes in sake of simplicity,
         * in real program atributes would be considered.
         */
        result = pthread_mutex_init(&(p->mutex), NULL);
        if(result)
        {
            free(p);
            return;
        }
    }
    else
    {
        return;
    }

    pthread_mutex_lock(&list->mutex);
    p_head = list->p_head;
    pthread_mutex_unlock(&list->mutex);

    /* Traverse the list till the node with bigger value */
    if(NULL == p_head) /* Empty list, just add first node */
    {
        pthread_mutex_lock(&list->mutex);
        list->p_head = p;
        pthread_mutex_unlock(&list->mutex);
    }
    else
    {
        node *p_prev = NULL;
        node *p_next = NULL;
        node *p_curr = NULL;

        p_curr = p_head;
        p_prev = p_head;

        while(p_curr)
        {
            pthread_mutex_lock(&(p_curr->mutex));
            if(p_curr->value >= value) /* insert new node between previous and current */
            {
                if(p_head == p_curr) /* insert node at the head of the list */
                {
                    pthread_mutex_lock(&list->mutex);
                    p->p_next = list->p_head;
                    list->p_head = p;
                    pthread_mutex_unlock(&list->mutex);
                    pthread_mutex_unlock(&(p_curr->mutex));
                    return;
                }
                else /* insert node in between */
                {
                    pthread_mutex_lock(&p_prev->mutex);
                    p_prev->p_next = p;
                    p->p_next = p_curr;
                    pthread_mutex_unlock(&p_prev->mutex);
                    pthread_mutex_unlock(&(p_curr->mutex));
                    return;
                }
            }
            else
            {
                p_next = p_curr->p_next;
                if(NULL == p_next) /* just add new node as last node */
                {
                    p->p_next = NULL;
                    p_curr->p_next = p;
                    pthread_mutex_unlock(&(p_curr->mutex));
                    return;
                }
                else /* go next step */
                {
                    pthread_mutex_unlock(&(p_curr->mutex));
                    p_prev = p_curr;
                    p_curr = p_curr->p_next;
                }
            }
        }
    }
}

// Function to remove a value from the list
void remove_value(list* list, int value)
{
    node *p_head;
    if(NULL == list) /* sanity check */
        return;

    pthread_mutex_lock(&list->mutex);
    p_head = list->p_head;
    pthread_mutex_unlock(&list->mutex);

    /* Traverse the list till desired node */
    if(NULL == p_head) /* Empty list */
        return;
    else
    {
        node *p_prev = p_head;
        node *p_next = NULL;
        node *p_curr = p_head;

        while(p_curr)
        {
            pthread_mutex_lock(&(p_curr->mutex));
            if(p_curr->value == value) /* node for remove is found */
            {
                if(p_curr == p_head) /* head node, substitute by next */
                {
                    pthread_mutex_lock(&list->mutex);
                    list->p_head = p_curr->p_next;
                    pthread_mutex_unlock(&list->mutex);
                    pthread_mutex_unlock(&(p_curr->mutex));
                    pthread_mutex_destroy(&(p_curr)->mutex);
                    free(p_curr);
                    return;
                }
                else /* delete node from list */
                {
                    p_next = p_curr->p_next;
                    pthread_mutex_lock(&(p_prev->mutex));
                    p_prev->p_next = p_next;
                    pthread_mutex_unlock(&(p_prev->mutex));
                    pthread_mutex_unlock(&(p_curr->mutex));
                    pthread_mutex_destroy(&(p_curr)->mutex);
                    free(p_curr);
                    return;
                }
            }
            else /* go next step */
            {
                p_next = p_curr->p_next;
                pthread_mutex_unlock(&(p_curr->mutex));
                p_prev = p_curr;
                p_curr = p_next;
            }
        }
        /* node is not found */
    }
}


// Function to print the list
void print_list(list* list)
{
    node *p_head;
    node *tmp = NULL;
    if(NULL == list)
        return;

    pthread_mutex_lock(&list->mutex);
    p_head = list->p_head;
    pthread_mutex_unlock(&list->mutex);

    while(p_head)
    {
        pthread_mutex_lock(&p_head->mutex);
        print_node(p_head);
        tmp = p_head;
        p_head = p_head->p_next;
        pthread_mutex_unlock(&tmp->mutex);
    }

    printf("\n"); // DO NOT DELETE
}

// Function to count the number of nodes that match a predicate
void count_list(list* list, int (*predicate)(int))
{
    int count = 0; // DO NOT DELETE
    node *p_head;
    node *tmp = NULL;
    if(NULL == list)
    {
        printf("%d items were counted\n", count);
        return;
    }

    pthread_mutex_lock(&list->mutex);
    p_head = list->p_head;
    pthread_mutex_unlock(&list->mutex);

    while(p_head)
    {
        pthread_mutex_lock(&p_head->mutex);
        if(predicate(p_head->value))
        {
            count++;
        }
        tmp = p_head;
        p_head = p_head->p_next;
        pthread_mutex_unlock(&tmp->mutex);
    }
    printf("%d items were counted\n", count); // DO NOT DELETE
}

