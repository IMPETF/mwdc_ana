//This is a source file written by ufan 
//Created at 2015 01/20/15 
#include <cstdio>
#include <cstdlib>
#include <vector>
#include <map>
#include <fstream>
#include <iostream>
#include <algorithm>
#include "TTree.h"
#include "TFile.h"
#include "TMath.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TCanvas.h"
#include "TGraph.h"
#include "TGraphErrors.h"
#include "TMultiGraph.h"
#include "TAxis.h"
#include "TChain.h"
#include "TEntryList.h"
#include "TROOT.h"
#include "TProfile.h"
#include "TString.h"
#include "json/jsoncpp.cpp"
//
typedef std::map< UInt_t,std::vector<int> > ChannelMap;
typedef std::vector<int> TDCContainer;
//
enum EBoardType {
    EMWDC=0,
    ETOF,
    EUNKNOWN
};
enum ELocation {
  EDOWN=0,
  EUP
};
enum EDirection {
  EX=0,
  EY,
  EU
};
namespace Encoding {
    inline UInt_t EncodeType(UChar_t type){
      return (type&0xFF)<<24;
    }
    inline UInt_t EncodeLocation(UChar_t location){
      return (location&0xF)<<20;
    }
    inline UInt_t EncodeDirection(UChar_t direction){
      return (direction&0xF)<<16;
    }
    inline UInt_t Encode(UChar_t type,UChar_t location,UChar_t direction,UShort_t index){
      return ((type&0xFF)<<24)+((location&0xF)<<20)+((direction&0xF)<<16)+index;
    }
    //
    inline UChar_t DecodeType(UInt_t encoded_id){
      return encoded_id>>24;
    }
    inline UChar_t DecodeLocation(UInt_t encoded_id){
      return (encoded_id>>20)&0xF;
    }
    inline UChar_t DecodeDirection(UInt_t encoded_id){
      return (encoded_id>>16)&0xF;
    }
    inline UShort_t DecodeIndex(UInt_t encoded_id){
      return encoded_id&0xFFFF;
    }
    inline void Decode(UInt_t encoded_id,UChar_t& type,UChar_t& location,UChar_t& direction,UShort_t& index){
      type = (encoded_id>>24);
      location = (encoded_id>>20)&0xF;
      direction = (encoded_id>>16)&0xF;
      index = encoded_id&0xFFFF;
    }
}
//
class BoardInfo :public TNamed{
public:
  BoardInfo():TNamed("unknown","unknown"),
  fBoardType(EUNKNOWN) {}
  BoardInfo(const char*name,const char* title,EBoardType type):
  TNamed(name,title),fBoardType(type) {}
  virtual ~BoardInfo() {}
  
  virtual UInt_t 	GetEncodedID(UInt_t channel_id) =0;
  virtual void   	SetEncodedID(UInt_t channel_id,UInt_t encoded_id)=0;
  virtual const char* 	GetLocDescription(UInt_t channel_id)=0;
  virtual const char* 	GetDirDescription(UInt_t channel_id)=0;
  virtual UShort_t 	GetChannelID(UInt_t channel_id)=0;
  virtual Bool_t   	IsChannelValid(UInt_t channel_id)=0;
  virtual int 		GetChannelNum()=0;
  
public:
  void SetType(EBoardType type) {fBoardType=type;}
  EBoardType GetType() {return fBoardType;}
  void SetInit(Bool_t status) {fInit=status;}
  Bool_t IsInit() {return fInit;}
  const char* GetTypeDescription(){
    switch (fBoardType) {
      case EMWDC:
	return "mwdc";
	break;
      case ETOF:
	return "tof";
      default:
	return "unknown";
	break;
    }
  }

private:
  EBoardType	 fBoardType;
  Bool_t	 fInit;
  
  ClassDef(BoardInfo,1)
};

class MWDCBoard:public BoardInfo {
public:
  MWDCBoard() {}
  MWDCBoard(const char* name,const char* title):
  BoardInfo(name,title,EMWDC) {}
  virtual ~MWDCBoard() {}
  
public:
  virtual UInt_t GetEncodedID(UInt_t channel_id) {return fEncodedID[channel_id];}
  virtual void   SetEncodedID(UInt_t channel_id,UInt_t encoded_id) {fEncodedID[channel_id]=encoded_id;}
  virtual const char* GetLocDescription(UInt_t channel_id){
    UChar_t location=Encoding::DecodeLocation(fEncodedID[channel_id]);
    switch (location) {
      case 0:
	return "Down";
	break;
      case 1:
	return "Up";
	break;
      default:
	return 0;
	break;
    }
  }
  virtual const char* GetDirDescription(UInt_t channel_id){
    UChar_t direction=Encoding::DecodeDirection(fEncodedID[channel_id]);
    switch (direction) {
      case 0:
	return "X";
	break;
      case 1:
	return "Y";
	break;
      case 2:
	return "U";
	break;
      default:
	return 0;
	break;
    }
  }
  virtual UShort_t GetChannelID(UInt_t channel_id){
    return Encoding::DecodeIndex(fEncodedID[channel_id]);
  }
  virtual Bool_t IsChannelValid(UInt_t channel_id){
    UChar_t type=Encoding::DecodeType(fEncodedID[channel_id]);
    if(type == 0xFF){
      return false;
    }
    return true;
  }
    virtual int GetChannelNum(){return 128;}
private:
  UInt_t fEncodedID[128];
  
  ClassDef(MWDCBoard,1)
};

class TOFBoard:public BoardInfo {
public:
  TOFBoard() {}
  TOFBoard(const char* name,const char* title):
  BoardInfo(name,title,ETOF) {}
  virtual ~TOFBoard() {}
  
public:
  virtual UInt_t GetEncodedID(UInt_t channel_id) {return fEncodedID[channel_id];}
  virtual void   SetEncodedID(UInt_t channel_id,UInt_t encoded_id) {fEncodedID[channel_id]=encoded_id;}
  virtual const char* GetLocDescription(UInt_t channel_id){
    UChar_t location=Encoding::DecodeLocation(fEncodedID[channel_id]);
    switch (location) {
      case 0:
	return "Down";
	break;
      case 1:
	return "Up";
	break;
      default:
	return 0;
	break;
    }
  }
  virtual const char* GetDirDescription(UInt_t channel_id){
    return "undefined";
  }
  virtual UShort_t GetChannelID(UInt_t channel_id){
    return Encoding::DecodeIndex(fEncodedID[channel_id]);
  }
  virtual Bool_t IsChannelValid(UInt_t channel_id){
    UChar_t type=Encoding::DecodeType(fEncodedID[channel_id]);
    if(type == 0xFF){
      return false;
    }
    return true;
  }
  virtual int GetChannelNum(){return 16;}
private:
  UInt_t fEncodedID[16];
  
  ClassDef(TOFBoard,1)
};

class CrateInfo :public TNamed{
public:
  CrateInfo() {}
  CrateInfo(const char* name):TNamed(name,name) {SetName(name);}
  virtual ~CrateInfo(){    
    Clear();
  }
  int GetBoardNum(){ return fileid.size(); }
  int GetFileid(int index){ return fileid[index]; }
  TString GetFilename(int index){
    int id=GetFileid(index);
    return filename[id];
  }
  TString GetBoardtype(int index){
    int id=GetFileid(index);
    return boardtype[id];
  }
  TString GetBoardname(int index){
    int id=GetFileid(index);
    return boardname[id];
  }
  TString GetBoardtitle(int index){
    int id=GetFileid(index);
    return boardtitle[id];
  }
  BoardInfo* GetBoardInfo(int index){
    int id=GetFileid(index);
    return boardinfo[id];
  }
  void SetBoardInfo(BoardInfo* info){
    TString name=info->GetName();
    int size=fileid.size();
    int i;
    for (i = 0 ; i <size; ++i) {
      if (boardname[fileid[i]] == name) {
	  if(boardinfo[fileid[i]]){
	    delete boardinfo[fileid[i]];
	  }
	  boardinfo[fileid[i]]=info;
	  break;
      }
    }
    if(i==size){
      printf("warning: no such board :%s\n",name.Data());
    }
  }
  void Clear(){
    int size=fileid.size();
    for(int i=0;i<size;i++){
      if(boardinfo[fileid[i]])
	delete boardinfo[fileid[i]];
    }
    fileid.clear();
    filename.clear();
    boardtype.clear();
    boardname.clear();
    boardtitle.clear();
  }
  void Print(){
    int boardnum=GetBoardNum();
    printf("crate config:\n");
    printf("filename\t\tboardtype\t\tboardname\t\tboardtitle\n");
    for(int i=0;i<boardnum;i++){
      printf("%s\t%s\t%s\t%s\n",GetFilename(i).Data(),GetBoardtype(i).Data(),GetBoardname(i).Data(),GetBoardtitle(i).Data());
    }
    printf("\n");
  }
  bool IsContained(int newfileid){
    std::vector<int>::iterator it;
    it=std::find(fileid.begin(),fileid.end(),newfileid);
    return (it!=fileid.end())?true:false;
  }
  
  std::vector<int>	fileid;
  std::map<int,TString>	filename;
  std::map<int,TString>	boardtype;
  std::map<int,TString>	boardname;
  std::map<int,TString>	boardtitle;
  std::map<int,BoardInfo*> boardinfo;
  
  ClassDef(CrateInfo,1)
};

/////////////////////////////////////////////////////////////
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

CrateInfo* read_config(const char* filename,const char* prefix,const char* suffix=".at1")
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
  
  
  info->Print();
  
  return info;
}

TTree* convert_mwdc(const char* infile,const char* name,const char* title)
{
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
  int event_id=0;
  int bunch_id=0;
  ChannelMap leading_raw;
  ChannelMap trailing_raw;
  int tdc_index,channel_index,tdc_value;
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
	printf("unexpected behavior\n");
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
    if(event_len != word_count){
      //printf("event_len != word_count: %d_%d\n",event_len,word_count);
    }
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
  int tdc_index,channel_index,tdc_value;
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
	printf("unexpected behavior\n");
	exit(1);
      }
      printf("(packet_%u) insufficent data\n",packet_num);
    }
    if(event_len%4){
      printf("(packet_%u) event_len error\n",packet_num);
    }
    event_len=event_len/4;
    word_count=buffer[event_len-1]&0xFF;
    if(event_len != word_count){//interesting behavior:word_count and event_len is not always consistent.
      //printf("event_len != word_count: %d_%d\n",event_len,word_count);
    }
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
	      tdc_value=(buffer[i]&0x7FFFF)<<2 + ((buffer[i]>>19)&0x3);
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
	      tdc_value=(buffer[i]&0x7FFFF)<<2 + ((buffer[i]>>19)&0x3);
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

int convert_hptdc(const char* datadir,const char* outfile,const char* prefix,const char* suffix=".at1")
{
  TString file_config=TString(datadir)+"/crate.json";
  CrateInfo* info=read_config(file_config.Data(),prefix,suffix);
  //info->Print();
  
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

int print_info(const char* datadir, const char* rootfile,const char* logfile,int bunchid_diff=3,const char* prefix="fuck",const char* suffix=".at1")
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
    /*
    if(i == (min-1)){
	for(int j=0;j<boardnum;j++){
	  finalbunch_id[j]=bunch_id[j];
          finalevent_id[j]=event_id[j];
        }
    }
    */
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
  BoardInfo** boardinfo=new BoardInfo*[boardnum];
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
  
  TH1F* h1=new TH1F("h1","h1",512,0,512);
  int tem=0;
  //start merge loop
  for(int i=0;i<entries;i++){
    tem=0;
    if(!(i%4999)){
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
	  tem++;
	}
      }
      for(it=mwdc_trailing_raw[j]->begin();it!=mwdc_trailing_raw[j]->end();it++){
	if(mwdc_boardinfo[j]->IsChannelValid(it->first)){
	  mwdc_trailing[mwdc_boardinfo[j]->GetEncodedID(it->first)]=it->second;
	}
      }
    }
    h1->Fill(tem);
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
  h1->Write(0,TObject::kOverwrite);
  delete file_out;
  //h1->Draw();
  return 0;
}

void analysis(const char* datadir,const char* outfile)
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
  //c->cd(1);hmwdc_size->Draw();
  //c->cd(2);htof_size->Draw();
  
}
#ifdef __MAKECINT__
//#pragma link C++ class std::vector<int>+;
#pragma link C++ class std::map<UInt_t,std::vector<int> >+;
#endif