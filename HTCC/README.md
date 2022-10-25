Here are codes etc. for HTCC

HTCCEfficiencyData.dat contains the efficiencies in a x-y space
htcc.png shows a visualization of the file.

An example to read the file is shown below utilizing trajectories to HTCC with CLAS12root functions
void readHTCCEff(float HTCCEff[250][250]){
ifstream htccEffFile("params/HTCCEfficiencyData.dat");
for (int x = 0; x < 250; x++){
for (int y = 0; y < 250; y++){
htccEffFile >> HTCCEff[x][y];
}
}
htccEffFile.close();
} 

const int nHTCCXBins = 250;
const int nHTCCYBins = 250;
int stepXHTCC = 1;
int stepYHTCC = 1;

//In order to get actual X and Y  use:

int htccXBin = (x + 125)/stepXHTCC;
int htccYBin = (y + 125)/stepYHTCC;

float eff = HTCCEff[htccXBin][htccYBin];

//where x and y is (clas12root code):
electron[u]->traj(HTCC,1)->getX()
electron[u]->traj(HTCC,1)->getY()

//eff is efficiency for x and y from htcc data bank.