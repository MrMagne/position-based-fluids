__kernel void computeScaling(__global float4 *predicted,
                             __global float *scaling,
#if defined(USE_LINKEDCELL)
                             const __global int *cells,
                             const __global int *particles_list,
#else
                             const __global int2 *radixCells,
                             const __global int2 *foundCells,
#endif // USE_LINKEDCELL
                             const int N) {
  // Scaling = lambda
  const int i = get_global_id(0);
  if (i >= N) return;

  const int END_OF_CELL_LIST = -1;
  const float e = 10000.0f;

  // calculate $$$\Delta p_i$$$
  int current_cell[3];

  current_cell[0] = (int) ( (predicted[i].x - SYSTEM_MIN_X)
                            / CELL_LENGTH_X );
  current_cell[1] = (int) ( (predicted[i].y - SYSTEM_MIN_Y)
                            / CELL_LENGTH_Y );
  current_cell[2] = (int) ( (predicted[i].z - SYSTEM_MIN_Z)
                            / CELL_LENGTH_Z );

  // Sum of rho_i, |nabla p_k C_i|^2 and nabla p_k C_i for k = i
  float density_sum = 0.0f;
  float gradient_sum_k = 0.0f;
  float3 gradient_sum_k_i = (float3) 0.0f;

  int neighbour_cell[3];
#pragma unroll 3
  for (int x = -1; x <= 1; ++x) {
#pragma unroll 3
    for (int y = -1; y <= 1; ++y) {
#pragma unroll 3
      for (int z = -1; z <= 1; ++z) {

        neighbour_cell[0] = current_cell[0] + x;
        neighbour_cell[1] = current_cell[1] + y;
        neighbour_cell[2] = current_cell[2] + z;

        if (neighbour_cell[0] < 0 || neighbour_cell[0] >= NUMBER_OF_CELLS_X ||
            neighbour_cell[1] < 0 || neighbour_cell[1] >= NUMBER_OF_CELLS_Y ||
            neighbour_cell[2] < 0 || neighbour_cell[2] >= NUMBER_OF_CELLS_Z) {
          continue;
        }

        uint cell_index = neighbour_cell[0] +
                          neighbour_cell[1] * NUMBER_OF_CELLS_X +
                          neighbour_cell[2] * NUMBER_OF_CELLS_X * NUMBER_OF_CELLS_Y;

#if defined(USE_LINKEDCELL)
        // Next particle in list
        int next = cells[cell_index];

        while (next != END_OF_CELL_LIST) {
          if (i != next) {
            float3 r = predicted[i].xyz - predicted[next].xyz;
            float r_length_2 = (r.x * r.x + r.y * r.y + r.z * r.z);

            // If h == r every term gets zero, so < h not <= h
            if (r_length_2 > 0.0f && r_length_2 < PBF_H_2) {
              float r_length = sqrt(r_length_2);

              //CAUTION: the two spiky kernels are only the same
              //because the result is only used sqaured
              // equation (8), if k = i
              float3 gradient_spiky = r / (r_length)
                                      * GRAD_SPIKY_FACTOR
                                      * (PBF_H - r_length)
                                      * (PBF_H - r_length);

              // equation (2)
              float poly6 = POLY6_FACTOR * (PBF_H_2 - r_length_2)
                            * (PBF_H_2 - r_length_2)
                            * (PBF_H_2 - r_length_2);
              density_sum += poly6;

              // equation (9), denominator, if k = j
              gradient_sum_k += length(gradient_spiky);

              // equation (8), if k = i
              gradient_sum_k_i += gradient_spiky;
            }
          }

          next = particles_list[next];
        }
#else
        int2 cellRange = foundCells[cell_index];
        if (cellRange.x == END_OF_CELL_LIST) continue;

        for (uint r = cellRange.x; r <= cellRange.y; ++r) {
          const int next = radixCells[r].y;

          if (i != next) {
            float3 r = predicted[i].xyz - predicted[next].xyz;
            float r_length_2 = (r.x * r.x + r.y * r.y + r.z * r.z);

            // If h == r every term gets zero, so < h not <= h
            if (r_length_2 > 0.0f && r_length_2 < h2) {
              float r_length = sqrt(r_length_2);

              //CAUTION: the two spiky kernels are only the same
              //because the result is only used sqaured
              // equation (8), if k = j
              float3 gradient_spiky = r / (r_length)
                                      * gradSpiky_factor
                                      * (h - r_length)
                                      * (h - r_length);

              // equation (2)
              float poly6 = poly6_factor * (h2 - r_length_2)
                            * (h2 - r_length_2) * (h2 - r_length_2);
              density_sum += poly6;

              // equation (9), denominator, if k = j
              gradient_sum_k += length(gradient_spiky);

              // equation (8), if k = i
              gradient_sum_k_i += gradient_spiky;
            }
          }
        }
#endif
      }
    }
  }

  // equation (9), denominator, if k = i
  gradient_sum_k += length(gradient_sum_k_i);

  predicted[i].w = density_sum;

  // equation (1)
  float density_constraint = (density_sum / REST_DENSITY) - 1.0f;

  // equation (11)
  scaling[i] = -1.0f * density_constraint / (gradient_sum_k * gradient_sum_k
               / (REST_DENSITY * REST_DENSITY) + e);
}
