#include "TString.h"
#include "TGeoManager.h"
#include "TGeoNode.h"
#include "TGeoTube.h"
#include "TGLViewer.h"
#include "TEveGeoShapeExtract.h"
#include "TEveGeoShape.h"

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