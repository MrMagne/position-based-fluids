__kernel void predictPositions(const __global hesp_float4 *positions,
                               __global hesp_float4 *predicted,
                               const __global hesp_float4 *velocities,
                               const hesp_float4 system_length_min,
                               const hesp_float4 system_length_max,
                               const hesp_float timestep,
                               const uint N)
{
    const uint i = get_global_id(0);
    if (i >= N) return;

    predicted[i].xyz = positions[i].xyz + timestep * velocities[i].xyz;
}
