__kernel void findCells(const __global uint2 *cells,
                        const __global int2 *foundCells,
                        const uint numParticles)
{
    const int i = get_global_id(0);
    if (i >= numParticles) return;

    if (i == 0)
    {
        foundCells[cells[i].x].x = i;
    }
    else
    {
        if (cells[i].x != cells[i - 1].x)
        {
            foundCells[cells[i - 1].x].y = i - 1;
            foundCells[cells[i].x].x = i;
        }

        if (i == numParticles - 1)
        {
            foundCells[cells[i].x].y = i;
        }
    }
}
