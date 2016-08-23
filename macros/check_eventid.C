/*   $Id: check_eventid.h, 2016-07-12 08:33:45+08:00 MWDC_ana Raw$
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

int check_eventid(const char* datadir,const char* outfile)
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
  tree_in[i]->SetBranchStatus("event_id",1);
  //tree_in[i]->SetBranchStatus("bunch_id",1);
  //tree_in[i]->SetBranchStatus("leading_raw",1);
  //tree_in[i]->SetBranchStatus("trailing_raw",1);

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
  tree_in[i]->SetBranchStatus("event_id",1);
  //tree_in[i]->SetBranchStatus("bunch_id",1);
  //tree_in[i]->SetBranchStatus("time_leading_raw",1);
  //tree_in[i]->SetBranchStatus("time_trailing_raw",1);
  //tree_in[i]->SetBranchStatus("tot_leading_raw",1);
  //tree_in[i]->SetBranchStatus("tot_trailing_raw",1);

	tof_boardinfo[tofnum-1]->Print();
	break;
      default:
	break;
    }
  }

  // make new histogram here
  std::vector<TH1F*> histrepo1d_mwdc;
  for(int i=0;i<mwdcnum;i++){
    histrepo1d_mwdc.push_back(new TH1F(TString("h")+boardinfo[i]->GetName()+TString("eventid_current_previous_diff"),boardinfo[i]->GetTitle()+TString("eventid_current_previous_diff"),10,-4.5,5.5));
    histrepo1d_mwdc[i]->SetDirectory(0);
  }
  std::vector<TH1F*> histrepo1d_tof;
  for(int i=0;i<tofnum;i++){
    histrepo1d_tof.push_back(new TH1F(TString("h")+boardinfo[mwdcnum+i]->GetName()+TString("eventid_current_previous_diff"),boardinfo[mwdcnum+i]->GetTitle()+TString("eventid_current_previous_diff"),10,-4.5,5.5));
    histrepo1d_tof[i]->SetDirectory(0);
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
  
  for(int j=0;j<boardnum;j++){
    tree_in[j]->GetEntry(0);
  }
  Int_t*  mwdc_eventid_previous=new Int_t[mwdcnum]{};
  Int_t*  tof_eventid_previous=new Int_t[tofnum]{};
  printf("Init EventID:\n");
  for(int i=0;i<mwdcnum;i++){
    mwdc_eventid_previous[i]=mwdc_eventid[i];
    printf("%d,", mwdc_eventid[i]);
  }
  for(int i=0;i<tofnum;i++){
    tof_eventid_previous[i]=tof_eventid[i];
    printf("%d,", tof_eventid[i]);
  }

  ChannelMap::iterator it;  
  //for(int i=0;i<100;i++){
  for(int i=1;i<entries;i++){
    if(!((i+1)%5000)){
      printf("%d events processed\n",i+1);
    }
    //
    for(int j=0;j<boardnum;j++){
      tree_in[j]->GetEntry(i);
    }
    //process
    for(int j=0;j<mwdcnum;j++){
      // for(it=mwdc_leading_raw[j]->begin();it!=mwdc_leading_raw[j]->end();it++){
	
      // }
      // for(it=mwdc_trailing_raw[j]->begin();it!=mwdc_trailing_raw[j]->end();it++){

      // }
      if(mwdc_eventid[j] >= mwdc_eventid_previous[j]){
        histrepo1d_mwdc[j]->Fill(mwdc_eventid[j]-mwdc_eventid_previous[j]);
        if(mwdc_eventid[j] != (mwdc_eventid_previous[j]+1)){
          printf("ERROR: event id not continuous\n");
        }
      }
      else{
        histrepo1d_mwdc[j]->Fill(mwdc_eventid[j]+g_range_bunchid-mwdc_eventid_previous[j]);
        if((mwdc_eventid[j]+g_range_bunchid) != (mwdc_eventid_previous[j]+1)){
          printf("ERROR: event id not continuous\n");
        }
      }
      mwdc_eventid_previous[j]=mwdc_eventid[j];
    }

    for(int j=0;j<tofnum;j++){
      // for(it=tof_timeleading_raw[j]->begin();it!=tof_timeleading_raw[j]->end();it++){

      // }
      // for(it=tof_timetrailing_raw[j]->begin();it!=tof_timetrailing_raw[j]->end();it++){

      // }
      // for(it=tof_totleading_raw[j]->begin();it!=tof_totleading_raw[j]->end();it++){

      // }
      // for(it=tof_tottrailing_raw[j]->begin();it!=tof_tottrailing_raw[j]->end();it++){

      // }
      if(tof_eventid[j] >= tof_eventid_previous[j]){
        histrepo1d_tof[j]->Fill(tof_eventid[j]-tof_eventid_previous[j]);
      }
      else{
        histrepo1d_tof[j]->Fill(tof_eventid[j]+g_range_bunchid-tof_eventid_previous[j]);
      }
      tof_eventid_previous[j]=tof_eventid[j];
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
  //
  for(int i=0;i<mwdcnum;i++){
    TCanvas *can = (TCanvas*) gROOT->FindObject(Form("canmwdc_%d",i+1));
    if(can) delete can;
    can=new TCanvas(Form("canmwdc_%d",i+1),Form("canmwdc_%d",i+1),500,500);
    can->SetLogy();
    histrepo1d_mwdc[i]->Draw();
  }
  for(int i=0;i<tofnum;i++){
    TCanvas *can = (TCanvas*) gROOT->FindObject(Form("cantof_%d",i+1));
    if(can) delete can;
    can=new TCanvas(Form("cantof_%d",i+1),Form("cantof_%d",i+1),500,500);
    can->SetLogy();    
    histrepo1d_tof[i]->Draw();
  }
  //
  delete [] mwdc_eventid_previous;
  delete [] tof_eventid_previous;

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
