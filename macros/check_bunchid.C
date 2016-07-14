/*   $Id: check_bunchid.h, 2016-07-12 11:09:29+08:00 MWDC_ana Raw$
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
#include "TMath.h"
#include "TH2F.h"

int check_bunchid(const char* datadir,const char* outfile)
{
  gStyle->SetOptStat(111111);

  const Double_t typical_percentage[9]={0.00731,0.00761,0.00250,0.01944,0.02810,0.00340,0,0.03355,0.03274};

  TH1F* h1eventsep=new TH1F("h1eventsep","BunchID Unsync Event Separation",501,-0.5,500.5);
  TH1F* h1bunchsep=new TH1F("h1bunchsep","BunchID Unsync MeanBunchID Diff",200,-4095.5,4095.5);
  TH2F* h2eventsep_vs_bunchsep=new TH2F("h2eventsep_vs_bunchsep","BunchID Unsync Event: EventInde Separation VS BunchID Separation",501,-0.5,500.5,200,-4095.5,4095.5);
  TH1F* h1bunchsep_uncontinuous=new TH1F("h1bunchsep_uncontinuous","Uncontinuous BunchID Unsync MeanBunchID Diff",200,-4095.5,4095.5);
  TH1F* h1bunchdist=new TH1F("h1bunchdist","BunchID Unsync MeanBunchID Dist",4096,-0.5,4095.5);
  TH2F* h2bunchdiff_vs_cardnum=new TH2F("h2bunchdiff_vs_cardnum","BunchID Unsync Event: BunchID Offset VS Cards",12,0.5,12.5,11,-5.5,5.5);
  TH1F* h1bunchmaxoffset=new TH1F("h1bunchmaxoffset","BunchID Unsync Event Max BunchID Offset",4096,-0.5,4095.5);
  //read the config file which include channelmapping info
  TString file_config=TString(datadir)+"../../crate.json";
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

  // // read merge ttree
  // TTree *tree_mwdc,*tree_tof;
  // file_out->GetObject("merge/mwdc",tree_mwdc);
  // file_out->GetObject("merge/tof",tree_tof);
  
  // ChannelMap *mwdc_leading=0,*mwdc_trailing=0;
  // tree_mwdc->SetBranchAddress("leading_raw",&mwdc_leading);
  // tree_mwdc->SetBranchAddress("trailing_raw",&mwdc_trailing);
  // // Activate the referenced branches
  // tree_mwdc->SetBranchStatus("*",0);
  // //tree_mwdc->SetBranchStatus("leading_raw",1);
  // //tree_mwdc->SetBranchStatus("trailing_raw",1);

  // ChannelMap *tof_timeleading=0,*tof_timetrailing=0,*tof_totleading=0,*tof_tottrailing=0;
  // tree_tof->SetBranchAddress("time_leading_raw",&tof_timeleading);
  // tree_tof->SetBranchAddress("time_trailing_raw",&tof_timetrailing);
  // tree_tof->SetBranchAddress("tot_leading_raw",&tof_totleading);
  // tree_tof->SetBranchAddress("tot_trailing_raw",&tof_tottrailing);
  // // Activate the referenced branches
  // tree_tof->SetBranchStatus("*",0);
  // //tree_tof->SetBranchStatus("time_leading_raw",1);
  // //tree_tof->SetBranchStatus("time_trailing_raw",1);
  // //tree_tof->SetBranchStatus("tot_leading_raw",1);
  // //tree_tof->SetBranchStatus("tot_trailing_raw",1);
  
  //read raw ttree
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
  //tree_in[i]->SetBranchStatus("event_id",1);
  tree_in[i]->SetBranchStatus("bunch_id",1);
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
    histrepo1d_mwdc.push_back(new TH1F(TString("h")+boardinfo[i]->GetName()+TString("unsync_bunchid_eventindex_separation"),boardinfo[i]->GetTitle()+TString("unsync_bunchid_eventindex_separation"),500,-0.5,499.5));
    histrepo1d_mwdc[i]->SetDirectory(0);
  }
  std::vector<TH1F*> histrepo1d_tof;
  for(int i=0;i<tofnum;i++){
    histrepo1d_tof.push_back(new TH1F(TString("h")+boardinfo[mwdcnum+i]->GetName()+TString("unsync_bunchid_eventindex_separation"),boardinfo[mwdcnum+i]->GetTitle()+TString("unsync_bunchid_eventindex_separation"),500,-0.5,499.5));
    histrepo1d_tof[i]->SetDirectory(0);
  }
  //
  Int_t max_bunchid,min_bunchid;
  Int_t max_mwdc_bunchid,min_mwdc_bunchid,max_tof_bunchid,min_tof_bunchid;
  Int_t* init_mwdc_bunchid=new Int_t[mwdcnum]{};
  Int_t* init_tof_bunchid=new Int_t[tofnum]{};
  Int_t* bunchid=new Int_t[boardnum]{};
  const Int_t  testnum=5;
  Int_t test_bunchid[testnum];
  Int_t test_index[testnum]={3,7,6,2,4};
  Int_t test_median_bunchid;

  const Int_t expectednum=9;
  Int_t expected_index[expectednum]={8,9,1,3,7,6,2,4,5};
  Int_t expected_bunchid[expectednum];
  Int_t max_lowhalf_bunchid,min_lowhalf_bunchid,max_highhalf_bunchid,min_highhalf_bunchid;
  Int_t separation_index;

  Int_t* previous_mwdc_bunchid=new Int_t[mwdcnum]{};
  Int_t* previous_tof_bunchid=new Int_t[tofnum]{};
  Double_t previous_mean_bunchid,mean_bunchid,median_bunchid;
  Int_t* mwdc_unsync_eventnum= new Int_t[mwdcnum]{};
  Int_t* tof_unsync_eventnum= new Int_t[tofnum]{};
  Int_t previous_event_index=0;
  Int_t* mwdc_previous_event_index=new Int_t[mwdcnum]{};
  Int_t* tof_previous_event_index=new Int_t[tofnum]{};
  Bool_t init_unsync_flag=true;
  Bool_t* mwdc_init_unsync_flag=new Bool_t[mwdcnum]{};
  Bool_t* tof_init_unsync_flag=new Bool_t[tofnum]{};
  Int_t continuous_start,continuous_stop;
  Bool_t continuous_flag=false;
  //init
  Int_t temp_entries;
  Int_t entries=tree_in[0]->GetEntriesFast();
  for(int i=0;i<boardnum;i++){
    temp_entries=tree_in[i]->GetEntriesFast();
    if(temp_entries<entries){
      entries=temp_entries;
    }
  }
  //
  TH1F* h1eventindex=new TH1F("h1eventindex","Unsync BunchID EventIndex",entries,-0.5,entries-0.5);
  h1eventindex->SetDirectory(0);
  TH1F* h1continuousevents_length=new TH1F("h1continuousevents_length","Unsync BunchID ContinuousEvents_Length",100,-0.5,99.5);
  h1continuousevents_length->SetDirectory(0);

  for(int j=0;j<boardnum;j++){
    tree_in[j]->GetEntry(0);
  }
  for(int j=0;j<mwdcnum;j++){
    init_mwdc_bunchid[j]=mwdc_bunchid[j];
    mwdc_init_unsync_flag[j]=true;
  }
  for(int j=0;j<tofnum;j++){
    init_tof_bunchid[j]=tof_bunchid[j];
    tof_init_unsync_flag[j]=true;
  }
  //for(int i=0;i<100;i++){
  for(int i=0;i<entries;i++){
    if(!((i+1)%100000)){
      printf("%d events processed\n",i+1);
    }
    //
    for(int j=0;j<boardnum;j++){
      tree_in[j]->GetEntry(i);
    }
    for(int j=0;j<mwdcnum;j++){
      bunchid[j]=mwdc_bunchid[j];
    }
    for(int j=0;j<tofnum;j++){
      bunchid[j+mwdcnum]=tof_bunchid[j];
    }
    //
    for(int j=0;j<testnum;j++){
      test_bunchid[j]=bunchid[test_index[j]-1];
    }
    for(int j=0;j<expectednum;j++){
      expected_bunchid[j]=bunchid[expected_index[j]-1];
    }
    //process
    max_mwdc_bunchid=TMath::MaxElement(mwdcnum,mwdc_bunchid);
    min_mwdc_bunchid=TMath::MinElement(mwdcnum,mwdc_bunchid);

    max_tof_bunchid=TMath::MaxElement(tofnum,tof_bunchid);
    min_tof_bunchid=TMath::MinElement(tofnum,tof_bunchid);

    if(max_mwdc_bunchid>=max_tof_bunchid){
      max_bunchid=max_mwdc_bunchid;
    }
    else{
      max_bunchid=max_tof_bunchid;
    }

    if(min_mwdc_bunchid <= min_tof_bunchid){
      min_bunchid=min_mwdc_bunchid;
    }
    else{
      min_bunchid=min_tof_bunchid;
    }

    if(max_bunchid != min_bunchid){
      // printf("Event %d:\n", i);
      // for(int j=0;j<mwdcnum;j++){
      //   printf("%d\t", mwdc_bunchid[j]);
      // }
      // for(int j=0;j<tofnum;j++){
      //   printf("%d\t", tof_bunchid[j]);
      // }
      // printf("\n");
      
      if(init_unsync_flag){
        init_unsync_flag=false;
      }
      else{
        // mean_bunchid=TMath::Mean(boardnum,bunchid);
        mean_bunchid=TMath::Median(boardnum,bunchid);
        h1bunchsep->Fill(mean_bunchid - previous_mean_bunchid);
        h1eventsep->Fill(i - previous_event_index);
        h2eventsep_vs_bunchsep->Fill(i - previous_event_index,mean_bunchid - previous_mean_bunchid);
        if((i- previous_event_index)>1){
          h1bunchsep_uncontinuous->Fill(mean_bunchid - previous_mean_bunchid);
          //
          if(continuous_flag==true){
            h1continuousevents_length->Fill(continuous_stop - continuous_start);
          }
          continuous_flag=false;
          continuous_start=i;
        }
        else{
          continuous_flag=true;
          continuous_stop=i;
        }
      }
      //
      previous_event_index=i;
      for(int j=0;j<mwdcnum;j++){
        previous_mwdc_bunchid[j]=mwdc_bunchid[j];
      }
      for(int j=0;j<tofnum;j++){
        previous_tof_bunchid[j]=tof_bunchid[j];
      }
      // previous_mean_bunchid=TMath::Mean(boardnum,bunchid);
      previous_mean_bunchid=TMath::Median(boardnum,bunchid);
      //
      h1eventindex->Fill(i);
      h1bunchdist->Fill(previous_mean_bunchid);
      //
      median_bunchid=TMath::Median(boardnum,bunchid);
      // median_bunchid=TMath::Median(mwdcnum,mwdc_bunchid);
      // median_bunchid=TMath::Median(testnum,test_bunchid);
      for(int j=0;j<mwdcnum;j++){
        h2bunchdiff_vs_cardnum->Fill(j+1,mwdc_bunchid[j]- static_cast<int>(median_bunchid),1./entries);
        if(mwdc_bunchid[j] != median_bunchid){
          if(mwdc_bunchid[j] > median_bunchid){
            mwdc_unsync_eventnum[j]++;
          }
          else{
            mwdc_unsync_eventnum[j]--;
          }
          
          if(mwdc_init_unsync_flag[j]){
            mwdc_previous_event_index[j]=i;
            mwdc_init_unsync_flag[j]=false;
          }

          histrepo1d_mwdc[j]->Fill(i- mwdc_previous_event_index[j]);
          mwdc_previous_event_index[j]=i;
        }
      }
      for(int j=0;j<tofnum;j++){
        h2bunchdiff_vs_cardnum->Fill(mwdcnum+j+3,tof_bunchid[j]-static_cast<int>(median_bunchid),1./entries);
        if(tof_bunchid[j] != median_bunchid){
          if(tof_bunchid[j] > median_bunchid){
            tof_unsync_eventnum[j]++;
          }
          else{
            tof_unsync_eventnum[j]--;
          }

          if(tof_init_unsync_flag[j]){
            tof_previous_event_index[j]=i;
            tof_init_unsync_flag[j]=false;
          }

          histrepo1d_tof[j]->Fill(i- tof_previous_event_index[j]);
          tof_previous_event_index[j]=i;
        }
      }
      //
      h1bunchmaxoffset->Fill(max_bunchid-min_bunchid);
      //validation
      separation_index=TMath::LocMax(expectednum,expected_bunchid);//LocMax returns the index of the first maximum element
      // roll-over events correct
      if(separation_index==0 && expected_bunchid[0]==(g_range_bunchid-1)){
        separation_index=TMath::LocMin(expectednum,expected_bunchid);

        max_lowhalf_bunchid=TMath::MaxElement(separation_index,expected_bunchid);
        min_lowhalf_bunchid=TMath::MinElement(separation_index,expected_bunchid);
        max_highhalf_bunchid=TMath::MaxElement(expectednum- separation_index, expected_bunchid+separation_index)+ g_range_bunchid ;
        min_highhalf_bunchid=TMath::MinElement(expectednum - separation_index, expected_bunchid+separation_index) + g_range_bunchid;
      }
      else{
        max_lowhalf_bunchid=TMath::MaxElement(separation_index,expected_bunchid);
        min_lowhalf_bunchid=TMath::MinElement(separation_index,expected_bunchid);
        max_highhalf_bunchid=TMath::MaxElement(expectednum- separation_index, expected_bunchid+separation_index);
        min_highhalf_bunchid=TMath::MinElement(expectednum - separation_index, expected_bunchid+separation_index);
      }

      if((max_lowhalf_bunchid != min_lowhalf_bunchid) || (max_highhalf_bunchid != min_highhalf_bunchid)){
        printf("ERROR: bunchid validation failed!\n" );

        printf("\tEvent_%d Current BunchID:\n\t",i);
        // for(int j=0;j<mwdcnum;j++){
        //   printf("%d\t", mwdc_bunchid[j]);
        // }
        // for(int j=0;j<tofnum;j++){
        //   printf("%d\t", tof_bunchid[j]);
        // }
        for(int j=0;j<expectednum;j++){
          printf("%d\t",expected_bunchid[j]);
        }
        printf("\n");
        
        // exit(1);
      }
      else if((max_highhalf_bunchid-1) != max_lowhalf_bunchid){
        printf("ERROR: bunchid offset should only be 1(%d)\n",max_highhalf_bunchid- max_lowhalf_bunchid);
      }
    }
  }
  // 
  if(continuous_flag){
    h1continuousevents_length->Fill(entries-1 - continuous_start);
  }
  
  printf("%d events processed totally!\n",entries);

  printf("Init BunchID:\n");
  for(int j=0;j<mwdcnum;j++){
    printf("%d\t", init_mwdc_bunchid[j]);
  }
  for(int j=0;j<tofnum;j++){
    printf("%d\t", init_tof_bunchid[j]);
  }
  printf("\n");
  
  printf("Unsync BunchID Percent:\n");
  for(int j=0;j<mwdcnum;j++){
    // printf("%.5f\t", mwdc_unsync_eventnum[j]/(float)entries - typical_percentage[j]);
    printf("%.5f\t", mwdc_unsync_eventnum[j]/(float)entries);
  }
  for(int j=0;j<tofnum;j++){
    // printf("%.5f\t", tof_unsync_eventnum[j]/(float)entries- typical_percentage[mwdcnum+j]);
    printf("%.5f\t", tof_unsync_eventnum[j]/(float)entries);
  }
  printf("\n");
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
  TCanvas* c=new TCanvas("c1","c1",600,600);
  h1eventsep->Draw();
  c=new TCanvas("c2","c2",600,600);
  h1bunchsep->Draw();
  c=new TCanvas("c3","c3",600,600);
  h1bunchsep_uncontinuous->Draw();
  c=new TCanvas("c4","c4",600,600);
  h1eventindex->Draw();
  c=new TCanvas("c5","c5",600,600);
  h1continuousevents_length->Draw();
  c=new TCanvas("c6","c6",600,600);
  h1bunchdist->Draw();
  c=new TCanvas("c7","c7",600,600);
  h2bunchdiff_vs_cardnum->Draw("colz");
  c=new TCanvas("c8","c8",600,600);
  h1bunchmaxoffset->Draw();
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
  c=new TCanvas("c9","c9",600,600);
  h2eventsep_vs_bunchsep->Draw("colz");
  //
  delete [] init_mwdc_bunchid;
  delete [] init_tof_bunchid;
  delete [] bunchid;
  delete [] previous_tof_bunchid;
  delete [] previous_mwdc_bunchid;
  delete [] mwdc_previous_event_index;
  delete [] tof_previous_event_index;
  delete [] mwdc_init_unsync_flag;
  delete [] tof_init_unsync_flag;

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
