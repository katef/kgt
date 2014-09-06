/* $Id$ */

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "../ast.h"

#include "../rrd/rrd.h"
#include "../rrd/node.h"

#include "io.h"

static void
escputc(int c, FILE *f)
{
	size_t i;

	struct {
		int in;
		const char *out;
	} esc[] = {
		{ '&',  "&amp;"  },
		{ '\"', "&quot;" },
		{ '<',  "&#x3C;" },
		{ '>',  "&#x3E;" },
		{ '\\', "\\\\"   },
		{ '\n', "\\n"    },
		{ '\r', "\\r"    },
		{ '\f', "\\f"    },
		{ '\v', "\\v"    },
		{ '\t', "\\t"    },

		{ '^',  "\\^"    },
		{ '|',  "\\|"    },
		{ '{',  "\\{"    },
		{ '{',  "\\}"    },
		{ '[',  "\\["    },
		{ ']',  "\\]"    },
		{ '_',  "\\_"    },
		{ '-',  "\\-"    }
	};

	assert(f != NULL);

	for (i = 0; i < sizeof esc / sizeof *esc; i++) {
		if (esc[i].in == c) {
			fputs(esc[i].out, f);
			return;
		}
	}

	if (!isprint(c)) {
		fprintf(f, "\\x%x", (unsigned char) c);
		return;
	}

	putc(c, f);
}

static void
escputs(const char *s, FILE *f)
{
	const char *p;

	for (p = s; *p != '\0'; p++) {
		escputc(*p, f);
	}
}

void
rrd_print_dot(const char *prefix, const void *parent, const char *port, const struct node *node)
{
	const struct node *p;

	printf("\t{ rank = same;\n");
	for (p = node; p != NULL; p = p->next) {
		printf("\t\t\"%s/%p\";\n", prefix, (void *) p);
	}
	printf("\t};\n");

	for (p = node; p != NULL; p = p->next) {
		printf("\t\"%s/%p\"%s -> \"%s/%p\";\n",
			prefix, parent, port,
			prefix, (void *) p);

		printf("\t\"%s/%p\" [ ",
			prefix, (void *) p);

		switch (p->type) {
		case NODE_SKIP:
			printf("label = \"&epsilon;\"");
			break;

		case NODE_TERMINAL:
			printf("style = filled, shape = box, label = \"\\\"");
			escputs(p->u.terminal, stdout);
			printf("\\\"\"");
			break;

		case NODE_IDENTIFIER:
			printf("label = \"\\<");
			escputs(p->u.terminal, stdout);
			printf("\\>\"");
			break;

		case NODE_CHOICE:
			printf("label = \"CHOICE\"");
			break;

		case NODE_SEQUENCE:
			printf("label = \"SEQUENCE\"");
			break;

		case NODE_LOOP:
			printf("label = \"<b> &larr;|LOOP|<r> &rarr;\""); /* TODO: utf */
			break;

		default:
			printf("label = \"?\", color = red");
		}

		printf(" ];\n");

		switch (p->type) {
		case NODE_CHOICE:
			rrd_print_dot(prefix, p, "", p->u.choice);
			break;

		case NODE_SEQUENCE:
			rrd_print_dot(prefix, p, "", p->u.sequence);
			break;

		case NODE_LOOP:
			rrd_print_dot(prefix, p, ":f", p->u.loop.forward);
			rrd_print_dot(prefix, p, ":b", p->u.loop.backward);
			break;

		default:
			break;
		}
	}
}

