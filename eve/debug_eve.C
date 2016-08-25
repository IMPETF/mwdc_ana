#include "TrackFit.h"
#include "global.h"
#include "TEveManager.h"
#include "TEveGeoNode.h"
#include "TEveLine.h"
#include "TEveGeoShapeExtract.h"
#include "TEveGeoShape.h"
#include "TEveSelection.h"
#include "TEveTrans.h"
#include "TEveText.h"
#include "TEveArrow.h"
#include "TMath.h"
#include "TString.h"
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


TEveElement* get_cell(TEveElement* mwdc, Int_t l, Int_t p, Int_t index)
{
	const char name[3][3]={"x","y","u"};

	TString plane_name=TString::Format("%s_plane_%d",name[p],l);
	TEveElement* plane=mwdc->FindChild(plane_name);
	// printf("%s: %d\n",plane_name.Data(),plane );

	TString chamber_name;
	if(p==2){
		switch(l){
			case 0:
				chamber_name=TString::Format("u_chamber%d_0",105-index);
				break;
			case 1:
				chamber_name=TString::Format("u_chamber%d_0",index);
				break;
		}
	}
	else{
		switch(l){
			case 0:
				chamber_name=TString::Format("%s_cell_%d",name[p],index+1);
				break;
			case 1:
				chamber_name=TString::Format("%s_cell_%d",name[p],80-index);
				break;
		}
	}
	TEveElement* chamber=plane->FindChild(chamber_name);
	// printf("%d\n", chamber);

	return chamber;
}

TEveElement* get_wire(TEveElement* mwdc, Int_t l, Int_t p, Int_t index)
{
	const char name[3][3]={"x","y","u"};

	TString plane_name=TString::Format("%s_plane_%d",name[p],l);
	TEveElement* plane=mwdc->FindChild(plane_name);
	// printf("%s: %d\n",plane_name.Data(),plane );

	TString chamber_name;
	if(p==2){
		switch(l){
			case 0:
				chamber_name=TString::Format("u_chamber%d_0",105-index);
				break;
			case 1:
				chamber_name=TString::Format("u_chamber%d_0",index);
				break;
		}
	}
	else{
		switch(l){
			case 0:
				chamber_name=TString::Format("%s_cell_%d",name[p],index+1);
				break;
			case 1:
				chamber_name=TString::Format("%s_cell_%d",name[p],80-index);
				break;
		}
	}
	TEveElement* chamber=plane->FindChild(chamber_name);
	// printf("%d\n", chamber);
	
	TEveElement* wire=chamber->FirstChild();

	return wire;
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

void add_debug_event(TEveElement* mwdc)
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

	gEve->AddElement(track_line);
	gMultiView->ImportEventXOZ(track_line);
	gMultiView->ImportEventYOZ(track_line);
	gMultiView->ImportEventUpUOZ(track_line);
	gMultiView->ImportEventDownUOZ(track_line);
	gMultiView->ImportEvent3D(track_line);

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
		
			gEve->AddElement(hitted_wire_line[l][p]);
			gMultiView->ImportEvent3D(hitted_wire_line[l][p]);
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
			printf("raidus[%d][%d]=%.7f\n",l,p,drift_radius[l][p]);
			
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
			
			// add into event scene
			gEve->AddElement(driftcircles[l][p]);
			gMultiView->ImportEvent3D(driftcircles[l][p]);
		}
	}
	gMultiView->ImportEventXOZ(driftcircles[0][0]);
	gMultiView->ImportEventXOZ(driftcircles[1][1]);	

	gMultiView->ImportEventYOZ(driftcircles[0][1]);
	gMultiView->ImportEventYOZ(driftcircles[1][0]);

	gMultiView->ImportEventUpUOZ(driftcircles[1][2]);
	gMultiView->ImportEventDownUOZ(driftcircles[0][2]);

	// hitted wire highlighting 
	for(int l=0;l<2;l++){
		for(int p=0;p<3;p++){
			UChar_t type,location,direction;
			UShort_t index;
			Encoding::Decode(gid[l][p],type,location,direction,index);
			TEveElement *cell=get_cell(mwdc,location,direction,index);
			cell->HighlightElement(kTRUE);
			// cell->SelectElement(kTRUE);

			TEveGeoShape* shape=dynamic_cast<TEveGeoShape*>(cell);
			TEveGeoShape* cell_clone=new TEveGeoShape(shape->GetName(),shape->GetTitle());
			cell_clone->SetShape(shape->GetShape());
			cell_clone->RefMainTrans().SetTrans(shape->RefMainTrans());
			cell_clone->CopyVizParams(shape);
			gMultiView->ImportEvent3D(cell_clone);
		}
	}
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

	// 
	// TEveElement* mwdc_clone=mwdc->CloneElementRecurse(-1);
	// mwdc->SetRnrSelf(kFALSE);
	gEve->AddGlobalElement(mwdc);
	
	// add text and arrows
	Double_t t_pos[3];
	TEveText* t_up=new TEveText("Up");
	t_up->SetFontSize(20);t_up->SetMainColor(kBlue);
   	TEveElement* x_up=get_cell(mwdc,1,0,39);
   	x_up->RefMainTrans().GetPos(t_pos);
   	t_pos[0]-=70;t_pos[2]+=10;
   	t_up->RefMainTrans().SetPos(t_pos);
   	gEve->AddGlobalElement(t_up);
   	
   	Double_t t_pos_down[3];
   	TEveText* t_down=new TEveText("Down");
	t_down->SetFontSize(20);t_down->SetMainColor(kBlue);
   	TEveElement* x_down=get_cell(mwdc,0,0,39);
   	x_down->RefMainTrans().GetPos(t_pos_down);
   	t_pos_down[0]-=70;t_pos_down[2]-=10;
   	t_down->RefMainTrans().SetPos(t_pos_down);
   	gEve->AddGlobalElement(t_down);

   	TEveArrow* ax = new TEveArrow(15., 0., 0., t_pos[0], t_pos[1], (t_pos[2]+t_pos_down[2])/2);
   	ax->SetMainColor(kRed);ax->SetTubeR(0.05);ax->SetConeR(0.15);ax->SetConeL(0.2);
   	ax->SetPickable(kTRUE);
   	gEve->AddGlobalElement(ax);
   	TEveText* tx=new TEveText("+X");
	tx->SetFontSize(15);tx->SetMainColor(kRed);
   	tx->RefMainTrans().SetPos(t_pos[0]+15,t_pos[1],(t_pos[2]+t_pos_down[2])/2);
   	gEve->AddGlobalElement(tx);

   	TEveArrow* ay = new TEveArrow(0., 15., 0., t_pos[0], t_pos[1], (t_pos[2]+t_pos_down[2])/2);
   	ay->SetMainColor(kGreen);ay->SetTubeR(0.05);ay->SetConeR(0.15);ay->SetConeL(0.2);
   	ay->SetPickable(kTRUE);
   	gEve->AddGlobalElement(ay);
   	TEveText* ty=new TEveText("+Y");
	ty->SetFontSize(15);ty->SetMainColor(kGreen);
   	ty->RefMainTrans().SetPos(t_pos[0],t_pos[1]+15,(t_pos[2]+t_pos_down[2])/2);
   	gEve->AddGlobalElement(ty);

	// multiview
	gMultiView = new MultiView;
	// gMultiView->DisablePreScale();	

	gMultiView->SetDepth(-10);
	gMultiView->ImportGeomXOZ(mwdc);
	gMultiView->ImportGeomYOZ(mwdc);
	gMultiView->ImportGeomUpUOZ(mwdc);
	gMultiView->ImportGeomDownUOZ(mwdc);
	gMultiView->SetDepth(0);

	gMultiView->ImportEvent3D(t_up);
	gMultiView->ImportEvent3D(t_down);
	gMultiView->ImportEvent3D(tx);
	gMultiView->ImportEvent3D(ax);
	gMultiView->ImportEvent3D(ty);
	gMultiView->ImportEvent3D(ay);
	
	make_gui();

	// add event
	add_debug_event(mwdc);

	// draw
	gEve->GetViewers()->SwitchColorSet();
	gEve->GetDefaultGLViewer()->SetStyle(TGLRnrCtx::kOutline);
	gEve->GetDefaultGLViewer()->SetGuideState(TGLUtil::kAxesEdge, kTRUE, kFALSE, 0);

	gEve->GetBrowser()->GetTabRight()->SetTab(1);

	// 
	// TGLCameraOverlay* co = gEve->GetDefaultGLViewer()->GetCameraOverlay();
	// co->SetShowOrthographic(kTRUE);
	// co->SetOrthographicMode(TGLCameraOverlay::kGridFront);

	gEve->Redraw3D();

}

