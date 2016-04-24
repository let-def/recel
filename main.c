#include "recel.h"
#include "stb_image.h"
#include "stb_image_write.h"

int main(int argc, char **argv)
{
  int w, h, n;
  void *data = stbi_load(argv[1], &w, &h, &n, 4);

  uint32_t *distance = recel_distance(w, h, data);

  offset_t *offsets = recel_solve(w, h, distance);
  uint32_t *out = recel_render(w, h, data, offsets);

  stbi_write_png("out.png", 2 * w, 2 * h, 4, out, 0);
}
