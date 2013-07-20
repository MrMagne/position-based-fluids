__kernel void histogram(const __global uint2 *radixCells,
                        __global uint *histograms,
                        const uint pass,
                        __local uint *local_histograms,
                        const uint numKeys,
                        const uint _RADIX,
                        const uint _BITS) {
  int it = get_local_id(0);  // i local number of the processor
  int ig = get_global_id(0); // global number = i + g I

  int gr = get_group_id(0); // g group number

  int groups = get_num_groups(0);
  int items = get_local_size(0);

  // set the local histograms to zero
  for (int ir = 0; ir < _RADIX; ir++) {
    local_histograms[ir * items + it] = 0;
  }

  barrier(CLK_LOCAL_MEM_FENCE);

  // range of keys that are analyzed by the work item
  int size = numKeys / groups / items; // size of the sub-list
  int start = ig * size; // beginning of the sub-list

  int key = 0, shortkey = 0, k = 0;

  for (int j = 0; j < size; j++) {
    k = j + start;
    key = radixCells[k].x;

    // extract the group of _BITS bits of the pass
    // the result is in the range 0.._RADIX-1
    shortkey = ( ( key >> (pass * _BITS)) & (_RADIX - 1) );

    // #if defined(USE_DEBUG)
    // printf("%d: k=%d, key=%d, shortkey=%d\n", ig, k, key, shortkey);
    // #endif // USE_DEBUG

    // increment the local histogram
    local_histograms[shortkey *  items + it]++;
  }

  barrier(CLK_LOCAL_MEM_FENCE);

  // copy the local histogram to the global one
  for (int ir = 0; ir < _RADIX; ir++) {
    histograms[items * (ir * groups + gr) + it] = local_histograms[ir * items + it];
    // #if defined(USE_DEBUG)
    // printf("%d: [%d] = [%d]%d\n", ig, items * (ir * groups + gr) + it, ir * items + it, local_histograms[ir * items + it]);
    // #endif // USE_DEBUG
  }

  barrier(CLK_GLOBAL_MEM_FENCE);

  // #if defined(USE_DEBUG)
  // //printf("pass: %d\n", pass);

  // printf("(it, ig, gr, groups, items,size,start)=(%d,%d,%d,%d,%d,%d,%d)\n", it, ig, gr, groups, items, size, start);
  // #endif // USE_DEBUG

}
