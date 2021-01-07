#!/bin/sh 
#------------- QCD cluster related --------------------- 
QUEUE=green
TIMELIMIT=24:00:00
THREADS=40
#------------- PATH --------------------- 
TAG=Dec18
OUTPUTDIR=/raid4/haoli/2020_MC/test_lamlambar/mech6s/test1
LOCALDIR=/home/haoli/test/Simulation_test/src/mc
decay_channel=plambarM6  #plambarM5  
CONFIG=lamlambar.config

#--- sample size ---
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
	DEF=`printf "gen_%s_%04d.def" "$TAG" "$fileNumber"`
	Name=`printf "%s_%04d" "$decay_channel" "$fileNumber"`
	echo "Job name:" $Name
	echo "DEF     :" $DEF
 	#------------- RUNNING SCRIPTS --------------------- 
	jobid_str=$(sbatch --job-name=$Name --ntasks=${THREADS} --partition=${QUEUE} --mem=${MEM} --time=${TIMELIMIT}  --output=$OUTPUTDIR/$Name.out --error=$OUTPUTDIR/$Name.err --export=DEF=$DEF,translator=$translator,OUTPUTDIR=$OUTPUTDIR,LOCALDIR=$LOCALDIR,THREADS=$THREADS,QUEUE=$QUEUE,Name=$Name,fileNumber=$fileNumber,CONFIG=$CONFIG run_grid_sim.csh) 
	jobid=$(echo $jobid_str | sed 's/[^0-9]*//g')
	echo "Job ID = " $jobid	
	echo "----------------"

	sleep 0.5
done











