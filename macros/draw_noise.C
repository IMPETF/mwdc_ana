#include "TTree.h"
#include "TFile.h"
#include "TROOT.h"
#include "TCanvas.h"
#include "TH1F.h"
#include "CrateInfo.h"
#include "BoardInfo.h"
#include "utility.h"
//
#include "TStyle.h"
#include "TH2F.h"


int draw_noise_raw(const char* datadir,const char* outfile)
{
  //
  std::vector<TH1F*> histrepo_mwdc,histrepo_mwdc_weight,histrepo_mwdc_ratio;
  std::vector<TH1F*> histrepo_tof,histrepo_tof_weight,histrepo_tof_ratio;
  //readin the config file which include channelmapping info
  TString file_config=TString(datadir)+"/crate.json";
  CrateInfo* info=Utility::read_config(file_config.Data(),"mapping");
  info->Print();
  //check the structure of root file,check the consitency between root file and config file
  TString file_data=TString(datadir)+"/"+outfile;  
  TFile* file_out=new TFile(file_data);
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
  UInt_t*	mwdc_triggerid=new UInt_t[mwdcnum]{};
  Int_t*	mwdc_bunchid=new Int_t[mwdcnum]{};
  ChannelMap** 	mwdc_leading_raw=new ChannelMap*[mwdcnum]{};
  ChannelMap** 	mwdc_trailing_raw=new ChannelMap*[mwdcnum]{};
  
  TTree** 	tree_in_tof=new TTree*[tofnum]{};
  BoardInfo**	tof_boardinfo=new BoardInfo*[tofnum]{};
  UInt_t*	tof_triggerid=new UInt_t[tofnum]{};
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
	tree_in[i]->SetBranchAddress("trigger_id",&mwdc_triggerid[mwdcnum-1]);
	tree_in[i]->SetBranchAddress("bunch_id",&mwdc_bunchid[mwdcnum-1]);
	tree_in[i]->SetBranchAddress("leading_raw",&mwdc_leading_raw[mwdcnum-1]);
	tree_in[i]->SetBranchAddress("trailing_raw",&mwdc_trailing_raw[mwdcnum-1]);
	histrepo_mwdc.push_back(new TH1F(TString("h")+boardinfo[i]->GetName(),boardinfo[i]->GetTitle(),128,0.5,128.5));
	histrepo_mwdc_weight.push_back(new TH1F(TString("h")+boardinfo[i]->GetName()+"_weight",boardinfo[i]->GetTitle()+TString("_weight"),128,0.5,128.5));
	histrepo_mwdc_ratio.push_back(new TH1F(TString("h")+boardinfo[i]->GetName()+"_ratio",boardinfo[i]->GetTitle()+TString("_ratio"),128,0.5,128.5));
	break;
      case ETOF:
	raw_dir->GetObject(boardinfo[i]->GetName(),tree_in_tof[tofnum++]);
	tof_boardinfo[tofnum-1]=boardinfo[i];
	tree_in[i]=tree_in_tof[tofnum-1];	
	tree_in[i]->SetBranchAddress("trigger_id",&tof_triggerid[tofnum-1]);
	tree_in[i]->SetBranchAddress("bunch_id",&tof_bunchid[tofnum-1]);
	tree_in[i]->SetBranchAddress("time_leading_raw",&tof_timeleading_raw[tofnum-1]);
	tree_in[i]->SetBranchAddress("time_trailing_raw",&tof_timetrailing_raw[tofnum-1]);
	tree_in[i]->SetBranchAddress("tot_leading_raw",&tof_totleading_raw[tofnum-1]);
	tree_in[i]->SetBranchAddress("tot_trailing_raw",&tof_tottrailing_raw[tofnum-1]);
	histrepo_tof.push_back(new TH1F(TString("h")+boardinfo[i]->GetName(),boardinfo[i]->GetTitle(),16,0.5,16.5));
	histrepo_tof_weight.push_back(new TH1F(TString("h")+boardinfo[i]->GetName()+"_weight",boardinfo[i]->GetTitle()+TString("_weight"),16,0.5,16.5));
	histrepo_tof_ratio.push_back(new TH1F(TString("h")+boardinfo[i]->GetName()+"_ratio",boardinfo[i]->GetTitle()+TString("_ratio"),16,0.5,16.5));	
	break;
      default:
	break;
    }
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
  
  ChannelMap::iterator it;  
  //start merge loop
  for(int i=0;i<entries;i++){
    if(!((i+1)%5000)){
      printf("%d events processed\n",i+1);
    }
    //
    for(int j=0;j<boardnum;j++){
      tree_in[j]->GetEntry(i);
    }
    //process
    for(int j=0;j<mwdcnum;j++){
      for(it=mwdc_leading_raw[j]->begin();it!=mwdc_leading_raw[j]->end();it++){
	histrepo_mwdc[j]->Fill(it->first+1);
	histrepo_mwdc_weight[j]->Fill(it->first+1,it->second.size());
      }
      for(it=mwdc_trailing_raw[j]->begin();it!=mwdc_trailing_raw[j]->end();it++){
	histrepo_mwdc[j]->Fill(it->first+1);
	histrepo_mwdc_weight[j]->Fill(it->first+1,it->second.size());
      }
    }
    //
    for(int j=0;j<tofnum;j++){
      for(it=tof_timeleading_raw[j]->begin();it!=tof_timeleading_raw[j]->end();it++){
	histrepo_tof[j]->Fill(it->first+1);
	histrepo_tof_weight[j]->Fill(it->first+1,it->second.size());
      }
      for(it=tof_timetrailing_raw[j]->begin();it!=tof_timetrailing_raw[j]->end();it++){
	histrepo_tof[j]->Fill(it->first+1);
	histrepo_tof_weight[j]->Fill(it->first+1,it->second.size());
      }
      for(it=tof_totleading_raw[j]->begin();it!=tof_totleading_raw[j]->end();it++){
	//histrepo_tof[j]->Fill(it->first+1);
	//histrepo_tof_weight[j]->Fill(it->first+1,it->second.size());
      }
      for(it=tof_tottrailing_raw[j]->begin();it!=tof_tottrailing_raw[j]->end();it++){
	//histrepo_tof[j]->Fill(it->first+1);
	//histrepo_tof_weight[j]->Fill(it->first+1,it->second.size());
      }
    }
  }
  
  printf("%d events processed totally!\n",entries);
  //
  TCanvas* cmwdc=new TCanvas("cmwdc","cmwdc",1200,900);
  cmwdc->Divide(mwdcnum,3);
  for(int i=0;i<mwdcnum;i++){
    cmwdc->cd(i+1);
    histrepo_mwdc[i]->DrawCopy();
  }
  for(int i=0;i<mwdcnum;i++){
    cmwdc->cd(mwdcnum+i+1);
    histrepo_mwdc_weight[i]->DrawCopy();
  }
  for(int i=0;i<mwdcnum;i++){
    cmwdc->cd(2*mwdcnum+i+1);
    histrepo_mwdc_ratio[i]->Divide(histrepo_mwdc_weight[i],histrepo_mwdc[i]);
    histrepo_mwdc_ratio[i]->DrawCopy();
  }
  TCanvas* ctof=new TCanvas("ctof","ctof",1200,900);
  ctof->Divide(tofnum,3);
  for(int i=0;i<tofnum;i++){
    ctof->cd(i+1);
    histrepo_tof[i]->DrawCopy();
  }
  for(int i=0;i<tofnum;i++){
    ctof->cd(tofnum+i+1);
    histrepo_tof_weight[i]->DrawCopy();
  }
  for(int i=0;i<tofnum;i++){
    ctof->cd(2*tofnum+i+1);
    histrepo_tof_ratio[i]->Divide(histrepo_tof_weight[i],histrepo_tof[i]);
    histrepo_tof_ratio[i]->DrawCopy();
  }
  
  //
  delete file_out;
  delete [] tree_in_mwdc;
  delete [] mwdc_boardinfo;
  delete [] mwdc_triggerid;
  delete [] mwdc_bunchid;
  delete [] mwdc_leading_raw;
  delete [] mwdc_trailing_raw;
  
  delete [] tree_in_tof;
  delete [] tof_boardinfo;
  delete [] tof_triggerid;
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

int draw_noise_merge(const char* datadir,const char* outfile)
{
  gStyle->SetOptStat(111111);
  //
  TString label_location[2]={"Down","Up"};
  TString label_direction[3]={"X","Y","U"};
  //leading_hitnum distribution of each channel
  TH1* htemp;TH2* htemp2d;
  std::vector<TH1F*> histrepo_mwdc[2],histrepo_mwdc_weight[2],histrepo_mwdc_ratio[2];
  std::vector<TH2F*> histrepo_dist[2];
  for(int i=0;i<2;i++){
    for(int j=0;j<3;j++){
      htemp=(TH1*)gROOT->FindObject("h"+label_direction[j]+"_"+label_location[i]);
      if(htemp) delete htemp;
      histrepo_mwdc[i].push_back(new TH1F("h"+label_direction[j]+"_"+label_location[i],label_direction[j]+"_"+label_location[i],108,-0.5,107.5));
      
      htemp=(TH1*)gROOT->FindObject("h"+label_direction[j]+"_"+label_location[i]+"_weight");
      if(htemp) delete htemp;
      histrepo_mwdc_weight[i].push_back(new TH1F("h"+label_direction[j]+"_"+label_location[i]+"_weight",label_direction[j]+"_"+label_location[i]+"_weight",108,-0.5,107.5));
      
      htemp=(TH1*)gROOT->FindObject("h"+label_direction[j]+"_"+label_location[i]+"_ratio");
      if(htemp) delete htemp;
      histrepo_mwdc_ratio[i].push_back(new TH1F("h"+label_direction[j]+"_"+label_location[i]+"_ratio",label_direction[j]+"_"+label_location[i]+"_ratio",108,-0.5,107.5));
      //
      htemp2d=(TH2*)gROOT->FindObject("h"+label_direction[j]+"_"+label_location[i]+"_dist");
      if(htemp2d) delete htemp2d;
      histrepo_dist[i].push_back(new TH2F("h"+label_direction[j]+"_"+label_location[i]+"_dist",label_direction[j]+"_"+label_location[i]+"_dist",108,-0.5,107.5,10,-0.5,9.5));
    }
  }
  //
  TH1F*	hist_distall_leading,*hist_distall_trailing;
  htemp=(TH1*)gROOT->FindObject("hdist_all_leading");
  if(htemp) delete htemp;
  hist_distall_leading=new TH1F("hdist_all_leading","hdist_all_leading",12,-0.5,11.5);
  
  htemp=(TH1*)gROOT->FindObject("hdist_all_trailing");
  if(htemp) delete htemp;
  hist_distall_trailing=new TH1F("hdist_all_trailing","hdist_all_trailing",12,-0.5,11.5);

  TH2F* h2d_leading_vs_trailing;
  htemp2d=(TH2*)gROOT->FindObject("h2d_leading_vs_trailing");
  if(htemp2d) delete htemp2d;
  h2d_leading_vs_trailing=new TH2F("h2d_leading_vs_trailing","h2d_leading_vs_trailing",10,-0.5,9.5,10,-0.5,10.5);
  //leading_hitnum-trailing_hitnum in a hptdc channel
  std::vector<TH1F*> histrepo_sub[2];
  for(int i=0;i<2;i++){
    for(int j=0;j<3;j++){
      htemp=(TH1*)gROOT->FindObject("h"+label_direction[j]+"_"+label_location[i]+"_"+"subtract");
      if(htemp)	delete htemp;
      histrepo_sub[i].push_back(new TH1F("h"+label_direction[j]+"_"+label_location[i]+"_"+"subtract",label_direction[j]+"_"+label_location[i]+"_"+"subtract",17,-8.5,8.5));
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
  ChannelMap *tof_timeleading=0,*tof_timetrailing=0,*tof_totleading=0,*tof_tottrailing=0;
  tree_tof->SetBranchAddress("time_leading_raw",&tof_timeleading);
  tree_tof->SetBranchAddress("time_trailing_raw",&tof_timetrailing);
  tree_tof->SetBranchAddress("tot_leading_raw",&tof_totleading);
  tree_tof->SetBranchAddress("tot_trailing_raw",&tof_tottrailing);
  //
  int entries=tree_mwdc->GetEntriesFast();
  ChannelMap::iterator it;
  ChannelMap::iterator it_trailing;
  UChar_t type,location,direction;
  UShort_t index;
  for(int i=0;i<entries;i++){
  //for(int i=0;i<100;i++){
    if(!((i+1)%5000)){
      printf("%d events analyzed\n",i+1);
    }
    tree_mwdc->GetEntry(i);
    tree_tof->GetEntry(i);
    //
    for(it=mwdc_leading->begin();it!=mwdc_leading->end();it++){
      Encoding::Decode(it->first,type,location,direction,index);
      if (type!=EMWDC) {
	printf("event_%d:MWDC unmatched type\n",i+1);
      }
      histrepo_mwdc[location][direction]->Fill(index+1);
      histrepo_mwdc_weight[location][direction]->Fill(index+1,it->second.size());
      //
      histrepo_dist[location][direction]->Fill(index+1,it->second.size());
      hist_distall_leading->Fill(it->second.size());
      //
      it_trailing=mwdc_trailing->find(it->first);
      if(it_trailing!=mwdc_trailing->end()){
	//std::vector::size() return a unsigned int type,so need to cast to int before substract
	histrepo_sub[location][direction]->Fill((int)it->second.size()-(int)it_trailing->second.size());
	h2d_leading_vs_trailing->Fill(it->second.size(),it_trailing->second.size());
      }
      else{
	histrepo_sub[location][direction]->Fill(it->second.size());
	h2d_leading_vs_trailing->Fill(it->second.size(),0);
      }
    }
    for(it=mwdc_trailing->begin();it!=mwdc_trailing->end();it++){
      Encoding::Decode(it->first,type,location,direction,index);
      if (type!=EMWDC) {
	printf("event_%d:MWDC unmatched type\n",i+1);
      }
      hist_distall_trailing->Fill(it->second.size());
      //
      it_trailing=mwdc_leading->find(it->first);
      if(it_trailing==mwdc_leading->end()){
	histrepo_sub[location][direction]->Fill(-((int)it->second.size()));
	h2d_leading_vs_trailing->Fill(0.0,(float)it->second.size());
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
  TCanvas *can = (TCanvas*) gROOT->FindObject("can");
  if(can) delete can;
  can=new TCanvas("can","can",900,600);
  can->Divide(3,2,0,0,3);
  for(int i=0;i<2;i++){
    for(int j=0;j<3;j++){
      can->cd(3*i+j+1);
      histrepo_mwdc_ratio[i][j]->Divide(histrepo_mwdc_weight[i][j],histrepo_mwdc[i][j]);
      htemp=histrepo_mwdc_ratio[i][j]->DrawCopy();
      htemp->SetMinimum(0.95);
      htemp->SetMaximum(1.45);
      //htemp->GetYaxis()->SetRangeUser(1.05,1.25);
      histrepo_mwdc_ratio[i][j]->Write(0,TObject::kOverwrite);
      histrepo_mwdc[i][j]->Write(0,TObject::kOverwrite);
    }
  }
  TCanvas *can2 = (TCanvas*) gROOT->FindObject("can2");
  if(can2) delete can2;
  can2=new TCanvas("can2","can2",900,600);
  can2->Divide(3,2,0,0,3);
  for(int i=0;i<2;i++){
    for(int j=0;j<3;j++){
      can2->cd(3*i+j+1);
      gPad->SetLogz();
      histrepo_dist[i][j]->DrawCopy("lego2z");
      gDirectory->Print();
      histrepo_dist[i][j]->Write(0,TObject::kOverwrite);
    }
  }
  //
  TCanvas *can3 = (TCanvas*) gROOT->FindObject("can3");
  if(can3) delete can3;
  can3=new TCanvas("can3","can3",600,600);
  can3->SetLogy();
  hist_distall_leading->DrawCopy();
  hist_distall_leading->Write(0,TObject::kOverwrite);
  
  htemp=hist_distall_trailing->DrawCopy("same");
  htemp->SetLineColor(kRed);
  htemp=(TH1*)hist_distall_leading->Clone("hsub");
  htemp->Add(hist_distall_trailing,hist_distall_leading,1,-1);
  htemp->SetLineColor(kGreen);
  htemp->DrawCopy("same");
  can3->BuildLegend();
  hist_distall_trailing->Write(0,TObject::kOverwrite);
  //
  Int_t nx=3,ny=2;
  TCanvas *can4 = (TCanvas*) gROOT->FindObject("can4");
  if(can4) delete can4;
  can4=new TCanvas("can4","can4",300*nx,300*ny);
  can4->Divide(nx,ny,0,0,3);
  for(int i=0;i<ny;i++){
    for(int j=0;j<nx;j++){
      can4->cd(nx*i+j+1);
      gPad->SetLogy();
      histrepo_sub[i][j]->DrawCopy();
      histrepo_sub[i][j]->Write(0,TObject::kOverwrite);
    }
  }
  //
  TCanvas *can5 = (TCanvas*) gROOT->FindObject("can5");
  if(can5) delete can5;
  can5=new TCanvas("can5","can5",600,600);
  //can5->SetLogy();
  htemp2d=(TH2*)h2d_leading_vs_trailing->DrawCopy("text");
  htemp2d->GetXaxis()->SetTitle("leading");
  htemp2d->GetYaxis()->SetTitle("trailing");
  h2d_leading_vs_trailing->Write(0,TObject::kOverwrite);
  //
  delete file_out;
  
  return 0;
}