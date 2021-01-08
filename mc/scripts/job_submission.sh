
#!/bin/sh 

#------------- QCD cluster --------------------- 
QUEUE=green
TIMELIMIT=24:00:00
THREADS=40

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
ENV_FILE=/home/haoli/env/test.csh
# light weight version of gluex database to be moved to cluster nodes to avoid internet bottleneck
RCDB_SQLITE=/home/gluex2/gluexdb/rcdb_2020_11_13.sqlite 
CCDB_SQLITE=/home/gluex2/gluexdb/ccdb_2020_11_13.sqlite

#------------- CONFIG --------------------- 
CONFIG=lamlambar.config  
FLUX_FILE=/home/haoli/Physics_Analysis/flux_REST/ascii_files/flux_40856_42559.ascii
E_MIN=5.8
E_MAX=11.6
TRIGGER=100000
START=1
END=5

#------------- NAME & PATH --------------------- 
DECAY=ksksp_acceptance   
OUTPUTDIR=/raid4/haoli/ksks/acceptance/test1




#------------- JOB SUBMISSION --------------

echo "Start batch job submission:"

#make def for the jobs, replace keywords:
mkdir -p $OUTPUTDIR/gen/def
cd $OUTPUTDIR/gen/def
cp ${MC_LOCALDIR}/def/mc_gen_gluex_${DECAY}.def input.def
sed -i 's/TRIGGER/$TRIGGER/g' input.def
sed -i 's/FLUX_FILE/$FLUX_FILE/g' input.def
sed -i 's/E_MIN/$E_MIN/g' input.def
sed -i 's/E_MAX/$E_MAX/g' input.def


for fileNumber in `seq $START $END`;
do
	Name=`printf "%s_%04d" "$decay_channel" "$fileNumber"`
	echo "Job name:" $Name
 	#------------- RUNNING SCRIPTS --------------------- 
	jobid_str=$(sbatch --job-name=$Name --ntasks=${THREADS} --partition=${QUEUE} --mem=${MEM} --time=${TIMELIMIT}  --output=$OUTPUTDIR/$Name.out --error=$OUTPUTDIR/$Name.err --export=OUTPUTDIR=$OUTPUTDIR,LOCALDIR=$MC_LOCALDIR,THREADS=$THREADS,QUEUE=$QUEUE,Name=$Name,fileNumber=$fileNumber,CONFIG=$CONFIG,TRIGGER=$TRIGGER,ENV_FILE=$ENV_FILE,RCDB_SQLITE=$RCDB_SQLITE,CCDB_SQLITE=$CCDB_SQLITE run_job_cluster.csh) 
	jobid=$(echo $jobid_str | sed 's/[^0-9]*//g')
	echo "Job ID = " $jobid	
	echo "----------------"

	# waiting between jobs
	sleep 10 
done











