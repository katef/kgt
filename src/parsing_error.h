/*
 * Copyright 2021 Katherine Flavel
 *
 * See LICENCE for the full copyright terms.
 */

#ifndef KGT_PARSING_ERROR_H
#define KGT_PARSING_ERROR_H

#define PARSING_ERROR_DESCRIPTION_SIZE (256)

struct parsing_error {
	int line;
	int col;
	char description[PARSING_ERROR_DESCRIPTION_SIZE];
};

struct parsing_error_queue_element {
	struct parsing_error error;
	struct parsing_error_queue_element* next;
};

typedef struct parsing_error parsing_error;
typedef struct parsing_error_queue_element parsing_error_queue_element;
typedef parsing_error_queue_element* parsing_error_queue;

void parsing_error_queue_push(parsing_error_queue* queue, parsing_error error);
int parsing_error_queue_pop(parsing_error_queue* queue, parsing_error* error);

#endif
