__kernel void scan(__global uint *histograms,
                   __local uint *temp,
                   __global uint *globsum) {
  int it = get_local_id(0);
  int ig = get_global_id(0);
  int decale = 1;
  int n = get_local_size(0) * 2 ;
  int gr = get_group_id(0);

  // load input into local memory
  // up sweep phase
  temp[2 * it] = histograms[2 * ig];
  temp[2 * it + 1] = histograms[2 * ig + 1];

  // parallel prefix sum (algorithm of Blelloch 1990)
  for (int d = n >> 1; d > 0; d >>= 1) {
    barrier(CLK_LOCAL_MEM_FENCE);

    if (it < d) {
      int ai = decale * (2 * it + 1) - 1;
      int bi = decale * (2 * it + 2) - 1;
      temp[bi] += temp[ai];
    }

    decale *= 2;
  }

  // store the last element in the global sum vector
  // (maybe used in the next step for constructing the global scan)
  // clear the last element
  if (it == 0) {
    globsum[gr] = temp[n - 1];
    temp[n - 1] = 0;
  }

  // down sweep phase
  for (int d = 1; d < n; d *= 2) {
    decale >>= 1;
    barrier(CLK_LOCAL_MEM_FENCE);

    if (it < d) {
      int ai = decale * (2 * it + 1) - 1;
      int bi = decale * (2 * it + 2) - 1;

      int t = temp[ai];
      temp[ai] = temp[bi];
      temp[bi] += t;
    }

  }

  barrier(CLK_LOCAL_MEM_FENCE);

  // write results to device memory
  histograms[2 * ig] = temp[2 * it];
  histograms[2 * ig + 1] = temp[2 * it + 1];

  barrier(CLK_GLOBAL_MEM_FENCE);
}
