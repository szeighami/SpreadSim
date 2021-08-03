#include <string>
#include <iostream>
#include "Simulator.h"
#include <chrono>


int main()
{
    auto start = std::chrono::high_resolution_clock::now(); 

    std::string config_file_name = "config.txt";
    Simulator sim(config_file_name);
    sim.simulate();
    auto stop = std::chrono::high_resolution_clock::now(); 
    auto duration = std::chrono::duration_cast<std::chrono::seconds>(stop - start); 
    std::cout << "Simulation took " << duration.count() << "s" << std::endl;


    return 0;
}
