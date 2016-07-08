/*
 T his program is free s*oftware; you can redistribute it and/or
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

#ifndef _BoardInfo_h_
#define _BoardInfo_h_

#include "TNamed.h"
#include "global.h"

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
/////////////////////////////////////////////////////
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
  UInt_t fEncodedID[128];//index is global channel in utility.cxx(convert_mwdc),
                         //i.e. the channel index in the front-panel of the board.
  
  ClassDef(MWDCBoard,1)
};
////////////////////////////////////////////////////////////////
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
  UInt_t fEncodedID[16];//index is global channel in utility.cxx(convert_tof),
                        //i.e. the channel index in the front-panel of the board.
  
  ClassDef(TOFBoard,1)
};
#endif // _BoardInfo_h_