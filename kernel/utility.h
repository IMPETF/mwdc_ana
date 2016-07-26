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
class TDirectory;
class TF1;
class TH1F;
class TProfile;

namespace Utility {
  // Read channel mapping info(front-panel channel--> detector channel) from json configuration file
  void read_mwdcinfo(BoardInfo* boardinfo,Json::Value& board);
  void read_tofinfo(BoardInfo* boardinfo,Json::Value& board);
  CrateInfo* read_config(const char* filename,const char* prefix,const char* suffix=".at1");
  
  //Deprecated: For MWDC grouped raw format decoding, merging and validation
  TTree* convert_mwdc(const char* infile,const char* name,const char* title);
  TTree* convert_tof(const char* infile,const char* name,const char* title);
  int convert_hptdc(const char* datadir,const char* outfile,const char* prefix,const char* suffix=".at1");
  int print_info(const char* datadir, const char* rootfile,const char* logfile,int bunchid_diff=3,const char* prefix="fuck",const char* suffix=".at1");
  int check(const char* datadir,const char* outfile);
  //
  int merge_hptdc(const char* datadir,const char* outfile);
  void mapping_validation(const char* datadir,const char* outfile);
  
  //For PSD decoding, fitting and validation
  int convert_eventblock(char* data,Int_t *Xpos,Int_t *Ypos,Int_t *Xneg,Int_t *Yneg);
  int convert_psd(const Char_t* parentDir,const Char_t* infile,const Char_t* outDir,const Char_t* outfile="raw.root");
  TF1* langaus( TH1F *poHist,float& mpv,float& fwhm,float ped_mean,float ped_sigma);
  Double_t langaufun(Double_t *x, Double_t *par);
  TF1 *langaufit(TH1F *his, Double_t *fitrange, Double_t *startvalues, Double_t *parlimitslo, Double_t *parlimitshi, Double_t *fitparams, Double_t *fiterrors, Double_t *ChiSqr, Int_t *NDF);
  TF1* pedfit(TH1F* poHist,float ped_mean,float ped_sigma,float& pedout_mean,float& pedout_sigma);
  TF1* linear_fit(TProfile* hprofile,Double_t *fitrange);
  int draw_channels(const char* pardir,const char* filename,const char* outDir,const char* outName);
  int draw_relp(const Char_t* testfile,const Char_t* reffile,const Char_t* outdir,const char* outName);
  int draw_mip(const char* mipfile,const char* pedfile,const char* outDir,const char* outName);
  int draw_mapping(const char* pardir,const char* filename,const char* outDir,unsigned int max=600000);
  int fit_dy58(const char* infile,const char* pedfile,const char* outdir,const char* outfile,int pedcut=5,float range=200.0);
  
  //For ungrouped MWDC rawdata decoding, merging and validation
  char _GetNextEvent_ungrouped(FILE*fp,unsigned int** buffer,int* buffer_len,int* event_len,int* bunch_id,int* event_id,int* word_count);
  TTree* convert_mwdc_ungrouped(const char* infile,TDirectory* dir,const char* name,const char* title);
  TTree* convert_tof_ungrouped(const char* infile,TDirectory* dir,const char* name,const char* title);
  int convert_hptdc_ungrouped(const char* datadir,const char* outfile,const char* prefix,const char* configdir=0,const char* suffix=".at1");
  int print_info_ungrouped(const char* datadir, const char* rootfile,const char* logfile,int bunchid_diff=1,const char* prefix="fuck",const char* suffix=".at1");
  int check_ungrouped(const char* datadir,const char* outfile,const char* configdir=0);
  int merge_hptdc_ungrouped(const char* datadir,const char* outfile,const char* configdir=0);

  Double_t calc_starttime(Int_t* timetag,Double_t v_eff=15);
}
#endif // _Utility_h_