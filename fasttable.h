#ifndef FASTTABLE_H
#define FASTTABLE_H

#include <stdint.h>

typedef struct fasttable fasttable_t;

fasttable_t *fasttable_new(void);
void fasttable_delete(fasttable_t *t);
void **fasttable_cell(fasttable_t *t, uint32_t value);
void fasttable_flush(fasttable_t *t);

#endif /*FASTTABLE_H*/
