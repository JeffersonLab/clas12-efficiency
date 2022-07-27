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
MacroLoc="/volatile/clas12/matthewn/FTOF_Test"

# Where you want all the output files to go
OutputDir="/volatile/clas12/matthewn/FTOF_Test/temp3"

# Where the data is stored
DataDir="/cache/clas12/rg-a/production/recon/fall2018/torus-1/pass1/v0/dst/recon"


#############
# Setting runs to analyse
#############

## Option 1 ##
## You can either run over all available runs between two numbers with option 1
## For example "5032 5038" would run over runs: 5032, 5036, and 5038


## Option 2 ##
## Or you can specify a list of runs you would like to analyse.
## Simply list them inside the brackets below separated by a space 

RUNS=(5032 5036 5038)

# Make sure the option you want is included in section Making RCDB Files and the one 
# you don't is commented out
# To run option 1 
# ./FTOF_slurm_part1.sh minimumrun maximumrun
# To run option 2
# ./FTOF_slurm_part1.sh

#############
# TOF variables
#############

# ROOT output name, this is followed by the sequence number automatically
OutputName="RGA_Fall18_Inbending"

# use Inbending/inbending/In/in or Outbending/outbending/Out/out
Polarity="Inbending"

# Target mass
targetMass=0.93827


############################################################################
# Setting and making additional variables and data
############################################################################

# Output name, this is followed by the sequence number automatically
RCDBOutputName="RGA_PrepareDatabases_Fall18_Inbending"

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
for NUMBER in `seq $1 $2`;

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