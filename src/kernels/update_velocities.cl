__constant float3 gravity = (float3)(0.0f, -9.81f, 0.0f);

__kernel void updateVelocities(const __global float4 *positions,
                               const __global float4 *predicted,
                               __global float4 *velocities,
                               const __global float4 *deltaVelocities,
                               const __global float4 *vorticity_forces,
                               const float timestep,
                               const uint N)
{
    const uint i = get_global_id(0);
    if (i >= N) return;

    velocities[i].xyz = (predicted[i].xyz - positions[i].xyz) / timestep
                        + deltaVelocities[i].xyz + timestep * gravity.xyz;

    // Vorticity
    velocities[i].xyz += timestep * vorticity_forces[i].xyz;

    // #if defined(USE_DEBUG)
    // printf("updateVelocites: i,t: %d,%f\npos: [%f,%f,%f]\npredict: [%f,%f,%f]\nvel: [%f,%f,%f]\n",
    //        i, timestep, positions[i].x, positions[i].y, positions[i].z,
    //        predicted[i].x, predicted[i].y, predicted[i].z,
    //        velocities[i].x, velocities[i].y, velocities[i].z);
    // #endif
}
