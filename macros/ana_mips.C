/*
 T his *program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.
 
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor,
 Boston, MA  02110-1301, USA.
 
 ---
 Copyright (C) 2015, ufan <>
 */

#include <algorithm>
#include "TTree.h"
#include "TFile.h"
#include "TF1.h"
#include "TH1F.h"
#include "TMath.h"
#include "TGraphErrors.h"
#include "TCanvas.h"
#include "TROOT.h"
#include "TAxis.h"

Double_t langaufun(Double_t *x, Double_t *par) {

   //Fit parameters:
   //par[0]=Width (scale) parameter of Landau density
   //par[1]=Most Probable (MP, location) parameter of Landau density
   //par[2]=Total area (integral -inf to inf, normalization constant)
   //par[3]=Width (sigma) of convoluted Gaussian function
   //
   //In the Landau distribution (represented by the CERNLIB approximation),
   //the maximum is located at x=-0.22278298 with the location parameter=0.
   //This shift is corrected within this function, so that the actual
   //maximum is identical to the MP parameter.

      // Numeric constants
      Double_t invsq2pi = 0.3989422804014;   // (2 pi)^(-1/2)
      Double_t mpshift  = -0.22278298;       // Landau maximum location

      // Control constants
      Double_t np = 100.0;      // number of convolution steps
      Double_t sc =   5.0;      // convolution extends to +-sc Gaussian sigmas

      // Variables
      Double_t xx;
      Double_t mpc;
      Double_t fland;
      Double_t sum = 0.0;
      Double_t xlow,xupp;
      Double_t step;
      Double_t i;


      // MP shift correction
      mpc = par[1] - mpshift * par[0];

      // Range of convolution integral
      xlow = x[0] - sc * par[3];
      xupp = x[0] + sc * par[3];

      step = (xupp-xlow) / np;

      // Convolution integral of Landau and Gaussian by sum
      for(i=1.0; i<=np/2; i++) {
         xx = xlow + (i-.5) * step;
         fland = TMath::Landau(xx,mpc,par[0]) / par[0];
         sum += fland * TMath::Gaus(x[0],xx,par[3]);

         xx = xupp - (i-.5) * step;
         fland = TMath::Landau(xx,mpc,par[0]) / par[0];
         sum += fland * TMath::Gaus(x[0],xx,par[3]);
      }

      return (par[2] * step * sum * invsq2pi / par[3]);
}

TF1 *langaufit(TH1F *his, Double_t *fitrange, Double_t *startvalues, Double_t *parlimitslo, Double_t *parlimitshi, Double_t *fitparams, Double_t *fiterrors, Double_t *ChiSqr, Int_t *NDF)
{
   // Once again, here are the Landau * Gaussian parameters:
   //   par[0]=Width (scale) parameter of Landau density
   //   par[1]=Most Probable (MP, location) parameter of Landau density
   //   par[2]=Total area (integral -inf to inf, normalization constant)
   //   par[3]=Width (sigma) of convoluted Gaussian function
   //
   // Variables for langaufit call:
   //   his             histogram to fit
   //   fitrange[2]     lo and hi boundaries of fit range
   //   startvalues[4]  reasonable start values for the fit
   //   parlimitslo[4]  lower parameter limits
   //   parlimitshi[4]  upper parameter limits
   //   fitparams[4]    returns the final fit parameters
   //   fiterrors[4]    returns the final fit errors
   //   ChiSqr          returns the chi square
   //   NDF             returns ndf

   Int_t i;
   Char_t FunName[100];

   sprintf(FunName,"Fitfcn_%s",his->GetName());

   TF1 *ffitold = (TF1*)gROOT->GetListOfFunctions()->FindObject(FunName);
   if (ffitold) delete ffitold;

   TF1 *ffit = new TF1(FunName,langaufun,fitrange[0],fitrange[1],4);
   ffit->SetParameters(startvalues);
   ffit->SetParNames("Width","MP","Area","GSigma");

   for (i=0; i<4; i++) {
      ffit->SetParLimits(i, parlimitslo[i], parlimitshi[i]);
   }

   his->Fit(FunName,"RB0");   // fit within specified range, use ParLimits, do not plot

   ffit->GetParameters(fitparams);    // obtain fit parameters
   for (i=0; i<4; i++) {
      fiterrors[i] = ffit->GetParError(i);     // obtain fit parameter errors
   }
   ChiSqr[0] = ffit->GetChisquare();  // obtain chi^2
   NDF[0] = ffit->GetNDF();           // obtain ndf

   printf("ndf=%d, chisqr=%f\n",NDF[0],ChiSqr[0]);

   return (ffit);              // return fit function

}


TF1* langaus( TH1F *poHist,float& mpv,float& fwhm)
{
    poHist->Rebin(30);

    // Setting fit range and start values
    Double_t fr[2];
    Double_t sv[4],paramlow[4],paramhigh[4],fparam[4],fpe[4];
    Double_t chisqr;
    Int_t ndf;

    paramlow[0]=0.0;paramlow[1]=100.0;paramlow[2]=10;paramlow[3]=0.0;
    paramhigh[0]=150.0;paramhigh[1]=800.0;paramhigh[2]=100000.0;paramhigh[3]=200.0;

    if(poHist->GetEntries()<100){
        fr[0]=poHist->GetMean()*0.3;
        fr[1]=poHist->GetMean()+1500;
    }
    else{
        fr[0]=poHist->GetBinCenter(poHist->GetMaximumBin())*0.5;
        fr[1]=poHist->GetBinCenter(poHist->GetMaximumBin())+1500;
    }
    sv[0]=30;sv[1]=poHist->GetBinCenter(poHist->GetMaximumBin());sv[2]=poHist->Integral();sv[3]=50;

    TF1 *fit = langaufit(poHist,fr,sv,paramlow,paramhigh,fparam,fpe,&chisqr,&ndf);

    mpv=fparam[1];
    fwhm=fparam[0];

    return fit;
}


TF1* landau(TH1F *hist,float& mpv,float& fwhm)
{
    hist->Rebin(30);

    Double_t fr[2];
    Double_t para[3];

    if(hist->GetEntries()<100){
        fr[0]=hist->GetMean()*0.75;
        fr[1]=hist->GetMean()*2.5;
    }
    else{
        fr[0]=hist->GetBinCenter(hist->GetMaximumBin())*0.75;
        fr[1]=hist->GetBinCenter(hist->GetMaximumBin())*2.5;
    }
    para[0]=hist->Integral();para[1]=400;para[2]=40;

    TF1* ffit=new TF1("ffit","landau",fr[0],fr[1]);
    ffit->SetParameters(para);

    hist->Fit(ffit,"RB0");

    mpv=ffit->GetParameter(1);
    fwhm=ffit->GetParameter(2);

    //delete ffit;

    return ffit;
}

int draw_x_mips_u(char* infile,int strip_id,int segment_id,TH1F* hpos,TH1F* hneg,float psdoffset_x=6,float psdoffset_y=-2,float segment_limit=10)
{

    float x_psd_position[41]={374.28,354.28,334.28,314.28,294.28,274.28,254.28,234.28,214.28,194.28,174.28,154.28,134.28,114.28,94.28,74.28,54.28,34.28,14.28,-5.72,-25.72,-45.72,-65.72,-85.72,-105.72,-125.72,-145.72,-165.72,-185.72,-205.72,-225.72,-245.72,-265.72,-285.72,-305.72,-325.72,-345.72,-365.72,-385.72,-405.72,-425.72};
    float y_psd_position[41]={407.96,387.96,367.96,347.96,327.96,307.96,287.96,267.96,247.96,227.96,207.96,187.96,167.96,147.96,127.96,107.96,87.96,67.96,47.96,27.96,7.96,-12.04,-32.04,-52.04,-72.04,-92.04,-112.04,-132.04,-152.04,-172.04,-192.04,-212.04,-232.04,-252.04,-272.04,-292.04,-312.04,-332.04,-352.04,-372.04,-392.04};

    for(int i=0;i<41;i++){
      x_psd_position[i]+=psdoffset_x;
      y_psd_position[i]+=psdoffset_y;
    }
    //########################################################################################33

    TFile *f_in =new TFile(infile);
    TTree *tree_final=(TTree*)f_in->Get("event_simple");

    float ypsdhit_position[4];
    int xpsd_num;
    int xpsd_ch[41];
    float xpsd_energy_pos[41],xpsd_energy_neg[41];
    float track_angle;

    tree_final->SetBranchAddress("ypsdhit_y",ypsdhit_position);
    tree_final->SetBranchAddress("xpsd_num",&xpsd_num);
    tree_final->SetBranchAddress("xpsd_ch",xpsd_ch);
    tree_final->SetBranchAddress("xpsd_energy_pos",xpsd_energy_pos);
    tree_final->SetBranchAddress("xpsd_energy_neg",xpsd_energy_neg);
    tree_final->SetBranchAddress("track_angle",&track_angle);

    //----------------------------------------------------------------------

    int strip_layerid;
    if(strip_id%2)
        strip_layerid=2;
    else
        strip_layerid=3;
  
    //----------------------------------------------------------------------
    int entries=tree_final->GetEntries();  
    for(int i=0;i<entries;i++){
        tree_final->GetEntry(i);

        if(TMath::Abs(ypsdhit_position[strip_layerid]-y_psd_position[segment_id-1]) > segment_limit)
            continue;
        else{
            for(int j=0;j<xpsd_num;j++){
                if(xpsd_ch[j] == strip_id){
                    hpos->Fill(xpsd_energy_pos[j]*track_angle);//simple incident angle correction
                    hneg->Fill(xpsd_energy_neg[j]*track_angle);
                    break;
                }
            }
        }
    }

    delete f_in;

    return 0;
}

int draw_x_mips(char *infile, int strip_id, int segment_id, float segment_limit=10)
{

    TH1F* hpos=new TH1F(Form("hx%d_%d_pos",strip_id,segment_id),Form("Strip_X%d,Segment_%d,Positive_Side",strip_id,segment_id),3000,0,3000);
    TH1F* hneg=new TH1F(Form("hx%d_%d_neg",strip_id,segment_id),Form("Strip_X%d,Segment_%d,Negative_Side",strip_id,segment_id),3000,0,3000);
    hpos->SetDirectory(0);
    hneg->SetDirectory(0);
    draw_x_mips_u(infile,strip_id,segment_id,hpos,hneg,segment_limit);

/////////////////////////////////////////////////////////////////////////////////////////
    float mpv,fwhm;

    TF1* fitpos=landau(hpos,mpv,fwhm);
    TF1* fitneg=landau(hneg,mpv,fwhm);

    TCanvas* can=new TCanvas("can","can",800,400);
    can->Divide(2,1);
    can->cd(1);
    hpos->Draw();
    fitpos->Draw("lsame");
    can->cd(2);
    hneg->Draw();
    fitneg->Draw("lsame");

    return 0;
}

int draw_y_mips_u(char* infile,int strip_id,int segment_id,TH1F* hpos,TH1F* hneg,float psdoffset_x=6,float psdoffset_y=-2,float segment_limit=10)
{
    float x_psd_position[41]={374.28,354.28,334.28,314.28,294.28,274.28,254.28,234.28,214.28,194.28,174.28,154.28,134.28,114.28,94.28,74.28,54.28,34.28,14.28,-5.72,-25.72,-45.72,-65.72,-85.72,-105.72,-125.72,-145.72,-165.72,-185.72,-205.72,-225.72,-245.72,-265.72,-285.72,-305.72,-325.72,-345.72,-365.72,-385.72,-405.72,-425.72};
    float y_psd_position[41]={407.96,387.96,367.96,347.96,327.96,307.96,287.96,267.96,247.96,227.96,207.96,187.96,167.96,147.96,127.96,107.96,87.96,67.96,47.96,27.96,7.96,-12.04,-32.04,-52.04,-72.04,-92.04,-112.04,-132.04,-152.04,-172.04,-192.04,-212.04,-232.04,-252.04,-272.04,-292.04,-312.04,-332.04,-352.04,-372.04,-392.04};
    
    for(int i=0;i<41;i++){
      x_psd_position[i]+=psdoffset_x;
      y_psd_position[i]+=psdoffset_y;
    }
    //########################################################################################33

    TFile *f_in =new TFile(infile);
    TTree *tree_final=(TTree*)f_in->Get("event_simple");

    float xpsdhit_position[4];
    int ypsd_num;
    int ypsd_ch[41];
    float ypsd_energy_pos[41],ypsd_energy_neg[41];
    float track_angle;

    tree_final->SetBranchAddress("xpsdhit_x",xpsdhit_position);
    tree_final->SetBranchAddress("ypsd_num",&ypsd_num);
    tree_final->SetBranchAddress("ypsd_ch",ypsd_ch);
    tree_final->SetBranchAddress("ypsd_energy_pos",ypsd_energy_pos);
    tree_final->SetBranchAddress("ypsd_energy_neg",ypsd_energy_neg);
    tree_final->SetBranchAddress("track_angle",&track_angle);

    int strip_layerid;
    if(strip_id%2)
        strip_layerid=0;
    else
        strip_layerid=1;

    int entries=tree_final->GetEntries();
    //printf("entries: %d\n",entries);
    for(int i=0;i<entries;i++){
        tree_final->GetEntry(i);

        if(TMath::Abs(xpsdhit_position[strip_layerid]-x_psd_position[segment_id-1]) > segment_limit)
            continue;
        else{
            for(int j=0;j<ypsd_num;j++){
                if(ypsd_ch[j] == strip_id){
                    hpos->Fill(ypsd_energy_pos[j]*track_angle);//simple track angle correction
                    hneg->Fill(ypsd_energy_neg[j]*track_angle);
                    break;
                }
            }
        }
    }

    delete f_in;

    return 0;
}

int draw_y_mips(char *infile, int strip_id, int segment_id, float segment_limit=10)
{

    TH1F* hpos=new TH1F(Form("hy%d_%d_pos",strip_id,segment_id),Form("Strip_Y%d,Segment_%d,Positive_Side",strip_id,segment_id),3000,0,3000);
    TH1F* hneg=new TH1F(Form("hy%d_%d_neg",strip_id,segment_id),Form("Strip_Y%d,Segment_%d,Negative_Side",strip_id,segment_id),3000,0,3000);
    hpos->SetDirectory(0);
    hneg->SetDirectory(0);

    draw_y_mips_u(infile,strip_id,segment_id,hpos,hneg,segment_limit);
    //------------------------------------------------------------------
    float mpv,fwhm;
    TF1* fitpos=landau(hpos,mpv,fwhm);

    TF1* fitneg=landau(hneg,mpv,fwhm);

    TCanvas* can=new TCanvas("can","can",800,400);
    can->Divide(2,1);
    can->cd(1);
    hpos->Draw();
    fitpos->Draw("lsame");
    can->cd(2);
    hneg->Draw();
    fitneg->Draw("lsame");
    return 0;
}

int draw_mips(char* infile,char* outfile,float segment_limit=10)
{
    TFile *f_out=new TFile(outfile,"recreate");

    //int segment_index[5]={5,13,21,29,37};
    int segment_index[41];
    for(int i=0;i<41;i++){
        segment_index[i]=i+1;
    }
    //float segment_position[5]={91,251,411,571,731};

    TH1F* hxpos[41][41];
    TH1F* hxneg[41][41];
    TH1F* hypos[41][41];
    TH1F* hyneg[41][41];

    for(int i=0;i<41;i++){
        for(int j=0;j<41;j++){
            hxpos[i][j]=new TH1F(Form("hx%d_%d_pos",i+1,segment_index[j]),Form("Strip_X%d,Segment_%d,Positive_Side",i+1,segment_index[j]),3000,0,3000);
            hxneg[i][j]=new TH1F(Form("hx%d_%d_neg",i+1,segment_index[j]),Form("Strip_X%d,Segment_%d,Negative_Side",i+1,segment_index[j]),3000,0,3000);
            hypos[i][j]=new TH1F(Form("hy%d_%d_pos",i+1,segment_index[j]),Form("Strip_Y%d,Segment_%d,Positive_Side",i+1,segment_index[j]),3000,0,3000);
            hyneg[i][j]=new TH1F(Form("hy%d_%d_neg",i+1,segment_index[j]),Form("Strip_Y%d,Segment_%d,Negative_Side",i+1,segment_index[j]),3000,0,3000);
        }
    }

    for(int i=0;i<41;i++){
        for(int j=0;j<41;j++){
            draw_x_mips_u(infile,i+1,segment_index[j],hxpos[i][j],hxneg[i][j],segment_limit);
            f_out->cd();
            hxpos[i][j]->Write(0,TObject::kOverwrite);
            hxneg[i][j]->Write(0,TObject::kOverwrite);
            draw_y_mips_u(infile,i+1,segment_index[j],hypos[i][j],hyneg[i][j],segment_limit);
            f_out->cd();
            hypos[i][j]->Write(0,TObject::kOverwrite);
            hyneg[i][j]->Write(0,TObject::kOverwrite);

            printf("%d,%d\n",i+1,j+1);
        }
    }

    delete f_out;

    return 0;
}

int draw_mips_new(char* infile,char* outfile,float segment_limit=10)
{

    float x_psd_position[41]={374.28,354.28,334.28,314.28,294.28,274.28,254.28,234.28,214.28,194.28,174.28,154.28,134.28,114.28,94.28,74.28,54.28,34.28,14.28,-5.72,-25.72,-45.72,-65.72,-85.72,-105.72,-125.72,-145.72,-165.72,-185.72,-205.72,-225.72,-245.72,-265.72,-285.72,-305.72,-325.72,-345.72,-365.72,-385.72,-405.72,-425.72};

    float y_psd_position[41]={407.96,387.96,367.96,347.96,327.96,307.96,287.96,267.96,247.96,227.96,207.96,187.96,167.96,147.96,127.96,107.96,87.96,67.96,47.96,27.96,7.96,-12.04,-32.04,-52.04,-72.04,-92.04,-112.04,-132.04,-152.04,-172.04,-192.04,-212.04,-232.04,-252.04,-272.04,-292.04,-312.04,-332.04,-352.04,-372.04,-392.04};

    //########################################################################################33

    TFile *f_in =new TFile(infile);
    TTree *tree_final=(TTree*)f_in->Get("event_simple");

    float xpsdhit_position[4],ypsdhit_position[4];
    int xpsd_num,ypsd_num;
    int xpsd_ch[41],ypsd_ch[41];
    float xpsd_energy_pos[41],xpsd_energy_neg[41];
    float ypsd_energy_pos[41],ypsd_energy_neg[41];
    float track_angle;

    tree_final->SetBranchAddress("xpsdhit_x",xpsdhit_position);
    tree_final->SetBranchAddress("xpsd_num",&xpsd_num);
    tree_final->SetBranchAddress("xpsd_ch",xpsd_ch);
    tree_final->SetBranchAddress("xpsd_energy_pos",xpsd_energy_pos);
    tree_final->SetBranchAddress("xpsd_energy_neg",xpsd_energy_neg);
    tree_final->SetBranchAddress("track_angle",&track_angle);
    tree_final->SetBranchAddress("ypsdhit_y",ypsdhit_position);
    tree_final->SetBranchAddress("ypsd_num",&ypsd_num);
    tree_final->SetBranchAddress("ypsd_ch",ypsd_ch);
    tree_final->SetBranchAddress("ypsd_energy_pos",ypsd_energy_pos);
    tree_final->SetBranchAddress("ypsd_energy_neg",ypsd_energy_neg);

//----------------------------------------------------------------------
    TFile *f_out=new TFile(outfile,"recreate");

    //int segment_index[5]={5,13,21,29,37};
    int segment_index[41];
    for(int i=0;i<41;i++){
        segment_index[i]=i+1;
    }
    //float segment_position[5]={91,251,411,571,731};

    TH1F* hxpos[41][41];
    TH1F* hxneg[41][41];
    TH1F* hypos[41][41];
    TH1F* hyneg[41][41];

    for(int i=0;i<41;i++){
        for(int j=0;j<41;j++){
            hxpos[i][j]=new TH1F(Form("hx%d_%d_pos",i+1,segment_index[j]),Form("Strip_X%d,Segment_%d,Positive_Side",i+1,segment_index[j]),3000,0,3000);
            hxneg[i][j]=new TH1F(Form("hx%d_%d_neg",i+1,segment_index[j]),Form("Strip_X%d,Segment_%d,Negative_Side",i+1,segment_index[j]),3000,0,3000);
            hypos[i][j]=new TH1F(Form("hy%d_%d_pos",i+1,segment_index[j]),Form("Strip_Y%d,Segment_%d,Positive_Side",i+1,segment_index[j]),3000,0,3000);
            hyneg[i][j]=new TH1F(Form("hy%d_%d_neg",i+1,segment_index[j]),Form("Strip_Y%d,Segment_%d,Negative_Side",i+1,segment_index[j]),3000,0,3000);
        }
    }
//--------------------------
    int entries=tree_final->GetEntries();
    
    for(int i=0;i<entries;i++){
        tree_final->GetEntry(i);


  }

    return 0;
}

int fit_mips(char* infile,char* outfile)
{
    float strip_position[41];
    for(int i=0;i<41;i++)
        strip_position[i]=20.0*(i-20);
//------------------------------------------------------
    TFile *f_in=new TFile(infile);
    TFile *f_out=new TFile(outfile,"recreate");

    float xpos_MPV[41][41],xpos_FWHM[41][41],xpos_MPV_error[41][41];
    float xneg_MPV[41][41],xneg_FWHM[41][41],xneg_MPV_error[41][41];
    float ypos_MPV[41][41],ypos_FWHM[41][41],ypos_MPV_error[41][41];
    float yneg_MPV[41][41],yneg_FWHM[41][41],yneg_MPV_error[41][41];
//-----------------Fitting----------------------------------------------
    TH1F *hxpos,*hxneg,*hypos,*hyneg;
    TF1 *fxpos,*fxneg,*fypos,*fyneg;
    for(int i=0;i<41;i++){
        for(int j=0;j<41;j++){
            hxpos=(TH1F*)f_in->Get(Form("hx%d_%d_pos",i+1,j+1));
            fxpos=langaus(hxpos,xpos_MPV[i][j],xpos_FWHM[i][j]);
            //fxpos=landau(hxpos,xpos_MPV[i][j],xpos_FWHM[i][j]);
            xpos_MPV_error[i][j]=fxpos->GetParError(1);
            delete fxpos;

            hxneg=(TH1F*)f_in->Get(Form("hx%d_%d_neg",i+1,j+1));
            fxneg=langaus(hxneg,xneg_MPV[i][j],xneg_FWHM[i][j]);
            //fxneg=landau(hxneg,xneg_MPV[i][j],xneg_FWHM[i][j]);
            xneg_MPV_error[i][j]=fxneg->GetParError(1);
            delete fxneg;

            hypos=(TH1F*)f_in->Get(Form("hy%d_%d_pos",i+1,j+1));
            fypos=langaus(hypos,ypos_MPV[i][j],ypos_FWHM[i][j]);
            //fypos=landau(hypos,ypos_MPV[i][j],ypos_FWHM[i][j]);
            ypos_MPV_error[i][j]=fypos->GetParError(1);
            delete fypos;

            hyneg=(TH1F*)f_in->Get(Form("hy%d_%d_neg",i+1,j+1));
            fyneg=langaus(hyneg,yneg_MPV[i][j],yneg_FWHM[i][j]);
            //fyneg=landau(hyneg,yneg_MPV[i][j],yneg_FWHM[i][j]);
            yneg_MPV_error[i][j]=fyneg->GetParError(1);
            delete fyneg;
        }
    }

//-----------------Draw Attentuation Length-----------------------------------------------------------
    TGraphErrors* gxpos[41];
    TGraphErrors* gxneg[41];
    TGraphErrors* gypos[41];
    TGraphErrors* gyneg[41];
    int gxpos_np[41],gxneg_np[41],gypos_np[41],gyneg_np[41];
    for(int i=0;i<41;i++){
        gxpos[i]=new TGraphErrors;
        gxpos[i]->SetTitle(Form("+X Strip_%d: Amplitude Vs Postion",i+1));
        gxpos[i]->SetName(Form("gxpos_attlength_%d",i+1));
        gxpos[i]->SetMarkerStyle(22);
        gxpos_np[i]=0;

        gxneg[i]=new TGraphErrors;
        gxneg[i]->SetTitle(Form("-X Strip_%d: Amplitude Vs Postion",i+1));
        gxneg[i]->SetName(Form("gxneg_attlength_%d",i+1));
        gxneg[i]->SetMarkerStyle(22);
        gxneg_np[i]=0;

        gypos[i]=new TGraphErrors;
        gypos[i]->SetTitle(Form("+Y Strip_%d: Amplitude Vs Postion",i+1));
        gypos[i]->SetName(Form("gypos_attlength_%d",i+1));
        gypos[i]->SetMarkerStyle(22);
        gypos_np[i]=0;

        gyneg[i]=new TGraphErrors;
        gyneg[i]->SetTitle(Form("-Y Strip_%d: Amplitude Vs Postion",i+1));
        gyneg[i]->SetName(Form("gyneg_attlength_%d",i+1));
        gyneg[i]->SetMarkerStyle(22);
        gyneg_np[i]=0;
    }

    std::fill_n(gxpos_np,41,0);
    std::fill_n(gxneg_np,41,0);
    std::fill_n(gypos_np,41,0);
    std::fill_n(gyneg_np,41,0);
    for(int i=0;i<41;i++){
        for(int j=0;j<41;j++){
            if(xpos_MPV[i][j]){
                gxpos[i]->SetPoint(gxpos_np[i],strip_position[40-j],xpos_MPV[i][j]);
                gxpos[i]->SetPointError(gxpos_np[i],0,xpos_MPV_error[i][j]);
                gxpos_np[i]++;
            }

            if(xneg_MPV[i][j]){
                gxneg[i]->SetPoint(gxneg_np[i],strip_position[j],xneg_MPV[i][j]);
                gxneg[i]->SetPointError(gxneg_np[i],0,xneg_MPV_error[i][j]);
                gxneg_np[i]++;
            }

            if(ypos_MPV[i][j]){
                gypos[i]->SetPoint(gypos_np[i],strip_position[40-j],ypos_MPV[i][j]);
                gypos[i]->SetPointError(gypos_np[i],0.0,ypos_MPV_error[i][j]);
                gypos_np[i]++;
            }

            if(yneg_MPV[i][j]){
                gyneg[i]->SetPoint(gyneg_np[i],strip_position[j],yneg_MPV[i][j]);
                gyneg[i]->SetPointError(gyneg_np[i],0,yneg_MPV_error[i][j]);
                gyneg_np[i]++;
            }

        }
        gxpos[i]->Set(gxpos_np[i]);
        gxneg[i]->Set(gxneg_np[i]);
        gypos[i]->Set(gypos_np[i]);
        gyneg[i]->Set(gyneg_np[i]);

        f_out->cd();
        gxpos[i]->Write();
        gxneg[i]->Write();
        gypos[i]->Write();
        gyneg[i]->Write();
    }

    //----------------end-----------------------
    delete f_in;
    delete f_out;

    return 0;
}

int draw_energy_resolution(char* infile,char* outfile)
{

    TFile *f_in=new TFile(infile);
    TFile *f_out=new TFile(outfile,"recreate");

    float xpos_MPV[41],xpos_FWHM[41];
    float xneg_MPV[41],xneg_FWHM[41];
    float ypos_MPV[41],ypos_FWHM[41];
    float yneg_MPV[41],yneg_FWHM[41];
//-----------------Fitting----------------------------------------------
    TH1F *hxpos,*hxneg,*hypos,*hyneg;
    TF1 *fxpos,*fxneg,*fypos,*fyneg;
    for(int i=0;i<41;i++){
            hxpos=(TH1F*)f_in->Get(Form("hx%d_%d_pos",i+1,21));
            fxpos=landau(hxpos,xpos_MPV[i],xpos_FWHM[i]);
            delete fxpos;

            hxneg=(TH1F*)f_in->Get(Form("hx%d_%d_neg",i+1,21));
            fxneg=landau(hxneg,xneg_MPV[i],xneg_FWHM[i]);
            delete fxneg;

            hypos=(TH1F*)f_in->Get(Form("hy%d_%d_pos",i+1,21));
            fypos=landau(hypos,ypos_MPV[i],ypos_FWHM[i]);
            delete fypos;

            hyneg=(TH1F*)f_in->Get(Form("hy%d_%d_neg",i+1,21));
            fyneg=landau(hyneg,yneg_MPV[i],yneg_FWHM[i]);
            delete fyneg;
    }

    //---------------Draw Energy Resolution in the middle of each strip---------------------
    TH1F* hxpos_reslution=new TH1F("hxpos_res","X Layer Positive Side: Energy Resolution VS Strip",41,0.5,41.5);
    TH1F* hxneg_reslution=new TH1F("hxneg_res","X Layer Negative Side: Energy Resolution VS Strip",41,0.5,41.5);
    TH1F* hypos_reslution=new TH1F("hypos_res","Y Layer Positive Side: Energy Resolution VS Strip",41,0.5,41.5);
    TH1F* hyneg_reslution=new TH1F("hyneg_res","Y Layer Negative Side: Energy Resolution VS Strip",41,0.5,41.5);


    FILE *fp=fopen("energy_res.csv","w");
    for(int i=0;i<41;i++){
        hxpos_reslution->SetBinContent(i+1,xpos_FWHM[i]/xpos_MPV[i]);
        hxneg_reslution->SetBinContent(i+1,xneg_FWHM[i]/xneg_MPV[i]);

        hypos_reslution->SetBinContent(i+1,ypos_FWHM[i]/ypos_MPV[i]);
        hyneg_reslution->SetBinContent(i+1,yneg_FWHM[i]/yneg_MPV[i]);

        fprintf(fp,"x%d_pos:%.3f\tx%d_neg:%.3f\t##\ty%d_pos:%.3f\ty%d_neg:%.3f\n",i+1,xpos_FWHM[i]/xpos_MPV[i],i+1,xneg_FWHM[i]/xneg_MPV[i],
                i+1,ypos_FWHM[i]/ypos_MPV[i],i+1,yneg_FWHM[i]/ypos_MPV[i]);
    }
    hypos_reslution->SetBinContent(1,0.099);//manual fit for ypos_1
    fclose(fp);

    hxpos_reslution->GetYaxis()->SetRangeUser(0,0.25);
    hxneg_reslution->GetYaxis()->SetRangeUser(0,0.25);
    hypos_reslution->GetYaxis()->SetRangeUser(0,0.25);
    hyneg_reslution->GetYaxis()->SetRangeUser(0,0.25);

    f_out->cd();
    hxpos_reslution->Write(0,TObject::kOverwrite);
    hxneg_reslution->Write(0,TObject::kOverwrite);
    hypos_reslution->Write(0,TObject::kOverwrite);
    hyneg_reslution->Write(0,TObject::kOverwrite);

    //----------------end-----------------------
    delete f_in;
    delete f_out;

    return 0;

}

int draw_consistency(char* infile,char* outfile)
{
    TFile *f_in=new TFile(infile);
    TFile *f_out=new TFile(outfile,"recreate");

    float xpos_MPV[41],xpos_FWHM[41];
    float xneg_MPV[41],xneg_FWHM[41];
    float ypos_MPV[41],ypos_FWHM[41];
    float yneg_MPV[41],yneg_FWHM[41];
//-----------------Fitting----------------------------------------------
    TH1F *hxpos,*hxneg,*hypos,*hyneg;
    TF1 *fxpos,*fxneg,*fypos,*fyneg;

    for(int i=0;i<41;i++){
            hxpos=(TH1F*)f_in->Get(Form("hx%d_%d_pos",i+1,21));
            //fxpos=langaus(hxpos,xpos_MPV[i],xpos_FWHM[i]);
            fxpos=landau(hxpos,xpos_MPV[i],xpos_FWHM[i]);
            delete fxpos;

            hxneg=(TH1F*)f_in->Get(Form("hx%d_%d_neg",i+1,21));
            //fxneg=langaus(hxneg,xneg_MPV[i],xneg_FWHM[i]);
            fxneg=landau(hxneg,xneg_MPV[i],xneg_FWHM[i]);
            delete fxneg;

            hypos=(TH1F*)f_in->Get(Form("hy%d_%d_pos",i+1,21));
            //fypos=langaus(hypos,ypos_MPV[i],ypos_FWHM[i]);
            fypos=landau(hypos,ypos_MPV[i],ypos_FWHM[i]);
            delete fypos;

            hyneg=(TH1F*)f_in->Get(Form("hy%d_%d_neg",i+1,21));
            //fyneg=langaus(hyneg,yneg_MPV[i],yneg_FWHM[i]);
            fyneg=landau(hyneg,yneg_MPV[i],yneg_FWHM[i]);
            delete fyneg;
    }

    //---------------Draw Energy Resolution in the middle of each strip---------------------
    TH1F* hxpos_consistency=new TH1F("hxpos_cons","X Layer Positive Side",41,0.5,41.5);
    TH1F* hxneg_consistency=new TH1F("hxneg_cons","X Layer Negative Side",41,0.5,41.5);
    TH1F* hypos_consistency=new TH1F("hypos_cons","Y Layer Positive Side",41,0.5,41.5);
    TH1F* hyneg_consistency=new TH1F("hyneg_cons","Y Layer Negative Side",41,0.5,41.5);
    TH1F* hdist=new TH1F("h1","MIPs Distribution",200,350.0,550.0);

    FILE *fp=fopen("consistency.csv","w");
    for(int i=0;i<41;i++){
        hxpos_consistency->SetBinContent(i+1,xpos_MPV[i]);
        hxneg_consistency->SetBinContent(i+1,xneg_MPV[i]);

        hypos_consistency->SetBinContent(i+1,ypos_MPV[i]);
        hyneg_consistency->SetBinContent(i+1,yneg_MPV[i]);

        hdist->Fill(xpos_MPV[i]);hdist->Fill(xneg_MPV[i]);
        hdist->Fill(ypos_MPV[i]);hdist->Fill(yneg_MPV[i]);
        fprintf(fp,"x%d_pos:%.3f\tx%d_neg:%.3f\t##\ty%d_pos:%.3f\ty%d_neg:%.3f\n",i+1,xpos_MPV[i],i+1,xneg_MPV[i],
                i+1,ypos_MPV[i],i+1,yneg_MPV[i]);
    }
    fclose(fp);

    hxpos_consistency->GetYaxis()->SetRangeUser(0,700);
    hxneg_consistency->GetYaxis()->SetRangeUser(0,700);
    hypos_consistency->GetYaxis()->SetRangeUser(0,700);
    hyneg_consistency->GetYaxis()->SetRangeUser(0,700);

    f_out->cd();
    hxpos_consistency->Write(0,TObject::kOverwrite);
    hxneg_consistency->Write(0,TObject::kOverwrite);
    hypos_consistency->Write(0,TObject::kOverwrite);
    hyneg_consistency->Write(0,TObject::kOverwrite);
    hdist->Write(0,TObject::kOverwrite);

    //----------------end-----------------------
    delete f_in;
    delete f_out;

    return 0;
}

int draw_consistency_dist(char* infile)
{
    FILE *fp=fopen(infile,"r");
    TH1F* h1=new TH1F("h1","MIPs Distribution",200,300.0,500.0);
    int ch_id[4];
    float mips[4];
    for(int i=0;i<41;i++){
        fscanf(fp,"x%d_pos:%f\tx%d_neg:%f\t##\ty%d_pos:%f\ty%d_neg:%f\n",ch_id,mips,ch_id+1,mips+1,
            ch_id+2,mips+2,ch_id+3,mips+3);
        printf("x%d_pos:%.3f\tx%d_neg:%.3f\t##\ty%d_pos:%.3f\ty%d_neg:%.3f\n",ch_id[0],mips[0],ch_id[1],mips[1],
                ch_id[2],mips[2],ch_id[3],mips[3]);
        for(int j=0;j<4;j++){
            h1->Fill(mips[j]);
        }
    }
    h1->Draw();

    fclose(fp);

    return 0;
}