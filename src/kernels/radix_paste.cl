// use the global sum for updating the local histograms
// each work item updates two values
__kernel void paste( __global int *histograms, __global int *globsum) {
  int ig = get_global_id(0);
  int gr = get_group_id(0);

  int s;

  s = globsum[gr];

  // write results to device memory
  histograms[2 * ig] += s;
  histograms[2 * ig + 1] += s;

  barrier(CLK_GLOBAL_MEM_FENCE);
}
