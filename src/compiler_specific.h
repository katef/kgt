/*
 * Copyright 2020 Katherine Flavel
 *
 * See LICENCE for the full copyright terms.
 */

#ifndef KGT_COMPILER_SPECIFIC_H
#define KGT_COMPILER_SPECIFIC_H

/* Decorate a function. This indicates the caller must use the returned value. */
#undef WARN_UNUSED_RESULT
#if defined(__GNUC__) || defined(__clang__)
#define WARN_UNUSED_RESULT __attribute__((warn_unused_result))
#else
#define WARN_UNUSED_RESULT
#endif

#endif
