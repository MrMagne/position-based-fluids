__constant float3 gravity = (float3)(0.0f, -9.81f, 0.0f);

__kernel void predictPositions(const __global float4 *positions,
                               __global float4 *predicted,
                               __global float4 *velocities,
                               const float4 system_length_min,
                               const float4 system_length_max,
                               const float timestep,
                               const uint N) {
  const uint i = get_global_id(0);
  if (i >= N) return;

  velocities[i].xyz = velocities[i].xyz + timestep * gravity.xyz;
  predicted[i].xyz = positions[i].xyz + timestep * velocities[i].xyz;
}
