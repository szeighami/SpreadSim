#ifndef UTIL_H
#define UTIL_H

#include "string"
#include "iostream"
#include "fstream"
#include "vector"
#include "map"
#include <ctime>
#include <unistd.h>
#include <cmath>
#include "Simulator.h"

struct Checkin;
struct User;
struct Location;

double distance(double x1, double y1, double x2, double y2);

std::map<int, User*>::iterator add_user(int usrid, Location* loc, int stat, int time_change, std::map<int, User*>* usrs, bool is_observed, int checkins_update_rate, int sim_days);

bool sort_checkins(Checkin* a, Checkin* b);

void read_checkins_from_text_file(double& min_x, double& max_x, double& min_y, double& max_y, std::list<Checkin*>* checkins, std::string data_file_loc, int max_checkins, int& min_time, int& max_time);


#endif
