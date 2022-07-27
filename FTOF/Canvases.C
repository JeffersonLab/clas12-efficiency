

void Canvases(TString InFile, TString OutDir, TString OutName, TString Minimum, TString Maximum){
   //////////////////////////////////////////////////////////////////////////////
   //// Setting Variables  ////////////////////////////////////////////////////
   //////////////////////////////////////////////////////////////////////////////


   // Give the input analysis file to determine efficieny for
   TString InputFile = InFile;
   TFile *f1=new TFile(InputFile);


   // Information for canvas and histogram name
   // Where canvases and ROOT file are saved
   TString Canvas_Out = OutDir;
   // Setting the strings for canvas name
   TString FileName = OutName;
   ostringstream OutputFull;
   OutputFull << Canvas_Out << "/ROOT/" << FileName;

   // Output root file
   TFile fout(OutputFull.str().c_str(),"recreate");

   // Range for efficiency plots
   string MinimumValue = Minimum;
   string MaximumValue = Maximum;
   // convert string to double

   float Min;
   float Max;
   Min = atof(MinimumValue.c_str());
   Max = atof(MaximumValue.c_str());

   //////////////////////////////////////////////////////////////////////////////
   //// End of user defined values  ////////////////////////////////////////////////////
   //////////////////////////////////////////////////////////////////////////////

   // Further information for canvas and histogram name
   ostringstream Topology;
   ostringstream Layer;
   ostringstream Charge;
   ostringstream Sector;


   //////////////////////////////////////////////////////////////////////////////
   ////Creating arrays of histograms and canvases  /////////////////////////////
   //////////////////////////////////////////////////////////////////////////////


   // Denominator
   TH1F *Denominator_y[4][3][2][6];
   TH1F *Overall_Denominator_y[3][2][6];

   // Numerator
   TH1F *Numerator_y[4][3][2][6];
   TH1F *Overall_Numerator_y[3][2][6];

   // Efficiency
   TH1F *Efficiency[4][3][2][6];
   TH1F *Overall_Efficiency[3][2][6];

   // Creating arrays of canvases [FTOF layer] [charge]
   TCanvas* canvas_y[5][3][2]; // canvases for efficieny vs L

   gStyle->SetOptStat(0); // Remove statistics from canvases

   //////////////////////////////////////////////////////////////////////////////
   ////Loop over topology, layer, charge, and sector ///////////
   //////////////////////////////////////////////////////////////////////////////

   Int_t detector_min, detector_max; // Limits for FTOF layers
   Int_t charge_min, charge_max; // Limits for charge

   // Looping over topology
   for(Int_t i_topology=0;i_topology < 4;i_topology++){

      // Defining topology specific strings and parameters

      // Missing pi^-
      if(i_topology == 0) {
         // Limiting the charge and layers to look at based on topology
         charge_min = 0; // Start at negatives
         charge_max = 1; // Only do negatives for pi-
         detector_min = 0; // Start at 1A
         detector_max = 3; // Do all layers (1A, 1B, and 2) for pi-

         // Name the topology
         Topology.str("pim");

      }

      // Missing pi^+ and missing proton
      else if(i_topology == 1 || i_topology == 2) {
         // Limiting the charge and layers to look at based on topology
         charge_min = 1; // Start at positives
         charge_max = 2; // Only do positives for pi+ and p
         detector_min = 0; // Start at 1A
         detector_max = 3; // Do all layers (1A, 1B, and 2) for pi+ and p

         // Name the topology
         if(i_topology == 1) Topology.str("pip");
         if(i_topology == 2) Topology.str("proton");
      }

      // Single track events
      else if(i_topology == 3) {
         // Limiting the charge and layers to look at based on topology
         charge_min = 0; // Start at negatives
         charge_max = 2; // Do positives and negatives for single track
         detector_min = 0; // Start at 1A
         detector_max = 2; // Only do 1A and 1B for single track

         // Name the topology
         Topology.str("single_track");
      }


      // Looping over the FTOF layers
      for(Int_t i_detector = detector_min; i_detector < detector_max; i_detector++){

         // Name the FTOF layer
         if(i_detector == 0) Layer.str("1A");
         else if(i_detector == 1) Layer.str("1B");
         else if(i_detector == 2) Layer.str("2");


         // Looping over charge
         for(Int_t i_charge = charge_min; i_charge < charge_max; i_charge++){

            // Name the charge
            if(i_charge == 0) Charge.str("negatives");
            else if(i_charge == 1) Charge.str("positives");

            // Creating strings for canvas names and file names
            ostringstream canvas_y_name_stream;
            ostringstream canvas_y_file_name_stream;


            // Setting the name for each canvas
            if(i_topology < 3){
               canvas_y_name_stream << Topology.str().c_str() << "_" << Layer.str().c_str() ;
            }
            else{
               canvas_y_name_stream << Topology.str().c_str() << "_" << Charge.str().c_str() << "_" << Layer.str().c_str() ;
            }


            // Setting the file name for each canvas
            if(i_topology < 3){
               canvas_y_file_name_stream << Canvas_Out << "/Plots/" << Topology.str().c_str() << "_" << Layer.str().c_str() << "_" << FileName<<".pdf";
            }
            else{
               canvas_y_file_name_stream << Canvas_Out << "/Plots/" << Topology.str().c_str() << "_" << Charge.str().c_str() << "_" << Layer.str().c_str() << "_" << FileName << ".pdf";

            }

            // creating canvas for each topology, FTOF layer, and charge
            canvas_y[i_topology][i_detector][i_charge] = new TCanvas(canvas_y_name_stream.str().c_str(),"", 2500,1600);
            canvas_y[i_topology][i_detector][i_charge]->Divide(3,2);


            // Looping over the sectors
            for(Int_t i_sector = 0; i_sector < 6; i_sector++){

               // Printing out the loop
               cout<<"topology "<<i_topology<<" detector "<<i_detector<<" charge "<<i_charge<<" sector "<<i_sector<<endl;

               // Creating strings for the histograms and canvases
               ostringstream Denominator_name_stream, Numerator_name_stream;
               ostringstream pi_Denominator_name_stream, pi_Numerator_name_stream;
               ostringstream Efficiency_name_stream, Efficiency_title_stream;

               //////////////////////////////////////////////////////////////////////////////
               // Setting the histogram names and titles //////////////////////////////////////////////
               //////////////////////////////////////////////////////////////////////////////

               Denominator_name_stream<<"h_Denominator_Topo_"<<i_topology<<"_Det_"<<i_detector<<"_Charge_"<<i_charge<<"_Sec_"<<i_sector;
               Numerator_name_stream<<"h_Numerator_Topo_"<<i_topology<<"_Det_"<<i_detector<<"_Charge_"<<i_charge<<"_Sec_"<<i_sector;
               Efficiency_name_stream << "h_eff_Topo_" << i_topology << "_Det_" << i_detector << "_Charge_" << i_charge << "_Sec_" << i_sector;

               // For 2 pi topologies
               if(i_topology < 3){
                  Efficiency_title_stream<<Topology.str().c_str()<<" Efficiency of FTOF"<<Layer.str().c_str()<<" Sector "<<i_sector + 1<<"; Counter; #epsilon";
               }
               // single track topology
               else if(i_topology == 3){
                  Efficiency_title_stream<<Topology.str().c_str()<<" Efficiency of "<<Charge.str().c_str()<<" FTOF"<<Layer.str().c_str()<<" Sector "<<i_sector + 1<<"; Counter; #epsilon";
               }

               //////////////////////////////////////////////////////////////////////////////
               // Getting histograms from input file  ///////////////////////////////////////
               //////////////////////////////////////////////////////////////////////////////

               // Getting the histograms from data file
               TH2F *h_Denominator = (TH2F*) f1->Get(Denominator_name_stream.str().c_str());
               TH2F *h_Numerator = (TH2F*) f1->Get(Numerator_name_stream.str().c_str());

               // Take 1d projection
               Denominator_y[i_topology][i_detector][i_charge][i_sector] = (TH1F*) h_Denominator->ProjectionX(); // denominator
               Numerator_y[i_topology][i_detector][i_charge][i_sector] = (TH1F*) h_Numerator->ProjectionX(); // numerator

               // Determine errors
               Denominator_y[i_topology][i_detector][i_charge][i_sector]->Sumw2();
               Numerator_y[i_topology][i_detector][i_charge][i_sector]->Sumw2();


               //////////////////////////////////////////////////////////////////////
               // Setting presentation on canvas   //////////////////////////////////
               //////////////////////////////////////////////////////////////////////

               // // Setting titles
               Numerator_y[i_topology][i_detector][i_charge][i_sector]->SetTitle(Efficiency_title_stream.str().c_str());

               // Aligning Title
               Numerator_y[i_topology][i_detector][i_charge][i_sector]->GetXaxis()->SetTitleSize(0.045);
               Numerator_y[i_topology][i_detector][i_charge][i_sector]->GetYaxis()->SetTitleSize(0.045);
               Numerator_y[i_topology][i_detector][i_charge][i_sector]->GetYaxis()->SetTitleOffset(1.1);




               // Setting ranges
               // FTOF 1
               if(i_detector<2)  Numerator_y[i_topology][i_detector][i_charge][i_sector]->GetXaxis()->SetRangeUser(0,500);

               // FTOF 2
               else  Numerator_y[i_topology][i_detector][i_charge][i_sector]->GetXaxis()->SetRangeUser(600,900);

               // Setting colour
               Numerator_y[i_topology][i_detector][i_charge][i_sector]->SetLineColor(kBlue);

               // Make copy for usage in the combined method plotting
               Efficiency[i_topology][i_detector][i_charge][i_sector] = (TH1F*)Numerator_y[i_topology][i_detector][i_charge][i_sector]->Clone(Efficiency_name_stream.str().c_str());


               // Dividing the numerator histogram by the denominator histogram
               Efficiency[i_topology][i_detector][i_charge][i_sector]->Divide(Denominator_y[i_topology][i_detector][i_charge][i_sector]);

               // Setting minimum and maximum
               Efficiency[i_topology][i_detector][i_charge][i_sector]->SetMinimum(Min);
               Efficiency[i_topology][i_detector][i_charge][i_sector]->SetMaximum(Max);

               //////////////////////////////////////////////////////////////////////
               // Renaming the bin labels to match the counters  ///////////
               //////////////////////////////////////////////////////////////////////

               // Loop over bins in histogram
               for(Int_t l = 2; l < Numerator_y[i_topology][i_detector][i_charge][i_sector]->GetNbinsX(); l++){

                  // Changing the bin labels to match the counter numbers
                  ostringstream test;
                  test<< l-1;

                  // Single track and 2pi
                  Numerator_y[i_topology][i_detector][i_charge][i_sector]->GetXaxis()->SetBinLabel(l,test.str().c_str());
                  Denominator_y[i_topology][i_detector][i_charge][i_sector]->GetXaxis()->SetBinLabel(l,test.str().c_str());
                  Efficiency[i_topology][i_detector][i_charge][i_sector]->GetXaxis()->SetBinLabel(l,test.str().c_str());
               }

               //////////////////////////////////////////////////////////////////////
               // Plotting efficieny against counter for each sector on canvas  ///////////
               //////////////////////////////////////////////////////////////////////

               // Selecting correct canvas and drawing histogram
               canvas_y[i_topology][i_detector][i_charge]->cd(i_sector+1);

               // Single track and 2pi
               Efficiency[i_topology][i_detector][i_charge][i_sector]->Draw();

               // Remove the 2D histograms
               delete h_Numerator;
               delete h_Denominator;
            }
            // Saving the canvases
            canvas_y[i_topology][i_detector][i_charge]->SaveAs(canvas_y_file_name_stream.str().c_str());

            // Clearing all ostringstreams
            //FileName.clear();
            Topology.clear();
            Layer.clear();
            Charge.clear();

         }
      }
   }

   //////////////////////////////////////////////////////////////////////
   // Making final result with single track and 2pi method combined  ///////////
   //////////////////////////////////////////////////////////////////////


   // Looping over the FTOF layers
   for(Int_t i_detector = 0; i_detector < 3; i_detector++){

      // Name the FTOF layer
      if(i_detector == 0) Layer.str("1A");
      else if(i_detector == 1) Layer.str("1B");
      else if(i_detector == 2) Layer.str("2");


      // Looping over charge
      for(Int_t i_charge = 0; i_charge < 2; i_charge++){

         // Name the charge
         if(i_charge == 0) Charge.str("negatives");
         else if(i_charge == 1) Charge.str("positives");

         // Creating strings for canvas names and file names
         ostringstream canvas_y_name_stream;
         ostringstream canvas_y_file_name_stream;


         // Setting the name for each canvas
         canvas_y_name_stream << "Overall_Result_" << Layer.str().c_str() << "_" << Charge.str().c_str();

         // Setting the file name for each canvas
         canvas_y_file_name_stream << Canvas_Out << "/Plots/Overall_Result_" << Layer.str().c_str() << "_" << Charge.str().c_str() << "_" << FileName << ".pdf";


         // creating canvas for each topology, FTOF layer, and charge
         canvas_y[4][i_detector][i_charge] = new TCanvas(canvas_y_name_stream.str().c_str(),"", 2500,1600);
         canvas_y[4][i_detector][i_charge]->Divide(3,2);


         // Looping over the sectors
         for(Int_t i_sector = 0; i_sector < 6; i_sector++){

            // Creating strings for names and titles of histograms for overall efficieny
            ostringstream histogram_name_num, histogram_title_num;
            ostringstream histogram_name_den, histogram_title_den;
            ostringstream histogram_name_eff;

            // Setting the names and titles of histograms for overall efficieny
            histogram_name_num<<"overall_hist_num_"<<i_detector<<"_"<<i_charge<<"_"<<i_sector;
            histogram_title_num<<"overall_hist_num_"<<i_detector<<"_"<<i_charge<<"_"<<i_sector;
            histogram_name_den<<"overall_hist_den_"<<i_detector<<"_"<<i_charge<<"_"<<i_sector;
            histogram_title_den<<"overall_hist_den_"<<i_detector<<"_"<<i_charge<<"_"<<i_sector;
            histogram_name_eff<<"overall_hist_eff_"<<i_detector<<"_"<<i_charge<<"_"<<i_sector;

            ostringstream Efficiency_title_stream;


            Efficiency_title_stream<<" Efficiency of " << Charge.str().c_str() << " FTOF"<<Layer.str().c_str()<<" Sector "<<i_sector + 1<<"; Counter; #epsilon";


            // For FTOF1A and FTOF1B get single track result where calorimeter
            // covers and 2 pi values where it doesn't
            if(i_detector == 0 || i_detector == 1){

               // Change bin content for those outside of calorimeter acceptance
               // For FTOF 1A change last counter to 2 pi method
               if(i_detector == 0){
                  // Define the overall histograms
                  Overall_Numerator_y[i_detector][i_charge][i_sector] = new TH1F(histogram_name_num.str().c_str(),histogram_title_num.str().c_str(),25, 61.98, 441.48);
                  Overall_Denominator_y[i_detector][i_charge][i_sector] = new TH1F(histogram_name_den.str().c_str(),histogram_title_den.str().c_str(),25, 61.98, 441.48);


                  for(Int_t bin_0 = 2; bin_0 < 25; bin_0++){


                     // Changing the bin labels to match the counter numbers
                     ostringstream test;
                     test<< bin_0-1;
                     Overall_Numerator_y[i_detector][i_charge][i_sector]->GetXaxis()->SetBinLabel(bin_0,test.str().c_str());
                     Overall_Denominator_y[i_detector][i_charge][i_sector]->GetXaxis()->SetBinLabel(bin_0,test.str().c_str());


                     // These bins are covered by the calorimeter, so single track
                     // is used
                     if(bin_0 < 24){
                        // Getting denominator values from single track
                        Overall_Denominator_y[i_detector][i_charge][i_sector]->SetBinContent
                        (bin_0,Denominator_y[3][i_detector][i_charge][i_sector]->GetBinContent(bin_0));

                        // Getting numerator values from single track
                        Overall_Numerator_y[i_detector][i_charge][i_sector]->SetBinContent
                        (bin_0,Numerator_y[3][i_detector][i_charge][i_sector]->GetBinContent(bin_0));
                     }
                     // These bins are not covered by the calorimeter, so 2pi
                     // is used
                     else{
                        // Getting denominator values from 2 pi
                        if(i_charge==0){
                           // Checking charge to determine which 2pi topology to use
                           Overall_Denominator_y[i_detector][i_charge][i_sector]->SetBinContent
                           (bin_0,Denominator_y[0][i_detector][i_charge][i_sector]->GetBinContent(bin_0));

                           // Getting numerator values from 2pi
                           Overall_Numerator_y[i_detector][i_charge][i_sector]->SetBinContent
                           (bin_0,Numerator_y[0][i_detector][i_charge][i_sector]->GetBinContent(bin_0));
                        }
                        else{
                           // Checking charge to determine which 2pi topology to use
                           Overall_Denominator_y[i_detector][i_charge][i_sector]->SetBinContent
                           (bin_0,Denominator_y[1][i_detector][i_charge][i_sector]->GetBinContent(bin_0));

                           // Getting numerator values from 2pi
                           Overall_Numerator_y[i_detector][i_charge][i_sector]->SetBinContent
                           (bin_0,Numerator_y[1][i_detector][i_charge][i_sector]->GetBinContent(bin_0));
                        }
                     }
                  }
               }

               // For FTOF 1B change last 5 counters to 2 pi method
               if(i_detector == 1){
                  // Define the overall histograms
                  Overall_Numerator_y[i_detector][i_charge][i_sector] = new TH1F(histogram_name_num.str().c_str(),histogram_title_num.str().c_str(),65, 46.04, 443.84);
                  Overall_Denominator_y[i_detector][i_charge][i_sector] = new TH1F(histogram_name_den.str().c_str(),histogram_title_den.str().c_str(),65, 46.04, 443.84);

                  for(Int_t bin_1 = 2; bin_1 < 64; bin_1++){

                     // Changing the bin labels to match the counter numbers
                     ostringstream test;
                     test << bin_1-1;
                     Overall_Numerator_y[i_detector][i_charge][i_sector]->GetXaxis()->SetBinLabel(bin_1,test.str().c_str());
                     Overall_Denominator_y[i_detector][i_charge][i_sector]->GetXaxis()->SetBinLabel(bin_1,test.str().c_str());



                     // These bins are covered by the calorimeter, so single track
                     // is used
                     if(bin_1 < 59){
                        // Getting denominator values from single track
                        Overall_Denominator_y[i_detector][i_charge][i_sector]->SetBinContent
                        (bin_1,Denominator_y[3][i_detector][i_charge][i_sector]->GetBinContent(bin_1));

                        // Getting numerator values from single track
                        Overall_Numerator_y[i_detector][i_charge][i_sector]->SetBinContent
                        (bin_1,Numerator_y[3][i_detector][i_charge][i_sector]->GetBinContent(bin_1));
                     }

                     else{
                        // Getting denominator values from 2 pi
                        if(i_charge==0){
                           // Checking charge to determine which 2pi topology to use
                           Overall_Denominator_y[i_detector][i_charge][i_sector]->SetBinContent
                           (bin_1,Denominator_y[0][i_detector][i_charge][i_sector]->GetBinContent(bin_1));

                           // Getting numerator values from 2pi
                           Overall_Numerator_y[i_detector][i_charge][i_sector]->SetBinContent
                           (bin_1,Numerator_y[0][i_detector][i_charge][i_sector]->GetBinContent(bin_1));
                        }
                        else{
                           // Checking charge to determine which 2pi topology to use
                           Overall_Denominator_y[i_detector][i_charge][i_sector]->SetBinContent
                           (bin_1,Denominator_y[1][i_detector][i_charge][i_sector]->GetBinContent(bin_1));

                           // Getting numerator values from 2pi
                           Overall_Numerator_y[i_detector][i_charge][i_sector]->SetBinContent
                           (bin_1,Numerator_y[1][i_detector][i_charge][i_sector]->GetBinContent(bin_1));
                        }
                     }

                  }
               }
            }

            // FTOF 2
            if(i_detector == 2){
               // Define overall histograms
               Overall_Numerator_y[i_detector][i_charge][i_sector] = new TH1F(histogram_name_num.str().c_str(),histogram_title_num.str().c_str(),7, 693, 847);
               Overall_Denominator_y[i_detector][i_charge][i_sector] = new TH1F(histogram_name_den.str().c_str(),histogram_title_den.str().c_str(),7, 693, 847);

               // Loop over counters
               for(Int_t bin_2 = 2; bin_2 < 7; bin_2++){

                  // Changing the bin labels to match the counter numbers
                  ostringstream test;
                  test<< bin_2-1;
                  Overall_Numerator_y[i_detector][i_charge][i_sector]->GetXaxis()->SetBinLabel(bin_2,test.str().c_str());
                  Overall_Denominator_y[i_detector][i_charge][i_sector]->GetXaxis()->SetBinLabel(bin_2,test.str().c_str());

                  // For negative efficieny get the pi- results
                  if(i_charge == 0){
                     // Getting denominator from 2pi
                     Overall_Denominator_y[i_detector][i_charge][i_sector]->SetBinContent
                     (bin_2,Denominator_y[0][i_detector][i_charge][i_sector]->GetBinContent(bin_2));

                     // Getting numerator from 2pi
                     Overall_Numerator_y[i_detector][i_charge][i_sector]->SetBinContent
                     (bin_2,Numerator_y[0][i_detector][i_charge][i_sector]->GetBinContent(bin_2));
                  }

                  // For positive efficieny get the pi+ results
                  else{
                     // Getting denominator from 2pi
                     Overall_Denominator_y[i_detector][i_charge][i_sector]->SetBinContent
                     (bin_2,Denominator_y[1][i_detector][i_charge][i_sector]->GetBinContent(bin_2));

                     // Getting numerator from 2pi
                     Overall_Numerator_y[i_detector][i_charge][i_sector]->SetBinContent
                     (bin_2,Numerator_y[1][i_detector][i_charge][i_sector]->GetBinContent(bin_2));
                  }
               }
            }

            // Determine errors for overall numerator
            Overall_Numerator_y[i_detector][i_charge][i_sector]->Sumw2();
            Overall_Denominator_y[i_detector][i_charge][i_sector]->Sumw2();


            // Aligning Title
            Overall_Numerator_y[i_detector][i_charge][i_sector]->GetXaxis()->SetTitleSize(0.045);
            Overall_Numerator_y[i_detector][i_charge][i_sector]->GetYaxis()->SetTitleSize(0.045);
            Overall_Numerator_y[i_detector][i_charge][i_sector]->GetYaxis()->SetTitleOffset(1.1);




            // Setting ranges
            // FTOF 1
            if(i_detector<2)  Overall_Numerator_y[i_detector][i_charge][i_sector]->GetXaxis()->SetRangeUser(0,500);

            // FTOF 2
            else  Overall_Numerator_y[i_detector][i_charge][i_sector]->GetXaxis()->SetRangeUser(600,900);

            // Setting colour
            Overall_Numerator_y[i_detector][i_charge][i_sector]->SetLineColor(kBlue);

            Overall_Efficiency[i_detector][i_charge][i_sector] = (TH1F*)Overall_Numerator_y[i_detector][i_charge][i_sector]->Clone(histogram_name_eff.str().c_str());
            Overall_Efficiency[i_detector][i_charge][i_sector]->SetTitle(Efficiency_title_stream.str().c_str());

            // Get ratio between numerator and denominator for efficieny
            Overall_Efficiency[i_detector][i_charge][i_sector]->Divide(Overall_Denominator_y[i_detector][i_charge][i_sector]);

            // Setting minimum and maximum
            Overall_Efficiency[i_detector][i_charge][i_sector]->SetMinimum(Min);
            Overall_Efficiency[i_detector][i_charge][i_sector]->SetMaximum(Max);

            //////////////////////////////////////////////////////////////////////
            // Plotting efficieny against counter for each sector on canvas  ///////////
            //////////////////////////////////////////////////////////////////////

            // Selecting correct canvas and drawing histogram
            canvas_y[4][i_detector][i_charge]->cd(i_sector+1);

            // Single track and 2pi
            Overall_Efficiency[i_detector][i_charge][i_sector]->Draw();

         }
         // Saving the canvases
         canvas_y[4][i_detector][i_charge]->SaveAs(canvas_y_file_name_stream.str().c_str());

         // Clearing all ostringstreams
         //FileName.clear();
         Topology.clear();
         Layer.clear();
         Charge.clear();

      }
   }

   fout.Write();
}
