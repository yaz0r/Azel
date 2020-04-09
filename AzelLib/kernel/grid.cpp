#include "PDS.h"
#include "grid.h"

const sGrid* readGrid(sSaturnPtr source, int sizeX, int sizeY)
{
    sGrid* pGrid = new sGrid;

    pGrid->cells.resize(sizeX);
    for (int x=0; x<sizeX; x++)
    {
        pGrid->cells[x].resize(sizeY);
        for (int y=0; y<sizeY; y++)
        {
            pGrid->cells[x][y] = readSaturnEA(source + 4 * (sizeX * y + x));
        }
    }

    return pGrid;
}

