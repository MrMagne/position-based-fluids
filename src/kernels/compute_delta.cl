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
                           const float timestep,
                           const float4 system_length_min,
                           const float4 system_length_max,
                           const float4 cell_length,
                           const int4 number_cells,
                           const float wave_generator,
                           const float rest_density,
                           const int N) {
  const int i = get_global_id(0);
  if (i >= N) return;

  const int END_OF_CELL_LIST = -1;

  // smoothing radius
  const float h = cell_length.x;
  const float h2 = h * h;
  const float h6 = h2 * h2 * h2;
  const float h9 = h6 * h2 * h;
  const float poly6_factor = 315.0f / (64.0f * M_PI * h9);
  const float gradSpiky_factor = 45.0f / (M_PI * h6);

  int current_cell[3];

  current_cell[0] = (int) ( (predicted[i].x - system_length_min.x)
                            / cell_length.x );
  current_cell[1] = (int) ( (predicted[i].y - system_length_min.y)
                            / cell_length.y );
  current_cell[2] = (int) ( (predicted[i].z - system_length_min.z)
                            / cell_length.z );

  // Sum of lambdas
  float4 sum = (float4) 0.0f;

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
        // Next particle in list
        int next = cells[cell_index];

        while (next != END_OF_CELL_LIST) {
          if (i != next) {
            float4 r = predicted[i] - predicted[next];
            float r_length_2 = r.x * r.x + r.y * r.y + r.z * r.z;
            float r_length = sqrt(r_length_2);

            if (r_length > 0.0f && r_length < h) {
              float4 gradient_spiky = -1.0f * r / (r_length)
                                      * gradSpiky_factor
                                      * (h - r_length)
                                      * (h - r_length);

              float poly6_r = poly6_factor * (h2 - r_length_2)
                              * (h2 - r_length_2)
                              * (h2 - r_length_2);

              // equation (13)
              const float q = 0.3f * h;
              float poly6_q = poly6_factor * (h2 - q)
                              * (h2 - q) * (h2 - q);
              const float k = 0.1f;
              const uint n = 4;

              float s_corr = -1.0f * k * pow(poly6_r / poly6_q, n);

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
            float r_length = sqrt(r_length_2);

            if (r_length > 0.0f && r_length < h) {
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
  float4 delta_p = sum / rest_density;

  float radius = h;
  float4 future = predicted[i] + delta_p;

  if ( (future.x - radius) < (system_length_min.x + wave_generator) ) {
    future.x = system_length_min.x + wave_generator + radius;
    //future.x += ((system_length_min.x + wave_generator) - (future.x - radius) ) * 2.0f;
  } else if ( (future.x + radius) > system_length_max.x ) {
    future.x = system_length_max.x - radius;
    //future.x += (system_length_max.x - (future.x + radius)) * 2.0f;
  }
  if ( (future.y - radius) < system_length_min.y ) {
    future.y = system_length_min.y + radius;
    //future.y += (system_length_min.y - (future.y - radius)) * 2.0f;
  } else if ( (future.y + radius) > system_length_max.y ) {
    future.y = system_length_max.y - radius;
    //future.y += (system_length_max.y - (future.y + radius)) * 2.0f;
  }
  if ( (future.z - radius) < system_length_min.z ) {
    future.z = system_length_min.z + radius;
    //future.z += (system_length_min.z - (future.z - radius)) * 2.0f;
  } else if ( (future.z + radius) > system_length_max.z ) {
    future.z = system_length_max.z - radius;
    //future.z += (system_length_max.z - (future.z + radius)) * 2.0f;
  }

  delta[i] = future - predicted[i];

  // #if defined(USE_DEBUG)
  //     printf("compute_delta: result: i: %d\ndelta: [%f,%f,%f]\n",
  //            i,
  //            delta[i].x, delta[i].y, delta[i].z);
  // #endif
}
