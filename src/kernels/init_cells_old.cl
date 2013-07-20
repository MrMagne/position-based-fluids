__kernel void initCellsOld(__global int *cells,
                           __global int *particles_list,
                           const uint numCells,
                           const uint numParticles) {
  const uint i = get_global_id(0);

  if (i < numCells) {
    cells[i] = -1;
  }

  if (i < numParticles) {
    particles_list[i] = -1;
  }
}
