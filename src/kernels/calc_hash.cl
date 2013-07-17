__kernel void calcHash(const __global hesp_float4 *predicted,
                       __global uint2 *radixCells,
                       const hesp_float4 cell_length,
                       const uint4 number_cells,
                       const hesp_float4 system_length_min,
                       const uint maxInt,
                       const uint numParticles,
                       const uint numKeys)
{
    // Get particle and assign them to a cell
    const uint i = get_global_id(0);

    // #if defined(USE_DEBUG)
    // printf("%d\n", i);
    // #endif // USE_DEBUG

    if (i < numParticles)
    {
        // Get cell that belongs to particle
        uint cell_pos = (uint) ( (predicted[i].x - system_length_min.x)
                                 / cell_length.x )
                        + (uint) ( (predicted[i].y - system_length_min.y)
                                   / cell_length.y ) * number_cells.x
                        + (uint) ( (predicted[i].z - system_length_min.z)
                                   / cell_length.z ) * number_cells.x * number_cells.y;

        radixCells[i] = (uint2)(cell_pos, i);
    }
    else
    {
        radixCells[i] = (uint2)(maxInt, 0);
    }
}
