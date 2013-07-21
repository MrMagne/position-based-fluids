__kernel void calcHash(const __global float4 *predicted,
                       __global uint2 *radixCells,
                       const uint maxInt,
                       const uint numParticles,
                       const uint numKeys) {
  // Get particle and assign them to a cell
  const uint i = get_global_id(0);

  // #if defined(USE_DEBUG)
  // printf("%d\n", i);
  // #endif // USE_DEBUG

  if (i < numParticles) {
    // Get cell that belongs to particle
    uint cell_pos = (uint) ( (predicted[i].x - SYSTEM_MIN_X)
                             / CELL_LENGTH_X )
                    + (uint) ( (predicted[i].y - SYSTEM_MIN_Y)
                               / CELL_LENGTH_Y ) * NUMBER_OF_CELLS_X
                    + (uint) ( (predicted[i].z - SYSTEM_MIN_Z)
                               / CELL_LENGTH_Z ) * NUMBER_OF_CELLS_X * NUMBER_OF_CELLS_Y;

    radixCells[i] = (uint2)(cell_pos, i);
  } else {
    radixCells[i] = (uint2)(maxInt, 0);
  }
}
