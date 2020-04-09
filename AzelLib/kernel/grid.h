#pragma once

struct sGrid
{
    std::vector < std::vector<sSaturnPtr>> cells;
};

const struct sGrid* readGrid(sSaturnPtr source, int sizeX, int sizeY);
