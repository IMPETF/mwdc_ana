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

#include "TGraphErrors.h"
#include "TFile.h"
#include "TMath.h"

void ana_attenuation_dump(const char* infile,const char* outdir)
{
  char label[4][10]={"xpos","xneg","ypos","yneg"};
  //  
  int nsegments;
  double *Pos,*MPV,*MPV_error;
  
  TGraphErrors* graph;
  
  FILE* fp[4];
  
  TFile* file_in=new TFile(infile);
  //
  for(int side=0;side<4;side++){
    fp[side]=fopen(Form("%s/attenuation_dump_%s.csv",outdir,label[side]),"w");
    //
    for(int bar=0;bar<41;bar++){
      graph=(TGraphErrors*)file_in->Get(Form("g%s_attlength_%d",label[side],bar+1));
      graph->Sort();
      nsegments=graph->GetN();
      Pos=graph->GetX();
      MPV=graph->GetY();
      MPV_error=graph->GetEY();
      //
      if(bar==0){
	fprintf(fp[side],"Position");
	for(int segment=0;segment<nsegments;segment++){
	  fprintf(fp[side],",%.4f",Pos[segment]);
	}
	fprintf(fp[side],"\n");
      }
      //
      fprintf(fp[side],"%d",bar+1);
      for(int segment=0;segment<nsegments;segment++){
	fprintf(fp[side],",%.4f",MPV[segment]);
      }
      fprintf(fp[side],"\n");
      //
      delete graph;
    }
    //
    fclose(fp[side]);
  }
  
  delete file_in;
}

void ana_attenuation_maxmin(const char* infile,const char* outdir)
{
  char label[4][10]={"xpos","xneg","ypos","yneg"};
  //  
  int nsegments;
  double *Pos,*MPV,*MPV_error;
  double max,min,middle;
  
  TGraphErrors* graph;
  
  FILE* fp[4];
  
  TFile* file_in=new TFile(infile);
  //
  for(int side=0;side<4;side++){
    fp[side]=fopen(Form("%s/attenuation_maxmin_%s.csv",outdir,label[side]),"w");
    fprintf(fp[side],",MAX,MIDDLE,MIN,,MAX/MIN,MAX/MIDDLE,MIDDLE/MIN\n");
    //
    for(int bar=0;bar<41;bar++){
      graph=(TGraphErrors*)file_in->Get(Form("g%s_attlength_%d",label[side],bar+1));
      graph->Sort();
      nsegments=graph->GetN();
      Pos=graph->GetX();
      MPV=graph->GetY();
      MPV_error=graph->GetEY();
      //
      max=TMath::MaxElement(nsegments,MPV);
      min=TMath::MinElement(nsegments,MPV);
      middle=MPV[20];
      fprintf(fp[side],"%d,%.4f,%.4f,%.4f,,%.4f,%.4f,%.4f\n",bar+1,max,middle,min,max/min,max/middle,middle/min);
      //
      delete graph;
    }
    //
    fclose(fp[side]);
  }
  
  delete file_in;
}