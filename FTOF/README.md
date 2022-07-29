The FTOF effi	ciency is performed in two stages. The first stage produces RCDB files for run information and processes the data files separately for reach run. The second stage combines all the processed files, determines the efficiencies and produces both a ROOT and canvas output of these. Currently, this is only to be run on the ifarm. 

Before running the FTOF efficiency code:
Make sure all hipo files are present, jcache any missing ones
Load clas12root:
	source /group/clas12/packages/setup.csh
	module load clas12/pro
Make sure you have access to slurm account ‘clas12’ (https://scicomp.jlab.org/scicomp/slurmJob/slurmAccount)

Stage 1
Setting up stage 1

Open FTOF_slurm_part1.sh
Fill in user defined variables section

# Efficiency macro location
MacroLoc="/Where/FTOF/macros/are"

# Where you want all the output files to go
OutputDir="/Where/Outputs/Go"

# Where the data is stored
DataDir="/Location/Of/Data"


There are two options for running this. 
Option 1 – This option is useful for running over lots of files. Run over all available runs between minimum and maximum. For example running with options (Min=5032 and Max=5036) would process runs 5032 and 5036 as there is no data for 5033, 5034, and 5035. 

Option 2 – This option is good for running over a few select runs. Run over specific runs in an array, just list runs in the RUNS array with a space between each. For example, to run over 5032, 5036, and 5038 use RUNS=(5032 5036 5038)
RUNS=(5032 5036 5038)

Comment out the option you do not want in the “Making RCDB files” section. For example, if you want to use option 1:
############################################################################
# Making RCDB files
############################################################################

## Option 1 ##
for NUMBER in $(seq $Min $Max);

## Option 2 ##
#for NUMBER in "${RUNS[@]}"

# ROOT output name, this is followed by the run number automatically
OutputName="Name_Of_Ouput_Files"

# Polarity of data, use Inbending/inbending/In/in or Outbending/outbending/Out/out
Polarity="Inbending"

# Target mass (GeV)
targetMass=0.93827

Running stage 1
Once set up, simply navigate to the location of the FTOF macros and enter the following command in the terminal:
./FTOF_slurm_part1.sh


Stage 2
Setting up stage 2

Open FTOF_slurm_part2.sh
Fill in user defined variables section

# MacroLoc and OutputDir should match what was used in part 1

# Efficiency macro location
MacroLoc="/Where/FTOF/macros/are"

# Where you want all the output files to go
OutputDir="/Where/Outputs/Go"

# Name of the efficiency output ROOT file
OutName="Name_Of_Efficiency_Ouput_Files"

# Range for efficiency histograms, only effects canvas pdfs not ROOT output
Min=0.75
Max=1.1

Before running Stage 2:
Make sure stage 1 is complete. Check your slurm jobs either through ‘squeue -u username’ in ifarm terminal or go to https://scicomp.jlab.org/scicomp/slurmJob/activeJob
Depending on the size of the runs you’re processing, it can take anywhere up to about an hour.

Load root5:
source /apps/root/5.34.36/root/bin/thisroot.csh

Running stage 2
Once stage 1 is finished, navigate to location of FTOF macros and enter the following command in the terminal:
./FTOF_slurm_part2.sh

