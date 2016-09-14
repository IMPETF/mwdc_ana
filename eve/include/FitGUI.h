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

#ifndef __FitGUI__
#define __FitGUI__

#include "EventHandler.h"
#include "TrackFit.h"
#include <Fit/Fitter.h>

class TFile;
class TTree;
class TSpline5;
class TGraph;
class DriftInfo;
class TF1;

class FitGUI : public EventHandler
{

public:
  FitGUI();
  FitGUI(const char* filename);
  ~FitGUI();

  void SetFile(const char* filename);
  void Init();

private:
  virtual void DropEvent();
  virtual Bool_t ReadEvent();
  virtual void AddEvent();

  // void GetRtRelation();
  // Bool_t	Fit();

private:
  // fit related
  TrackFit::LineFit*		fLineFitAlg;
  ROOT::Fit::Fitter		fFitter;
  ROOT::Fit::FitResult	fFitResult;
  TrackFit::Line      fInitTrack;
  TrackFit::Line      fFinalTrack;

  // input/output
  TString   fFileNameInput;
  TFile*		fFileInput;
  TTree*		fTreeDriftTime;

  DriftInfo*  fDriftInfo;
  TSpline5*   fRtRelation_Spline[2][3];
  TGraph*		fRtRelation_Graph[2][3];
  TF1*		fRtRelation[2][3];

  UInt_t		fGid[2][3];
  Double_t	fTot[2][3];
  Double_t	fRawDriftTime[2][3];

  Double_t	fCorrectedDriftTime[2][3];
  Double_t	fDriftRadius[2][3];
  Double_t  fInitFittedDistances[2][3];
  Double_t  fFinalFittedDistances[2][3];
  TrackFit::Line fHittedWires[2][3];

  ClassDef(FitGUI,0);
};
#endif
