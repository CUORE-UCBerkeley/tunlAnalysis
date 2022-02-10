#include <iostream>
#include <vector>
#include <algorithm>
#include <tuple>
#include <getopt.h>
#include <stdio.h>
#include <sstream>
#include <string>


#include "TChain.h"
#include "TCanvas.h"
#include "TApplication.h"
#include "TH1F.h"
#include "TSpectrum.h"
#include <ROOT/RDataFrame.hxx>
#include "TLine.h"
#include "TF1.h"
#include "TGraphErrors.h"
#include "TStyle.h"


std::tuple<double, double>  GetMaxInRange(TH1D *hist, double xlow, double xhigh){

    hist->GetXaxis()->SetRangeUser(xlow, xhigh);
    int binmax = hist->GetMaximumBin(); 
    double xmax = hist->GetXaxis()->GetBinCenter(binmax);

    //Fit the peak with a gaussian to get an error estimate on mean
    hist->GetXaxis()->SetRangeUser(xmax - 50, xmax + 50);
    hist->Fit("gaus");
    auto gfit = (TF1*)hist->GetListOfFunctions()->FindObject("gaus");
    double mean = gfit->GetParameter(1);
    double meanerr = gfit->GetParError(1);
    return std::make_tuple(mean, meanerr);

}

std::tuple<TGraphErrors*, TF1*> Calibrate( TH1D *hist, int channel, int energy){
    
    //Define containers for energy and amplitude position
    std::vector<double> energies{510.998, 1460.820, 2614.511};
    std::vector<double> xpos(3);
    std::vector<double> xposErr(3);
    
    //Find peaks in the histogram for calibratio
    //Unpack the tuple elements returned by the function 
    if(energy == 6 ){
        std::tie(xpos[0], xposErr[0]) = GetMaxInRange(hist, 4000, 7000);   //Get 511 position
        std::tie(xpos[1], xposErr[1]) = GetMaxInRange(hist, 13000, 16000); //Get 1460 position
        std::tie(xpos[2], xposErr[2]) = GetMaxInRange(hist, 25000, 28000); //Get 2615 position
    } else { // Other energies were taking with a different ADC range settings
        std::tie(xpos[0], xposErr[0]) = GetMaxInRange(hist, 2000, 4000);   //Get 511 position
        std::tie(xpos[1], xposErr[1]) = GetMaxInRange(hist, 8000, 10000); //Get 1460 position
        std::tie(xpos[2], xposErr[2]) = GetMaxInRange(hist, 15000, 17000); //Get 2615 position
    }
    

    auto calibPlot = new TGraphErrors(xpos.size(), &xpos[0], &energies[0], &xposErr[0] , 0);
    calibPlot->Draw();
    calibPlot->SetTitle(Form("Calibration of Channel %d", channel));
    calibPlot->SetName(Form("CalibChannel%d", channel));
    calibPlot->GetXaxis()->SetTitle("Amplitude");
    calibPlot->GetYaxis()->SetTitle("Energy (keV)");
    calibPlot->SetMarkerColor(4);
    calibPlot->SetMarkerStyle(21);
    calibPlot->Draw("AP");

    gStyle->SetOptFit(1);
    std::string fitname(Form("calibfunc_ch%d",channel));
    TF1 *polFit = new TF1 (fitname.data() ,"[0] + [1] * x", xpos[0] , xpos[3]);
    polFit->SetParameter(0,0);
    polFit->SetParameter(1,0.01);
    calibPlot->Fit(fitname.data());

    return std::make_tuple(calibPlot, polFit);
}

void print_usage() {
  std::cout << "Usage: ./calibrate --material c --energy i -r run1,run2,run3\n" ;
}

int main(int argc, char **argv){    
    
    std::cout << "Entering the calibration script ... " << std::endl;
    // This activates implicit multi-threading
    ROOT::EnableImplicitMT();

    std::string matname;
    int energy;
    std::string runs;

    static struct option long_options[] = {
        {"material",   required_argument, 0, 'c'  },
        {"energy"  ,   required_argument, 0, 'p'  },
        {"runs"    ,   required_argument, 0, 'r'  },
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
        default: print_usage();
                exit(EXIT_FAILURE);
        }
    }

    //create a nameID for storing files
    std::string nameID = Form("_%s_%dMeV",matname.data(),energy);

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

    
    //Add the files in a chain; tree name is SSA
    TChain *chain = new TChain("SSA");
    for(auto &run: runnumbers){
        chain->Add(Form("../root/root_data_SSA_%03d.bin_tree.root",run));
        std::cout << "Added run number " << run << std::endl;
    }

    std::cout << "The total number of entries in the root files are: " << chain->GetEntries() << std::endl;
   
    //Take the raw amplitudes and put them in histograms
    
    std::vector<TH1D *> hAmpRaw; //contains raw amplitude spectra
        
    for(int i = 0; i < 4; i++){

        int kk = 2 * i; //Takes care of the fact that detector numbers are 0, 2, 4, 6

        //Fill in the raw spectra; We will use it for calibration
        hAmpRaw.push_back(new TH1D(Form("hAmpRaw%d", kk),Form("hAmpRaw%d", kk),100000,0,100000));
        chain->Draw(Form("amplitude[%d]>>hAmpRaw%d",kk,kk),"", "goff");

    }

    //Create a root file for storing the calibration graph and the fit to it
    std::unique_ptr<TFile> calibFile( TFile::Open(Form("../processedFiles/calibration%s.root",nameID.data()), "RECREATE") );
    
    //Prepare containers to store the graphs and the fits
    std::vector<TGraphErrors *> gr(4);
    std::vector<TF1 *> fit(4);

    for(int i = 0; i < 4; i++ ){
        std::tie(gr[i],fit[i]) = Calibrate(hAmpRaw[i], 2*i, energy); //The second argument in Calibrate is detector id
        calibFile->WriteObject(gr[i],Form("calibGraph_ch%d",2*i)); //Write to root file
        calibFile->WriteObject(fit[i],Form("calibFit_ch%d",2*i)); //Write to root file
    }
    calibFile->Close();
    
    std::cout << ".... exiting the calibration script." << std::endl;
    //app.Run();
}
