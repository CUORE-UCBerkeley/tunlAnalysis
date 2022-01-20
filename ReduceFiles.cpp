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

    std::cout << energy << std::endl;
    //Create a vector of run numbers 
    std::vector<int> runnumbers;
    std::string nameID;
    
    if(matname.compare("MoFoil") ==0 && energy == 6){
        runnumbers = {58, 59, 60, 61, 62, 63, 64, 65, 66};
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

    runnumbers = {58};
    if(runnumbers.size()==0){
        std::cout << "Please pass correct arguments with material and neutron energy." << std::endl ;
        exit(EXIT_FAILURE);
    }

    //TApplication to diaplay output on screen
    TApplication app("app",0,0);

    
    //Add the files in a chain; tree name is SSA
    TChain chain("SSA");
    for(auto &run: runnumbers){
        chain.Add(Form("../root/root_data_SSA_%03d.bin_tree.root",run));
    }

    std::cout << chain.GetEntries() << std::endl;

    /////// Try something new ////
    ROOT::EnableImplicitMT();
    ROOT::RDataFrame df(chain);
    auto h = df.Define("energy0", [](const RvecD &amplitude) { return amplitude[2]; }, {"amplitude"}).Histo1D("energy0");
    h->Draw();

    auto cutTOF = [](const RvecD ChannelTimes, const RvecD TriggerTimes){
        return (ChannelTimes[0] - TriggerTimes[0]);
    };
    auto hTOF = df.Define("TOF_ch0", cutTOF, {"channel_time","trigger_time"})
                  . Histo1D({"name", "title", 1000, -4000, 0},"TOF_ch0");
    hTOF->Draw();

    //////////////////////////////

    // //Create canvas for diagnostic plots 
    // auto c1 = new TCanvas("c1","c1", 1200, 800);
    // c1->Divide(1,4);
   
    // //Take the raw amplitudes and put them in histograms
    // std::vector<TH1F> hAmplitude;
    // for(int i = 0; i < 4; i++){
    //     c1->cd(i+1);
    //     TH1F *h = new TH1F(Form("h%d",2*i),Form("h%d",2*i),10000,0,60000);
    //     chain.Draw(Form("amplitude[0]:amplitude[2]:amplitude[4]:amplitude[6] \
    //                      >>h%d",(2*i),(2*i)),"","goff");
    //     hAmplitude.push_back(*h);
    // }

    // //Find peaks in the histogram for calibration
    // // Use TSpectrum to find the peak candidates
    // int npeaks = 20;
    // TSpectrum *s = new TSpectrum(2*npeaks);
    // int nfound = s->Search(&hAmplitude[0],2,"",0.005);
    // printf("Found %d candidate peaks to fit\n",nfound);
    // // Estimate background using TSpectrum::Background
    // TH1 *hb = s->Background(&hAmplitude[0],20,"same");
    // if (hb) c1->Update();
    

    // c1->SaveAs("diagnosticPlots.pdf");
        
    app.Run();
}
