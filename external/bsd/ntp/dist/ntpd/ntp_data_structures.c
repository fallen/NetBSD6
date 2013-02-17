/*	$NetBSD: ntp_data_structures.c,v 1.1.1.1 2009/12/13 16:56:17 kardel Exp $	*/

/* ntp_data_structures.c
 *
 * This file contains the data structures used by the ntp configuration
 * code and the discrete event simulator.
 *
 * Written By: Sachin Kamboj
 *             University of Delaware
 *             Newark, DE 19711
 * Copyright (c) 2006
 */


#include <stdlib.h>    /* Needed for malloc */
#include "ntp_data_structures.h"
#include "ntp_stdlib.h"

/* Priority Queue
 * --------------
 * Define a priority queue in which the relative priority of the elements
 * is determined by a function 'get_order' which is supplied to the
 * priority_queue
 */


queue *create_priority_queue(int (*get_order)(void *, void *))
{
    queue *my_queue = (queue *) emalloc(sizeof(queue));
    my_queue->get_order = get_order;
    my_queue->front = NULL;
    my_queue->no_of_elements = 0;
    return my_queue;
}


/* Define a function to "destroy" a priority queue, freeing-up
 * all the allocated resources in the process 
 */

void destroy_queue(queue *my_queue)
{
    node *temp = NULL;

    /* Empty out the queue elements if they are not already empty */
    while (my_queue->front != NULL) {
        temp = my_queue->front;
        my_queue->front = my_queue->front->node_next;
        free(temp);
    }

    /* Now free the queue */
    free(my_queue);
}


/* Define a function to allocate memory for one element 
 * of the queue. The allocated memory consists of size
 * bytes plus the number of bytes needed for bookkeeping
 */

void *get_node(size_t size)
{
    node *new_node = emalloc(sizeof(*new_node) + size);
    new_node->node_next = NULL; 
    return new_node + 1;
}

/* Define a function to free the allocated memory for a queue node */
void free_node(void *my_node)
{
    node *old_node = my_node;
    free(old_node - 1);
}


void *
next_node(
	void *pv
	)
{
	node *pn;

	pn = pv;
	pn--;

	if (pn->node_next == NULL)
		return NULL;

	return pn->node_next + 1;
}


/* Define a function to check if the queue is empty. */
int empty(queue *my_queue)
{
    return (!my_queue || !my_queue->front);
}


void *
queue_head(
	queue *q
	)
{
	if (NULL == q || NULL == q->front)
		return NULL;
		
	return q->front + 1;
}


/* Define a function to add an element to the priority queue.
 * The element is added according to its priority - 
 * relative priority is given by the get_order function
 */
queue *enqueue(queue *my_queue, void *my_node)
{
    node *new_node = ((node *) my_node) - 1;
    node *i = NULL;
    node *j = my_queue->front;

    while (j != NULL && ((*my_queue->get_order)(new_node + 1, j + 1) > 0)) {
        i = j;
        j = j->node_next;
    }
    
    if (i == NULL) {       /* Insert at beginning of the queue */
        new_node->node_next = my_queue->front;
        my_queue->front = new_node;
    }
    else {                /* Insert Elsewhere, including the end */
        new_node->node_next = i->node_next;
        i->node_next = new_node;
    }

    ++my_queue->no_of_elements;    
    return my_queue;
}


/* Define a function to dequeue the first element from the priority
 * queue and return it
 */

void *dequeue(queue *my_queue)
{
    node *my_node = my_queue->front;
    if (my_node != NULL) {
        my_queue->front = my_node->node_next;
        --my_queue->no_of_elements;    
        return (void *)(my_node + 1);
    }
    else
        return NULL;
}

/* Define a function that returns the number of elements in the 
 * priority queue
 */
int get_no_of_elements(queue *my_queue)
{
    return my_queue->no_of_elements;
}

/* Define a function to append a queue onto another.
 * Note: there is a faster way (O(1) as opposed to O(n))
 * to do this for simple (FIFO) queues, but we can't rely on
 * that for priority queues. (Given the current representation)
 * 
 * I don't anticipate this to be a problem. If it does turn
 * out to be a bottleneck, I will consider replacing the 
 * current implementation with a binomial or fibonacci heap.
 */

void append_queue(queue *q1, queue *q2) 
{
    while (!empty(q2))
        enqueue(q1, dequeue(q2));
    destroy_queue(q2);
}

/* FIFO Queue
 * ----------
 * Use the priority queue to create a traditional FIFO queue.
 * The only extra function needed is the create_queue 
 */

/* C is not Lisp and does not allow anonymous lambda functions :-(. 
 * So define a get_fifo_order function here
 */

int get_fifo_order(void *el1, void *el2)
{   return 1;
}

/* Define a function to create a FIFO queue */

queue *create_queue()
{    return create_priority_queue(get_fifo_order);
}
