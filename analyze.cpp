#include <iostream>
#include <vector>
#include <getopt.h>
#include <stdio.h>

#include "TTree.h"
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
  std::cout << "Usage: ./calibrate --material c --energy i --runs run1,run2,run3 --fCalib C\n" ; 
}

int main(int argc, char **argv){    

    std::cout << "Entering the analyze module ... " << std::endl;
    
    // This activates implicit multi-threading
    ROOT::EnableImplicitMT();

    std::string matname;
    int energy;
    std::string runs;
    std::string fCalib;

    static struct option long_options[] = {
        {"material"     ,   required_argument, 0, 'c'  },
        {"energy"       ,   required_argument, 0, 'p'  },
        {"runs"         ,   required_argument, 0, 'r'  },
        {"fCalib"       ,   required_argument, 0, 'C'  },
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
        case 'r' :
            runs = std::string(optarg);
            break;
        case 'C' :
            fCalib = std::string(optarg);
        break;
        default: print_usage();
                exit(EXIT_FAILURE);
        }
    }

    //create a nameID for storing files
    std::string nameID = Form("_%s_%dMeV",matname.data(),energy);
    std::string calibID;
    if(!fCalib.empty()){
        calibID = fCalib;
        std::cout << "Using calibration from " << calibID << ".root" << std::endl;
    } else {
        calibID = Form("calibration_%s_%dMeV",matname.data(),energy);
        std::cout << "Using calibration from " << calibID << ".root" << std::endl;
    }

    //Create a vector of run numbers 
    std::vector<int> runnumbers;
    
    //Parse the list of runs to runnumbers
    char delimiter = ',';
    size_t pos = 0;
    std::stringstream sstream(runs);
    std::string temp;
    while (std::getline(sstream, temp, delimiter)){
        runnumbers.push_back(std::stoi(temp));
    }

    if(runnumbers.size()==0){
        std::cout << "Please pass correct arguments" << std::endl ;
        exit(EXIT_FAILURE);
    }
   
    // Get the calibration from calibration root file 
    std::string calibFilename = Form("../processedFiles/%s.root", calibID.data());
    std::unique_ptr<TFile> calibFile( TFile::Open(calibFilename.data()) );
    std::vector<TF1*> calib(4);
    if(calibFile){
        for(int i =0; i < 4 ; i++){
            calib[i] = (TF1*)calibFile->Get(Form("calibFit_ch%d",2*i));
        }
    }
    calibFile->Close();

    gDirectory->cd();
    //Create the root file for writing the reduced data
    std::unique_ptr<TFile> reducedFile( TFile::Open(Form("../processedFiles/reduced%s.root",nameID.data()), "RECREATE") );
    auto outTree = std::make_unique<TTree>("ssa", "ssa");
    
    std::vector<double> energy_ch(4);
    std::vector<double> tof_ch(4);
    int runNum;

    outTree->Branch("energy_ch0", &energy_ch[0]);
    outTree->Branch("energy_ch2", &energy_ch[1]);
    outTree->Branch("energy_ch4", &energy_ch[2]);
    outTree->Branch("energy_ch6", &energy_ch[3]);
    outTree->Branch("tof_ch0", &tof_ch[0]);
    outTree->Branch("tof_ch2", &tof_ch[1]);
    outTree->Branch("tof_ch4", &tof_ch[2]);
    outTree->Branch("tof_ch6", &tof_ch[3]);
    outTree->Branch("run", &runNum);

    // We will use TTree draw to extract the useful variables
    // Create a list of variables for the tree
    std::string var = "amplitude[0]:amplitude[2]:amplitude[4]:amplitude[6]:"
             "channel_time[0]:channel_time[2]:channel_time[4]:channel_time[6]:"
             "trigger_time[0]:"
             "amplitude[14]";

    //loop over all runnumber files and get the required quantities 
    for(auto run: runnumbers){

        std::unique_ptr<TFile> inFile( TFile::Open( Form("../root/root_data_SSA_%03d.bin_tree.root",run)) );
        auto inTree = inFile->Get<TTree>("SSA");
        runNum = run;
        inTree->SetEstimate(-1);  ///Extremly important when the number of events are more than 1000000 and using TTree::Draw
        inTree->Draw(var.data(),"","goff");
        size_t nEntries = inTree->GetSelectedRows();
        std::cout << "Run Number = " << run << "\t" << " Number of selected rows: " << nEntries << std::endl;
        std::vector<double> amp_ch0(inTree->GetVal(0), inTree->GetVal(0) + nEntries);
        std::vector<double> amp_ch2(inTree->GetVal(1), inTree->GetVal(1) + nEntries);
        std::vector<double> amp_ch4(inTree->GetVal(2), inTree->GetVal(2) + nEntries);
        std::vector<double> amp_ch6(inTree->GetVal(3), inTree->GetVal(3) + nEntries);
        std::vector<double> time_ch0(inTree->GetVal(4), inTree->GetVal(4) + nEntries);
        std::vector<double> time_ch2(inTree->GetVal(5), inTree->GetVal(5) + nEntries);
        std::vector<double> time_ch4(inTree->GetVal(6), inTree->GetVal(6) + nEntries);
        std::vector<double> time_ch6(inTree->GetVal(7), inTree->GetVal(7) + nEntries);
        std::vector<double> time_trg(inTree->GetVal(8), inTree->GetVal(8) + nEntries);
        std::vector<double> bci(inTree->GetVal(9), inTree->GetVal(9) + nEntries);

         for(int64_t i = 0; i < nEntries; i++){
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

            outTree->Fill();

        }

    }
    reducedFile->cd();
    outTree->Write();

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
        outTree->Draw(Form("energy_ch%d>>hEnergy_ch%d",kk,kk), "","goff");
        hE[i]->Write();

        //Create and save energies of TOF of each detector
        htof[i]= new TH1D(Form("htof_ch%d",kk),Form("htof_ch%d",kk), 1000, -4096, 0);
        outTree->Draw(Form("tof_ch%d>>htof_ch%d",kk,kk), "","goff");
        htof[i]->Write();

        //Get the peak position of TOF to make tof cuts
        int binmax = htof[i]->GetMaximumBin(); 
        double xbinmax = htof[i]->GetXaxis()->GetBinCenter(binmax);

        //Define Prompt such that (xbinmax - 500) < TOF < (xbinmax + 500)
        hE_Prompt[i]= new TH1D(Form("hE_Prompt_ch%d",kk),Form("hE_Prompt_ch%d",kk), 10000, 0, 10000);
        outTree->Draw(Form("energy_ch%d>>hE_Prompt_ch%d",kk,kk), \
                    Form("tof_ch%d < abs(%f - 500)", kk, xbinmax), \
                    "goff");
        double promptRange = 1000;
        hE_Prompt[i]->Write();

        //Define Early 
        hE_Early[i]= new TH1D(Form("hE_Early_ch%d",kk),Form("hE_Early_ch%d",kk), 10000, 0, 10000);
        outTree->Draw(Form("energy_ch%d>>hE_Early_ch%d",kk,kk), \
                    Form("tof_ch%d > -4000 && tof_ch%d < (%f - 500)", kk, kk, xbinmax), \
                    "goff");
        double earlyRange = abs((-4000) - (xbinmax - 500));
        hE_Early[i]->Write();

        //Define Late 
        hE_Late[i]= new TH1D(Form("hE_Late_ch%d",kk),Form("hE_Late_ch%d",kk), 10000, 0, 10000);
        outTree->Draw(Form("energy_ch%d>>hE_Late_ch%d",kk,kk), \
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
    std::cout << ".... exiting the analyze module." << std::endl;
        
}
