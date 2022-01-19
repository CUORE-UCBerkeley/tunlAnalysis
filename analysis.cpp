#include <iostream>
#include <vector>

#include "TChain.h"
#include "TCanvas.h"
#include "TApplication.h"
#include "TH1F.h"
#include "TSpectrum.h"
#include <ROOT/RDataFrame.hxx>
#include "TLine.h"



int main(){

    //TApplication to diaplay output on screen
    TApplication app("app",0,0);

    //Store the tun numbers for Mo foil in a vector
    //std::vector<int> runMoFoil{58,59,60,61,62,63,64,65,66,67};
    std::vector<int> runMoFoil{58};
    
    //Add the files in a chain; tree name is SSA
    TChain *chain = new TChain("SSA");
    for(auto &run: runMoFoil){
        chain->Add(Form("../root/root_data_SSA_%03d.bin_tree.root",run));
    }

    std::cout << "The total number of entries in the root files are: " << chain->GetEntries() << std::endl;

    //Create canvas for diagnostic plots 
    auto c1 = new TCanvas("c1","c1", 1200, 800);
    c1->cd();
   
    //Take the raw amplitudes and put them in histograms
    
    std::vector<TH1D *> hToF; //contans Time of flight spectra
    std::vector<TH1D *> hAmpToFCorr; //contains raw amplitude spectra
    std::vector<TH1D *> hAmpToFPrompt; //contains time of flight corrected amplitude spectra
    std::vector<TH1D *> hAmpToFEarly; //contains accidental amplitude spectra on Early
    std::vector<TH1D *> hAmpToFLate; //contans accidental amplitude spectra on Late
    
    gPad->Print("diagnosticPlots.pdf(");
        
    for(int i = 0; i < 4; i++){

        int kk = 2 * i; //Takes care of the fact that detector numbers are 0, 2, 4, 6

        //Fill in the time of flight spectra & stick with main peak only for now
        hToF.push_back(new TH1D(Form("hToF%d",kk),Form("hToF%d",kk), 1000, -4000, 0));
        chain->Draw(Form("(channel_time[%d] - trigger_time[0])>>hToF%d",kk,kk), \
                    Form("(channel_time[%d] - trigger_time[0]) > -4000", kk),"goff"); 
        //Add lines for where the ToF cut will be made
        hToF[i]->Draw();
        double ymax = hToF[i]->GetMaximum();;
        int binmax = hToF[i]->GetMaximumBin(); 
        double xmax = hToF[i]->GetXaxis()->GetBinCenter(binmax);
        TLine *line1 = new TLine(xmax - 500, 0, xmax - 500,ymax);
        line1->SetLineColor(kRed);
        line1->Draw();
        TLine *line2 = new TLine(xmax + 500, 0, xmax + 500,ymax);
        line2->SetLineColor(kRed);
        line2->Draw();
        gPad->Print("diagnosticPlots.pdf");

        //Fill in the prompt spectra
        hAmpToFPrompt.push_back(new TH1D(Form("hAmpToFPrompt%d", kk),Form("hAmptToFPrompt%d", kk),10000,0,60000));
        chain->Draw(Form("amplitude[%d]>>hAmpToFPrompt%d",kk,kk),
                    Form("(channel_time[%d] - trigger_time[0]) > %f && (channel_time[%d] - trigger_time[0]) < %f ", kk, xmax - 500, kk, xmax + 500), \
                    "goff");
        double cutRangePrompt = abs( (xmax + 500) - (xmax - 500) );
        std::cout << "cutRangePrompt =  " << cutRangePrompt << "\n";
        hAmpToFPrompt[i]->Draw(); 
        gPad->Print("diagnosticPlots.pdf"); 

        // Get the early accidental spectra 
        hAmpToFEarly.push_back(new TH1D(Form("hAmpToFEarly%d", kk),Form("hAmptToFEarly%d", kk),10000,0,60000));
        chain->Draw(Form("amplitude[%d]>>hAmpToFEarly%d",kk,kk),
                    Form("(channel_time[%d] - trigger_time[0]) > -4000 && (channel_time[%d] - trigger_time[0]) < %f ", kk, kk, xmax - 500), \
                    "goff");
        double cutRangeEarly = abs(-4000 - (xmax -500));
        std::cout << "cutRangeEarly =  " << cutRangeEarly << "\n";
        hAmpToFEarly[i]->Draw(); 
        gPad->Print("diagnosticPlots.pdf");           

        // Get the early accidental spectra 
        hAmpToFLate.push_back(new TH1D(Form("hAmpToFLate%d", kk),Form("hAmpToFLate%d", kk),10000,0,60000));
        chain->Draw(Form("amplitude[%d]>>hAmpToFLate%d",kk,kk), \
                    Form("(channel_time[%d] - trigger_time[0]) < 0 && (channel_time[%d] - trigger_time[0]) > %f ", kk, kk, xmax + 500), \
                    "goff");
        double cutRangeLate = abs(0 - (xmax + 500));
        std::cout << "cutRangeLate =  " << cutRangeLate << "\n";
        hAmpToFLate[i]->Draw(); 
        gPad->Print("diagnosticPlots.pdf");

        // Fill in the ToF corrected amplitude spectra
        hAmpToFCorr.push_back(new TH1D(Form("hAmpToF%d",kk),Form("hAmpToF%d", kk),10000,0,60000));
        // The chosen width for ToF prompt spectra is 1000 (500 + 500), so we will normalize the early and late spectra to that
        hAmpToFCorr[i]->Add(hAmpToFPrompt[i], 1.);
        hAmpToFCorr[i]->Add(hAmpToFEarly[i], - 0.5*cutRangePrompt/cutRangeEarly);
        hAmpToFCorr[i]->Add(hAmpToFLate[i] , - 0.5*cutRangePrompt/cutRangeLate);
        hAmpToFCorr[i]->Draw(); 
        gPad->Print("diagnosticPlots.pdf");

    }

    c1->Clear();
    gPad->Print("diagnosticPlots.pdf)");

    // //Find peaks in the histogram for calibration
    // // Use TSpectrum to find the peak candidates
    // int npeaks = 30;
    // TSpectrum *s = new TSpectrum(2*npeaks);
    // int nfound = s->Search(&hAmplitude[0],2,"",0.005);
    // printf("Found %d candidate peaks to fit\n",nfound);
    // // Estimate background using TSpectrum::Background
    // TH1 *hb = s->Background(&hAmplitude[0],20,"same");
    // if (hb) c1->Update();
    

    // c1->SaveAs("diagnosticPlots.pdf");
        
    app.Run();
}
