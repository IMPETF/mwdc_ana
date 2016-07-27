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

int extract_minrising_drifttime(const char* datadir,const char* outfile)
{
  gStyle->SetOptStat(111111);

  std::map<UInt_t,TH1F*> h1d_minrising_drifttime_mwdc;
  for(int l=0;l<g_mwdc_location;l++){
    for(int p=0;p<g_mwdc_wireplane;p++){
        for(int w=0;w<g_mwdc_wireindex[p];w++){
            UInt_t gid=Encoding::Encode(EMWDC,l,p,w);
            h1d_minrising_drifttime_mwdc[gid]=new TH1F(Form("h1d_minrising_drifttime_mwdc_%s_%s_%d",g_str_location[l],g_str_plane[p],w+1),Form("h1d_minrising_drifttime_mwdc_%s_%s_%d",g_str_location[l],g_str_plane[p],w+1),600,-100.5,499.5);
          }
      }
  }
  std::map<UInt_t,TH1F*> h1d_maxtot_drifttime_mwdc;
  for(int l=0;l<g_mwdc_location;l++){
    for(int p=0;p<g_mwdc_wireplane;p++){
        for(int w=0;w<g_mwdc_wireindex[p];w++){
            UInt_t gid=Encoding::Encode(EMWDC,l,p,w);
            h1d_maxtot_drifttime_mwdc[gid]=new TH1F(Form("h1d_maxtot_drifttime_mwdc_%s_%s_%d",g_str_location[l],g_str_plane[p],w+1),Form("h1d_maxtot_drifttime_mwdc_%s_%s_%d",g_str_location[l],g_str_plane[p],w+1),600,-100.5,499.5);
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
  ChannelMap::iterator it,it_falling;
  UChar_t type,location,direction;
  UShort_t index;

  UInt_t starttime_gid=Encoding::Encode(ETOF,EDOWN,0,0);
  UInt_t invalid_gid=Encoding::Encode(0xFF,0,0,0);
  Double_t starttime[g_tof_location];
  Int_t  timetag[g_tof_location][g_tof_wireindex];
  Double_t drift_time,maxtot_drifttime[g_mwdc_location][g_mwdc_wireplane];

  UInt_t maxtot_gid[g_mwdc_location][g_mwdc_wireplane];
  Double_t tot,maxtot[g_mwdc_location][g_mwdc_wireplane];//unit: ns
  Double_t tot_limit=2775;// if a channel has only rising edge and no falling edge, we think its tot is beyond the limit.
  Double_t drifttime_limt=3000;
                             // thus assign this value as the tot value of this channel. It's kind of arbitrarily.
  UInt_t minrising_gid[g_mwdc_location][g_mwdc_wireplane];
  Double_t minrising[g_mwdc_location][g_mwdc_wireplane],minrising_tot[g_mwdc_location][g_mwdc_wireplane];
  
  // outfile
  TString drifttime_suffix(outfile);
  drifttime_suffix.Remove(0,4);
  TFile* file_drifttime=new TFile(Form("%s/drifttime%s",datadir,drifttime_suffix.Data()),"recreate");

  TDirectory* dir_maxtot=file_drifttime->GetDirectory("maxtot_drifttime");
  if(!dir_maxtot){
    dir_maxtot=file_drifttime->mkdir("maxtot_drifttime");
    if(!dir_maxtot){
      printf("error!can't mkdir \"maxtot_drifttime\" in drifttime%s\n",drifttime_suffix.Data());
      exit(1);
    }
    dir_maxtot=file_drifttime->GetDirectory("maxtot_drifttime");
  }
  dir_maxtot->cd();
  TTree* tree_maxtot_drifttime=new TTree("maxtot_drifttime","maxtot_drifttime");
  tree_maxtot_drifttime->Branch("gid",maxtot_gid,"gid[2][3]/i");
  tree_maxtot_drifttime->Branch("tot",maxtot,"tot[2][3]/D");
  tree_maxtot_drifttime->Branch("drifttime",maxtot_drifttime,"drifttime[2][3]/D");

  TDirectory* dir_minrising=file_drifttime->GetDirectory("minrising_drifttime");
  if(!dir_minrising){
    dir_minrising=file_drifttime->mkdir("minrising_drifttime");
    if(!dir_minrising){
      printf("error!can't mkdir \"minrising_drifttime\" in drifttime%s\n",drifttime_suffix.Data());
      exit(1);
    }
    dir_minrising=file_drifttime->GetDirectory("minrising_drifttime");
  }
  dir_minrising->cd();
  TTree* tree_minrising_drifttime=new TTree("minrising_drifttime","minrising_drifttime");
  tree_minrising_drifttime->Branch("gid",minrising_gid,"gid[2][3]/i");
  tree_minrising_drifttime->Branch("tot",minrising_tot,"tot[2][3]/D");
  tree_minrising_drifttime->Branch("drifttime",minrising,"drifttime[2][3]/D");

  // for(int i=0;i<100;i++){
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
        maxtot_drifttime[l][p]=drifttime_limt;
        
        minrising[l][p]=drifttime_limt;// a large value limit
        minrising_tot[l][p]=tot_limit;
        minrising_gid[l][p]=invalid_gid;
      }
    }
    //
    if(tof_timeleading->size() == 8){
      // get trigger time
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
        // drift time
        drift_time=it->second[0]*25./256 - starttime[location];
        if(drift_time<-40){
          drift_time+=g_range_bunchid/2*25;
          if(drift_time< -1000000){
            printf("starttime=%.2f, drifttime=%.2f: timetag=%d,%d,%d,%d\n",starttime[location],drift_time,timetag[location][0],timetag[location][1],timetag[location][2],timetag[location][3]);
          }
        }
        // 
        it_falling=mwdc_leading->find(it->first);
        if(it_falling != mwdc_leading->end()){
          tot = (it_falling->second[0] - it->second[0])*25./256;
          if(tot < 0){
            tot+=g_range_bunchid/2*25;
          }
          if(tot>40000){
            // printf("tot=%.2f, leading=%d, falling=%d\n",tot,it->second[0],it_falling->second[0]);
            for(int fallingid=1;fallingid<it_falling->second.size();fallingid++){
              tot = (it_falling->second[fallingid] - it->second[0])*25./256;
              if(tot < 0){
                tot+=g_range_bunchid/2*25;
              }
              if(tot>40000)
                continue;
              else
                break;
            }
          }
          if(tot>40000){
            tot = tot_limit;
          }
        }
        else{
          tot = tot_limit;
        }
        //
        if(tot > maxtot[location][direction]){
          maxtot[location][direction]=tot;
          maxtot_gid[location][direction]=it->first;
          maxtot_drifttime[location][direction]=drift_time;
        }

        //
        if(drift_time < minrising[location][direction]){
          minrising[location][direction]=drift_time;
          minrising_gid[location][direction]=it->first;
          minrising_tot[location][direction]=tot;
        }
      }
      //
      for(int l=0;l<g_mwdc_location;l++){
        for(int p=0;p<g_mwdc_wireplane;p++){
          if(Encoding::IsChannelValid(maxtot_gid[l][p])){
            h1d_maxtot_drifttime_mwdc[maxtot_gid[l][p]]->Fill(maxtot_drifttime[l][p]);
          }
          if(Encoding::IsChannelValid(minrising_gid[l][p])){
            h1d_minrising_drifttime_mwdc[minrising_gid[l][p]]->Fill(minrising[l][p]);
          }
        }
      }
      //
      tree_maxtot_drifttime->Fill();
      tree_minrising_drifttime->Fill();
    }
  }
  
  printf("%d events processed totally\n",entries);

  //dir "histogram"
  TDirectory* dir_maxtot_hist=dir_maxtot->GetDirectory("histogram");
  if(!dir_maxtot_hist){
    dir_maxtot_hist=dir_maxtot->mkdir("histogram");
    if(!dir_maxtot_hist){
      printf("error!can't mkdir \"maxtot_drifttime/histogram\" in drifttime%s\n",drifttime_suffix.Data());
      exit(1);
    }
    dir_maxtot_hist=dir_maxtot->GetDirectory("histogram");
  }
  dir_maxtot_hist->cd();
  for(int l=0;l<g_mwdc_location;l++){
    for(int p=0;p<g_mwdc_wireplane;p++){
        for(int w=0;w<g_mwdc_wireindex[p];w++){
            UInt_t gid=Encoding::Encode(EMWDC,l,p,w);
            h1d_maxtot_drifttime_mwdc[gid]->Write(0,TObject::kOverwrite);
          }
      }
  }

  TDirectory* dir_minrising_hist=dir_minrising->GetDirectory("histogram");
  if(!dir_minrising_hist){
    dir_minrising_hist=dir_minrising->mkdir("histogram");
    if(!dir_minrising_hist){
      printf("error!can't mkdir \"minrising_drifttime/histogram\" in drifttime%s\n",drifttime_suffix.Data());
      exit(1);
    }
    dir_minrising_hist=dir_minrising->GetDirectory("histogram");
  }
  dir_minrising_hist->cd();
  for(int l=0;l<g_mwdc_location;l++){
    for(int p=0;p<g_mwdc_wireplane;p++){
        for(int w=0;w<g_mwdc_wireindex[p];w++){
            UInt_t gid=Encoding::Encode(EMWDC,l,p,w);
            h1d_minrising_drifttime_mwdc[gid]->Write(0,TObject::kOverwrite);
          }
      }
  }

  //
  dir_maxtot->cd();
  tree_maxtot_drifttime->Write(0,TObject::kOverwrite);
  dir_minrising->cd();
  tree_minrising_drifttime->Write(0,TObject::kOverwrite);
  
  //
  //
  
  UInt_t test_gid=Encoding::Encode(EMWDC,EUP,EX,40);
  TCanvas* c1=new TCanvas("cmaxtot_drifttime","cmaxtot_drifttime");
  c1->SetLogy();
  h1d_maxtot_drifttime_mwdc[test_gid]->Draw();
  h1d_minrising_drifttime_mwdc[test_gid]->SetLineColor(kRed);
  h1d_minrising_drifttime_mwdc[test_gid]->Draw("same");
  TCanvas* c2=new TCanvas("cminrising_drifttime","cminrising_drifttime");
  c2->SetLogy();
  h1d_minrising_drifttime_mwdc[test_gid]->Draw();

  //
  delete file_out;
  
  return 0;
}

