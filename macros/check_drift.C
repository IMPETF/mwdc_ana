/*   $Id: check_drift.h, 2016-07-16 20:08:30+08:00 MWDC_ana Merge$
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

int check_drift(const char* datadir,const char* outfile)
{
  gStyle->SetOptStat(111111);
  // make histogram here
  std::map<UInt_t,TH1F*> h1d_drift_mwdc;
  for(int l=0;l<g_mwdc_location;l++){
    for(int p=0;p<g_mwdc_wireplane;p++){
      for(int w=0;w<g_mwdc_wireindex[p];w++){
        UInt_t gid=Encoding::Encode(EMWDC,l,p,w);
        h1d_drift_mwdc[gid]=new TH1F(Form("h1d_drift_mwdc_%s_%s_%d",g_str_location[l],g_str_plane[p],w+1),Form("h1d_drift_mwdc_%s_%s_%d",g_str_location[l],g_str_plane[p],w+1),1600,-20000,300000);
      }
    }
  }
  std::map<UInt_t,TH1F*> h1d_driftsingle_mwdc;
  for(int l=0;l<g_mwdc_location;l++){
    for(int p=0;p<g_mwdc_wireplane;p++){
        for(int w=0;w<g_mwdc_wireindex[p];w++){
            UInt_t gid=Encoding::Encode(EMWDC,l,p,w);
            h1d_driftsingle_mwdc[gid]=new TH1F(Form("h1d_driftsingle_mwdc_%s_%s_%d",g_str_location[l],g_str_plane[p],w+1),Form("h1d_driftsingle_mwdc_%s_%s_%d",g_str_location[l],g_str_plane[p],w+1),1600,-20000,300000);
          }
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
  // Activate the referenced branches
  tree_mwdc->SetBranchStatus("*",0);
  // tree_mwdc->SetBranchStatus("leading_raw",1);
  tree_mwdc->SetBranchStatus("trailing_raw",1);

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
  Int_t multihit[2][3];
  tree_multihit->SetBranchAddress("multihit",multihit);
  //
  int entries=tree_mwdc->GetEntriesFast();
  ChannelMap::iterator it,it_starttime;
  UChar_t type,location,direction;
  UShort_t index;
  //
  UInt_t starttime_gid=Encoding::Encode(ETOF,EDOWN,0,0);
  Double_t starttime_tag,drift_time;
  // for(int i=0;i<100;i++){
  for(int i=0;i<entries;i++){
    if(!((i+1)%5000)){
      printf("%d events analyzed\n",i+1);
    }
    tree_mwdc->GetEntry(i);
    tree_tof->GetEntry(i);
    tree_multihit->GetEntry(i);
    //
    // for(it=mwdc_leading->begin();it!=mwdc_leading->end();it++){
    //   Encoding::Decode(it->first,type,location,direction,index);
    //   if (type!=EMWDC) {
	   //     printf("event_%d:MWDC unmatched type\n",i+1);
    //   }
    // }
    it_starttime=tof_timeleading->find(starttime_gid);
    if(it_starttime!=tof_timeleading->end()){
      starttime_tag=it_starttime->second[0]*g_timeunit_highprecision;
      //
      for(it=mwdc_trailing->begin();it!=mwdc_trailing->end();it++){
        drift_time=it->second[0]*g_timeunit_precision - starttime_tag;
        if(drift_time<-30000){
          drift_time+=g_range_highprecision*g_timeunit_highprecision;
        }
        //
        h1d_drift_mwdc[it->first]->Fill(drift_time);
        //
        Encoding::Decode(it->first,type,location,direction,index);
        if(multihit[location][direction]==1){
          h1d_driftsingle_mwdc[it->first]->Fill(drift_time);
        }
      }
    }
    
    // //
    // for(it=tof_timeleading->begin();it!=tof_timeleading->end();it++){
    //   Encoding::Decode(it->first,type,location,direction,index);
    //   if (type!=ETOF) {
	   //     printf("event_%d:TOF unmatched type\n",i+1);
    //   }
    // }
    // for(it=tof_timetrailing->begin();it!=tof_timetrailing->end();it++){
    //   Encoding::Decode(it->first,type,location,direction,index);
    //   if (type!=ETOF) {
	   //     printf("event_%d:TOF unmatched type\n",i+1);
    //   }
    // }
    // for(it=tof_totleading->begin();it!=tof_totleading->end();it++){
    //   Encoding::Decode(it->first,type,location,direction,index);
    //   if (type!=ETOF) {
	   //     printf("event_%d:TOF unmatched type\n",i+1);
    //   }
    // }
    // for(it=tof_tottrailing->begin();it!=tof_tottrailing->end();it++){
    //   Encoding::Decode(it->first,type,location,direction,index);
    //   if (type!=ETOF) {
	   //     printf("event_%d:TOF unmatched type\n",i+1);
    //   }
    // }
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

  TDirectory* dir_drift=dir_hist->GetDirectory("test_drift");
  if(!dir_drift){
    dir_drift=dir_hist->mkdir("test_drift");
    if(!dir_drift){
      printf("error!can't mkdir \"merge/hitogram/test_drift\" in %s\n",outfile);
      exit(1);
    }
    dir_drift=dir_hist->GetDirectory("test_drift");
  }
  dir_drift->cd();
  //
  TCanvas* c1=new TCanvas();
  c1->Print(Form("%s/drift_time.pdf[",datadir));
  for(int l=0;l<g_mwdc_location;l++){
    for(int p=0;p<g_mwdc_wireplane;p++){
      for(int w=0;w<g_mwdc_wireindex[p];w++){
        UInt_t gid=Encoding::Encode(EMWDC,l,p,w);
        h1d_drift_mwdc[gid]->Write(0,TObject::kOverwrite);
        //
        h1d_drift_mwdc[gid]->Draw();
        c1->Print(Form("%s/drift_time.pdf",datadir));
      }
    }
  }
  c1->Print(Form("%s/drift_time.pdf]",datadir));

  TCanvas* c2=new TCanvas();
  c2->Print(Form("%s/drift_time_single.pdf[",datadir));
  for(int l=0;l<g_mwdc_location;l++){
    for(int p=0;p<g_mwdc_wireplane;p++){
      for(int w=0;w<g_mwdc_wireindex[p];w++){
        UInt_t gid=Encoding::Encode(EMWDC,l,p,w);
        h1d_driftsingle_mwdc[gid]->Write(0,TObject::kOverwrite);
        //
        h1d_driftsingle_mwdc[gid]->Draw();
        c2->Print(Form("%s/drift_time_single.pdf",datadir));
      }
    }
  }
  c2->Print(Form("%s/drift_time_single.pdf]",datadir));
  
  delete file_out;
  
  return 0;
}

