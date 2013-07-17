__kernel void computeDelta(__global hesp_float4 *delta,
                           const __global hesp_float4 *predicted,
                           const __global hesp_float *scaling,
#if defined(USE_LINKEDCELL)
                           const __global int *cells,
                           const __global int *particles_list,
#else
                           const __global int2 *radixCells,
                           const __global int2 *foundCells,
#endif // USE_LINKEDCELL
                           const hesp_float timestep,
                           const hesp_float4 system_length_min,
                           const hesp_float4 system_length_max,
                           const hesp_float4 cell_length,
                           const int4 number_cells,
                           const hesp_float wave_generator,
                           const hesp_float rest_density,
                           const int N)
{
    const int i = get_global_id(0);
    if (i >= N) return;

    const int END_OF_CELL_LIST = -1;

    // smoothing radius
    const hesp_float h = cell_length.x;
    const hesp_float h2 = h * h;
    const hesp_float h6 = h2 * h2 * h2;
    const hesp_float h9 = h6 * h2 * h;
    const hesp_float poly6_factor = 315.0f / (64.0f * M_PI * h9);
    const hesp_float gradSpiky_factor = 45.0f / (M_PI * h6);

    int current_cell[3];

    current_cell[0] = (int) ( (predicted[i].x - system_length_min.x)
                              / cell_length.x );
    current_cell[1] = (int) ( (predicted[i].y - system_length_min.y)
                              / cell_length.y );
    current_cell[2] = (int) ( (predicted[i].z - system_length_min.z)
                              / cell_length.z );

    // Sum of lambdas
    hesp_float4 sum = (hesp_float4) 0.0f;

    for (int x = -1; x <= 1; ++x)
    {
        for (int y = -1; y <= 1; ++y)
        {
            for (int z = -1; z <= 1; ++z)
            {
                int neighbour_cell[3];

                neighbour_cell[0] = current_cell[0] + x;
                neighbour_cell[1] = current_cell[1] + y;
                neighbour_cell[2] = current_cell[2] + z;

                if (neighbour_cell[0] < 0 || neighbour_cell[0] >= number_cells.x ||
                        neighbour_cell[1] < 0 || neighbour_cell[1] >= number_cells.y ||
                        neighbour_cell[2] < 0 || neighbour_cell[2] >= number_cells.z)
                {
                    continue;
                }

                uint cell_index = neighbour_cell[0] +
                                  neighbour_cell[1] * number_cells.x +
                                  neighbour_cell[2] * number_cells.x * number_cells.y;

#if defined(USE_LINKEDCELL)
                // Next particle in list
                int next = cells[cell_index];

                while (next != END_OF_CELL_LIST)
                {
                    if (i != next)
                    {
                        hesp_float4 r = predicted[i] - predicted[next];
                        hesp_float r_length_2 = r.x * r.x + r.y * r.y + r.z * r.z;
                        hesp_float r_length = sqrt(r_length_2);

                        if (r_length > 0.0f && r_length < h)
                        {
                            hesp_float4 gradient_spiky = -1.0f * r / (r_length)
                                                         * gradSpiky_factor
                                                         * (h - r_length)
                                                         * (h - r_length);

                            hesp_float poly6_r = poly6_factor * (h2 - r_length_2)
                                                 * (h2 - r_length_2)
                                                 * (h2 - r_length_2);

                            // equation (13)
                            const hesp_float q = 0.3f * h;
                            hesp_float poly6_q = poly6_factor * (h2 - q)
                                                 * (h2 - q) * (h2 - q);
                            const hesp_float k = 0.1f;
                            const uint n = 4;

                            hesp_float s_corr = -1.0f * k * pow(poly6_r / poly6_q, n);

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

                for (uint n = cellRange.x; n <= cellRange.y; ++n)
                {
                    const int next = radixCells[n].y;

                    if (i != next)
                    {
                        hesp_float4 r = predicted[i] - predicted[next];
                        hesp_float r_length_2 = r.x * r.x + r.y * r.y + r.z * r.z;
                        hesp_float r_length = sqrt(r_length_2);

                        if (r_length > 0.0f && r_length < h)
                        {
                            hesp_float4 gradient_spiky = -1.0f * r / (r_length)
                                                         * gradSpiky_factor
                                                         * (h - r_length)
                                                         * (h - r_length);

                            hesp_float poly6_r = poly6_factor * (h2 - r_length_2)
                                                 * (h2 - r_length_2) * (h2 - r_length_2);

                            // equation (13) correction term
                            const hesp_float q = 0.3f * h;
                            hesp_float poly6_q = poly6_factor * (h2 - q)
                                                 * (h2 - q) * (h2 - q);
                            const hesp_float k = 0.1f;
                            const int n = 4;

                            hesp_float s_corr = -k * pow(poly6_r / poly6_q, n);

                            sum += (scaling[i] + scaling[next]) * gradient_spiky;
                        }
                    }
                }
#endif
            }
        }
    }

    // equation (12)
    hesp_float4 delta_p = sum / rest_density;

    hesp_float radius = h * 0.5f;
    hesp_float4 future = predicted[i] + delta_p;

    if ( (future.x - radius) < (system_length_min.x + wave_generator) )
    {
        future.x += 2.0f * (system_length_min.x + wave_generator - future.x + radius);
        //future.x = system_length_min.x + wave_generator + radius;
        //future.x += ((system_length_min.x + wave_generator) - (future.x - radius) ) * 2;
    }
    else if ( (future.x + radius) > system_length_max.x )
    {
        future.x -= 2.0f * (future.x + radius - system_length_max.x);
        //future.x = system_length_max.x - radius;
        //future.x += (system_length_max.x - (future.x + radius)) * 2;
    }

    if ( (future.y - radius) < system_length_min.y )
    {
        future.y += 2.0f * (system_length_min.y - future.y + radius);
        //future.y = system_length_min.y + radius;
        //future.y += (system_length_min.y - (future.y - radius)) * 2;
    }
    else if ( (future.y + radius) > system_length_max.y )
    {
        future.y -= 2.0f * (future.y + radius - system_length_max.y);
        //future.y = system_length_max.y - radius;
        //future.y += (system_length_max.y - (future.y + radius)) * 2;
    }

    if ( (future.z - radius) < system_length_min.z )
    {
        future.z += 2.0f * (system_length_min.z - future.z + radius);
        //future.z = system_length_min.z + radius;
        //future.z += (system_length_min.z - (future.z - radius)) * 2;
    }
    else if ( (future.z + radius) > system_length_max.z )
    {
        future.z -= 2.0f * (future.z + radius - system_length_max.z);
        //future.z = system_length_max.z - radius;
        //future.z += (system_length_max.z - (future.z + radius)) * 2;
    }

    delta[i] = future - predicted[i];

    // #if defined(USE_DEBUG)
    //     printf("compute_delta: result: i: %d\ndelta: [%f,%f,%f]\n",
    //            i,
    //            delta[i].x, delta[i].y, delta[i].z);
    // #endif
}
