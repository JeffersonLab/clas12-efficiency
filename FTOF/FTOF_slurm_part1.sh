#!/bin/bash

############################################################################
# Before running the code
############################################################################

# jcache hipo files that are not in the cache already
# Make sure to load clas12root:
### source /group/clas12/packages/setup.csh
### module load clas12/pro
# Make sure you have access to slurm account clas12
# Fill in the user defined variables section below
# Follow instructions in Setting runs to analyse

############################################################################
# User defined variables
############################################################################

# Efficiency macro location
MacroLoc="/Where/FTOF/macros/are"

# Where you want all the output files to go
OutputDir="/Where/Outputs/Go"

# Where the data is stored
DataDir="/Location/Of/Data"


#############
# Setting runs to analyse
#############

## Option 1 ##
## This option is useful for running over lots of files. Run over all
## available runs between minimum and maximum. For example running with
## options (Min=5032 and Max=5036) would process runs 5032 and 5036 as there
## is no data for 5033, 5034, and 5035.
Min=5032
Max=5038

## Option 2 ##
## This option is good for running over a few select runs. Run over
## specific runs in an array, just list runs in the RUNS array with a space
## between each. For example, to run over 5032, 5036, and 5038
## use RUNS=(5032 5036 5038)
RUNS=(5032 5036 5038)

# Comment out the option you do not want in the “Making RCDB files” section

#############
# TOF variables
#############

# ROOT output name, this is followed by the sequence number automatically
OutputName="Name_Of_Ouput_Files"

# Polarity of data, use Inbending/inbending/In/in or Outbending/outbending/Out/out
Polarity="Inbending"

# Target mass (GeV)
targetMass=0.93827


############################################################################
# Setting and making additional variables and data
############################################################################

# Output name, this is followed by the sequence number automatically
RCDBOutputName=$OutputName'_PrepareDatabases'

# List of ouput ROOT files, used in part 2
List=$OutputDir"/ROOT/list.txt"


# Check if the output directory exists, if not it makes them
if [ ! -d "$OutputDir" ]; then
   mkdir $OutputDir
   mkdir $OutputDir'/ROOT'
   mkdir $OutputDir'/slurm'
   mkdir $OutputDir'/RCDB'
   mkdir $OutputDir'/Plots'
fi

# Move to output directory
cd $OutputDir'/slurm'


# Check if a list already exists, if it does it deletes it
if [ -f "$List" ]; then
   rm $List
fi
# Making list
touch $List

############################################################################
# Making RCDB files
############################################################################

## Option 1 ##
for NUMBER in $(seq $Min $Max);

## Option 2 ##
#for NUMBER in "${RUNS[@]}"


do
   echo $NUMBER
   # Check if the run exists
   FILE=$DataDir'/00'$NUMBER
   if [ ! -d "$FILE" ]; then
      continue
   fi


   # Check if the RCDB slurm files exists
   RCDBSlurm='process_PrepareDatabases_'$NUMBER'.sh'
   if [ -f "$RCDBSlurm" ]; then
      rm process_PrepareDatabases_"$NUMBER".sh
      rm slurm_PrepareDatabases_"$NUMBER".sh
      echo removed
   fi



   touch process_PrepareDatabases_"$NUMBER".sh

   echo 'gROOT->ProcessLine(".L '$MacroLoc'/PrepareDatabases.C");' >> process_PrepareDatabases_"$NUMBER".sh
   echo 'gROOT->ProcessLine("PrepareDatabases(\"'$DataDir'/00'$NUMBER'/*.hipo\",\"'$OutputDir'/RCDB/'$RCDBOutputName'_'$NUMBER'.root\")");' >> process_PrepareDatabases_"$NUMBER".sh

   echo "Made process_PrepareDatabases_"$NUMBER".sh"



   touch slurm_PrepareDatabases_"$NUMBER".sh

   echo '#!/bin/bash' >> slurm_PrepareDatabases_"$NUMBER".sh
   echo '#SBATCH --nodes=1' >> slurm_PrepareDatabases_"$NUMBER".sh
   echo '#SBATCH --ntasks=1' >> slurm_PrepareDatabases_"$NUMBER".sh
   echo '#SBATCH --mem-per-cpu=3000' >> slurm_PrepareDatabases_"$NUMBER".sh
   echo '#SBATCH --job-name=PrepareDatabases_'$NUMBER'' >> slurm_PrepareDatabases_"$NUMBER".sh
   echo '#SBATCH --partition=production' >> slurm_PrepareDatabases_"$NUMBER".sh
   echo '#SBATCH --account=clas12' >> slurm_PrepareDatabases_"$NUMBER".sh
   echo '/bin/more process_PrepareDatabases_'$NUMBER'.sh | clas12root -b' >> slurm_PrepareDatabases_"$NUMBER".sh

   echo "Made slurm_PrepareDatabases_"$NUMBER".sh"

   # Running the RCDB file creating
   RES=$(sbatch slurm_PrepareDatabases_$NUMBER.sh)


   sleep 2

   ############################################################################
   # Making FTOF efficieny files
   ############################################################################

   # Check if the FTOF slurm files exists
   FTOFSlurm='process_FTOF'$NUMBER'.sh'
   if [ -f "$FTOFSlurm" ]; then
      rm process_FTOF"$NUMBER".sh
      rm slurm_FTOF_$NUMBER.sh
   fi


   touch process_FTOF"$NUMBER".sh


   echo 'gROOT->ProcessLine(".L '$MacroLoc'/FTOF_Efficiency.C");' >> process_FTOF"$NUMBER".sh
   echo 'gROOT->ProcessLine("FTOF_Efficiency(\"'$OutputDir'/RCDB/'$RCDBOutputName'_'$NUMBER'.root\",\"'$DataDir'/00'$NUMBER'/*.hipo\",\"'$OutputDir'/ROOT/'$OutputName$NUMBER'.root\",\"'$Polarity'\",\"'$targetMass'\")");' >> process_FTOF"$NUMBER".sh

   echo $OutputDir"/ROOT/"$OutputName$NUMBER".root" >> $List

   echo "Made process_FTOF"$NUMBER".sh"

   touch slurm_FTOF_$NUMBER.sh

   echo '#!/bin/bash' >> slurm_FTOF_$NUMBER.sh
   echo '#SBATCH --nodes=1' >> slurm_FTOF_$NUMBER.sh
   echo '#SBATCH --ntasks=1' >> slurm_FTOF_$NUMBER.sh
   echo '#SBATCH --mem-per-cpu=3000' >> slurm_FTOF_$NUMBER.sh
   echo '#SBATCH --job-name=FTOF_'$NUMBER'' >> slurm_FTOF_$NUMBER.sh
   echo '#SBATCH --partition=production' >> slurm_FTOF_$NUMBER.sh
   echo '#SBATCH --account=clas12' >> slurm_FTOF_$NUMBER.sh
   echo '/bin/more process_FTOF'$NUMBER'.sh | clas12root -b' >> slurm_FTOF_$NUMBER.sh

   echo "Made /slurm_FTOF_"$NUMBER".sh"

   sbatch --dependency=afterok:${RES##* } slurm_FTOF_$NUMBER.sh
   sleep 2

done
