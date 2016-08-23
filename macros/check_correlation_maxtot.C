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

int check_correlation_maxtot(const char* datadir,const char* outfile)
{
  gStyle->SetOptStat(111111);
  //
  std::map<UInt_t,TH2F*> h2d_mwdc_drifttime_vs_tot_allhits;
  for(int l=0;l<g_mwdc_location;l++){
    for(int p=0;p<g_mwdc_wireplane;p++){
        for(int w=0;w<g_mwdc_wireindex[p];w++){
            UInt_t gid=Encoding::Encode(EMWDC,l,p,w);
            h2d_mwdc_drifttime_vs_tot_allhits[gid]=new TH2F(Form("h2d_mwdc_%s_%s_%d",g_str_location[l],g_str_plane[p],w+1)+TString("_DriftTime_VS_TOT_allhits"),Form("h2d_mwdc_%s_%s_%d",g_str_location[l],g_str_plane[p],w+1)+TString("_DriftTime_VS_TOT_allhits"),120,-50,550,300,-45.5,2950.5);
            h2d_mwdc_drifttime_vs_tot_allhits[gid]->SetDirectory(0);
          }
      }
  }
  std::map<UInt_t,TH2F*> h2d_mwdc_drifttime_vs_tot_maxtot;
  for(int l=0;l<g_mwdc_location;l++){
    for(int p=0;p<g_mwdc_wireplane;p++){
      for(int w=0;w<g_mwdc_wireindex[p];w++){
        UInt_t gid=Encoding::Encode(EMWDC,l,p,w);
        h2d_mwdc_drifttime_vs_tot_maxtot[gid]=new TH2F(Form("h2d_mwdc_%s_%s_%d",g_str_location[l],g_str_plane[p],w+1)+TString("_DriftTime_VS_TOT_maxtot"),Form("h2d_mwdc_%s_%s_%d",g_str_location[l],g_str_plane[p],w+1)+TString("_DriftTime_VS_TOT_maxtot"),120,-50,550,300,-45.5,2950.5);
        h2d_mwdc_drifttime_vs_tot_maxtot[gid]->SetDirectory(0);
      }
    }
  }
  std::map<UInt_t,TH2F*> h2d_mwdc_drifttime_vs_tot_minrising;
  for(int l=0;l<g_mwdc_location;l++){
    for(int p=0;p<g_mwdc_wireplane;p++){
      for(int w=0;w<g_mwdc_wireindex[p];w++){
          UInt_t gid=Encoding::Encode(EMWDC,l,p,w);
          h2d_mwdc_drifttime_vs_tot_minrising[gid]=new TH2F(Form("h2d_mwdc_%s_%s_%d",g_str_location[l],g_str_plane[p],w+1)+TString("_DriftTime_VS_TOT_minrising"),Form("h2d_mwdc_%s_%s_%d",g_str_location[l],g_str_plane[p],w+1)+TString("_DriftTime_VS_TOT_minrising"),120,-50,550,300,-45.5,2950.5);
          h2d_mwdc_drifttime_vs_tot_minrising[gid]->SetDirectory(0);
        }
    }
  }
  std::map<UInt_t,TH2F*> h2d_mwdc_drifttime_vs_tot_maxtot_and_minrising;
   for(int l=0;l<g_mwdc_location;l++){
     for(int p=0;p<g_mwdc_wireplane;p++){
         for(int w=0;w<g_mwdc_wireindex[p];w++){
             UInt_t gid=Encoding::Encode(EMWDC,l,p,w);
             h2d_mwdc_drifttime_vs_tot_maxtot_and_minrising[gid]=new TH2F(Form("h2d_mwdc_%s_%s_%d",g_str_location[l],g_str_plane[p],w+1)+TString("_DriftTimie_VS_TOT_maxtot_and_minrising"),Form("h2d_mwdc_%s_%s_%d",g_str_location[l],g_str_plane[p],w+1)+TString("_DriftTimie_VS_TOT_maxtot_and_minrising"),120,-50,550,300,-45.5,2950.5);
             h2d_mwdc_drifttime_vs_tot_maxtot_and_minrising[gid]->SetDirectory(0);
           }
       }
   }
  // std::map<UInt_t,TH2F*> h2d_mwdc_risingtimediff_vs_totdiff_gid;
  // for(int l=0;l<g_mwdc_location;l++){
  //   for(int p=0;p<g_mwdc_wireplane;p++){
  //       for(int w=0;w<g_mwdc_wireindex[p];w++){
  //           UInt_t gid=Encoding::Encode(EMWDC,l,p,w);
  //           h2d_mwdc_risingtimediff_vs_totdiff_gid[gid]=new TH2F(Form("h2d_mwdc_%s_%s_%d",g_str_location[l],g_str_plane[p],w+1)+TString("_RisingtimeDiff_VS_TotDiff_gid"),Form("h2d_mwdc_%s_%s_%d",g_str_location[l],g_str_plane[p],w+1)+TString("_RisingtimeDiff_VS_TotDiff_gid"),300,-599.5,0.5,300,-0.5,2999.5);
  //           h2d_mwdc_risingtimediff_vs_totdiff_gid[gid]->SetDirectory(0);
  //         }
  //     }
  // }    
  TH2F* h2d_mwdc_risingtimediff_vs_totdiff[g_mwdc_location][g_mwdc_wireplane];
  for(int l=0;l<g_mwdc_location;l++){
    for(int p=0;p<g_mwdc_wireplane;p++){
      h2d_mwdc_risingtimediff_vs_totdiff[l][p]=new TH2F(Form("h2d_mwdc_%s_%s",g_str_location[l],g_str_plane[p])+TString("_RisingtimeDiff_VS_TotDiff"),Form("h2d_mwdc_%s_%s",g_str_location[l],g_str_plane[p])+TString("_RisingtimeDiff_VS_TotDiff"),3000,-0.5,2999.5,300,-0.5,2999.5);
      h2d_mwdc_risingtimediff_vs_totdiff[l][p]->SetDirectory(0);
    }
  }

  TH1F* h1d_mwdc_maxtot_minrising_giddiff[g_mwdc_location][g_mwdc_wireplane];
  TH1F* h1d_mwdc_maxtot_minrising_risingtimediff_adjacent[g_mwdc_location][g_mwdc_wireplane];
  TH1F* h1d_mwdc_maxtot_minrising_totdiff_adjacent[g_mwdc_location][g_mwdc_wireplane];
  TH1F* h1d_mwdc_maxtot_minrising_risingtimediff_other[g_mwdc_location][g_mwdc_wireplane];
  TH1F* h1d_mwdc_maxtot_minrising_totdiff_other[g_mwdc_location][g_mwdc_wireplane];
  TH1F* h1d_mwdc_test[g_mwdc_location][g_mwdc_wireplane];
  for(int l=0;l<g_mwdc_location;l++){
    for(int p=0;p<g_mwdc_wireplane;p++){
      h1d_mwdc_maxtot_minrising_giddiff[l][p]=new TH1F(Form("h1d_mwdc_%s_%s_maxtot_minrising_giddiff",g_str_location[l],g_str_plane[p]),Form("h1d_mwdc_%s_%s_maxtot_minrising_giddiff",g_str_location[l],g_str_plane[p]),220,-109.5,110.5);
      h1d_mwdc_maxtot_minrising_risingtimediff_adjacent[l][p]=new TH1F(Form("h1d_mwdc_%s_%s_maxtot_minrising_risingtimediff_adjacent",g_str_location[l],g_str_plane[p]),Form("h1d_mwdc_%s_%s_maxtot_minrising_risingtimediff_adjacent",g_str_location[l],g_str_plane[p]),300,-0.5,399.5);
      h1d_mwdc_maxtot_minrising_risingtimediff_other[l][p]=new TH1F(Form("h1d_mwdc_%s_%s_maxtot_minrising_risingtimediff_other",g_str_location[l],g_str_plane[p]),Form("h1d_mwdc_%s_%s_maxtot_minrising_risingtimediff_other",g_str_location[l],g_str_plane[p]),300,-0.5,399.5);
      h1d_mwdc_maxtot_minrising_totdiff_adjacent[l][p]=new TH1F(Form("h1d_mwdc_%s_%s_maxtot_minrising_totdiff_adjacent",g_str_location[l],g_str_plane[p]),Form("h1d_mwdc_%s_%s_maxtot_minrising_totdiff_adjacent",g_str_location[l],g_str_plane[p]),3000,-0.5,2999.5);
      h1d_mwdc_maxtot_minrising_totdiff_other[l][p]=new TH1F(Form("h1d_mwdc_%s_%s_maxtot_minrising_totdiff_other",g_str_location[l],g_str_plane[p]),Form("h1d_mwdc_%s_%s_maxtot_minrising_totdiff_other",g_str_location[l],g_str_plane[p]),3000,-0.5,2999.5);
      
      h1d_mwdc_test[l][p]=new TH1F(Form("h1d_mwdc_test_%s_%s",g_str_location[l],g_str_plane[p]),Form("h1d_mwdc_test_%s_%s",g_str_location[l],g_str_plane[p]),6000,-3000,3000);
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
  Double_t starttime[g_tof_location];
  Int_t  timetag[g_tof_location][g_tof_wireindex];
  Double_t drift_time,max_drift_time[g_mwdc_location][g_mwdc_wireplane];

  UInt_t maxtot_gid[g_mwdc_location][g_mwdc_wireplane];
  Double_t tot,maxtot[g_mwdc_location][g_mwdc_wireplane];//unit: ns
  Double_t tot_limit=2775;// if a channel has only rising edge and no falling edge, we think its tot is beyond the limit.
                             // thus assign this value as the tot value of this channel. It's kind of arbitrarily.
  UInt_t minrising_gid[g_mwdc_location][g_mwdc_wireplane];
  Double_t rising,minrising[g_mwdc_location][g_mwdc_wireplane],minrising_tot[g_mwdc_location][g_mwdc_wireplane];
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
        minrising[l][p]=3000;// a large value limit
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
        }

        //
        if(drift_time < minrising[location][direction]){
          minrising[location][direction]=drift_time;
          minrising_gid[location][direction]=it->first;
          minrising_tot[location][direction]=tot;
        }
        // fill histograms
        h2d_mwdc_drifttime_vs_tot_allhits[it->first]->Fill(drift_time,tot);
      }
      //
      for(int l=0;l<g_mwdc_location;l++){
        for(int p=0;p<g_mwdc_wireplane;p++){
          if(Encoding::IsChannelValid(maxtot_gid[l][p])){
            h2d_mwdc_drifttime_vs_tot_maxtot[maxtot_gid[l][p]]->Fill(max_drift_time[l][p],maxtot[l][p]);
          }
          if(Encoding::IsChannelValid(minrising_gid[l][p])){
            h2d_mwdc_drifttime_vs_tot_minrising[minrising_gid[l][p]]->Fill(minrising[l][p],minrising_tot[l][p]);
          }

          if(Encoding::IsChannelValid(maxtot_gid[l][p]) && Encoding::IsChannelValid(minrising_gid[l][p])){
            if(maxtot_gid[l][p] == minrising_gid[l][p]){
              h2d_mwdc_drifttime_vs_tot_maxtot_and_minrising[maxtot_gid[l][p]]->Fill(max_drift_time[l][p],maxtot[l][p]);
              if(max_drift_time[l][p] != minrising[l][p] || maxtot[l][p] != minrising_tot[l][p]){
                printf("fuck!\n");
              }
            }
            else if(TMath::Abs(Encoding::DecodeIndex(maxtot_gid[l][p]) -Encoding::DecodeIndex(minrising_gid[l][p])) == 1){//adjacent
              h1d_mwdc_maxtot_minrising_risingtimediff_adjacent[l][p]->Fill(max_drift_time[l][p]);
              h1d_mwdc_maxtot_minrising_totdiff_adjacent[l][p]->Fill(maxtot[l][p]);
              h1d_mwdc_test[l][p]->Fill(max_drift_time[l][p]-minrising[l][p]-minrising_tot[l][p]);
              // if(minrising[l][p]>60 || max_drift_time[l][p]>120){
              h2d_mwdc_risingtimediff_vs_totdiff[l][p]->Fill(max_drift_time[l][p]-minrising[l][p],maxtot[l][p]-minrising_tot[l][p]);
              // }
            }
            else{//other
              h1d_mwdc_maxtot_minrising_risingtimediff_other[l][p]->Fill(max_drift_time[l][p]);
              h1d_mwdc_maxtot_minrising_totdiff_other[l][p]->Fill(maxtot[l][p]);
              // h2d_mwdc_risingtimediff_vs_totdiff[l][p]->Fill(max_drift_time[l][p],Encoding::DecodeIndex(maxtot_gid[l][p]) -Encoding::DecodeIndex(minrising_gid[l][p]));
              // h2d_mwdc_risingtimediff_vs_totdiff[l][p]->Fill(-minrising[l][p],Encoding::DecodeIndex(maxtot_gid[l][p]) -Encoding::DecodeIndex(minrising_gid[l][p]));
            h2d_mwdc_risingtimediff_vs_totdiff[l][p]->Fill(max_drift_time[l][p]-minrising[l][p],maxtot[l][p]-minrising_tot[l][p]);
            }
            //
            // h1d_mwdc_maxtot_minrising_giddiff[l][p]->Fill(Encoding::DecodeIndex(maxtot_gid[l][p]) -Encoding::DecodeIndex(minrising_gid[l][p]));
          }
        }
      }
    }
  }
  
  printf("%d events processed totally\n",entries);

  Int_t bindiff[3]={-1,0,1};
  Int_t bin[3];
  Double_t integral[3],area,other;
  printf("\nMax_TOT_Wire and Min_Riring_Wire Index Diff:\t-1\t0\t1\tother\n");
  for(int l=0;l<g_mwdc_location;l++){
    for(int p=0;p<g_mwdc_wireplane;p++){
      area=h1d_mwdc_maxtot_minrising_giddiff[l][p]->Integral();
      other=area;
      printf("%s_%s:\t",g_str_location[l],g_str_plane[p]);
      for(int i=0;i<3;i++){
        bin[i]=h1d_mwdc_maxtot_minrising_giddiff[l][p]->FindBin(bindiff[i]);
        integral[i]=h1d_mwdc_maxtot_minrising_giddiff[l][p]->GetBinContent(bin[i]);
        other-=integral[i];
        printf("%.4f\t", integral[i]/area);
      }
      printf("%.4f\n",other/area);
    }
  }
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

  TDirectory* dir_maxtot=dir_hist->GetDirectory("check_correlation_maxtot");
  if(!dir_maxtot){
    dir_maxtot=dir_hist->mkdir("check_correlation_maxtot");
    if(!dir_maxtot){
      printf("error!can't mkdir \"raw/hitogram/check_correlation_maxtot\" in %s\n",outfile);
      exit(1);
    }
    dir_maxtot=dir_hist->GetDirectory("check_correlation_maxtot");
  }
  dir_maxtot->cd();
  //
 
  //
  UInt_t test_gid=Encoding::Encode(EMWDC,EUP,EX,40);
  // TCanvas* c1=new TCanvas("cmwdc_drifttime_vs_tot_allhits","cmwdc_drifttime_vs_tot_allhits");
  // c1->SetLogy();
  // h2d_mwdc_drifttime_vs_tot_allhits[test_gid]->Draw("colz");
  TCanvas* c2=new TCanvas("cmwdc_drifttime_vs_tot_maxtot","cmwdc_drifttime_vs_tot_maxtot");
  // c2->SetLogy();
  h2d_mwdc_drifttime_vs_tot_maxtot[test_gid]->Draw("colz");
  TCanvas* c3=new TCanvas("cmwdc_drifttime_vs_tot_minrising","cmwdc_drifttime_vs_tot_minrising");
  // c3->SetLogy();
  h2d_mwdc_drifttime_vs_tot_minrising[test_gid]->Draw("colz");
  TCanvas* c4=new TCanvas("cmwdc_drifttime_vs_tot_maxtot_and_minrising","cmwdc_drifttime_vs_tot_maxtot_and_minrising");
  // c4->SetLogy();
  h2d_mwdc_drifttime_vs_tot_maxtot_and_minrising[test_gid]->Draw("colz");
  // 
  // for(int l=0;l<g_mwdc_location;l++){
  //   for(int p=0;p<g_mwdc_wireplane;p++){
  //     TCanvas* c=new TCanvas(Form("cmwdc_%s_%s_maxtot_minrising_giddiff",g_str_location[l],g_str_plane[p]),Form("cmwdc_%s_%s_maxtot_minrising_giddiff",g_str_location[l],g_str_plane[p]),500,500);
  //     // c4->SetLogy();
  //     h1d_mwdc_maxtot_minrising_giddiff[l][p]->Draw();
  //   }
  // }
  // printf("\nTOT limit proportion(adjacent events):\n");
  // for(int l=0;l<g_mwdc_location;l++){
  //   for(int p=0;p<g_mwdc_wireplane;p++){
  //     TCanvas* c=new TCanvas(Form("cmwdc_%s_%s_maxtot_minrising_risingtimediff_adjacent",g_str_location[l],g_str_plane[p]),Form("cmwdc_%s_%s_maxtot_minrising_risingtimediff_adjacent",g_str_location[l],g_str_plane[p]),500,500);
  //     // c4->SetLogy();
  //     h1d_mwdc_maxtot_minrising_risingtimediff_adjacent[l][p]->Draw();
  //     printf("%.4f\n", h1d_mwdc_maxtot_minrising_risingtimediff_adjacent[l][p]->GetBinContent(h1d_mwdc_maxtot_minrising_risingtimediff_adjacent[l][p]->FindBin(tot_limit))/h1d_mwdc_maxtot_minrising_risingtimediff_adjacent[l][p]->Integral());
  //   }
  // }
  // for(int l=0;l<g_mwdc_location;l++){
  //   for(int p=0;p<g_mwdc_wireplane;p++){
  //     TCanvas* c=new TCanvas(Form("cmwdc_%s_%s_maxtot_minrising_totdiff_adjacent",g_str_location[l],g_str_plane[p]),Form("cmwdc_%s_%s_maxtot_minrising_totdiff_adjacent",g_str_location[l],g_str_plane[p]),500,500);
  //     // c4->SetLogy();
  //     h1d_mwdc_maxtot_minrising_totdiff_adjacent[l][p]->Draw();
  //   }
  // }
  // printf("\nTOT limit proportion(other events):\n");
  // for(int l=0;l<g_mwdc_location;l++){
  //   for(int p=0;p<g_mwdc_wireplane;p++){
  //     TCanvas* c=new TCanvas(Form("cmwdc_%s_%s_maxtot_minrising_risingtimediff_other",g_str_location[l],g_str_plane[p]),Form("cmwdc_%s_%s_maxtot_minrising_risingtimediff_other",g_str_location[l],g_str_plane[p]),500,500);
  //     // c4->SetLogy();
  //     h1d_mwdc_maxtot_minrising_risingtimediff_other[l][p]->Draw();
  //     printf("%.4f\n", h1d_mwdc_maxtot_minrising_risingtimediff_other[l][p]->GetBinContent(h1d_mwdc_maxtot_minrising_risingtimediff_other[l][p]->FindBin(tot_limit))/h1d_mwdc_maxtot_minrising_risingtimediff_other[l][p]->Integral());
  //   }
  // }
  // for(int l=0;l<g_mwdc_location;l++){
  //   for(int p=0;p<g_mwdc_wireplane;p++){
  //     TCanvas* c=new TCanvas(Form("cmwdc_%s_%s_maxtot_minrising_totdiff_other",g_str_location[l],g_str_plane[p]),Form("cmwdc_%s_%s_maxtot_minrising_totdiff_other",g_str_location[l],g_str_plane[p]),500,500);
  //     // c4->SetLogy();
  //     h1d_mwdc_maxtot_minrising_totdiff_other[l][p]->Draw();
  //   }
  // }
  for(int l=0;l<g_mwdc_location;l++){
    for(int p=0;p<g_mwdc_wireplane;p++){
      TCanvas* c=new TCanvas(Form("cmwdc_%s_%s_risingtimediff_vs_totdiff",g_str_location[l],g_str_plane[p]),Form("cmwdc_%s_%s_risingtimediff_vs_totdiff",g_str_location[l],g_str_plane[p]),500,500);
      // c4->SetLogy();
      h2d_mwdc_risingtimediff_vs_totdiff[l][p]->Draw("colz");
    }
  }
  for(int l=0;l<g_mwdc_location;l++){
      for(int p=0;p<g_mwdc_wireplane;p++){
        TCanvas* c=new TCanvas(Form("cmwdc_%s_%s_test",g_str_location[l],g_str_plane[p]),Form("cmwdc_%s_%s_test",g_str_location[l],g_str_plane[p]),500,500);
        // c4->SetLogy();
        h1d_mwdc_test[l][p]->Draw();
      }
    }
  //
  delete file_out;
  
  return 0;
}

