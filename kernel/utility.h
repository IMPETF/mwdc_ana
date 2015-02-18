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

#ifndef _Utility_h_
#define _Utility_h_

#include "json.h"
#include "Rtypes.h"
#include <stdio.h>

class BoardInfo;
class CrateInfo;
class TTree;

namespace Utility {
  //
  void read_mwdcinfo(BoardInfo* boardinfo,Json::Value& board);
  void read_tofinfo(BoardInfo* boardinfo,Json::Value& board);
  CrateInfo* read_config(const char* filename,const char* prefix,const char* suffix=".at1");
  //
  TTree* convert_mwdc(const char* infile,const char* name,const char* title);
  TTree* convert_tof(const char* infile,const char* name,const char* title);
  int convert_hptdc(const char* datadir,const char* outfile,const char* prefix,const char* suffix=".at1");
  int print_info(const char* datadir, const char* rootfile,const char* logfile,int bunchid_diff=3,const char* prefix="fuck",const char* suffix=".at1");
  int check(const char* datadir,const char* outfile);
  //
  int merge_hptdc(const char* datadir,const char* outfile);
  void mapping_validation(const char* datadir,const char* outfile);
  //For PSD
  int convert_eventblock(char* data,Int_t *Xpos,Int_t *Ypos,Int_t *Xneg,Int_t *Yneg);
  int convert_psd(const Char_t* parentDir,const Char_t* infile,const Char_t* outDir,const Char_t* outfile="raw.root");

  //For ungrouped MWDC rawdata
  char _GetNextEvent_ungrouped(FILE*fp,unsigned int* buffer,int* buffer_len,int* event_len,int* bunch_id,int* event_id);
  TTree* convert_mwdc_ungrouped(const char* infile,const char* name,const char* title);
  TTree* convert_tof_ungrouped(const char* infile,const char* name,const char* title);
  int convert_hptdc_ungrouped(const char* datadir,const char* outfile,const char* prefix,const char* suffix=".at1");
  
}
#endif // _Utility_h_