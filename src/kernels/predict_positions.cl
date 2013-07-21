__kernel void predictPositions(const __global float4 *positions,
                               __global float4 *predicted,
                               __global float4 *velocities,
                               const uint N) {
  const uint i = get_global_id(0);
  if (i >= N) return;

  velocities[i].xyz = velocities[i].xyz + TIMESTEP * (float3)(0.0f, -9.81f, 0.0f);
  predicted[i].xyz = positions[i].xyz + TIMESTEP * velocities[i].xyz;
}
