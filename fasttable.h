#ifndef FASTTABLE_H
#define FASTTABLE_H

#include <stdint.h>

typedef struct fasttable fasttable_t;

fasttable_t *fasttable_new(void);
void fasttable_delete(fasttable_t *t);
uint32_t *fasttable_cell(fasttable_t *t, uint32_t value);
void fasttable_flush(fasttable_t *t);

typedef struct colorcounter colorcounter_t;

colorcounter_t *colorcounter_new(void);
void colorcounter_delete(colorcounter_t *t);

void colorcounter_start(colorcounter_t *t);
void colorcounter_incr(colorcounter_t *t, uint32_t value);
uint32_t colorcounter_distinct_count(colorcounter_t *t);
void colorcounter_rank(colorcounter_t *t);
int colorcounter_get_rank(colorcounter_t *t, uint32_t value);

#endif /*FASTTABLE_H*/
