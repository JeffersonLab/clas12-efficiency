#!/bin/bash

############################################################################
# Running the code
############################################################################

# Make sure to load root5:
## source /apps/root/5.34.36/root/bin/thisroot.csh
# Fill in the user defined variables section below
# Run using the command:
## ./FTOF_slurm_part2.sh

############################################################################
# User defined variables
############################################################################


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


############################################################################
# combining the ROOT output files
############################################################################

cd $OutputDir'/ROOT/'

# List of ouput ROOT files, used in part 2
List=$OutputDir"/ROOT/list.txt"

# Name of merged total output
Total="Total.root"

rm canvas.sh
touch canvas.sh


hadd -f "$OutputDir"/ROOT/"$Total" @"$List"

sleep 2


echo 'gROOT->ProcessLine(".L '$MacroLoc'/Canvases.C");' >> canvas.sh
echo 'gROOT->ProcessLine("Canvases(\"'$OutputDir'/ROOT/'$Total'\",\"'$OutputDir'\",\"'$OutName'\",\"'$Min'\",\"'$Max'\")");' >> canvas.sh

/bin/more canvas.sh | root -b
