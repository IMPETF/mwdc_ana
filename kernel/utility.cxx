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

#include "utility.h"
#include "CrateInfo.h"
#include "BoardInfo.h"
#include "TString.h"
#include "TMath.h"
#include "TTree.h"
#include "TCanvas.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TF1.h"
#include "TFile.h"
#include "TStyle.h"
#include <fstream>
#include <stdio.h>
#include <iostream>
#include "TGraphErrors.h"
#include "TMath.h"
#include "TMultiGraph.h"
#include "TGaxis.h"
#include "TAxis.h"
#include "TPaletteAxis.h"
#include "TProfile.h"
#include "TLegend.h"
#include "TROOT.h"

namespace Utility {
 void read_mwdcinfo(BoardInfo* boardinfo,Json::Value& board)
 {
  if (!boardinfo->InheritsFrom("MWDCBoard")) {
    printf("read_mwdcinfo error: unmatched boardtype\n");
    exit(1);
  }
  else if(!board){
    printf("read_mwdcinfo error: invalid Json node\n");
    exit(1);
  }
  
  Json::Value connector;
  TString connector_name;
  Int_t size;
  UInt_t channel_invalid=(0xFF)<<24;
  for (int i = 0 ; i <4; ++i) {
    connector_name.Form("connector%d",i+1);
    connector=board[connector_name.Data()];
    if(!connector){
      for(int j=0;j<32;j++){
	boardinfo->SetEncodedID(i*32+j,channel_invalid);
      }
    }
    else{
      size=connector.size();
      for(int j=0;j<size;j++){
	boardinfo->SetEncodedID(i*32+j,connector[j].asUInt());
      }
      for(int j=size;j<32;j++){
	boardinfo->SetEncodedID(i*32+j,channel_invalid);
      }
    }
  }
 }
 //
void read_tofinfo(BoardInfo* boardinfo,Json::Value& board)
{
  if (!boardinfo->InheritsFrom("TOFBoard")) {
    printf("read_tofinfo error: unmatched boardtype\n");
    exit(1);
  }
  else if(!board){
    printf("read_tofinfo error: invalid Json node\n");
    exit(1);
  }
  
  Int_t size=board.size();
  UInt_t channel_invalid=(0xFF)<<24;
  for(int i=0;i<size;i++){
    boardinfo->SetEncodedID(i,board[i].asUInt());
  }
  for (int i = size ; i <16; ++i) {
    boardinfo->SetEncodedID(i,channel_invalid);
  }
}
//
CrateInfo* read_config(const char* filename,const char* prefix,const char* suffix)
{

  std::ifstream input(filename);
  if(input.fail()){
    printf("error opening\n");
    exit(1);
  }
  //
  CrateInfo* info=new CrateInfo();
  Json::Reader reader;
  Json::Value  root;
  if(reader.parse(input,root)){
    info->SetName(root["name"].asCString());

    Json::Value board;
    board=root["board"];
    int sizenum=board.size();
    int fileid;
    
    for(int i=0;i<sizenum;i++){
      fileid=board[i]["fileid"].asInt();
      if(!info->IsContained(fileid)){
	info->fileid.push_back(fileid);
	info->filename[fileid]=(TString(prefix)+board[i]["fileid"].asString()+TString(suffix));
	info->boardtype[fileid]=board[i]["boardtype"].asString();
	info->boardname[fileid]=board[i]["boardname"].asString();
	info->boardtitle[fileid]=board[i]["boardtitle"].asString();
      }
    }
    
    //Read in ChannelMap info
    Json::Value map=root["channelmap"];
    BoardInfo* boardinfo;
    TString type;
    TString name;
    TString title;
    if(!map){
      printf("no channelmapping info in this file: %s\n",filename);
    }
    else{
      Json::Value board;
      sizenum=info->GetBoardNum();
      for(int i=0;i<sizenum;i++){
	name=info->GetBoardname(i);
	type=info->GetBoardtype(i);
	title=info->GetBoardtitle(i);
	board=map[name.Data()];
	if(!board.isNull()){
	  if(type=="mwdc"){
	    boardinfo=new MWDCBoard(name.Data(),title.Data());
	    read_mwdcinfo(boardinfo,board);
	  }
	  else if(type == "tof"){
	    boardinfo=new TOFBoard(name.Data(),title.Data());
	    read_tofinfo(boardinfo,board);
	  }
	  boardinfo->SetInit(true);
	  info->SetBoardInfo(boardinfo);
	}
      }
    }
    
  }
  else{
    printf("error: parse JSON file\n");
    exit(1);
  }
  
  return info;
}
}

namespace Utility {
TTree* convert_mwdc(const char* infile,const char* name,const char* title)
{
  //TH1F* h1=new TH1F("h1","h1",200,-99.5,100.5);
  //TH2F* h2=new TH2F("h2","h2",1025,-0.5,1024.5,1025,-0.5,1024.5);
  
  //channel map in the mwdc board: from hptdc electronic channel id(index) to board front-panel channel id(value) 
  UInt_t hptdc_index[128]={0,1,63,62,61,2,3,60,59,4,58,5,57,6,56,7,55,8,9,54,53,52,51,11,10,12,50,13,49,48,14,15,
                          16,17,47,46,45,18,19,44,43,20,42,21,41,22,40,23,39,24,25,38,37,36,35,27,26,28,34,29,33,32,30,31,
                          64,65,127,126,125,66,67,124,123,68,122,69,121,70,120,71,119,72,73,118,117,116,115,75,74,76,114,77,113,112,78,79,
                          80,81,111,110,109,82,83,108,107,84,106,85,105,86,104,87,103,88,89,102,101,100,99,91,90,92,98,93,97,96,94,95};
			  
  unsigned int* buffer;
  //IMPORTANT: buffer should be large enough to hold an event,otherwise fread error may occur.
  buffer=(unsigned int*)malloc(sizeof(unsigned int)*8192);

  FILE* file_in=fopen(infile,"rb");
  if(!file_in){
    perror("fopen");
    exit(1);
  }
  
  TTree* tree_out=new TTree(name,title);
  int word_count=0;
  int event_len;
  unsigned int trigger_id=0;
  unsigned int packet_num=0;
  //unsigned int pre_triggerid=0;
  //int pre_eventid=0;
  Int_t event_id=0;
  Int_t bunch_id=0;
  ChannelMap leading_raw;
  ChannelMap trailing_raw;
  UInt_t tdc_index,channel_index,tdc_value;
  UInt_t global_channel;
  int event_flag=0;//0 event complete;1 event incomplete
  tree_out->Branch("trigger_id",&trigger_id,"trigger_id/i");
  tree_out->Branch("bunch_id",&bunch_id,"bunch_id/I");
  tree_out->Branch("event_id",&event_id,"event_id/I");
  tree_out->Branch("leading_raw",&leading_raw);
  tree_out->Branch("trailing_raw",&trailing_raw);
  
  int counter;
  int type_id;
  int tempcounter=0;
  unsigned int total_words=0;
  while(!feof(file_in)){
    memset(buffer,0,8192*sizeof(unsigned int));
    //packet header
    counter=fread(&trigger_id,1,4,file_in);
    if(counter<4){
      if(ferror(file_in)){
	printf("fread error\n");
	exit(1);
      }
      else{
	break;
      }
    }
    total_words++;
    //
    packet_num++;
    //
    counter=fread(&event_len,1,4,file_in);
    if(counter<4){
      if(ferror(file_in)){
	printf("fread error\n");
	exit(1);
      }
      else{
	break;
      }
    }
    total_words++;
    //
    if(event_len > 8192*4){
      free(buffer);
      buffer=(unsigned int*)malloc(event_len);
    }
    //group decoding
    counter=fread(buffer,1,event_len,file_in);
    if(counter!=event_len){
      if(ferror(file_in)){
	printf("(packet_%u)unexpected behavior\n",packet_num);
	exit(1);
      }
      printf("(packet_%u triggerid_%u) insufficent data:%d words\n",packet_num,event_len/4,trigger_id);
      printf("total words read:%u\n",total_words);
      break;
    }
    if(event_len%4){
      printf("(packet_%u) event_len error\n",packet_num);
    }
    
    event_len=event_len/4;
    word_count=buffer[event_len-1]&0xFF;
    total_words+=event_len;
    /*
    //if(event_len != word_count){
      printf("(packet_%u): event_len != word_count: %d_%d\n",packet_num,event_len,word_count);
      h1->Fill(event_len-word_count);
      h2->Fill(event_len,word_count);
    //}
    */
    type_id=buffer[0]>>28;
    if(type_id==0){
      tdc_index=(buffer[0]>>24)&0xF;
      event_id=(buffer[0]>>12)&0xFFF;
      bunch_id=buffer[0]&0xFFF;
      type_id=buffer[event_len-1]>>28;
      if(type_id == 1){
	if(tdc_index != ((buffer[event_len-1]>>24)&0xF)){
	  printf("(packet_%u)unmatched tdc_index in group header/trailer\n",packet_num);
	}
	if(event_id != ((buffer[event_len-1]>>12)&0xFFF)){
	  printf("(packet_%u)unmatched event_id in group header/trailer\n",packet_num);
	}
      }
      else{
	printf("(packet_%u)not group trailer\n",packet_num);
	continue;
      }
    }
    else{
      printf("\n(packet_%u)not group header\n",packet_num);
    }
    
    leading_raw.clear();
    trailing_raw.clear();
    for(int i=1;i<event_len-1;i++){
	type_id=buffer[i]>>28;
	switch(type_id){
	  case 0x4:
	    tdc_index=(buffer[i]>>24)&0xF;
	    channel_index=(buffer[i]>>19)&0x1F;
	    tdc_value=buffer[i]&0x7FFFF;
      // global_channle here is actually the board front-panel channel index 
	    global_channel=hptdc_index[tdc_index*32+channel_index];
	    leading_raw[global_channel].push_back(tdc_value);
	    break;
	  case 0x5:
	    tdc_index=(buffer[i]>>24)&0xF;
	    channel_index=(buffer[i]>>19)&0x1F;
	    tdc_value=buffer[i]&0x7FFFF;
	    global_channel=hptdc_index[tdc_index*32+channel_index];
	    trailing_raw[global_channel].push_back(tdc_value);
	    break;
	  case 0x6:
	    tdc_index=(buffer[i]>>24)&0xF;
	    printf("(packet_%u)data error_flag: tdc%d(0x%x)\n",packet_num,tdc_index,buffer[i]&0x7FFF);
	    if(i!=(event_len-1)){
	      printf("someother data in packet\n");
	    }
	    break;
	  default:
	    printf("(packet_%u)unexpected type_id in packet\n",packet_num);
	}	
    }
    
    tree_out->Fill();
  }
  
  free(buffer);
  fclose(file_in);
  printf("\n%s packet_num: %d\n",infile,packet_num);
  //
  /*
  TCanvas* can=new TCanvas("can","can",800,400);
  can->Divide(2,1);
  can->cd(1);
  h1->Draw();
  can->cd(2);
  h2->Draw("colz");
  */
  return tree_out;
  
}

TTree* convert_tof(const char* infile,const char* name,const char* title)
{
  // TOF has 3 hptdc and 16 front-panel inputs: tdc 0 and 1 are in very high resolution mode, and tdc 3 are in high resolution mode.
  // In very high resolution mode, each hptdc has 8 valid measurement channel; while in high resolution mode, each hptdc has 32 valid measurement channel.
  // Thus, tdc 3 has 16 spare channels which are not ustilzed in measurement.   
  UInt_t time_index[16]={8,9,10,11,12,13,14,15,
		       0,1,2,3,4,5,6,7};
  UInt_t tot_index[32]={0,1,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,2,0xFFFFFFFF,3
		    ,0xFFFFFFFF,0xFFFFFFFF,4,5,6,8,7,0xFFFFFFFF,9,10
		    ,11,0xFFFFFFFF,12,13,14,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF
		    ,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,15,0xFFFFFFFF};
		    
  //IMPORTANT: buffer should be large enough to hold an event,otherwise fread error may occur.
  unsigned int* buffer;
  buffer=(unsigned int*)malloc(sizeof(unsigned int)*8192);

  FILE* file_in=fopen(infile,"rb");
  if(!file_in){
    perror("fopen");
    exit(1);
  }
  
  TTree* tree_out=new TTree(name,title);
  int word_count=0;
  int event_len;
  unsigned int trigger_id=0;
  unsigned int packet_num=0;
  //unsigned int pre_triggerid=0;
  //int pre_eventid=0;
  int event_id=0;
  int bunch_id=0;
  ChannelMap time_leading_raw;
  ChannelMap time_trailing_raw;
  ChannelMap tot_leading_raw;
  ChannelMap tot_trailing_raw;
  UInt_t tdc_index,channel_index,tdc_value;
  UInt_t global_channel;
  int event_flag=0;//0 event complete;1 event incomplete
  tree_out->Branch("trigger_id",&trigger_id,"trigger_id/i");
  tree_out->Branch("bunch_id",&bunch_id,"bunch_id/I");
  tree_out->Branch("event_id",&event_id,"event_id/I");
  tree_out->Branch("time_leading_raw",&time_leading_raw);
  tree_out->Branch("time_trailing_raw",&time_trailing_raw);
  tree_out->Branch("tot_leading_raw",&tot_leading_raw);
  tree_out->Branch("tot_trailing_raw",&tot_trailing_raw);
  
  int counter;
  int type_id;
  while(!feof(file_in)){
    memset(buffer,0,8192*sizeof(unsigned int));
    //packet header
    counter=fread(&trigger_id,1,4,file_in);
    if(counter<4){
      if(ferror(file_in)){
	printf("fread error\n");
	exit(1);
      }
      else{
	break;
      }
    }
    packet_num++;
    counter=fread(&event_len,1,4,file_in);
    if(counter<4){
      if(ferror(file_in)){
	printf("fread error\n");
	exit(1);
      }
      else{
	break;
      }
    }
    //group decoding
    counter=fread(buffer,1,event_len,file_in);
    if(counter!=event_len){
      if(ferror(file_in)){
	printf("(packet_%u)unexpected behavior\n",packet_num);
	exit(1);
      }
      printf("(packet_%u) insufficent data\n",packet_num);
      break;
    }
    if(event_len%4){
      printf("(packet_%u) event_len error\n",packet_num);
    }
    event_len=event_len/4;
    word_count=buffer[event_len-1]&0xFF;
    /*
    if(event_len != word_count){//interesting behavior:word_count and event_len is not always consistent.
      printf("event_len != word_count: %d_%d\n",event_len,word_count);
    }
    */
    type_id=buffer[0]>>28;
    if(type_id==0){
      tdc_index=(buffer[0]>>24)&0xF;
      event_id=(buffer[0]>>12)&0xFFF;
      bunch_id=buffer[0]&0xFFF;
      type_id=buffer[event_len-1]>>28;
      if(type_id == 1){
	if(tdc_index != ((buffer[event_len-1]>>24)&0xF)){
	  printf("(packet_%u)unmatched tdc_index in group header/trailer\n",packet_num);
	}
	if(event_id != ((buffer[event_len-1]>>12)&0xFFF)){
	  printf("(packet_%u)unmatched event_id in group header/trailer\n",packet_num);
	}
      }
      else{
	printf("(packet_%u)not group trailer\n",packet_num);
	continue;
      }
    }
    else{
      printf("\n(packet_%u)not group header\n",packet_num);
    }
    
    time_leading_raw.clear();
    time_trailing_raw.clear();
    tot_leading_raw.clear();
    tot_trailing_raw.clear();
    for(int i=1;i<event_len-1;i++){
	type_id=buffer[i]>>28;
	switch(type_id){
	  case 0x4:
	    tdc_index=(buffer[i]>>24)&0xF;
	    if(tdc_index == 2){
	      channel_index=(buffer[i]>>19)&0x1F;
	      tdc_value=buffer[i]&0x7FFFF;// low,medium and high resolution has the same data format, with the ADC count unit as 25us/256
	      global_channel=tot_index[channel_index];
	      tot_leading_raw[global_channel].push_back(tdc_value);
	    }
	    else if((tdc_index == 0) || (tdc_index == 1)){
	      channel_index=(buffer[i]>>21)&0x7;
	      tdc_value=((buffer[i]&0x7FFFF)<<2) + ((buffer[i]>>19)&0x3);// very high resolution has different data format, with the ADCc count unit as 25us/256/4
	      global_channel=time_index[tdc_index*8+channel_index];
	      time_leading_raw[global_channel].push_back(tdc_value);
	    }
	    else{
	      printf("(packet_%u)unexpected tdc_index in packet\n",packet_num);
	    }
	    break;
	  case 0x5:
	    tdc_index=(buffer[i]>>24)&0xF;
	    if(tdc_index == 2){
	      channel_index=(buffer[i]>>19)&0x1F;
	      tdc_value=buffer[i]&0x7FFFF;
	      global_channel=tot_index[channel_index];
	      tot_trailing_raw[global_channel].push_back(tdc_value);
	    }
	    else if((tdc_index == 0) || (tdc_index == 1)){
	      channel_index=(buffer[i]>>21)&0x7;
	      tdc_value=((buffer[i]&0x7FFFF)<<2) + ((buffer[i]>>19)&0x3);
	      global_channel=time_index[tdc_index*8+channel_index];
	      time_trailing_raw[global_channel].push_back(tdc_value);
	    }
	    else{
	      printf("(packet_%u)unexpected tdc_index in packet\n",packet_num);
	    }
	    break;
	  case 0x6:
	    tdc_index=(buffer[i]>>24)&0xF;
	    printf("(packet_%u)data error_flag: tdc%d(0x%x)\n",packet_num,tdc_index,buffer[i]&0x7FFF);
	    if(i!=(event_len-1)){
	      printf("someother data in packet\n");
	    }
	    break;
	  default:
	    printf("(packet_%u)unexpected type_id in packet\n",packet_num);
	}	
    }
    tree_out->Fill();
  }
  
  free(buffer);
  fclose(file_in);
  printf("\n%s packet_num: %d\n",infile,packet_num);
  
  return tree_out;
}

int convert_hptdc(const char* datadir,const char* outfile,const char* prefix,const char* suffix)
{
  TString file_config=TString(datadir)+"/crate.json";
  CrateInfo* info=read_config(file_config.Data(),prefix,suffix);
  info->Print();
  
  TString file_data=TString(datadir)+"/"+outfile;  
  TTree* tree_out;
  TFile* file_out=new TFile(file_data,"recreate");
  TDirectory* raw_dir=file_out->mkdir("raw");
  
  int boardnum=info->GetBoardNum();
  for(int i=0;i<boardnum;i++){
    file_data=TString(datadir)+"/"+info->GetFilename(i);
    if(info->GetBoardtype(i) == "mwdc"){
      tree_out=convert_mwdc(file_data.Data(),info->GetBoardname(i).Data(),info->GetBoardtitle(i).Data());
    }
    else if(info->GetBoardtype(i) == "tof"){
      tree_out=convert_tof(file_data.Data(),info->GetBoardname(i).Data(),info->GetBoardtitle(i).Data());
    }
    else{
      printf("error: unrecognized board type %s\n",info->GetBoardtype(i).Data());
      exit(1);
    }
    
    raw_dir->cd();
    tree_out->Write();
    delete tree_out;
  }
  delete file_out;
  delete info;
  return 0;
}

int print_info(const char* datadir, const char* rootfile,const char* logfile,int bunchid_diff,const char* prefix,const char* suffix)
{
  TString file_config=TString(datadir)+"/crate.json";
  CrateInfo* info=read_config(file_config.Data(),prefix,suffix);
  info->Print();
  
  TString outfile=TString(datadir)+"/"+logfile;
  FILE* fp=fopen(outfile.Data(),"w");
  
  TString file_data=TString(datadir)+"/"+rootfile;
  TFile* file_in=new TFile(file_data);
  TDirectory* dir_in=file_in->GetDirectory("raw");
  
  int boardnum=info->GetBoardNum();
  
  std::vector<TTree*> tree;
  for(int i=0;i<boardnum;i++){
    tree.push_back((TTree*)dir_in->Get(info->GetBoardname(i)));
  }
  
  /*
  typedef TTree* TTreePtr;
  TTreePtr *tree=new TTreePtr[boardnum];
  for(int i=0;i<boardnum;i++){
    tree[i]=(TTree*)file_in->Get(info->GetBoardname(i));
  }
  */
  int *prebunch_id=new int[boardnum];
  int *deltabunch_id=new int[boardnum];
  int *initbunch_id=new int[boardnum];
  int *initevent_id=new int[boardnum];
  int *finalbunch_id=new int[boardnum];
  int *finalevent_id=new int[boardnum];
  int max_bunch,min_bunch;
  int max_deltabunch,min_deltabunch;
  
  unsigned int *trigger_id=new unsigned int[boardnum];
  int *event_id=new int[boardnum];
  int *bunch_id=new int[boardnum];
  
  unsigned int min,size;
  min=0xFFFFFFFF;
  unsigned int max_event,min_event,max_trigger,min_trigger;
  for(int i=0;i<boardnum;i++){
    tree[i]->SetBranchAddress("trigger_id",trigger_id+i);
    tree[i]->SetBranchAddress("bunch_id",bunch_id+i);
    tree[i]->SetBranchAddress("event_id",event_id+i);
    
    size=tree[i]->GetEntries();
    if(size < min){
      min = size;
    }
    prebunch_id[i]=0;
  }
  //
  for(int i=0;i<min;i++){
    for(int j=0;j<boardnum;j++){
      tree[j]->GetEntry(i);
    }
    //get initial and final bunch_id and event_id,these ids are original
    if(i==0){
      for(int j=0;j<boardnum;j++){
	initbunch_id[j]=bunch_id[j];
        initevent_id[j]=event_id[j];
      }
    }
    if(i==(min-1)){
      for(int j=0;j<boardnum;j++){
	finalbunch_id[j]=bunch_id[j];
        finalevent_id[j]=event_id[j];
      }
    }
    //--------------------------------------------------
    //substract init bunch_offset
    for(int j=0;j<boardnum;j++){
      if(bunch_id[j]<initbunch_id[j]){
	bunch_id[j]=bunch_id[j]+0x1000-initbunch_id[j];
      }
      else{
	bunch_id[j]=bunch_id[j]-initbunch_id[j];
      }
    }
    max_bunch=TMath::MaxElement(boardnum,bunch_id);
    min_bunch=TMath::MinElement(boardnum,bunch_id);
    if(max_bunch>min_bunch+4000){
      for(int j=0;j<boardnum;j++){
	if(bunch_id[j]<95)
	  bunch_id[j]=bunch_id[j]+0x1000;
        }
    }
    //---------------------------------------------------
    for(int j=0;j<boardnum;j++){
      if(bunch_id[j] < prebunch_id[j]){
	deltabunch_id[j]=bunch_id[j]+0x1000-prebunch_id[j];
        if(deltabunch_id[j]>4090){
	  deltabunch_id[j]=bunch_id[j]-prebunch_id[j];
        }
                //fprintf(fp,"%d\t",deltabunch_id[j]);
      }
      else{
	deltabunch_id[j]=bunch_id[j]-prebunch_id[j];
                //fprintf(fp,"%d\t",deltabunch_id[j]);
      }
    }
    max_deltabunch=TMath::MaxElement(boardnum,deltabunch_id);
    min_deltabunch=TMath::MinElement(boardnum,deltabunch_id);
    max_bunch=TMath::MaxElement(boardnum,bunch_id);
    min_bunch=TMath::MinElement(boardnum,bunch_id);
    if((max_bunch>(min_bunch+bunchid_diff)) && (max_deltabunch>(min_deltabunch+bunchid_diff))){
      fprintf(fp,"---------------------------\n");
      fprintf(fp,"pre %d: ",i-1);
      for(int j=0;j<boardnum;j++){
	if(j != (boardnum-1)){
	  fprintf(fp,"%d\t",prebunch_id[j]);
	}
	else{
	  fprintf(fp,"%d: max=%d\n",prebunch_id[j],max_bunch);
	}
      }
      fprintf(fp,"cur %d: ",i);
      for(int j=0;j<boardnum;j++){
	if(j != (boardnum-1)){
	  fprintf(fp,"%d\t",bunch_id[j]);
	}
	else{
	  fprintf(fp,"%d: min=%d\n",bunch_id[j],min_bunch);
	}
      }
      fprintf(fp,"---------------------------\n");      
    }
    for(int j=0;j<boardnum;j++){
      prebunch_id[j]=bunch_id[j];
    }
    //
    /*
    max_event=TMath::MaxElement(boardnum,event_id);
    min_event=TMath::MinElement(boardnum,event_id);
    if(max_event != min_event){
      printf("error_%d: unmatched event_id\n",i);
      for(int j=0;j<boardnum;j++){
	if(j != (boardnum-1)){
	  printf("%d\t",event_id[j]);
	}
	else{
	  printf("%d\n",event_id[j]);
	}
      }
      //exit(EXIT_FAILURE);
    }
    */
    //
    max_trigger=TMath::MaxElement(boardnum,trigger_id);
    min_trigger=TMath::MinElement(boardnum,trigger_id);
    if(max_trigger != min_trigger){
      printf("error_%d: unmatched trigger_id\n",i);
      exit(EXIT_FAILURE);
    }
  }

  fclose(fp);
  
  printf("event_num(min of %d boards)=%d\n",boardnum,min);
  printf("init: ");
  for(int j=0;j<boardnum;j++){
    printf("%d\t",initevent_id[j]);
  }
  printf(": ");
  for(int j=0;j<boardnum;j++){
    printf("%d\t",initbunch_id[j]);
  }
  printf("\n");
  printf("final: ");
  for(int j=0;j<boardnum;j++){
    printf("%d\t",finalevent_id[j]);
  }
  printf(": ");
  for(int j=0;j<boardnum;j++){
    printf("%d\t",finalbunch_id[j]);
  }
  printf("\n");
    
  delete file_in;
  //delete[] tree;
  delete[] prebunch_id;
  delete[] deltabunch_id;
  delete[] initbunch_id;
  delete[] initevent_id;
  delete[] finalbunch_id;
  delete[] finalevent_id;
  delete[] trigger_id;
  delete[] event_id;
  delete[] bunch_id;
  
  delete info;
  return 0;
}

//trigger_id consitency,bunch_id consitency
//difference of bunch_ids.
int check(const char* datadir,const char* outfile)
{
  TH1F* hbunch_mwdc=new TH1F("hbunch_mwdc","hbunch_mwdc",6,-0.5,5.5);
   TH1F* hbunch_tof=new TH1F("hbunch_tof","hbunch_tof",6,-0.5,5.5);
   TH1F* hbunch_all=new TH1F("hbunch_all","hbunch_all",6,2.5,8.5);
  //readin the config file which include channelmapping info
  TString file_config=TString(datadir)+"/crate.json";
  CrateInfo* info=read_config(file_config.Data(),"mapping");
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
  
  TTree** 	tree_in_tof=new TTree*[tofnum]{};
  BoardInfo**	tof_boardinfo=new BoardInfo*[tofnum]{};
  UInt_t*	tof_triggerid=new UInt_t[tofnum]{};
  Int_t*	tof_bunchid=new Int_t[tofnum]{};
  
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
	/*tree_in[i]->Print();
	*/
	mwdc_boardinfo[mwdcnum-1]->Print();
	break;
      case ETOF:
	raw_dir->GetObject(boardinfo[i]->GetName(),tree_in_tof[tofnum++]);
	tof_boardinfo[tofnum-1]=boardinfo[i];
	tree_in[i]=tree_in_tof[tofnum-1];
	
	tree_in[i]->SetBranchAddress("trigger_id",&tof_triggerid[tofnum-1]);
	tree_in[i]->SetBranchAddress("bunch_id",&tof_bunchid[tofnum-1]);
	/*tree_in[i]->Print();
	*/
	tof_boardinfo[tofnum-1]->Print();
	break;
      default:
	break;
    }
  }

  //
  Int_t temp_entries;
  Int_t entries=tree_in[0]->GetEntriesFast();
  for(int i=0;i<boardnum;i++){
    temp_entries=tree_in[i]->GetEntriesFast();
    if(temp_entries<entries){
      entries=temp_entries;
    }
  }
  Int_t max_triggerid,min_triggerid,temp_triggerid;
  Int_t max_bunchid,min_bunchid,temp_bunchid;
  Int_t unmatched_trigger=0,unmatched_bunch_mwdc=0,unmatched_bunch_tof=0;
  Int_t matched_bunch=0;
  //start merge loop
  for(int i=0;i<entries;i++){
    if(!((i+1)%5000)){
      printf("%d events checked\n",i+1);
    }
    //
    for(int j=0;j<boardnum;j++){
      tree_in[j]->GetEntry(i);
    }
    //check trigger_id and bunch_id
    max_triggerid=TMath::MaxElement(mwdcnum,mwdc_triggerid);
    min_triggerid=TMath::MinElement(mwdcnum,mwdc_triggerid);
    max_bunchid=TMath::MaxElement(mwdcnum,mwdc_bunchid);
    min_bunchid=TMath::MinElement(mwdcnum,mwdc_bunchid);
    if(max_triggerid!=min_triggerid){
      printf("ERROR event_%d:unmatched trigger_id between MWDC boards(T:%d,%d)\n",i+1,max_triggerid,min_triggerid);
    }
    else{
      temp_triggerid=max_triggerid;
    }
    if(max_bunchid != min_bunchid){
      unmatched_bunch_mwdc++;
      temp_bunchid=-1;
    }
    else{
      temp_bunchid=max_bunchid;
    }
    hbunch_mwdc->Fill(max_bunchid-min_bunchid);
    //
    max_triggerid=TMath::MaxElement(tofnum,tof_triggerid);
    min_triggerid=TMath::MinElement(tofnum,tof_triggerid);
    max_bunchid=TMath::MaxElement(tofnum,tof_bunchid);
    min_bunchid=TMath::MinElement(tofnum,tof_bunchid);
    if((max_triggerid!=min_triggerid)){
      printf("ERROR event_%d:unmatched trigger_id between TOF boards(T:%d,%d)\n",i+1,max_triggerid,min_triggerid);
    }
    else if((temp_triggerid != max_triggerid)){
      printf("ERROR event_%d:unmatched trigger_id between MWDC and TOF boards(T:%d,%d)\n",i+1,max_triggerid,temp_triggerid);
    }
    if(max_bunchid != min_bunchid){
      unmatched_bunch_tof++;
    }
    else if((temp_bunchid != -1) && (TMath::Abs(temp_bunchid-max_bunchid)==6)){ //|| temp_bunchid==max_bunchid) ){
      matched_bunch++;
    }
    hbunch_tof->Fill(max_bunchid-min_bunchid);
    //
    hbunch_all->Fill(TMath::MaxElement(tofnum,tof_bunchid)-TMath::MinElement(mwdcnum,mwdc_bunchid));
  }
  printf("total events %d\n",entries);
  printf("total mathced bunch events %d\n",matched_bunch);
  printf("MWDC unmatched bunch events: %d\n",unmatched_bunch_mwdc);
  printf("TOF unmatched bunch events: %d\n",unmatched_bunch_tof);
  //
  gStyle->SetOptStat(111111);
  TCanvas *c1=new TCanvas("c1","c1",1200,400);
  c1->Divide(3,1);
  c1->cd(1);
  hbunch_mwdc->Draw();
  c1->cd(2);
  hbunch_tof->Draw();
  c1->cd(3);
  hbunch_all->Draw();
  //
  delete file_out;
  delete [] tree_in_mwdc;
  delete [] mwdc_boardinfo;
  delete [] mwdc_triggerid;
  delete [] mwdc_bunchid;
  
  delete [] tree_in_tof;
  delete [] tof_boardinfo;
  delete [] tof_triggerid;
  delete [] tof_bunchid;
  
  delete [] tree_in;
  delete [] boardinfo;
  //
  return 0;
}

}

namespace Utility {
int merge_hptdc(const char* datadir,const char* outfile)
{
  //readin the config file which include channelmapping info
  TString file_config=TString(datadir)+"/crate.json";
  CrateInfo* info=read_config(file_config.Data(),"mapping");
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
	/*tree_in[i]->Print();
	*/
	mwdc_boardinfo[mwdcnum-1]->Print();
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
	/*tree_in[i]->Print();
	*/
	tof_boardinfo[tofnum-1]->Print();
	break;
      default:
	break;
    }
  }
  //init output variables
  TDirectory* dir_out=file_out->GetDirectory("merge");
  if(!dir_out){
    dir_out=file_out->mkdir("merge");
    if(!dir_out){
      printf("error!can't mkdir \"merge\" in %s\n",outfile);
      exit(1);
    }
  }
  dir_out->cd();
  
  ChannelMap mwdc_leading,mwdc_trailing;
  ChannelMap tof_timeleading,tof_timetrailing,tof_totleading,tof_tottrailing;
  TTree* tree_out_mwdc=new TTree("mwdc","mwdc");
  tree_out_mwdc->Branch("leading_raw",&mwdc_leading);
  tree_out_mwdc->Branch("trailing_raw",&mwdc_trailing);
  TTree* tree_out_tof=new TTree("tof","tof");
  tree_out_tof->Branch("time_leading_raw",&tof_timeleading);
  tree_out_tof->Branch("time_trailing_raw",&tof_timetrailing);
  tree_out_tof->Branch("tot_leading_raw",&tof_totleading);
  tree_out_tof->Branch("tot_trailing_raw",&tof_tottrailing);
  
  Int_t temp_entries;
  Int_t entries=tree_in[0]->GetEntriesFast();
  for(int i=0;i<boardnum;i++){
    temp_entries=tree_in[i]->GetEntriesFast();
    if(temp_entries<entries){
      entries=temp_entries;
    }
  }
  ChannelMap::iterator it;
  Int_t max_triggerid,min_triggerid,temp_triggerid;
  Int_t max_bunchid,min_bunchid,temp_bunchid;
  
  //start merge loop
  for(int i=0;i<entries;i++){
    if(!((i+1)%5000)){
      printf("%d events merged\n",i+1);
    }
    //
    for(int j=0;j<boardnum;j++){
      tree_in[j]->GetEntry(i);
    }
    //check trigger_id and bunch_id
    max_triggerid=TMath::MaxElement(mwdcnum,mwdc_triggerid);
    min_triggerid=TMath::MinElement(mwdcnum,mwdc_triggerid);
    max_bunchid=TMath::MaxElement(mwdcnum,mwdc_bunchid);
    min_bunchid=TMath::MinElement(mwdcnum,mwdc_bunchid);
    if((max_triggerid!=min_triggerid) || (max_bunchid!=min_bunchid)){
      //printf("event_%d:unmatched trigger_id/bunch_id between MWDC boards(T:%d,%d|B:%d,%d)\n",i+1,max_triggerid,min_triggerid,max_bunchid,min_bunchid);
    }
    else{
      temp_bunchid=max_bunchid;
      temp_triggerid=max_triggerid;
    }
    max_triggerid=TMath::MaxElement(tofnum,tof_triggerid);
    min_triggerid=TMath::MinElement(tofnum,tof_triggerid);
    max_bunchid=TMath::MaxElement(tofnum,tof_bunchid);
    min_bunchid=TMath::MinElement(tofnum,tof_bunchid);
    if((max_triggerid!=min_triggerid) || (max_bunchid!=min_bunchid)){
      //printf("event_%d:unmatched trigger_id/bunch_id between TOF boards(T:%d,%d|B:%d,%d)\n",i+1,max_triggerid,min_triggerid,max_bunchid,min_bunchid);
    }
    else if((temp_triggerid != max_triggerid) || ((TMath::Abs(max_bunchid-temp_bunchid)!=6) && (max_bunchid!=temp_bunchid))){
      //printf("event_%d:unmatched trigger_id/bunch_id between TOF and MWDC boards(B:%d,%d)\n",i+1,temp_bunchid,max_bunchid);
    }
   
    //main merge process
    mwdc_leading.clear();mwdc_trailing.clear();
    for(int j=0;j<mwdcnum;j++){
      for(it=mwdc_leading_raw[j]->begin();it!=mwdc_leading_raw[j]->end();it++){
	if(mwdc_boardinfo[j]->IsChannelValid(it->first)){
	  mwdc_leading[mwdc_boardinfo[j]->GetEncodedID(it->first)]=it->second;
	}
      }
      for(it=mwdc_trailing_raw[j]->begin();it!=mwdc_trailing_raw[j]->end();it++){
	if(mwdc_boardinfo[j]->IsChannelValid(it->first)){
	  mwdc_trailing[mwdc_boardinfo[j]->GetEncodedID(it->first)]=it->second;
	}
      }
    }
    tof_timeleading.clear();tof_timetrailing.clear();
    tof_totleading.clear();tof_tottrailing.clear();
    for(int j=0;j<tofnum;j++){
      for(it=tof_timeleading_raw[j]->begin();it!=tof_timeleading_raw[j]->end();it++){
	if(tof_boardinfo[j]->IsChannelValid(it->first)){
	  tof_timeleading[tof_boardinfo[j]->GetEncodedID(it->first)]=it->second;
	}
      }
      for(it=tof_timetrailing_raw[j]->begin();it!=tof_timetrailing_raw[j]->end();it++){
	if(tof_boardinfo[j]->IsChannelValid(it->first)){
	  tof_timetrailing[tof_boardinfo[j]->GetEncodedID(it->first)]=it->second;
	}
      }
      for(it=tof_totleading_raw[j]->begin();it!=tof_totleading_raw[j]->end();it++){
	if(tof_boardinfo[j]->IsChannelValid(it->first)){
	  tof_totleading[tof_boardinfo[j]->GetEncodedID(it->first)]=it->second;
	}
      }
      for(it=tof_tottrailing_raw[j]->begin();it!=tof_tottrailing_raw[j]->end();it++){
	if(tof_boardinfo[j]->IsChannelValid(it->first)){
	  tof_tottrailing[tof_boardinfo[j]->GetEncodedID(it->first)]=it->second;
	}
      }
    }
    //
    tree_out_mwdc->Fill();
    tree_out_tof->Fill();
  }
  
  printf("%d events merged!\n",entries);
  
  tree_out_mwdc->Write(0,TObject::kOverwrite);
  tree_out_tof->Write(0,TObject::kOverwrite);
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

void mapping_validation(const char* datadir,const char* outfile)
{
  //TH1F* hmwdc_size=new TH1F("hmwdc_size","hmwdc_size",513,-0.5,512.5);
  //TH1F* htof_size=new TH1F("htof_size","htof_size",513,-0.5,512.5);
  TH1F* hwireid[2][3];
  TString label_location[2]={"Down","Up"};
  TString label_direction[3]={"X","Y","U"};
  for(int i=0;i<2;i++){
    for(int j=0;j<3;j++){
      hwireid[i][j]=new TH1F("MWDC_"+label_location[i]+"_"+label_direction[j],"MWDC_"+label_location[i]+"_"+label_direction[j],106,0.5,106.5);
    }
  }
  //
  TString file_data=TString(datadir)+"/"+outfile;  
  TFile* file_out=new TFile(file_data);
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
  ChannelMap *tof_timeleading,*tof_timetrailing,*tof_totleading,*tof_tottrailing;
  tree_tof->SetBranchAddress("time_leading_raw",&tof_timeleading);
  tree_tof->SetBranchAddress("time_trailing_raw",&tof_timetrailing);
  tree_tof->SetBranchAddress("tot_leading_raw",&tof_totleading);
  tree_tof->SetBranchAddress("tot_trailing_raw",&tof_tottrailing);
  //
  int entries=tree_mwdc->GetEntriesFast();
  ChannelMap::iterator it;
  UChar_t type,location,direction;
  UShort_t index;
  for(int i=0;i<entries;i++){
    if(!(i%5000)){
      printf("%d events analyzed\n",i+1);
    }
    tree_mwdc->GetEntry(i);
    //tree_tof->GetEntry(i);
    //
    for(it=mwdc_leading->begin();it!=mwdc_leading->end();it++){
      Encoding::Decode(it->first,type,location,direction,index);
      if (type!=EMWDC) {
	printf("event_%d:MWDC unmatched type\n",i+1);
      }
      hwireid[location][direction]->Fill(index+1);
    }
  }
  
  delete file_out;
  
  TCanvas* c=new TCanvas("c","c",900, 300);
  c->Divide(3,2);
  for(int i=0;i<2;i++){
    for(int j=0;j<3;j++){
      c->cd(i*3+j+1);
      hwireid[i][j]->Draw();
    }
  }
  c->Print(TString(datadir)+"/"+"mapping_validation.pdf");
  //c->cd(1);hmwdc_size->Draw();
  //c->cd(2);htof_size->Draw();
  
}

}

namespace Utility {
  int convert_eventblock(char* data,Int_t *Xpos,Int_t *Ypos,Int_t *Xneg,Int_t *Yneg)
  {
    //printf("in burst\n");
    const Int_t FEE[4]={0x20,0x24,0x28,0x2c};
    Int_t* tmp[4];
    tmp[0]=Xpos;
    tmp[1]=Xneg;
    tmp[2]=Ypos;
    tmp[3]=Yneg;
    const int data_length=186;
    const int block_length=194;
    unsigned char* burst=(unsigned char*)data;

    int read_length=0;
    int trig_couter[4];
    int init=0;
    for(int feeid=0;feeid<4;feeid++){
        init=feeid*block_length;
        if(burst[init]==0x55 && burst[init+1]==0xaa && burst[init+2]==0xeb && burst[init+3]==0x90
                && burst[init+192]==0x5a && burst[init+193]==0xa5){
            if(burst[init+5]==FEE[feeid]){
                read_length=((burst[init+6]&0xFF)<<8) + (burst[init+7]&0xFF);
                if(read_length != data_length){
                    printf("error! wrong data length %d != %d \n",read_length,data_length);
                    return -1;
                }
                for(int data_id=0;data_id<90;data_id++){
                    tmp[feeid][data_id]=((burst[init+8+data_id*2]&0xFF)<<8) + (burst[init+8+data_id*2+1]&0xFF);
                    if(tmp[feeid][data_id]==0){
                        tmp[feeid][data_id] = -5;
                    }
                    else if(tmp[feeid][data_id] > 0x3FFF){
                        tmp[feeid][data_id] = 16400;
                    }
                }
                trig_couter[feeid] = ((burst[init+188]&0xF)<<8)+burst[init+189];
                //trig_couter[feeid] = (burst[init+189]);
            }
            else
            {
                printf("error! wrong sequence in a burst\n");
                return -1;
            }
        }
        else{
            printf("error! incomplete block\n");
            return -1;
        }
    }

    if((trig_couter[0] != trig_couter[1]) || (trig_couter[0] != trig_couter[2]) || (trig_couter[0] != trig_couter[3]) ){
        printf("error! inconsistent trigger count in a burst");
        return -1;
    }
    else
        return trig_couter[0];
}

  int convert_psd(const Char_t* parentDir,const Char_t* infile,const Char_t* outDir,const Char_t* outfile)
{
    int id8[41]={0,1,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,22,46,47,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64,65,66};
    int id5[41]={23,24,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,45,68,69,71,72,73,74,75,76,77,78,79,80,81,82,83,84,85,86,87,88};
    //
    const Int_t FEE[4]={0x20,0x24,0x28,0x2c};
    const Int_t block_length=194;

    Char_t infname[200],outfname[200];
    sprintf(infname,"%s/%s",parentDir,infile);
    sprintf(outfname,"%s/%s",outDir,outfile);
    //
    TFile *f=new TFile(outfname,"RECREATE");
    if(f->IsZombie()){
        printf("this file will not been recreated!\n");
        delete f;
        return -1;
    }
    else if(f->IsOpen()){
        ifstream in;
        in.open(infname,std::ios_base::in | std::ios_base::binary);
        if(!in.is_open()){
            std::cout << "can't open"<< infname << std::endl;
        }

//-----------------------------------------------------------------------
        int Ch_tmp1,Ch_tmp2;
        unsigned char x;

        TH1F *hxpos[90];
        TH1F *hxneg[90];
        TH1F *hypos[90];
        TH1F *hyneg[90];
        for(int i=0;i<90;i++){
            hxpos[i]=new TH1F(Form("xpos_%d",i+1),Form("xpos_%d",i+1),4000,-0.5,3999.5);
            hxneg[i]=new TH1F(Form("xneg_%d",i+1),Form("xneg_%d",i+1),4000,-0.5,3999.5);
            hypos[i]=new TH1F(Form("ypos_%d",i+1),Form("ypos_%d",i+1),4000,-0.5,3999.5);
            hyneg[i]=new TH1F(Form("yneg_%d",i+1),Form("yneg_%d",i+1),4000,-0.5,3999.5);
        }

        TTree *tree = new TTree("PSD","PSD Testing event mode");
        Int_t Xpos[90],Ypos[90],Xneg[90],Yneg[90];
        Int_t trigger_id;
        tree->Branch("event_id",&trigger_id,"event_id/I");
        tree->Branch("xpos",Xpos,"xpos[90]/I");
        tree->Branch("ypos",Ypos,"ypos[90]/I");
        tree->Branch("xneg",Xneg,"xneg[90]/I");
        tree->Branch("yneg",Yneg,"yneg[90]/I");
	
	TTree *tree_psd_out=new TTree("psd_hitinfo","psd_hitinfo");
	int x_dy8_pos[41],x_dy5_pos[41],x_dy8_neg[41],x_dy5_neg[41];
	int y_dy8_pos[41],y_dy5_pos[41],y_dy8_neg[41],y_dy5_neg[41];
	tree_psd_out->Branch("xpos_dy8",x_dy8_pos,"xpos_dy8[41]/I");
	tree_psd_out->Branch("xpos_dy5",x_dy5_pos,"xpos_dy5[41]/I");
	tree_psd_out->Branch("xneg_dy8",x_dy8_neg,"xneg_dy8[41]/I");
	tree_psd_out->Branch("xneg_dy5",x_dy5_neg,"xneg_dy5[41]/I");
	tree_psd_out->Branch("ypos_dy8",y_dy8_pos,"ypos_dy8[41]/I");
	tree_psd_out->Branch("ypos_dy5",y_dy5_pos,"ypos_dy5[41]/I");
	tree_psd_out->Branch("yneg_dy8",y_dy8_neg,"yneg_dy8[41]/I");
	tree_psd_out->Branch("yneg_dy5",y_dy5_neg,"yneg_dy5[41]/I");
//----------------------------------------------------------------------------------------
    unsigned int event_num=0;
    char burst_block[block_length*4];
    int trigger_id_pre=0;
    while(!in.eof()){
        //printf("test");
        in.read(burst_block,block_length*4);
        if(!in.eof()){
            trigger_id=convert_eventblock(burst_block,Xpos,Ypos,Xneg,Yneg);
            if(trigger_id == -1){
                exit(EXIT_FAILURE);
            }
            else{
                event_num++;
		if(event_num%5000==0){
		  printf("%d events converted\n",event_num);
		}
		if(event_num==1 && trigger_id!=0){
		  printf("ERROR: Init trigger_id should be 0\n");
		  exit(EXIT_FAILURE);
		}
		else if(event_num != 1){
		  if(trigger_id_pre == 0xFFF){
		    trigger_id_pre=0;
		  }
		  else{
		    trigger_id_pre++;
		  }
		  if(trigger_id_pre != trigger_id){
		    printf("ERROR :incontinuous trigger id\n");
		  }
		}
		trigger_id_pre=trigger_id;
		
                for(int i=0;i<90;i++){
                    hxpos[i]->Fill(Xpos[i]);
                    hypos[i]->Fill(Ypos[i]);
                    hxneg[i]->Fill(Xneg[i]);
                    hyneg[i]->Fill(Yneg[i]);
                }
                tree->Fill();
		//
		for(int j=0;j<41;j++){
		  x_dy8_pos[j]=Xpos[id8[j]];
		  x_dy5_pos[j]=Xpos[id5[j]];

		  x_dy8_neg[j]=Xneg[id8[40-j]];
		  x_dy5_neg[j]=Xneg[id5[40-j]];

		  y_dy8_pos[j]=Ypos[id8[40-j]];
		  y_dy5_pos[j]=Ypos[id5[40-j]];

		  y_dy8_neg[j]=Yneg[id8[j]];
		  y_dy5_neg[j]=Yneg[id5[j]];
		}
		tree_psd_out->Fill();
            }
        }
        else{
            break;
        }
    }
    std::cout<< event_num <<" events converted totally!"<<std::endl;

    in.close();
    f->Write(0,TObject::kOverwrite);//overwrite AutoSave keys
    f->Close();
    delete f;
  }
  else{
    printf("error: %s can not be created!\n",outfile);
    delete f;
    return -1;
  }
    return 0;
}

Double_t langaufun(Double_t *x, Double_t *par) {

   //Fit parameters:
   //par[0]=Width (scale) parameter of Landau density
   //par[1]=Most Probable (MP, location) parameter of Landau density
   //par[2]=Total area (integral -inf to inf, normalization constant)
   //par[3]=Width (sigma) of convoluted Gaussian function
   //
   //In the Landau distribution (represented by the CERNLIB approximation),
   //the maximum is located at x=-0.22278298 with the location parameter=0.
   //This shift is corrected within this function, so that the actual
   //maximum is identical to the MP parameter.

      // Numeric constants
      Double_t invsq2pi = 0.3989422804014;   // (2 pi)^(-1/2)
      Double_t mpshift  = -0.22278298;       // Landau maximum location

      // Control constants
      Double_t np = 100.0;      // number of convolution steps
      Double_t sc =   5.0;      // convolution extends to +-sc Gaussian sigmas

      // Variables
      Double_t xx;
      Double_t mpc;
      Double_t fland;
      Double_t sum = 0.0;
      Double_t xlow,xupp;
      Double_t step;
      Double_t i;


      // MP shift correction
      mpc = par[1] - mpshift * par[0];

      // Range of convolution integral
      xlow = x[0] - sc * par[3];
      xupp = x[0] + sc * par[3];

      step = (xupp-xlow) / np;

      // Convolution integral of Landau and Gaussian by sum
      for(i=1.0; i<=np/2; i++) {
         xx = xlow + (i-.5) * step;
         fland = TMath::Landau(xx,mpc,par[0]) / par[0];
         sum += fland * TMath::Gaus(x[0],xx,par[3]);

         xx = xupp - (i-.5) * step;
         fland = TMath::Landau(xx,mpc,par[0]) / par[0];
         sum += fland * TMath::Gaus(x[0],xx,par[3]);
      }

      return (par[2] * step * sum * invsq2pi / par[3]);
}


TF1* langaus( TH1F *poHist,float& mpv,float& fwhm,float ped_mean,float ped_sigma)
{
    // Setting fit range and start values
    Double_t fr[2];
    Double_t sv[4],paramlow[4],paramhigh[4],fparam[4],fpe[4];
    Double_t chisqr;
    Int_t ndf;

    fr[0]=ped_mean+25*ped_sigma;
    fr[1]=fr[0]+2500;

    paramlow[0]=0.0;paramlow[1]=400.0;paramlow[2]=10;paramlow[3]=0.0;
    paramhigh[0]=200.0;paramhigh[1]=1500.0;paramhigh[2]=10000000.0;paramhigh[3]=200.0;

    sv[0]=30;
    sv[1]=poHist->GetBinCenter(poHist->GetMaximumBin());
    sv[2]=poHist->Integral(poHist->GetBin(fr[0]),poHist->GetBin(fr[1]));
    sv[3]=50;

    TF1 *fit = langaufit(poHist,fr,sv,paramlow,paramhigh,fparam,fpe,&chisqr,&ndf);

    mpv=fparam[1];
    fwhm=fparam[0];

    return fit;
}

TF1 *langaufit(TH1F *his, Double_t *fitrange, Double_t *startvalues, Double_t *parlimitslo, Double_t *parlimitshi, Double_t *fitparams, Double_t *fiterrors, Double_t *ChiSqr, Int_t *NDF)
{
   // Once again, here are the Landau * Gaussian parameters:
   //   par[0]=Width (scale) parameter of Landau density
   //   par[1]=Most Probable (MP, location) parameter of Landau density
   //   par[2]=Total area (integral -inf to inf, normalization constant)
   //   par[3]=Width (sigma) of convoluted Gaussian function
   //
   // Variables for langaufit call:
   //   his             histogram to fit
   //   fitrange[2]     lo and hi boundaries of fit range
   //   startvalues[4]  reasonable start values for the fit
   //   parlimitslo[4]  lower parameter limits
   //   parlimitshi[4]  upper parameter limits
   //   fitparams[4]    returns the final fit parameters
   //   fiterrors[4]    returns the final fit errors
   //   ChiSqr          returns the chi square
   //   NDF             returns ndf

   Int_t i;
   Char_t FunName[100];

   sprintf(FunName,"Fitfcn_%s",his->GetName());

   TF1 *ffitold = (TF1*)gROOT->GetListOfFunctions()->FindObject(FunName);
   if (ffitold) delete ffitold;

   TF1 *ffit = new TF1(FunName,langaufun,fitrange[0],fitrange[1],4);
   ffit->SetParameters(startvalues);
   ffit->SetParNames("Width","MP","Area","GSigma");

   for (i=0; i<4; i++) {
      ffit->SetParLimits(i, parlimitslo[i], parlimitshi[i]);
   }

   his->Fit(FunName,"RB0q");   // fit within specified range, use ParLimits, do not plot

   ffit->GetParameters(fitparams);    // obtain fit parameters
   for (i=0; i<4; i++) {
      fiterrors[i] = ffit->GetParError(i);     // obtain fit parameter errors
   }
   ChiSqr[0] = ffit->GetChisquare();  // obtain chi^2
   NDF[0] = ffit->GetNDF();           // obtain ndf

   return (ffit);              // return fit function

}


TF1* pedfit(TH1F* poHist,float ped_mean,float ped_sigma,float& pedout_mean,float& pedout_sigma)
{
    //get pedestal from mips histogram
    TF1* fgaus=new TF1(Form("ped_%s",poHist->GetName()),"gaus",ped_mean-30.0,ped_mean+30.0);
    //fgaus->SetParameter(1,ped_mean);
    //fgaus->SetParameter(2,ped_sigma);
    poHist->Fit(Form("ped_%s",poHist->GetName()),"Rq0");
    pedout_mean=fgaus->GetParameter(1);
    pedout_sigma=fgaus->GetParameter(2);

    return fgaus;
}

TF1* linear_fit(TProfile* hprofile,Double_t *fitrange)
{
    char FunName[100];
    sprintf(FunName,"Fitfcn_%s",hprofile->GetName());
    TF1* ffitold=(TF1*)gROOT->GetListOfFunctions()->FindObject(FunName);
    if(ffitold) delete ffitold;

    TF1* ffit=new TF1(FunName,"pol1",fitrange[0],fitrange[1]);
    hprofile->Fit(FunName,"R0");

    return ffit;
}

int draw_channels(const char* pardir,const char* filename,const char* outDir,const char* outName)
{
  float xpos_mean[90]={584.294617,576.240051,581.164001,630.100952,633.256775,590.692505,619.308228,538.72583,589.004883,537.398987,508.311035,505.871155,497.872742,480.174408,421.386871,433.97702,419.323151,382.717804,356.94162,323.870361,310.849365,332.737915,242.328568,366.943726,409.12381,410.210663,417.011932,392.849243,446.975281,460.704498,441.16098,418.733307,438.953766,476.407196,459.46875,425.584656,502.695648,441.838135,453.17099,400.869904,462.463562,453.460693,436.400116,453.824158,358.34964,432.515961,494.741791,531.399963,457.447693,556.542175,545.790466,462.783691,560.625,468.630524,556.486145,563.903809,552.457825,499.961823,561.731995,577.157166,560.032776,682.703491,563.598267,588.721313,614.796936,584.363281,687.861755,550.098206,160.243561,204.823334,246.58548,204.760941,215.072159,319.15799,188.806015,263.66214,271.196228,273.882446,260.506165,285.928772,338.344177,284.140259,326.216522,373.446381,351.823212,434.46402,342.91568,427.743408,451.03656,420.587067};
  float xneg_mean[90]={412.640625,420.22049,383.134827,363.454803,361.210236,400.499725,369.742279,360.144531,341.975983,310.302216,353.92514,289.153137,356.269684,354.769806,452.026367,351.103455,409.603271,392.92334,459.708038,464.962524,443.755798,411.947632,438.929901,469.589294,455.414917,449.192505,490.115936,433.367493,467.21521,451.273621,406.520447,382.199463,433.963623,428.524841,405.993591,397.500977,400.979492,383.901123,404.991608,393.934998,365.254852,303.645355,325.478607,374.719116,275.742767,376.912628,437.347778,478.959534,536.700378,524.631592,583.300903,570.28241,543.453674,567.08075,534.942139,581.025146,506.381989,561.965759,566.79541,500.279236,543.35144,521.750305,486.412689,434.306854,430.802887,398.623077,399.741272,424.579285,569.753906,606.433289,670.790527,620.566284,649.590088,559.81604,614.076477,633.877014,659.122986,552.479919,599.680847,589.135254,517.895813,547.584595,527.218689,510.608276,543.245911,470.077576,481.11615,411.843903,479.949707,456.42569};
  float ypos_mean[90]={525.703735,454.832947,540.47821,480.421631,503.846558,478.363373,462.736481,430.16745,432.864044,363.705963,357.424957,403.753326,469.51767,415.723755,370.797699,484.94693,440.466858,424.605103,401.859497,483.259949,392.386108,360.093842,427.05307,460.790405,512.390076,414.44043,436.080139,344.632263,347.17569,319.066681,321.586426,328.432892,322.262207,263.6362,251.817184,139.394302,185.291168,159.283936,202.315369,146.166489,200.430099,245.470413,274.535278,227.144058,291.889709,256.011444,507.027802,469.424683,472.413086,491.288483,459.775482,478.343262,386.282349,503.630066,505.038788,524.324097,447.53717,511.572845,455.635162,431.90564,465.718964,499.220612,501.192749,526.60083,472.679718,505.577515,505.75882,552.519775,453.367157,410.998352,324.468903,434.47345,467.277191,457.264221,461.12851,533.402039,477.564941,502.423126,491.461853,528.89917,532.13623,511.590393,498.92868,591.384277,580.792969,486.374481,587.452271,540.275513,574.382202,580.537659};
  float yneg_mean[90]={407.284943,378.982086,422.624146,296.235718,385.15686,349.989105,432.825409,367.855988,316.949066,374.797791,353.974365,371.516418,362.315826,346.890961,354.726685,402.84787,345.36438,296.271027,323.732147,298.757172,254.023666,292.669525,265.195435,491.397552,403.231476,423.547791,462.401123,481.291199,501.159454,519.220886,469.177551,504.120209,444.380005,551.471191,488.32724,469.207947,512.630127,465.746796,457.369202,455.684875,471.269775,462.078705,417.789703,512.679138,414.644287,412.095337,430.085846,432.139282,443.692017,477.442932,488.097595,450.599335,403.671021,435.282806,423.522491,426.41626,386.354248,441.463104,440.246643,383.401611,450.13147,418.886169,355.929108,326.737152,414.888794,360.327026,332.082642,301.811798,389.747803,436.322327,382.288666,388.173309,475.136536,421.518768,390.63446,474.334747,431.192841,447.824921,397.248566,411.191528,446.79364,308.062622,330.206726,337.361511,346.767059,327.43219,337.078369,232.048798,162.122818,173.874573};
  
  float xpos_sigma[90]={3.736824,3.856736,1.956659,3.685497,3.732478,3.641981,3.752078,3.748013,3.865553,3.798889,3.62252,3.749232,3.782544,3.775584,3.610459,3.685533,3.673125,3.746416,3.886756,3.684114,3.734951,1.890485,3.844028,4.069125,3.893293,1.964721,4.005772,3.865482,3.838456,3.919654,3.785001,3.886893,3.877472,4.00319,3.841863,3.885488,3.591914,3.698619,3.737026,3.777386,3.771372,3.698289,3.826869,3.717773,2.091568,3.760845,3.833019,3.646833,2.122498,3.779329,3.772663,3.805115,3.598073,3.659907,3.658247,3.672962,3.49913,3.613281,3.827566,3.838894,3.863858,3.693914,3.808953,3.983469,3.857419,3.787388,3.981404,2.036184,3.852746,3.841657,2.178376,3.781579,3.810807,3.672637,3.842719,3.560457,3.600716,3.655565,3.407631,3.813492,3.670916,3.782258,3.666457,3.586771,3.765878,3.750612,3.638302,3.664599,3.68575,1.835002};
  float xneg_sigma[90]={3.831681,3.682275,2.135663,3.661865,3.60962,3.70191,3.830006,3.641161,3.503102,3.467145,3.665128,3.771296,3.720505,3.581003,3.626289,3.600181,3.570236,3.400048,3.553622,3.677131,3.590026,1.949121,3.608756,3.706244,3.849428,2.179197,3.706939,3.821805,3.80637,3.812203,3.783731,3.793351,3.829973,3.851948,3.697848,3.85562,3.798345,3.823635,3.756814,3.690592,3.725092,3.73193,3.617701,3.573387,2.140543,3.610691,3.625423,3.598734,2.320743,3.608574,3.429263,3.498973,3.497967,3.686585,3.514136,3.521484,3.647474,3.662632,3.655866,3.529503,3.63179,3.567672,3.696316,3.836162,3.83159,3.936044,3.931192,2.296582,3.658065,3.796442,1.831813,3.67332,3.623253,3.686143,3.606783,3.571892,3.4095,3.598759,3.662241,3.660496,3.554791,3.732886,3.785451,3.68656,3.879786,3.669902,3.759837,3.818184,3.826776,2.293706};
  float ypos_sigma[90]={4.014359,3.844612,2.148752,3.810551,3.763489,3.860472,3.898153,3.780694,3.841065,3.837565,3.819789,3.610198,3.683641,3.831621,3.751889,3.673229,3.473596,3.710137,3.649487,3.608658,3.603105,2.087114,3.834611,3.960583,3.674311,2.355702,3.968959,3.924924,3.879615,3.98435,4.042142,4.097819,3.868711,4.094149,3.831926,4.132722,3.88276,3.993323,3.839893,3.956246,3.727843,3.874009,3.949801,3.883882,1.902613,3.974283,3.793596,3.678008,1.87146,3.606533,3.648665,3.741112,3.678857,3.60664,3.531926,3.709444,3.838147,3.532824,3.98011,3.766423,3.803941,3.724727,3.458132,4.029863,3.637432,3.773289,3.765155,1.882769,3.741154,3.817095,2.096767,3.832448,3.869336,3.775453,3.876192,3.704838,3.697862,3.530832,3.67084,3.771876,3.633844,3.476959,4.028599,3.767445,3.62454,3.734328,3.755919,3.604094,3.864374,1.912329};
  float yneg_sigma[90]={3.697204,3.636625,1.97408,3.747848,3.791435,3.796491,3.670822,3.763608,3.747871,3.65173,3.713995,3.614409,3.425848,3.695052,3.679054,3.389057,3.684838,3.58604,3.591891,3.683398,3.863297,1.842534,3.68907,3.576635,3.677915,2.013921,3.854437,3.825918,3.665926,3.781783,3.760055,3.747263,3.752604,3.765151,3.614896,3.621856,3.415171,3.95441,3.678627,3.672492,3.71629,3.779259,3.781167,3.352875,2.269341,3.800362,3.64822,3.666638,1.926999,3.856159,3.571758,3.628246,3.511022,3.785855,3.437363,3.607491,3.795056,3.497369,3.471038,3.856634,3.887709,3.822003,3.782335,3.793339,3.933351,3.790573,3.949285,2.037488,3.706804,3.781064,1.831734,3.789472,3.783057,3.578064,3.464689,3.783234,3.695818,3.770837,3.857774,3.561934,3.751848,3.821143,3.664293,3.747664,3.756274,3.815036,3.761996,3.563645,3.840531,2.102546};
  
  int id8[41]={0,1,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,22,46,47,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64,65,66};
  int id5[41]={23,24,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,45,68,69,71,72,73,74,75,76,77,78,79,80,81,82,83,84,85,86,87,88};
  
  TFile* file=new TFile(Form("%s/%s",pardir,filename));
  int channel;
  float ped,sigma;
  char label[4][5]={"xpos","xneg","ypos","yneg"};
  TFile* file_ped=new TFile(Form("%s/new_ped.root",outDir),"recreate");
  TTree* tree_ped[4];
  for(int i=0;i<4;i++){
    tree_ped[i]=new TTree(Form("%s_ped",label[i]),Form("%s_ped",label[i]));
    tree_ped[i]->Branch("channel",&channel,"channel/I");
    tree_ped[i]->Branch("mean",&ped,"mean/F");
    tree_ped[i]->Branch("sigma",&sigma,"sigma/F");
  }
  
  TCanvas* xcan=new TCanvas("xcan","xcan",800,900);
  TCanvas* ycan=new TCanvas("ycan","ycan",800,900);
  xcan->Divide(2,2);
  ycan->Divide(2,2);
  for(int i=0;i<4;i++){
    xcan->cd(i+1);gPad->SetLogy();
    ycan->cd(i+1);gPad->SetLogy();
  }
  xcan->Print(Form("%s/xmips.pdf[",outDir));
  ycan->Print(Form("%s/ymips.pdf[",outDir));
  /*
   *    FILE* fp_xpos=fopen(Form("%s/xpos_mip.csv",pardir),"w");
   *    FILE* fp_xneg=fopen(Form("%s/xneg_mip.csv",pardir),"w");
   *    FILE* fp_ypos=fopen(Form("%s/ypos_mip.csv",pardir),"w");
   *    FILE* fp_yneg=fopen(Form("%s/yneg_mip.csv",pardir),"w");
   */
  //FILE* fp_mip=fopen(Form("%s/mips.csv",outDir),"w");
  //fprintf(fp_mip,"channel,xpos,xneg,ypos,yneg\n");
  FILE* fp_mip2=fopen(Form("%s/mips_2.csv",outDir),"w");
  fprintf(fp_mip2,"channel,xpos,xneg,ypos,yneg\n");
  
  TH1F *hxpos8[41],*hxpos5[41],*hxneg8[41],*hxneg5[41];
  TH1F *hypos8[41],*hypos5[41],*hyneg8[41],*hyneg5[41];
  /*
   *    TTree* tree_in=(TTree*)file->Get("PSD");
   *    Int_t begin=130001;
   *    for(int ch_id=0;ch_id<41;ch_id++){
   *        hxpos8[41]=new TH1F(Form("xpos_%d_dy8",ch_id+1),Form("xpos_%d_dy8",ch_id+1),3000,100,3100);
   *        tree_in->Project(Form("xpos_%d_dy8",ch_id+1),Form("xpos[%d]",id8[ch_id]),"","",1000000000,begin);
   *        hxpos5[41]=new TH1F(Form("xpos_%d_dy5",ch_id+1),Form("xpos_%d_dy5",ch_id+1),800,100,900);
   *        tree_in->Project(Form("xpos_%d_dy5",ch_id+1),Form("xpos[%d]",id5[ch_id]),"","",1000000000,begin);
   *        hxneg8[41]=new TH1F(Form("xneg_%d_dy8",ch_id+1),Form("xneg_%d_dy8",ch_id+1),3000,100,3100);
   *        tree_in->Project(Form("xneg_%d_dy8",ch_id+1),Form("xneg[%d]",id8[40-ch_id]),"","",1000000000,begin);
   *        hxneg5[41]=new TH1F(Form("xneg_%d_dy5",ch_id+1),Form("xneg_%d_dy5",ch_id+1),800,100,900);
   *        tree_in->Project(Form("xneg_%d_dy5",ch_id+1),Form("xneg[%d]",id5[40-ch_id]),"","",1000000000,begin);
   *        hypos8[41]=new TH1F(Form("ypos_%d_dy8",ch_id+1),Form("ypos_%d_dy8",ch_id+1),3000,100,3100);
   *        tree_in->Project(Form("ypos_%d_dy8",ch_id+1),Form("ypos[%d]",id8[40-ch_id]),"","",1000000000,begin);
   *        hypos5[41]=new TH1F(Form("ypos_%d_dy5",ch_id+1),Form("ypos_%d_dy5",ch_id+1),800,100,900);
   *        tree_in->Project(Form("ypos_%d_dy5",ch_id+1),Form("ypos[%d]",id5[40-ch_id]),"","",1000000000,begin);
   *        hyneg8[41]=new TH1F(Form("yneg_%d_dy8",ch_id+1),Form("yneg_%d_dy8",ch_id+1),3000,100,3100);
   *        tree_in->Project(Form("yneg_%d_dy8",ch_id+1),Form("yneg[%d]",id8[ch_id]),"","",1000000000,begin);
   *        hyneg5[41]=new TH1F(Form("yneg_%d_dy5",ch_id+1),Form("yneg_%d_dy5",ch_id+1),800,100,900);
   *        tree_in->Project(Form("yneg_%d_dy5",ch_id+1),Form("yneg[%d]",id5[ch_id]),"","",1000000000,begin);
   }
   */
  TF1* ffit;
  float mpv,fwhm;
  for(int ch_id=0;ch_id<41;ch_id++){
    //fprintf(fp_mip,"%d,",ch_id+1);
    fprintf(fp_mip2,"%d,",ch_id+1);
    
    hxpos8[ch_id]=(TH1F*)file->Get(Form("xpos_%d",id8[ch_id]+1));
    hxpos8[ch_id]->Rebin(10);
    hxpos8[ch_id]->SetTitle(Form("X_layer,Strip_%d,pos_dy8",ch_id+1));
    ffit=langaus(hxpos8[ch_id],mpv,fwhm,xpos_mean[id8[ch_id]],xpos_sigma[id8[ch_id]]);
    xcan->cd(1);
    hxpos8[ch_id]->Draw();
    ffit->Draw("lsame");
    //fprintf(fp_mip,"%.2f,",(mpv-xpos_mean[id8[ch_id]]));
    ffit=pedfit(hxpos8[ch_id],xpos_mean[id8[ch_id]],xpos_sigma[id8[ch_id]],ped,sigma);
    ffit->Draw("lsame");
    fprintf(fp_mip2,"%.2f,",(mpv-ped));
    channel=id8[ch_id]+1;
    tree_ped[0]->Fill();
    
    hxpos5[ch_id]=(TH1F*)file->Get(Form("xpos_%d",id5[ch_id]+1));
    hxpos5[ch_id]->Rebin(10);
    hxpos5[ch_id]->SetTitle(Form("X_layer,Strip_%d,pos_dy5",ch_id+1));
    ffit=pedfit(hxpos5[ch_id],xpos_mean[id5[ch_id]],xpos_sigma[id5[ch_id]],ped,sigma);
    channel=id5[ch_id]+1;
    tree_ped[0]->Fill();
    xcan->cd(2);
    hxpos5[ch_id]->Draw();
    ffit->Draw("lsame");
    
    hxneg8[ch_id]=(TH1F*)file->Get(Form("xneg_%d",id8[40-ch_id]+1));
    hxneg8[ch_id]->Rebin(10);
    hxneg8[ch_id]->SetTitle(Form("X_layer,Strip_%d,neg_dy8",ch_id+1));
    ffit=langaus(hxneg8[ch_id],mpv,fwhm,xneg_mean[id8[40-ch_id]],xneg_sigma[id8[40-ch_id]]);
    xcan->cd(3);
    hxneg8[ch_id]->Draw();
    ffit->Draw("lsame");
    //fprintf(fp_mip,"%.2f,",(mpv-xneg_mean[id8[40-ch_id]]));
    ffit=pedfit(hxneg8[ch_id],xneg_mean[id8[40-ch_id]],xneg_sigma[id8[40-ch_id]],ped,sigma);
    ffit->Draw("lsame");
    fprintf(fp_mip2,"%.2f,",(mpv-ped));
    channel=id8[40-ch_id]+1;
    tree_ped[1]->Fill();
    
    hxneg5[ch_id]=(TH1F*)file->Get(Form("xneg_%d",id5[40-ch_id]+1));
    hxneg5[ch_id]->Rebin(10);
    hxneg5[ch_id]->SetTitle(Form("X_layer,Strip_%d,neg_dy5",ch_id+1));
    ffit=pedfit(hxneg5[ch_id],xneg_mean[id5[40-ch_id]],xneg_sigma[id5[40-ch_id]],ped,sigma);
    channel=id5[40-ch_id]+1;
    tree_ped[1]->Fill();
    xcan->cd(4);
    hxneg5[ch_id]->Draw();
    ffit->Draw("lsame");
    xcan->Print(Form("%s/xmips.pdf",outDir));
    
    hypos8[ch_id]=(TH1F*)file->Get(Form("ypos_%d",id8[40-ch_id]+1));
    hypos8[ch_id]->Rebin(10);
    hypos8[ch_id]->SetTitle(Form("Y_layer,Strip_%d,pos_dy8",ch_id+1));
    ffit=langaus(hypos8[ch_id],mpv,fwhm,ypos_mean[id8[40-ch_id]],ypos_sigma[id8[40-ch_id]]);
    ycan->cd(1);
    hypos8[ch_id]->Draw();
    ffit->Draw("lsame");
    //fprintf(fp_mip,"%.2f,",(mpv-ypos_mean[id8[40-ch_id]]));
    ffit=pedfit(hypos8[ch_id],ypos_mean[id8[40-ch_id]],ypos_sigma[id8[40-ch_id]],ped,sigma);
    ffit->Draw("lsame");
    fprintf(fp_mip2,"%.2f,",(mpv-ped));
    channel=id8[40-ch_id]+1;
    tree_ped[2]->Fill();
    
    hypos5[ch_id]=(TH1F*)file->Get(Form("ypos_%d",id5[40-ch_id]+1));
    hypos5[ch_id]->Rebin(10);
    hypos5[ch_id]->SetTitle(Form("Y_layer,Strip_%d,pos_dy5",ch_id+1));
    ffit= pedfit(hypos5[ch_id],ypos_mean[id5[40-ch_id]],ypos_sigma[id5[40-ch_id]],ped,sigma);
    channel=id5[40-ch_id]+1;
    tree_ped[2]->Fill();
    ycan->cd(2);
    hypos5[ch_id]->Draw();
    ffit->Draw("lsame");
    
    hyneg8[ch_id]=(TH1F*)file->Get(Form("yneg_%d",id8[ch_id]+1));
    hyneg8[ch_id]->Rebin(10);
    hyneg8[ch_id]->SetTitle(Form("Y_layer,Strip_%d,neg_dy8",ch_id+1));
    ffit=langaus(hyneg8[ch_id],mpv,fwhm,yneg_mean[id8[ch_id]],yneg_sigma[id8[ch_id]]);
    ycan->cd(3);
    hyneg8[ch_id]->Draw();
    ffit->Draw("lsame");
    //fprintf(fp_mip,"%.2f\n",(mpv-yneg_mean[id8[ch_id]]));
    ffit=pedfit(hyneg8[ch_id],yneg_mean[id8[ch_id]],yneg_sigma[id8[ch_id]],ped,sigma);
    ffit->Draw("lsame");
    fprintf(fp_mip2,"%.2f\n",(mpv-ped));
    channel=id8[ch_id]+1;
    tree_ped[3]->Fill();
    
    hyneg5[ch_id]=(TH1F*)file->Get(Form("yneg_%d",id5[ch_id]+1));
    hyneg5[ch_id]->Rebin(10);
    hyneg5[ch_id]->SetTitle(Form("Y_layer,Strip_%d,neg_dy5",ch_id+1));
    ffit=pedfit(hyneg5[ch_id],yneg_mean[id5[ch_id]],yneg_sigma[id5[ch_id]],ped,sigma);
    channel=id5[ch_id]+1;
    tree_ped[3]->Fill();
    ycan->cd(4);
    hyneg5[ch_id]->Draw();
    ffit->Draw("lsame");
    ycan->Print(Form("%s/ymips.pdf",outDir));
  }
  
  xcan->Print(Form("%s/xmips.pdf]",outDir));
  ycan->Print(Form("%s/ymips.pdf]",outDir));
  
  TFile* file_out=new TFile(Form("%s/%s",outDir,outName),"update");
  for(int i=0;i<41;i++){
    hxpos8[i]->Write(0,TObject::kOverwrite);
    hxneg5[i]->Write(0,TObject::kOverwrite);
    hxpos8[i]->Write(0,TObject::kOverwrite);
    hxpos8[i]->Write(0,TObject::kOverwrite);
    
    hypos8[i]->Write(0,TObject::kOverwrite);
    hyneg5[i]->Write(0,TObject::kOverwrite);
    hypos8[i]->Write(0,TObject::kOverwrite);
    hypos8[i]->Write(0,TObject::kOverwrite);
  }
  
  file_ped->cd();
  for(int i=0;i<4;i++){
    tree_ped[i]->Write();
  }
  delete file_ped;
  delete file_out;
  delete xcan;
  delete ycan;
  delete file;
  
  //fclose(fp_mip);
  fclose(fp_mip2);
  
  return 0;
   }

   int draw_relp(const Char_t* testfile,const Char_t* reffile,const Char_t* outdir,const char* outName)
   {
     int id8[41]={0,1,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,22,46,47,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64,65,66};
     int id5[41]={23,24,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,45,68,69,71,72,73,74,75,76,77,78,79,80,81,82,83,84,85,86,87,88};
     
     TString label[4]={"xpos","xneg","ypos","yneg"};
     Float_t sigma,sigma_ref,ratio_sigma;
     Float_t mean,mean_ref,ratio_mean;
     Int_t channel;
     
     TFile *file_in=new TFile(Form("%s",testfile));
     TFile *file_ref=new TFile(Form("%s",reffile));
     TFile *file_out=new TFile(Form("%s/%s",outdir,outName),"update");
     
     TTree* tree_test;
     TTree* tree_ref;
     
     TMultiGraph *mg_sigma_x;
     TMultiGraph *mg_ref_sigma_x;
     TMultiGraph *mg_sigma_y;
     TMultiGraph *mg_ref_sigma_y;
     TGraph* graphp_sigma_xpos;
     TGraph* graphp_ref_sigma_xpos;
     TGraph* graphp_sigma_ypos;
     TGraph* graphp_ref_sigma_ypos;
     
     TGraph* graphp_sigma_xneg;
     TGraph* graphp_ref_sigma_xneg;
     TGraph* graphp_sigma_yneg;
     TGraph* graphp_ref_sigma_yneg;
     
     
     //-----------------------------------------------------------------------------------------------
     graphp_sigma_xpos=new TGraph(41);
     graphp_sigma_xpos->SetTitle("X_layer_dy8: sigma of pedestal");
     graphp_ref_sigma_xpos=new TGraph(41);
     graphp_ref_sigma_xpos->SetTitle("X_layer_dy8: ratio of sigma value of ped");
     graphp_ref_sigma_xpos->SetMarkerStyle(26);
     graphp_sigma_ypos=new TGraph(41);
     graphp_sigma_ypos->SetTitle("Y_layer_dy8: sigma of pedestal");
     graphp_ref_sigma_ypos=new TGraph(41);
     graphp_ref_sigma_ypos->SetTitle("Y_layer_dy8: ratio of sigma value of ped");
     graphp_ref_sigma_ypos->SetMarkerStyle(26);
     
     graphp_sigma_xneg=new TGraph(41);
     graphp_sigma_xneg->SetTitle("X_layer_dy8: sigma of pedestal");
     graphp_ref_sigma_xneg=new TGraph(41);
     graphp_ref_sigma_xneg->SetTitle("X_layer_dy8: ratio of sigma value of ped");
     graphp_ref_sigma_xneg->SetMarkerStyle(26);
     graphp_sigma_yneg=new TGraph(41);
     graphp_sigma_yneg->SetTitle("Y_layer_dy8: sigma of pedestal");
     graphp_ref_sigma_yneg=new TGraph(41);
     graphp_ref_sigma_yneg->SetTitle("Y_layer_dy8: ratio of sigma value of ped");
     graphp_ref_sigma_yneg->SetMarkerStyle(26);
     
     mg_sigma_x = new TMultiGraph();
     mg_sigma_x->SetName("mg_x_sigma_dy8");
     mg_sigma_x->SetTitle("X_layer_dy8: sigma of pedestal");
     mg_sigma_x->Add(graphp_sigma_xpos);
     mg_sigma_x->Add(graphp_sigma_xneg);
     mg_sigma_y = new TMultiGraph();
     mg_sigma_y->SetName("mg_y_sigma_dy8");
     mg_sigma_y->SetTitle("Y_layer_dy8: sigma of pedestal");
     mg_sigma_y->Add(graphp_sigma_ypos);
     mg_sigma_y->Add(graphp_sigma_yneg);
     mg_ref_sigma_x = new TMultiGraph();
     mg_ref_sigma_x->SetName("mg_x_ref_sigma_dy8");
     mg_ref_sigma_x->SetTitle("X_layer_dy8: ratio of sigma value of ped");
     mg_ref_sigma_x->Add(graphp_ref_sigma_xpos);
     mg_ref_sigma_x->Add(graphp_ref_sigma_xneg);
     mg_ref_sigma_y = new TMultiGraph();
     mg_ref_sigma_y->SetName("mg_y_ref_sigma_dy8");
     mg_ref_sigma_y->SetTitle("Y_layer_dy8: ratio of sigma value of ped");
     mg_ref_sigma_y->Add(graphp_ref_sigma_ypos);
     mg_ref_sigma_y->Add(graphp_ref_sigma_yneg);
     
     for(int fee_id=0;fee_id<2;fee_id++){
       //-----------------X--------------------------------------------------
       tree_test=(TTree*)file_in->Get(Form("%s_ped",label[fee_id].Data()));
       tree_test->SetBranchAddress("channel",&channel);
       tree_test->SetBranchAddress("sigma",&sigma);
       tree_test->BuildIndex("channel");
       
       tree_ref=(TTree*)file_ref->Get(Form("%s_ped",label[fee_id].Data()));
       tree_ref->SetBranchAddress("sigma",&sigma_ref);
       tree_ref->BuildIndex("channel");
       
       for(int ch_id=0;ch_id<41;ch_id++){
	 tree_test->GetEntryWithIndex(id8[ch_id]+1);
	 tree_ref->GetEntryWithIndex(channel);
	 if(fee_id==0){
	   graphp_sigma_xpos->SetPoint(ch_id,ch_id+1,sigma);
	 }
	 else{
	   graphp_sigma_xneg->SetPoint(ch_id,41-ch_id,-sigma);
	 }
	 ratio_sigma=sigma/sigma_ref;
	 if(fee_id==0){
	   graphp_ref_sigma_xpos->SetPoint(ch_id,ch_id+1,ratio_sigma);
	 }
	 else{
	   graphp_ref_sigma_xneg->SetPoint(ch_id,41-ch_id,-ratio_sigma);
	 }
       }
       //-----------------Y---------------------------------------------------------------
       tree_test=(TTree*)file_in->Get(Form("%s_ped",label[fee_id+2].Data()));
       tree_test->SetBranchAddress("channel",&channel);
       tree_test->SetBranchAddress("sigma",&sigma);
       tree_test->BuildIndex("channel");
       
       tree_ref=(TTree*)file_ref->Get(Form("%s_ped",label[fee_id+2].Data()));
       tree_ref->SetBranchAddress("sigma",&sigma_ref);
       tree_ref->BuildIndex("channel");
       
       for(int ch_id=0;ch_id<41;ch_id++){
	 tree_test->GetEntryWithIndex(id8[ch_id]+1);
	 tree_ref->GetEntryWithIndex(channel);
	 if(fee_id==0){
	   graphp_sigma_ypos->SetPoint(ch_id,41-ch_id,sigma);
	 }
	 else{
	   graphp_sigma_yneg->SetPoint(ch_id,ch_id+1,-sigma);
	 }
	 ratio_sigma=sigma/sigma_ref;
	 if(fee_id==0){
	   graphp_ref_sigma_ypos->SetPoint(ch_id,41-ch_id,ratio_sigma);
	 }
	 else{
	   graphp_ref_sigma_yneg->SetPoint(ch_id,ch_id+1,-ratio_sigma);
	 }
       }
     }
     
     file_out->cd();
     TCanvas* canp=new TCanvas("canp","Rel_ped VS positon",900,900);
       canp->Divide(2,2);
       canp->cd(1);
       mg_sigma_x->Draw("A*");
       mg_sigma_x->Write(0,TObject::kOverwrite);
       canp->cd(2);
       mg_ref_sigma_x->Draw("AP");
       mg_ref_sigma_x->Write(0,TObject::kOverwrite);
       canp->cd(3);
       mg_sigma_y->Draw("A*");
       mg_sigma_y->Write(0,TObject::kOverwrite);
       canp->cd(4);
       mg_ref_sigma_y->Draw("AP");
       mg_ref_sigma_y->Write(0,TObject::kOverwrite);
       canp->Print(Form("%s/pedrel_sigma_dy8.pdf",outdir));
       
       delete mg_sigma_x;
       delete mg_sigma_y;
       delete mg_ref_sigma_x;
       delete mg_ref_sigma_y;
       delete canp;
       //------------------------------------------------
       graphp_sigma_xpos=new TGraph(41);
       graphp_sigma_xpos->SetTitle("X_layer_dy5: sigma of pedestal");
       graphp_ref_sigma_xpos=new TGraph(41);
       graphp_ref_sigma_xpos->SetTitle("X_layer_dy5: ratio of sigma value of ped");
       graphp_ref_sigma_xpos->SetMarkerStyle(26);
       graphp_sigma_ypos=new TGraph(41);
       graphp_sigma_ypos->SetTitle("Y_layer_dy5: sigma of pedestal");
       graphp_ref_sigma_ypos=new TGraph(41);
       graphp_ref_sigma_ypos->SetTitle("Y_layer_dy5: ratio of sigma value of ped");
       graphp_ref_sigma_ypos->SetMarkerStyle(26);
       
       graphp_sigma_xneg=new TGraph(41);
       graphp_sigma_xneg->SetTitle("X_layer_dy5: sigma of pedestal");
       graphp_ref_sigma_xneg=new TGraph(41);
       graphp_ref_sigma_xneg->SetTitle("X_layer_dy5: ratio of sigma value of ped");
       graphp_ref_sigma_xneg->SetMarkerStyle(26);
       graphp_sigma_yneg=new TGraph(41);
       graphp_sigma_yneg->SetTitle("Y_layer_dy5: sigma of pedestal");
       graphp_ref_sigma_yneg=new TGraph(41);
       graphp_ref_sigma_yneg->SetTitle("Y_layer_dy5: ratio of sigma value of ped");
       graphp_ref_sigma_yneg->SetMarkerStyle(26);
       
       mg_sigma_x = new TMultiGraph();
       mg_sigma_x->SetName("mg_x_sigma_dy5");
       mg_sigma_x->SetTitle("X_layer_dy5: sigma of pedestal");
       mg_sigma_x->Add(graphp_sigma_xpos);
       mg_sigma_x->Add(graphp_sigma_xneg);
       mg_sigma_y = new TMultiGraph();
       mg_sigma_y->SetName("mg_y_sigma_dy5");
       mg_sigma_y->SetTitle("Y_layer_dy5: sigma of pedestal");
       mg_sigma_y->Add(graphp_sigma_ypos);
       mg_sigma_y->Add(graphp_sigma_yneg);
       mg_ref_sigma_x = new TMultiGraph();
       mg_ref_sigma_x->SetName("mg_x_ref_sigma_dy5");
       mg_ref_sigma_x->SetTitle("X_layer_dy5: ratio of sigma value of ped");
       mg_ref_sigma_x->Add(graphp_ref_sigma_xpos);
       mg_ref_sigma_x->Add(graphp_ref_sigma_xneg);
       mg_ref_sigma_y = new TMultiGraph();
       mg_ref_sigma_y->SetName("mg_y_ref_sigma_dy5");
       mg_ref_sigma_y->SetTitle("Y_layer_dy5: ratio of sigma value of ped");
       mg_ref_sigma_y->Add(graphp_ref_sigma_ypos);
       mg_ref_sigma_y->Add(graphp_ref_sigma_yneg);
       
       for(int fee_id=0;fee_id<2;fee_id++){
	 //-----------------X--------------------------------------------------
	 tree_test=(TTree*)file_in->Get(Form("%s_ped",label[fee_id].Data()));
	 tree_test->SetBranchAddress("channel",&channel);
	 tree_test->SetBranchAddress("sigma",&sigma);
	 tree_test->BuildIndex("channel");
	 
	 tree_ref=(TTree*)file_ref->Get(Form("%s_ped",label[fee_id].Data()));
	 tree_ref->SetBranchAddress("sigma",&sigma_ref);
	 tree_ref->BuildIndex("channel");
	 
	 for(int ch_id=0;ch_id<41;ch_id++){
	   tree_test->GetEntryWithIndex(id5[ch_id]+1);
	   tree_ref->GetEntryWithIndex(channel);
	   if(fee_id==0){
	     graphp_sigma_xpos->SetPoint(ch_id,ch_id+1,sigma);
	   }
	   else{
	     graphp_sigma_xneg->SetPoint(ch_id,41-ch_id,-sigma);
	   }
	   ratio_sigma=sigma/sigma_ref;
	   if(fee_id==0){
	     graphp_ref_sigma_xpos->SetPoint(ch_id,ch_id+1,ratio_sigma);
	   }
	   else{
	     graphp_ref_sigma_xneg->SetPoint(ch_id,41-ch_id,-ratio_sigma);
	   }
	 }
	 //-----------------Y---------------------------------------------------------------
	 tree_test=(TTree*)file_in->Get(Form("%s_ped",label[fee_id+2].Data()));
	 tree_test->SetBranchAddress("channel",&channel);
	 tree_test->SetBranchAddress("sigma",&sigma);
	 tree_test->BuildIndex("channel");
	 
	 tree_ref=(TTree*)file_ref->Get(Form("%s_ped",label[fee_id+2].Data()));
	 tree_ref->SetBranchAddress("sigma",&sigma_ref);
	 tree_ref->BuildIndex("channel");
	 
	 for(int ch_id=0;ch_id<41;ch_id++){
	   tree_test->GetEntryWithIndex(id5[ch_id]+1);
	   tree_ref->GetEntryWithIndex(channel);
	   if(fee_id==0){
	     graphp_sigma_ypos->SetPoint(ch_id,41-ch_id,sigma);
	   }
	   else{
	     graphp_sigma_yneg->SetPoint(ch_id,ch_id+1,-sigma);
	   }
	   ratio_sigma=sigma/sigma_ref;
	   if(fee_id==0){
	     graphp_ref_sigma_ypos->SetPoint(ch_id,41-ch_id,ratio_sigma);
	   }
	   else{
	     graphp_ref_sigma_yneg->SetPoint(ch_id,ch_id+1,-ratio_sigma);
	   }
	 }
       }
       
       file_out->cd();
       canp=new TCanvas("canp","Rel_ped VS positon",900,900);
	 canp->Divide(2,2);
	 canp->cd(1);
	 mg_sigma_x->Draw("A*");
	 mg_sigma_x->Write(0,TObject::kOverwrite);
	 canp->cd(2);
	 mg_ref_sigma_x->Draw("AP");
	 mg_ref_sigma_x->Write(0,TObject::kOverwrite);
	 canp->cd(3);
	 mg_sigma_y->Draw("A*");
	 mg_sigma_y->Write(0,TObject::kOverwrite);
	 canp->cd(4);
	 mg_ref_sigma_y->Draw("AP");
	 mg_ref_sigma_y->Write(0,TObject::kOverwrite);
	 canp->Print(Form("%s/pedrel_sigma_dy5.pdf",outdir));
	 
	 delete mg_sigma_x;
	 delete mg_sigma_y;
	 delete mg_ref_sigma_x;
	 delete mg_ref_sigma_y;
	 delete canp;
	 //----------------------------------------------------------------------------------------------
	 graphp_sigma_xpos=new TGraph(41);
	 graphp_sigma_xpos->SetTitle("X_layer_dy8: mean of pedestal");
	 graphp_ref_sigma_xpos=new TGraph(41);
	 graphp_ref_sigma_xpos->SetTitle("X_layer_dy8: ratio of mean value of ped");
	 graphp_ref_sigma_xpos->SetMarkerStyle(26);
	 graphp_sigma_ypos=new TGraph(41);
	 graphp_sigma_ypos->SetTitle("Y_layer_dy8: mean of pedestal");
	 graphp_ref_sigma_ypos=new TGraph(41);
	 graphp_ref_sigma_ypos->SetTitle("Y_layer_dy8: ratio of mean value of ped");
	 graphp_ref_sigma_ypos->SetMarkerStyle(26);
	 
	 graphp_sigma_xneg=new TGraph(41);
	 graphp_sigma_xneg->SetTitle("X_layer_dy8: mean of pedestal");
	 graphp_ref_sigma_xneg=new TGraph(41);
	 graphp_ref_sigma_xneg->SetTitle("X_layer_dy8: ratio of mean value of ped");
	 graphp_ref_sigma_xneg->SetMarkerStyle(26);
	 graphp_sigma_yneg=new TGraph(41);
	 graphp_sigma_yneg->SetTitle("Y_layer_dy8: mean of pedestal");
	 graphp_ref_sigma_yneg=new TGraph(41);
	 graphp_ref_sigma_yneg->SetTitle("Y_layer_dy8: ratio of mean value of ped");
	 graphp_ref_sigma_yneg->SetMarkerStyle(26);
	 
	 mg_sigma_x = new TMultiGraph();
	 mg_sigma_x->SetName("mg_x_mean_dy8");
	 mg_sigma_x->SetTitle("X_layer_dy8: mean of pedestal");
	 mg_sigma_x->Add(graphp_sigma_xpos);
	 mg_sigma_x->Add(graphp_sigma_xneg);
	 mg_sigma_y = new TMultiGraph();
	 mg_sigma_y->SetName("mg_y_mean_dy8");
	 mg_sigma_y->SetTitle("Y_layer_dy8: mean of pedestal");
	 mg_sigma_y->Add(graphp_sigma_ypos);
	 mg_sigma_y->Add(graphp_sigma_yneg);
	 mg_ref_sigma_x = new TMultiGraph();
	 mg_ref_sigma_x->SetName("mg_x_ref_mean_dy8");
	 mg_ref_sigma_x->SetTitle("X_layer_dy8: ratio of mean value of ped");
	 mg_ref_sigma_x->Add(graphp_ref_sigma_xpos);
	 mg_ref_sigma_x->Add(graphp_ref_sigma_xneg);
	 mg_ref_sigma_y = new TMultiGraph();
	 mg_ref_sigma_y->SetName("mg_y_ref_mean_dy8");
	 mg_ref_sigma_y->SetTitle("Y_layer_dy8: ratio of mean value of ped");
	 mg_ref_sigma_y->Add(graphp_ref_sigma_ypos);
	 mg_ref_sigma_y->Add(graphp_ref_sigma_yneg);
	 
	 for(int fee_id=0;fee_id<2;fee_id++){
	   //-----------------X--------------------------------------------------
	   tree_test=(TTree*)file_in->Get(Form("%s_ped",label[fee_id].Data()));
	   tree_test->SetBranchAddress("channel",&channel);
	   tree_test->SetBranchAddress("mean",&mean);
	   tree_test->BuildIndex("channel");
	   
	   tree_ref=(TTree*)file_ref->Get(Form("%s_ped",label[fee_id].Data()));
	   tree_ref->SetBranchAddress("mean",&mean_ref);
	   tree_ref->BuildIndex("channel");
	   
	   for(int ch_id=0;ch_id<41;ch_id++){
	     tree_test->GetEntryWithIndex(id8[ch_id]+1);
	     tree_ref->GetEntryWithIndex(channel);
	     if(fee_id==0){
	       graphp_sigma_xpos->SetPoint(ch_id,ch_id+1,mean);
	     }
	     else{
	       graphp_sigma_xneg->SetPoint(ch_id,41-ch_id,-mean);
	     }
	     ratio_mean=mean/mean_ref;
	     if(fee_id==0){
	       graphp_ref_sigma_xpos->SetPoint(ch_id,ch_id+1,ratio_mean);
	     }
	     else{
	       graphp_ref_sigma_xneg->SetPoint(ch_id,41-ch_id,-ratio_mean);
	     }
	   }
	   //-----------------Y---------------------------------------------------------------
	   tree_test=(TTree*)file_in->Get(Form("%s_ped",label[fee_id+2].Data()));
	   tree_test->SetBranchAddress("channel",&channel);
	   tree_test->SetBranchAddress("mean",&mean);
	   tree_test->BuildIndex("channel");
	   
	   tree_ref=(TTree*)file_ref->Get(Form("%s_ped",label[fee_id+2].Data()));
	   tree_ref->SetBranchAddress("mean",&mean_ref);
	   tree_ref->BuildIndex("channel");
	   
	   for(int ch_id=0;ch_id<41;ch_id++){
	     tree_test->GetEntryWithIndex(id8[ch_id]+1);
	     tree_ref->GetEntryWithIndex(channel);
	     if(fee_id==0){
	       graphp_sigma_ypos->SetPoint(ch_id,41-ch_id,mean);
	     }
	     else{
	       graphp_sigma_yneg->SetPoint(ch_id,ch_id+1,-mean);
	     }
	     ratio_mean=mean/mean_ref;
	     if(fee_id==0){
	       graphp_ref_sigma_ypos->SetPoint(ch_id,41-ch_id,ratio_mean);
	     }
	     else{
	       graphp_ref_sigma_yneg->SetPoint(ch_id,ch_id+1,-ratio_mean);
	     }
	   }
	 }
	 
	 file_out->cd();
	 canp=new TCanvas("canp","Rel_ped VS positon",900,900);
	 canp->Divide(2,2);
	 canp->cd(1);
	 mg_sigma_x->Draw("A*");
	 mg_sigma_x->Write(0,TObject::kOverwrite);
	 canp->cd(2);
	 mg_ref_sigma_x->Draw("AP");
	 mg_ref_sigma_x->Write(0,TObject::kOverwrite);
	 canp->cd(3);
	 mg_sigma_y->Draw("A*");
	 mg_sigma_y->Write(0,TObject::kOverwrite);
	 canp->cd(4);
	 mg_ref_sigma_y->Draw("AP");
	 mg_ref_sigma_y->Write(0,TObject::kOverwrite);
	 canp->Print(Form("%s/pedrel_mean_dy8.pdf",outdir));
	 
	 delete mg_sigma_x;
	 delete mg_sigma_y;
	 delete mg_ref_sigma_x;
	 delete mg_ref_sigma_y;
	 delete canp;
	 //----------------------------------------------------------------
	 graphp_sigma_xpos=new TGraph(41);
	 graphp_sigma_xpos->SetTitle("X_layer_dy5: mean of pedestal");
	 graphp_ref_sigma_xpos=new TGraph(41);
	 graphp_ref_sigma_xpos->SetTitle("X_layer_dy5: ratio of mean value of ped");
	 graphp_ref_sigma_xpos->SetMarkerStyle(26);
	 graphp_sigma_ypos=new TGraph(41);
	 graphp_sigma_ypos->SetTitle("Y_layer_dy5: mean of pedestal");
	 graphp_ref_sigma_ypos=new TGraph(41);
	 graphp_ref_sigma_ypos->SetTitle("Y_layer_dy5: ratio of mean value of ped");
	 graphp_ref_sigma_ypos->SetMarkerStyle(26);
	 
	 graphp_sigma_xneg=new TGraph(41);
	 graphp_sigma_xneg->SetTitle("X_layer_dy5: mean of pedestal");
	 graphp_ref_sigma_xneg=new TGraph(41);
	 graphp_ref_sigma_xneg->SetTitle("X_layer_dy5: ratio of mean value of ped");
	 graphp_ref_sigma_xneg->SetMarkerStyle(26);
	 graphp_sigma_yneg=new TGraph(41);
	 graphp_sigma_yneg->SetTitle("Y_layer_dy5: mean of pedestal");
	 graphp_ref_sigma_yneg=new TGraph(41);
	 graphp_ref_sigma_yneg->SetTitle("Y_layer_dy5: ratio of mean value of ped");
	 graphp_ref_sigma_yneg->SetMarkerStyle(26);
	 
	 mg_sigma_x = new TMultiGraph();
	 mg_sigma_x->SetName("mg_x_mean_dy5");
	 mg_sigma_x->SetTitle("X_layer_dy5: mean of pedestal");
	 mg_sigma_x->Add(graphp_sigma_xpos);
	 mg_sigma_x->Add(graphp_sigma_xneg);
	 mg_sigma_y = new TMultiGraph();
	 mg_sigma_y->SetName("mg_y_mean_dy5");
	 mg_sigma_y->SetTitle("Y_layer_dy5: mean of pedestal");
	 mg_sigma_y->Add(graphp_sigma_ypos);
	 mg_sigma_y->Add(graphp_sigma_yneg);
	 mg_ref_sigma_x = new TMultiGraph();
	 mg_ref_sigma_x->SetName("mg_x_ref_mean_dy5");
	 mg_ref_sigma_x->SetTitle("X_layer_dy5: ratio of mean value of ped");
	 mg_ref_sigma_x->Add(graphp_ref_sigma_xpos);
	 mg_ref_sigma_x->Add(graphp_ref_sigma_xneg);
	 mg_ref_sigma_y = new TMultiGraph();
	 mg_ref_sigma_y->SetName("mg_y_ref_mean_dy5");
	 mg_ref_sigma_y->SetTitle("Y_layer_dy5: ratio of mean value of ped");
	 mg_ref_sigma_y->Add(graphp_ref_sigma_ypos);
	 mg_ref_sigma_y->Add(graphp_ref_sigma_yneg);
	 
	 for(int fee_id=0;fee_id<2;fee_id++){
	   //-----------------X--------------------------------------------------
	   tree_test=(TTree*)file_in->Get(Form("%s_ped",label[fee_id].Data()));
	   tree_test->SetBranchAddress("channel",&channel);
	   tree_test->SetBranchAddress("mean",&mean);
	   tree_test->BuildIndex("channel");
	   
	   tree_ref=(TTree*)file_ref->Get(Form("%s_ped",label[fee_id].Data()));
	   tree_ref->SetBranchAddress("mean",&mean_ref);
	   tree_ref->BuildIndex("channel");
	   
	   for(int ch_id=0;ch_id<41;ch_id++){
	     tree_test->GetEntryWithIndex(id5[ch_id]+1);
	     tree_ref->GetEntryWithIndex(channel);
	     if(fee_id==0){
	       graphp_sigma_xpos->SetPoint(ch_id,ch_id+1,mean);
	     }
	     else{
	       graphp_sigma_xneg->SetPoint(ch_id,41-ch_id,-mean);
	     }
	     ratio_mean=mean/mean_ref;
	     if(fee_id==0){
	       graphp_ref_sigma_xpos->SetPoint(ch_id,ch_id+1,ratio_mean);
	     }
	     else{
	       graphp_ref_sigma_xneg->SetPoint(ch_id,41-ch_id,-ratio_mean);
	     }
	   }
	   //-----------------Y---------------------------------------------------------------
	   tree_test=(TTree*)file_in->Get(Form("%s_ped",label[fee_id+2].Data()));
	   tree_test->SetBranchAddress("channel",&channel);
	   tree_test->SetBranchAddress("mean",&mean);
	   tree_test->BuildIndex("channel");
	   
	   tree_ref=(TTree*)file_ref->Get(Form("%s_ped",label[fee_id+2].Data()));
	   tree_ref->SetBranchAddress("mean",&mean_ref);
	   tree_ref->BuildIndex("channel");
	   
	   for(int ch_id=0;ch_id<41;ch_id++){
	     tree_test->GetEntryWithIndex(id5[ch_id]+1);
	     tree_ref->GetEntryWithIndex(channel);
	     if(fee_id==0){
	       graphp_sigma_ypos->SetPoint(ch_id,41-ch_id,mean);
	     }
	     else{
	       graphp_sigma_yneg->SetPoint(ch_id,ch_id+1,-mean);
	     }
	     ratio_mean=mean/mean_ref;
	     if(fee_id==0){
	       graphp_ref_sigma_ypos->SetPoint(ch_id,41-ch_id,ratio_mean);
	     }
	     else{
	       graphp_ref_sigma_yneg->SetPoint(ch_id,ch_id+1,-ratio_mean);
	     }
	   }
	 }
	 
	 file_out->cd();
	 canp=new TCanvas("canp","Rel_ped VS positon",900,900);
	 canp->Divide(2,2);
	 canp->cd(1);
	 mg_sigma_x->Draw("A*");
	 mg_sigma_x->Write(0,TObject::kOverwrite);
	 canp->cd(2);
	 mg_ref_sigma_x->Draw("AP");
	 mg_ref_sigma_x->Write(0,TObject::kOverwrite);
	 canp->cd(3);
	 mg_sigma_y->Draw("A*");
	 mg_sigma_y->Write(0,TObject::kOverwrite);
	 canp->cd(4);
	 mg_ref_sigma_y->Draw("AP");
	 mg_ref_sigma_y->Write(0,TObject::kOverwrite);
	 canp->Print(Form("%s/pedrel_mean_dy5.pdf",outdir));
	 
	 delete mg_sigma_x;
	 delete mg_sigma_y;
	 delete mg_ref_sigma_x;
	 delete mg_ref_sigma_y;
	 delete canp;
	 //////////////////////////////////////////
	 
	 delete file_in;
	 delete file_ref;
	 delete file_out;
	 
	 return 0;
}

int draw_mip(const char* mipfile,const char* pedfile,const char* outDir,const char* outName)
{
  gStyle->SetOptStat(0);
  
  int id8[41]={0,1,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,22,46,47,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64,65,66};
  int id5[41]={23,24,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,45,68,69,71,72,73,74,75,76,77,78,79,80,81,82,83,84,85,86,87,88};
  TString label[4]={"xpos","xneg","ypos","yneg"};
  
  Float_t xped_mean[41],xped_sigma[41],yped_mean[41],yped_sigma[41];
  Float_t xmean_buffer,xsigma_buffer,ymean_buffer,ysigma_buffer;
  Float_t xlimit[41],ylimit[41];
  Int_t xmip[90],ymip[90];
  Int_t xmip_tmp,ymip_tmp;
  Float_t x,y;
  Int_t strip_id;
  
  TFile* fped=new TFile(pedfile);
  TTree* txped=0;
  TTree* typed=0;
  
  TFile* fmip=new TFile(mipfile);
  TTree* tmip=(TTree*)fmip->Get("PSD");
  
  
  TH2F *hx,*hy;
  hx=new TH2F("hx_mip","",41,0.5,41.5,1600,0,8000);
  hy=new TH2F("hy_mip","",41,0.5,41.5,1600,0,8000);
  
  Int_t nentries=tmip->GetEntries();
  for(Int_t fee_id=0;fee_id<2;fee_id++){
    txped=(TTree*)fped->Get(Form("%s_ped",label[fee_id].Data()));
    txped->BuildIndex("channel");
    txped->SetBranchAddress("mean",&xmean_buffer);
    txped->SetBranchAddress("sigma",&xsigma_buffer);
    
    typed=(TTree*)fped->Get(Form("%s_ped",label[fee_id+2].Data()));
    typed->BuildIndex("channel");
    typed->SetBranchAddress("mean",&ymean_buffer);
    typed->SetBranchAddress("sigma",&ysigma_buffer);
    for(Int_t i=0;i<41;i++){
      if(fee_id==0){
	txped->GetEntryWithIndex(id8[i]+1);
	typed->GetEntryWithIndex(id8[40-i]+1);
      }
      else{
	txped->GetEntryWithIndex(id8[40-i]+1);
	typed->GetEntryWithIndex(id8[i]+1);
      }
      
      xped_mean[i]=xmean_buffer;
      xped_sigma[i]=xsigma_buffer;
      xlimit[i]=2*xped_sigma[i];
      yped_mean[i]=ymean_buffer;
      yped_sigma[i]=ysigma_buffer;
      ylimit[i]=2*yped_sigma[i];
    }
    
    tmip->ResetBranchAddresses();
    tmip->SetBranchAddress(label[fee_id].Data(),xmip);
    tmip->SetBranchAddress(label[fee_id+2].Data(),ymip);
    for(int event_id=0;event_id<nentries;event_id++){
      tmip->GetEntry(event_id);
      
      for(int ch_id=0;ch_id<41;ch_id++){
	if(fee_id==0){
	  xmip_tmp=xmip[id8[ch_id]];
	  ymip_tmp=ymip[id8[40-ch_id]];
	}
	else{
	  xmip_tmp=xmip[id8[40-ch_id]];
	  ymip_tmp=ymip[id8[ch_id]];
	}
	if(xmip_tmp> (xped_mean[ch_id]+xlimit[ch_id])){
	  if(fee_id==0){
	    x=xmip_tmp-xped_mean[ch_id];
	    strip_id=ch_id+1;
	    hx->Fill(strip_id,x);
	  }
	  else{
	    x= 8000-(xmip_tmp-xped_mean[ch_id]);
	    strip_id=ch_id+1;
	    hx->Fill(strip_id,x);
	  }
	}
	
	if(ymip_tmp > yped_mean[ch_id]+ylimit[ch_id]){
	  if(fee_id==0){
	    y=ymip_tmp-yped_mean[ch_id];
	    strip_id=ch_id+1;
	    hy->Fill(strip_id,y);
	  }
	  else{
	    y=8000-(ymip_tmp-yped_mean[ch_id]);
	    strip_id=ch_id+1;
	    hy->Fill(strip_id,y);
	  }
	}
      }
    }
    
    
  }
  TCanvas *can=new TCanvas("can_mip","can_mip",800,500);
  can->Divide(2,1);
  can->cd(1);
  gPad->SetLogz();
  hx->Draw("colz");
  gPad->Update();
  Double_t xmin=gPad->GetUxmin();
  Double_t xmax=gPad->GetUxmax();
  Double_t ymin=gPad->GetUymin();
  Double_t ymax=gPad->GetUymax();
  TGaxis *ax1=new TGaxis(xmin,ymax,xmax,ymax,0.5,41.5,510,"-L");
  ax1->SetLabelSize(hx->GetXaxis()->GetLabelSize());
  ax1->SetLabelFont(hx->GetXaxis()->GetLabelFont());
  ax1->Draw();
  hx->GetXaxis()->SetTitle("+X");
  hx->GetXaxis()->CenterTitle();
  //hx->GetXaxis()->SetTitleColor(kRed);
  //hx->GetXaxis()->SetLineColor(kRed);
  
  ax1->SetTitle("-X");
  ax1->CenterTitle();
  ax1->SetTitleFont(hx->GetXaxis()->GetTitleFont());
  TGaxis *ax2=new TGaxis(xmax,ymax,xmax-0.01,ymin,0,8000,510,"-");
  ax2->SetLabelSize(hx->GetYaxis()->GetLabelSize());
  ax2->SetLabelFont(hx->GetYaxis()->GetLabelFont());
  ax2->Draw();
  TPaletteAxis *paletx=(TPaletteAxis*)hx->GetListOfFunctions()->FindObject("palette");
  TGaxis* paletx_axis=paletx->GetAxis();
  paletx_axis->SetLabelSize(0);
  paletx_axis->SetTickSize(0);
  
  can->cd(2);
  gPad->SetLogz();
  hy->GetXaxis()->SetTitle("+Y");
  hy->GetXaxis()->CenterTitle();
  hy->Draw("colz");
  gPad->Update();
  TGaxis *ax3=new TGaxis(xmin,ymax,xmax,ymax,0.5,41.5,510,"-L");
  ax3->SetLabelSize(hy->GetXaxis()->GetLabelSize());
  ax3->SetLabelFont(hy->GetXaxis()->GetLabelFont());
  ax3->SetTitle("-Y");
  ax3->CenterTitle();
  ax3->SetTitleFont(hy->GetXaxis()->GetTitleFont());
  ax3->Draw();
  ax2->Draw();
  
  TPaletteAxis *palety=(TPaletteAxis*)hy->GetListOfFunctions()->FindObject("palette");
  TGaxis* palety_axis=palety->GetAxis();
  
  palety_axis->SetLabelSize(0);
  palety_axis->SetTickSize(0);
  
  can->Print(Form("%s/mipVSstrip.pdf",outDir));
  TFile *file_out=new TFile(Form("%s/%s",outDir,outName),"update");
  file_out->cd();
  can->Write(0,TObject::kOverwrite);
  hx->Write(0,TObject::kOverwrite);
  hy->Write(0,TObject::kOverwrite);
  delete file_out;
  
  delete can;
  delete fped;
  delete fmip;
  return 0;
  }
  
  int draw_mapping(const char* pardir,const char* filename,const char* outDir,unsigned int max)
  {
    int id8_pos[41]={0,1,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,22,46,47,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64,65,66};
    int id8_neg[41]={66,65,64,63,62,61,60,59,58,57,56,55,54,53,52,51,50,49,47,46,22,20,19,18,17,16,15,14,13,12,11,10,9,8,7,6,5,4,3,1,0};
    int id5_pos[41]={23,24,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,45,68,69,71,72,73,74,75,76,77,78,79,80,81,82,83,84,85,86,87,88};
    int id5_neg[41]={88,87,86,85,84,83,82,81,80,79,78,77,76,75,74,73,72,71,69,68,45,43,42,41,40,39,38,37,36,35,34,33,32,31,30,29,28,27,26,24,23};
    int id_empty[8]={2,21,25,44,48,67,70,89};
    
    TFile* file=new TFile(Form("%s/%s",pardir,filename),"update");
    TTree *tree_in=(TTree*)file->Get("PSD");
    
    int nevents=tree_in->GetEntries();
    int maxentry;
    if(nevents < max){
      maxentry = nevents;
    }
    else{
      maxentry = max;
    }
    //printf("event_no = %d\n",tree_in->GetEntries());
    
    TCanvas *canvans=new TCanvas("canvas","canvas",600,500);
    //TH2F *h8=new TH2F("h8","h8",500,0,5000,500,0,5000);
    TH2F *h5_xpos=new TH2F("h5_xpos","h5_xpos",200,0,1000,3200,0,16000);
    h5_xpos->GetXaxis()->SetTitle("Dy5");h5_xpos->GetYaxis()->SetTitle("Dy8");
    TH2F *h5_xneg=new TH2F("h5_xneg","h5_xneg",200,0,1000,3200,0,16000);
    h5_xneg->GetXaxis()->SetTitle("Dy5");h5_xneg->GetYaxis()->SetTitle("Dy8");
    TH2F *h5_ypos=new TH2F("h5_ypos","h5_ypos",200,0,1000,3200,0,16000);
    h5_ypos->GetXaxis()->SetTitle("Dy5");h5_ypos->GetYaxis()->SetTitle("Dy8");
    TH2F *h5_yneg=new TH2F("h5_yneg","h5_yneg",200,0,1000,3200,0,16000);
    h5_yneg->GetXaxis()->SetTitle("Dy5");h5_yneg->GetYaxis()->SetTitle("Dy8");
    /*
     *  canvans->Print(Form("%s/dy8_match.pdf[",pardir));
     * 
     *  for(int i=0;i<41;i++){
     *      h8->SetTitle(Form("X%d",i+1));
     *      tree_in->Draw(Form("xpos[%d]:xneg[%d] >> h8",id8_pos[i],id8_neg[40-i]),"","",maxentry);
     *      canvans->Print(Form("%s/dy8_match.pdf",pardir));
     *      h8->SetTitle(Form("Y%d",41-i));
     *      tree_in->Draw(Form("ypos[%d]:yneg[%d] >> h8",id8_pos[i],id8_neg[40-i]),"","",maxentry);
     *      canvans->Print(Form("%s/dy8_match.pdf",pardir));
     }
     canvans->Print(Form("%s/dy8_match.pdf]",pardir));
     */
    canvans->Print(Form("%s/dy5_match.pdf[",outDir));
    for(int i=0;i<41;i++){
      h5_xpos->SetName(Form("hxpos_dy58_%d",i+1));
      h5_xpos->SetTitle(Form("X%d_pos",i+1));
      tree_in->Draw(Form("xpos[%d]:xpos[%d] >> hxpos_dy58_%d",id8_pos[i],id5_pos[i],i+1),"","",maxentry);
      canvans->Print(Form("%s/dy5_match.pdf",outDir));
      h5_xpos->Write(0,TObject::kOverwrite);
      
      h5_xneg->SetName(Form("hxneg_dy58_%d",i+1));
      h5_xneg->SetTitle(Form("X%d_neg",i+1));
      tree_in->Draw(Form("xneg[%d]:xneg[%d] >> hxneg_dy58_%d",id8_pos[40-i],id5_pos[40-i],i+1),"","",maxentry);
      canvans->Print(Form("%s/dy5_match.pdf",outDir));
      h5_xneg->Write(0,TObject::kOverwrite);
      
      h5_ypos->SetName(Form("hypos_dy58_%d",i+1));
      h5_ypos->SetTitle(Form("Y%d_pos",i+1));
      tree_in->Draw(Form("ypos[%d]:ypos[%d] >> hypos_dy58_%d",id8_pos[40-i],id5_pos[40-i],i+1),"","",maxentry);
      canvans->Print(Form("%s/dy5_match.pdf",outDir));
      h5_ypos->Write(0,TObject::kOverwrite);
      
      h5_yneg->SetName(Form("hyneg_dy58_%d",i+1));
      h5_yneg->SetTitle(Form("Y%d_neg",i+1));
      tree_in->Draw(Form("yneg[%d]:yneg[%d] >> hyneg_dy58_%d",id8_pos[i],id5_pos[i],i+1),"","",maxentry);
      canvans->Print(Form("%s/dy5_match.pdf",outDir));
      h5_yneg->Write(0,TObject::kOverwrite);
    }
    canvans->Print(Form("%s/dy5_match.pdf]",outDir));
    
    
    delete canvans;
    delete file;
    
    return 0;
  }
  
  int fit_dy58(const char* infile,const char* pedfile,const char* outdir,const char* outfile,int pedcut,float range)
  {
    //--- pedestal ----------------
    int id8[41]={0,1,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,22,46,47,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64,65,66};
    int id5[41]={23,24,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,45,68,69,71,72,73,74,75,76,77,78,79,80,81,82,83,84,85,86,87,88};
    
    int channel;
    float mean,sigma;
    float xpedmean_dy8_pos[41],xpedmean_dy5_pos[41],xpedmean_dy8_neg[41],xpedmean_dy5_neg[41];
    float ypedmean_dy8_pos[41],ypedmean_dy5_pos[41],ypedmean_dy8_neg[41],ypedmean_dy5_neg[41];
    float xpedsigma_dy8_pos[41],xpedsigma_dy5_pos[41],xpedsigma_dy8_neg[41],xpedsigma_dy5_neg[41];
    float ypedsigma_dy8_pos[41],ypedsigma_dy5_pos[41],ypedsigma_dy8_neg[41],ypedsigma_dy5_neg[41];
    TFile *f_ped=new TFile(pedfile);
    
    TTree *tree_ped=(TTree*)f_ped->Get("xpos_ped");
    tree_ped->SetBranchAddress("channel",&channel);
    tree_ped->SetBranchAddress("mean",&mean);
    tree_ped->SetBranchAddress("sigma",&sigma);
    tree_ped->BuildIndex("channel");
    for(int i=0;i<41;i++){
      tree_ped->GetEntryWithIndex(id8[i]+1);
      xpedmean_dy8_pos[i]=mean;
      xpedsigma_dy8_pos[i]=sigma;
      
      tree_ped->GetEntryWithIndex(id5[i]+1);
      xpedmean_dy5_pos[i]=mean;
      xpedsigma_dy5_pos[i]=sigma;
    }
    
    tree_ped=(TTree*)f_ped->Get("xneg_ped");
    tree_ped->SetBranchAddress("channel",&channel);
    tree_ped->SetBranchAddress("mean",&mean);
    tree_ped->SetBranchAddress("sigma",&sigma);
    tree_ped->BuildIndex("channel");
    for(int i=0;i<41;i++){
      tree_ped->GetEntryWithIndex(id8[40-i]+1);
      xpedmean_dy8_neg[i]=mean;
      xpedsigma_dy8_neg[i]=sigma;
      
      tree_ped->GetEntryWithIndex(id5[40-i]+1);
      xpedmean_dy5_neg[i]=mean;
      xpedsigma_dy5_neg[i]=sigma;
    }
    
    tree_ped=(TTree*)f_ped->Get("ypos_ped");
    tree_ped->SetBranchAddress("channel",&channel);
    tree_ped->SetBranchAddress("mean",&mean);
    tree_ped->SetBranchAddress("sigma",&sigma);
    tree_ped->BuildIndex("channel");
    for(int i=0;i<41;i++){
      tree_ped->GetEntryWithIndex(id8[40-i]+1);
      ypedmean_dy8_pos[i]=mean;
      ypedsigma_dy8_pos[i]=sigma;
      
      tree_ped->GetEntryWithIndex(id5[40-i]+1);
      ypedmean_dy5_pos[i]=mean;
      ypedsigma_dy5_pos[i]=sigma;
    }
    
    tree_ped=(TTree*)f_ped->Get("yneg_ped");
    tree_ped->SetBranchAddress("channel",&channel);
    tree_ped->SetBranchAddress("mean",&mean);
    tree_ped->SetBranchAddress("sigma",&sigma);
    tree_ped->BuildIndex("channel");
    for(int i=0;i<41;i++){
      tree_ped->GetEntryWithIndex(id8[i]+1);
      ypedmean_dy8_neg[i]=mean;
      ypedsigma_dy8_neg[i]=sigma;
      
      tree_ped->GetEntryWithIndex(id5[i]+1);
      ypedmean_dy5_neg[i]=mean;
      ypedsigma_dy5_neg[i]=sigma;
    }
    delete f_ped;
    
    //----get profile of dy58,and fit---------------------------
    Double_t fitrange[2];
    TFile *f_in=new TFile(infile,"update");
    TH1F* hdy58_dist=new TH1F("hdy58_dist","Dy58 Ratio Distribution",200,40.0,60.0);
    TH2F* hdy58;
    TProfile* hdy58_pfx;
    TF1* lffit;
    Float_t dy58_ratio;
    
    FILE* fp=fopen(Form("%s/%s.txt",outdir,outfile),"w");
    fprintf(fp,"dy58_ratio:\n");
    fprintf(fp,"index\txpos\txneg\typos\tyneg\n");
    
    TCanvas* can=new TCanvas("can_dy58_fit","can_dy58_fit",900,900);
    can->Divide(2,2);
    can->Print(Form("%s/%s.pdf[",outdir,outfile));
    
    for(int i=0;i<41;i++){
      fprintf(fp,"%d\t",i+1);
      
      can->cd(1);
      hdy58=(TH2F*)f_in->Get(Form("hxpos_dy58_%d",i+1));
      hdy58_pfx=hdy58->ProfileX();
      fitrange[0]=xpedmean_dy5_pos[i]+pedcut*xpedsigma_dy5_pos[i];
      fitrange[1]=xpedmean_dy5_pos[i]+pedcut*xpedsigma_dy5_pos[i]+range;
      lffit=linear_fit(hdy58_pfx,fitrange);
      dy58_ratio=lffit->GetParameter(1);
      fprintf(fp,"%.3f\t",dy58_ratio);
      hdy58_dist->Fill(dy58_ratio);
      lffit->SetLineColor(kRed);
      hdy58->Draw();
      lffit->Draw("same");
      
      can->cd(2);
      hdy58=(TH2F*)f_in->Get(Form("hxneg_dy58_%d",i+1));
      hdy58_pfx=hdy58->ProfileX();
      fitrange[0]=xpedmean_dy5_neg[i]+pedcut*xpedsigma_dy5_neg[i];
      fitrange[1]=xpedmean_dy5_neg[i]+pedcut*xpedsigma_dy5_neg[i]+range;
      lffit=linear_fit(hdy58_pfx,fitrange);
      dy58_ratio=lffit->GetParameter(1);
      fprintf(fp,"%.3f\t",dy58_ratio);
      hdy58_dist->Fill(dy58_ratio);
      lffit->SetLineColor(kRed);
      hdy58->Draw();
      lffit->Draw("same");
      
      can->cd(3);
      hdy58=(TH2F*)f_in->Get(Form("hypos_dy58_%d",i+1));
      hdy58_pfx=hdy58->ProfileX();
      fitrange[0]=ypedmean_dy5_pos[i]+pedcut*ypedsigma_dy5_pos[i];
      fitrange[1]=ypedmean_dy5_pos[i]+pedcut*ypedsigma_dy5_pos[i]+range;
      lffit=linear_fit(hdy58_pfx,fitrange);
      dy58_ratio=lffit->GetParameter(1);
      fprintf(fp,"%.3f\t",dy58_ratio);
      hdy58_dist->Fill(dy58_ratio);
      lffit->SetLineColor(kRed);
      hdy58->Draw();
      lffit->Draw("same");
      
      can->cd(4);
      hdy58=(TH2F*)f_in->Get(Form("hyneg_dy58_%d",i+1));
      hdy58_pfx=hdy58->ProfileX();
      fitrange[0]=ypedmean_dy5_neg[i]+pedcut*ypedsigma_dy5_neg[i];
      fitrange[1]=ypedmean_dy5_neg[i]+pedcut*ypedsigma_dy5_neg[i]+range;
      lffit=linear_fit(hdy58_pfx,fitrange);
      dy58_ratio=lffit->GetParameter(1);
      fprintf(fp,"%.3f\n",dy58_ratio);
      hdy58_dist->Fill(dy58_ratio);
      lffit->SetLineColor(kRed);
      hdy58->Draw();
      lffit->Draw("same");
      
      can->Print(Form("%s/%s.pdf",outdir,outfile));
    }
    can->Print(Form("%s/%s.pdf]",outdir,outfile));
    
    hdy58_dist->Write(0,TObject::kOverwrite);
    fclose(fp);
    delete f_in;
    delete can;
    
    return 0;
  }
  
}

namespace Utility
{
  /*return value:
    1) 1: complete event GroupHead and GroupTrailer found
    2) 2: no GroupTrailer
    3) 3: unmatched eventid in Gheader and Gtrailer
    3) -1:file stream error
    4) -2:not GroupHeader
    5) -3:some other data in packet
    5) 0:EOF
  */
  char _GetNextEvent_ungrouped(FILE*fp,unsigned int** old_buffer,int* buffer_len,int* event_len,int* bunch_id,int* event_id,int* word_count)
  {
    unsigned int* new_buffer,*buffer;
    buffer=*old_buffer;
    unsigned int gheader,gtrailer;
    int type_id,eventid_trailer;
    *event_len=0;
    *word_count=-1;
    //GroupHeader
    if(fread(&gheader,4,1,fp) <1){
      if(feof(fp))
	return 0;
      else
	return -1;
    }
    else{
      type_id=gheader>>28;
      if(type_id==0x0){
	*bunch_id=gheader&0xFFF;
	*event_id=(gheader>>12)&0xFFF;
      }
      else{
	return -2;
      }
    }
    //
    while(type_id!=0x1){
      //Read next word
      if(fread(buffer+(*event_len),4,1,fp) <1){
	if(feof(fp))
	  return 0;
	else
	  return -1;
      }
      else{
	type_id=buffer[*event_len]>>28;
	if(type_id==0x4 || type_id==0x5 || type_id==0x6){
	  (*event_len)++;
	}
	else if(type_id==0x1){
	  *word_count=buffer[*event_len]&0xFFF;
	  eventid_trailer=(buffer[*event_len]>>12)&0xFFF;
	  continue;
	}
	else if(type_id==0x0){
	  fseek(fp,-4,SEEK_CUR);
	  break;
	}
	else{
	  return -3;
	}
      }
      //Expand Buffer size in case of large events
      if((*event_len)==(*buffer_len)){
	new_buffer=(unsigned int*)malloc(((*buffer_len)+1024)*4);
	memcpy(new_buffer,buffer,(*buffer_len)*4);
	(*buffer_len)+=1024;
	free(buffer);
	buffer=new_buffer;
	(*old_buffer)=new_buffer;
	printf("buffer expanded(new buffer_len:%d,event_len:%d)\n",*buffer_len,*event_len);
      }
    }
    //
    if(type_id==0x0){
      *word_count=-1;
      return 2;
    }
    else if((*event_id)!=eventid_trailer){
      return 3;
    }
    else{
      return 1;
    }
  }
  //
  TTree* convert_mwdc_ungrouped(const char* infile,TDirectory* dir,const char* name,const char* title)
  {
    UInt_t hptdc_index[128]={0,1,63,62,61,2,3,60,59,4,58,5,57,6,56,7,55,8,9,54,53,52,51,11,10,12,50,13,49,48,14,15,
                          16,17,47,46,45,18,19,44,43,20,42,21,41,22,40,23,39,24,25,38,37,36,35,27,26,28,34,29,33,32,30,31,
                          64,65,127,126,125,66,67,124,123,68,122,69,121,70,120,71,119,72,73,118,117,116,115,75,74,76,114,77,113,112,78,79,
                          80,81,111,110,109,82,83,108,107,84,106,85,105,86,104,87,103,88,89,102,101,100,99,91,90,92,98,93,97,96,94,95};
    //
    unsigned int* buffer;
    int buffer_len=1024;
    //IMPORTANT: buffer should be large enough to hold an event,otherwise fread error may occur.
    buffer=(unsigned int*)malloc(sizeof(unsigned int)*buffer_len);
    //
    FILE* file_in=fopen(infile,"rb");
    if(!file_in){
      perror("fopen");
      fprintf(stderr,"fopen() failed in file %s at line # %d\n", __FILE__,__LINE__);
      exit(EXIT_FAILURE);
    }
    //
    int event_len,word_count,event_id=0,bunch_id=0;
    char event_flag=1;//0 eof; 1 event complete;2 event incomplete
    int eventid_pre=0;
    unsigned int packet_num=0;
    unsigned int total_words=0;
    ChannelMap leading_raw;
    ChannelMap trailing_raw;
    UInt_t tdc_index,channel_index,tdc_value;
    UInt_t global_channel;
    
    TTree* tree_out=new TTree(name,title);
    tree_out->SetDirectory(dir);
    tree_out->Branch("event_flag",&event_flag,"event_flag/B");
    tree_out->Branch("bunch_id",&bunch_id,"bunch_id/I");
    tree_out->Branch("event_id",&event_id,"event_id/I");
    tree_out->Branch("leading_raw",&leading_raw);
    tree_out->Branch("trailing_raw",&trailing_raw);
    //
    int type_id;
    int i;
    while(event_flag>0){
      event_flag=_GetNextEvent_ungrouped(file_in,&buffer,&buffer_len,&event_len,&bunch_id,&event_id,&word_count);
      if(event_flag>0){
	packet_num++;
	if(packet_num%20000==0){
	  printf("%d packets converted\n",packet_num);
	}
	//
	if(packet_num!=1){
	  if(eventid_pre==0xFFF){
	    eventid_pre=0;
	  }
	  else{
	    eventid_pre++;
	  }
	}
	if(eventid_pre != event_id){
	  if(packet_num==1)
	    printf("(packet_%u)init event_id is not )(cur=%d)\n",packet_num,event_id);
	  else
	    printf("(packet_%u)incontinuous event_id(pre=%d,cur=%d)\n",packet_num,eventid_pre-1,event_id);
	  eventid_pre=event_id;
	}
	
	//
	leading_raw.clear();
	trailing_raw.clear();
	//
	if(event_len>=1024){
	  printf("(packet_%u,total_words %u already read)large packet,event_len=%d\n",packet_num,total_words,event_len);
	}
	for(i=0;i<event_len;i++){
	  type_id=(buffer[i]>>28);
	  switch(type_id){
	    case 0x4:
	      tdc_index=(buffer[i]>>24)&0xF;
	      channel_index=(buffer[i]>>19)&0x1F;
	      tdc_value=buffer[i]&0x7FFFF;
	      global_channel=hptdc_index[tdc_index*32+channel_index];
	      leading_raw[global_channel].push_back(tdc_value);
	      break;
	    case 0x5:
	      tdc_index=(buffer[i]>>24)&0xF;
	      channel_index=(buffer[i]>>19)&0x1F;
	      tdc_value=buffer[i]&0x7FFFF;
	      global_channel=hptdc_index[tdc_index*32+channel_index];
	      trailing_raw[global_channel].push_back(tdc_value);
	      break;
	    case 0x6:
	      tdc_index=(buffer[i]>>24)&0xF;
	      printf("(packet_%u)data error_flag: tdc%d(0x%x)\n",packet_num,tdc_index,buffer[i]&0x7FFF);
	      break;
	    default:
	      printf("(packet_%u)unexpected type_id(0x%x) in packet\n",packet_num,type_id);
	  }	
	}
	switch (event_flag) {
	  case 2:
	    total_words+=(event_len+1);
	    printf("(packet_%u)no GroupTrailer,eventlen=%d\n",packet_num,event_len+1);
	    break;
	  case 3:
	    total_words+=(event_len+2);
	    printf("(packet_%u)unmatched eventid in GH and GT \n",packet_num);
	    break;
	  default:
	    total_words+=(event_len+2);
	    break;
	}
	//
	tree_out->Fill();
      }
    }
    
    switch(event_flag){
      case -1:
	perror("fread()");
	break;
      case -2:
	printf("(packet_%u)the first word is not group header\n",packet_num++);
	break;
      case -3:
	printf("(packet_%u)unexpected type_id in packet\n",packet_num++);
	break;
      default:
	printf("\n%s EOF reached,total packet_num: %d\n",infile,packet_num);
	break;
    }
    //
    free(buffer);
    fclose(file_in);
    //
    return tree_out;
  }
  
  TTree* convert_tof_ungrouped(const char* infile,TDirectory* dir,const char* name,const char* title)
  {
    UInt_t time_index[16]={8,9,10,11,12,13,14,15,
		       0,1,2,3,4,5,6,7};
    UInt_t tot_index[32]={0,1,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,2,0xFFFFFFFF,3
		    ,0xFFFFFFFF,0xFFFFFFFF,4,5,6,8,7,0xFFFFFFFF,9,10
		    ,11,0xFFFFFFFF,12,13,14,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF
		    ,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,15,0xFFFFFFFF};
    //
    unsigned int* buffer;
    int buffer_len=1024;
    //IMPORTANT: buffer should be large enough to hold an event,otherwise fread error may occur.
    buffer=(unsigned int*)malloc(sizeof(unsigned int)*buffer_len);
    //
    FILE* file_in=fopen(infile,"rb");
    if(!file_in){
      perror("fopen");
      fprintf(stderr,"fopen() failed in file %s at line # %d\n", __FILE__,__LINE__);
      exit(EXIT_FAILURE);
    }
    //
    int event_len,word_count,event_id=0,bunch_id=0;
    char event_flag=1;//0 eof; 1 event complete;2 event incomplete;3 eveid incontinuous
    int eventid_pre=0;
    unsigned int packet_num=0;
    ChannelMap time_leading_raw;
    ChannelMap time_trailing_raw;
    ChannelMap tot_leading_raw;
    ChannelMap tot_trailing_raw;
    UInt_t tdc_index,channel_index,tdc_value;
    UInt_t global_channel;
    
    TTree* tree_out=new TTree(name,title);
    tree_out->SetDirectory(dir);
    tree_out->Branch("event_flag",&event_flag,"event_flag/B");
    tree_out->Branch("bunch_id",&bunch_id,"bunch_id/I");
    tree_out->Branch("event_id",&event_id,"event_id/I");
    tree_out->Branch("time_leading_raw",&time_leading_raw);
    tree_out->Branch("time_trailing_raw",&time_trailing_raw);
    tree_out->Branch("tot_leading_raw",&tot_leading_raw);
    tree_out->Branch("tot_trailing_raw",&tot_trailing_raw);
    //
    int type_id;
    int i;
    while(event_flag>0){
      event_flag=_GetNextEvent_ungrouped(file_in,&buffer,&buffer_len,&event_len,&bunch_id,&event_id,&word_count);
      if(event_flag>0){
	packet_num++;
	if(packet_num%20000==0){
	  printf("%d packets converted\n",packet_num);
	}
	//
	if(packet_num!=1){
	  if(eventid_pre==0xFFF){
	    eventid_pre=0;
	  }
	  else{
	    eventid_pre++;
	  }
	}
	if(eventid_pre != event_id){
	  printf("(packet_%u)incontinuous event_id(pre=%d,cur=%d)\n",packet_num,eventid_pre-1,event_id);
	  eventid_pre=event_id;
	}
	
	//
	time_leading_raw.clear();
	time_trailing_raw.clear();
	tot_leading_raw.clear();
	tot_trailing_raw.clear();
	//
	for(i=0;i<event_len;i++){
	  type_id=buffer[i]>>28;
	  switch(type_id){
	    case 0x4:
	      tdc_index=(buffer[i]>>24)&0xF;
	      if(tdc_index == 2){
		channel_index=(buffer[i]>>19)&0x1F;
		tdc_value=buffer[i]&0x7FFFF;
		global_channel=tot_index[channel_index];
		tot_leading_raw[global_channel].push_back(tdc_value);
	      }
	      else if((tdc_index == 0) || (tdc_index == 1)){
		channel_index=(buffer[i]>>21)&0x7;
		tdc_value=((buffer[i]&0x7FFFF)<<2) + ((buffer[i]>>19)&0x3);
		global_channel=time_index[tdc_index*8+channel_index];
		time_leading_raw[global_channel].push_back(tdc_value);
	      }
	      else{
		printf("(packet_%u)unexpected tdc_index in packet\n",packet_num);
	      }
	      break;
	    case 0x5:
	      tdc_index=(buffer[i]>>24)&0xF;
	      if(tdc_index == 2){
		channel_index=(buffer[i]>>19)&0x1F;
		tdc_value=buffer[i]&0x7FFFF;
		global_channel=tot_index[channel_index];
		tot_trailing_raw[global_channel].push_back(tdc_value);
	      }
	      else if((tdc_index == 0) || (tdc_index == 1)){
		channel_index=(buffer[i]>>21)&0x7;
		tdc_value=((buffer[i]&0x7FFFF)<<2) + ((buffer[i]>>19)&0x3);
		global_channel=time_index[tdc_index*8+channel_index];
		time_trailing_raw[global_channel].push_back(tdc_value);
	      }
	      else{
		printf("(packet_%u)unexpected tdc_index in packet\n",packet_num);
	      }
	      break;
	    case 0x6:
	      tdc_index=(buffer[i]>>24)&0xF;
	      printf("(packet_%u)data error_flag: tdc%d(0x%x)\n",packet_num,tdc_index,buffer[i]&0x7FFF);
	      if(i!=(event_len-1)){
		printf("someother data in packet\n");
	      }
	      break;
	    default:
	      printf("(packet_%u)unexpected type_id in packet\n",packet_num);
	  }	
	}
	switch (event_flag) {
	  case 2:
	    printf("(packet_%u)no GroupTrailer,eventlen=%d\n",packet_num,event_len+1);
	    break;
	  case 3:
	    printf("(packet_%u)unmatched eventid in GH and GT \n",packet_num);
	    break;
	  default:
	    break;
	}
	//
	tree_out->Fill();
      }
    }
    
    switch(event_flag){
      case -1:
	perror("fread()");
	break;
      case -2:
	printf("(packet_%u)the first word is not group header\n",packet_num++);
	break;
      case -3:
	printf("(packet_%u)unexpected type_id in packet\n",packet_num++);
	break;
      default:
	printf("\n%s EOF reached,total packet_num: %d\n",infile,packet_num);
	break;
    }
    //
    free(buffer);
    fclose(file_in);
    //
    return tree_out;
  }
  
  int convert_hptdc_ungrouped(const char* datadir,const char* outfile,const char* prefix,const char* configdir,const char* suffix)
  {
    TString file_config;
    if(!configdir)
      file_config=TString(datadir)+"/crate.json";
    else
      file_config=TString(configdir)+"/crate.json";
    CrateInfo* info=read_config(file_config.Data(),prefix,suffix);
    info->Print();
    
    TString file_data=TString(datadir)+"/"+outfile;  
    TTree* tree_out;
    TFile* file_out=new TFile(file_data,"recreate");
    TDirectory* raw_dir=file_out->mkdir("raw");
    
    int boardnum=info->GetBoardNum();
    for(int i=0;i<boardnum;i++){
      file_data=TString(datadir)+"/"+info->GetFilename(i);
      if(info->GetBoardtype(i) == "mwdc"){
	tree_out=convert_mwdc_ungrouped(file_data.Data(),raw_dir,info->GetBoardname(i).Data(),info->GetBoardtitle(i).Data());
      }
      else if(info->GetBoardtype(i) == "tof"){
	tree_out=convert_tof_ungrouped(file_data.Data(),raw_dir,info->GetBoardname(i).Data(),info->GetBoardtitle(i).Data());
      }
      else{
	printf("error: unrecognized board type %s\n",info->GetBoardtype(i).Data());
	exit(1);
      }
      
      raw_dir->cd();
      tree_out->Write();
      delete tree_out;
    }
    delete file_out;
    delete info;
    return 0;

  }
  
  int print_info_ungrouped(const char* datadir, const char* rootfile,const char* logfile,int bunchid_diff,const char* prefix,const char* suffix)
  {
    
  }
  
  int check_ungrouped(const char* datadir,const char* outfile,const char* configdir)
  {
    TH1F* hbunch_mwdc=new TH1F("hbunch_mwdc","hbunch_mwdc",6,-0.5,5.5);
    TH1F* hbunch_tof=new TH1F("hbunch_tof","hbunch_tof",6,-0.5,5.5);
    TH1F* hbunch_all=new TH1F("hbunch_all","hbunch_all",6,-0.5,5.5);
    //readin the config file which include channelmapping info
    TString file_config;
    if(!configdir)
      file_config=TString(datadir)+"/crate.json";
    else
      file_config=TString(configdir)+"/crate.json";
    CrateInfo* info=read_config(file_config.Data(),"mapping");
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
    Int_t*	mwdc_eventid=new Int_t[mwdcnum]{};
    Int_t*	mwdc_bunchid=new Int_t[mwdcnum]{};
    
    TTree** 	tree_in_tof=new TTree*[tofnum]{};
    BoardInfo**	tof_boardinfo=new BoardInfo*[tofnum]{};
    Int_t*	tof_eventid=new Int_t[tofnum]{};
    Int_t*	tof_bunchid=new Int_t[tofnum]{};
    
    TTree** 	tree_in=new TTree*[boardnum]{};
    mwdcnum=0;tofnum=0;
    for(int i=0;i<boardnum;i++){
      switch (boardinfo[i]->GetType()){
	case EMWDC:
	  raw_dir->GetObject(boardinfo[i]->GetName(),tree_in_mwdc[mwdcnum++]);
	  mwdc_boardinfo[mwdcnum-1]=boardinfo[i];
	  tree_in[i]=tree_in_mwdc[mwdcnum-1];
	  
	  tree_in[i]->SetBranchAddress("event_id",&mwdc_eventid[mwdcnum-1]);
	  tree_in[i]->SetBranchAddress("bunch_id",&mwdc_bunchid[mwdcnum-1]);
	  /*tree_in[i]->Print();
	   */
	  mwdc_boardinfo[mwdcnum-1]->Print();
	  break;
	case ETOF:
	  raw_dir->GetObject(boardinfo[i]->GetName(),tree_in_tof[tofnum++]);
	  tof_boardinfo[tofnum-1]=boardinfo[i];
	  tree_in[i]=tree_in_tof[tofnum-1];
	  
	  tree_in[i]->SetBranchAddress("event_id",&tof_eventid[tofnum-1]);
	  tree_in[i]->SetBranchAddress("bunch_id",&tof_bunchid[tofnum-1]);
	  /*tree_in[i]->Print();
	   */
	  tof_boardinfo[tofnum-1]->Print();
	  break;
	default:
	  break;
      }
    }
    
    //
    Int_t temp_entries;
    Int_t entries=tree_in[0]->GetEntriesFast();
    for(int i=0;i<boardnum;i++){
      temp_entries=tree_in[i]->GetEntriesFast();
      if(temp_entries<entries){
	entries=temp_entries;
      }
    }
    Int_t max_eventid,min_eventid,temp_eventid;
    Int_t max_bunchid,min_bunchid,temp_bunchid;
    Int_t unmatched_event=0,unmatched_bunch_mwdc=0,unmatched_bunch_tof=0;
    Int_t matched_bunch=0;
    //start merge loop
    for(int i=0;i<entries;i++){
      if(!((i+1)%20000)){
	printf("%d events checked\n",i+1);
      }
      //
      for(int j=0;j<boardnum;j++){
	tree_in[j]->GetEntry(i);
      }
      //check trigger_id and bunch_id
      max_eventid=TMath::MaxElement(mwdcnum,mwdc_eventid);
      min_eventid=TMath::MinElement(mwdcnum,mwdc_eventid);
      max_bunchid=TMath::MaxElement(mwdcnum,mwdc_bunchid);
      min_bunchid=TMath::MinElement(mwdcnum,mwdc_bunchid);
      if(max_eventid!=min_eventid){
	//printf("ERROR event_%d:unmatched event_id between MWDC boards(T:%d,%d)\n",i+1,max_eventid,min_eventid);
      }
      else{
	temp_eventid=max_eventid;
      }
      if(max_bunchid != min_bunchid){
	unmatched_bunch_mwdc++;
	temp_bunchid=-1;
      }
      else{
	temp_bunchid=max_bunchid;
      }
      hbunch_mwdc->Fill(max_bunchid-min_bunchid);
      //
      max_eventid=TMath::MaxElement(tofnum,tof_eventid);
      min_eventid=TMath::MinElement(tofnum,tof_eventid);
      max_bunchid=TMath::MaxElement(tofnum,tof_bunchid);
      min_bunchid=TMath::MinElement(tofnum,tof_bunchid);
      if((max_eventid!=min_eventid)){
	//printf("ERROR event_%d:unmatched event_id between TOF boards(T:%d,%d)\n",i+1,max_eventid,min_eventid);
      }
      else if((temp_eventid != max_eventid)){
	//printf("ERROR event_%d:unmatched event_id between MWDC and TOF boards(T:%d,%d)\n",i+1,max_eventid,temp_eventid);
      }
      if(max_bunchid != min_bunchid){
	unmatched_bunch_tof++;
      }
      else if((temp_bunchid != -1) && (TMath::Abs(temp_bunchid-max_bunchid)==0)){ //|| temp_bunchid==max_bunchid) ){
	  matched_bunch++;
      }
      hbunch_tof->Fill(max_bunchid-min_bunchid);
      //
      hbunch_all->Fill(TMath::MaxElement(tofnum,tof_bunchid)-TMath::MinElement(mwdcnum,mwdc_bunchid));
    }
    printf("total events %d\n",entries);
    printf("total mathced bunch events %d\n",matched_bunch);
    printf("MWDC unmatched bunch events: %d\n",unmatched_bunch_mwdc);
    printf("TOF unmatched bunch events: %d\n",unmatched_bunch_tof);
    //
    gStyle->SetOptStat(111111);
    TCanvas *c1=new TCanvas("c1","c1",1200,400);
    c1->Divide(3,1);
    c1->cd(1);
    hbunch_mwdc->Draw();
    c1->cd(2);
    hbunch_tof->Draw();
    c1->cd(3);
    hbunch_all->Draw();
    TString bunchvalid=TString(datadir)+"/bunchvalidation.pdf";
    c1->Print(bunchvalid.Data());
    //
    delete file_out;
    delete [] tree_in_mwdc;
    delete [] mwdc_boardinfo;
    delete [] mwdc_eventid;
    delete [] mwdc_bunchid;
    
    delete [] tree_in_tof;
    delete [] tof_boardinfo;
    delete [] tof_eventid;
    delete [] tof_bunchid;
    
    delete [] tree_in;
    delete [] boardinfo;
    //
    if(((float)matched_bunch)/entries > 0.9){
      printf("all boards synchronized successfully!\n");
      return 0;
    }
    else
      return -1;
  }
  
  int merge_hptdc_ungrouped(const char* datadir,const char* outfile,const char* configdir)
  {
    //readin the config file which include channelmapping info
    TString file_config;
    if(!configdir)
      file_config=TString(datadir)+"/crate.json";
    else
      file_config=TString(configdir)+"/crate.json";
    CrateInfo* info=read_config(file_config.Data(),"mapping");
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
    ChannelMap** 	mwdc_leading_raw=new ChannelMap*[mwdcnum]{};
    ChannelMap** 	mwdc_trailing_raw=new ChannelMap*[mwdcnum]{};
    
    TTree** 	tree_in_tof=new TTree*[tofnum]{};
    BoardInfo**	tof_boardinfo=new BoardInfo*[tofnum]{};
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
	  
	  tree_in[i]->SetBranchAddress("event_id",&mwdc_eventid[mwdcnum-1]);
	  tree_in[i]->SetBranchAddress("bunch_id",&mwdc_bunchid[mwdcnum-1]);
	  tree_in[i]->SetBranchAddress("leading_raw",&mwdc_leading_raw[mwdcnum-1]);
	  tree_in[i]->SetBranchAddress("trailing_raw",&mwdc_trailing_raw[mwdcnum-1]);
	  /*tree_in[i]->Print();
	   */
	  mwdc_boardinfo[mwdcnum-1]->Print();
	  break;
	case ETOF:
	  raw_dir->GetObject(boardinfo[i]->GetName(),tree_in_tof[tofnum++]);
	  tof_boardinfo[tofnum-1]=boardinfo[i];
	  tree_in[i]=tree_in_tof[tofnum-1];
	  
	  tree_in[i]->SetBranchAddress("event_id",&tof_eventid[tofnum-1]);
	  tree_in[i]->SetBranchAddress("bunch_id",&tof_bunchid[tofnum-1]);
	  tree_in[i]->SetBranchAddress("time_leading_raw",&tof_timeleading_raw[tofnum-1]);
	  tree_in[i]->SetBranchAddress("time_trailing_raw",&tof_timetrailing_raw[tofnum-1]);
	  tree_in[i]->SetBranchAddress("tot_leading_raw",&tof_totleading_raw[tofnum-1]);
	  tree_in[i]->SetBranchAddress("tot_trailing_raw",&tof_tottrailing_raw[tofnum-1]);
	  /*tree_in[i]->Print();
	   */
	  tof_boardinfo[tofnum-1]->Print();
	  break;
	default:
	  break;
      }
    }
    //init output variables
    TDirectory* dir_out=file_out->GetDirectory("merge");
    if(!dir_out){
      dir_out=file_out->mkdir("merge");
      if(!dir_out){
	printf("error!can't mkdir \"merge\" in %s\n",outfile);
	exit(1);
      }
    }
    dir_out->cd();
    
    ChannelMap mwdc_leading,mwdc_trailing;
    ChannelMap tof_timeleading,tof_timetrailing,tof_totleading,tof_tottrailing;
    TTree* tree_out_mwdc=new TTree("mwdc","mwdc");
    tree_out_mwdc->Branch("leading_raw",&mwdc_leading);
    tree_out_mwdc->Branch("trailing_raw",&mwdc_trailing);
    TTree* tree_out_tof=new TTree("tof","tof");
    tree_out_tof->Branch("time_leading_raw",&tof_timeleading);
    tree_out_tof->Branch("time_trailing_raw",&tof_timetrailing);
    tree_out_tof->Branch("tot_leading_raw",&tof_totleading);
    tree_out_tof->Branch("tot_trailing_raw",&tof_tottrailing);
    
    Int_t temp_entries;
    Bool_t flag_syn=true;
    Int_t entries=tree_in[0]->GetEntriesFast();
    for(int i=0;i<boardnum;i++){
      temp_entries=tree_in[i]->GetEntriesFast();
      if(temp_entries!=entries){
	flag_syn=false;
	if(temp_entries<entries){
	  entries=temp_entries;
	}
      }
    }
    //
    ChannelMap::iterator it;
    Int_t max_eventid,min_eventid,temp_eventid;
    Int_t max_bunchid,min_bunchid,temp_bunchid;
    
    //start merge loop
    for(int i=0;i<entries;i++){
      if(!((i+1)%5000)){
	printf("%d events merged\n",i+1);
      }
      //
      for(int j=0;j<boardnum;j++){
	tree_in[j]->GetEntry(i);
      }
      //check trigger_id and bunch_id
      /*
      max_eventid=TMath::MaxElement(mwdcnum,mwdc_eventid);
      min_eventid=TMath::MinElement(mwdcnum,mwdc_eventid);
      max_bunchid=TMath::MaxElement(mwdcnum,mwdc_bunchid);
      min_bunchid=TMath::MinElement(mwdcnum,mwdc_bunchid);
      if((max_eventid!=min_eventid) || TMath::Abs(max_bunchid-min_bunchid)>1){
	printf("event_%d:unmatched event_id/bunch_id between MWDC boards(T:%d,%d|B:%d,%d)\n",i+1,max_eventid,min_eventid,max_bunchid,min_bunchid);
      }
      else{
	temp_bunchid=max_bunchid;
	temp_eventid=max_eventid;
      }
      max_eventid=TMath::MaxElement(tofnum,tof_eventid);
      min_eventid=TMath::MinElement(tofnum,tof_eventid);
      max_bunchid=TMath::MaxElement(tofnum,tof_bunchid);
      min_bunchid=TMath::MinElement(tofnum,tof_bunchid);
      if((max_eventid!=min_eventid) || TMath::Abs(max_bunchid-min_bunchid)>1){
	printf("event_%d:unmatched event_id/bunch_id between TOF boards(T:%d,%d|B:%d,%d)\n",i+1,max_eventid,min_eventid,max_bunchid,min_bunchid);
      }
      else if((temp_eventid != max_eventid) || ((TMath::Abs(max_bunchid-temp_bunchid)!=1) && (max_bunchid!=temp_bunchid))){
	printf("event_%d:unmatched event_id/bunch_id between TOF and MWDC boards(T:%d,%d|B:%d,%d)\n",i+1,temp_eventid,max_eventid,temp_bunchid,max_bunchid);
      }
      */
      //main merge process
      mwdc_leading.clear();mwdc_trailing.clear();
      for(int j=0;j<mwdcnum;j++){
	for(it=mwdc_leading_raw[j]->begin();it!=mwdc_leading_raw[j]->end();it++){
	  if(mwdc_boardinfo[j]->IsChannelValid(it->first)){
	    mwdc_leading[mwdc_boardinfo[j]->GetEncodedID(it->first)]=it->second;
	  }
	}
	for(it=mwdc_trailing_raw[j]->begin();it!=mwdc_trailing_raw[j]->end();it++){
	  if(mwdc_boardinfo[j]->IsChannelValid(it->first)){
	    mwdc_trailing[mwdc_boardinfo[j]->GetEncodedID(it->first)]=it->second;
	  }
	}
      }
      tof_timeleading.clear();tof_timetrailing.clear();
      tof_totleading.clear();tof_tottrailing.clear();
      for(int j=0;j<tofnum;j++){
	for(it=tof_timeleading_raw[j]->begin();it!=tof_timeleading_raw[j]->end();it++){
	  if(tof_boardinfo[j]->IsChannelValid(it->first)){
	    tof_timeleading[tof_boardinfo[j]->GetEncodedID(it->first)]=it->second;
	  }
	}
	for(it=tof_timetrailing_raw[j]->begin();it!=tof_timetrailing_raw[j]->end();it++){
	  if(tof_boardinfo[j]->IsChannelValid(it->first)){
	    tof_timetrailing[tof_boardinfo[j]->GetEncodedID(it->first)]=it->second;
	  }
	}
	for(it=tof_totleading_raw[j]->begin();it!=tof_totleading_raw[j]->end();it++){
	  if(tof_boardinfo[j]->IsChannelValid(it->first)){
	    tof_totleading[tof_boardinfo[j]->GetEncodedID(it->first)]=it->second;
	  }
	}
	for(it=tof_tottrailing_raw[j]->begin();it!=tof_tottrailing_raw[j]->end();it++){
	  if(tof_boardinfo[j]->IsChannelValid(it->first)){
	    tof_tottrailing[tof_boardinfo[j]->GetEncodedID(it->first)]=it->second;
	  }
	}
      }
      //
      tree_out_mwdc->Fill();
      tree_out_tof->Fill();
    }
    
    printf("%d events merged!\n",entries);
	
	tree_out_mwdc->Write(0,TObject::kOverwrite);
	tree_out_tof->Write(0,TObject::kOverwrite);
	//
	delete file_out;
	delete [] tree_in_mwdc;
	delete [] mwdc_boardinfo;
	delete [] mwdc_eventid;
	delete [] mwdc_bunchid;
	delete [] mwdc_leading_raw;
	delete [] mwdc_trailing_raw;
	
	delete [] tree_in_tof;
	delete [] tof_boardinfo;
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
	return flag_syn;
  }

}