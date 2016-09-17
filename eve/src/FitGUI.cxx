#include "global.h"
#include <TGeoTube.h>
#include <TEveLine.h>
#include <TEveGeoShape.h>
#include <TEveTrans.h>
#include "FitGUI.h"
#include "TTree.h"
#include "TFile.h"
#include "TH1F.h"
#include "TSpline.h"
#include "TGraph.h"
#include "GeometryInfo.h"
#include "DriftInfo.h"
#include <Fit/Fitter.h>

ClassImp(FitGUI)
using namespace TrackFit;

FitGUI::FitGUI()
{
  fLineFitAlg = new LineFit(GeometryInfo());
}

FitGUI::FitGUI(const char* filename)
{
  fLineFitAlg = new LineFit(GeometryInfo());
  //
  SetFile(filename);
  //
  Init();
}

FitGUI::~FitGUI()
{
  delete fLineFitAlg;
  if (fFileInput)    delete fFileInput;
}

void FitGUI::SetFile(const char* filename)
{
  fFileNameInput = filename;
}

void FitGUI::Init()
{
  //open root file
  fFileInput = new TFile(fFileNameInput);
  if (!fFileInput) {
    PrintWarningMessage("Open File Error!");
    return;
  }

  //init drifttime tree
  fFileInput->GetObject("minrising_drifttime/minrising_drifttime", fTreeDriftTime);
  if (!fTreeDriftTime) {
    PrintWarningMessage("Drifttime tree not exist");
  }
  fTreeDriftTime->SetBranchAddress("gid", fGid);
  fTreeDriftTime->SetBranchAddress("tot", fTot);
  fTreeDriftTime->SetBranchAddress("drifttime", fRawDriftTime);
  fTotalEvents = fTreeDriftTime->GetEntries();

  //get t0 information
  fFileInput->GetObject("minrising_drifttime/init_edge_fitting_result", fDriftInfo);
  if (!fDriftInfo) {
    PrintWarningMessage("DriftInfo not exist");
  }

  //get init R-T relation
  const char hist[6][40] = {"h1d_minrising_drifttime_mwdc_Down_X_40"
                            , "h1d_minrising_drifttime_mwdc_Down_Y_40"
                            , "h1d_minrising_drifttime_mwdc_Down_U_50"
                            , "h1d_minrising_drifttime_mwdc_Up_X_40"
                            , "h1d_minrising_drifttime_mwdc_Up_Y_40"
                            , "h1d_minrising_drifttime_mwdc_Up_U_50"
  };
  for (int l = 0; l < g_mwdc_location; l++) {
    for (int p = 0; p < g_mwdc_wireplane; p++) {
      fFileInput->GetObject(Form("minrising_drifttime/trackfit_test/seed/grt_%s", hist[l * 3 + p]), fRtRelation_Graph[l][p]);
      fFileInput->GetObject(Form("minrising_drifttime/trackfit_test/seed/spline_%s", hist[l * 3 + p]), fRtRelation_Spline[l][p]);
    }
  }
}

void FitGUI::DropEvent()
{
  gEvtDisplay->Clean();
  gEvtDisplay->RemoveHightedCells();
}

Bool_t FitGUI::ReadEvent()
{
  fTreeDriftTime->GetEntry(fCurrentEvent);

  // first step: check event validity
  // broken wires
  UInt_t brokenwire_gid[12];
  // DownX
  brokenwire_gid[0] = Encoding::Encode(EMWDC, EDOWN, EX, 45);
  brokenwire_gid[1] = Encoding::Encode(EMWDC, EDOWN, EX, 46);
  brokenwire_gid[2] = Encoding::Encode(EMWDC, EDOWN, EX, 47);
  brokenwire_gid[3] = Encoding::Encode(EMWDC, EDOWN, EX, 74);
  brokenwire_gid[4] = Encoding::Encode(EMWDC, EDOWN, EX, 75);
  brokenwire_gid[5] = Encoding::Encode(EMWDC, EDOWN, EX, 76);
  // Down Y
  brokenwire_gid[6] = Encoding::Encode(EMWDC, EDOWN, EY, 31);
  brokenwire_gid[7] = Encoding::Encode(EMWDC, EDOWN, EY, 32);
  brokenwire_gid[8] = Encoding::Encode(EMWDC, EDOWN, EY, 47);
  // Down U
  brokenwire_gid[9] = Encoding::Encode(EMWDC, EDOWN, EU, 94);
  brokenwire_gid[10] = Encoding::Encode(EMWDC, EDOWN, EU, 95);
  brokenwire_gid[11] = Encoding::Encode(EMWDC, EDOWN, EU, 96);

  // don't use boundary wires
  UShort_t boundary_wire_low[g_mwdc_wireplane] = {3, 3, 6};
  UShort_t boundary_wire_high[g_mwdc_wireplane] = {77, 77, 100};

  Bool_t valid_event = true;
  UShort_t wireindex;
  const Double_t tot_limit=2775;// if a channel has only rising edge and no falling edge, we think its tot is beyond the limit.
  const Double_t drifttime_limt=3000;

  UChar_t l,p;
  for (l = 0; l < g_mwdc_location; l++) {
    for (p = 0; p < g_mwdc_wireplane; p++) {
      if (valid_event) {
        // whether invalid channel
        if (!Encoding::IsChannelValid(fGid[l][p])) {
          valid_event = false;
          break;
        }
        // whether boundary wires
        wireindex = Encoding::DecodeIndex(fGid[l][p]);
        if (wireindex < boundary_wire_low[p] || wireindex > boundary_wire_high[p]) {
          valid_event = false;
          break;
        }
        // whether broken wires
        for (int brokenwire_index = 0; brokenwire_index < 11; brokenwire_index++) {
          if (fGid[l][p] == brokenwire_gid[brokenwire_index]) {
            valid_event = false;
            break;
          }
        }
        // whether valid timing information
        if (fRawDriftTime[l][p] == drifttime_limt || fTot[l][p] == tot_limit) {
          valid_event = false;
          break;
        }
      }
      else {
        break;
      }
    }
  }

  Double_t t0,T0;
  TVector3 hitwireUpX,hitwireUpY,hitwireDownX,hitwireDownY,hitposUp,hitposDown;
  GeometryInfo gm_info = fLineFitAlg->GetGeometryInfo();
  Double_t p0_start[4];
  const Double_t* p0_stop;
  double p0_step=0.01;

  if(valid_event){
    // reset fLineFitAlg
    fLineFitAlg->Reset();

    // correct drifttime and get drift distance
    // And push them into fLineFitAlg
    for(l=0;l<g_mwdc_location;l++){
      for(p=0;p<g_mwdc_wireplane;p++){
        t0=fDriftInfo->Get_t0(fGid[l][p]);
        T0=fDriftInfo->Get_T0(fGid[l][p]);
        fCorrectedDriftTime[l][p] = fRawDriftTime[l][p] - (t0-2*T0);
        fDriftRadius[l][p] = fRtRelation_Spline[l][p]->Eval(fCorrectedDriftTime[l][p]);

        fHittedWires[Encoding::DecodeLocation(fGid[l][p])][Encoding::DecodeDirection(fGid[l][p])]
          =  fLineFitAlg->AddHit(fGid[l][p], fDriftRadius[l][p]);
      }
    }

    // get init track
    hitwireUpX=gm_info.GetPoint(fGid[1][0]);
    hitwireUpY=gm_info.GetPoint(fGid[1][1]);
    hitwireDownX=gm_info.GetPoint(fGid[0][0]);
    hitwireDownY=gm_info.GetPoint(fGid[0][1]);
    hitposUp.SetXYZ(hitwireUpY.X(),hitwireUpX.Y(),(hitwireUpY.Z()+hitwireUpX.Z())/2);
    hitposDown.SetXYZ(hitwireDownX.X(),hitwireDownY.Y(),(hitwireDownX.Z()+hitwireDownY.Z())/2);

    fInitTrack.Reset(hitposUp, hitposDown, false);
    fInitTrack.GetParameter(p0_start);

    // fitting
    fFitter.SetFCN(*fLineFitAlg,p0_start);
    for (int j = 0; j < 4; ++j)
      fFitter.Config().ParSettings(j).SetStepSize(p0_step);

    if (!fFitter.FitFCN()) {
      PrintWarningMessage("Line3D Fit failed");
    }

    // fit result
    fFitResult = fFitter.Result();
    p0_stop=fFitResult.GetParams();
    fFinalTrack.Reset(p0_stop);

    // get fitted distances
    std::map<UInt_t,Double_t>::const_iterator it;
    std::map<UInt_t, Double_t> fitted_distances;
    fLineFitAlg->CalcResiduals(p0_start);
    fitted_distances = fLineFitAlg->GetFittedDistances();
    for (it = fitted_distances.begin(); it != fitted_distances.end(); ++it) {
      UChar_t l,p;
      l=Encoding::DecodeLocation(it->first);
      p=Encoding::DecodeDirection(it->first);
      fInitFittedDistances[l][p] = it->second;
    }

    fLineFitAlg->CalcResiduals(p0_stop);
    fitted_distances = fLineFitAlg->GetFittedDistances();
    for (it = fitted_distances.begin(); it != fitted_distances.end(); ++it) {
      UChar_t l,p;
      l=Encoding::DecodeLocation(it->first);
      p=Encoding::DecodeDirection(it->first);
      fFinalFittedDistances[l][p] = it->second;
    }
  }
  else{
    return false;
  }
  return true;
}

void FitGUI::AddEvent()
{

  // hitted wire highlighting
  for(int l=0;l<2;l++){
    for(int p=0;p<3;p++){
      UChar_t type,location,direction;
      UShort_t index;
      Encoding::Decode(fGid[l][p],type,location,direction,index);
      TEveElement *cell=gEvtDisplay->GetCell(location,direction,index);
      gEvtDisplay->AddCellToHighlight(location,direction,index);

      TEveGeoShape* shape=dynamic_cast<TEveGeoShape*>(cell);
      TEveGeoShape* cell_clone=new TEveGeoShape(shape->GetName(),shape->GetTitle());
      cell_clone->SetShape(shape->GetShape());
      cell_clone->RefMainTrans().SetTrans(shape->RefMainTrans());
      cell_clone->CopyVizParams(shape);

      gEvtDisplay->AddTo3D(cell_clone);
    }
  }

  // init track
  TVector3 point;
  TEveLine* track_line = new TEveLine;
  track_line->SetMainColor(kBlack);
  // track_line->SetLineStyle(2);
  track_line->SetLineWidth(2);
  for(int i=0;i<400;i++){
    point=fInitTrack.GetPoint(-400+i*1400/400);
    track_line->SetNextPoint(point.X()/10,point.Y()/10,point.Z()/10);
  }
  gEvtDisplay->AddToAll(track_line);

  // final fitted track
  track_line = new TEveLine;
  track_line->SetMainColor(kBlue);
  track_line->SetLineWidth(2);
  for(int i=0;i<400;i++){
    point=fFinalTrack.GetPoint(-400+i*1400/400);
    track_line->SetNextPoint(point.X()/10,point.Y()/10,point.Z()/10);
  }
  gEvtDisplay->AddToAll(track_line);

  // drift circles
  TEveGeoManagerHolder gmgr(TEveGeoShape::GetGeoMangeur());// ref to TEveGeoShape.cxx
  TEveGeoShape* driftcircles[2][3];
  Double_t wire_angles[2][3]={{0,TMath::Pi()/2,TMath::Pi()/6},
                              {TMath::Pi()/2,0,TMath::Pi()/3}};
  for(int l=0;l<2;l++){
    for(int p=0;p<3;p++){
      driftcircles[l][p]=new TEveGeoShape(Form("drift_circle_%s_%s",g_str_location[l],g_str_plane[p]));
      driftcircles[l][p]->SetShape(new TGeoTube(0,fDriftRadius[l][p]/10,60));

      // tube rotation and translation
      driftcircles[l][p]->RefMainTrans().SetPos((fHittedWires[l][p].GetPoint(0).X())/10,(fHittedWires[l][p].GetPoint(0).Y())/10,fHittedWires[l][p].GetPoint(0).Z()/10);
      driftcircles[l][p]->RefMainTrans().SetRotByAngles(wire_angles[l][p],0,0);
      driftcircles[l][p]->RefMainTrans().RotateLF(2,3,TMath::Pi()/2);

      driftcircles[l][p]->SetNSegments(100);
      driftcircles[l][p]->SetMainTransparency(90);
      driftcircles[l][p]->SetMainColor(kGreen);
      driftcircles[l][p]->SetLineColor(kBlack);
    }
  }
  gEvtDisplay->AddToProjection(driftcircles);

  // coordinates guide
  gEvtDisplay->AddGuides();

  // table update
  gEvtDisplay->UpdateDriftRadius(fDriftRadius);
  gEvtDisplay->UpdateInitialFittedDistance(fInitFittedDistances);
  gEvtDisplay->UpdateFinalFittedDistance(fFinalFittedDistances);

  //
  gEvtDisplay->Draw();
}
