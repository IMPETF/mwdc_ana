/*
 T his *program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.
 
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor,
 Boston, MA  02110-1301, USA.
 
 ---
 Copyright (C) 2015, ufan <>
 */

//#include "TFile.h"
#include "TChain.h"
#include <map>
#include "TString.h"
#include "stdio.h"
#include "TFile.h"
#include "TCanvas.h"
#include "TROOT.h"
#include "TF1.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TProfile.h"
#include "TAxis.h"
#include "TSystem.h"

std::map<TString,int> read_filelist(const char* configfile)
{
  std::map<TString,int> filelist;
  TString filedir;
  char filedirtemp[256];
  int  testindextemp;
  
  FILE* fp=fopen(configfile,"r");
  while (!feof(fp)) {
    fscanf(fp,"%s\t\t%d\n",filedirtemp,&testindextemp);
    filedir=filedirtemp;
    filelist[filedir]=testindextemp;
  }
  fclose(fp);
  
  
  return filelist;
}

void print_fileinfo(std::map<TString,int> filelist)
{
  std::map<TString,int>::iterator it;
 
  for(it=filelist.begin();it!=filelist.end();it++){
    printf("%s\t\t%d\n",it->first.Data(),it->second);
  }
  return;
}

void batch_ana_dy58(const char* configfile,char* outdir)
{
  gROOT->ProcessLine(".L ana_dy58.C+");
  //
  std::map<TString,int> filelist;
  filelist=read_filelist(configfile);
  //
  TChain* tree_in=new TChain("psd_hitinfo");
  std::map<TString,int>::iterator it;
  for(it=filelist.begin();it!=filelist.end();it++){
    tree_in->Add(Form("%s/%s/psd%d.root",gSystem->DirName(configfile),it->first.Data(),it->second));
  }
  int entries=tree_in->GetEntries();
  printf("entries=%d\n",entries);
  //
  TFile* f_out=new TFile(Form("%s/dy58.root",outdir),"recreate");
  
  //TH2F *hpos,*hneg;
  TH2F* hpos=new TH2F("hpos","hpos",200,0,2000,1700,0,17000);
  TH2F* hneg=new TH2F("hneg","hneg",200,0,2000,1700,0,17000);
  hpos->GetXaxis()->SetTitle("Dy5");hpos->GetYaxis()->SetTitle("Dy8");
  hneg->GetXaxis()->SetTitle("Dy5");hneg->GetYaxis()->SetTitle("Dy8");
  
  TString pos_label,neg_label;
  //pos_label.Form("%cpos_dy8[%d]:%cpos_dy5[%d]>>hpos",layer,strip_index-1,layer,strip_index-1);
  //neg_label.Form("%cneg_dy8[%d]:%cneg_dy5[%d]>>hneg",layer,strip_index-1,layer,strip_index-1);
  
  TCanvas* can=new TCanvas("can","can",900,450);
  can->Divide(2,1);
  can->Print(Form("%s/dy58.pdf[",outdir));
  for(int i=0;i<41;i++){
    hpos->SetName(Form("hxpos_dy58_%d",i+1));
    hneg->SetName(Form("hxneg_dy58_%d",i+1));
    hpos->SetTitle(Form("hxpos_dy58_%d",i+1));
    hneg->SetTitle(Form("hxneg_dy58_%d",i+1));
    pos_label.Form("xpos_dy8[%d]:xpos_dy5[%d]>>hxpos_dy58_%d",i,i,i+1);
    neg_label.Form("xneg_dy8[%d]:xneg_dy5[%d]>>hxneg_dy58_%d",i,i,i+1);
    can->cd(1);
    tree_in->Draw(pos_label.Data());
    can->cd(2);
    tree_in->Draw(neg_label.Data());
    can->Print(Form("%s/dy58.pdf",outdir));
    f_out->cd();
    hpos->Write();hneg->Write();
  }
  
  for(int i=0;i<41;i++){
    hpos->SetName(Form("hypos_dy58_%d",i+1));
    hneg->SetName(Form("hyneg_dy58_%d",i+1));
    hpos->SetTitle(Form("hypos_dy58_%d",i+1));
    hneg->SetTitle(Form("hyneg_dy58_%d",i+1));
    pos_label.Form("ypos_dy8[%d]:ypos_dy5[%d]>>hypos_dy58_%d",i,i,i+1);
    neg_label.Form("yneg_dy8[%d]:yneg_dy5[%d]>>hyneg_dy58_%d",i,i,i+1);
    can->cd(1);
    tree_in->Draw(pos_label.Data());
    can->cd(2);
    tree_in->Draw(neg_label.Data());
    can->Print(Form("%s/dy58.pdf",outdir));
    f_out->cd();
    hpos->Write();hneg->Write();
  }
  can->Print(Form("%s/dy58.pdf]",outdir));
  
  f_out->Close();
  
  delete f_out;
  delete tree_in;
}

//int extract_event(const char* datadir,int testindex,const char* outdir,const char* psd_pedfile,int ped_cut=5);
void batch_extract_event(const char* configfile,const char* outdir,float psdoffset_x=6,float psdoffset_y=-2,int ped_cut=5)
{
  gROOT->ProcessLine(".L extract_event.C+");
  //
  std::map<TString,int> filelist;
  filelist=read_filelist(configfile);
  //
  std::map<TString,int>::iterator it;
  TString parentdir,realdir,pedir;
  parentdir=gSystem->DirName(configfile);
  pedir=parentdir+"/ped.root";
  
  for(it=filelist.begin();it!=filelist.end();it++){
    realdir=parentdir+it->first;
    realdir=gSystem->DirName(realdir.Data());
    printf("extract_event(\"%s\",%d,\"%s\",\"%s\",%.4f,%.4f,%d)\n",realdir.Data(),it->second,outdir,pedir.Data(),psdoffset_x,psdoffset_y,ped_cut);
    gROOT->ProcessLine(Form("extract_event(\"%s\",%d,\"%s\",\"%s\",%.4f,%.4f,%d)",realdir.Data(),it->second,outdir,pedir.Data(),psdoffset_x,psdoffset_y,ped_cut));  
  }
  
  FILE* fp=fopen(Form("%s/log.txt",outdir),"w");
  fprintf(fp,"pedestal cut:\t%d sigma\n",ped_cut);
  fprintf(fp,"psdoffset_x:\t%.2f mm\n",psdoffset_x);
  fprintf(fp,"psdoffset_y:\t%.2f mm\n",psdoffset_y);
  fclose(fp);
}