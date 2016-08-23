#include "TrackFit.h"
#include "global.h"
#include "TEveManager.h"
#include "TEveGeoNode.h"
#include "TEveLine.h"
#include "TEveGeoShapeExtract.h"
#include "TEveGeoShape.h"
#include "TEveSelection.h"
#include "TEveTrans.h"
#include "TMath.h"
#include "TString.h"
#include "TGeoManager.h"
#include "TGeoNode.h"
#include "TGeoTube.h"
#include "TGLViewer.h"
#include "TGLCameraOverlay.h"
#include "TGTab.h"
#include "TEveBrowser.h"
#include "TFile.h"
#include "TSystem.h"
#include "TGButton.h"
#include <iostream>

// Implemented in MultiView.C
#include "MultiView.C"
MultiView* gMultiView = 0;

// debug selection/highlight
class SigTestSpitter
{
   TEveSelection *fSel;
   TString        fPrefix;

public:
   SigTestSpitter(TEveSelection* sel, const TString& prefix) :
      fSel(sel), fPrefix(prefix)
   {
      fSel->Connect("SelectionAdded(TEveElement*)", "SigTestSpitter", this, "Added(TEveElement*)");
      fSel->Connect("SelectionRemoved(TEveElement*)", "SigTestSpitter", this, "Removed(TEveElement*)");
      fSel->Connect("SelectionCleared()", "SigTestSpitter", this, "Cleared()");
   }
   ~SigTestSpitter()
   {
      fSel->Disconnect("SelectionAdded(TEveElement*)", this, "Added(TEveElement*)");
      fSel->Disconnect("SelectionRemoved(TEveElement*)", this, "Removed(TEveElement*)");
      fSel->Disconnect("SelectionCleared()", this, "Cleared()");
   }
   // ----------------------------------------------------------------
   void Added(TEveElement* el)
   {
      printf("%s Added 0x%lx '%s'\n", fPrefix.Data(), el, el ? el->GetElementName() : "");
   }
   void Removed(TEveElement* el)
   {
      printf("%s Removed 0x%lx '%s'\n", fPrefix.Data(), el, el ? el->GetElementName() : "");
   }
   void Cleared()
   {
      printf("%s Cleared'\n", fPrefix.Data());
   }
};


void turnoff_wire()
{
	const TString kEH("turnoff_wire");
	// 
	Bool_t s = gGeoManager->cd("/x_plane_0/x_cell_1/x_wire_0");
	if (!s) {
	   Error(kEH, "Start node not found.");
	   return;
	}
	TGeoNode *x_wrie = gGeoManager->GetCurrentNode();
	x_wrie->GetVolume()->SetVisibility(kFALSE);

	s = gGeoManager->cd("/y_plane_0/y_cell_1/y_wire_0");
	if (!s) {
	   Error(kEH, "Start node not found.");
	   return;
	}
	TGeoNode *y_wrie = gGeoManager->GetCurrentNode();
	y_wrie->GetVolume()->SetVisibility(kFALSE);

	for(int i=0;i<106;i++){
		s = gGeoManager->cd(Form("/u_plane_0/u_chamber%d_0/u_wire%d_0",i,i));
		if (!s) {
		   Error(kEH, "Start node not found.");
		   return;
		}
		TGeoNode *u_wrie=gGeoManager->GetCurrentNode();
		u_wrie->GetVolume()->SetVisibility(kFALSE);	
	}
}

void turnoff_chamber()
{
	const TString kEH("turnoff_chamber");

	Bool_t s = gGeoManager->cd("/x_plane_0/x_cell_1");
	if (!s) {
		Error(kEH, "Start node not found.");
		return;
	}
	TGeoNode *x_cell = gGeoManager->GetCurrentNode();
	x_cell->GetVolume()->SetVisibility(kFALSE);

	s = gGeoManager->cd("/y_plane_0/y_cell_1");
	if (!s) {
		Error(kEH, "Start node not found.");
		return;
	}
	TGeoNode *y_cell = gGeoManager->GetCurrentNode();
	y_cell->GetVolume()->SetVisibility(kFALSE);

	for(int i=0;i<106;i++){
		s = gGeoManager->cd(Form("/u_plane_0/u_chamber%d_0",i,i));
		if (!s) {
			Error(kEH, "Start node not found.");
			return;
		}
		TGeoNode *u_chamber=gGeoManager->GetCurrentNode();
		u_chamber->GetVolume()->SetVisibility(kFALSE);	
	}
}

void extract_geom(int flag)
{
	const char filename[40]="mwdc_wire.root";
	TString outfilename;

	TEveManager::Create();

	gGeoManager= gEve->GetGeometry(filename);
	gGeoManager->DefaultColors();
	TGeoNode* node = gGeoManager->GetTopNode();
	TEveGeoTopNode* top = new TEveGeoTopNode(gGeoManager, node);

	top->SetVisOption(0);
	top->SetVisLevel(3);
	top->GetNode()->GetVolume()->SetVisibility(kFALSE);

	gEve->AddGlobalElement(top);

	// container invisible
	TGeoNode* x_plane = gGeoManager->GetTopVolume()->FindNode("x_plane_0");
	x_plane->GetVolume()->SetVisibility(kFALSE);

	TGeoNode* y_plane = gGeoManager->GetTopVolume()->FindNode("y_plane_0");
	y_plane->GetVolume()->SetVisibility(kFALSE);

	switch(flag){
		case 0:
		{
			outfilename="mwdc_extract_full.root";
			break;
		}
		case 1:
		{
			outfilename="mwdc_extract_chamber.root";
			turnoff_wire();
			break;
		}
		case 2:
		{
			outfilename="mwdc_extract_wire.root";
			turnoff_chamber();
			break;
		}
		default:
			break;
	}

	top->ExpandIntoListTreesRecursively();
	top->SaveExtract(outfilename, "mwdc",kFALSE);
}

void make_gui()
{
   // Create minimal GUI for event navigation.

   TEveBrowser* browser = gEve->GetBrowser();
   browser->StartEmbedding(TRootBrowser::kLeft);

   TGMainFrame* frmMain = new TGMainFrame(gClient->GetRoot(), 1000, 600);
   frmMain->SetWindowName("XX GUI");
   frmMain->SetCleanup(kDeepCleanup);

   TGHorizontalFrame* hf = new TGHorizontalFrame(frmMain);
   {
      TString icondir(TString::Format("%s/icons/", gSystem->Getenv("ROOTSYS")));
      TGTextButton* b = 0;

      b = new TGTextButton(hf, "EnableScale");
      hf->AddFrame(b);
      b->Connect("Clicked()", "MultiView", gMultiView, "EnablePreScale()");

      b = new TGTextButton(hf, "DisableScale");
      hf->AddFrame(b);
      b->Connect("Clicked()", "MultiView", gMultiView, "DisablePreScale()");
   }
   frmMain->AddFrame(hf);

   frmMain->MapSubwindows();
   frmMain->Resize();
   frmMain->MapWindow();

   browser->StopEmbedding();
   browser->SetTabTitle("Event Control", 0);
}

void add_debug_event()
{
	GeometryInfo gm_info;

	TrackFit::Line hitted_wire[2][3];
	UInt_t gid[2][3];
	for(int l=0;l<2;l++){
		for(int p=0;p<3;p++){
			gid[l][p]=Encoding::Encode(EMWDC,l,p,20);
			hitted_wire[l][p].Reset_Unstandard(gm_info.GetPoint(gid[l][p]),gm_info.GetDirection(gid[l][p]));
		}
	}

	TrackFit::Line track;
	TVector3 hitwireUpX,hitwireUpY,hitwireDownX,hitwireDownY,hitposUp,hitposDown;
	
	hitwireUpX=gm_info.GetPoint(gid[1][0]);
	hitwireUpY=gm_info.GetPoint(gid[1][1]);
	hitwireDownX=gm_info.GetPoint(gid[0][0]);
	hitwireDownY=gm_info.GetPoint(gid[0][1]);
	hitposUp.SetXYZ(hitwireUpY.X()-200,hitwireUpX.Y()-105,(hitwireUpY.Z()+hitwireUpX.Z())/2);
	hitposDown.SetXYZ(hitwireDownX.X()+450,hitwireDownY.Y()+405,(hitwireDownX.Z()+hitwireDownY.Z())/2);
	
	track.Reset(hitposUp,hitposDown,false);
	// track.Reset_Unstandard(hitwireUpY,TVector3(0,1,0));

	// track
	TVector3 point;
	TEveLine* track_line = new TEveLine;
	track_line->SetMainColor(kBlue);
	
	// point=track.GetPoint(-300);
	// track_line->SetNextPoint(point.X()/10,point.Y()/10,point.Z()/10);
	// point=track.GetPoint(900);
	// track_line->SetNextPoint(point.X()/10,point.Y()/10,point.Z()/10);
	for(int i=0;i<400;i++){
		point=track.GetPoint(-400+i*1400/400);
		track_line->SetNextPoint(point.X()/10,point.Y()/10,point.Z()/10);
	}
	// track_line->SetRnrSelf(kFALSE);

	gEve->AddElement(track_line);
	gMultiView->ImportEventXOZ(track_line);
	gMultiView->ImportEventYOZ(track_line);
	gMultiView->ImportEventUpUOZ(track_line);
	gMultiView->ImportEventDownUOZ(track_line);
	
	// hitted wire
	TEveLine* hitted_wire_line[2][3];
	for(int l=0;l<2;l++){
		for(int p=0;p<3;p++){
			hitted_wire_line[l][p]=new TEveLine;
			hitted_wire_line[l][p]->SetMainColor(kRed);
		
			point=hitted_wire[l][p].GetPoint(-500);
			hitted_wire_line[l][p]->SetNextPoint(point.X()/10,point.Y()/10,point.Z()/10);
			point=hitted_wire[l][p].GetPoint(1000);
			hitted_wire_line[l][p]->SetNextPoint(point.X()/10,point.Y()/10,point.Z()/10);			
		
			gEve->AddElement(hitted_wire_line[l][p]);
			// gMultiView->ImportEventXOZ(hitted_wire_line[l][p]);
			// gMultiView->ImportEventYOZ(hitted_wire_line[l][p]);
			// gMultiView->ImportEventUpUOZ(hitted_wire_line[l][p]);
			// gMultiView->ImportEventDownUOZ(hitted_wire_line[l][p]);
		}
	}
	gMultiView->ImportEventXOZ(hitted_wire_line[0][0]);
	gMultiView->ImportEventXOZ(hitted_wire_line[1][1]);	

	gMultiView->ImportEventYOZ(hitted_wire_line[0][1]);
	gMultiView->ImportEventYOZ(hitted_wire_line[1][0]);

	gMultiView->ImportEventUpUOZ(hitted_wire_line[1][2]);
	gMultiView->ImportEventDownUOZ(hitted_wire_line[0][2]);


	TEveGeoShape* driftcircles[2][3];
	Double_t drift_radius[2][3];
	Double_t wire_angles[2][3]={{0,TMath::Pi()/2,TMath::Pi()/6},
								{TMath::Pi()/2,0,TMath::Pi()/3}};	
	for(int l=0;l<2;l++){
		for(int p=0;p<3;p++){
			drift_radius[l][p]= hitted_wire[l][p].DistanceToLine(track)/10;
			printf("raidus[%d][%d]=%.4f\n",l,p,drift_radius[l][p]);
			driftcircles[l][p]=new TEveGeoShape(Form("drift_circle_%s_%s",g_str_location[l],g_str_plane[p]));
			driftcircles[l][p]->SetShape(new TGeoTube(0,drift_radius[l][p],60));
			driftcircles[l][p]->RefMainTrans().SetPos((hitted_wire[l][p].GetPoint(0).X())/10,(hitted_wire[l][p].GetPoint(0).Y())/10,hitted_wire[l][p].GetPoint(0).Z()/10);
			driftcircles[l][p]->RefMainTrans().SetRotByAngles(wire_angles[l][p],0,0);
			driftcircles[l][p]->RefMainTrans().RotateLF(2,3,TMath::Pi()/2);
			
			driftcircles[l][p]->SetNSegments(100);
			driftcircles[l][p]->SetMainTransparency(90);
			driftcircles[l][p]->SetMainColor(kGreen);
			driftcircles[l][p]->SetLineColor(kBlack);
			// driftcircles[l][p]->SetLineWidth(2);

			gEve->AddElement(driftcircles[l][p]);
			// switch(p){
			// 	case 0:
			// 	{
			// 		if(l==0){

			// 		}
			// 		break;
			// 	}
			// }
			// gMultiView->ImportEventXOZ(driftcircles[l][p]);
			// gMultiView->ImportEventYOZ(driftcircles[l][p]);
			// gMultiView->ImportEventUpUOZ(driftcircles[l][p]);
			// gMultiView->ImportEventDownUOZ(driftcircles[l][p]);
		}
	}
	gMultiView->ImportEventXOZ(driftcircles[0][0]);
	gMultiView->ImportEventXOZ(driftcircles[1][1]);	

	gMultiView->ImportEventYOZ(driftcircles[0][1]);
	gMultiView->ImportEventYOZ(driftcircles[1][0]);

	gMultiView->ImportEventUpUOZ(driftcircles[1][2]);
	gMultiView->ImportEventDownUOZ(driftcircles[0][2]);
}

void debug_multiview()
{
	// load multiview in compiled mode
	// if (gROOT->LoadMacro("MultiView.C+") != 0)
	// {
	//    Error("turnoff_wire()", "Failed loading MultiView.C in compiled mode.");
	//    return;
	// }

	TEveManager::Create();

	// debug selection/highlight
	// new SigTestSpitter(gEve->GetSelection(), "Selection");
   	// new SigTestSpitter(gEve->GetHighlight(), "Highlight");
	
	// import geometry
	const char filename[40]="mwdc_extract_chamber.root";
	TFile* file=new TFile(filename);
	TEveGeoShapeExtract* gse = (TEveGeoShapeExtract*)file->Get("mwdc");
	TEveGeoShape* mwdc=TEveGeoShape::ImportShapeExtract(gse,0);
	delete file;
	
	
	TEveElement* x_plane=mwdc->FindChild("x_plane_0");
	TEveElement* x_cell=x_plane->FindChild("x_cell_40");
	TEveTrans& t = x_plane->RefMainTrans();
	std::cout<<x_plane->GetElementName()<<std::endl;
	x_cell->HighlightElement(kTRUE);
	// x_cell->SelectElement(kTRUE);
	
	// 
	gEve->AddGlobalElement(mwdc);
	

	// multiview
	gMultiView = new MultiView;
	gMultiView->DisablePreScale();	

	gMultiView->SetDepth(-10);
	gMultiView->ImportGeomXOZ(mwdc);
	gMultiView->ImportGeomYOZ(mwdc);
	gMultiView->ImportGeomUpUOZ(mwdc);
	gMultiView->ImportGeomDownUOZ(mwdc);
	gMultiView->SetDepth(0);

	make_gui();

	// add event
	add_debug_event();

	// draw
	gEve->GetViewers()->SwitchColorSet();
	gEve->GetDefaultGLViewer()->SetStyle(TGLRnrCtx::kOutline);

	gEve->GetBrowser()->GetTabRight()->SetTab(1);

	// 
	TGLCameraOverlay* co = gEve->GetDefaultGLViewer()->GetCameraOverlay();
	co->SetShowOrthographic(kTRUE);
	co->SetOrthographicMode(TGLCameraOverlay::kGridFront);

	gEve->Redraw3D(kTRUE);

}

void debug_eve()
{
	// load multiview in compiled mode
	// if (gROOT->LoadMacro("MultiView.C+") != 0)
	// {
	//    Error("turnoff_wire()", "Failed loading MultiView.C in compiled mode.");
	//    return;
	// }

	TEveManager::Create();

	// debug selection/highlight
	new SigTestSpitter(gEve->GetSelection(), "Selection");
   	new SigTestSpitter(gEve->GetHighlight(), "Highlight");

   	// import geometry 
	const char filename[40]="mwdc_wire.root";

	gGeoManager= gEve->GetGeometry(filename);
	gGeoManager->DefaultColors();
	// 
	TGeoNode* node = gGeoManager->GetTopNode();
	// TGeoNode* node = gGeoManager->GetTopVolume()->FindNode("TPC_M_1");

	TEveGeoTopNode* top = new TEveGeoTopNode(gGeoManager, node);
	top->SetVisOption(0);
	top->SetVisLevel(3);
	top->GetNode()->GetVolume()->SetVisibility(kFALSE);
	// en->GetNode()->GetVolume()->SetTransparency(0);

	// container invisible
	TGeoNode* x_plane = gGeoManager->GetTopVolume()->FindNode("x_plane_0");
	x_plane->GetVolume()->SetVisibility(kFALSE);

	TGeoNode* y_plane = gGeoManager->GetTopVolume()->FindNode("y_plane_0");
	y_plane->GetVolume()->SetVisibility(kFALSE);
	
	// wire invisible 
	turnoff_wire();

	// chamber invisible
	// turnoff_chamber();
	
	// 
	gEve->AddGlobalElement(top);

	// multiview
	gMultiView = new MultiView;

	gMultiView->SetDepth(-10);
	gMultiView->ImportGeomUpUOZ(top);
	gMultiView->ImportGeomDownUOZ(top);
	gMultiView->SetDepth(0);

	// draw
	gEve->GetViewers()->SwitchColorSet();
	gEve->GetDefaultGLViewer()->SetStyle(TGLRnrCtx::kOutline);

	gEve->GetBrowser()->GetTabRight()->SetTab(2);

	gEve->Redraw3D(kTRUE);	

	// en->ExpandIntoListTreesRecursively();
	// en->SaveExtract("mwdc_cell_extract.root", "mwdc_cell",kFALSE);
}

void debug_eve_extract()
{
	TEveManager::Create();

	const char filename[40]="mwdc_cell_extract.root";

	TFile* file=new TFile(filename);
	TEveGeoShapeExtract* gse = (TEveGeoShapeExtract*) file->Get("mwdc_cell");
	TEveGeoShape* mwdc=TEveGeoShape::ImportShapeExtract(gse,0);
	delete file;

	
	gEve->AddGlobalElement(mwdc);

	gEve->Redraw3D(kTRUE);
}