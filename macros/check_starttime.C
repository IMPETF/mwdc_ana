/*   $Id: check_toftime.h, 2016-07-18 22:23:21+08:00 MWDC_ana Merge$
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
#include "TH2F.h"
#include "TStyle.h"
#include "TMath.h"

// Double_t calc_starttime(Int_t* timetag,Double_t v_eff=15)
// {
//   Double_t a=TMath::Sqrt(2)/2*82.5;//half diagonal length
//   // Double_t v_eff=15;//light effective velocity 15 cm/ns
//   Double_t unit=25./256/4;// hptdc least bin
//   Double_t starttime,tmptime[4];
//   for(int i=0;i<4;i++){
//     tmptime[i]=timetag[i]*unit;
//   }

//   starttime = TMath::Mean(4,tmptime) + v_eff/16/a*(TMath::Power(tmptime[0]-tmptime[2],2)+TMath::Power(tmptime[1]-tmptime[3],2));
  
//   return starttime;
// }

int check_starttime(const char* datadir,const char* outfile)
{
  gStyle->SetOptStat(111111);
  //
  std::map<UInt_t,TH1F*> h1d_timetag_tof;
  for(int l=0;l<g_tof_location;l++){
       for(int w=0;w<g_tof_wireindex;w++){
           UInt_t gid=Encoding::Encode(ETOF,l,0,w);
           h1d_timetag_tof[gid]=new TH1F(Form("h1d_timetag_tof_%s_%d",g_str_location[l],w+1),Form("h1d_timetag_tof_%s_%d",g_str_location[l],w+1),300,100,400);
      }
  }
  
  std::map<UInt_t,TH1F*> h1d_timetagt0_tof_15cm,h1d_timetagt0_tof_8cm;
  for(int l=0;l<g_tof_location;l++){
       for(int w=0;w<g_tof_wireindex;w++){
           UInt_t gid=Encoding::Encode(ETOF,l,0,w);
           h1d_timetagt0_tof_15cm[gid]=new TH1F(Form("h1d_timetagt0_tof_15cm_%s_%d",g_str_location[l],w+1),Form("h1d_timetagt0_tof_15cm_%s_%d",g_str_location[l],w+1),500,-40,10);
           h1d_timetagt0_tof_8cm[gid]=new TH1F(Form("h1d_timetagt0_tof_8cm_%s_%d",g_str_location[l],w+1),Form("h1d_timetagt0_tof_8cm_%s_%d",g_str_location[l],w+1),500,-40,10);
      }
  }

  TH1F* h1d_hitnum_tof=new TH1F("h1d_hitnum_tof","h1d_hitnum_tof",13,-0.5,12.5);
  TH1F* h1d_hitrate_tof=new TH1F("h1d_hitrate_tof","h1d_hitrate_tof",13,-0.5,12.5);
  
  std::vector<TH1F*> h1d_starttime_tof,h1d_starttime_tof_8cm;
  std::vector<TH1F*> h1d_test_tof;
  for(int l=0;l<g_tof_location;l++){
    h1d_starttime_tof.push_back(new TH1F(Form("h1d_starttime_tof_%s",g_str_location[l]),Form("h1d_starttime_tof_%s",g_str_location[l]),300,100,400));
    h1d_starttime_tof[l]->SetDirectory(0);

    h1d_starttime_tof_8cm.push_back(new TH1F(Form("h1d_starttime_tof_8cm_%s",g_str_location[l]),Form("h1d_starttime_tof_8cm_%s",g_str_location[l]),300,100,400));
    h1d_starttime_tof_8cm[l]->SetDirectory(0);

    h1d_test_tof.push_back(new TH1F(Form("h1d_test_tof_%s",g_str_location[l]),Form("h1d_test_tof_%s",g_str_location[l]),2*g_range_highprecision-1,-g_range_highprecision+0.5,g_range_highprecision-0.5));
  }

  TH1F* h1d_startimediff_tof=new TH1F("h1d_starttimediff_tof","h1d_starttimediff_tof",300,-10,20);
  //
  TString file_data=TString(datadir)+"/"+outfile;  
  TFile* file_out=new TFile(file_data,"update");
  if(!file_out){
    printf("open file error: %s\n",outfile);
    exit(1);
  }
  //
  TTree *tree_mwdc,*tree_tof,*tree_bunchid;
  file_out->GetObject("merge/mwdc",tree_mwdc);
  file_out->GetObject("merge/tof",tree_tof);
  file_out->GetObject("raw/tof2",tree_bunchid);
  // tree_tof->AddFriend(tree_mwdc);
  // tree_tof->AddFriend(tree_bunchid);

  ChannelMap *mwdc_leading=0,*mwdc_trailing=0;
  tree_mwdc->SetBranchAddress("leading_raw",&mwdc_leading);
  tree_mwdc->SetBranchAddress("trailing_raw",&mwdc_trailing);
  // Activate the referenced branches
  tree_mwdc->SetBranchStatus("*",0);
  // tree_mwdc->SetBranchStatus("leading_raw",1);
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

  Int_t bunchid;
  tree_bunchid->SetBranchAddress("bunch_id",&bunchid);
  //
  int entries=tree_tof->GetEntriesFast();
  ChannelMap::iterator it;
  UChar_t type,location,direction;
  UShort_t index;

  UInt_t hitnum[g_tof_location]={0,0};
  UInt_t hitevents[g_tof_location][g_tof_wireindex]={{0,0,0,0},{0,0,0,0}};
  Int_t  timetag[g_tof_location][g_tof_wireindex];
  Double_t bunchid_time,interval,starttime,starttime_8cm;
  Double_t startime_cache[g_tof_location];
  // for(int i=0;i<100;i++){
  for(int i=0;i<entries;i++){
    if(!((i+1)%5000)){
      printf("%d events analyzed\n",i+1);
    }
    tree_mwdc->GetEntry(i);
    tree_tof->GetEntry(i);
    tree_bunchid->GetEntry(i);

    for(int l=0;l<g_tof_location;l++){
      hitnum[l]=0;
    }

    bunchid_time=(bunchid&0x7FF)*25.;
    // printf("%.4f\n", bunchid_time);
    for(it=tof_timeleading->begin();it!=tof_timeleading->end();it++){
      Encoding::Decode(it->first,type,location,direction,index);

      hitnum[location]++;
      hitevents[location][index]++;
      timetag[location][index]=it->second[0];

      interval=timetag[location][index]*25./256/4 - bunchid_time;
      if(interval < 0){
       interval+=g_range_bunchid/2*25;
      }
      h1d_timetag_tof[it->first]->Fill(interval);
    }
    //
    for(int l=0;l<g_tof_location;l++){
      h1d_hitnum_tof->Fill(hitnum[l]+l*6);
      if(hitnum[l]==4){
        starttime=Utility::calc_starttime(timetag[l]);
        starttime_8cm=Utility::calc_starttime(timetag[l],8);
        startime_cache[l]=Utility::calc_starttime(timetag[l],0.1);

        h1d_starttime_tof[l]->Fill(starttime - bunchid_time);
        h1d_starttime_tof_8cm[l]->Fill(starttime_8cm - bunchid_time);

        h1d_test_tof[l]->Fill(timetag[l][1]-timetag[l][2]);
        //
        for(int w=0;w<g_tof_wireindex;w++){
          interval=timetag[location][index]*25./256/4 - starttime;
          // if(interval < 0){
           // interval+=g_range_bunchid/2*25;
          // }
          UInt_t gid=Encoding::Encode(ETOF,l,0,w);
          h1d_timetagt0_tof_15cm[gid]->Fill(interval);

          interval=timetag[location][index]*25./256/4 - starttime_8cm;
          h1d_timetagt0_tof_8cm[gid]->Fill(interval);
        }
      }
    }

    if(tof_timeleading->size() == 8){
      h1d_startimediff_tof->Fill(startime_cache[0]-startime_cache[1]);
    }
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
  
  for(int l=0;l<g_tof_location;l++){
    for(int w=0;w<g_tof_wireindex;w++){
      h1d_hitrate_tof->Fill(l*g_tof_wireindex+w+1,((Double_t)hitevents[l][w])/entries);
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
  TCanvas* c1=new TCanvas("ctof_hitnum","ctof_hitnum");
  c1->SetLogy();
  h1d_hitnum_tof->Draw();
  TCanvas* c2=new TCanvas("ctof_hitrate","ctof_hitrate");
  c2->SetLogy();
  h1d_hitrate_tof->Draw();
  for(int l=0;l<g_tof_location;l++){
    for(int w=0;w<g_tof_wireindex;w++){
        TCanvas *can = (TCanvas*) gROOT->FindObject(Form("cantof_timetag_%s%d",g_str_location[l],w+1));
        if(can) delete can;
        can=new TCanvas(Form("cantof_timetag_%s%d",g_str_location[l],w+1),Form("cantof_timetag_%s%d",g_str_location[l],w+1),500,500);
          
        UInt_t gid=Encoding::Encode(ETOF,l,0,w);
        h1d_timetag_tof[gid]->Draw();
        h1d_starttime_tof[l]->SetLineColor(kRed);
        h1d_starttime_tof[l]->Draw("same");
        h1d_starttime_tof_8cm[l]->SetLineColor(kGreen);
        h1d_starttime_tof_8cm[l]->Draw("same");
      }
  }
  for(int l=0;l<g_tof_location;l++){
    TCanvas *can = (TCanvas*) gROOT->FindObject(Form("cantof_starttime_%s",g_str_location[l]));
    if(can) delete can;
    can=new TCanvas(Form("cantof_starttime_%s%d",g_str_location[l]),Form("cantof_starttime_%s",g_str_location[l]),500,500);
    
    h1d_starttime_tof[l]->Draw();
  }  
  for(int l=0;l<g_tof_location;l++){
    TCanvas *can = (TCanvas*) gROOT->FindObject(Form("cantof_test_%s",g_str_location[l]));
    if(can) delete can;
    can=new TCanvas(Form("cantof_test_%s%d",g_str_location[l]),Form("cantof_test_%s",g_str_location[l]),500,500);
    
    h1d_test_tof[l]->Draw();
  }
  for(int l=0;l<g_tof_location;l++){
      for(int w=0;w<g_tof_wireindex;w++){
          TCanvas *can = (TCanvas*) gROOT->FindObject(Form("cantof_timetagt0_%s%d",g_str_location[l],w+1));
            if(can) delete can;
            can=new TCanvas(Form("cantof_timetagt0_%s%d",g_str_location[l],w+1),Form("cantof_timetagt0_%s%d",g_str_location[l],w+1),500,500);
            
            UInt_t gid=Encoding::Encode(ETOF,l,0,w);
            h1d_timetagt0_tof_15cm[gid]->Draw();
            h1d_timetagt0_tof_8cm[gid]->SetLineColor(kRed);
            h1d_timetagt0_tof_8cm[gid]->Draw("same");
        }
  }
  TCanvas* cstarttimediff=new TCanvas("ctof_starttimediff","ctof_starttimediff");
    // cstarttimediff->SetLogy();
  h1d_startimediff_tof->Draw();
// 
  delete file_out;
  
  return 0;
}

