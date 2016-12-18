#include "fasttable.h"
#include <stdlib.h>

struct cell_s {
  uint32_t key;
  uint32_t gen;
  uint32_t value;
};

struct fasttable {
  int capacity;
  unsigned int gen;
  struct cell_s *cells;
  int filled;
};

fasttable_t *fasttable_new(void)
{
  fasttable_t *t = malloc(sizeof(fasttable_t));
  t->capacity = 16;
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
  int oldsize = t->capacity;
  struct cell_s *oldcells = t->cells;

  int new_capacity = oldsize * 2, mask = new_capacity - 1;
  struct cell_s *newcells = calloc(sizeof(struct cell_s), new_capacity);

  t->capacity = new_capacity;
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

uint32_t *fasttable_cell(fasttable_t *t, uint32_t key)
{
  int index = fasttable_index(key);
  int mask = t->capacity - 1;
  struct cell_s *cells = t->cells;

  while (cells[index & mask].gen == t->gen)
  {
    if (cells[index & mask].key == key)
      return &cells[index & mask].value;
    index += 1;
  }

  t->filled += 1;

  struct cell_s *cell = &cells[index & mask];
  cell->gen = t->gen;
  cell->key = key;
  cell->value = -1;

  if (t->filled * 4 >= t->capacity * 3)
  {
    fasttable_resize(t);
    mask = t->capacity - 1;
    cells = t->cells;

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
  if (t->filled != 0)
  {
    t->gen += 1;
    t->filled = 0;
  }
}

struct colorcell_s {
  uint32_t key;
  uint32_t value;
};

struct colorcounter {
  struct colorcell_s *cells;
  int filled;
  int capacity;
  fasttable_t *table;
};

colorcounter_t *colorcounter_new(void)
{
  colorcounter_t *t = malloc(sizeof(colorcounter_t));
  t->capacity = 16;
  t->filled = 0;
  t->cells = calloc(sizeof(struct colorcell_s), 16);
  t->table = fasttable_new();
  return t;
}

void colorcounter_delete(colorcounter_t *t)
{
  fasttable_delete(t->table);
  free(t->cells);
  free(t);
}

void colorcounter_start(colorcounter_t *t)
{
  t->filled = 0;
  fasttable_flush(t->table);
}

void colorcounter_incr(colorcounter_t *t, uint32_t value)
{
  uint32_t *index = fasttable_cell(t->table, value);
  if (*index == -1)
  {
    *index = t->filled;
    t->filled += 1;
    if (t->filled >= t->capacity)
    {
      t->capacity = t->filled * 2;
      t->cells = realloc(t->cells, t->capacity * sizeof(struct colorcell_s));
    }
    t->cells[*index].key = value;
    t->cells[*index].value = 1;
  }
  else
    t->cells[*index].value += 1;
}

uint32_t colorcounter_distinct_count(colorcounter_t *t)
{
  return t->filled;
}

static int colorcell_compar(const void *a, const void *b)
{
  const struct colorcell_s *ca = a, *cb = b;
  return (long int)cb->value - (long int)ca->value;
}

void colorcounter_rank(colorcounter_t *t)
{
  qsort(t->cells, t->filled, sizeof(struct colorcell_s), colorcell_compar);
  for (int i = 0; i < t->filled; ++i)
  {
    *fasttable_cell(t->table, t->cells[i].key) = i;
  }
}

int colorcounter_get_rank(colorcounter_t *t, uint32_t value)
{
  return *fasttable_cell(t->table, value);
}
