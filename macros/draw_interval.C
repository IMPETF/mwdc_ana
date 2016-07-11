/*   $Id: draw_interval.h, 2016-07-10 15:42:36+08:00 MWDC_ana Merge$
 *--------------------------------------------------------
 *  Author(s):
 *
 *--------------------------------------------------------
*/

#include "TTree.h"
#include "TFile.h"
#include "TROOT.h"
#include "TCanvas.h"
#include "TH1D.h"
#include "CrateInfo.h"
#include "BoardInfo.h"
#include "utility.h"
#include "TH2D.h"

int draw_interval(const char* datadir,const char* outfile)
{
  //
  TString label_location[2]={"Down","Up"};
  TString label_direction[3]={"X","Y","U"};
/*
  //define histogram
  std::vector<TH1F*> histrepo[2];
  TH1* htemp;
  for(int i=0;i<2;i++){
    for(int j=0;j<3;j++){
      htemp=(TH1*)gROOT->FindObject("h"+label_direction[j]+"_"+label_location[i]+"_"+"multihit");
      if(htemp)	{
	delete htemp;
      }
      histrepo[i].push_back(new TH1F("h"+label_direction[j]+"_"+label_location[i]+"_"+"multihit",label_direction[j]+"_"+label_location[i]+"_"+"multihit",11,-0.5,10.5));
    }
  }
*/
  //
  TString file_data=TString(datadir)+"/"+outfile;  
  TFile* file_out=new TFile(file_data,"update");
  if(!file_out){
    printf("open file error: %s\n",outfile);
    exit(1);
  }
  //
  TTree *tree_mwdc,*tree_tof;
  file_out->GetObject("merge/mwdc",tree_mwdc);
  file_out->GetObject("merge/tof",tree_tof);
  
  ChannelMap *mwdc_leading=0,*mwdc_trailing=0;
  tree_mwdc->SetBranchAddress("leading_raw",&mwdc_leading);
  tree_mwdc->SetBranchAddress("trailing_raw",&mwdc_trailing);
  // Activate the referenced branches
  tree_mwdc->SetBranchStatus("*",0);
  //tree_mwdc->SetBranchStatus("leading_raw",1);
  //tree_mwdc->SetBranchStatus("trailing_raw",1);

  ChannelMap *tof_timeleading=0,*tof_timetrailing=0,*tof_totleading=0,*tof_tottrailing=0;
  tree_tof->SetBranchAddress("time_leading_raw",&tof_timeleading);
  tree_tof->SetBranchAddress("time_trailing_raw",&tof_timetrailing);
  tree_tof->SetBranchAddress("tot_leading_raw",&tof_totleading);
  tree_tof->SetBranchAddress("tot_trailing_raw",&tof_tottrailing);
  // Activate the referenced branches
  tree_tof->SetBranchStatus("*",0);
  tree_tof->SetBranchStatus("time_leading_raw",1);
  //tree_tof->SetBranchStatus("time_trailing_raw",1);
  //tree_tof->SetBranchStatus("tot_leading_raw",1);
  //tree_tof->SetBranchStatus("tot_trailing_raw",1);

  // make new histogram here
  TH1D* histrepo_tof[g_tof_location][g_tof_wireindex];
  for(int i=0;i<g_tof_location;i++){
    for(int j=0;j<g_tof_wireindex;j++){
      histrepo_tof[i][j]=new TH1D(Form("h1tof_%s_%s_%d",TString("").Data(),g_str_location[i],j+1),Form("h1tof_%s_%s_%d",TString("").Data(),g_str_location[i],j+1),2*g_range_highprecision-1,-g_range_highprecision+0.5,g_range_highprecision-0.5);
      histrepo_tof[i][j]->SetDirectory(0);
    }
  }
  TH2D* histrepo2d_tof[g_tof_location][g_tof_wireindex];
  for(int i=0;i<g_tof_location;i++){
    for(int j=0;j<g_tof_wireindex;j++){
      histrepo2d_tof[i][j]=new TH2D(Form("h2tof_%s_%s_%d",TString("").Data(),g_str_location[i],j+1),Form("h2tof_%s_%s_%d",TString("").Data(),g_str_location[i],j+1),1000,-0.5,g_range_highprecision-0.5,1000,-0.5,g_range_highprecision-0.5);
      histrepo2d_tof[i][j]->SetDirectory(0);
    }
  }
  //
  int entries=tree_mwdc->GetEntriesFast();
  ChannelMap::iterator it;
  UChar_t type,location,direction;
  UShort_t index;
  UInt_t gid;
  int previous_time[g_tof_location][g_tof_wireindex];

  // tree_mwdc->GetEntry(i);
  tree_tof->GetEntry(0);
  for(int i=0;i<g_tof_location;i++){
      for(int j=0;j<g_tof_wireindex;j++){
        gid=Encoding::Encode(ETOF,i,0,j);
        it=tof_timeleading->find(gid);
        if(it!=tof_timeleading->end()){
          previous_time[i][j]=it->second.at(0);
        }
        else{
          printf("ERROR: TOF_%s_%d no signal\n", g_str_location[g_tof_location],j+1);
          exit(1);
        }
      }
  }
  // for(int i=1;i<10000;i++){
  for(int i=1;i<entries;i++){
    if(!((i+1)%5000)){
      printf("%d events analyzed\n",i+1);
    }
    tree_mwdc->GetEntry(i);
    tree_tof->GetEntry(i);
    //
    for(int i=0;i<g_tof_location;i++){
      for(int j=0;j<g_tof_wireindex;j++){
        gid=Encoding::Encode(ETOF,i,0,j);
        it=tof_timeleading->find(gid);
        if(it!=tof_timeleading->end()){
          if(it->second.at(0) >= previous_time[i][j]){
            histrepo_tof[i][j]->Fill(it->second.at(0) - previous_time[i][j]);
          }
          else{
            histrepo_tof[i][j]->Fill(it->second.at(0) - previous_time[i][j] + g_range_highprecision);
          }
          //
          histrepo2d_tof[i][j]->Fill(it->second.at(0),previous_time[i][j]);
          //
          previous_time[i][j]=it->second.at(0);
        }
      }
    }

 //    for(it=mwdc_leading->begin();it!=mwdc_leading->end();it++){
 //      Encoding::Decode(it->first,type,location,direction,index);
 //      if (type!=EMWDC) {
	// printf("event_%d:MWDC unmatched type\n",i+1);
 //      }
 //    }
 //    for(it=mwdc_trailing->begin();it!=mwdc_trailing->end();it++){
 //      Encoding::Decode(it->first,type,location,direction,index);
 //      if (type!=EMWDC) {
	// printf("event_%d:MWDC unmatched type\n",i+1);
 //      }
 //    }
 //    //
 //    for(it=tof_timeleading->begin();it!=tof_timeleading->end();it++){
 //      Encoding::Decode(it->first,type,location,direction,index);
 //      if (type!=ETOF) {
	// printf("event_%d:TOF unmatched type\n",i+1);
 //      }
 //    }
 //    for(it=tof_timetrailing->begin();it!=tof_timetrailing->end();it++){
 //      Encoding::Decode(it->first,type,location,direction,index);
 //      if (type!=ETOF) {
	// printf("event_%d:TOF unmatched type\n",i+1);
 //      }
 //    }
 //    for(it=tof_totleading->begin();it!=tof_totleading->end();it++){
 //      Encoding::Decode(it->first,type,location,direction,index);
 //      if (type!=ETOF) {
	// printf("event_%d:TOF unmatched type\n",i+1);
 //      }
 //    }
 //    for(it=tof_tottrailing->begin();it!=tof_tottrailing->end();it++){
 //      Encoding::Decode(it->first,type,location,direction,index);
 //      if (type!=ETOF) {
	// printf("event_%d:TOF unmatched type\n",i+1);
 //      }
 //    }
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
  for(int i=0;i<g_tof_location;i++){
    for(int j=0;j<g_tof_wireindex;j++){
      TCanvas *can = (TCanvas*) gROOT->FindObject(Form("cantof_%d",i*g_tof_wireindex+j));
      if(can) delete can;
      can=new TCanvas(Form("cantof_%d",i*g_tof_wireindex+j),Form("cantof_%d",i*g_tof_wireindex+j),500,500);

      histrepo_tof[i][j]->Draw();
    }
  }

  for(int i=0;i<g_tof_location;i++){
    for(int j=0;j<g_tof_wireindex;j++){
      TCanvas *can = (TCanvas*) gROOT->FindObject(Form("cantof2d_%d",i*g_tof_wireindex+j));
      if(can) delete can;
      can=new TCanvas(Form("cantof2d_%d",i*g_tof_wireindex+j),Form("cantof2d_%d",i*g_tof_wireindex+j),500,500);

      histrepo2d_tof[i][j]->Draw();
    }
  }
  //
  delete file_out;
  
  return 0;
}

