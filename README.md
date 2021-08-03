# SpreadSim
This repository implements SpreadSim from [1] to simulate the spread of a contagion and PollSpreader and PollSusceptible algorithms from [2] that estimate the spread of a contagion through subsampling.

## Instalation and requirements
Running SpreadSim requires c++11. It has been tested with g++ 7.5. 

## To Run
Compile SpreadSim by calling make. Upon successful compilation, calling ./simulator runs SpreadSim. Running SpreadSim requires a config file called config.txt at the working directory. The python script run.py is provided for convinience, and calling python run.py automatically creates this config file and then runs SpreadSim. Possible configuration and their description are discussed in run.py. 

## Location Dataset
SpreadSim requires a dataset of location trajectories. Specify the path to this dataset in run.py using DATA_LOC parameter. Everyline of the dataset must contains check-ins of the format (usr_id, lat, lon, time_stamp), where usr_id is a unique identifier for each agent in the simulation. All the check-ins of a user must be written consequitively and for each user, its check-ins must be sorted in the decreasing order of time.

## Simulation and Up-Sampling
Setting POPULATION_COUNT to be the same as NO_SAMPLE_USERS and ALG_SPREAD and ALG_SUS to 0 in run.py runs SpreadSim, as specified in [1] (without any up-sampling considerations). The output will be written in the file specified by OUTPUT_LOC parameter, where for each day, number of infected, susceptile and recovered agents will be provided. 

On the other hand, setting POPULATION_COUNT > NO_SAMPLE_USERS and having either ALG_SPREAD and/or ALG_SUS to be 1 runs  PollSpreader and PollSusceptible algorithms from [2], assuming the available location dataset is a sub-sample of the true population, whose size is POPULATION_COUNT.

## References
[1] S. Rambhatla, S. Zeighami, K. Shahabi, C. Shahabi and Y. Liu, "Towards Accurate Spatiotemporal COVID-19 Risk Scores using High Resolution Real-World Mobility Data," 2021, arXiv preprint arXiv:2012.07283.

[2] S. Zeighami, C. Shahabi, and J. Krumm, “Estimating spread of contact-based contagions in a population through sub-sampling,” in Proceedings of the VLDB Endowment Volume 14 Issue 9, 2021.
