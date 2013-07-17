__kernel void updateCells(const __global hesp_float4 *predicted,
                          __global int *cells,
                          __global int *particles_list,
                          const hesp_float4 cell_length,
                          const uint4 number_cells,
                          const hesp_float4 system_length_min,
                          const uint N)
{
    // Get particle and assign them to a cell
    const uint i = get_global_id(0);
    if (i >= N) return;

    // Get cell that belongs to particle
    uint cell_pos = (int) ( (predicted[i].x - system_length_min.x)
                            / cell_length.x )
                    + (int) ( (predicted[i].y - system_length_min.y)
                              / cell_length.y ) * number_cells.x
                    + (int) ( (predicted[i].z - system_length_min.z)
                              / cell_length.z ) * number_cells.x * number_cells.y;

    // Exchange cells[cell_pos] and particle_list at i
    particles_list[i] = atomic_xchg(&cells[cell_pos], i);

    // #if defined(USE_DEBUG)
    // printf("UPDATE_CELL: %d cell_pos:%d\n", i, cell_pos);
    // #endif
}
