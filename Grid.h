#ifndef GRID_H
#define GRID_H
#include <vector>
#include <string>
#include <map>
#include <functional>
#include "Utils.h"
#include "Simulator.h"

double distance(double x1, double y1, double x2, double y2);
struct User;

class Grid
{
public:
    Grid(int no_cells_per_dim, double min_x, double max_x, double min_y, double max_y, std::string name="")
    {
        this->name=name;
        this->size = 0;

        checkins_count_curr = new int*[no_cells_per_dim];
        for (int i = 0; i < no_cells_per_dim; i++)
        {
            checkins_count_curr[i] = new int[no_cells_per_dim];
            for (int j = 0; j < no_cells_per_dim; j++)
                checkins_count_curr[i][j] = 0;
        }

        checkins_count = new int*[no_cells_per_dim];
        for (int i = 0; i < no_cells_per_dim; i++)
        {
            checkins_count[i] = new int[no_cells_per_dim];
            for (int j = 0; j < no_cells_per_dim; j++)
                checkins_count[i][j] = 0;
        }

        grid = new std::map<int, User*>*[no_cells_per_dim];
        for (int i = 0; i < no_cells_per_dim; i++)
            grid[i] = new std::map<int, User*>[no_cells_per_dim];

        total_checkins_count = 1;
        total_checkins_count_curr = 1;
        this->no_cells_per_dim = no_cells_per_dim;
        double eps = 0.00000001;
        this->min_x = min_x-eps;
        this->min_y = min_y-eps;
        this->max_x = max_x+eps;
        this->max_y = max_y+eps;
    }

    void insert(double x, double y, User* u);

    void move(double old_x, double old_y, double new_x, double new_y, User* u)
    {
        if (remove(old_x, old_y, u))
            insert(new_x, new_y, u);
        else
            throw std::runtime_error("User wasn't there!");
    }

    bool remove(double x, double y, User* u);

    long get_total_checkin_count()
    {
        return total_checkins_count_curr;
    }

    void inc_checkins_count(double x, double y)
    {
        int x_cell; int y_cell;
        find_cell(x, y, x_cell, y_cell);
        checkins_count[x_cell][y_cell]++;
        total_checkins_count++;
    }


    int get_user_count(double x, double y)
    {
        int x_cell; int y_cell;
        find_cell(x, y, x_cell, y_cell);
        return grid[x_cell][y_cell].size();
    }
    int get_checkins_count(double x, double y)
    {
        int x_cell; int y_cell;
        find_cell(x, y, x_cell, y_cell);
        return checkins_count_curr[x_cell][y_cell];
    }

    void reset_checkin_counts()
    {
        for (int i = 0; i < no_cells_per_dim; i++)
        {
            for (int j = 0; j < no_cells_per_dim; j++)
            {
                checkins_count_curr[i][j] = checkins_count[i][j];
                checkins_count[i][j] = 0;
            }
        }
        total_checkins_count_curr = total_checkins_count;
        total_checkins_count = 0;
    }

    float for_each_user(std::function< float(User*) > f)
    {
        float res = 0;
        for (int i = 0; i < no_cells_per_dim; i++)
        {
            for (int j = 0; j < no_cells_per_dim; j++)
            {
                for (auto it = grid[i][j].begin(); it != grid[i][j].end(); it++)
                {
                    res += f(it->second);
                }
            }
        }
        return res;
    }
    

    void find_users_within_range(double x, double y, double range, std::vector<User*>* res)
    {
        int x_cell; int y_cell;
        find_cell(x, y, x_cell, y_cell);

        bool should_check = true;
        for (int i = 0; i < no_cells_per_dim; i++)
        {
            int j_p;
            for (j_p = 0; j_p < no_cells_per_dim; j_p++)
            {
                should_check = check_cell(x, y, range, x_cell + i, y_cell+j_p, res); if (j_p != 0)
                    should_check = should_check || check_cell(x, y, range, x_cell + i, y_cell-j_p, res);

                if (!should_check)
                    break;
            }

            if (i!=0)
            {
                int j_n;
                for (j_n = 0; j_n < no_cells_per_dim; j_n++)
                {
                    should_check = check_cell(x, y, range, x_cell - i, y_cell+j_n, res);
                    if (j_n != 0)
                        should_check = should_check || check_cell(x, y, range, x_cell - i, y_cell-j_n, res);

                    if (!should_check)
                        break;
                }
            
                if (j_p == 0 && j_n == 0)
                    break;
            }
        }
    }

    int get_cell_size(double x, double y)
    {
        int x_cell; int y_cell;
        find_cell(x, y, x_cell, y_cell);
        return grid[x_cell][y_cell].size();
    }

    double get_cell_area()
    {
        return get_x_gran()*get_y_gran();

    }

    int get_size()
    {
        return size;
    }

    void get_rand_within_cell(double x, double y, double& rand_x, double& rand_y)
    {
        int x_cell; int y_cell;
        find_cell(x, y, x_cell, y_cell);

        double cell_y_begin = get_y_gran()*(y_cell)+min_y;
        double cell_y_end = get_y_gran()*(y_cell+1)+min_y;
        double cell_x_begin = get_x_gran()*(x_cell)+min_x;
        double cell_x_end = get_x_gran()*(x_cell+1)+min_x;

        
        rand_x = (rand()/(double)INT_MAX)*(cell_x_end-cell_x_begin)+cell_x_begin;
        rand_y = (rand()/(double)INT_MAX)*(cell_y_end-cell_y_begin)+cell_y_begin;

    }

private:
    bool check_cell(double x, double y, double range, int x_cell, int y_cell, std::vector<User*>* res);
    


    bool cell_within_range(double x, double y, double range, int x_cell, int y_cell)
    {
        if (is_inside(x, y, x_cell, y_cell))
            return true;
        double x_corner;
        double y_corner;
        double min_dist = closest_corner(x, y, x_cell, y_cell, x_corner, y_corner);
        return range >= min_dist;

    }

    bool is_inside(double x, double y, int x_cell, int y_cell)
    {
        double cell_y_begin = get_y_gran()*(y_cell)+min_y;
        if (y_cell == 0)
            cell_y_begin = -180;
        double cell_y_end = get_y_gran()*(y_cell+1)+min_y;
        if (y_cell == no_cells_per_dim)
            cell_y_end = 180;
        double cell_x_begin = get_x_gran()*(x_cell)+min_x;
        if (x_cell == 0)
            cell_x_begin = -180;
        double cell_x_end = get_x_gran()*(x_cell+1)+min_x;
        if (x_cell == no_cells_per_dim)
            cell_x_end = 180;

        if (y <= cell_y_end && y>=cell_y_begin && x <= cell_x_end && x>=cell_x_begin)
            return true;
        return false;
    }

    double closest_corner(double x, double y, int x_cell, int y_cell, double& x_corner, double& y_corner)
    {
        double cell_y_begin = get_y_gran()*(y_cell)+min_y;
        if (y_cell == 0)
            cell_y_begin = -180;
        double cell_y_end = get_y_gran()*(y_cell+1)+min_y;
        if (y_cell == no_cells_per_dim)
            cell_y_end = 180;
        double cell_x_begin = get_x_gran()*(x_cell)+min_x;
        if (x_cell == 0)
            cell_x_begin = -180;
        double cell_x_end = get_x_gran()*(x_cell+1)+min_x;
        if (x_cell == no_cells_per_dim)
            cell_x_end = 180;

        double min_dist = distance(x, y, cell_x_begin, cell_y_begin);
        x_corner = cell_x_begin;
        y_corner = cell_y_begin;
        if (distance(x, y, cell_x_end, cell_y_begin) < min_dist)
        {
            min_dist = distance(x, y, cell_x_end, cell_y_begin);
            x_corner = cell_x_end;
            y_corner = cell_y_begin;
        }
        if (distance(x, y, cell_x_begin, cell_y_end) < min_dist)
        {
            min_dist = distance(x, y, cell_x_begin, cell_y_end);
            x_corner = cell_x_begin;
            y_corner = cell_y_end;
        }
        if (distance(x, y, cell_x_end, cell_y_end) < min_dist)
        {
            min_dist = distance(x, y, cell_x_end, cell_y_end);
            x_corner = cell_x_end;
            y_corner = cell_y_end;
        }
        return min_dist;
    }

    double get_x_gran()
    {
        return (max_x-min_x)/((double)no_cells_per_dim);
    }
    double get_y_gran()
    {
        return (max_y-min_y)/((double)no_cells_per_dim);
    }

    int get_x_cell(double x)
    {
        if (x <= min_x)
            return 0;
        if (x>= max_x)
            return no_cells_per_dim - 1;
        int x_cell =  (int)((x-min_x)/get_x_gran());
        return x_cell;
    }

    int get_y_cell(double y)
    {
        if (y <= min_y)
            return 0;
        if (y>= max_y)
            return no_cells_per_dim - 1;
        int y_cell = (int)((y-min_y)/get_y_gran());
        return y_cell;
    }

    std::map<int, User*>* find_cell(double x, double y, int& x_cell, int& y_cell)
    {
        x_cell = get_x_cell(x);
        y_cell = get_y_cell(y);

        return &grid[x_cell][y_cell];
    }


    int no_cells_per_dim;
    std::map<int, User*>** grid;
    int** checkins_count;
    int** checkins_count_curr;
    long total_checkins_count;
    long total_checkins_count_curr;
    double min_x;
    double min_y;
    double max_x;
    double max_y;
    int size;
    std::string name;

};

#endif
