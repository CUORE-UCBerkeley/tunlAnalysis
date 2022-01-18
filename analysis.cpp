#include <iostream>
#include <vector>

#include "TChain.h"
#include "TCanvas.h"
#include "TApplication.h"
#include "TH1F.h"
#include "TSpectrum.h"



int main(){

    //TApplication to diaplay output on screen
    TApplication app("app",0,0);

    //Store the tun numbers for Mo foil in a vector
    //std::vector<int> runMoFoil{58,59,60,61,62,63,64,65,66,67};
    std::vector<int> runMoFoil{58};
    
    //Add the files in a chain; tree name is SSA
    TChain chain("SSA");
    for(auto &run: runMoFoil){
        chain.Add(Form("../root/root_data_SSA_%03d.bin_tree.root",run));
    }

    std::cout << chain.GetEntries() << std::endl;

    //Create canvas for diagnostic plots 
    auto c1 = new TCanvas("c1","c1", 1200, 800);
    c1->Divide(1,4);
   
    //Take the raw amplitudes and put them in histograms
    std::vector<TH1F> hAmplitude;
    for(int i = 0; i < 4; i++){
        c1->cd(i+1);
        TH1F *h = new TH1F(Form("h%d",2*i),Form("h%d",2*i),10000,0,60000);
        chain.Draw(Form("amplitude[%d]>>h%d",(2*i),(2*i)),"","goff");
        hAmplitude.push_back(*h);
    }

    //Find peaks in the histogram for calibration
    // Use TSpectrum to find the peak candidates
    int npeaks = 20;
    TSpectrum *s = new TSpectrum(2*npeaks);
    int nfound = s->Search(&hAmplitude[0],2,"",0.005);
    printf("Found %d candidate peaks to fit\n",nfound);
    // Estimate background using TSpectrum::Background
    TH1 *hb = s->Background(&hAmplitude[0],20,"same");
    if (hb) c1->Update();
    

    c1->SaveAs("diagnosticPlots.pdf");
        
    app.Run();
}
