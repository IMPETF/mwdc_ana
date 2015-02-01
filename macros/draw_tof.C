/*   $Id: draw_tof.h, 2015-01-30 09:48:07+08:00 MWDC_ana $
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

inline int get_index(int small,int large)
{
  if(small>=large){
    printf("arg1 should be smaller than arg2\n");
    //EXIT(-1);
    //return -1;
  }
  int sum=0;
  int step=small;
  for(int i=0;i<step;i++){
    sum+=(7-i);
  }
  return sum+large-small-1;
}

int draw_tof(const char* datadir,const char* outfile)
{
  gStyle->SetOptStat(1111111);
  //
  TString label_location[2]={"Down","Up"};
  //TString label_direction[3]={"X","Y","U"};
  TString label_direction[4]={"1","2","3","4"};
  //define histogram
  std::vector<TH1F*> histrepo[2];
  TH1* htemp;
  for(int i=0;i<2;i++){
    for(int j=0;j<4;j++){
      htemp=(TH1*)gROOT->FindObject("h"+label_direction[j]+"_"+label_location[i]+"_"+"time-tot");
      if(htemp)	{
	delete htemp;
      }
      histrepo[i].push_back(new TH1F("h"+label_direction[j]+"_"+label_location[i]+"_"+"time-tot",label_direction[j]+"_"+label_location[i]+"_"+"time-tot",2001,-1000.5,1000.5));
    }
  }
  //
  std::vector<TH1F*> histreop_tot[2];
  for(int i=0;i<2;i++){
    for(int j=0;j<4;j++){
      htemp=(TH1*)gROOT->FindObject("h"+label_direction[j]+"_"+label_location[i]+"_"+"tot");
      if(htemp)	delete htemp;
      histreop_tot[i].push_back(new TH1F("h"+label_direction[j]+"_"+label_location[i]+"_"+"tot",label_direction[j]+"_"+label_location[i]+"_"+"tot",4000,1000.5,5000.5));
    }
  }
  //
  std::vector<TH1F*> histrepo_tot_diff,histrepo_time_diff;
  for(int i=0;i<8;i++){
    for(int j=(i+1);j<8;j++){
      htemp=(TH1*)gROOT->FindObject("h_"+TString::Format("tot_diff[%d/%d]",i+1,j+1));
      if(htemp)	delete htemp;
      histrepo_tot_diff.push_back(new TH1F("h_"+TString::Format("tot_diff[%d/%d]",i+1,j+1),TString::Format("tot_diff[%d/%d]",i+1,j+1),2001,-1000.5,1000.5));
    }
  }
  for(int i=0;i<8;i++){
    for(int j=(i+1);j<8;j++){
      htemp=(TH1*)gROOT->FindObject("h_"+TString::Format("time_diff[%d/%d]",i+1,j+1));
      if(htemp)	delete htemp;
      histrepo_time_diff.push_back(new TH1F("h_"+TString::Format("time_diff[%d/%d]",i+1,j+1),TString::Format("time_diff[%d/%d]",i+1,j+1),2001,-1000.5,1000.5));
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
  TTree *tree_tof;
  file_out->GetObject("merge/tof",tree_tof);
  
  ChannelMap *tof_timeleading=0,*tof_timetrailing=0,*tof_totleading=0,*tof_tottrailing=0;
  tree_tof->SetBranchAddress("time_leading_raw",&tof_timeleading);
  tree_tof->SetBranchAddress("time_trailing_raw",&tof_timetrailing);
  tree_tof->SetBranchAddress("tot_leading_raw",&tof_totleading);
  tree_tof->SetBranchAddress("tot_trailing_raw",&tof_tottrailing);
  //
  int entries=tree_tof->GetEntriesFast();
  ChannelMap::iterator it,it_found,it_another;
  TDCContainer tdc;
  UChar_t type,location,direction;
  UShort_t index;
  UChar_t type_another,location_another,direction_another;
  UShort_t index_another;
  int tmp1,tmp2;
  
  //for(int i=0;i<100;i++){
  for(int i=0;i<entries;i++){
    if(!((i+1)%5000)){
      printf("%d events analyzed\n",i+1);
    }
    
    tree_tof->GetEntry(i);
    //TOF
    for(it=tof_timeleading->begin();it!=tof_timeleading->end();it++){
      //printf("her1\n");
      Encoding::Decode(it->first,type,location,direction,index);
      if (type!=ETOF) {
	printf("event_%d:TOF unmatched type\n",i+1);
      }
      //
      it_found=tof_totleading->find(it->first);
      if(it_found!=tof_totleading->end()){
	histrepo[location][index]->Fill(((it->second)[0]>>2)-(it_found->second)[0]);
      }
      //
      tmp1=location*4+index;
      for(it_another=it;it_another!=tof_timeleading->end();it_another++){
	//printf("her2\n");
	if(std::distance(it,it_another)>0){
	  //printf("her3\n");
	  
	  Encoding::Decode(it_another->first,type_another,location_another,direction_another,index_another);
	  tmp2=location_another*4+index_another;
	  if(tmp1<tmp2){
	    histrepo_time_diff[get_index(tmp1,tmp2)]->Fill(((it_another->second)[0]>>2)-((it->second)[0]>>2));
	  }
	  else if(tmp1>tmp2){
	    histrepo_time_diff[get_index(tmp2,tmp1)]->Fill(((it->second)[0]>>2)-((it_another->second)[0]>>2));
	  }
	  
	}
	
      }
      
    }
    for(it=tof_totleading->begin();it!=tof_totleading->end();it++){
      Encoding::Decode(it->first,type,location,direction,index);
      if (type!=ETOF) {
	printf("event_%d:TOF unmatched type\n",i+1);
      }
      //
      it_found=tof_tottrailing->find(it->first);
      if(it_found!=tof_tottrailing->end()){
	histreop_tot[location][index]->Fill((it_found->second)[0]-(it->second)[0]);
      }
      //
      tmp1=location*4+index;
      for(it_another=it;it_another!=tof_totleading->end();it_another++){
	if(std::distance(it,it_another)>0){
	  Encoding::Decode(it_another->first,type_another,location_another,direction_another,index_another);
	  tmp2=location_another*4+index_another;
	  if(tmp1<tmp2){
	    histrepo_tot_diff[get_index(tmp1,tmp2)]->Fill((it_another->second)[0]-(it->second)[0]);
	  }
	  else if(tmp1>tmp2){
	    histrepo_tot_diff[get_index(tmp2,tmp1)]->Fill((it->second)[0]-(it_another->second)[0]);
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
  //
  const Int_t nx=4,ny=2;
  TCanvas *can = (TCanvas*) gROOT->FindObject("can");
  if(can) delete can;
  can=new TCanvas("can","can",300*nx,300*ny);
  can->Divide(nx,ny,0,0,3);
  for(int i=0;i<ny;i++){
    for(int j=0;j<nx;j++){
      can->cd(nx*i+j+1);
      
      histrepo[i][j]->DrawCopy();
      
      histrepo[i][j]->Write(0,TObject::kOverwrite);
      
    }
  }
  //
  //Int_t nx=4,ny=2;
  TCanvas *can2 = (TCanvas*) gROOT->FindObject("can2");
  if(can2) delete can2;
  can2=new TCanvas("can2","can2",300*nx,300*ny);
  can2->Divide(nx,ny);
  for(int i=0;i<ny;i++){
    for(int j=0;j<nx;j++){
      can2->cd(nx*i+j+1);
      gPad->SetLogy();
      histreop_tot[i][j]->DrawCopy();
    }
  }
  //
  TCanvas *can3 = (TCanvas*) gROOT->FindObject("can3");
  if(can3) delete can3;
  can3=new TCanvas("can3","can3",300*7,300*4);
  can3->Divide(7,4,0,0,3);
  printf("UltraHigh precision:\n");
  for(int i=0;i<4;i++){
    for(int j=0;j<7;j++){
      can3->cd(7*i+j+1);
      gPad->SetLogy();
      histrepo_time_diff[7*i+j]->DrawCopy();
      printf("%s: %.2f\n",histrepo_time_diff[7*i+j]->GetName(),histrepo_time_diff[7*i+j]->GetMean());
    }
  }
  printf("\n");
  //
  TCanvas *can4 = (TCanvas*) gROOT->FindObject("can4");
  if(can4) delete can4;
  can4=new TCanvas("can4","can4",300*7,300*4);
  can4->Divide(7,4,0,0,3);
  printf("High precision:\n");
  for(int i=0;i<4;i++){
    for(int j=0;j<7;j++){
      can4->cd(7*i+j+1);
      gPad->SetLogy();
      histrepo_tot_diff[7*i+j]->DrawCopy();
      printf("%s: %.2f\n",histrepo_tot_diff[7*i+j]->GetName(),histrepo_tot_diff[7*i+j]->GetMean());
    }
  }
  delete file_out;
  
  return 0;
}

