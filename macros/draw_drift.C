/*   $Id: draw_drift.h, 2015-02-01 15:49:48+08:00 MWDC_ana $
 *--------------------------------------------------------
 *  Author(s):
 *
 *--------------------------------------------------------
*/

#include "TTree.h"
#include "TFile.h"
#include "TROOT.h"
#include "TCanvas.h"
#include "TH1F.h"
#include "CrateInfo.h"
#include "BoardInfo.h"
#include "utility.h"
#include "TStyle.h"

int draw_drift(const char* datadir,const char* outfile)
{
  gStyle->SetOptStat(1111111);
  //
  TString label_location[2]={"Down","Up"};
  TString label_direction[3]={"X","Y","U"};

  //define histogram
  std::vector<TH1F*> histrepo_all[2];
  TH1* htemp;
  for(int i=0;i<2;i++){
    for(int j=0;j<3;j++){
      htemp=(TH1*)gROOT->FindObject("h"+label_direction[j]+"_"+label_location[i]+"_"+"drift_all");
      if(htemp)	{
	delete htemp;
      }
      histrepo_all[i].push_back(new TH1F("h"+label_direction[j]+"_"+label_location[i]+"_"+"drift_all",label_direction[j]+"_"+label_location[i]+"_"+"drift_all",1000,-1000.5,8999.5));
    }
  }
  std::vector<TH1F*> histrepo_single[2];
  for(int i=0;i<2;i++){
    for(int j=0;j<3;j++){
      htemp=(TH1*)gROOT->FindObject("h"+label_direction[j]+"_"+label_location[i]+"_"+"drift_single");
      if(htemp)	delete htemp;
      histrepo_single[i].push_back(new TH1F("h"+label_direction[j]+"_"+label_location[i]+"_"+"drift_single",label_direction[j]+"_"+label_location[i]+"_"+"drift_single",1000,-1000.5,8999.5));
    }
  }
  
  //
  TString file_data=TString(datadir)+"/"+outfile;  
  TFile* file_out=new TFile(file_data,"update");
  if(!file_out){
    printf("open file error: %s\n",outfile);
    exit(1);
  }
  //
  TTree *tree_mwdc,*tree_tof,*tree_multihit;
  file_out->GetObject("merge/mwdc",tree_mwdc);
  file_out->GetObject("merge/tof",tree_tof);
  file_out->GetObject("merge/mwdc_multihit",tree_multihit);
  
  ChannelMap *mwdc_leading=0,*mwdc_trailing=0;
  tree_mwdc->SetBranchAddress("leading_raw",&mwdc_leading);
  tree_mwdc->SetBranchAddress("trailing_raw",&mwdc_trailing);
  ChannelMap *tof_timeleading=0,*tof_timetrailing=0,*tof_totleading=0,*tof_tottrailing=0;
  tree_tof->SetBranchAddress("time_leading_raw",&tof_timeleading);
  tree_tof->SetBranchAddress("time_trailing_raw",&tof_timetrailing);
  tree_tof->SetBranchAddress("tot_leading_raw",&tof_totleading);
  tree_tof->SetBranchAddress("tot_trailing_raw",&tof_tottrailing);
  Int_t multihit[2][3];
  tree_multihit->SetBranchAddress("multihit",multihit);
  //
  int entries=tree_mwdc->GetEntriesFast();
  ChannelMap::iterator it,it_trigger;
  UChar_t type,location,direction;
  UShort_t index;
  //trigger time channel id:up/ch_1
  UInt_t start_time_id=Encoding::Encode(1,1,0,0);
  //for(int i=0;i<100;i++){
  for(int i=0;i<entries;i++){
    if(!((i+1)%5000)){
      printf("%d events analyzed\n",i+1);
    }
    tree_mwdc->GetEntry(i);
    tree_tof->GetEntry(i);
    tree_multihit->GetEntry(i);
    //
    for(it=mwdc_leading->begin();it!=mwdc_leading->end();it++){
      Encoding::Decode(it->first,type,location,direction,index);
      if (type!=EMWDC) {
	printf("event_%d:MWDC unmatched type\n",i+1);
      }
    }
    for(it=mwdc_trailing->begin();it!=mwdc_trailing->end();it++){
      Encoding::Decode(it->first,type,location,direction,index);
      if (type!=EMWDC) {
	printf("event_%d:MWDC unmatched type\n",i+1);
      }
      //
      it_trigger=tof_totleading->find(start_time_id);
      if(it_trigger!=tof_totleading->end()){
	histrepo_all[location][direction]->Fill((it->second)[0]-(it_trigger->second)[0]);
	if(multihit[location][direction]==1){
	  histrepo_single[location][direction]->Fill((it->second)[0]-(it_trigger->second)[0]);
	}
      }
    }
    //
    for(it=tof_timeleading->begin();it!=tof_timeleading->end();it++){
      Encoding::Decode(it->first,type,location,direction,index);
      if (type!=ETOF) {
	printf("event_%d:TOF unmatched type\n",i+1);
      }
    }
    for(it=tof_timetrailing->begin();it!=tof_timetrailing->end();it++){
      Encoding::Decode(it->first,type,location,direction,index);
      if (type!=ETOF) {
	printf("event_%d:TOF unmatched type\n",i+1);
      }
    }
    for(it=tof_totleading->begin();it!=tof_totleading->end();it++){
      Encoding::Decode(it->first,type,location,direction,index);
      if (type!=ETOF) {
	printf("event_%d:TOF unmatched type\n",i+1);
      }
    }
    for(it=tof_tottrailing->begin();it!=tof_tottrailing->end();it++){
      Encoding::Decode(it->first,type,location,direction,index);
      if (type!=ETOF) {
	printf("event_%d:TOF unmatched type\n",i+1);
      }
    }
  }
  
  printf("%d events processed totally\n",entries);
  //dir "histogram"
  TDirectory* dir_hist=file_out->GetDirectory("merge/histogram");
  if(!dir_hist){
    dir_hist=file_out->mkdir("merge/histogram");
    if(!dir_hist){
      printf("error!can't mkdir \"merge/hitogram\" in %s\n",outfile);
      exit(1);
    }
    dir_hist=file_out->GetDirectory("merge/histogram");
  }
  dir_hist->cd();
  //
  const Int_t nx=3,ny=2;
  TCanvas *can = (TCanvas*) gROOT->FindObject("can");
  if(can) delete can;
  can=new TCanvas("can","can",300*nx,300*ny);
  can->Divide(nx,ny);
  for(int i=0;i<ny;i++){
    for(int j=0;j<nx;j++){
      can->cd(nx*i+j+1);
      gPad->SetLogy();
      histrepo_all[i][j]->DrawCopy();
      histrepo_single[i][j]->DrawCopy("same")->SetLineColor(kRed);
      
      histrepo_all[i][j]->Write(0,TObject::kOverwrite);
      
    }
  }

  TCanvas *can2 = (TCanvas*) gROOT->FindObject("can2");
  if(can2) delete can2;
  can2=new TCanvas("can2","can2",300*nx,300*ny);
  can2->Divide(nx,ny);
  for(int i=0;i<ny;i++){
    for(int j=0;j<nx;j++){
      can2->cd(nx*i+j+1);
      gPad->SetLogy();
      histrepo_single[i][j]->DrawCopy();
      histrepo_single[i][j]->Write(0.TObject::kOverwrite);
    }
  }
  //
  delete file_out;
  
  return 0;
}

