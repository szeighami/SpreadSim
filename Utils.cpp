#include "Utils.h"


double distance(double x1, double y1, double x2, double y2)
{
    return sqrt(double((x2-x1) * (x2-x1) + (y2-y1) * (y2-y1)));
}


std::map<int, User*>::iterator add_user(int usrid, Location* loc, int stat, int time_change, std::map<int, User*>* usrs, bool is_observing, int checkins_update_rat, int sim_days)
{
    User* usr = new User();
    usr->id = usrid;
    usr->location = loc;
    usr->status = stat;
    usr->time_status_changed = time_change;
    usr->no_people_infected = 0;
    usr->curr_checkin = NULL;
    usr->is_observing = is_observing;
    usr->no_people_infected_curr_day=0;
    usr->prob_infected=0;
    usr->prev_prob_infected=0;
    usr->first_checkin=-1;
    long sim_time = sim_days*24*3600;
    for (int i = 0; i < sim_time/checkins_update_rat; i++)
    {
        std::map<int, int> a;
        usr->curr_day_colocation.push_back(a);
        usr->curr_day_colocation[i].clear();
    }

    auto res = usrs->insert(std::make_pair(usrid, usr));
    return res.first;

}

bool sort_checkins(Checkin* a, Checkin* b)
{
    return (a->time < b->time);
}


void read_checkins_from_text_file(double& min_x, double& max_x, double& min_y, double& max_y, std::list<Checkin*>* checkins, std::string data_file_loc, int max_checkins, int& min_time, int& max_time)
{

    std::ifstream ifile(data_file_loc);
    double usrid, locid;
    double x, y;
    double time;
    Checkin* temp;
    Checkin* next = NULL;
    int id = 0;

    int no_checkins = 0;


    max_time = 0;
    min_time = INT_MAX;
    while (ifile >> usrid)
    {
        if ((max_checkins != -1) && (no_checkins > max_checkins))
            break;
        ifile >> x >> y >> time;

        if (x < min_x) min_x=x;
        if (x > max_x) max_x=x;
        if (y < min_y) min_y=y;
        if (y > max_y) max_y=y;

        Location* loc = new Location();
        loc->x = x;
        loc->y = y;
        loc->id = id;

        no_checkins++;
        temp = new Checkin();
        temp->location = loc;
        temp->time = (int)time;
        temp->usr_id = (int)usrid;
        if (temp->time>max_time) 
            max_time = temp->time;
        if (temp->time<min_time) 
            min_time = temp->time;

        // next_usr_checkin
        if (next != NULL && next->usr_id == temp->usr_id)
        {
            temp->next_usr_checkin = next;
        }
        else
        {
            temp->next_usr_checkin = NULL;
        }
        next = temp;

        checkins->push_back(temp);
        id++;
    }
    std::cout << "TOTAL CHECKINS: " << no_checkins << std::endl;

    checkins->sort(sort_checkins);
}
