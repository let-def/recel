#include "recel.h"

void recel_scanline(
    uint32_t w,
    const uint32_t *dista, const uint32_t *distb,
    const uint32_t *linea, const uint32_t *lineb,
    uint32_t *disto, uint32_t *lineo
    )
{
  int i = 0;
  while (i < w - 1)
  {
    if (dista[i] == distb[i])
    { // Flat area
      lineo[i] = linea[i];
      disto[i] = dista[i];
      i += 1;
    }
    else
    { // Compute length of scan
      const uint32_t *dist1, *line1, *dist2, *line2;
      if (dista[i] < distb[i]) {
        dist1 = dista; line1 = linea;
        dist2 = distb; line2 = lineb;
      } else {
        dist1 = distb; line1 = lineb;
        dist2 = dista; line2 = linea;
      }

      uint32_t k = dist1[i];

      int j = i + 1;
      while (j < w - 1 && dist1[j] == k && dist2[j] > k)
        j += 1;

      // Scan runs from i to j - 1
      // dist1[scan] < dist2[scan]
      // line[i-1] and line[j] are valid

      // Favor line2 (higher)
      int stickleft = dist1[i-1] >= dist2[i];
      int stickright = dist1[j] >= dist2[j-1];

      if (stickleft == stickright)
      { // Inflexion point
        const uint32_t *lineout = stickleft ? line2 : line1;
        const uint32_t *linein = stickleft ? line1 : line2;
        const uint32_t *distout = stickleft ? dist2 : dist1;
        const uint32_t *distin = stickleft ? dist1 : dist2;

        if (j - i > 6)
        { // Draw a curve
          int d = (j - i) / 3;
          int i2 = i + d;
          while (i < i2)
          {
            lineo[i] = lineout[i];
            disto[i] = distout[i];
            i += 1;
          }
          while (i < j - d)
          {
            lineo[i] = linein[i];
            disto[i] = distin[i];
            i += 1;
          }
          while (i < j)
          {
            lineo[i] = lineout[i];
            disto[i] = distout[i];
            i += 1;
          }
        }
        else
        { // Too short for a curve
          while (i < j)
          {
            lineo[i] = lineout[i];
            disto[i] = distout[i];
            i += 1;
          }
        }
      }
      else
      {
        const uint32_t *lineleft = stickleft ? line2 : line1;
        const uint32_t *lineright = stickleft ? line1 : line2;
        const uint32_t *distleft = stickleft ? dist2 : dist1;
        const uint32_t *distright = stickleft ? dist1 : dist2;
        int d = (j + i) / 2;
        while (i < d)
        {
          lineo[i] = lineleft[i];
          disto[i] = distleft[i];
          i += 1;
        }
        while (i < j)
        {
          lineo[i] = lineright[i];
          disto[i] = distright[i];
          i += 1;
        }
      }
    }
  }
}

void recel_scan(
    uint32_t w, uint32_t h,
    const uint32_t *disti, const uint32_t *linei,
    uint32_t *disto, uint32_t *lineo
    )
{
  for (uint32_t y = 0; y < h - 1; ++y)
  {
    recel_scanline(w,
        disti + y * w, disti + y * w + w,
        linei + y * w, linei + y * w + w,
        disto + y * w, lineo + y * w);
  }
}
