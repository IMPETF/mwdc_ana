#include <vector>
#include "TFile.h"
#include "TH1F.h"
#include "TCanvas.h"
#include "TStyle.h"
#include "TROOT.h"

int compare_drift(const char* file1,const char* file2)
{
  gStyle->SetOptStat(1111111);
  //
  TString label_location[2]={"Down","Up"};
  TString label_direction[3]={"X","Y","U"};
  //
  TFile *f1=new TFile(file1);
  TFile *f2=new TFile(file2);
  
  std::vector<TH1F*> hcompare1[2];
  std::vector<TH1F*> hcompare2[2];
  
  TH1F* htemp;
  for(int i=0;i<2;i++){
    for(int j=0;j<3;j++){
      htemp=(TH1F*)f1->Get("merge/histogram/h"+label_direction[j]+"_"+label_location[i]+"_"+"drift_all");
      hcompare1[i].push_back(htemp);
      htemp=(TH1F*)f2->Get("merge/histogram/h"+label_direction[j]+"_"+label_location[i]+"_"+"drift_all");
      hcompare2[i].push_back(htemp);
      hcompare2[i][j]->SetLineColor(kRed);
    }
  }
  Int_t nx=3,ny=2;
  TCanvas *can1 = (TCanvas*) gROOT->FindObject("can1");
  if(can1) delete can1;
  can1=new TCanvas("can1","can1",300*nx,300*ny);
  can1->Divide(nx,ny);
  for(int i=0;i<ny;i++){
    for(int j=0;j<nx;j++){
      can1->cd(nx*i+j+1);
      hcompare1[i][j]->Scale(10./hcompare1[i][j]->Integral());
      hcompare1[i][j]->DrawCopy();
      hcompare2[i][j]->Scale(10./hcompare2[i][j]->Integral());
      hcompare2[i][j]->DrawCopy("same");
    }
  }
  
  delete f1;
  delete f2;
  
  return 0;
}