#include "Grid.h"

void Grid::insert(double x, double y, User* u)
{
    int x_cell; int y_cell;
    std::map<int, User*>* cell_to_insert = find_cell(x, y, x_cell, y_cell);
    auto ret = cell_to_insert->insert(std::make_pair(u->id, u));
    size++;
    checkins_count[x_cell][y_cell]++;
    total_checkins_count++;
}

bool Grid::remove(double x, double y, User* u)
{
    int x_cell; int y_cell;
    std::map<int, User*>* cell_to_delete = find_cell(x, y, x_cell, y_cell);
    int deleted_count = cell_to_delete->erase(u->id);
    size--;
    return deleted_count == 1;
}


bool Grid::check_cell(double x, double y, double range, int x_cell, int y_cell, std::vector<User*>* res)
{
    if (x_cell < 0 || x_cell >= no_cells_per_dim || y_cell < 0 || y_cell >= no_cells_per_dim)
        return false;
    if (!cell_within_range(x, y, range, x_cell, y_cell))
        return false;

    for (auto it = grid[x_cell][y_cell].begin(); it != grid[x_cell][y_cell].end(); it++)
    {
        if (distance(x, y, it->second->location->x, it->second->location->y)<=range)
            res->push_back(it->second);
    }
    return true;
}


