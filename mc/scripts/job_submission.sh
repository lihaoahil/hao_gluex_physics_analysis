 #########################################################
 #  Usage: ./job_submission.sh 
 ########################################################
 # take ranges of parameter b_1 and b_2        
 # take certain steps        
 # replace template config of MC_GEN with specific b_1 and b_2 values
 ########################################################
 # 
 # created by:  Hao Li (Carnegie Mellon University)
 #              08-Jan-2021
 ######################################################## 

#!/bin/sh 

#------------- QCD cluster --------------------- 
QUEUE=green
TIMELIMIT=24:00:00
THREADS=40

#------------- ENV --------------------- 
# set up paths to all simulation softwares
ENV_FILE=/home/haoli/env/test.csh
# light weight version of gluex database to be moved to cluster nodes to avoid internet bottleneck
RCDB_SQLITE=/home/gluex2/gluexdb/rcdb_2020_11_13.sqlite 
CCDB_SQLITE=/home/gluex2/gluexdb/ccdb_2020_11_13.sqlite

#------------- CONFIG --------------------- 
LOCALDIR=/home/haoli/test/Simulation_test/src/mc
CONFIG=lamlambar.config

#------------- I/O PATH --------------------- 
INPUTPATH=
OUTPUTDIR=/raid4/haoli/2020_MC/test_lamlambar/mech6s/test1




#--- sample related ---
decay_channel=plambarM6  #plambarM5  
TRIGGER=100000
START=94
END=94


#------------- Set up data path for running jobs --------------
if [ "$QUEUE" == "green" ]; then
  CPUMEM=1600 #max=1603 mb
elif [ "$QUEUE" == "red" ]; then
  CPUMEM=1990 #max=2008 mb
fi
let MEM=$THREADS*$CPUMEM
echo "Memory to be allocated per job: " $MEM
#---------------------------------
echo "Start batch job submission:"

cd $LOCALDIR
cp control/run.mac .
sed -i 's/TRIGGER/'$TRIGGER'/' run.mac



for fileNumber in `seq $START $END`;
do
	DEF=`printf "gen_%s_%04d.def" "$fileNumber"`
	Name=`printf "%s_%04d" "$decay_channel" "$fileNumber"`
	echo "Job name:" $Name
	echo "DEF     :" $DEF
 	#------------- RUNNING SCRIPTS --------------------- 
	jobid_str=$(sbatch --job-name=$Name --ntasks=${THREADS} --partition=${QUEUE} --mem=${MEM} --time=${TIMELIMIT}  --output=$OUTPUTDIR/$Name.out --error=$OUTPUTDIR/$Name.err --export=DEF=$DEF,translator=$translator,OUTPUTDIR=$OUTPUTDIR,LOCALDIR=$LOCALDIR,THREADS=$THREADS,QUEUE=$QUEUE,Name=$Name,fileNumber=$fileNumber,CONFIG=$CONFIG run_job_cluster.csh) 
	jobid=$(echo $jobid_str | sed 's/[^0-9]*//g')
	echo "Job ID = " $jobid	
	echo "----------------"

	# waiting between jobs
	sleep 10 
done











