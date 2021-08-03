#include "Simulator.h"



Simulator::Simulator(std::string config_file_name)
{
    read_config(config_file_name);

    double min_x = DBL_MAX;
    double min_y = DBL_MAX;
    double max_x = -DBL_MAX;
    double max_y = -DBL_MAX;



    int last_checkin_time;
    int first_checkin_time;
    read_checkins_from_text_file(min_x, max_x, min_y, max_y, &checkins, data_file_loc, max_checkins, first_checkin_time, last_checkin_time);
    int total_duration = last_checkin_time-first_checkin_time;
    if ((sim_time == -1) || (sim_time > total_duration))
        sim_time = total_duration;
    std::cout << "Simulating for " << sim_time/(3600*24) << " days.\n";

    int days_to_report = sim_time/(3600*24)+10;

    for (int i = 0; i < days_to_report; i++)
    {
       is_on_day.push_back(0);
       susc_on_day.push_back(0);
       new_infected_on_day.push_back(0);
       new_infected_on_day_upper.push_back(0);
       new_infected_on_day_lower.push_back(0);
    }


    if (simulate_transmission)
    {
        spreading_users = new Grid(grid_gran, min_x, max_x, min_y, max_y, "spreading");
        susceptible_users = new Grid(grid_gran, min_x, max_x, min_y, max_y, "susceptible");
        checkin_counts = new Grid(grid_gran, min_x, max_x, min_y, max_y, "counts");
    }
    all_users = new Grid(grid_gran, min_x, max_x, min_y, max_y, "all");

    infected_count = 0;
    recovered_count = 0;
    init_infected = 0;
    susceptible_count = 0;

    last_period = -1;
}

Simulator::~Simulator()
{

    for (auto it = checkins.begin(); it != checkins.end(); ++it)
    {
        delete (*it)->location;
        delete *it;
    }

    for (auto it = usrs.begin(); it != usrs.end(); ++it)
    {
        delete it->second;
    }

    while (!events.empty())
    {
        Event* temp = events.top();
        events.pop();
        delete temp;
    }   
}

void Simulator::read_config(std::string config_file_name)
{
    std::ifstream ifile(config_file_name);
    std::string str;
    int seed = time(NULL);
    while(!ifile.eof()) {
        while(getline(ifile,str)) {
            std::string::size_type begin = str.find_first_not_of(" \f\t\v");
            //Skips blank lines
            if(begin == std::string::npos)
                continue;
            //Skips #
            if(std::string("#").find(str[begin]) != std::string::npos)
                continue;
            std::string firstWord;
            try {
                firstWord = str.substr(0,str.find("="));
            }
            catch(std::exception& e) {
                firstWord = str.erase(str.find_first_of(" "),str.find_first_not_of(" "));
            }
            std::transform(firstWord.begin(),firstWord.end(),firstWord.begin(), ::toupper);
            if(firstWord == "PROB_INIT_INFECTED")
                prob_init_infected = std::stof(str.substr(str.find("=")+1,str.length()));
            if(firstWord == "INFECTING_PROB")
                infecting_prob = std::stof(str.substr(str.find("=")+1,str.length()));
            if(firstWord == "TIME_TO_RECOVER_MEAN")
                time_to_recover_mean = std::stoi(str.substr(str.find("=")+1,str.length()));
            if(firstWord == "TIME_TO_RECOVER_STD")
                time_to_recover_std = std::stoi(str.substr(str.find("=")+1,str.length()));
            if(firstWord == "TIME_TO_SPREAD_STD")
                time_to_spread_std = std::stoi(str.substr(str.find("=")+1,str.length()));
            if(firstWord == "TIME_TO_SPREAD_MEAN")
                time_to_spread_mean = std::stoi(str.substr(str.find("=")+1,str.length()));
            if(firstWord == "INFECTING_DISTANCE")
                infecting_distance = std::stof(str.substr(str.find("=")+1,str.length()));
            if(firstWord == "INFECTING_TIME")
                infecting_time = std::stoi(str.substr(str.find("=")+1,str.length()));
            if(firstWord == "MAX_CHECKINS")
                max_checkins = std::stoi(str.substr(str.find("=")+1,str.length()));
            if(firstWord == "POPULATION_COUNT")
                population_count = std::stoi(str.substr(str.find("=")+1,str.length()));
            if(firstWord == "GRID_GRAN")
                grid_gran = std::stoi(str.substr(str.find("=")+1,str.length()));
            if(firstWord == "OUTPUT_LOC")
                output_file_loc = str.substr(str.find("=")+1,str.length());
            if(firstWord == "NO_SAMPLE_USERS")
                no_sample_users = std::stoi(str.substr(str.find("=")+1,str.length()));
            if(firstWord == "DATA_LOC")
                data_file_loc = str.substr(str.find("=")+1,str.length());
            if(firstWord == "SIMULATION_TIME")
                sim_time = std::stoi(str.substr(str.find("=")+1,str.length()));
            if(firstWord == "SEED")
                seed = std::stoi(str.substr(str.find("=")+1,str.length()));
            if(firstWord == "CHECKINS_UPDATE_RATE")
                checkins_update_rate = std::stoi(str.substr(str.find("=")+1,str.length()));
            if(firstWord == "NO_TOTAL_HOPS")
                no_total_hops = std::stoi(str.substr(str.find("=")+1,str.length()));
            if(firstWord == "ALG_SUS")
                alg_sus = std::stoi(str.substr(str.find("=")+1,str.length())) == 1;
            if(firstWord == "ALG_SPREAD")
                alg_spread = std::stoi(str.substr(str.find("=")+1,str.length())) == 1;
            if(firstWord == "SIMULATE_TRANSMISSION")
                simulate_transmission = std::stoi(str.substr(str.find("=")+1,str.length())) == 1;
            if(firstWord == "C_U")
                c_u = std::stof(str.substr(str.find("=")+1,str.length()));


        }
    }


    srand(seed);
}



long Simulator::time_passed(long time_now, long time_init)
{
    return (time_now - time_init);
}

void Simulator::close_infected_user(Checkin* checkin, std::vector<User*>* res)
{
    spreading_users->find_users_within_range(checkin->location->x, checkin->location->y, infecting_distance, res);
}


bool Simulator::usr_can_spread(User* usr, long time)
{
    if (usr->status == INFECTED)
    {
        if (time_passed(time, usr->time_status_changed) >= usr->time_to_spread)
        {
            return true;
        }
    }
    return false;
}

void Simulator::update_infections(Checkin* new_checkin)
{
    User* usr = (usrs.find(new_checkin->usr_id))->second;
    if (usr->status == INFECTED || usr->would_ve_infected != NULL)
    {
        std::vector<User*> res;
        if (usr_can_spread(usr, new_checkin->time))
            infect_users(usr, new_checkin->time);
    }
    if (usr->status == SUSCEPTIBLE)
    {
        std::vector<User*> infected_nearby;
        close_infected_user(new_checkin, &infected_nearby);

        for (auto it = infected_nearby.begin(); it!=infected_nearby.end();it++)
        {
            if (should_infect(usr, *it, new_checkin->time))
            {
                infect(usr, new_checkin->location, new_checkin->time, *it, false);
                break;

            }
        }
    }
    if (usr->status == RECOVERED)
    {
        //Recovered people are immune so no change in infections as they move
    }
}

void Simulator::update_colocation(User* usr, int time)
{
    std::vector<User*> res;
    all_users->find_users_within_range(usr->location->x, usr->location->y, infecting_distance, &res);
    for (auto it = res.begin(); it != res.end(); ++it)
    {
        if ((*it)->id == usr->id)
            continue;
        if ((usr->curr_colocations.count((*it)->id) != 0) || ((*it)->curr_colocations.count(usr->id) != 0))
            continue;
        int contact_duration =  time_of_contact((*it), usr, time, infecting_distance);
        bool long_enough_contact = contact_duration >= infecting_time;
        if (long_enough_contact)
        {
            usr->curr_colocations.insert((*it)->id);
            (*it)->curr_colocations.insert(usr->id);
            add_event(EVENT_COLOCATION_ENDED, *it, NULL, time+contact_duration, usr);
            usr->colocations.push_back(std::make_pair(time, *it));
            (*it)->colocations.push_back(std::make_pair(time, usr));

            usr->colocations_map[(*it)->id].push_back(time);
            (*it)->colocations_map[usr->id].push_back(time);

            double prob_u_is_spreading = calc_prob_spreading(usr, time);
            double prob_v_is_spreading = calc_prob_spreading((*it), time);

            double prob_transmission_to_v = prob_u_is_spreading*infecting_prob;
            double prob_transmission_to_u = prob_v_is_spreading*infecting_prob;
            
            usr->events.insert(std::make_pair(time, prob_transmission_to_u));
            (*it)->events.insert(std::make_pair(time, prob_transmission_to_v));
            (usr->curr_day_colocation[(time-begin_time)/checkins_update_rate])[(*it)->id]++;
            ((*it)->curr_day_colocation[(time-begin_time)/checkins_update_rate])[usr->id]++;
        }
    }
}

void Simulator::update_user_info(Checkin* checkin)
{
    auto usr = usrs.find(checkin->usr_id);

    int days_to_report = sim_time/(3600*24)+10;
    
    if (usr == usrs.end())
    {
        usr = add_user(checkin->usr_id, checkin->location, NEW_UNKNOWN, checkin->time, &usrs, false, checkins_update_rate, days_to_report);

        for (int i = 0; i < (sim_time/checkins_update_rate); i++)
            usr->second->prob_infected_graph.push_back(0);

        usr->second->first_checkin = checkin->time;

        all_users->insert(checkin->location->x, checkin->location->y, usr->second);

        if (simulate_transmission)
        {
            susceptible_count++;
            if (should_init_infect())
            {
                init_infected++;
                infect(usr->second, checkin->location, checkin->time, NULL, true);

            }
            else
            {
                usr->second->status = SUSCEPTIBLE;
                susceptible_users->insert(checkin->location->x, checkin->location->y, usr->second);
            }
        }
    }
    else
    {
        all_users->move(usr->second->location->x, usr->second->location->y, checkin->location->x, checkin->location->y, usr->second);
        if (simulate_transmission)
        {
            if (usr->second->status == SUSCEPTIBLE)
                susceptible_users->move(usr->second->location->x, usr->second->location->y, checkin->location->x, checkin->location->y, usr->second);
            else if (usr->second->status == INFECTED && usr_can_spread(usr->second, checkin->time))
                spreading_users->move(usr->second->location->x, usr->second->location->y, checkin->location->x, checkin->location->y, usr->second);
        }
        //delete prev checkin
        //auto it = usr->second->curr_checkin;
        //checkins.erase(it->list_pointer);
        //delete (it)->location;
        //delete it;
    }

    usr->second->location = checkin->location;
    usr->second->curr_checkin = checkin;

    if (simulate_transmission)
    {
        checkin_counts->inc_checkins_count(usr->second->location->x, usr->second->location->y);
    }
}


void Simulator::exec_event(Event* event)
{
    if (event->type == EVENT_COLOCATION_ENDED)
    {
        event->usr->curr_colocations.erase(event->usr2->id);
        event->usr2->curr_colocations.erase(event->usr->id);
    }
    else if (event->type == EVENT_UPDATE_INFECTIONS)
    {
        alg_spread_update(event);
    }
    else if (event->type == EVENT_DAILY_REPORT)
    {
        save_daily_spread_report((event->time-begin_time)/(3600*24)+1, event->time-begin_time);
        add_event(EVENT_DAILY_REPORT, NULL, NULL, event->time+3600*24, NULL);
    }
    else if (event->type == EVENT_USER_START_SPREAD)
    {
        spreading_users->insert(event->usr->location->x, event->usr->location->y, event->usr);
        infect_users(event->usr, event->time);
    }
    else if (event->type == EVENT_USER_RECOVER)
    {
        infected_count--;
        recovered_count++;

        event->usr->status = RECOVERED;
        event->usr->time_status_changed = event->time;
        spreading_users->remove(event->usr->location->x, event->usr->location->y, event->usr);
    }
}

void Simulator::infect(User* usr, Location* loc, long time, User* infected_by, bool init_infect)
{

    susceptible_count--;
    infected_count++;
    usr->status = INFECTED;
    usr->time_status_changed = time;
    usr->time_to_spread = get_time_to_spread();
    usr->time_to_recover = get_time_to_recover();

    if (init_infect)
    {
        usr->time_to_spread += 3600;
        usr->time_to_recover += 3600;
    }
    long spread_time = (time + usr->time_to_spread);
    long recover_time = (time + usr->time_to_recover);

    add_event(EVENT_USER_START_SPREAD, usr, loc, spread_time, NULL);
    add_event(EVENT_USER_RECOVER, usr, loc, recover_time, NULL);


    if (!init_infect)
    {
	    susceptible_users->remove(usr->location->x, usr->location->y, usr);
        if (infected_by!= NULL)
        {
            infected_by->no_people_infected++;
            infected_by->no_people_infected_curr_day++;

            if (distance(infected_by->location->x, infected_by->location->y, usr->location->x, usr->location->y) > infecting_distance)
                throw std::runtime_error("WRONG INFECTION!");
        }
    }
}


double Simulator::calc_prob_spreading(User* u, int time)
{
    if (u->first_checkin<0 || (u->first_checkin > time))
        return 0;


    double prob_not_recovered = 1;
    double prob_not_spreading = 1;
    for (auto it = u->events.begin(); it!=u->events.end(); it++)
    {
        if (it->first+ get_time_to_recover()< time)
        {
            prob_not_recovered = prob_not_recovered*(1-it->second);
            continue;
        }
        if (it->first+get_time_to_spread() > time)
            continue;
        
        prob_not_spreading = prob_not_spreading*(1-it->second);
    }
    double prob_spreading_by_transmission = prob_not_recovered*(1-prob_not_spreading);
    double prob_spreading = prob_spreading_by_transmission;
    if (u->first_checkin+get_time_to_recover() > time && u->first_checkin+get_time_to_spread() < time)
        prob_spreading = prob_init_infected + (1-prob_init_infected)*prob_spreading_by_transmission;
    else if (u->first_checkin+get_time_to_recover() < time)
        prob_spreading = (1-prob_init_infected)*prob_spreading_by_transmission;
    return prob_spreading;
}




void Simulator::simulate()
{
    this->begin_time = (*checkins.begin())->time;
    long begin_time = this->begin_time;//(*checkins.begin())->time;
    auto checkins_iter = checkins.begin();
    Event* event;

    std::ofstream daily_out_file("daily_"+output_file_loc, std::fstream::app);
    daily_out_file << "day,infected count,recovered count,susceptible count,PollSusc_L,PollSusc_U,PollSpreader" << std::endl;
    daily_out_file.close();
    add_event(EVENT_DAILY_REPORT, NULL, NULL, this->begin_time, NULL);
    if (alg_spread)
        add_event(EVENT_UPDATE_INFECTIONS, NULL, NULL, begin_time-1+checkins_update_rate, NULL);

    long checkins_processed = 0;

    std::cout << "STARTING SIMULATION" << std::endl;
    while (true)
    {
        if (!events.empty())
            event = events.top();


        if (checkins_iter == checkins.end() && events.empty())
            break;
        
        long curr_time;
        if (checkins_iter != checkins.end() && (events.empty() || (event->time > (*checkins_iter)->time)))
        {
            //Process a checkin
            checkins_processed++;
            if (checkins_processed %100000 == 0)
                std::cout << "Processed " << checkins_processed << " check-ins" << std::endl;
            curr_time = time_passed((*checkins_iter)->time, begin_time);

            (*checkins_iter)->list_pointer = checkins_iter;
            update_user_info(*checkins_iter);

            if (simulate_transmission)
                update_infections(*checkins_iter);

            auto usr = usrs.find((*checkins_iter)->usr_id);
            update_colocation(usr->second, (*checkins_iter)->time);
            ++checkins_iter;
        }
        else
        {
            //Process an event
            curr_time = time_passed(event->time, begin_time);
            events.pop();
            exec_event(event);
            delete event;
        }

        if (curr_time>sim_time)
            break;
    }
}

bool Simulator::should_init_infect()
{
    double rand_number = ((double) rand() / ((double)INT_MAX));
    if (rand_number < prob_init_infected)
        return true;
    return false;
}

bool Simulator::should_infect(User* s, User* i, long init_time, bool randomize, int infecting_time, double infecting_distance)
{
    if (infecting_time == -1)
        infecting_time = this->infecting_time;
    if (infecting_distance == -1)
        infecting_distance = this->infecting_distance;

    if (i->curr_colocations.count(s->id) != 0 || s->curr_colocations.count(i->id) != 0)
        return false;

    bool long_enoug_contact = time_of_contact(s, i, init_time, infecting_distance) >= infecting_time;
    bool i_recovers_after_infecting_time = i->time_status_changed + i->time_to_recover > init_time+infecting_time;
    bool infect_rand =  (((double) rand() / ((double)INT_MAX)) < infecting_prob) || (!randomize);
    return long_enoug_contact && i_recovers_after_infecting_time && infect_rand;
}

long Simulator::time_of_contact(User* a, User* b, long curr_time, double dist)
{
    Checkin* first = a->curr_checkin;
    Checkin* second = b->curr_checkin;
    long time = curr_time;
    while (true)
    {
        if (first == NULL)
            break;
        if (second == NULL)
            break;

        if (!within_range(first->location, second->location, dist))
            break;

        if (first->time < second->time)
        {
            first = first->next_usr_checkin;
            if (first == NULL)
                break;
            time = first->time;
        }
        else
        {
            second = second->next_usr_checkin;
            if (second == NULL)
                break;
            time = second->time;
        }
    }

    return time - curr_time;
}

void Simulator::infect_users(User* infected, long time)
{
    std::vector<User*> res;
    susceptible_users->find_users_within_range(infected->location->x, infected->location->y, infecting_distance, &res);
    for (auto it = res.begin(); it != res.end(); ++it)
    {
        if (should_infect(*it, infected, time))
            infect(*it, (*it)->location, time, infected, false);
    }
}



void Simulator::add_event(int type, User* usrid, Location* loc, long time, User* userid2)
{
    Event* temp = new Event();
    temp->time = time;
    temp->type = type;
    temp->usr = usrid;
    temp->usr2 = userid2;
    temp->location = loc;
    events.push(temp);
}

bool Simulator::within_range(Location* a, Location* b, double infecting_distance)
{
    if (infecting_distance == -1)
        infecting_distance = this->infecting_distance;
    return (distance(a->x, a->y, b->x, b->y) <= infecting_distance);
}




double Simulator::calc_avg_daily_no_colocations(int day)
{
    double coloc_count = 0;
    for (auto it = usrs.begin(); it != usrs.end(); it++)
    {
        coloc_count += it->second->curr_day_colocation[day].size();
    }
    return (coloc_count/2.0)/(double)usrs.size();
}

void Simulator::save_daily_spread_report(int curr_day, int time)
{
    std::ofstream daily_out_file("daily_"+output_file_loc, std::fstream::app);
    double est_lower = 0;
    double est_upper = 0;
    if (alg_sus)
    {
        double curr_total_infections_lower = 0;
        double curr_total_infections_upper = 0;
        calc_expected_inf_at_time(time+begin_time, curr_total_infections_lower, curr_total_infections_upper);
        new_infected_on_day_lower[curr_day] = curr_total_infections_lower;
        new_infected_on_day_upper[curr_day] = curr_total_infections_upper;
        est_lower = curr_total_infections_lower;
        est_upper = curr_total_infections_upper;
        if (curr_day-get_time_to_recover()/(3600*24)>0)
        {
            est_lower = curr_total_infections_lower-new_infected_on_day_lower[curr_day-get_time_to_recover()/(3600*24)];
            est_upper = curr_total_infections_upper-new_infected_on_day_upper[curr_day-get_time_to_recover()/(3600*24)];
        }
    }
    
    int est_infected_count = 0;
    if (alg_spread)
    {
        int curr_period = last_period;
        if (curr_period < 0)
            curr_period = 0;
        int begin_period = last_period - (get_time_to_recover())/checkins_update_rate; 
        int begin_inf = population_count*prob_init_infected;
        if (begin_period >= 0)
            begin_inf = new_infected_on_day[begin_period];
        else
            begin_inf = -1*new_infected_on_day[begin_period];
        est_infected_count = new_infected_on_day[curr_period] - begin_inf;
        if (begin_period <= 0)
            est_infected_count += population_count*prob_init_infected;
    }
    

    daily_out_file << curr_day <<  "," << infected_count << "," << recovered_count << "," << susceptible_count << ","  << est_lower << "," << est_upper << "," << est_infected_count << std::endl;
    daily_out_file.close();
}

double Simulator::calc_prob_inf_given_path(std::vector<User*> path, int depth, int parent_inf_time)
{
    if (depth == 0)
        return 1;

    auto u_v_colocs = &(path[depth]->colocations_map[path[depth-1]->id]);
    int parent_spreading_time = parent_inf_time + get_time_to_spread();
    int parent_recovery_time = parent_inf_time + get_time_to_recover();
    auto it = std::lower_bound(u_v_colocs->begin(), u_v_colocs->end(), parent_spreading_time);
    double total_inf_prob = 0;
    double child_not_inf_so_far_prob = 1;
    for (; it != u_v_colocs->end() && *it < parent_recovery_time; it++)
    {
        int child_inf_time = (*it);
        double path_prob = calc_prob_inf_given_path(path, depth-1, child_inf_time)*(infecting_prob*child_not_inf_so_far_prob);
        total_inf_prob += path_prob;
        child_not_inf_so_far_prob = child_not_inf_so_far_prob*(1-infecting_prob);
    }
    return total_inf_prob;

}


void Simulator::calc_prob_inf_through_neighbour(User* u, std::vector<User*> curr_path, int end_time, double& res_lower, double& res_upper)
{
    double inf_start_at_u = calc_prob_inf_given_path(curr_path, curr_path.size()-1, u->first_checkin)*prob_init_infected;

    if (end_time < begin_time+get_time_to_spread())
    {
        res_lower = inf_start_at_u; 
        res_upper = inf_start_at_u;
        return;
    }

    double prob_not_inf_from_us_neighbour_lower = 1;
    double prob_not_inf_from_us_neighbour_upper = 1;
    for (auto v_it = u->colocations_map.begin(); v_it != u->colocations_map.end(); v_it++)
    {
        bool already_on_path = false;
        for (auto it =curr_path.begin(); it!=curr_path.end(); it++)
        {
            if ((*it)->id == v_it->first)
            {
                already_on_path = true;
                break;
            }
        }
        if (already_on_path)
            continue;

        User* v = usrs.find(v_it->first)->second;

        auto appended_path = curr_path;
        appended_path.push_back(v);
        double prob_inf_from_v_lower;
        double prob_inf_from_v_upper;
        calc_prob_inf_through_neighbour(v, appended_path, end_time - get_time_to_spread(), prob_inf_from_v_lower, prob_inf_from_v_upper);
        prob_not_inf_from_us_neighbour_lower = prob_not_inf_from_us_neighbour_lower*(1-prob_inf_from_v_upper);
        prob_not_inf_from_us_neighbour_upper = prob_not_inf_from_us_neighbour_upper*(1-prob_inf_from_v_lower);
    }

    double sample_p = no_sample_users/((double) population_count);

    prob_not_inf_from_us_neighbour_lower = pow(prob_not_inf_from_us_neighbour_lower, 1/sample_p);
    prob_not_inf_from_us_neighbour_upper = pow(prob_not_inf_from_us_neighbour_upper, c_u);
    double prob_inf_lower = inf_start_at_u + (1-inf_start_at_u)*(1-prob_not_inf_from_us_neighbour_upper);
    double prob_inf_upper = inf_start_at_u + (1-inf_start_at_u)*(1-prob_not_inf_from_us_neighbour_lower);

    res_lower = prob_inf_lower; 
    res_upper = prob_inf_upper;
    return;
}

void Simulator::calc_prob_inf(User* u, double sample_p, int curr_time, double& prob_inf_lower, double& prob_inf_upper)
{
    if (u->first_checkin<0 || (u->first_checkin > curr_time))
    {
        prob_inf_lower = 0;
        prob_inf_upper = 0;
        return;
    }

    std::vector<User*> curr_path;
    curr_path.push_back(u);
    calc_prob_inf_through_neighbour(u, curr_path, curr_time, prob_inf_lower, prob_inf_upper);
}


void Simulator::calc_expected_inf_at_time(int curr_time, double& exp_inf_lower, double& exp_inf_upper)
{
    double sample_p = no_sample_users/((double) population_count);
    double total_p_lower=0;
    double total_p_upper=0;

    for (auto it = usrs.begin(); it != usrs.end(); it++)
    {
        double lower;
        double upper;
        calc_prob_inf(it->second, sample_p, curr_time, lower, upper);
        total_p_lower += lower;
        total_p_upper += upper;
    }

    exp_inf_lower =  total_p_lower/(sample_p);
    exp_inf_upper =  total_p_upper/(sample_p);
}



void Simulator::get_k_hop_edges(int begin, int end, int no_hops, User* u, std::map<int, double>* neighours, double prev_prob_till_k_hop, double prob_till_k_hop, double& total_weight, int src_u_id, double sample_p)
{
    if (no_hops == 0)
        return;

    double total = usrs.size();

    for (int i = begin; i < end && i < begin+(get_time_to_recover()-get_time_to_spread())/checkins_update_rate; i++)
    {
        for (auto it = u->curr_day_colocation[i].begin(); it != u->curr_day_colocation[i].end(); it++)
        {
            if (it->first == src_u_id)
                continue;
            double prev_prob = (*neighours)[it->first];
            double inf_prob = 1-pow(1-infecting_prob, (double)it->second);
            double not_inf_prob = 1-inf_prob;
            double new_prob = 1-(1-prev_prob)*(not_inf_prob);
            (*neighours)[it->first] = new_prob;
            total_weight = total_weight+(-prev_prob+new_prob);
            if (new_prob>prev_prob && i+get_time_to_spread()/checkins_update_rate < end)
                get_k_hop_edges(i+get_time_to_spread()/checkins_update_rate, end, no_hops-1, usrs.find(it->first)->second, neighours, prev_prob, new_prob, total_weight, src_u_id, sample_p);
        }
    }
}

float Simulator::calc_avg_no_coloc(int begin, int end)
{
    double total = usrs.size();
    double sample_p = no_sample_users/((double)population_count);
    double no_edges = 0;

    for (auto it = usrs.begin(); it != usrs.end(); it++)
    {
        std::map<int, double> neighours;
        double total_weight = 0;
        get_k_hop_edges(begin, end, no_total_hops, it->second, &neighours, 0, 1, total_weight, it->first, sample_p);
        no_edges+=total_weight;
    }
    no_edges = no_edges/pow(sample_p, 2);
    return no_edges;  
}


void Simulator::alg_spread_update(Event* event)
{
    int no_new_infect=0;
    int curr_period = (event->time-begin_time)/checkins_update_rate;
    if (event->time - begin_time<= sim_time)
        add_event(EVENT_UPDATE_INFECTIONS, NULL, NULL, event->time+checkins_update_rate, NULL);

    last_period = curr_period;

    if (curr_period < 0)
        return;
    if (curr_period == 0)
    {
        is_on_day[(event->time-1+get_time_to_spread()-begin_time)/checkins_update_rate]+=population_count*prob_init_infected;
        susc_on_day[curr_period] = population_count-population_count*prob_init_infected;
    }
    else
        susc_on_day[curr_period] = susc_on_day[curr_period-1];

    double no_balls = 0;
    for (int i =0; i <= curr_period;i++)
    {
        double unknown_infected = is_on_day[i];
        if (unknown_infected>0)
        {
            double avg_coloc;
            avg_coloc = calc_avg_no_coloc(i, curr_period);

            if (unknown_infected >= 1)
                no_balls += avg_coloc*((unknown_infected/(double)population_count))*(susc_on_day[curr_period]/(double)population_count);

            double p_infected = 1-pow(1-1.0/(double)susc_on_day[curr_period], no_balls);
            double no_infect = p_infected*susc_on_day[curr_period];
            no_new_infect += no_infect;;
        }
    }

    new_infected_on_day[curr_period] = (int)no_new_infect;
    if (curr_period > 0)
        no_new_infect = new_infected_on_day[curr_period] - new_infected_on_day[curr_period-1];

    susc_on_day[curr_period]-=no_new_infect;
}
