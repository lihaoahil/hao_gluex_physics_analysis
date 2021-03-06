
#!/bin/sh 

#------------- QCD cluster --------------------- 
QUEUE=green
TIMELIMIT=4:00:00
THREADS=4

if [ "$QUEUE" == "green" ]; then
  CPUMEM=1600 #max=1603 mb
elif [ "$QUEUE" == "red" ]; then
  CPUMEM=1990 #max=2008 mb
fi
let MEM=$THREADS*$CPUMEM
echo "Memory to be allocated per job: " $MEM

#------------- ENV --------------------- 
echo "The local directory is: " $MC_LOCALDIR
# set up paths to all simulation softwares
ENV_FILE=/home/haoli/env/test3.csh
# light weight version of gluex database to be moved to cluster nodes to avoid internet bottleneck
RCDB_SQLITE=/home/gluex2/gluexdb/rcdb_2021_04_12.sqlite 
CCDB_SQLITE=/home/gluex2/gluexdb/ccdb_2021_04_12.sqlite

#------------- CONFIG --------------------- 
CONFIG=ppbar.config 
FLUX_FILE=/home/haoli/Physics_Analysis/flux_REST/ascii_files/flux_40856_42559.ascii
#FLUX_FILE=/home/haoli/Physics_Analysis/flux_REST/ascii_files/flux_FALL18_lowE_51384_51457.ascii
E_MIN=5.8
E_MAX=11.6
TRIGGER=100000
START=0
END=9

#------------- NAME & PATH --------------------- 
DECAY=ppbarM6   # in /def folder the name of def file is generally formatted as mc_gen_gluex_$DECAY.def
OUTPUTDIR=/raid2/haoli/test/ppbarM6_match0

MC_LOCALDIR=/home/haoli/test/hao_gluex_physics_analysis/mc/scripts




#------------- JOB SUBMISSION --------------
#make def for the jobs, replace keywords:
mkdir -p $OUTPUTDIR/gen/def
cd $OUTPUTDIR/gen/def
cp ${MC_LOCALDIR}/def/mc_gen_gluex_${DECAY}.def input.def

sed -i 's/E_MIN/'$E_MIN'/' input.def
sed -i 's/E_MAX/'$E_MAX'/' input.def
sed -i 's/TRIGGER/'$TRIGGER'/g' input.def
sed -i 's|FLUX_FILE|'$FLUX_FILE'|' input.def

# start loop over jobs to submit
echo "Start batch job submission:"
cd $MC_LOCALDIR
for fileNumber in `seq $START $END`;
do
	Name=`printf "%s_%04d" "$DECAY" "$fileNumber"`
	echo "Job name:" $Name
 	#------------- RUNNING SCRIPTS --------------------- 
	jobid_str=$(sbatch --job-name=$Name --ntasks=${THREADS} --partition=${QUEUE} --mem=${MEM} --time=${TIMELIMIT}  --output=$OUTPUTDIR/$Name.out --error=$OUTPUTDIR/$Name.err --export=OUTPUTDIR=$OUTPUTDIR,LOCALDIR=$MC_LOCALDIR,THREADS=$THREADS,QUEUE=$QUEUE,Name=$Name,fileNumber=$fileNumber,CONFIG=$CONFIG,TRIGGER=$TRIGGER,ENV_FILE=$ENV_FILE,RCDB_SQLITE=$RCDB_SQLITE,CCDB_SQLITE=$CCDB_SQLITE run_job_cluster.csh) 
	jobid=$(echo $jobid_str | sed 's/[^0-9]*//g')
	echo "Job ID = " $jobid	
	echo "----------------"

	# waiting between jobs
	sleep 1
done











