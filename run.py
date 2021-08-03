import os


config={}
config['NAME']="test_sample"# Experiment name, the results are written in folder tests/config['NAME']

# Spread parameters
config['TIME_TO_RECOVER_MEAN'] = 12*24*3600
config['TIME_TO_RECOVER_STD'] = 0
config['TIME_TO_SPREAD_MEAN'] = 5*24*3600
config['TIME_TO_SPREAD_STD'] = 0
config['INFECTING_TIME'] = 3600 # Minimum duration for a contact
config['INFECTING_DISTANCE'] = 0.0001 # Maximum distance for a contact
config['INFECTING_PROB'] =0.01 # Probability of transmission given a contact
config['PROB_INIT_INFECTED'] = 0.1 # Probability of an agent being infected initially

#Location dataset
config['DATA_LOC'] = '/path/to/data' #Path to location dataset
config['NO_SAMPLE_USERS']=1000 #number of users in the dataset
config['POPULATION_COUNT']=20000 #Total population count. If different from NO_SAMPLE_USERS, its assume the available location dataset is a subsample of true population
config['SIMULATION_TIME'] =20*24*3600 #Total duration to run the simulation for, set to -1 to run for the duration of available checkins
config['MAX_CHECKINS']=-1 #Total number of checkins to read, set to -1 to read all from dataset

#Result output file
config['OUTPUT_LOC'] = 'res.txt'


config['C_U']=8.728# Data dependent Constant for PollSusceptible
config['CHECKINS_UPDATE_RATE']=3600*24#Frequency of temporal contact network
config['NO_TOTAL_HOPS']=1# Number of hops for PollSpreader

#Algorithms to run
config['SIMULATE_TRANSMISSION']=1 # Set to 1 to run a simulation of disease transmission based on the location dataset
config['ALG_SUS']=1 # Set to 1 to run PollSusceptible
config['ALG_SPREAD']=1 # Set to 1 to run PollSpreader

#Grid granularity for indexin
config['GRID_GRAN']=100


os.system('mkdir tests')
os.system('mkdir tests/'+ config['NAME'])
os.system('rm tests/'+ config['NAME']+'/*.txt')

with open('tests/'+config['NAME']+'/config.txt', 'w') as f:
    for k, v in config.items():
        f.write(k+"="+str(v)+'\n')

cmd = 'cd tests/'+ config['NAME'] + ' && ../../simulator > out.txt'
os.system(cmd)
