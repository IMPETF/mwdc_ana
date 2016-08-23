/*   $Id: check_maxtot.h, 2016-07-18 14:46:24+08:00 MWDC_ana Merge$
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
#include "TMath.h"
#include "TStyle.h"
#include "TH2F.h"

int check_maxtot(const char* datadir,const char* outfile)
{
  gStyle->SetOptStat(111111);
  //
  std::map<UInt_t,TH1F*> h1d_tot_maxchannel_mwdc;
  for(int l=0;l<g_mwdc_location;l++){
    for(int p=0;p<g_mwdc_wireplane;p++){
        for(int w=0;w<g_mwdc_wireindex[p];w++){
            UInt_t gid=Encoding::Encode(EMWDC,l,p,w);
            h1d_tot_maxchannel_mwdc[gid]=new TH1F(Form("h1d_tot_maxchannel_mwdc_%s_%s_%d",g_str_location[l],g_str_plane[p],w+1),Form("h1d_tot_maxchannel_mwdc_%s_%s_%d",g_str_location[l],g_str_plane[p],w+1),3500,-45.5,3000.5);
          }
      }
  }
  std::map<UInt_t,TH1F*> h1d_tot_allhits_mwdc;
  for(int l=0;l<g_mwdc_location;l++){
    for(int p=0;p<g_mwdc_wireplane;p++){
        for(int w=0;w<g_mwdc_wireindex[p];w++){
            UInt_t gid=Encoding::Encode(EMWDC,l,p,w);
            h1d_tot_allhits_mwdc[gid]=new TH1F(Form("h1d_tot_allhits_mwdc_%s_%s_%d",g_str_location[l],g_str_plane[p],w+1),Form("h1d_tot_allhits_mwdc_%s_%s_%d",g_str_location[l],g_str_plane[p],w+1),3500,-40.5,3000.5);
          }
      }
  }
  std::map<UInt_t,TH1F*> h1d_drifttime_maxhit_mwdc;
  for(int l=0;l<g_mwdc_location;l++){
    for(int p=0;p<g_mwdc_wireplane;p++){
        for(int w=0;w<g_mwdc_wireindex[p];w++){
            UInt_t gid=Encoding::Encode(EMWDC,l,p,w);
            h1d_drifttime_maxhit_mwdc[gid]=new TH1F(Form("h1d_drifttime_maxhit_mwdc_%s_%s_%d",g_str_location[l],g_str_plane[p],w+1),Form("h1d_drifttime_maxhit_mwdc_%s_%s_%d",g_str_location[l],g_str_plane[p],w+1),500,-100.5,399.5);
          }
      }
  }
  std::map<UInt_t,TH1F*> h1d_drifttime_allhits_mwdc;
  for(int l=0;l<g_mwdc_location;l++){
    for(int p=0;p<g_mwdc_wireplane;p++){
        for(int w=0;w<g_mwdc_wireindex[p];w++){
            UInt_t gid=Encoding::Encode(EMWDC,l,p,w);
            h1d_drifttime_allhits_mwdc[gid]=new TH1F(Form("h1d_drifttime_allhits_mwdc_%s_%s_%d",g_str_location[l],g_str_plane[p],w+1),Form("h1d_drifttime_allhits_mwdc_%s_%s_%d",g_str_location[l],g_str_plane[p],w+1),500,-100.5,399.5);
          }
      }
  }
  std::map<UInt_t,TH1F*> h1d_drifttime_maxhits_correctedstarttiime_mwdc;
  for(int l=0;l<g_mwdc_location;l++){
    for(int p=0;p<g_mwdc_wireplane;p++){
        for(int w=0;w<g_mwdc_wireindex[p];w++){
            UInt_t gid=Encoding::Encode(EMWDC,l,p,w);
            h1d_drifttime_maxhits_correctedstarttiime_mwdc[gid]=new TH1F(Form("h1d_drifttime_maxhits_correctedstarttiime_mwdc_%s_%s_%d",g_str_location[l],g_str_plane[p],w+1),Form("h1d_drifttime_maxhits_correctedstarttiime_mwdc_%s_%s_%d",g_str_location[l],g_str_plane[p],w+1),500,-100.5,399.5);
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
  TTree *tree_mwdc,*tree_tof;
  file_out->GetObject("merge/mwdc",tree_mwdc);
  file_out->GetObject("merge/tof",tree_tof);
  
  ChannelMap *mwdc_leading=0,*mwdc_trailing=0;
  tree_mwdc->SetBranchAddress("leading_raw",&mwdc_leading);
  tree_mwdc->SetBranchAddress("trailing_raw",&mwdc_trailing);
  // Activate the referenced branches
  tree_mwdc->SetBranchStatus("*",0);
  tree_mwdc->SetBranchStatus("leading_raw",1);
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

  //
  int entries=tree_mwdc->GetEntriesFast();
  ChannelMap::iterator it,it_starttime,it_falling;
  UChar_t type,location,direction;
  UShort_t index;

  UInt_t starttime_gid=Encoding::Encode(ETOF,EDOWN,0,0);
  UInt_t invalid_gid=Encoding::Encode(0xFF,0,0,0);
  Double_t starttime_tag,starttime[g_tof_location];
  Double_t drift_time,max_drift_time[g_mwdc_location][g_mwdc_wireplane];
  Double_t drift_time_starttimecorrected,max_drift_time_starttimecorrected[g_mwdc_location][g_mwdc_wireplane];


  UInt_t maxtot_gid[g_mwdc_location][g_mwdc_wireplane];
  Double_t tot,maxtot[g_mwdc_location][g_mwdc_wireplane];//unit: ns
  Double_t tot_limit=2775;// if a channel has only rising edge and no falling edge, we think its tot is beyond the limit.
                             // thus assign this value as the tot value of this channel. It's kind of arbitrarily.

  Int_t  timetag[g_tof_location][g_tof_wireindex];
  //for(int i=0;i<100;i++){
  for(int i=0;i<entries;i++){
    if(!((i+1)%5000)){
      printf("%d events analyzed\n",i+1);
    }
    tree_mwdc->GetEntry(i);
    tree_tof->GetEntry(i);

    for(int l=0;l<g_mwdc_location;l++){
      for(int p=0;p<g_mwdc_wireplane;p++){
        maxtot[l][p]=-1;
        maxtot_gid[l][p]=invalid_gid;
      }
    }
    //
    if(tof_timeleading->size() == 8){
      it_starttime=tof_timeleading->find(starttime_gid);
      starttime_tag=it_starttime->second[0]*25./256/4;

      for(it=tof_timeleading->begin();it!=tof_timeleading->end();it++){
        Encoding::Decode(it->first,type,location,direction,index);
        timetag[location][index]=it->second[0];
      }
      for(int l=0;l<g_tof_location;l++){
        starttime[l]=Utility::calc_starttime(timetag[l]);
      }
      //
      for(it=mwdc_trailing->begin();it!=mwdc_trailing->end();it++){
        Encoding::Decode(it->first,type,location,direction,index);

        drift_time=it->second[0]*25./256 - starttime_tag;
        if(drift_time<-40){
          drift_time+=g_range_bunchid/2*25;
        }

        drift_time_starttimecorrected=it->second[0]*25./256 - starttime[location];
        if(drift_time_starttimecorrected<-40){
          drift_time_starttimecorrected+=g_range_bunchid/2.*25.;
        }
        //
        it_falling=mwdc_leading->find(it->first);
        if(it_falling != mwdc_leading->end()){
          tot = (it_falling->second[0] - it->second[0])*25./256;
          if(tot < 0){
            tot+=g_range_bunchid/2*25;
          }
        }
        else{
          tot = tot_limit;
        }
        //
        if(tot > maxtot[location][direction]){
          maxtot[location][direction]=tot;
          maxtot_gid[location][direction]=it->first;
          max_drift_time[location][direction]=drift_time;
          max_drift_time_starttimecorrected[location][direction]=drift_time_starttimecorrected;
        }
        //
        h1d_tot_allhits_mwdc[it->first]->Fill(tot);
        h1d_drifttime_allhits_mwdc[it->first]->Fill(drift_time);
      }
      //
      for(int l=0;l<g_mwdc_location;l++){
        for(int p=0;p<g_mwdc_wireplane;p++){
          if(Encoding::IsChannelValid(maxtot_gid[l][p])){
            h1d_tot_maxchannel_mwdc[maxtot_gid[l][p]]->Fill(maxtot[l][p]);
            h1d_drifttime_maxhit_mwdc[maxtot_gid[l][p]]->Fill(max_drift_time[l][p]);
            h1d_drifttime_maxhits_correctedstarttiime_mwdc[maxtot_gid[l][p]]->Fill(max_drift_time_starttimecorrected[l][p]);
          }
        }
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

  TDirectory* dir_maxtot=dir_hist->GetDirectory("check_maxtot");
  if(!dir_maxtot){
    dir_maxtot=dir_hist->mkdir("check_maxtot");
    if(!dir_maxtot){
      printf("error!can't mkdir \"raw/hitogram/check_maxtot\" in %s\n",outfile);
      exit(1);
    }
    dir_maxtot=dir_hist->GetDirectory("check_maxtot");
  }
  dir_maxtot->cd();
  //
  for(int l=0;l<g_mwdc_location;l++){
    for(int p=0;p<g_mwdc_wireplane;p++){
        for(int w=0;w<g_mwdc_wireindex[p];w++){
            UInt_t gid=Encoding::Encode(EMWDC,l,p,w);
            h1d_tot_maxchannel_mwdc[gid]->Write(0,TObject::kOverwrite);
            h1d_tot_allhits_mwdc[gid]->Write(0,TObject::kOverwrite);
            h1d_drifttime_allhits_mwdc[gid]->Write(0,TObject::kOverwrite);
            h1d_drifttime_maxhit_mwdc[gid]->Write(0,TObject::kOverwrite);
            h1d_drifttime_maxhits_correctedstarttiime_mwdc[gid]->Write(0,TObject::kOverwrite);
          }
      }
  }
  //
  UInt_t test_gid=Encoding::Encode(EMWDC,EUP,EX,40);
  TCanvas* c1=new TCanvas("cmwdc_tot_all","cmwdc_tot_all");
  h1d_tot_allhits_mwdc[test_gid]->Draw();
  TCanvas* c2=new TCanvas("cmwdc_tot_max","cmwdc_tot_max");
  h1d_tot_maxchannel_mwdc[test_gid]->Draw();
  TCanvas* c3=new TCanvas("cmwdc_drifttime_all","cmwdc_drifttime_all");
  h1d_drifttime_allhits_mwdc[test_gid]->Draw();
  TCanvas* c4=new TCanvas("cmwdc_drifttime_max","cmwdc_drifttime_max");
  h1d_drifttime_maxhit_mwdc[test_gid]->Draw();
  h1d_drifttime_maxhits_correctedstarttiime_mwdc[test_gid]->SetLineColor(kRed);
  h1d_drifttime_maxhits_correctedstarttiime_mwdc[test_gid]->Draw("same");
  //
  delete file_out;
  
  return 0;
}

