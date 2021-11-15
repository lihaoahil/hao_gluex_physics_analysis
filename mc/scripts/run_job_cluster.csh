#!/bin/tcsh -f
set WORKINGDIR=/scratch/slurm_$SLURM_JOBID
hostname
#------------- Check Parameters --------------
echo "The job is running with         " $THREADS" threads on "$QUEUE" queue"
echo "Working directory is in         " $WORKINGDIR
echo "Output directory is in          " $OUTPUTDIR
echo "script directory is in          " $LOCALDIR
echo "config file is                  " ${LOCALDIR}/config/${CONFIG}
echo "single sample size =            " $TRIGGER

# set up environment
cd $WORKINGDIR
echo "source" $ENV_FILE ":"
source $ENV_FILE
scp $CCDB_SQLITE ./ccdb.sqlite
scp $RCDB_SQLITE ./rcdb.sqlite
setenv SQLITE_CCDB ./ccdb.sqlite
setenv CCDB_CONNECTION sqlite:////$WORKINGDIR/ccdb.sqlite
setenv SQLITE_RCDB ./rcdb.sqlite
setenv RCDB_CONNECTION sqlite:////$WORKINGDIR/rcdb.sqlite
setenv JANA_CALIB_URL "sqlite:///${PWD}/ccdb.sqlite"
setenv JANA_CALIB_CONTEXT "variation=mc"

# make new folders
mkdir -p $OUTPUTDIR/gen/
mkdir -p $OUTPUTDIR/root/
mkdir -p $OUTPUTDIR/rest/
mkdir -p $OUTPUTDIR/throwntrees/
mkdir -p $OUTPUTDIR/logs/
mkdir -p $OUTPUTDIR/mcthrown_trees/

################
# copy files in
################
cd $WORKINGDIR
cp ${OUTPUTDIR}/gen/def/input.def .
cp ${LOCALDIR}/control/control.in .
cp ${LOCALDIR}/config/${CONFIG}  reaction.config 

echo "-----------------------------------------------" 
echo "List of files already transferred to the QCD working directory"
ls -lah

###################
# run the programs
###################
touch timer.txt
echo
echo "----------------------------------------------------------------------------------------------" 
echo "path of softwares in the following simulations:"
which mc_gen
which GEN2HDDM_new 
which hdgeant4
which mcsmear
which hd_root
echo "----------------------------------------------------------------------------------------------" 
echo
echo


# run mc_gen and translator
echo "--------------------------------------------" |& tee -a timer.txt
echo "Running mc_gen"                               |& tee -a timer.txt
echo "--------------------------------------------" |& tee -a timer.txt
cd $WORKINGDIR
mc_gen input.def 
cp *.ascii input.ascii
GEN2HDDM_new -r040856 input.ascii 
echo "-----"
ls -lahS

# make run.mac
rm -f run.mac
echo "/run/beamOn $TRIGGER" >>! run.mac
echo "exit" >>! run.mac

# run simulation
echo "--------------------------------------------" 
echo "Running HDGeant4"
echo "--------------------------------------------"
hdgeant4 -t${THREADS} run.mac 


#smears the simulated data
if (-f hdgeant.hddm) then
		echo "--------------------------------------------"
		echo "Running mcsmear"
		echo "--------------------------------------------"
		mcsmear --nthreads=${THREADS} hdgeant.hddm -PTHREAD_TIMEOUT_FIRST_EVENT=300 -PTHREAD_TIMEOUT=300 
		rm hdgeant.hddm
	else
		echo "--------------------------------------------"
		echo "hdgeant.hddm is not found."
		exit
	endif



# runs the analysis
if (-f hdgeant_smeared.hddm) then
		echo "--------------------------------------------"
		echo "Running hd_root"
		echo "--------------------------------------------"
		hd_root hdgeant_smeared.hddm --nthreads=${THREADS} -PPLUGINS=danarest,mcthrown_tree -PTHREAD_TIMEOUT_FIRST_EVENT=300 -PTHREAD_TIMEOUT=300    
	else
		echo "--------------------------------------------"
		echo "hdgeant_smeared.hddm is not found."
		exit
	endif

# runs the reaction filter
if (-f dana_rest.hddm) then
		echo "--------------------------------------------"
		echo "Running hd_root --config=reaction.config "
		echo "--------------------------------------------"
		hd_root dana_rest.hddm --nthreads=${THREADS} --config=reaction.config   
	else
		echo "--------------------------------------------"
		echo "dana_rest.hddm is not found."
		exit
	endif




#################
# copy files out
#################
echo "--------------------------------------------"                                                                                                                                     
echo "Transferring files from compute nodes to server"                                                                                    
echo "Writing files to output directory" ${OUTPUTDIR}

#copy files and change their names to avoid override
scp timer.txt $OUTPUTDIR/logs/timer_${Name}.txt
if (-f input.ascii) then
		scp input.ascii $OUTPUTDIR/gen/thrown_${Name}.ascii
		#scp pre.seed $/gen/pre_${Name}.seed
		echo "input.ascii is copied back."
	else
		echo "The ASCII file is not found."
	endif

if (-f hd_root.root) then
		scp hd_root.root $OUTPUTDIR/root/${Name}.root
		echo hd_root.root " is copied back."
	else
		echo hd_root.root " is not found."
	endif
if (-f dana_rest.hddm) then
		scp dana_rest.hddm $OUTPUTDIR/rest/${Name}_rest.hddm
		echo dana_rest.hddm " is copied back."
	else
		echo dana_rest.hddm " is not found."
	endif
if (-f tree_thrown.root) then
		scp tree_thrown.root $OUTPUTDIR/mcthrown_trees/tree_thrown_${Name}.root
		echo tree_thrown.root " is copied back."
	else
		echo tree_thrown.root " is not found."
	endif


set myvar=`grep "Name" reaction.config | awk '{print $2}'`
foreach item($myvar)
	mkdir -p $OUTPUTDIR/throwntrees/${item}
	if (-f tree_${item}.root) then
		scp tree_${item}.root $OUTPUTDIR/throwntrees/${item}/tree_${item}_${fileNumber}.root
		echo tree_${item}.root " is copied back."
	else
		echo tree_${item}.root " is not found."
	endif
end
echo

echo "All found files have been transferred back to local output directory."
echo
echo "Simulation accomplished."
echo
echo "Prepare to exit."


	

exit








