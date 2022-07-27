#!/bin/bash

############################################################################
# Running the code
############################################################################

# Make sure to load root5: 
## For ifarm usage
### source /apps/root/5.34.36/root/bin/thisroot.csh
# Fill in the user defined variables section below
# Run using the command:
## ./FTOF_slurm_part2.sh

############################################################################
# User defined variables
############################################################################


#### These should match what was used in part 1
# Efficiency macro location
MacroLoc="/volatile/clas12/matthewn/FTOF_Test"

# Where you want the temporary files to go
OutputDir="/volatile/clas12/matthewn/FTOF_Test/temp3"

# Total outut file name
Total="Total_Output.root"

####

# Name of the efficiency output ROOT file
OutName="RGA_Fall18_Inbending5032_Efficiency_Output_260722.root"

# Range for efficiency histograms
Min=0.75
Max=1.1


############################################################################
# combining the ROOT output files
############################################################################

cd $OutputDir'/ROOT/'

# List of ouput ROOT files, used in part 2
List=$OutputDir"/ROOT/list.txt"

rm canvas.sh
touch canvas.sh


hadd -f "$OutputDir"/ROOT/"$Total" @"$List"

sleep 2


echo 'gROOT->ProcessLine(".L '$MacroLoc'/Canvases.C");' >> canvas.sh
echo 'gROOT->ProcessLine("Canvases(\"'$OutputDir'/ROOT/'$Total'\",\"'$OutputDir'\",\"'$OutName'\",\"'$Min'\",\"'$Max'\")");' >> canvas.sh
  
/bin/more canvas.sh | root -b
