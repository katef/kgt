/*
 * Copyright 2014-2020 Katherine Flavel
 *
 * See LICENCE for the full copyright terms.
 */

/*
 * AST node rewriting
 * (In this case not so much rewriting, but rather just plain removing;
 * invisible nodes have no semantic effect.)
 */

#include <assert.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

#include "txt.h"
#include "ast.h"
#include "rewrite.h"
#include "xalloc.h"

static void
walk_alts(struct ast_alt **alts);

static void
walk_term(struct ast_term *term)
{
	assert(term != NULL);

	switch (term->type) {
	case TYPE_EMPTY:
	case TYPE_CS_LITERAL:
	case TYPE_TOKEN:
	case TYPE_RULE:
	case TYPE_PROSE:
	case TYPE_CI_LITERAL:
		break;

	case TYPE_GROUP:
		walk_alts(&term->u.group);
		break;
	}
}

static void
walk_alts(struct ast_alt **alts)
{
	struct ast_alt **alt, **next_alt;
	struct ast_term **term, **next_term;

	assert(alts != NULL);

	for (alt = alts; *alt != NULL; alt = next_alt) {
		next_alt = &(*alt)->next;

		if ((*alt)->invisible) {
			struct ast_alt *dead;

			dead = *alt;
			*alt = (*alt)->next;

			dead->next = NULL;
			ast_free_alt(dead);

			next_alt = alt;
			continue;
		}

		for (term = &(*alt)->terms; *term != NULL; term = next_term) {
			next_term = &(*term)->next;

			if ((*term)->invisible) {
				struct ast_term *dead;

				dead = *term;
				*term = (*term)->next;

				dead->next = NULL;
				ast_free_term(dead);

				next_term = term;
				continue;
			}

			walk_term(*term);
		}
	}
}

void
rewrite_invisible(struct ast_rule *grammar)
{
	struct ast_rule *rule;

	for (rule = grammar; rule != NULL; rule = rule->next) {
		walk_alts(&rule->alts);
	}
}

