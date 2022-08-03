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
#include "clas12reader.h"
#include <vector>
#include "rcdb_reader.h"
#include "HipoChain.h"
#include "clas12defs.h"
#include "clas12databases.h"
#include "reader.h"
#include "particle.h"

void PrepareDatabases(TString infile, TString outfile){


#ifdef CLAS_RCDB
  /* For rcdb creaate a HipoChain of data files and the RCDB info will be
     downloaded and saved in a ROOT file for those runs*/
  clas12databases::SetRCDBRemoteConnection();

  clas12root::HipoChain chain;
  //Add you data files here
  TString inputFile = infile;
  TString outputFile = outfile;

  chain.Add(inputFile);
  chain.WriteRcdbData(outputFile);

#endif

}
