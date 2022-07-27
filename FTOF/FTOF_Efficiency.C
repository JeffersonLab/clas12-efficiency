

#include "DC_Fiducial_Cuts_CLAS12.cxx"
#include <cstdlib>
#include <iostream>
#include <chrono>
#include <TFile.h>
#include <TTree.h>
#include <TApplication.h>
#include <TROOT.h>
#include <TDatabasePDG.h>
#include <TLorentzVector.h>
#include <TH1.h>
#include <TH2.h>
#include <TChain.h>
#include <TCanvas.h>
#include <TBenchmark.h>
#include "HipoChain.h"
#include "clas12reader.h"
#include <vector>
#include "rcdb_reader.h"

using namespace clas12;
using namespace std;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Creating vectors and variables

// Access information from PDG e.g. particle masses
auto db=TDatabasePDG::Instance();

// Define beam and target
TLorentzVector beam(0,0,0,0); // beam Px,Py,Pz,E
Double_t beam_E; // Used for the beam energy obtained from the RCDB

// TLorentzVector target(0,0,0,db->GetParticle(2212)->Mass()); // target Px,Py,Pz,E
TLorentzVector target; // target Px,Py,Pz,E

// Creating TLorentzVectors for detected particles
TLorentzVector el(0,0,0,db->GetParticle(11)->Mass()); // scattered e^- Px,Py,Pz,E
TLorentzVector pr(0,0,0,db->GetParticle(2212)->Mass()); // proton Px,Py,Pz,E
TLorentzVector pip(0,0,0,db->GetParticle(211)->Mass()); // pi^+ Px,Py,Pz,E
TLorentzVector pim(0,0,0,db->GetParticle(-211)->Mass()); // pi^+ Px,Py,Pz,E

// Values to switch on/off cuts for single track or 2 pi method, done automatically for different topologies
Double_t DC_Fiducial_Cuts = 0; // DC fiducial cuts off = 0, Inbending = 1, Outbending = 2
Double_t Beta_Cut = 0; // Beta cut off = 0, on = 1
Double_t Calorimeter_Cut = 0; // Calorimeter cut off = 0, on = 1
Double_t FTOF_Fiducial_Cuts = 0; // FTOF fiducial cuts = 0 off, loose = 1, tight = 2 (in development)

// Defining variables required for DC fiducial cut
// PID value
Int_t part_pid;

// x,y,z positions for 3 DC layers
Double_t part_DC_c1x,part_DC_c1y,part_DC_c1z;
Double_t part_DC_c2x,part_DC_c2y,part_DC_c2z;
Double_t part_DC_c3x,part_DC_c3y,part_DC_c3z;

// Positions and angles
Double_t x_1a, x_1b, x_2, y_1a, y_1b, y_2, z_1a, z_1b, z_2; // DC trajectory x,y,z positions
Double_t d_1a, d_1b, d_2; // Distance to xy position (used for calculating perpendicular distance)
Double_t x_FTOF, y_FTOF, z_FTOF; // Scintillator x,y,z positions
Double_t L_1a, L_1b, L_2; // Distance from beam to point at centre of counter
Double_t L_det_1a, L_det_1b, L_det_2; // Distance from beam to point in plane of FTOF layer (e.g FTOF1A/B 25 deg to xy plane)
Double_t alpha_1a, alpha_1b, alpha_2; // angle to hit (used to determine sector)
Double_t L_Perp_1a, L_Perp_1b, L_Perp_2; // Distance from centre of counter to hit
Double_t L_Theta = 60; // Angle at middle of sector (check if hit is left or right of the middle of the sector)
Double_t radia_residual; // Distance between DC trajectory and scintillator hit

Double_t Component; // Getting specific counter hit in scintillator

// Calorimeter information
Double_t PCAL_Energy, ECIN_Energy, ECOUT_Energy; // ECAL energy depositions

// Timing and beta
Double_t beta = 0;
Double_t Momentum = 0;

// Reconstructed pi^-
TLorentzVector misspim, misspip, missproton;


// Event information
Int_t runno=0; // Run number
Int_t eventno=0; // Event number
vector<TString> v_Runno; // vector of run numbers turned into strings
Int_t Binno = 0; // Count total files this corresponds to the number of x bins


// Particle numbers for 2pi events
Int_t negative, positive, elno, nonelectron, nonpip, nonproton;
// Creating variables for comparing detected and missing pion
Double_t DeltaP, DeltaTheta, DeltaPhi;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Create histograms here

// Checking the counter number vs L
auto* hlvscounter1A=new TH2F();
auto* hlvscounter1B=new TH2F();
auto* hlvscounter2=new TH2F();

// Testing Cuts
// DC fiducial cut
// Before
auto* h_DC_c1_xy_before = new TH2F();
auto* h_DC_c2_xy_before = new TH2F();
auto* h_DC_c3_xy_before = new TH2F();
// After
auto* h_DC_c1_xy_after = new TH2F();
auto* h_DC_c2_xy_after = new TH2F();
auto* h_DC_c3_xy_after = new TH2F();

// Beta Cut
// Before
auto* h_beta=new TH2F();
// After
auto* h_beta_cut_test=new TH2F();

// Calorimeter Cut
// Before
auto* h_PCAL_Energy = new TH1F();
auto* h_ECIN_Energy = new TH1F();
//After
auto* h_PCAL_Energy_cut_test = new TH1F();
auto* h_ECIN_Energy_cut_test = new TH1F();

// FTOF fiducial cut
// Before
auto* h_FTOF1A_LvsLperp_before = new TH2F();
auto* h_FTOF1B_LvsLperp_before = new TH2F();
auto* h_FTOF2_LvsLperp_before = new TH2F();
// After
auto* h_FTOF1A_LvsLperp_after = new TH2F();
auto* h_FTOF1B_LvsLperp_after = new TH2F();
auto* h_FTOF2_LvsLperp_after = new TH2F();

// Particle information
auto* hMomentum_1A_p=new TH1F();
auto* hMomentum_1B_p=new TH1F();
auto* hMomentum_2_p=new TH1F();
auto* hMomentum_1A_n=new TH1F();
auto* hMomentum_1B_n=new TH1F();
auto* hMomentum_2_n=new TH1F();
auto *h_z_vertex_before=new TH1D();
auto *h_z_vertex_after=new TH1D();
auto *h_Pid=new TH1D();


// Create arrays of 2d histograms for efficieny
// Arrays of histograms [topology][layer][charge][sector]
TH2F *h_Denominator[4][3][2][6]; // Trajectories from DC
TH2F *h_Numerator[4][3][2][6]; // Trajectories from DC with energy deposited in FTOF

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


// Define SetLorentzVector for four vectors of particles
void SetLorentzVector(TLorentzVector &p4,clas12::region_part_ptr rp){
   p4.SetXYZM(rp->par()->getPx(),rp->par()->getPy(),
   rp->par()->getPz(),p4.M());

}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Function to produce numerator and denominator histograms
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Second_Loop(int runno, int eventno, int Polarity, int i_topology, int i_charge, int i_detector, int i_sector, vector<region_part_ptr> particles){

   ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   // 2nd Loop over data to get detector information

   //Set the particle index to 0 and loop through the particles
   int pindex=0;
   for(auto& p : particles){

      //get information from the different detectors
      switch(p->getRegion()) {
         // Only look at particles in FD as FTOF is only in FD
         case FD :

         // Increase the particle index with each loop of the particles
         pindex++;

         //Ignore the first particle (trigger), any neutrals, and electrons
         if(pindex==1 || p->par()->getCharge()==0 || p->par()->getPid()==11) continue;

         if(p->par()->getCharge() < 0) i_charge = 0;
         else if(p->par()->getCharge() > 0) i_charge = 1;

         // Applying cuts based on topology
         // Turn off all cuts for 2 pi method
         if(i_topology < 4){
            DC_Fiducial_Cuts = 0; // DC fiducial cuts off
            Beta_Cut = 0; // Beta cut off
            Calorimeter_Cut = 0; // Calorimeter cut off
            FTOF_Fiducial_Cuts = 0; // FTOF fiducial cut off

            // For missing pim, ignore all positive particles
            if(i_topology == 1){
               if(i_charge == 1) continue;
            }
            // For missing pip, ignore the detected proton and negative particles
            if(i_topology == 2) {
               if(p->par()->getPid() == 2212) continue;
               if(i_charge == 0) continue;
            }
            // For missing proton, ignore the detected pi^+ and negative particles
            if(i_topology == 3) {
               if(p->par()->getPid() == 211) continue;
               if(i_charge == 0) continue;
            }
         }
         // Turn on cuts for single track method
         else if(i_topology == 4){
            if(Polarity == 0) DC_Fiducial_Cuts = 1; // inbending DC fiducial cuts on
            else if(Polarity == 1) DC_Fiducial_Cuts = 2; // outbending DC fiducial cuts on
            Beta_Cut = 1; // Beta cut on
            Calorimeter_Cut = 1; // Calorimeter cut on
            FTOF_Fiducial_Cuts = 0; // FTOF fiducial cut off
         }

         // Checking the z vertex before applying a cut
         h_z_vertex_before->Fill(p->par()->getVz());
         if(p->par()->getVz() > 2 || p->par()->getVz() < -9)continue;
         h_z_vertex_after->Fill(p->par()->getVz());

         // Setting calorimeter energy values to zero
         PCAL_Energy = 0;
         ECIN_Energy = 0;
         ECOUT_Energy = 0;

         // Grabbing the energy deposited in the 3 calorimeter layers
         if(p->cal(PCAL)->getEnergy()) PCAL_Energy = p->cal(PCAL)->getEnergy();
         if(p->cal(ECIN)->getEnergy()) ECIN_Energy = p->cal(ECIN)->getEnergy();
         if(p->cal(ECOUT)->getEnergy()) ECOUT_Energy = p->cal(ECOUT)->getEnergy();

         ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
         // DC fiducial cuts

         // Check if DC cuts are on
         if(DC_Fiducial_Cuts != 0){
            // DC co-ordinates before DC cuts
            h_DC_c1_xy_before->Fill(part_DC_c1x, part_DC_c1y);
            h_DC_c2_xy_before->Fill(part_DC_c2x, part_DC_c2y);
            h_DC_c3_xy_before->Fill(part_DC_c3x, part_DC_c3y);

            // Require hit in three layers of DC
            if((p->traj(DC,6)->getLayer() != 6 || p->traj(DC,18)->getLayer() != 18 || p->traj(DC,36)->getLayer() != 36))continue;

            // Get co-ordinates for layer 6
            if(p->traj(DC,6)->getDetector() == 6 && p->traj(DC,6)->getLayer() == 6){
               part_DC_c1x = p->traj(DC,6)->getX();
               part_DC_c1y = p->traj(DC,6)->getY();
               part_DC_c1z = p->traj(DC,6)->getZ();
            }
            // Get co-ordinates for layer 18
            if(p->traj(DC,18)->getDetector() == 6 && p->traj(DC,18)->getLayer() == 18){
               part_DC_c2x = p->traj(DC,18)->getX();
               part_DC_c2y = p->traj(DC,18)->getY();
               part_DC_c2z = p->traj(DC,18)->getZ();
            }
            // Get co-ordinates for layer 36
            if(p->traj(DC,36)->getDetector() == 6 && p->traj(DC,36)->getLayer() == 36){
               part_DC_c3x = p->traj(DC,36)->getX();
               part_DC_c3y = p->traj(DC,36)->getY();
               part_DC_c3z = p->traj(DC,36)->getZ();
            }


            // Reset PID value to zero each loop
            part_pid = 0;

            // If there is a PID value then it is taken from particle bank
            if(p->par()->getPid()) part_pid = p->par()->getPid();

            // If no PID is assigned to a particle they are assumed to be pions
            else if(p->par()->getCharge()>0) part_pid = 2; // Positive particles set to pi^+
            else if(p->par()->getCharge()<0) part_pid = 3; // Negative particles set to pi^-

            // Sector is determined in DC_Fiducial_Cuts_CLAS12.cxx using this function
            int part_DC_sector = determineSector(part_DC_c2x, part_DC_c2y,part_DC_c2z);

            //Checking which polarity to use
            if(DC_Fiducial_Cuts == 1){
               // Use this cut if looking at inbending data
               if(!DC_fiducial_cut_theta_phi(part_DC_c1x,part_DC_c1y,part_DC_c1z,part_DC_c2x, part_DC_c2y, part_DC_c2z,part_DC_c3x,part_DC_c3y, part_DC_c3z,part_pid,part_DC_sector,1) ||
               !DC_fiducial_cut_theta_phi(part_DC_c1x,part_DC_c1y,part_DC_c1z,part_DC_c2x, part_DC_c2y, part_DC_c2z,part_DC_c3x,part_DC_c3y, part_DC_c3z,part_pid,part_DC_sector,2) ||
               !DC_fiducial_cut_theta_phi(part_DC_c1x,part_DC_c1y,part_DC_c1z,part_DC_c2x, part_DC_c2y, part_DC_c2z,part_DC_c3x, part_DC_c3y, part_DC_c3z,part_pid,part_DC_sector,3))continue;
            }
            else if(DC_Fiducial_Cuts == 2){
               // Use this cut if looking at outbending data
               if(!DC_fiducial_cut_XY(part_DC_c1x,part_DC_c1y,part_DC_c1z,part_DC_c2x, part_DC_c2y, part_DC_c2z,part_DC_c3x, part_DC_c3y, part_DC_c3z,part_pid,part_DC_sector,1) ||
               !DC_fiducial_cut_XY(part_DC_c1x,part_DC_c1y,part_DC_c1z,part_DC_c2x, part_DC_c2y, part_DC_c2z,part_DC_c3x, part_DC_c3y, part_DC_c3z, part_pid,part_DC_sector,2) ||
               !DC_fiducial_cut_XY(part_DC_c1x,part_DC_c1y,part_DC_c1z,part_DC_c2x, part_DC_c2y, part_DC_c2z,part_DC_c3x, part_DC_c3y, part_DC_c3z, part_pid,part_DC_sector,3)) continue;
            }

            // Testing DC fiducial cut
            // DC co-ordinates after DC cuts
            h_DC_c1_xy_after->Fill(part_DC_c1x, part_DC_c1y);
            h_DC_c2_xy_after->Fill(part_DC_c2x, part_DC_c2y);
            h_DC_c3_xy_after->Fill(part_DC_c3x, part_DC_c3y);
         }
         ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


         // Check if calorimeter cuts are switched on
         if(Calorimeter_Cut == 1){
            h_PCAL_Energy->Fill(PCAL_Energy);
            h_ECIN_Energy->Fill(ECIN_Energy);
            // Only accept particles that deposit energy in PCAL and ECIN
            if(PCAL_Energy == 0 || ECIN_Energy == 0) continue;
            // Testing Calrimeter cut
            h_PCAL_Energy_cut_test->Fill(PCAL_Energy);
            h_ECIN_Energy_cut_test->Fill(ECIN_Energy);
         }


         // Defining momentum and beta of particles
         Momentum = p->par()->getP();
         beta = p->par()->getBeta();


         // Check if Beta cut is on
         if(Beta_Cut == 1){
            h_beta->Fill(Momentum,p->par()->getBeta());
            // Removing particles with unphysical beta
            if(beta < 0.4 || beta >1.1)continue;
            // Testing Beta cut
            h_beta_cut_test->Fill(Momentum,p->par()->getBeta());
         }


         // Getting the PID values
         h_Pid->Fill(p->par()->getPid());

         ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
         //FTOF1A

         // Requiring a DC trajectory in FTOF1A
         if(p->traj(FTOF,FTOF1A)->getDetector()==12 && p->traj(FTOF,FTOF1A)->getLayer()==1){

            // Set the detector number
            i_detector = 0;

            // Looking at momentum distribution for positive and negative particles
            if(p->par()->getCharge()>0) hMomentum_1A_p->Fill(p->par()->getP());
            else if(p->par()->getCharge()<0) hMomentum_1A_n->Fill(p->par()->getP());

            // Getting the x-, y- and z- co-ordinates from DC
            x_1a =  p->traj(FTOF, FTOF1A)->getX();
            y_1a =  p->traj(FTOF, FTOF1A)->getY();
            z_1a = p->traj(FTOF, FTOF1A)->getZ();

            // Requiring hit in FTOF1A
            if(p->sci(FTOF1A)->getEnergy()){

               // Getting x-, y- and z- co-ordinates from FTOF hit
               x_FTOF =  p->sci(FTOF1A)->getX();
               y_FTOF =  p->sci(FTOF1A)->getY();
               z_FTOF =  p->sci(FTOF1A)->getZ();

               // Distance between DC and FTOF co-ordinates
               radia_residual = sqrt(pow(x_1a-x_FTOF,2) + pow(y_1a-y_FTOF,2) + pow(z_1a-z_FTOF,2));
               // h_radia_residual_1A->Fill(radia_residual);
            }

            // Calculating distance to DC trajectory from (0,0)
            d_1a = sqrt(pow(x_1a,2) + pow(y_1a,2));

            // Calculating alpha, angle from (0,0) to DC trajectory
            alpha_1a = TMath::RadToDeg()*atan(y_1a/x_1a);

            // Calculating length from (0,0) along centre of sector
            // For sectors 1 and 4, L is just the x value as their centre is the x-axis
            if(fabs(alpha_1a) < 30) L_1a = fabs(x_1a);
            else L_1a = (d_1a * cos(TMath::DegToRad()*(fabs(fabs(alpha_1a)-60))));

            // Getting L in detector plane by accounting for tilt of FTOF1A (25 deg)
            L_det_1a = L_1a / cos(TMath::DegToRad()*25);
            // Calculating the distance along the width of the counter
            L_Perp_1a = pow(pow(d_1a,2) - pow(L_1a,2), 0.5);
            // Checking if hit is left or right of counter centre
            if(alpha_1a > 30){
               if(L_Theta - alpha_1a < 0) L_Perp_1a = - L_Perp_1a;
            }
            else if(fabs(alpha_1a) < 30) {
               if(alpha_1a < 0)L_Perp_1a = - L_Perp_1a;
            }
            else if(alpha_1a < -30) {
               if(L_Theta + alpha_1a < 0) L_Perp_1a = - L_Perp_1a;
            }




            // Checking if FTOF fiducial cuts are on
            if(FTOF_Fiducial_Cuts != 0){
               // FTOF co-ordinates before FTOF fiducial cut
               h_FTOF1A_LvsLperp_before->Fill(L_Perp_1a, L_det_1a);

               // Loose FTOF1A fiducial cuts
               if(FTOF_Fiducial_Cuts == 1){
                  if(L_det_1a < 77 || L_det_1a > 420.6 || (L_det_1a - (2.1 * fabs(L_Perp_1a))) < 43.09) continue;
               }

               // Tight FTOF1A fiducial cuts
               if(FTOF_Fiducial_Cuts == 2){
                  if(L_det_1a < 80 || L_det_1a > 410 || (L_det_1a - (2.2 * fabs(L_Perp_1a))) < 58) continue;
               }
               // FTOF co-ordinates before FTOF fiducial cut
               h_FTOF1A_LvsLperp_after->Fill(L_Perp_1a, L_det_1a);
            }


            // Testing FTOF fiducial cut

            // Resetting the component to unphysical value as check
            Component = -10;

            // Checking if there is energy deposition in FTOF1A
            if(p->sci(FTOF1A)->getEnergy()){
               // Getting the component number for the counter hit
               Component = p->sci(FTOF1A)->getComponent();
               // Plotting L vs the counter number
               hlvscounter1A->Fill(Component,L_det_1a);
            }

            // Determine which sector the hit is in using alpha
            // Look at positive x (sectors 1,2 and 6)
            if(x_1a > 0){

               // Determining sector for filling histograms
               if(alpha_1a > 30) i_sector = 2;
               else if(fabs(alpha_1a) < 30) i_sector = 1;
               else if(alpha_1a < -30) i_sector = 6;

               h_Denominator[i_topology - 1][i_detector][i_charge][i_sector - 1]->Fill(L_det_1a, L_Perp_1a);
               // Check if there is energy deposited on the scintillator for numerator
               if(p->sci(FTOF1A)->getEnergy()){
                  h_Numerator[i_topology-1][i_detector][i_charge][i_sector - 1]->Fill(L_det_1a, L_Perp_1a);
               }
            }

            // Look at negative x (sectors 3,4 and 5)
            else if(x_1a < 0){
               // Determining sector for filling histograms
               if(alpha_1a > 30) i_sector = 5;
               else if(fabs(alpha_1a) < 30) i_sector = 4;
               else if(alpha_1a < -30) i_sector = 3;

               // Filling numerator and denominator histograms
               h_Denominator[i_topology-1][i_detector][i_charge][i_sector - 1]->Fill(L_det_1a, L_Perp_1a);
               // Check if there is energy deposited on the scintillator
               if(p->sci(FTOF1A)->getEnergy()){
                  h_Numerator[i_topology-1][i_detector][i_charge][i_sector - 1]->Fill(L_det_1a, L_Perp_1a);
               }
            }
         }
         ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
         //FTOF1B

         // Requiring a DC trajectory in FTOF1B
         if(p->traj(FTOF,FTOF1B)->getDetector()==12 && p->traj(FTOF,FTOF1B)->getLayer()==2){

            // Setting the detector number
            i_detector = 1;

            // Looking at momentum distribution for positive and negative particles
            if(p->par()->getCharge()>0) hMomentum_1B_p->Fill(p->par()->getP());
            else if(p->par()->getCharge()<0) hMomentum_1B_n->Fill(p->par()->getP());

            // Getting the x-, y- and z- co-ordinates from DC
            x_1b =  p->traj(FTOF, FTOF1B)->getX();
            y_1b =  p->traj(FTOF, FTOF1B)->getY();
            z_1b = p->traj(FTOF, FTOF1B)->getZ();

            // Requiring hit in FTOF1B
            if(p->sci(FTOF1B)->getEnergy()){

               // Getting x-, y- and z- co-ordinates from FTOF hit
               x_FTOF =  p->sci(FTOF1B)->getX();
               y_FTOF =  p->sci(FTOF1B)->getY();
               z_FTOF =  p->sci(FTOF1B)->getZ();


               // Distance between DC and FTOF co-ordinates
               radia_residual = sqrt(pow(x_1b-x_FTOF,2) + pow(y_1b-y_FTOF,2) + pow(z_1b-z_FTOF,2));
               // h_radia_residual_1B->Fill(radia_residual);
            }

            // Calculating d, distance to hit from (0,0) to (x,y)
            d_1b = sqrt(pow(x_1b,2) + pow(y_1b,2));

            // Calculating alpha, angle from (0,0) to hit
            alpha_1b = TMath::RadToDeg()*atan(y_1b/x_1b);

            // Calculating length from (0,0) along centre of sector
            // For sectors 1 and 4, L is just the x value as their centre is the x-axis
            if(fabs(alpha_1b) < 30) L_1b = fabs(x_1b);
            else L_1b = (d_1b * cos(TMath::DegToRad()*(fabs(fabs(alpha_1b)-60))));

            // Getting L in detector plane by accounting for tilt of FTOF1B (25 deg)
            L_det_1b = L_1b / cos(TMath::DegToRad()*25);
            // Calculating the distance along the width of the counter from centre
            L_Perp_1b = pow(pow(d_1b,2) - pow(L_1b,2), 0.5);
            // Checking if hit is left or right of counter centre
            if(alpha_1b > 30){
               if(L_Theta - alpha_1b < 0) L_Perp_1b = - L_Perp_1b;
            }
            else if(fabs(alpha_1b) < 30) {
               if(alpha_1b < 0)L_Perp_1b = - L_Perp_1b;
            }
            else if(alpha_1b < -30) {
               if(L_Theta + alpha_1b < 0) L_Perp_1b = - L_Perp_1b;
            }

            // Resetting the component to unphysical value
            Component = -10;

            // Checking if there is energy deposition in FTOF1B
            if(p->sci(FTOF1B)->getEnergy()){
               // Getting the component number for the counter hit
               Component = p->sci(FTOF1B)->getComponent();
               // Plotting L vs the counter number
               hlvscounter1B->Fill(Component,L_det_1b);
            }

            // Determine which sector the hit is in using alpha and x_1b
            // Look at positive x (sectors 1,2 and 6)
            if(x_1b > 0){

               // Determining sector for filling histograms
               if(alpha_1b > 30) i_sector = 2;
               else if(fabs(alpha_1b) < 30) i_sector = 1;
               else if(alpha_1b < -30) i_sector = 6;

               // Filling numerator and denominator histograms
               h_Denominator[i_topology-1][i_detector][i_charge][i_sector - 1]->Fill(L_det_1b, L_Perp_1b);
               // Check if there is energy deposited on the scintillator
               if(p->sci(FTOF1B)->getEnergy()){
                  h_Numerator[i_topology-1][i_detector][i_charge][i_sector - 1]->Fill(L_det_1b, L_Perp_1b);
               }
            }

            // Look at negative x (sectors 3,4 and 5)
            else if(x_1b < 0){

               // Determining sector for filling histograms
               if(alpha_1b > 30) i_sector = 5;
               else if(fabs(alpha_1b) < 30) i_sector = 4;
               else if(alpha_1b < -30) i_sector = 3;

               // Filling numerator and denominator histograms
               h_Denominator[i_topology-1][i_detector][i_charge][i_sector - 1]->Fill(L_det_1b, L_Perp_1b);
               // Check if there is energy deposited on the scintillator
               if(p->sci(FTOF1B)->getEnergy()){
                  h_Numerator[i_topology-1][i_detector][i_charge][i_sector - 1]->Fill(L_det_1b, L_Perp_1b);
               }
            }
         }
         ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
         //FTOF2

         // Requiring a DC trajectory in FTOF1B
         if(p->traj(FTOF,FTOF2)->getDetector()==12 && p->traj(FTOF,FTOF2)->getLayer()==3){

            // Setting the detector number
            i_detector = 2;

            // Looking at momentum distribution for positive and negative particles
            if(p->par()->getCharge()>0) hMomentum_2_p->Fill(p->par()->getP());
            else if(p->par()->getCharge()<0) hMomentum_2_n->Fill(p->par()->getP());

            // Getting the x-, y- and z- co-ordinates from DC
            x_2 =  p->traj(FTOF, FTOF2)->getX();
            y_2 =  p->traj(FTOF, FTOF2)->getY();
            z_2 = p->traj(FTOF, FTOF2)->getZ();

            // Checking if there is energy deposition in FTOF2
            if(p->sci(FTOF2)->getEnergy()){

               // Getting x-, y- and z- co-ordinates from FTOF hit
               x_FTOF =  p->sci(FTOF2)->getX();
               y_FTOF =  p->sci(FTOF2)->getY();
               z_FTOF =  p->sci(FTOF2)->getZ();

               // Distance between DC and FTOF co-ordinates
               radia_residual = sqrt(pow(x_2-x_FTOF,2) + pow(y_2-y_FTOF,2) + pow(z_2-z_FTOF,2));
               // h_radia_residual_2->Fill(radia_residual);
            }

            // Calculating d, distance to hit from (0,0) to (x,y)
            d_2 = sqrt(pow(x_2,2) + pow(y_2,2));

            // Calculating alpha, angle from (0,0) to hit
            alpha_2 = TMath::RadToDeg()*atan(y_2/x_2);

            // Calculating length from (0,0) along centre of sector
            // For sectors 1 and 4, L is just the x value as their centre is the x-axis
            if(fabs(alpha_2) < 30) L_2 = fabs(x_2);
            else L_2 = (d_2 * cos(TMath::DegToRad()*(fabs(fabs(alpha_2)-60))));

            // Getting L in detector plane by accounting for tilt of FTOF2 (58.11 deg)
            L_det_2 = L_2 / cos(TMath::DegToRad()*58.11);
            // Calculating the distance along the width of the counter
            L_Perp_2 = pow(pow(d_2,2) - pow(L_2,2), 0.5);
            // Checking if hit is left or right of counter centre
            if(alpha_2 > 30){
               if(L_Theta - alpha_2 < 0) L_Perp_2 = - L_Perp_2;
            }
            else if(fabs(alpha_2) < 30) {
               if(alpha_2 < 0)L_Perp_2 = - L_Perp_2;
            }
            else if(alpha_2 < -30) {
               if(L_Theta + alpha_2 < 0) L_Perp_2 = - L_Perp_2;
            }

            // Resetting the component to unphysical value
            Component = -10;

            // Checking if there is energy deposition in FTOF2
            if(p->sci(FTOF2)->getEnergy()){
               // Getting the component number for the counter hit
               Component = p->sci(FTOF2)->getComponent();
               // Plotting L vs the counter number
               hlvscounter2->Fill(Component,L_det_2);
            }

            // Determine which sector the hit is in using alpha
            // Look at positive x (sectors 1,2 and 6)
            if(x_2 > 0){

               // Determining sector for filling histograms
               if(alpha_2 > 30) i_sector = 2;
               else if(fabs(alpha_2) < 30) i_sector = 1;
               else if(alpha_2 < -30) i_sector = 6;

               // Filling numerator and denominator histograms
               h_Denominator[i_topology-1][i_detector][i_charge][i_sector - 1]->Fill(L_det_2, L_Perp_2);
               // Check if there is energy deposited on the scintillator
               if(p->sci(FTOF2)->getEnergy()){
                  h_Numerator[i_topology-1][i_detector][i_charge][i_sector - 1]->Fill(L_det_2, L_Perp_2);
               }
            }

            // Look at negative x (sectors 3,4 and 5)
            else if(x_2 < 0){

               // Determining sector for filling histograms
               if(alpha_2 > 30) i_sector = 5;
               else if(fabs(alpha_2) < 30) i_sector = 4;
               else if(alpha_2 < -30) i_sector = 3;

               // Filling numerator and denominator histograms
               h_Denominator[i_topology-1][i_detector][i_charge][i_sector - 1]->Fill(L_det_2, L_Perp_2);
               // Check if there is energy deposited on the scintillator
               if(p->sci(FTOF2)->getEnergy()){
                  h_Numerator[i_topology-1][i_detector][i_charge][i_sector - 1]->Fill(L_det_2, L_Perp_2);
               }
            }
         }
      }
   }
}

// Loading macro
void FTOF_Efficiency(const std::string databaseF, TString inFileName, TString outFileName, TString polarity, const std::string targetMass){

   // Creating a TChain of all the input files
   // TChain fake("hipo");

   /////////////////////////////////////////////////////////////////////////
   // Loading the various files required

   // Connecting to the RCDB database file for the input file
   const std::string DBfile = databaseF;
   clas12databases::SetRCDBRootConnection(DBfile);

   // Defining input and output files
   // Data files to process
   TString inputFile = inFileName;
   TString outputFile = outFileName;

   // Adding the different input files to the TChain
   // fake.Add(inputFile.Data());



   // Output file location and name
   // TFile fileOutput1("FTOF1B_Sector_6_Check_29042022_01.root","recreate");
   TFile fileOutput1(outputFile,"recreate");

   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   // Set the polarity of the magnetic field for the DC fiducial cuts

   Double_t Polarity; // inbending is 0, outbending is 1
   // used to set DC fiducial cut automatically
   if(polarity == "in" || polarity == "In" || polarity == "Inbending" || polarity == "inbending") Polarity = 0;
   else if(polarity == "out" || polarity == "Out" || polarity == "Outbending" || polarity == "outbending") Polarity = 1;

   Double_t Pim_values[4];

   // convert string to double
   double targetM;
   targetM = stod(targetMass.c_str());

   Int_t eventcount=0;

   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   // Defining histograms

   // Checking the counter number vs L
   hlvscounter1A=new TH2F("hlvscounter1A","L vs counter FTOF1A;Counter;L [cm]",24,0,24,500,0,500);
   hlvscounter1B=new TH2F("hlvscounter1B","L vs counter FTOF1B;Counter;L [cm]",63,0,63,500,0,500);
   hlvscounter2=new TH2F("hlvscounter2","L vs counter FTOF2;Counter;L [cm]",6,0,6,300,600,900);

   // Testing Cuts
   // DC fiducial cut
   // Before
   h_DC_c1_xy_before = new TH2F("h_DC_c1_xy_before","DC layer 1 co-ordinates before fiducial cut",200,-200,200,200,-200,200);
   h_DC_c2_xy_before = new TH2F("h_DC_c2_xy_before","DC layer 2 co-ordinates before fiducial cut",300,-300,300, 300,-300,300);
   h_DC_c3_xy_before = new TH2F("h_DC_c3_xy_before","DC layer 3 co-ordinates before fiducial cut",500,-500,500,500,-500,500);
   // After
   h_DC_c1_xy_after = new TH2F("h_DC_c1_xy_after","DC layer 1 co-ordinates before fiducial cut",200,-200,200,200,-200,200);
   h_DC_c2_xy_after = new TH2F("h_DC_c2_xy_after","DC layer 2 co-ordinates before fiducial cut",300,-300,300,300,-300,300);
   h_DC_c3_xy_after = new TH2F("h_DC_c3_xy_after","DC layer 3 co-ordinates before fiducial cut",500,-500,500,500,-500,500);

   // Beta Cut
   // Before
   h_beta=new TH2F("h_beta","beta;P [GeV];beta",1100,0,11,400,-2,2);
   // After
   h_beta_cut_test=new TH2F("h_beta_cut_test","beta;P [GeV];beta",1100,0,11,400,-2,2);

   // Calorimeter Cut
   // Before
   h_PCAL_Energy = new TH1F("h_PCAL_Energy","PCAL energy before calorimeter cut",200,-0.5,1.5);
   h_ECIN_Energy = new TH1F("h_ECIN_Energy","ECIN energy before calorimeter cut",200,-0.5,1.5);
   //After
   h_PCAL_Energy_cut_test = new TH1F("h_PCAL_Energy_cut_test","PCAL energy after calorimeter cut",200,-0.5,1.5);
   h_ECIN_Energy_cut_test = new TH1F("h_ECIN_Energy_cut_test","ECIN energy after calorimeter cut",200,-0.5,1.5);

   // FTOF fiducial cut
   // Before
   h_FTOF1A_LvsLperp_before = new TH2F("h_FTOF1A_LvsLperp_before","FTOF1A xy before FTOF fiducial cut", 500,-250,250,500,0,500);
   h_FTOF1B_LvsLperp_before = new TH2F("h_FTOF1B_LvsLperp_before","FTOF1B xy before FTOF fiducial cut", 500,-250,250,500,0,500);
   h_FTOF2_LvsLperp_before = new TH2F("h_FTOF2_LvsLperp_before","FTOF2 xy before FTOF fiducial cut", 500,-250,250,500,0,500);
   // After
   h_FTOF1A_LvsLperp_after = new TH2F("h_FTOF1A_LvsLperp_after","FTOF1A xy after FTOF fiducial cut", 500,-250,250,500,0,500);
   h_FTOF1B_LvsLperp_after = new TH2F("h_FTOF1B_LvsLperp_after","FTOF1B xy after FTOF fiducial cut", 500,-250,250,500,0,500);
   h_FTOF2_LvsLperp_after = new TH2F("h_FTOF2_LvsLperp_after","FTOF2 xy after FTOF fiducial cut", 500,-250,250,500,0,500);

   // Particle information
   hMomentum_1A_p=new TH1F("hMomentum_1A_p", "Momentum of positive tracks in FTOF1A; Momentum [GeV/c]; Counts;", 200,0,8);
   hMomentum_1B_p=new TH1F("hMomentum_1B_p", "Momentum of positive tracks in FTOF1B; Momentum [GeV/c]; Counts;", 200,0,8);
   hMomentum_2_p=new TH1F("hMomentum_2_p", "Momentum of positive tracks in FTOF2; Momentum [GeV/c]; Counts;", 200,0,8);
   hMomentum_1A_n=new TH1F("hMomentum_1A_n", "Momentum of negative tracks in FTOF1A; Momentum [GeV/c]; Counts;", 200,0,8);
   hMomentum_1B_n=new TH1F("hMomentum_1B_n", "Momentum of negative tracks in FTOF1B; Momentum [GeV/c]; Counts;", 200,0,8);
   hMomentum_2_n=new TH1F("hMomentum_2_n", "Momentum of negative tracks in FTOF2; Momentum [GeV/c]; Counts;", 200,0,8);
   h_z_vertex_before=new TH1D("h_z_vertex_before","z vertex",100,-15,15);
   h_z_vertex_after=new TH1D("h_z_vertex_after","z vertex",100,-15,15);

   h_Pid=new TH1D("h_Pid","PID after cuts",10000,-5000,5000);


   // Defining the arrays of histograms
   // Looping over the 4 topologies
   for(Int_t i_topo = 0; i_topo < 4; i_topo++){
      // Looping over the 3 FTOF layers (FTOF1A, FTOF1B and FTOF2)
      for(Int_t i_detector=0;i_detector<3;i_detector++){
         // Looping over negative and positive particles
         for(Int_t i_charge=0;i_charge<2;i_charge++){
            // Looping over the 6 sectors
            for(Int_t i_sector=0;i_sector<6;i_sector++){


               // Histogram names
               ostringstream Denominator_name_stream;
               ostringstream Numerator_name_stream;

               // Histogram Titles
               ostringstream Denominator_title_stream;
               ostringstream Numerator_title_stream, Energy_title_stream;

               // Defining the histogram name strings
               Denominator_name_stream<<"h_Denominator_Topo_"<<i_topo<<"_Det_"<<i_detector<<"_Charge_"<<i_charge<<"_Sec_"<<i_sector;
               Numerator_name_stream<<"h_Numerator_Topo_"<<i_topo<<"_Det_"<<i_detector<<"_Charge_"<<i_charge<<"_Sec_"<<i_sector;

               // FTOF1A
               if (i_detector==0){
                  // Define string for histogram title
                  Denominator_title_stream<<"Topology "<<i_topo<<" Denominator FTOF1A Charge "<<2*i_charge-1<<" Sec "<<i_sector+1<<"; L [cm]; L_{Perp} [cm]";

                  // Define current histogram in array
                  // Average distance between counter centres is 15.18 cm
                  // FTOF 1A first counter starts at L = 77.16 cm
                  // FTOF 1A last counter ends at L = 426.3 cm
                  h_Denominator[i_topo][i_detector][i_charge][i_sector] = new TH2F(Denominator_name_stream.str().c_str(),"", 25, 61.98, 441.48, 50, -250, 250);
                  h_Numerator[i_topo][i_detector][i_charge][i_sector] = new TH2F(Numerator_name_stream.str().c_str(),"", 25, 61.98, 441.48, 50, -250 , 250);
               }

               // FTOF1B
               else if (i_detector==1){
                  // Define string for histogram title
                  Denominator_title_stream<<"Topology "<<i_topo<<" Denominator FTOF1B Charge "<<2*i_charge-1<<" Sec "<<i_sector+1<<"; L [cm]; L_{Perp} [cm]";

                  // Define current histogram in array
                  // Average distance between counter centres is 6.12 cm
                  // First counter starts at L = 52.16 cm
                  // Last counter ends at L = 431.6 cm
                  h_Denominator[i_topo][i_detector][i_charge][i_sector] = new TH2F(Denominator_name_stream.str().c_str(),"", 65, 46.04, 443.84, 50, -250, 250);
                  h_Numerator[i_topo][i_detector][i_charge][i_sector] = new TH2F(Numerator_name_stream.str().c_str(),"", 65, 46.04, 443.84, 50, -250 , 250);
               }

               // FTOF2
               else if (i_detector==2){
                  // Define string for histogram title
                  Denominator_title_stream<<"Topology "<<i_topo<<"Denominator FTOF2 Charge "<<2*i_charge-1<<" Sec "<<i_sector+1<<"; L [cm]; L_{Perp} [cm]";

                  // Define current histogram in array
                  // Average distance between counter centres is 22 cm
                  // First counter starts at L = 715 cm
                  // Last counter ends at L = 825 cm
                  h_Denominator[i_topo][i_detector][i_charge][i_sector] = new TH2F(Denominator_name_stream.str().c_str(),"", 7, 693, 847, 50, -250, 250);
                  h_Numerator[i_topo][i_detector][i_charge][i_sector] = new TH2F(Numerator_name_stream.str().c_str(),"", 7, 693, 847, 50, -250 , 250);
               }


               // Setting the title for each histogram
               h_Denominator[i_topo][i_detector][i_charge][i_sector]->SetTitle(Denominator_title_stream.str().c_str());
               h_Numerator[i_topo][i_detector][i_charge][i_sector]->SetTitle(Numerator_title_stream.str().c_str());
            }
         }
      }
   }


   ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   // 1st Loop over data to find 2pi events

   //Creating a chain for the data from different filesc12->queryRcdb();
   clas12root::HipoChain chain;
   chain.Add(inputFile);
   chain.SetReaderTags({0});  //create clas12reader with just tag 0 events
   auto config_c12=chain.GetC12Reader();

   chain.db()->turnOffQADB();

   auto& rcdbData= config_c12->rcdb()->current();

   auto& c12=chain.C12ref();

      // Looping over events in the current file
      while(chain.Next()==true){

         // check if it's a different run
         if(runno != c12->runconfig()->getRun()){

            Binno++; // Count the number of files, therefore the number of x bins
            runno = c12->runconfig()->getRun(); // Getting the run number

         }

         eventno = c12->runconfig()->getEvent(); // Getting the event number

         // Setting beam energy depending on which run it is
         beam_E = rcdbData.beam_energy*0.001;
         beam.SetXYZM(0, 0, beam_E, 0.000511);

         // Access the particle bank for each event
         auto particles = c12->getDetParticles();


         // Setting variables to zero at start  of each event
         negative = 0; // Number of negatives in event
         positive = 0; // Number of positives in event
         nonelectron = 0; // Number of negatives that aren't electrons
         nonproton = 0; // Number of positives that aren't protons
         nonpip = 0; // Number of positives that aren't pi^+

         // Differences between detected negative and reconstructed pi^-
         DeltaP = 0; // Momentum
         DeltaTheta = 0; // Theta
         DeltaPhi = 0; // Phi

         // Use event builder PID to grab particles
         auto electrons=c12->getByID(11);
         auto protons=c12->getByID(2212);
         auto pips=c12->getByID(211);
         auto pims=c12->getByID(-211);

         // Looping over all particles in this event
         for(auto& p : particles){

            //Ignoring neutral particles
            if(p->par()->getCharge() == 0) continue;

            // Looking at negative particles
            else if(p->par()->getCharge() < 0){
               negative++; // Count negative particles in this event

               if(p->par()->getPid() == 11) elno++;

               // negative particles not electron are set to pi^-
               if(p->par()->getPid() != 11){
                  nonelectron++; // Count the number of negatives excluding electrons
                  pim.SetXYZM(p->par()->getPx(),p->par()->getPy(),p->par()->getPz(),db->GetParticle(-211)->Mass());
               }
            }

            // Looking at positive particles
            else if(p->par()->getCharge() > 0){
               positive++; // Count positive particles in this event

               // positive particles not proton are set to pi^+
               if(p->par()->getPid() != 2212){
                  nonproton++;
                  pip.SetXYZM(p->par()->getPx(),p->par()->getPy(),p->par()->getPz(),db->GetParticle(211)->Mass());
               }
               // positive particles not pi^+ are set to proton
               if(p->par()->getPid() != 211){
                  nonpip++;
                  pr.SetXYZM(p->par()->getPx(),p->par()->getPy(),p->par()->getPz(),db->GetParticle(2212)->Mass());
               }
            }
         }
         // Selecting events with electron in FD for dst files
         if(electrons.size() != 1) continue;

         // Setting four vector for electrons
         SetLorentzVector(el,electrons[0]);

         // Checking electron is in FD
         if(electrons[0]->getRegion() != FD) continue;

         // Perform efficieny calculations for single track events
         Second_Loop(runno, eventno, Polarity, 4, -1, -1, -1, particles);

         // Selecting only events with 2 positive, 2 negative and 1 electron
         if(positive != 2 || negative != 2) continue;

         target.SetXYZM(0,0,0,targetM);

         // Getting missing pi^- events
         if(nonelectron == 1 && protons.size() == 1 && pips.size() == 1)
         {
            // Setting the four vectors
            SetLorentzVector(pr,protons[0]);
            SetLorentzVector(pip,pips[0]);

            // Putting chi^2 PID cut on detected particles
            if(fabs(electrons[0]->par()->getChi2Pid())>3 || fabs(protons[0]->par()->getChi2Pid())>3 || fabs(pips[0]->par()->getChi2Pid())>3)continue;

            // Reconstructed pi^-
            misspim = beam + target - el - pr - pip;

            // Differences between detected negative and reconstructed pi^-
            DeltaP = misspim.Rho() - pim.Rho();
            DeltaTheta = TMath::RadToDeg()* (misspim.Theta() - pim.Theta());
            DeltaPhi = TMath::RadToDeg()* (misspim.Phi() - pim.Phi());
            Pim_values[0] = misspim.M2();
            Pim_values[1] = DeltaP;
            Pim_values[2] = DeltaTheta;
            Pim_values[3] = DeltaPhi;

            // Cut on missing mass of the pi^-
            if(misspim.M2() > - 0.1 && misspim.M2() < 0.2){

               // Cuts to delta P, theta and phi to select true pi^-
               if(fabs(DeltaP) < 0.3 && fabs(DeltaTheta) < 10 && fabs(DeltaPhi) < 10 ){

                  // Run through Second_Loop to fill histograms for this topology
                  Second_Loop(runno, eventno, Polarity, 1, -1, -1, -1, particles);

               }
            }
         }

         // Getting events for missing pi^+
         if(pims.size() == 1 && protons.size() == 1 && nonproton == 1)
         {

            // Setting the four vectors
            SetLorentzVector(el,electrons[0]);
            SetLorentzVector(pr,protons[0]);
            SetLorentzVector(pim,pims[0]);

            // Putting chi^2 PID cut on detected particles
            if(fabs(electrons[0]->par()->getChi2Pid())>3 || fabs(protons[0]->par()->getChi2Pid())>3 || fabs(pims[0]->par()->getChi2Pid())>3)continue;

            // Reconstructed pi^+
            misspip = beam + target - el - pr - pim;

            // Differences between detected negative and reconstructed pi^+
            DeltaP = misspip.Rho() - pip.Rho();
            DeltaTheta = TMath::RadToDeg()* (misspip.Theta() - pip.Theta());
            DeltaPhi = TMath::RadToDeg()* (misspip.Phi() - pip.Phi());

            // Putting all the pi+ values in an array for filling the THn
            Double_t Pip_values[4] = {misspip.M2(), DeltaP, DeltaTheta, DeltaPhi};

            // hrecon_pip->Fill(Pip_values);

            // Cut on missing mass of the pi^+
            if(misspip.M2() > - 0.1 && misspip.M2() < 0.2){

               // Cuts to delta P, theta and phi to select true pi^+
               if(fabs(DeltaP) < 0.3 && fabs(DeltaTheta) < 10 && fabs(DeltaPhi) < 10 ){

                  // Run through Second_Loop to fill histograms for this topology
                  Second_Loop(runno, eventno, Polarity, 2, -1, -1, -1, particles);

               }
            }
         }

         // Getting events for missing proton
         if(pims.size() == 1 && pips.size() == 1 && nonpip == 1)
         {

            // Setting the four vectors
            SetLorentzVector(el,electrons[0]);
            SetLorentzVector(pip,pips[0]);
            SetLorentzVector(pim,pims[0]);

            // Putting chi^2 PID cut on detected particles
            if(fabs(electrons[0]->par()->getChi2Pid())>3 || fabs(pips[0]->par()->getChi2Pid())>3 || fabs(pims[0]->par()->getChi2Pid())>3)continue;

            // Reconstructed proton
            missproton = beam + target - el - pip - pim;

            // Differences between detected negative and reconstructed proton
            DeltaP = missproton.Rho() - pr.Rho();
            DeltaTheta = TMath::RadToDeg()* (missproton.Theta() - pr.Theta());
            DeltaPhi = TMath::RadToDeg()* (missproton.Phi() - pr.Phi());

            // Putting all the p values in an array for filling the THn
            Double_t P_values[4] = {missproton.M2(), DeltaP, DeltaTheta, DeltaPhi};
            // hrecon_p->Fill(P_values);

            // Cut on missing mass of the proton
            if(missproton.M() > 0.838 && missproton.M() < 1.038){

               // Cuts to delta P, theta and phi to select true proton
               if(fabs(DeltaP) < 0.3 && fabs(DeltaTheta) < 10 && fabs(DeltaPhi) < 10 ){

                  // Run through Second_Loop to fill histograms for this topology
                  Second_Loop(runno, eventno, Polarity, 3, -1, -1, -1, particles);

               }
            }
         }
      }

      v_Runno.push_back(to_string(runno)); // Converting runno integer to a string
   // }
   //saving the file
   fileOutput1.Write();
   fileOutput1.Close();

}
