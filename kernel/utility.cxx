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
#include "TFile.h"
#include "TStyle.h"
#include <fstream>
#include <stdio.h>
#include <iostream>

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
  //
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
            hxpos[i]=new TH1F(Form("xpos_%d",i+1),Form("xpos_%d",i+1),400,0,4000);
            hxneg[i]=new TH1F(Form("xneg_%d",i+1),Form("xneg_%d",i+1),400,0,4000);
            hypos[i]=new TH1F(Form("ypos_%d",i+1),Form("ypos_%d",i+1),400,0,4000);
            hyneg[i]=new TH1F(Form("yneg_%d",i+1),Form("yneg_%d",i+1),400,0,4000);
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
  
  int convert_hptdc_ungrouped(const char* datadir,const char* outfile,const char* prefix,const char* suffix)
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
  
  int check_ungrouped(const char* datadir,const char* outfile)
  {
    TH1F* hbunch_mwdc=new TH1F("hbunch_mwdc","hbunch_mwdc",6,-0.5,5.5);
    TH1F* hbunch_tof=new TH1F("hbunch_tof","hbunch_tof",6,-0.5,5.5);
    TH1F* hbunch_all=new TH1F("hbunch_all","hbunch_all",6,-0.5,5.5);
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
    return 0;
  }
  
  int merge_hptdc_ungrouped(const char* datadir,const char* outfile)
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
    Int_t entries=tree_in[0]->GetEntriesFast();
    for(int i=0;i<boardnum;i++){
      temp_entries=tree_in[i]->GetEntriesFast();
      if(temp_entries<entries){
	entries=temp_entries;
      }
    }
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
	return 0;
  }

}