__kernel void initCells(__global int2 *foundCells,
                        const uint N) {
  const uint i = get_global_id(0);
  if (i >= N) return;

  foundCells[i].x = -1;
  foundCells[i].y = -1;

  // #if defined(USE_DEBUG)
  //printf("initCells: %d (%d,%d)\n", i, foundCells[i].x, foundCells[i].y);
  // #endif // USE_DEBUG
}
