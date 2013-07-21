__kernel void computeDelta(__global float4 *delta,
                           const __global float4 *predicted,
                           const __global float *scaling,
#if defined(USE_LINKEDCELL)
                           const __global int *cells,
                           const __global int *particles_list,
#else
                           const __global int2 *radixCells,
                           const __global int2 *foundCells,
#endif // USE_LINKEDCELL
                           const float wave_generator,
                           const int N) {
  const int i = get_global_id(0);
  if (i >= N) return;

  const int END_OF_CELL_LIST = -1;

  int current_cell[3];

  current_cell[0] = (int) ( (predicted[i].x - SYSTEM_MIN_X)
                            / CELL_LENGTH_X );
  current_cell[1] = (int) ( (predicted[i].y - SYSTEM_MIN_Y)
                            / CELL_LENGTH_Y );
  current_cell[2] = (int) ( (predicted[i].z - SYSTEM_MIN_Z)
                            / CELL_LENGTH_Z );

  // Sum of lambdas
  float4 sum = (float4) 0.0f;

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
            float4 r = predicted[i] - predicted[next];
            float r_length_2 = r.x * r.x + r.y * r.y + r.z * r.z;

            if (r_length_2 > 0.0f && r_length_2 < PBF_H_2) {
              float r_length = sqrt(r_length_2);
              float4 gradient_spiky = -1.0f * r / (r_length)
                                      * GRAD_SPIKY_FACTOR
                                      * (PBF_H - r_length)
                                      * (PBF_H - r_length);

              // float poly6_r = poly6_factor * (PBF_H_2 - r_length_2)
              //                 * (PBF_H_2 - r_length_2)
              //                 * (PBF_H_2 - r_length_2);

              // // equation (13)
              // const float q = 0.3f * h;
              // float poly6_q = poly6_factor * (h2 - q)
              //                 * (h2 - q) * (h2 - q);
              // const float k = 0.1f;
              // const uint n = 4;

              // float s_corr = -1.0f * k * pow(poly6_r / poly6_q, n);

              // Sum for delta p of scaling factors and grad spiky
              // in equation (12)
              sum += (scaling[i] + scaling[next]) * gradient_spiky;
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
            float r_length_2 = r.x * r.x + r.y * r.y + r.z * r.z;

            if (r_length_2 > 0.0f && r_length_2 < h2) {
              float r_length = sqrt(r_length_2);
              float4 gradient_spiky = -1.0f * r / (r_length)
                                      * gradSpiky_factor
                                      * (h - r_length)
                                      * (h - r_length);

              float poly6_r = poly6_factor * (h2 - r_length_2)
                              * (h2 - r_length_2) * (h2 - r_length_2);

              // equation (13) correction term
              const float q = 0.3f * h;
              float poly6_q = poly6_factor * (h2 - q)
                              * (h2 - q) * (h2 - q);
              const float k = 0.1f;
              const int n = 4;

              float s_corr = -k * pow(poly6_r / poly6_q, n);

              sum += (scaling[i] + scaling[next]) * gradient_spiky;
            }
          }
        }
#endif
      }
    }
  }

  // equation (12)
  float4 delta_p = sum / REST_DENSITY;

  float4 future = predicted[i] + delta_p;

  if ( (future.x - PBF_H) < (SYSTEM_MIN_X + wave_generator) ) {
    future.x = SYSTEM_MIN_X + wave_generator + PBF_H;
    //future.x += ((system_length_min.x + wave_generator) - (future.x - radius) ) * 2.0f;
  } else if ( (future.x + PBF_H) > SYSTEM_MAX_X ) {
    future.x = SYSTEM_MAX_X - PBF_H;
    //future.x += (system_length_max.x - (future.x + radius)) * 2.0f;
  }
  if ( (future.y - PBF_H) < SYSTEM_MIN_Y ) {
    future.y = SYSTEM_MIN_Y + PBF_H;
    //future.y += (system_length_min.y - (future.y - radius)) * 2.0f;
  } else if ( (future.y + PBF_H) > SYSTEM_MAX_Y ) {
    future.y = SYSTEM_MAX_Y - PBF_H;
    //future.y += (system_length_max.y - (future.y + radius)) * 2.0f;
  }
  if ( (future.z - PBF_H) < SYSTEM_MIN_Z ) {
    future.z = SYSTEM_MIN_Z + PBF_H;
    //future.z += (system_length_min.z - (future.z - radius)) * 2.0f;
  } else if ( (future.z + PBF_H) > SYSTEM_MAX_Z ) {
    future.z = SYSTEM_MAX_Z - PBF_H;
    //future.z += (system_length_max.z - (future.z + radius)) * 2.0f;
  }

  delta[i] = future - predicted[i];

  // #if defined(USE_DEBUG)
  //     printf("compute_delta: result: i: %d\ndelta: [%f,%f,%f]\n",
  //            i,
  //            delta[i].x, delta[i].y, delta[i].z);
  // #endif
}
