#include "fasttable.h"
#include <stdlib.h>

struct cell_s {
  uint32_t key;
  uint32_t gen;
  void     *value;
};

struct fasttable {
  int size;
  unsigned int gen;
  struct cell_s *cells;
  int filled;
};

fasttable_t *fasttable_new(void)
{
  fasttable_t *t = malloc(sizeof(fasttable_t));
  t->size = 16;
  t->filled = 0;
  t->gen = 1;
  t->cells = calloc(sizeof(struct cell_s), 16);
  return t;
}

void fasttable_delete(fasttable_t *t)
{
  free(t->cells);
  free(t);
}

static uint32_t fasttable_index(uint32_t key)
{
  return (key ^ 3087974849) * 2654435761;
}

static void fasttable_resize(fasttable_t *t)
{
  int oldgen = t->gen;
  int oldsize = t->size;
  struct cell_s *oldcells = t->cells;

  int newsize = oldsize * 2, mask = newsize - 1;
  struct cell_s *newcells = calloc(sizeof(struct cell_s), t->size);

  t->size = newsize;
  t->cells = newcells;
  t->gen = 1;

  for (int i = 0; i < oldsize; ++i)
  {
    if (oldcells[i].gen == oldgen)
    {
      int index = fasttable_index(oldcells[i].key);

      while (newcells[index & mask].gen == 1)
        index += 1;

      struct cell_s *cell = &newcells[index & mask];
      cell->gen = 1;
      cell->key = oldcells[i].key;
      cell->value = oldcells[i].value;
    }
  }

  free(oldcells);
}

void **fasttable_cell(fasttable_t *t, uint32_t key)
{
  int index = fasttable_index(key);
  int mask = t->size - 1;
  struct cell_s *cells = t->cells;

  while (cells[index & mask].gen == t->gen)
  {
    if (cells[index & mask].key == key)
      return &t->cells[index & mask].value;
    index += 1;
  }

  t->filled += 1;

  struct cell_s *cell = &cells[index & mask];
  cell->gen = t->gen;
  cell->key = key;
  cell->value = NULL;

  if (t->filled * 4 >= t->size * 3)
  {
    fasttable_resize(t);
    index = fasttable_index(key);

    while (cells[index & mask].key != key)
      index += 1;

    return &t->cells[index & mask].value;
  }
  else
    return &cell->value;
}

void fasttable_flush(fasttable_t *t)
{
  t->gen += 1;
  t->filled = 0;
}
