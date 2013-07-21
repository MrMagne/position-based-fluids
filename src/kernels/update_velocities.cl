__kernel void updateVelocities(const __global float4 *positions,
                               const __global float4 *predicted,
                               __global float4 *velocities,
                               const float timestep,
                               const uint N) {
  const uint i = get_global_id(0);
  if (i >= N) return;

  velocities[i].xyz = (predicted[i].xyz - positions[i].xyz) / timestep;

  // #if defined(USE_DEBUG)
  // printf("updateVelocites: i,t: %d,%f\npos: [%f,%f,%f]\npredict: [%f,%f,%f]\nvel: [%f,%f,%f]\n",
  //        i, timestep, positions[i].x, positions[i].y, positions[i].z,
  //        predicted[i].x, predicted[i].y, predicted[i].z,
  //        velocities[i].x, velocities[i].y, velocities[i].z);
  // #endif
}
