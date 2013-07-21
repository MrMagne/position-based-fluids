__kernel void updateCells(const __global float4 *predicted,
                          __global int *cells,
                          __global int *particles_list,
                          const uint N) {
  // Get particle and assign them to a cell
  const uint i = get_global_id(0);
  if (i >= N) return;

  // Get cell that belongs to particle
  uint cell_pos = (int) ( (predicted[i].x - SYSTEM_MIN_X)
                          / CELL_LENGTH_X )
                  + (int) ( (predicted[i].y - SYSTEM_MIN_Y)
                            / CELL_LENGTH_Y ) * NUMBER_OF_CELLS_X
                  + (int) ( (predicted[i].z - SYSTEM_MIN_Z)
                            / CELL_LENGTH_Z ) * NUMBER_OF_CELLS_X * NUMBER_OF_CELLS_Y;

  // Exchange cells[cell_pos] and particle_list at i
  particles_list[i] = atomic_xchg(&cells[cell_pos], i);

  // #if defined(USE_DEBUG)
  // printf("UPDATE_CELL: %d cell_pos:%d\n", i, cell_pos);
  // #endif
}
