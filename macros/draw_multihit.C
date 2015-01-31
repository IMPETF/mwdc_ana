/*   $Id: draw_multihit.h, 2015-01-30 09:52:57+08:00 MWDC_ana $
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
//
#include "TPaveText.h"

int draw_multihit(const char* datadir,const char* outfile)
{
  //
  TString label_location[2]={"Down","Up"};
  TString label_direction[3]={"X","Y","U"};
  //
  std::vector<TH1F*> histrepo[2];
  Int_t multihit[2][3],flag_singlehit;
  Float_t singlehit[2][3],zerohit[2][3],twohit[2][3],otherhit[2][3];
  Float_t singlehit_all=0;
  TH1* htemp;
  for(int i=0;i<2;i++){
    for(int j=0;j<3;j++){
      htemp=(TH1*)gROOT->FindObject("h"+label_direction[j]+"_"+label_location[i]+"_"+"multihit");
      if(htemp)	{
	delete htemp;
      }
      histrepo[i].push_back(new TH1F("h"+label_direction[j]+"_"+label_location[i]+"_"+"multihit",label_direction[j]+"_"+label_location[i]+"_"+"multihit",11,-0.5,10.5));
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
  TDirectory* dir_merge=file_out->GetDirectory("merge");
  dir_merge->cd();
  TTree *tree_multihit=new TTree("mwdc_multihit","mwdc_multihit");
  tree_multihit->Branch("multihit",multihit,"multihit[2][3]/I");
  
  int entries=tree_mwdc->GetEntriesFast();
  ChannelMap::iterator it;
  UChar_t type,location,direction;
  UShort_t index;
  //for(int i=0;i<100;i++){
  for(int i=0;i<entries;i++){
    if(!((i+1)%5000)){
      printf("%d events analyzed\n",i+1);
    }
    tree_mwdc->GetEntry(i);
    tree_tof->GetEntry(i);
    //
    flag_singlehit=0;
    for(int j=0;j<2;j++){
      for(int k=0;k<3;k++){
	multihit[j][k]=0;
      }
    }
    //
    for(it=mwdc_leading->begin();it!=mwdc_leading->end();it++){
      Encoding::Decode(it->first,type,location,direction,index);
      if (type!=EMWDC) {
	printf("event_%d:MWDC unmatched type\n",i+1);
      }
      multihit[location][direction]++;
    }
    for(it=mwdc_trailing->begin();it!=mwdc_trailing->end();it++){
      Encoding::Decode(it->first,type,location,direction,index);
      if (type!=EMWDC) {
	printf("event_%d:MWDC unmatched type\n",i+1);
      }
    }
    /*
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
    */
    for(int i=0;i<2;i++){
      for(int j=0;j<3;j++){
	histrepo[i][j]->Fill(multihit[i][j]);
	if(multihit[i][j]!=1){
	  flag_singlehit++;
	}
      }
    }
    if(!flag_singlehit)	singlehit_all++;
    //
    tree_multihit->Fill();
  }
  
  printf("%d events processed totally\n",entries);
  //
  tree_mwdc->AddFriend(tree_multihit);
  tree_tof->AddFriend(tree_multihit);
  tree_multihit->Write(0,TObject::kOverwrite);
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
  const Int_t nx=3,ny=2;
  TCanvas *can = (TCanvas*) gROOT->FindObject("can");
  if(can) delete can;
  can=new TCanvas("can","can",300*nx,300*ny);
  can->Divide(nx,ny,0,0,3);
  TPaveText* pavtxt[2][3];
  for(int i=0;i<ny;i++){
    for(int j=0;j<nx;j++){
      can->cd(nx*i+j+1);
      gPad->SetLogy();
      histrepo[i][j]->DrawCopy();
      histrepo[i][j]->Write(0,TObject::kOverwrite);
      
      pavtxt[i][j]=new TPaveText(0.5,0.5,1,1,"NDC");
      pavtxt[i][j]->AddText(Form("total:%d",entries));
      singlehit[i][j]=histrepo[i][j]->GetBinContent(histrepo[i][j]->FindFixBin(1));
      pavtxt[i][j]->AddText(Form("one:%.4f",singlehit[i][j]/entries));
      printf("%s:%.4f\t",(label_direction[j]+label_location[i]).Data(),singlehit[i][j]/entries);
      
      zerohit[i][j]=histrepo[i][j]->GetBinContent(histrepo[i][j]->FindFixBin(0));
      pavtxt[i][j]->AddText(Form("zero:%.4f",zerohit[i][j]/entries));
      
      twohit[i][j]=histrepo[i][j]->GetBinContent(histrepo[i][j]->FindFixBin(2));
      pavtxt[i][j]->AddText(Form("two:%.4f",twohit[i][j]/entries));
      
      otherhit[i][j]=entries-singlehit[i][j]-zerohit[i][j]-twohit[i][j];
      pavtxt[i][j]->AddText(Form("other:%.4f",otherhit[i][j]/entries));
      
      pavtxt[i][j]->Draw();
    }
    printf("\n");
  }
  printf("singlehit events:%.4f\n",singlehit_all/entries);
  //
  delete file_out;
  
  return 0;
}

