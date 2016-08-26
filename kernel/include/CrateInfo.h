// Copyright (C) 2016  Yong Zhou

// This program is free software: you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation, either version 3 of the License, or (at your option)
// any later version.

// This program is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
// more details.

// You should have received a copy of the GNU General Public License along
// with this program.  If not, see <http://www.gnu.org/licenses/>.

#ifndef _CrateInfo_h_
#define _CrateInfo_h_

#include "BoardInfo.h"
#include <vector>
#include <map>
#include <algorithm>

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

#endif // _CrateInfo_h_