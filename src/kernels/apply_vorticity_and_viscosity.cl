__kernel void applyVorticityAndViscosity(const __global float4 *predicted,
    const __global float4 *velocities,
    __global float4 *deltaVelocities,
#if defined(USE_LINKEDCELL)
    const __global int *cells,
    const __global int *particles_list,
#else
    const __global int2 *radixCells,
    const __global int2 *foundCells,
#endif // USE_LINKEDCELL
    const float4 system_length_min,
    const float4 system_length_max,
    const float4 cell_length,
    const int4 number_cells,
    const int N) {
  const int i = get_global_id(0);
  if (i >= N) return;

  const int END_OF_CELL_LIST = -1;
  const float h = cell_length.x;
  const float h2 = h * h;
  const float h6 = h2 * h2 * h2;
  const float h9 = h2 * h2 * h2 * h2 * h;
  const float poly6_factor = 315.0f / (64.0f * M_PI * h9);
  const float gradSpiky_factor = 45.0f / (M_PI * h6);

  int current_cell[3];

  current_cell[0] = (int) ( (predicted[i].x - system_length_min.x)
                            / cell_length.x );
  current_cell[1] = (int) ( (predicted[i].y - system_length_min.y)
                            / cell_length.y );
  current_cell[2] = (int) ( (predicted[i].z - system_length_min.z)
                            / cell_length.z );

  float4 viscosity_sum = (float4) 0.0f;

  for (int x = -1; x <= 1; ++x) {
    for (int y = -1; y <= 1; ++y) {
      for (int z = -1; z <= 1; ++z) {
        int neighbour_cell[3];

        neighbour_cell[0] = current_cell[0] + x;
        neighbour_cell[1] = current_cell[1] + y;
        neighbour_cell[2] = current_cell[2] + z;

        if (neighbour_cell[0] < 0 || neighbour_cell[0] >= number_cells.x ||
            neighbour_cell[1] < 0 || neighbour_cell[1] >= number_cells.y ||
            neighbour_cell[2] < 0 || neighbour_cell[2] >= number_cells.z) {
          continue;
        }

        uint cell_index = neighbour_cell[0] +
                          neighbour_cell[1] * number_cells.x +
                          neighbour_cell[2] * number_cells.x * number_cells.y;

#if defined(USE_LINKEDCELL)
        int next = cells[cell_index];

        while (next != END_OF_CELL_LIST) {
          if (i != next) {
            float4 r = predicted[i] - predicted[next];
            float r_length_2 = (r.x * r.x + r.y * r.y + r.z * r.z);

            if (r_length_2 > 0.0f && r_length_2 < h2) {
              float4 v = velocities[next] - velocities[i];
              float poly6 = poly6_factor * (h2 - r_length_2)
                            * (h2 - r_length_2)
                            * (h2 - r_length_2);

              viscosity_sum += (1.0f / predicted[next].w) * v * poly6;

              // #if defined(USE_DEBUG)
              // printf("viscosity: i,j: %d,%d result: [%f,%f,%f] density: %f\n", i, next,
              //        v.x, v.y, v.z, predicted[j].w);
              // #endif // USE_DEBUG
            }
          }

          next = particles_list[next];
        }
#else
        int2 cellRange = foundCells[cell_index];
        if (cellRange.x == END_OF_CELL_LIST) continue;

        for (uint n = cellRange.x; n <= cellRange.y; ++n) {
          const int next = radixCells[n].y;

          if (i != next) {
            float4 r = predicted[i] - predicted[next];
            float r_length_2 = (r.x * r.x + r.y * r.y + r.z * r.z);

            if (r_length_2 > 0.0f && r_length_2 < h2) {
              float4 v = velocities[next] - velocities[i];
              float poly6 = poly6_factor * (h2 - r_length_2)
                            * (h2 - r_length_2)
                            * (h2 - r_length_2);

              viscosity_sum += (1.0f / predicted[next].w) * v * poly6;

              // #if defined(USE_DEBUG)
              // printf("viscosity: i,j: %d,%d result: [%f,%f,%f] density: %f\n", i, next,
              //        v.x, v.y, v.z, predicted[j].w);
              // #endif // USE_DEBUG
            }
          }
        }
#endif
      }
    }
  }

  const float c = 0.01f;
  deltaVelocities[i] = c * viscosity_sum;

  // #if defined(USE_DEBUG)
  // printf("viscosity: i: %d sum:%f result: [%f,%f,%f]\n", i,
  //        c * viscosity_sum.x, c * viscosity_sum.y, c * viscosity_sum.z);
  // #endif // USE_DEBUG
}
