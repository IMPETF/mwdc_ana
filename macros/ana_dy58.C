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
#include "TTree.h"
#include "TFile.h"
#include "TF1.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TProfile.h"
#include "TMath.h"
#include "TGraphErrors.h"
#include "TCanvas.h"
#include "TROOT.h"
#include "TAxis.h"

int draw_dy58_u(char* infile,char layer,int strip_index)
{
    TFile* f_in=new TFile(infile);
    TTree* tree_in=(TTree*)f_in->Get("psd_hitinfo");

    TH2F* hpos=new TH2F("hpos",Form("%c_strip %d,positve",layer,strip_index),200,0,2000,1700,0,17000);
    TH2F* hneg=new TH2F("hneg",Form("%c_strip %d,negative",layer,strip_index),200,0,2000,1700,0,17000);
    hpos->GetXaxis()->SetTitle("Dy5");hpos->GetYaxis()->SetTitle("Dy8");
    hneg->GetXaxis()->SetTitle("Dy5");hneg->GetYaxis()->SetTitle("Dy8");

    TString pos_label,neg_label;
    pos_label.Form("%cpos_dy8[%d]:%cpos_dy5[%d]>>hpos",layer,strip_index-1,layer,strip_index-1);
    neg_label.Form("%cneg_dy8[%d]:%cneg_dy5[%d]>>hneg",layer,strip_index-1,layer,strip_index-1);

    TCanvas* can=new TCanvas("can","can",900,450);
    can->Divide(2,1);
    can->cd(1);
    tree_in->Draw(pos_label.Data());
    can->cd(2);
    tree_in->Draw(neg_label.Data());

    hpos->SetDirectory(0);
    hneg->SetDirectory(0);

    delete f_in;

    return 0;
}

int draw_dy58_uu(char* infile,char layer,int strip_index)
{
    TFile* f_in=new TFile(infile);
    TTree* tree_in=(TTree*)f_in->Get("psd_hitinfo");
    int pos_dy8[41],pos_dy5[41],neg_dy8[41],neg_dy5[41];
    tree_in->SetBranchAddress(Form("%cpos_dy8",layer),pos_dy8);
    tree_in->SetBranchAddress(Form("%cpos_dy5",layer),pos_dy5);
    tree_in->SetBranchAddress(Form("%cneg_dy8",layer),neg_dy8);
    tree_in->SetBranchAddress(Form("%cneg_dy5",layer),neg_dy5);

    TH2F* hpos=new TH2F("hpos",Form("%c_strip %d,positve",layer,strip_index),200,0,2000,1700,0,17000);
    TH2F* hneg=new TH2F("hneg",Form("%c_strip %d,negative",layer,strip_index),200,0,2000,1700,0,17000);
    hpos->GetXaxis()->SetTitle("Dy5");hpos->GetYaxis()->SetTitle("Dy8");
    hneg->GetXaxis()->SetTitle("Dy5");hneg->GetYaxis()->SetTitle("Dy8");

    //TString pos_label,neg_label;
    //pos_label.Form("%cpos_dy8[%d]:%cpos_dy5[%d]>>hpos",layer,strip_index-1,layer,strip_index-1);
    //neg_label.Form("%cneg_dy8[%d]:%cneg_dy5[%d]>>hneg",layer,strip_index-1,layer,strip_index-1);

    Int_t entries=tree_in->GetEntries();
    for(Int_t i=0;i<entries;i++){
        tree_in->GetEntry(i);
        hpos->Fill(pos_dy5[strip_index-1],pos_dy8[strip_index-1]);
        hneg->Fill(neg_dy5[strip_index-1],neg_dy8[strip_index-1]);
    }

    TCanvas* can=new TCanvas("can","can",900,450);
    can->Divide(2,1);
    can->cd(1);
    hpos->Draw();
    can->cd(2);
    hneg->Draw();

    hpos->SetDirectory(0);
    hneg->SetDirectory(0);

    delete f_in;

    return 0;
}

int draw_dy58(char* infile,char* outfile)
{
    TFile* f_in=new TFile(infile);
    TTree* tree_in=(TTree*)f_in->Get("psd_hitinfo");
    TFile* f_out=new TFile(outfile,"recreate");

    //TH2F *hpos,*hneg;
    TH2F* hpos=new TH2F("hpos","hpos",200,0,2000,1700,0,17000);
    TH2F* hneg=new TH2F("hneg","hneg",200,0,2000,1700,0,17000);
    hpos->GetXaxis()->SetTitle("Dy5");hpos->GetYaxis()->SetTitle("Dy8");
    hneg->GetXaxis()->SetTitle("Dy5");hneg->GetYaxis()->SetTitle("Dy8");

    TString pos_label,neg_label;
    //pos_label.Form("%cpos_dy8[%d]:%cpos_dy5[%d]>>hpos",layer,strip_index-1,layer,strip_index-1);
    //neg_label.Form("%cneg_dy8[%d]:%cneg_dy5[%d]>>hneg",layer,strip_index-1,layer,strip_index-1);

    TCanvas* can=new TCanvas("can","can",900,450);
    can->Divide(2,1);
    can->Print("dy58.pdf[");
    for(int i=0;i<41;i++){
        hpos->SetName(Form("hxpos_dy58_%d",i+1));
        hneg->SetName(Form("hxneg_dy58_%d",i+1));
        hpos->SetTitle(Form("hxpos_dy58_%d",i+1));
        hneg->SetTitle(Form("hxneg_dy58_%d",i+1));
        pos_label.Form("xpos_dy8[%d]:xpos_dy5[%d]>>hxpos_dy58_%d",i,i,i+1);
        neg_label.Form("xneg_dy8[%d]:xneg_dy5[%d]>>hxneg_dy58_%d",i,i,i+1);
        can->cd(1);
        tree_in->Draw(pos_label.Data());
        can->cd(2);
        tree_in->Draw(neg_label.Data());
        can->Print("dy58.pdf");
        f_out->cd();
        hpos->Write();hneg->Write();
    }

    for(int i=0;i<41;i++){
        hpos->SetName(Form("hypos_dy58_%d",i+1));
        hneg->SetName(Form("hyneg_dy58_%d",i+1));
        hpos->SetTitle(Form("hypos_dy58_%d",i+1));
        hneg->SetTitle(Form("hyneg_dy58_%d",i+1));
        pos_label.Form("ypos_dy8[%d]:ypos_dy5[%d]>>hypos_dy58_%d",i,i,i+1);
        neg_label.Form("yneg_dy8[%d]:yneg_dy5[%d]>>hyneg_dy58_%d",i,i,i+1);
        can->cd(1);
        tree_in->Draw(pos_label.Data());
        can->cd(2);
        tree_in->Draw(neg_label.Data());
        can->Print("dy58.pdf");
        f_out->cd();
        hpos->Write();hneg->Write();
    }
    can->Print("dy58.pdf]");

    f_out->Close();
    delete f_out;
    delete f_in;

    return 0;
}

TF1* linear_fit(TProfile* hprofile,Double_t *fitrange)
{
    char FunName[100];
    sprintf(FunName,"Fitfcn_%s",hprofile->GetName());
    TF1* ffitold=(TF1*)gROOT->GetListOfFunctions()->FindObject(FunName);
    if(ffitold) delete ffitold;

    TF1* ffit=new TF1(FunName,"pol1",fitrange[0],fitrange[1]);
    hprofile->Fit(FunName,"R0");

    return ffit;
}

int fit_dy58(char* infile,char* pedfile,char* outfile,int pedcut=7,float range=200.0)
{
    //--- pedestal ----------------
    int id8[41]={0,1,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,22,46,47,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64,65,66};
    int id5[41]={23,24,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,45,68,69,71,72,73,74,75,76,77,78,79,80,81,82,83,84,85,86,87,88};

    int channel;
    float mean,sigma;
    float xpedmean_dy8_pos[41],xpedmean_dy5_pos[41],xpedmean_dy8_neg[41],xpedmean_dy5_neg[41];
    float ypedmean_dy8_pos[41],ypedmean_dy5_pos[41],ypedmean_dy8_neg[41],ypedmean_dy5_neg[41];
    float xpedsigma_dy8_pos[41],xpedsigma_dy5_pos[41],xpedsigma_dy8_neg[41],xpedsigma_dy5_neg[41];
    float ypedsigma_dy8_pos[41],ypedsigma_dy5_pos[41],ypedsigma_dy8_neg[41],ypedsigma_dy5_neg[41];
    TFile *f_ped=new TFile(pedfile);

    TTree *tree_ped=(TTree*)f_ped->Get("xpos_ped");
    tree_ped->SetBranchAddress("channel",&channel);
    tree_ped->SetBranchAddress("mean",&mean);
    tree_ped->SetBranchAddress("sigma",&sigma);
    tree_ped->BuildIndex("channel");
    for(int i=0;i<41;i++){
        tree_ped->GetEntryWithIndex(id8[i]+1);
        xpedmean_dy8_pos[i]=mean;
        xpedsigma_dy8_pos[i]=sigma;

        tree_ped->GetEntryWithIndex(id5[i]+1);
        xpedmean_dy5_pos[i]=mean;
        xpedsigma_dy5_pos[i]=sigma;
    }

    tree_ped=(TTree*)f_ped->Get("xneg_ped");
    tree_ped->SetBranchAddress("channel",&channel);
    tree_ped->SetBranchAddress("mean",&mean);
    tree_ped->SetBranchAddress("sigma",&sigma);
    tree_ped->BuildIndex("channel");
    for(int i=0;i<41;i++){
        tree_ped->GetEntryWithIndex(id8[40-i]+1);
        xpedmean_dy8_neg[i]=mean;
        xpedsigma_dy8_neg[i]=sigma;

        tree_ped->GetEntryWithIndex(id5[40-i]+1);
        xpedmean_dy5_neg[i]=mean;
        xpedsigma_dy5_neg[i]=sigma;
    }

    tree_ped=(TTree*)f_ped->Get("ypos_ped");
    tree_ped->SetBranchAddress("channel",&channel);
    tree_ped->SetBranchAddress("mean",&mean);
    tree_ped->SetBranchAddress("sigma",&sigma);
    tree_ped->BuildIndex("channel");
    for(int i=0;i<41;i++){
        tree_ped->GetEntryWithIndex(id8[40-i]+1);
        ypedmean_dy8_pos[i]=mean;
        ypedsigma_dy8_pos[i]=sigma;

        tree_ped->GetEntryWithIndex(id5[40-i]+1);
        ypedmean_dy5_pos[i]=mean;
        ypedsigma_dy5_pos[i]=sigma;
    }

    tree_ped=(TTree*)f_ped->Get("yneg_ped");
    tree_ped->SetBranchAddress("channel",&channel);
    tree_ped->SetBranchAddress("mean",&mean);
    tree_ped->SetBranchAddress("sigma",&sigma);
    tree_ped->BuildIndex("channel");
    for(int i=0;i<41;i++){
        tree_ped->GetEntryWithIndex(id8[i]+1);
        ypedmean_dy8_neg[i]=mean;
        ypedsigma_dy8_neg[i]=sigma;

        tree_ped->GetEntryWithIndex(id5[i]+1);
        ypedmean_dy5_neg[i]=mean;
        ypedsigma_dy5_neg[i]=sigma;
    }
    delete f_ped;

    //----get profile of dy58,and fit---------------------------
    Double_t fitrange[2];
    TFile *f_in=new TFile(infile,"update");
    TH1F* hdy58_dist=new TH1F("hdy58_dist","Dy58 Ratio Distribution",200,40.0,60.0);
    TH2F* hdy58;
    TProfile* hdy58_pfx;
    TF1* lffit;
    Float_t dy58_ratio;

    FILE* fp=fopen(Form("%s.txt",outfile),"w");
    fprintf(fp,"dy58_ratio:\n");
    fprintf(fp,"index\txpos\txneg\typos\tyneg\n");

    TCanvas* can=new TCanvas("can_dy58_fit","can_dy58_fit",900,900);
    can->Divide(2,2);
    can->Print(Form("%s.pdf[",outfile));

    for(int i=0;i<41;i++){
        fprintf(fp,"%d\t",i+1);

        can->cd(1);
        hdy58=(TH2F*)f_in->Get(Form("hxpos_dy58_%d",i+1));
        hdy58_pfx=hdy58->ProfileX();
        fitrange[0]=xpedmean_dy5_pos[i]+pedcut*xpedsigma_dy5_pos[i];
        fitrange[1]=xpedmean_dy5_pos[i]+pedcut*xpedsigma_dy5_pos[i]+range;
        lffit=linear_fit(hdy58_pfx,fitrange);
        dy58_ratio=lffit->GetParameter(1);
        fprintf(fp,"%.3f\t",dy58_ratio);
        hdy58_dist->Fill(dy58_ratio);
        lffit->SetLineColor(kRed);
        hdy58->Draw();
        lffit->Draw("same");

        can->cd(2);
        hdy58=(TH2F*)f_in->Get(Form("hxneg_dy58_%d",i+1));
        hdy58_pfx=hdy58->ProfileX();
        fitrange[0]=xpedmean_dy5_neg[i]+pedcut*xpedsigma_dy5_neg[i];
        fitrange[1]=xpedmean_dy5_neg[i]+pedcut*xpedsigma_dy5_neg[i]+range;
        lffit=linear_fit(hdy58_pfx,fitrange);
        dy58_ratio=lffit->GetParameter(1);
        fprintf(fp,"%.3f\t",dy58_ratio);
        hdy58_dist->Fill(dy58_ratio);
        lffit->SetLineColor(kRed);
        hdy58->Draw();
        lffit->Draw("same");

        can->cd(3);
        hdy58=(TH2F*)f_in->Get(Form("hypos_dy58_%d",i+1));
        hdy58_pfx=hdy58->ProfileX();
        fitrange[0]=ypedmean_dy5_pos[i]+pedcut*ypedsigma_dy5_pos[i];
        fitrange[1]=ypedmean_dy5_pos[i]+pedcut*ypedsigma_dy5_pos[i]+range;
        lffit=linear_fit(hdy58_pfx,fitrange);
        dy58_ratio=lffit->GetParameter(1);
        fprintf(fp,"%.3f\t",dy58_ratio);
        hdy58_dist->Fill(dy58_ratio);
        lffit->SetLineColor(kRed);
        hdy58->Draw();
        lffit->Draw("same");

        can->cd(4);
        hdy58=(TH2F*)f_in->Get(Form("hyneg_dy58_%d",i+1));
        hdy58_pfx=hdy58->ProfileX();
        fitrange[0]=ypedmean_dy5_neg[i]+pedcut*ypedsigma_dy5_neg[i];
        fitrange[1]=ypedmean_dy5_neg[i]+pedcut*ypedsigma_dy5_neg[i]+range;
        lffit=linear_fit(hdy58_pfx,fitrange);
        dy58_ratio=lffit->GetParameter(1);
        fprintf(fp,"%.3f\n",dy58_ratio);
        hdy58_dist->Fill(dy58_ratio);
        lffit->SetLineColor(kRed);
        hdy58->Draw();
        lffit->Draw("same");

        can->Print(Form("%s.pdf",outfile));
    }
    can->Print(Form("%s.pdf]",outfile));

    hdy58_dist->Write(0,TObject::kOverwrite);
    fclose(fp);
    delete f_in;

    return 0;
}
