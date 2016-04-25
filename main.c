#include "recel.h"
#include "stb_image.h"
#include "stb_image_write.h"

int main(int argc, char **argv)
{
  int w, h, n;
  void *input = stbi_load(argv[1], &w, &h, &n, 4);
  printf("loaded '%s', %d*%d*%d\n", argv[1], w, h, n);

  uint32_t *distance = recel_distance(w, h, input);

  offset_map_t offsets = recel_line_map(w, h, distance);
  uint32_t *out = recel_render_map(w, h, offsets, input);

  stbi_write_png("out.png", 2 * w, 2 * h, 4, out, 0);
}
