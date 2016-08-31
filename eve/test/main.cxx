#include <TRint.h>
#include "TApplication.h"
#include "TGApplication.h"
#include "EventDisplay.h"
#include "GeometryInfo.h"
#include "TrackFit.h"
#include "global.h"
#include <TVector3.h>
#include <TEveLine.h>
#include <TGeoTube.h>
#include <TEveGeoShape.h>
#include <TEveTrans.h>
#include "EventHandler.h"
#include <TEveSelection.h>
#include <TEveManager.h>

void add_debug_event()
{
  GeometryInfo gm_info;

  TrackFit::Line hitted_wire[2][3];
  UInt_t gid[2][3];
  for(int l=0;l<2;l++){
    for(int p=0;p<3;p++){
      gid[l][p]=Encoding::Encode(EMWDC,l,p,20+2*l+p*3);
      hitted_wire[l][p].Reset_Unstandard(gm_info.GetPoint(gid[l][p]),gm_info.GetDirection(gid[l][p]));
    }
  }

  TrackFit::Line track;
  TVector3 hitwireUpX,hitwireUpY,hitwireDownX,hitwireDownY,hitposUp,hitposDown;
  
  hitwireUpX=gm_info.GetPoint(gid[1][0]);
  hitwireUpY=gm_info.GetPoint(gid[1][1]);
  hitwireDownX=gm_info.GetPoint(gid[0][0]);
  hitwireDownY=gm_info.GetPoint(gid[0][1]);
  hitposUp.SetXYZ(hitwireUpY.X(),hitwireUpX.Y(),(hitwireUpY.Z()+hitwireUpX.Z())/2);
  hitposDown.SetXYZ(hitwireDownX.X(),hitwireDownY.Y(),(hitwireDownX.Z()+hitwireDownY.Z())/2);
  
  track.Reset(hitposUp,hitposDown,false);

  TrackFit::Line track2;
  hitposUp.SetXYZ(hitwireUpY.X()+100,hitwireUpX.Y()+300,(hitwireUpY.Z()+hitwireUpX.Z())/2+200);
  hitposDown.SetXYZ(hitwireDownX.X()+10,hitwireDownY.Y()+30,(hitwireDownX.Z()+hitwireDownY.Z())/2+20);
  track2.Reset(hitposUp,hitposDown,false);

  // track
  TVector3 point;
  TEveLine* track_line = new TEveLine;
  track_line->SetMainColor(kBlack);
  track_line->SetLineWidth(2);
  for(int i=0;i<400;i++){
    point=track.GetPoint(-400+i*1400/400);
    track_line->SetNextPoint(point.X()/10,point.Y()/10,point.Z()/10);
  }
  // track_line->SetRnrSelf(kFALSE);

  gEvtDisplay->AddToAll(track_line);

  // hitted wire
  TEveLine* hitted_wire_line[2][3];
  for(int l=0;l<2;l++){
    for(int p=0;p<3;p++){
      hitted_wire_line[l][p]=new TEveLine;
      hitted_wire_line[l][p]->SetMainColor(kGray);
      // hitted_wire_line[l][p]->SetLineStyle(3);
    
      point=hitted_wire[l][p].GetPoint(-500);
      hitted_wire_line[l][p]->SetNextPoint(point.X()/10,point.Y()/10,point.Z()/10);
      point=hitted_wire[l][p].GetPoint(1000);
      hitted_wire_line[l][p]->SetNextPoint(point.X()/10,point.Y()/10,point.Z()/10);         
    }
  }
  gEvtDisplay->AddToProjection(hitted_wire_line);


  TEveGeoShape* driftcircles[2][3];
  Double_t drift_radius[2][3];
  Double_t wire_angles[2][3]={{0,TMath::Pi()/2,TMath::Pi()/6},
                {TMath::Pi()/2,0,TMath::Pi()/3}}; 
  for(int l=0;l<2;l++){
    for(int p=0;p<3;p++){
      drift_radius[l][p]= hitted_wire[l][p].DistanceToLine(track)/10;
      
      // 
      driftcircles[l][p]=new TEveGeoShape(Form("drift_circle_%s_%s",g_str_location[l],g_str_plane[p]));
      driftcircles[l][p]->SetShape(new TGeoTube(0,drift_radius[l][p],60));
      
      // tube rotation and translation
      driftcircles[l][p]->RefMainTrans().SetPos((hitted_wire[l][p].GetPoint(0).X())/10,(hitted_wire[l][p].GetPoint(0).Y())/10,hitted_wire[l][p].GetPoint(0).Z()/10);
      driftcircles[l][p]->RefMainTrans().SetRotByAngles(wire_angles[l][p],0,0);
      driftcircles[l][p]->RefMainTrans().RotateLF(2,3,TMath::Pi()/2);

      
      driftcircles[l][p]->SetNSegments(100);
      driftcircles[l][p]->SetMainTransparency(90);
      driftcircles[l][p]->SetMainColor(kGreen);
      driftcircles[l][p]->SetLineColor(kBlack);
      // // driftcircles[l][p]->SetLineWidth(2);
      
    }
  }
  gEvtDisplay->AddToProjection(driftcircles);

  // hitted wire highlighting 
  for(int l=0;l<2;l++){
    for(int p=0;p<3;p++){
      UChar_t type,location,direction;
      UShort_t index;
      Encoding::Decode(gid[l][p],type,location,direction,index);
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

  gEvtDisplay->AddGuides();

  // table update
  Double_t newvalue[2][3]={{1,2,3},{4,5,6}};
  gEvtDisplay->UpdateDriftRadius(newvalue);
  gEvtDisplay->UpdateFinalFittedDistance(newvalue);

  gEvtDisplay->Draw();
}

int main(int argc, char **argv)
{
  TString filename(argv[1]);
   // printf("%s\n",argv[1]);
  // Use TApplication if you don't need prompt.
  TRint  *app = new TRint("App", &argc, argv);
  // TGApplication  *app = new TGApplication("App", &argc, argv);
  
  // See arguments to Create() and constructor -- you can choose not to show the window
  // or some GUI parts.
  
  EventDisplay::Create();
  
  if(gEvtDisplay->ImportGeometry(filename.Data())){

    EventHandler *handler=new EventHandler();
    gEvtDisplay->SetEventHandler(handler); 
    gEvtDisplay->Initialize();

    add_debug_event();

    app->Run(kTRUE);
    // // Pass kFALSE if you want application to terminate by itself.
    // Then you just need "return 0;" below (to avoid compiler warnings).

  }
  // Create custom GUI, if needed.

  
  // Optionally shutdown eve here (not really needed):
  // TEveManager::Terminate();
  app->Terminate(0);

  // delete handler;

  return 0;
}