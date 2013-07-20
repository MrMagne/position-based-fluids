__kernel void predictPositions(const __global float4 *positions,
                               __global float4 *predicted,
                               const __global float4 *velocities,
                               const float4 system_length_min,
                               const float4 system_length_max,
                               const float timestep,
                               const uint N) {
  const uint i = get_global_id(0);
  if (i >= N) return;

  predicted[i].xyz = positions[i].xyz + timestep * velocities[i].xyz;
}
