#ifndef SIMULATOR_H
#define SIMULATOR_H
#include <vector>
#include <stdexcept>
#include <list>
#include "float.h"
#include "Utils.h"
#include <limits.h>
#include "Grid.h"
#include <random>
#include <map>
#include <string>
#include <queue>
#include <cmath>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <ctime>
#include <set>

class Grid;
#ifdef SAFEGRAPH
class Reader;
#endif


struct Location
{
    int id;
    double x;
    double y;
};

struct Checkin
{
    long time;
    int usr_id;
    Location* location;
    Checkin* next_usr_checkin;   
    std::list<Checkin*>::iterator list_pointer;
};


struct User
{
    int id; 
    int status;
    long time_status_changed;
    int time_to_spread;
    int time_to_recover;
    int no_people_infected;
    int no_people_infected_curr_day;
    int no_people_colocation_curr_day;
    std::vector<std::map<int, int> > curr_day_colocation;
    std::map<int,  double > events;//<time,  prob_transmission>
    std::vector<std::pair<int, User*> > colocations;
    std::map<int, std::vector<int> > colocations_map; //<user_id, vector<time> >
    std::set<int> curr_colocations;
    bool is_observing;
    Location* location;
    Checkin* curr_checkin;
    std::set<int>* would_ve_infected;
    double prob_infected;
    std::vector<double> prob_infected_graph;
    double prev_prob_infected;
    int first_checkin;
};

struct usr_cmp {
    bool operator() (User* const& lhs, User* const& rhs) const {
		return lhs->id < rhs->id ;
    }
};

struct Event
{
    long time;
    int type;
    User* usr;
    User* usr2;
    Location* location;
};

struct Compare
{
    bool operator()(const Event* a, const Event* b)
    {
        return (a->time > b->time);
    }
};


const int NEW_UNKNOWN = -1;
const int NEW_SUSCEPTIBLE = -2;
const int NEW_INFECTED = -3;
const int NEW_OBSERVED = -4;
const int SUSCEPTIBLE = 0;
const int INFECTED = 1;
const int RECOVERED = 2;

const int EVENT_USER_START_SPREAD = 0;
const int EVENT_USER_RECOVER = 1;
const int EVENT_LOCATION_CLEAR = 2;
const int EVENT_USER_INFECT = 3;
const int EVENT_USER_CALC_R0 = 4;
const int EVENT_UPDATE_CHECKINS_DIST = 5;
const int EVENT_UPDATE_INFECTIONS = 6;
const int EVENT_COLOCATION_ENDED = 7;
const int EVENT_UPDATE_INFECTIONS_BY_PROB = 8;
const int EVENT_DAILY_REPORT = 9;

class Simulator
{
public:
    Simulator(std::string config_file_name);

    ~Simulator();

    void simulate();

    void save_daily_spread_report(int curr_day, int time);
    
private:

    void read_config(std::string config_file_name);
    void exec_event(Event* event);
    void update_user_info(Checkin* checkin);
    void update_infections(Checkin* checkin);
    void update_colocation(User* usr, int time);

    void infect_users(User* location, long time);
    //void add_user(int usrid, Location* loc, int stat, int time_change);
    void add_event(int type, User* usrid, Location* loc, long time, User* userid2);
    bool usr_can_spread(User* usr, long time);
    bool within_range(Location* a, Location* b,double infecting_distance = -1);
    bool should_init_infect();
    bool should_infect(User* s, User* i, long init_time, bool randomize = true, int infecting_time = -1, double infecting_distance=-1);
    long time_passed(long time_now, long time_init);
    long time_of_contact(User* a, User* b, long curr_time, double dist);
    void close_infected_user(Checkin* checkin, std::vector<User*>* res);
    double calc_avg_daily_no_colocations(int day);

    void calc_expected_inf_at_time(int curr_time, double& exp_inf_lower, double& exp_inf_upper);
    double calc_prob_spreading(User* u, int time);
    void calc_prob_inf(User* u, double sample_p, int curr_time, double& prob_inf_lower, double& prob_inf_upper);
    float calc_avg_no_coloc(int begin, int end);
    void get_k_hop_edges(int begin, int end, int no_hops, User* u, std::map<int, double>* neighours, double prev_prob_till_k_hop, double prob_till_k_hop, double& total_weight, int src_u_id, double sample_p);

    void calc_prob_inf_through_neighbour(User* u, std::vector<User*> curr_path, int end_time, double& res_lower, double& res_upper);
    double calc_prob_inf_given_path(std::vector<User*> path, int depth, int parent_inf_time);
    void infect(User* usr, Location* loc, long time, User* infected_by, bool init_infect);

    void alg_spread_update(Event* event);

    int get_time_to_spread()
    {
        if (time_to_spread_std == 0)
            return time_to_spread_mean;
        std::normal_distribution<double> distribution(time_to_spread_mean,time_to_spread_std);
        return (int)distribution(generator);
    }
    int get_time_to_recover()
    {
        if (time_to_recover_std == 0)
            return time_to_recover_mean;
        std::normal_distribution<double> distribution(time_to_recover_mean,time_to_recover_std);
        return (int)distribution(generator);
    }
    

    std::list<Checkin*> checkins;
    std::map<int, User*> usrs;//key is usr id
    std::priority_queue<Event*, std::vector<Event*>, Compare> events;//priority based on time

    std::vector<int> is_on_day;
    std::vector<int> susc_on_day;
    std::vector<double> new_infected_on_day;
    std::vector<double> new_infected_on_day_lower;
    std::vector<double> new_infected_on_day_upper;

    Grid* spreading_users;
    Grid* susceptible_users;
    Grid* all_users;
    Grid* checkin_counts;


    float prob_init_infected;
    float infecting_prob;
    int time_to_recover_mean;
    int time_to_recover_std;
    int time_to_spread_mean;
    int time_to_spread_std;
    double infecting_distance;
    int infecting_time;

    int total_no_users;
    int infected_count;
    int susceptible_count;
    int recovered_count;

    int init_infected;
    int grid_gran;
    int max_checkins;

    long sim_time;



    int no_sample_users;

    std::string data_file_loc;
    std::string output_file_loc;

    std::default_random_engine generator;

    int population_count;

    int checkins_update_rate;

    int begin_time;
    int no_total_hops;
    bool simulate_transmission;

    int last_period;

    bool alg_sus;
    bool alg_spread;
    bool alg_dense;

    double c_u;



    int curr_day;


};

#endif
