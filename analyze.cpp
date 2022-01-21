#include <iostream>
#include <vector>
#include <getopt.h>
#include <stdio.h>

#include "TChain.h"
#include "TCanvas.h"
#include "TApplication.h"
#include "TH1F.h"
#include "TSpectrum.h"
#include <ROOT/RDataFrame.hxx>
#include <ROOT/RVec.hxx>
#include "TF1.h"

using namespace ROOT::VecOps;
using RvecD = ROOT::RVec<double>;

void print_usage() {
  std::cout << "Usage: ./Reduce --material c --energy i" << "\n" 
            << "Choose from the following : " << "\n"  
            << "material: {MoFoil,MoPowder,Fe,Cu,Empty}" << "\n" 
            << "energy: {0,4,6,8}" <<  "\n" 
            << "Note: Not all combinations are possible. Refer to spreadsheet." << "\n";
}

int main(int argc, char **argv){    
    
    // This activates implicit multi-threading
    ROOT::EnableImplicitMT();

    std::string matname;
    double energy;
    double energies[4]={0.,4.3,5.9,8};
    const char* pnames[5] = {"MoFoil","MoPowder","Fe","Cu","Blank"};

    static struct option long_options[] = {
        {"material",   required_argument, 0, 'c'  },
        {"energy"  ,   required_argument, 0, 'p'  },
    };

    int long_index =0;
    int opt;

    while ((opt = getopt_long_only(argc, argv,"", long_options, &long_index )) != -1) {
        switch (opt) {
        case 'c' :
            matname = std::string(optarg);
            break;
        case 'p' :
            energy = atoi(optarg);
            break;
        default: print_usage();
                exit(EXIT_FAILURE);
        }
    }

    //Create a vector of run numbers 
    std::vector<int> runnumbers;
    std::string nameID;
    
    if(matname.compare("MoFoil") ==0 && energy == 6){
        runnumbers = {58};
        //runnumbers = {58, 59, 60, 61, 62, 63, 64, 65, 66};
        nameID = "_MoFoil_6MeV";
    } else if(matname.compare("FeFoil") ==0 && energy == 6){
        runnumbers = {67, 77, 78};
        nameID = "_FeFoil_6MeV";
    } else if(matname.compare("MoPowder") ==0 && energy == 0){
        runnumbers = {68, 72, 76, 85};
        nameID = "_MoFoil_NoBeam";
    } else if(matname.compare("MoPowder") ==0 && energy == 6){
        runnumbers = {69, 70, 71, 73, 74, 75};
        nameID = "_MoPowder_6MeV";
    } else if(matname.compare("Empty") ==0 && energy == 6){
        runnumbers = {79};
        nameID = "_Empty_6MeV";
    } else if(matname.compare("Cu") ==0 && energy == 6){
        runnumbers = {80};
        nameID = "_Cu_6MeV";
    } else if(matname.compare("MoPowder") ==0 && energy == 8){
        runnumbers = {81, 82, 83, 84, 86};
        nameID = "_MoPowder_8MeV";
    } else if(matname.compare("FeFoil") ==0 && energy == 8){
        runnumbers = {87};
        nameID = "_FeFoil_8MeV";
    } else if(matname.compare("FeFoil") ==0 && energy == 4){
        runnumbers = {88};
        nameID = "_FeFoil_4MeV";
    } else if(matname.compare("MoPowder") ==0 && energy == 4){
        runnumbers = {89,90};
        nameID = "_MoPowder_4MeV";
    } else if(matname.compare("Empty") ==0 && energy == 4){
        runnumbers = {91};
        nameID = "_Empty_4MeV";
    }

    if(runnumbers.size()==0){
        std::cout << "Please pass correct arguments with material and neutron energy." << std::endl ;
        exit(EXIT_FAILURE);
    }
   
    //Add the files in a chain; tree name is SSA
    TChain *chain = new TChain("SSA");
    for(auto &run: runnumbers){
        chain->Add(Form("../root/root_data_SSA_%03d.bin_tree.root",run));
    }

    std::cout << "Number of entries in the chained files: " << chain->GetEntries() << std::endl;

    // Get the calibration from calibration root file 
    std::string calibFilename = Form("../processedFiles/calibration%s.root", nameID.data());
    std::unique_ptr<TFile> calibFile( TFile::Open(calibFilename.data()) );
    std::vector<TF1*> calib(4);
    if(calibFile){
        for(int i =0; i < 4 ; i++){
            calib[i] = (TF1*)calibFile->Get(Form("calibFit_ch%d",2*i));
            //calib[i]->Print();
        }
    }
    calibFile->Close();

    // Use TTree draw to extract the useful variables 
    std::string var = "amplitude[0]:amplitude[2]:amplitude[4]:amplitude[6]:"
             "channel_time[0]:channel_time[2]:channel_time[4]:channel_time[6]:"
             "trigger_time[0]:"
             "amplitude[14]";
    
    ///Extremly important when the number of events are more than 1000000 and using TTree::Draw
    chain->SetEstimate(-1);  
    chain->Draw(var.data(),"","goff");
    size_t nEntries = chain->GetSelectedRows();
    std::cout << "Number of selected rows after event selection: " << nEntries << std::endl;

    std::vector<double> amp_ch0(chain->GetVal(0), chain->GetVal(0) + nEntries);
    std::vector<double> amp_ch2(chain->GetVal(1), chain->GetVal(1) + nEntries);
    std::vector<double> amp_ch4(chain->GetVal(2), chain->GetVal(2) + nEntries);
    std::vector<double> amp_ch6(chain->GetVal(3), chain->GetVal(3) + nEntries);
    std::vector<double> time_ch0(chain->GetVal(4), chain->GetVal(4) + nEntries);
    std::vector<double> time_ch2(chain->GetVal(5), chain->GetVal(5) + nEntries);
    std::vector<double> time_ch4(chain->GetVal(6), chain->GetVal(6) + nEntries);
    std::vector<double> time_ch6(chain->GetVal(7), chain->GetVal(7) + nEntries);
    std::vector<double> time_trg(chain->GetVal(8), chain->GetVal(8) + nEntries);
    std::vector<double> bci(chain->GetVal(9), chain->GetVal(9) + nEntries);

    gDirectory->cd();
    std::unique_ptr<TFile> reducedFile( TFile::Open(Form("../processedFiles/reduced%s.root",nameID.data()), "RECREATE") );
    auto rtree = std::make_unique<TTree>("ssa", "ssa");
    
    std::vector<double> energy_ch(4);
    std::vector<double> tof_ch(4);

    rtree->Branch("energy_ch0", &energy_ch[0]);
    rtree->Branch("energy_ch2", &energy_ch[1]);
    rtree->Branch("energy_ch4", &energy_ch[2]);
    rtree->Branch("energy_ch6", &energy_ch[3]);
    rtree->Branch("tof_ch0", &tof_ch[0]);
    rtree->Branch("tof_ch2", &tof_ch[1]);
    rtree->Branch("tof_ch4", &tof_ch[2]);
    rtree->Branch("tof_ch6", &tof_ch[3]);
    
    for(size_t i = 0; i < nEntries; i++){
        
        energy_ch[0] = calib[0]->Eval(amp_ch0[i]);
        energy_ch[1] = calib[1]->Eval(amp_ch2[i]);
        energy_ch[2] = calib[2]->Eval(amp_ch4[i]);
        energy_ch[3] = calib[3]->Eval(amp_ch6[i]);
        tof_ch[0] = time_ch0[i] - time_trg[i];
        tof_ch[1] = time_ch2[i] - time_trg[i];
        tof_ch[2] = time_ch4[i] - time_trg[i];
        tof_ch[3] = time_ch6[i] - time_trg[i];

        //Bump up the satellite tof peaks to main peak between -4096 and 0
        //Due to some DAQ error the satellite peaks are shifted by multiples of 4096.
        // We correct that here.
        for(int jj = 0; jj < 5 ; jj++){
            for(int kk = 0; kk < 4; kk++){
                if( (-((jj+1) * 4096) < tof_ch[kk]) &&  (tof_ch[kk] <= -(jj*4096)) ){
                    tof_ch[kk] += (jj * 4096);
                }
            }
        }
        
        rtree->Fill();

    }
    rtree->Write();

    std::vector<TH1D*> hE(4);
    std::vector<TH1D*> htof(4);
    std::vector<TH1D*> hE_Prompt(4);
    std::vector<TH1D*> hE_Early(4);
    std::vector<TH1D*> hE_Late(4);
    std::vector<TH1D*> hE_tofCorr(4);

    //Lets make some histos 
    for(int i = 0; i < 4; i++){
        int kk = 2*i;
        //Create and save energies of each detector
        hE[i]= new TH1D(Form("hEnergy_ch%d",kk),Form("hEnergy_ch%d",kk), 10000, 0, 10000);
        rtree->Draw(Form("energy_ch%d>>hEnergy_ch%d",kk,kk), "","goff");
        hE[i]->Write();

        //Create and save energies of TOF of each detector
        htof[i]= new TH1D(Form("htof_ch%d",kk),Form("htof_ch%d",kk), 1000, -4096, 0);
        rtree->Draw(Form("tof_ch%d>>htof_ch%d",kk,kk), "","goff");
        htof[i]->Write();

        //Get the peak position of TOF to make tof cuts
        int binmax = htof[i]->GetMaximumBin(); 
        double xbinmax = htof[i]->GetXaxis()->GetBinCenter(binmax);

        //Define Prompt such that (xbinmax - 500) < TOF < (xbinmax + 500)
        hE_Prompt[i]= new TH1D(Form("hE_Prompt_ch%d",kk),Form("hE_Prompt_ch%d",kk), 10000, 0, 10000);
        rtree->Draw(Form("energy_ch%d>>hE_Prompt_ch%d",kk,kk), \
                    Form("tof_ch%d < abs(%f - 500)", kk, xbinmax), \
                    "goff");
        double promptRange = 1000;
        hE_Prompt[i]->Write();

        //Define Early 
        hE_Early[i]= new TH1D(Form("hE_Early_ch%d",kk),Form("hE_Early_ch%d",kk), 10000, 0, 10000);
        rtree->Draw(Form("energy_ch%d>>hE_Early_ch%d",kk,kk), \
                    Form("tof_ch%d > -4000 && tof_ch%d < (%f - 500)", kk, kk, xbinmax), \
                    "goff");
        double earlyRange = abs((-4000) - (xbinmax - 500));
        hE_Early[i]->Write();

        //Define Late 
        hE_Late[i]= new TH1D(Form("hE_Late_ch%d",kk),Form("hE_Late_ch%d",kk), 10000, 0, 10000);
        rtree->Draw(Form("energy_ch%d>>hE_Late_ch%d",kk,kk), \
                    Form("tof_ch%d > -4100 && tof_ch%d < (%f - 500)", kk, kk, xbinmax), \
                    "goff");
        double lateRange = abs(0 - (xbinmax + 500));
        hE_Late[i]->Write();
       
        //Estimate TOF corrected plot by subtracting accidental spectra from prompt spectra
        hE_tofCorr[i]= new TH1D(Form("hE_tofCorr_ch%d",kk),Form("hE_tofCorr_ch%d",kk), 10000, 0, 10000);
        hE_tofCorr[i]->Add(hE_Prompt[i], 1);
        hE_tofCorr[i]->Add(hE_Early[i], - 0.5 * promptRange / earlyRange );
        hE_tofCorr[i]->Add(hE_Late[i], - 0.5 * promptRange / lateRange );
        hE_tofCorr[i]->Write();

    }
    
    std::cout << "Closed the generated reduced file." << std::endl;
        
}
