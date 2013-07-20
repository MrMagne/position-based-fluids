__kernel void updatePredicted(__global float4 *predicted,
                              const __global float4 *delta,
                              const uint N)
{
    const uint i = get_global_id(0);
    if (i >= N) return;

    predicted[i].xyz = predicted[i].xyz + delta[i].xyz;

    // #if defined(USE_DEBUG)
    //     // printf("UPDATE_PREDICTED: %d: predict:[%f,%f,%f]\n",
    //     //        i,
    //     //        predicted[i].x, predicted[i].y, predicted[i].z);
    // #endif
}
