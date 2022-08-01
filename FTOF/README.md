The FTOF efficiency is performed in two stages. The first stage produces RCDB files for run information and processes the data files separately for reach run. The second stage combines all the processed files, determines the efficiencies and produces both a ROOT and canvas output of these. Currently, this is only to be run on the ifarm. 

<h3>Before running the FTOF efficiency code: </h3>
Make sure all hipo files are present, jcache any missing ones <br />
Load clas12root: <br />
	source /group/clas12/packages/setup.csh <br />
	module load clas12/pro <br />
Make sure you have access to slurm account ‘clas12’ (https://scicomp.jlab.org/scicomp/slurmJob/slurmAccount) <br />

<h3>Stage 1 </h3>
<h4>Setting up stage 1 </h4>

Open FTOF_slurm_part1.sh<br />
Fill in user defined variables section<br />
<br />

There are two options for running this. <br />
Option 1 – This option is useful for running over lots of files. Run over all available runs between minimum and maximum. For example running with options (Min=5032 and Max=5036) would process runs 5032 and 5036 as there is no data for 5033, 5034, and 5035. <br />
<br />
Option 2 – This option is good for running over a few select runs. Run over specific runs in an array, just list runs in the RUNS array with a space between each. For example, to run over 5032, 5036, and 5038 use RUNS=(5032 5036 5038)<br />
<br />
Comment out the option you do not want in the “Making RCDB files” section. For example, if you want to use option 1:<br />
<pre>############################################################################
# Making RCDB files 
############################################################################ 

## Option 1 ##
for NUMBER in $(seq $Min $Max);

## Option 2 ##
#for NUMBER in "${RUNS[@]}"

</pre>
<h4>Running stage 1 </h4>
Once set up, simply navigate to the location of the FTOF macros and enter the following command in the terminal:<br />
./FTOF_slurm_part1.sh<br />
<br />

<h3>Stage 2 </h3>
<h4>Setting up stage 2 </h4>

Open FTOF_slurm_part2.sh<br />
Fill in user defined variables section<br />
<br />
MacroLoc and OutputDir should match what was used in part 1 <br />
<br />

<h4>Before running Stage 2: </h4>
Make sure stage 1 is complete. Check your slurm jobs either through ‘squeue -u username’ in ifarm terminal or go to  
<a href="https://scicomp.jlab.org/scicomp/slurmJob/activeJob">(Slurm Accounts)</a> <br />

Depending on the size of the runs you’re processing, it can take anywhere up to about an hour. <br />
<br />
Load root5:<br />
source /apps/root/5.34.36/root/bin/thisroot.csh<br />
<br />
<h4>Running stage 2</h4>
Once stage 1 is finished, navigate to location of FTOF macros and enter the following command in the terminal:<br />
./FTOF_slurm_part2.sh<br />
<br />

