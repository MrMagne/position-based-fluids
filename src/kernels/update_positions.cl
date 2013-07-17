__kernel void updatePositions(__global hesp_float4 *positions,
                              const __global hesp_float4 *predicted,
                              const __global hesp_float4 *velocities,
                              const uint N)
{
    const uint i = get_global_id(0);
    if (i >= N) return;

    positions[i].xyz = predicted[i].xyz;
    positions[i].w = length(velocities[i].xyz);

    // #if defined(USE_DEBUG)
    // printf("%d: pos:[%f,%f,%f]\nvel: [%f,%f,%f]\n", i,
    //        positions[i].x, positions[i].y, positions[i].z,
    //        velocities[i].x, velocities[i].y, velocities[i].z
    //       );
    // #endif
}
