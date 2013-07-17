// each virtual processor reorders its data using the scanned histogram
__kernel void reorder(const __global uint2 *radixCells,
                      __global uint2 *cells,
                      __global uint *histograms,
                      const uint pass,
                      __local uint *local_histograms,
                      const uint numKeys,
                      const uint _RADIX,
                      const uint _BITS)
{
    int it = get_local_id(0);
    int ig = get_global_id(0);

    int gr = get_group_id(0);
    int groups = get_num_groups(0);
    int items = get_local_size(0);

    int start = ig * (numKeys / groups / items);
    int size = numKeys / groups / items;

    // take the histogram in the cache
    for (int ir = 0; ir < _RADIX; ir++)
    {
        local_histograms[ir * items + it] =
            histograms[items * (ir * groups + gr) + it];
    }

    barrier(CLK_LOCAL_MEM_FENCE);

    int newpos, key, shortkey, k, newpost;

    for (int j = 0; j < size; j++)
    {
        k = j + start;
        key = radixCells[k].x;

        shortkey = ((key >> (pass * _BITS)) & (_RADIX - 1));

        newpost = local_histograms[shortkey * items + it];

        cells[newpost] = radixCells[k]; // killing line !!!

        newpos++;
        local_histograms[shortkey * items + it] = newpos;
    }
}
