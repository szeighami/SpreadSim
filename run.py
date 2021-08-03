import subprocess
import numpy as np
import os
import random
random.seed()


config={}
config['TIME_TO_RECOVER_MEAN'] = 12*24*3600
config['TIME_TO_RECOVER_STD'] = 0
config['TIME_TO_SPREAD_MEAN'] = 5*24*3600
config['TIME_TO_SPREAD_STD'] = 0
config['INFECTING_TIME'] = 3600
config['INFECTING_DISTANCE'] = 0.0001
config['INFECTING_PROB'] =0.01
config['PROB_INIT_INFECTED'] = 0.1
config['DATA_LOC'] = '/path/to/data'
config['OUTPUT_LOC'] = 'res.txt'
config['SIMULATION_TIME'] =20*24*3600
config['POPULATION_COUNT']=20000
config['MAX_CHECKINS']=-1
config['NO_SAMPLE_USERS']=1000
config['C_U']=8.728
config['CHECKINS_UPDATE_RATE']=3600*24
config['NO_TOTAL_HOPS']=1
config['SIMULATE_TRANSMISSION']=1
config['ALG_SUS']=1
config['ALG_SPREAD']=1
config['GRID_GRAN']=100
config['NAME']="test_sample"


os.system('mkdir tests')
os.system('mkdir tests/'+ config['NAME'])
os.system('rm tests/'+ config['NAME']+'/*.txt')

with open('tests/'+config['NAME']+'/config.txt', 'w') as f:
    for k, v in config.items():
        f.write(k+"="+str(v)+'\n')

cmd = 'cd tests/'+ config['NAME'] + ' && ../../simulator > out.txt'
os.system(cmd)
