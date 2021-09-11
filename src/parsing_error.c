/*
 * Copyright 2021 Katherine Flavel
 *
 * See LICENCE for the full copyright terms.
 */

#include <stddef.h>
#include <stdlib.h>

#include "xalloc.h"

#include "parsing_error.h"

void
parsing_error_queue_push(parsing_error_queue *queue, parsing_error error)
{
	/* Find the end of the queue: */
	parsing_error_queue_element **tail = queue;
	while (*tail != NULL) {
		tail = &((*tail)->next);
	}

	/* Allocate a parsing_error_queue and initialize it: */
	*tail = xmalloc(sizeof (parsing_error_queue_element));
	(*tail)->error = error;
	(*tail)->next = NULL;
}

int
parsing_error_queue_pop(parsing_error_queue *queue, parsing_error *error)
{
	if (!*queue) {
		return 0;
	}

	parsing_error_queue_element *head = *queue;
	*error = head->error;
	*queue = head->next;

	free(head);

	return 1;
}
