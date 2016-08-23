/*   $Id: check_triggermatching.h, 2016-07-17 09:07:05+08:00 MWDC_ana Raw$
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
#include "TH2F.h"
#include "CrateInfo.h"
#include "BoardInfo.h"
#include "utility.h"
#include "TStyle.h"

int check_triggermatching_single(const char* datadir,const char* outfile)
{
  gStyle->SetOptStat(111111);
  //read the config file which include channelmapping info
  TString file_config=TString(datadir)+"../crate.json";
  CrateInfo* info=Utility::read_config(file_config.Data(),"mapping");
  info->Print();

  //check the structure of root file,check the consitency between root file and config file
  TString file_data=TString(datadir)+"/"+outfile;  
  TFile* file_out=new TFile(file_data,"update");
  if(!file_out){
    printf("open file error: %s\n",outfile);
    exit(1);
  }
  TDirectory* raw_dir=file_out->GetDirectory("raw");
  if(!raw_dir){
    printf("dir \"raw\" not exist in this file.invoke convert_hptdc first\n");
    exit(1);
  }
  
  TList* keys=raw_dir->GetListOfKeys();
  int boardnum=info->GetBoardNum();
  int mwdcnum=0;
  int tofnum=0;
  BoardInfo** boardinfo=new BoardInfo*[boardnum]{};
  for(int i=0;i<boardnum;i++){
    boardinfo[i]=info->GetBoardInfo(i);
    switch (boardinfo[i]->GetType()) {
	  case EMWDC:
	    mwdcnum++;
	    break;
	  case ETOF:
	    tofnum++;
	  default:
	    break;
    }
    if(!keys->FindObject(boardinfo[i]->GetName())){
	    printf("error missing raw tree: you may not use the same config file\n");
	    exit(1);
    }
  }

  //init and get corresponding tree from root file
  TTree** 	tree_in_mwdc=new TTree*[mwdcnum]{};
  BoardInfo**	mwdc_boardinfo=new BoardInfo*[mwdcnum]{};
  Int_t*	mwdc_eventid=new Int_t[mwdcnum]{};
  Int_t*	mwdc_bunchid=new Int_t[mwdcnum]{};
  Char_t* mwdc_eventflag=new Char_t[mwdcnum]{};
  ChannelMap** 	mwdc_leading_raw=new ChannelMap*[mwdcnum]{};
  ChannelMap** 	mwdc_trailing_raw=new ChannelMap*[mwdcnum]{};
  
  TTree** 	tree_in_tof=new TTree*[tofnum]{};
  BoardInfo**	tof_boardinfo=new BoardInfo*[tofnum]{};
  Char_t* tof_eventflag=new Char_t[tofnum]{};
  Int_t*	tof_eventid=new Int_t[tofnum]{};
  Int_t*	tof_bunchid=new Int_t[tofnum]{};
  ChannelMap** 	tof_timeleading_raw=new ChannelMap*[tofnum]{};
  ChannelMap** 	tof_timetrailing_raw=new ChannelMap*[tofnum]{};
  ChannelMap** 	tof_totleading_raw=new ChannelMap*[tofnum]{};
  ChannelMap** 	tof_tottrailing_raw=new ChannelMap*[tofnum]{};
  
  TTree** 	tree_in=new TTree*[boardnum]{};
  mwdcnum=0;tofnum=0;
  for(int i=0;i<boardnum;i++){
    switch (boardinfo[i]->GetType()){
      case EMWDC:
	raw_dir->GetObject(boardinfo[i]->GetName(),tree_in_mwdc[mwdcnum++]);
	mwdc_boardinfo[mwdcnum-1]=boardinfo[i];
	tree_in[i]=tree_in_mwdc[mwdcnum-1];
  tree_in[i]->SetBranchAddress("event_flag",&mwdc_eventflag[mwdcnum-1]);	
	tree_in[i]->SetBranchAddress("event_id",&mwdc_eventid[mwdcnum-1]);
	tree_in[i]->SetBranchAddress("bunch_id",&mwdc_bunchid[mwdcnum-1]);
	tree_in[i]->SetBranchAddress("leading_raw",&mwdc_leading_raw[mwdcnum-1]);
	tree_in[i]->SetBranchAddress("trailing_raw",&mwdc_trailing_raw[mwdcnum-1]);
  // Activate the referenced branches
  tree_in[i]->SetBranchStatus("*",0);
  //tree_in[i]->SetBranchStatus("event_flag",1);
  //tree_in[i]->SetBranchStatus("event_id",1);
  tree_in[i]->SetBranchStatus("bunch_id",1);
  tree_in[i]->SetBranchStatus("leading_raw",1);
  tree_in[i]->SetBranchStatus("trailing_raw",1);

	mwdc_boardinfo[mwdcnum-1]->Print();
	break;
      case ETOF:
	raw_dir->GetObject(boardinfo[i]->GetName(),tree_in_tof[tofnum++]);
	tof_boardinfo[tofnum-1]=boardinfo[i];
	tree_in[i]=tree_in_tof[tofnum-1];	
  tree_in[i]->SetBranchAddress("event_flag",&tof_eventflag[tofnum-1]);
	tree_in[i]->SetBranchAddress("event_id",&tof_eventid[tofnum-1]);
	tree_in[i]->SetBranchAddress("bunch_id",&tof_bunchid[tofnum-1]);
	tree_in[i]->SetBranchAddress("time_leading_raw",&tof_timeleading_raw[tofnum-1]);
	tree_in[i]->SetBranchAddress("time_trailing_raw",&tof_timetrailing_raw[tofnum-1]);
	tree_in[i]->SetBranchAddress("tot_leading_raw",&tof_totleading_raw[tofnum-1]);
	tree_in[i]->SetBranchAddress("tot_trailing_raw",&tof_tottrailing_raw[tofnum-1]);
  // Activate the referenced branches
  tree_in[i]->SetBranchStatus("*",0);
  //tree_in[i]->SetBranchStatus("event_flag",1);
  //tree_in[i]->SetBranchStatus("event_id",1);
  tree_in[i]->SetBranchStatus("bunch_id",1);
  tree_in[i]->SetBranchStatus("time_leading_raw",1);
  tree_in[i]->SetBranchStatus("time_trailing_raw",1);
  tree_in[i]->SetBranchStatus("tot_leading_raw",1);
  tree_in[i]->SetBranchStatus("tot_trailing_raw",1);

	tof_boardinfo[tofnum-1]->Print();
	break;
      default:
	break;
    }
  }

  TTree* tree_multihit;
  file_out->GetObject("merge/mwdc_multihit",tree_multihit);
  Int_t multihit[g_mwdc_location][g_mwdc_wireplane];
  tree_multihit->SetBranchAddress("multihit",multihit);

  // make new histogram here
  std::vector<TH1F*> histrepo1d_mwdc_global;
  for(int i=0;i<mwdcnum;i++){
    histrepo1d_mwdc_global.push_back(new TH1F(TString("h")+boardinfo[i]->GetName()+TString("_TriggerMatching_SingleHit_Board"),boardinfo[i]->GetTitle()+TString("_TriggerMatching_SingleHit_Board"),3500,-45.5,3000.5));
    histrepo1d_mwdc_global[i]->SetDirectory(0);
  }
  std::vector<TH1F*> histrepo1d_mwdc_global_trailing;
  for(int i=0;i<mwdcnum;i++){
    histrepo1d_mwdc_global_trailing.push_back(new TH1F(TString("h")+boardinfo[i]->GetName()+TString("_TriggerMatching_SingleHit_Board_Trailing"),boardinfo[i]->GetTitle()+TString("_TriggerMatching_SingleHit_Board_Trailing"),3500,-45.5,3000.5));
    histrepo1d_mwdc_global_trailing[i]->SetDirectory(0);
  }
  std::vector<TH1F*> histrepo1d_mwdc_global_tot;
  for(int i=0;i<mwdcnum;i++){
    histrepo1d_mwdc_global_tot.push_back(new TH1F(TString("h")+boardinfo[i]->GetName()+TString("_TriggerMatching_SingleHit_Board_ToT"),boardinfo[i]->GetTitle()+TString("_TriggerMatching_SingleHit_Board_ToT"),500,-45.5,3000.5));
    histrepo1d_mwdc_global_tot[i]->SetDirectory(0);
  }
  std::vector<TH1F*> histrepo1d_mwdc_global_tot_highcut;
  for(int i=0;i<mwdcnum;i++){
    histrepo1d_mwdc_global_tot_highcut.push_back(new TH1F(TString("h")+boardinfo[i]->GetName()+TString("_TriggerMatching_SingleHit_Board_ToT_HighCut"),boardinfo[i]->GetTitle()+TString("_TriggerMatching_SingleHit_Board_ToT_HighCut"),500,-45.5,3000.5));
    histrepo1d_mwdc_global_tot_highcut[i]->SetDirectory(0);
  }
  std::vector<TH1F*> histrepo1d_mwdc_global_tot_lowcut;
  for(int i=0;i<mwdcnum;i++){
    histrepo1d_mwdc_global_tot_lowcut.push_back(new TH1F(TString("h")+boardinfo[i]->GetName()+TString("_TriggerMatching_SingleHit_Board_ToT_LowCut"),boardinfo[i]->GetTitle()+TString("_TriggerMatching_SingleHit_Board_ToT_LowCut"),500,-45.5,3000.5));
    histrepo1d_mwdc_global_tot_lowcut[i]->SetDirectory(0);
  }

  std::vector<TH1F*> histrepo1d_tof_leading;
  for(int i=0;i<tofnum;i++){
    histrepo1d_tof_leading.push_back(new TH1F(TString("h")+boardinfo[mwdcnum+i]->GetName()+TString("_TriggerMatching_Board_Leading"),boardinfo[mwdcnum+i]->GetTitle()+TString("_TriggerMatching_Board_Leading"),950,-45.5,900.5));
    histrepo1d_tof_leading[i]->SetDirectory(0);
  }
  std::vector<TH1F*> histrepo1d_tof_trailing;
  for(int i=0;i<tofnum;i++){
    histrepo1d_tof_trailing.push_back(new TH1F(TString("h")+boardinfo[mwdcnum+i]->GetName()+TString("_TriggerMatching_Board_Trailing"),boardinfo[mwdcnum+i]->GetTitle()+TString("_TriggerMatching_Board_Trailing"),950,-45.5,900.5));
    histrepo1d_tof_trailing[i]->SetDirectory(0);
  }
  std::vector<TH1F*> histrepo1d_tof_tot;
  for(int i=0;i<tofnum;i++){
    histrepo1d_tof_tot.push_back(new TH1F(TString("h")+boardinfo[mwdcnum+i]->GetName()+TString("_TriggerMatching_Board_Tot"),boardinfo[mwdcnum+i]->GetTitle()+TString("_TriggerMatching_Board_Tot"),950,-45.5,900.5));
    histrepo1d_tof_tot[i]->SetDirectory(0);
  }
  std::vector<TH1F*> histrepo1d_tof_timeleading;
  for(int i=0;i<tofnum;i++){
    histrepo1d_tof_timeleading.push_back(new TH1F(TString("h")+boardinfo[mwdcnum+i]->GetName()+TString("_TriggerMatching_Board_Timeleading"),boardinfo[mwdcnum+i]->GetTitle()+TString("_TriggerMatching_Board_Timeleading"),750,-45.5,700.5));
    histrepo1d_tof_timeleading[i]->SetDirectory(0);
  }
  std::vector<TH1F*> histrepo1d_tof_timetrailing;
  for(int i=0;i<tofnum;i++){
    histrepo1d_tof_timetrailing.push_back(new TH1F(TString("h")+boardinfo[mwdcnum+i]->GetName()+TString("_TriggerMatching_Board_Timetrailing"),boardinfo[mwdcnum+i]->GetTitle()+TString("_TriggerMatching_Board_Timetrailing"),750,-45.5,700.5));
    histrepo1d_tof_timetrailing[i]->SetDirectory(0);
  }
  std::vector<TH1F*> histrepo1d_tof_timetot;
  for(int i=0;i<tofnum;i++){
    histrepo1d_tof_timetot.push_back(new TH1F(TString("h")+boardinfo[mwdcnum+i]->GetName()+TString("_TriggerMatching_Board_TimeTot"),boardinfo[mwdcnum+i]->GetTitle()+TString("_TriggerMatching_Board_TimeTot"),750,-45.5,700.5));
    histrepo1d_tof_timetot[i]->SetDirectory(0);
  }

  std::vector<TH2F*> h2d_mwdc_leading_vs_trailing;
  for(int i=0;i<mwdcnum;i++){
    h2d_mwdc_leading_vs_trailing.push_back(new TH2F(TString("h2d_")+boardinfo[i]->GetName()+TString("_Leading_VS_Trailing_SingleHit"),boardinfo[i]->GetTitle()+TString("_Leading_VS_Trailing_SingleHit"),3500,-45.5,3000.5,3500,-45.5,3000.5));
    h2d_mwdc_leading_vs_trailing[i]->SetDirectory(0);
  }
  std::vector<TH2F*> h2d_tof_leading_vs_trailing;
  for(int i=0;i<tofnum;i++){
    h2d_tof_leading_vs_trailing.push_back(new TH2F(TString("h2d_")+boardinfo[i+mwdcnum]->GetName()+TString("_Leading_VS_Trailing"),boardinfo[i+mwdcnum]->GetTitle()+TString("_Leading_VS_Trailing"),950,-45.5,900.5,950,-45.5,900.5));
    h2d_tof_leading_vs_trailing[i]->SetDirectory(0);
  }
  std::vector<TH2F*> h2d_tof_timeleading_vs_timetrailing;
  for(int i=0;i<tofnum;i++){
    h2d_tof_timeleading_vs_timetrailing.push_back(new TH2F(TString("h2d_")+boardinfo[i+mwdcnum]->GetName()+TString("_TimeLeading_VS_TimeTrailing"),boardinfo[i+mwdcnum]->GetTitle()+TString("_TimeLeading_VS_TimeTrailing"),850,-45.5,800.5,850,-45.5,800.5));
    h2d_tof_timeleading_vs_timetrailing[i]->SetDirectory(0);
  }
  //init
  Int_t temp_entries;
  Int_t entries=tree_in[0]->GetEntriesFast();
  for(int i=0;i<boardnum;i++){
    temp_entries=tree_in[i]->GetEntriesFast();
    if(temp_entries<entries){
      entries=temp_entries;
    }
  }
  
  ChannelMap::iterator it,it_falling;
  Double_t interval,tot;
  Double_t leadingedge,trailingedge;
  UChar_t type,location,direction;
  UShort_t index;
  //for(int i=0;i<100;i++){
  for(int i=0;i<entries;i++){
    if(!((i+1)%5000)){
      printf("%d events processed\n",i+1);
    }
    //
    for(int j=0;j<boardnum;j++){
      tree_in[j]->GetEntry(i);
    }
    tree_multihit->GetEntry(i);
    //process
    for(int j=0;j<mwdcnum;j++){
      // falling edge
      for(it=mwdc_leading_raw[j]->begin();it!=mwdc_leading_raw[j]->end();it++){
         if(Encoding::IsChannelValid(it->first)){
            Encoding::Decode(mwdc_boardinfo[j]->GetEncodedID(it->first),type,location,direction,index);

            if(multihit[location][direction] == 1){
              interval=it->second[0]*25./256- (mwdc_bunchid[j]&0x7FF)*25;//bunchid is 12 bits, while data is only 11 bits (11=19-8).
              if(interval < 0){
                interval+=g_range_bunchid/2*25;
              }
	           histrepo1d_mwdc_global_trailing[j]->Fill(interval);
            }
         }
      }
      // rising edge
      for(it=mwdc_trailing_raw[j]->begin();it!=mwdc_trailing_raw[j]->end();it++){
        if(Encoding::IsChannelValid(it->first)){
          Encoding::Decode(mwdc_boardinfo[j]->GetEncodedID(it->first),type,location,direction,index);

          if(multihit[location][direction] == 1){
            interval=it->second[0]*25./256 - (mwdc_bunchid[j]&0x7FF)*25;
            if(interval < 0){
              interval+=g_range_bunchid/2*25;
            }
            histrepo1d_mwdc_global[j]->Fill(interval);
            leadingedge=interval;
            // tot
            it_falling=mwdc_leading_raw[j]->find(it->first);
            if(it_falling != mwdc_leading_raw[j]->end()){
              tot = (it_falling->second[0] - it->second[0])*25./256;
              if(tot < 0){
                tot+=g_range_bunchid/2*25;
              }
              trailingedge=tot+leadingedge;
              //
              if(interval>420){
                histrepo1d_mwdc_global_tot_highcut[j]->Fill(tot);
              }
              else{
                histrepo1d_mwdc_global_tot_lowcut[j]->Fill(tot);
              }
              //
              h2d_mwdc_leading_vs_trailing[j]->Fill(leadingedge,trailingedge);
            }
          }
        }
      }
    }

    for(int j=0;j<tofnum;j++){
      for(it=tof_timeleading_raw[j]->begin();it!=tof_timeleading_raw[j]->end();it++){
        if(Encoding::IsChannelValid(it->first)){
          interval=it->second[0]*25./256/4 - (tof_bunchid[j]&0x7FF)*25;
          if(interval < 0){
           interval+=g_range_bunchid/2*25;
          }
          histrepo1d_tof_timeleading[j]->Fill(interval);
          leadingedge=interval;
          // tot
          it_falling=tof_timetrailing_raw[j]->find(it->first);
          if(it_falling != tof_timetrailing_raw[j]->end()){
            tot = (it_falling->second[0] - it->second[0])*25./256/4;
            if(tot < 0){
              tot+=g_range_bunchid/2*25;
            }
            trailingedge=leadingedge+tot;

            histrepo1d_tof_timetot[j]->Fill(tot);

            h2d_tof_timeleading_vs_timetrailing[j]->Fill(leadingedge,trailingedge);
          }
        }
      }
      for(it=tof_timetrailing_raw[j]->begin();it!=tof_timetrailing_raw[j]->end();it++){
        if(Encoding::IsChannelValid(it->first)){
            interval=it->second[0]*25./256/4- (tof_bunchid[j]&0x7FF)*25;//bunchid is 12 bits, while data is only 11 bits (11=19-8).
            if(interval < 0){
              interval+=g_range_bunchid/2*25;
            }
            histrepo1d_tof_timetrailing[j]->Fill(interval);   
        }
      }

      for(it=tof_totleading_raw[j]->begin();it!=tof_totleading_raw[j]->end();it++){
        if(Encoding::IsChannelValid(it->first)){
          interval=it->second[0]*25./256 - (tof_bunchid[j]&0x7FF)*25;
          if(interval < 0){
           interval+=g_range_bunchid/2*25;
          }
          leadingedge=interval;
          histrepo1d_tof_leading[j]->Fill(interval);
          // tot
          it_falling=tof_tottrailing_raw[j]->find(it->first);
          if(it_falling != tof_tottrailing_raw[j]->end()){
            tot = (it_falling->second[0] - it->second[0])*25./256;
            if(tot < 0){
              tot+=g_range_bunchid/2*25;
            }
            trailingedge=leadingedge+tot;

            histrepo1d_tof_tot[j]->Fill(tot);

            h2d_tof_leading_vs_trailing[j]->Fill(leadingedge,trailingedge);
          }
        }
      }
      for(it=tof_tottrailing_raw[j]->begin();it!=tof_tottrailing_raw[j]->end();it++){
        if(Encoding::IsChannelValid(it->first)){
            interval=it->second[0]*25./256- (tof_bunchid[j]&0x7FF)*25;//bunchid is 12 bits, while data is only 11 bits (11=19-8).
            if(interval < 0){
              interval+=g_range_bunchid/2*25;
            }
            histrepo1d_tof_trailing[j]->Fill(interval);
         }
      }
    }
  }
  
  printf("%d events processed totally!\n",entries);
  
  //dir "histogram"
  TDirectory* dir_hist=file_out->GetDirectory("raw/histogram");
  if(!dir_hist){
    dir_hist=file_out->mkdir("raw/histogram");
    if(!dir_hist){
      printf("error!can't mkdir \"raw/hitogram\" in %s\n",outfile);
      exit(1);
    }
    dir_hist=file_out->GetDirectory("raw/histogram");
  }
  dir_hist->cd();
  TDirectory* dir_trigger=dir_hist->GetDirectory("check_triggermatching");
  if(!dir_trigger){
    dir_trigger=dir_hist->mkdir("check_triggermatching");
    if(!dir_trigger){
      printf("error!can't mkdir \"raw/hitogram/check_triggermatching\" in %s\n",outfile);
      exit(1);
    }
    dir_trigger=dir_hist->GetDirectory("check_triggermatching");
  }
  dir_trigger->cd();

  //
  for(int i=0;i<mwdcnum;i++){
    TCanvas *can = (TCanvas*) gROOT->FindObject(Form("canmwdc_%d",i+1));
    if(can) delete can;
    // can=new TCanvas(Form("canmwdc_%d",i+1),Form("canmwdc_%d",i+1),500,500);
    // can->SetLogy();
    histrepo1d_mwdc_global[i]->Draw();
    histrepo1d_mwdc_global[i]->Write(0,TObject::kOverwrite);
  }
  for(int i=0;i<mwdcnum;i++){
    TCanvas *can = (TCanvas*) gROOT->FindObject(Form("canmwdc_global_trailing%d",i+1));
    if(can) delete can;
    // can=new TCanvas(Form("canmwdc_global_trailing%d",i+1),Form("canmwdc_global_trailing%d",i+1),500,500);
    // can->SetLogy();
    histrepo1d_mwdc_global_trailing[i]->Draw();
    histrepo1d_mwdc_global_trailing[i]->Write(0,TObject::kOverwrite);
  }
  for(int i=0;i<mwdcnum;i++){
    TCanvas *can = (TCanvas*) gROOT->FindObject(Form("canmwdc_global_tot%d",i+1));
    if(can) delete can;
    can=new TCanvas(Form("canmwdc_global_tot%d",i+1),Form("canmwdc_global_tot%d",i+1),500,500);
    can->SetLogy();
    histrepo1d_mwdc_global_tot[i]->Add(histrepo1d_mwdc_global_tot_lowcut[i],histrepo1d_mwdc_global_tot_highcut[i]);
    histrepo1d_mwdc_global_tot[i]->Draw();
    histrepo1d_mwdc_global_tot[i]->Write(0,TObject::kOverwrite);
    // histrepo1d_mwdc_global_tot_lowcut[i]->Scale(1./histrepo1d_mwdc_global_tot_lowcut[i]->Integral());
    histrepo1d_mwdc_global_tot_lowcut[i]->SetLineColor(kBlack);
    histrepo1d_mwdc_global_tot_lowcut[i]->Draw("same");
    histrepo1d_mwdc_global_tot_lowcut[i]->Write(0,TObject::kOverwrite);
    // histrepo1d_mwdc_global_tot_highcut[i]->Scale(1./histrepo1d_mwdc_global_tot_highcut[i]->Integral());
    histrepo1d_mwdc_global_tot_highcut[i]->SetLineColor(kRed);
    histrepo1d_mwdc_global_tot_highcut[i]->Draw("same");
    histrepo1d_mwdc_global_tot_highcut[i]->Write(0,TObject::kOverwrite);
  }

  // for(int i=0;i<tofnum;i++){
  //   TCanvas *can = (TCanvas*) gROOT->FindObject(Form("cantof_timetrailing%d",i+1));
  //   if(can) delete can;
  //   // can=new TCanvas(Form("cantof_timetrailing%d",i+1),Form("cantof_timetrailing%d",i+1),500,500);
  //   // histrepo1d_tof_timetrailing[i]->Draw();
  //   histrepo1d_tof_timetrailing[i]->Write(0,TObject::kOverwrite);
  // }
  // for(int i=0;i<tofnum;i++){
  //   TCanvas *can = (TCanvas*) gROOT->FindObject(Form("cantof_trailing%d",i+1));
  //   if(can) delete can;
  //   // can=new TCanvas(Form("cantof_trailing%d",i+1),Form("cantof_trailing%d",i+1),500,500);
  //   // histrepo1d_tof_trailing[i]->Draw();
  //   histrepo1d_tof_trailing[i]->Write(0,TObject::kOverwrite);
  // }
  // for(int i=0;i<tofnum;i++){
  //   TCanvas *can = (TCanvas*) gROOT->FindObject(Form("cantof_leading%d",i+1));
  //   if(can) delete can;
  //   // can=new TCanvas(Form("cantof_leading%d",i+1),Form("cantof_leading%d",i+1),500,500);
  //   // histrepo1d_tof_leading[i]->Draw();
  //   histrepo1d_tof_leading[i]->Write(0,TObject::kOverwrite);
  // }
  // for(int i=0;i<tofnum;i++){
  //   TCanvas *can = (TCanvas*) gROOT->FindObject(Form("cantof_timeleading%d",i+1));
  //   if(can) delete can;
  //   // can=new TCanvas(Form("cantof_timeleading%d",i+1),Form("cantof_timeleading%d",i+1),500,500);
  //   // histrepo1d_tof_timeleading[i]->Draw();
  //   histrepo1d_tof_timeleading[i]->Write(0,TObject::kOverwrite);
  // }
  // for(int i=0;i<tofnum;i++){
  //   TCanvas *can = (TCanvas*) gROOT->FindObject(Form("cantof_tot%d",i+1));
  //   if(can) delete can;
  //   // can=new TCanvas(Form("cantof_tot%d",i+1),Form("cantof_tot%d",i+1),500,500);
  //   // histrepo1d_tof_tot[i]->Draw();
  //   histrepo1d_tof_tot[i]->Write(0,TObject::kOverwrite);
  // }
  // for(int i=0;i<tofnum;i++){
  //   TCanvas *can = (TCanvas*) gROOT->FindObject(Form("cantof_timetot%d",i+1));
  //   if(can) delete can;
  //   // can=new TCanvas(Form("cantof_timetot%d",i+1),Form("cantof_timetot%d",i+1),500,500);
  //   // histrepo1d_tof_timetot[i]->Draw();
  //   histrepo1d_tof_timetot[i]->Write(0,TObject::kOverwrite);
  // }

  for(int i=0;i<mwdcnum;i++){
    TCanvas *can = (TCanvas*) gROOT->FindObject(Form("canmwdc_leading_vs_trailing%d",i+1));
    if(can) delete can;
    can=new TCanvas(Form("canmwdc_leading_vs_trailing%d",i+1),Form("canmwdc_leading_vs_trailing%d",i+1),500,500);
    h2d_mwdc_leading_vs_trailing[i]->Draw();
    h2d_mwdc_leading_vs_trailing[i]->Write(0,TObject::kOverwrite);
  }
  // for(int i=0;i<tofnum;i++){
  //   TCanvas *can = (TCanvas*) gROOT->FindObject(Form("cantof_timeleading_vs_timetrailing%d",i+1));
  //   if(can) delete can;
  //   can=new TCanvas(Form("cantof_timeleading_vs_timetrailing%d",i+1),Form("cantof_timeleading_vs_timetrailing%d",i+1),500,500);
  //   h2d_tof_timeleading_vs_timetrailing[i]->Draw();
  // }
  // for(int i=0;i<tofnum;i++){
  //   TCanvas *can = (TCanvas*) gROOT->FindObject(Form("cantof_leading_vs_trailing%d",i+1));
  //   if(can) delete can;
  //   can=new TCanvas(Form("cantof_leading_vs_trailing%d",i+1),Form("cantof_leading_vs_trailing%d",i+1),500,500);
  //   h2d_tof_leading_vs_trailing[i]->Draw();
  // }

  delete file_out;
  delete [] tree_in_mwdc;
  delete [] mwdc_boardinfo;
  delete [] mwdc_eventflag;
  delete [] mwdc_eventid;
  delete [] mwdc_bunchid;
  delete [] mwdc_leading_raw;
  delete [] mwdc_trailing_raw;
  
  delete [] tree_in_tof;
  delete [] tof_boardinfo;
  delete [] tof_eventflag;
  delete [] tof_eventid;
  delete [] tof_bunchid;
  delete [] tof_timeleading_raw;
  delete [] tof_timetrailing_raw;
  delete [] tof_totleading_raw;
  delete [] tof_tottrailing_raw;
  
  delete [] tree_in;
  delete [] boardinfo;
  //
  delete info;
  
  return 0;
}
