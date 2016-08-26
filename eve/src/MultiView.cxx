#include "MultiView.h"

/////////////
// MultiView
////////////
ClassImp(MultiView)

MultiView::MultiView()
{
	// Constructor --- creates required scenes, projection managers
	// and GL viewers.

	// Scenes
	//========
	fXOZGeomScene  = gEve->SpawnNewScene("XOZ Geometry",
	                                     "Scene holding projected geometry for the XOZ view.");
	fYOZGeomScene  = gEve->SpawnNewScene("YOZ Geometry",
	                                     "Scene holding projected geometry for the YOZ view.");
	fUpUOZGeomScene  = gEve->SpawnNewScene("UpUOZ Geometry",
	                                       "Scene holding projected geometry for the UpUOZ view.");
	fDownUOZGeomScene  = gEve->SpawnNewScene("DownUOZ Geometry",
	                     "Scene holding projected geometry for the DownUOZ view.");

	f3DEventScene = gEve->SpawnNewScene("Simplified 3D Event Data",
	                                    "Scene holding projected event-data for the simplified geometry.");
	fXOZEventScene = gEve->SpawnNewScene("XOZ Event Data",
	                                     "Scene holding projected event-data for the XOZ view.");
	fYOZEventScene = gEve->SpawnNewScene("YOZ Event Data",
	                                     "Scene holding projected event-data for the YOZ view.");
	fUpUOZEventScene = gEve->SpawnNewScene("UpUOZ Event Data",
	                                       "Scene holding projected event-data for the UpUOZ view.");
	fDownUOZEventScene = gEve->SpawnNewScene("DownUOZ Event Data",
	                     "Scene holding projected event-data for the DownUOZ view.");

	// Basic Parameters
	//=====================
	Double_t up_z = 74.3, down_z = -14.9;
	Double_t thickness = 5;
	Double_t z_display = (up_z + down_z) / 2; //origin displacement
	TEveVector displace(0, 0, z_display);
	lrange = up_z - z_display - 1;
	hrange = lrange + 7;


	// Projection managers
	//=====================
	fXOZMgr = new TEveProjectionManager(TEveProjection::kPT_XOZ);
	gEve->AddToListTree(fXOZMgr, kFALSE);
	{
		// displace origin z-center-ed
		TEveProjection* p = fXOZMgr->GetProjection();
		p->SetDisplaceOrigin(kTRUE);
		p->SetCenter(displace);

		// fish-eye transformation zoom out the gap between the up and down mwdc
		UsePreScaleXOZ(lrange, hrange);

		// add axises
		TEveProjectionAxes* a = new TEveProjectionAxes(fXOZMgr);
		a->SetMainColor(kWhite);
		a->SetTitle("X-Z");
		a->SetTitleSize(0.05);
		a->SetTitleFont(102);
		a->SetLabelSize(0.025);
		a->SetLabelFont(102);
		fXOZGeomScene->AddElement(a);
	}

	fYOZMgr = new TEveProjectionManager(TEveProjection::kPT_YOZ);
	gEve->AddToListTree(fYOZMgr, kFALSE);
	{
		// displace origin z-center-ed
		TEveProjection* p = fYOZMgr->GetProjection();
		p->SetDisplaceOrigin(kTRUE);
		p->SetCenter(displace);

		// fish-eye transformation zoom out the gap between the up and down mwdc
		UsePreScaleYOZ(lrange, hrange);

		// add axises
		TEveProjectionAxes* a = new TEveProjectionAxes(fYOZMgr);
		a->SetMainColor(kWhite);
		a->SetTitle("Y-Z");
		a->SetTitleSize(0.05);
		a->SetTitleFont(102);
		a->SetLabelSize(0.025);
		a->SetLabelFont(102);
		fYOZGeomScene->AddElement(a);
	}

	fUpUOZMgr = new TEveProjectionManager(TEveProjection::kPT_UpUOZ);
	gEve->AddToListTree(fUpUOZMgr, kFALSE);
	{
		// displace origin z-center-ed
		TEveProjection* p = fUpUOZMgr->GetProjection();
		p->SetDisplaceOrigin(kTRUE);
		p->SetCenter(displace);

		// fish-eye transformation zoom out the gap between the up and down mwdc
		UsePreScaleUpUOZ(lrange, hrange);

		// add axises
		TEveProjectionAxes* a = new TEveProjectionAxes(fUpUOZMgr);
		a->SetMainColor(kWhite);
		a->SetTitle("Up U-Z");
		a->SetTitleSize(0.05);
		a->SetTitleFont(102);
		a->SetLabelSize(0.025);
		a->SetLabelFont(102);
		fUpUOZGeomScene->AddElement(a);
	}

	fDownUOZMgr = new TEveProjectionManager(TEveProjection::kPT_DownUOZ);
	gEve->AddToListTree(fDownUOZMgr, kFALSE);
	{
		// displace origin z-center-ed
		TEveProjection* p = fDownUOZMgr->GetProjection();
		p->SetDisplaceOrigin(kTRUE);
		p->SetCenter(displace);

		// fish-eye transformation zoom out the gap between the up and down mwdc
		UsePreScaleDownUOZ(lrange, hrange);

		// add axises
		TEveProjectionAxes* a = new TEveProjectionAxes(fDownUOZMgr);
		a->SetMainColor(kWhite);
		a->SetTitle("Down U-Z");
		a->SetTitleSize(0.05);
		a->SetTitleFont(102);
		a->SetLabelSize(0.025);
		a->SetLabelFont(102);
		fDownUOZGeomScene->AddElement(a);
	}


	// Viewers
	//=========

	TEveWindowSlot *slot = 0;
	TEveWindowPack *pack = 0;
	// 3D Simplified
	f3DView = gEve->SpawnNewViewer("3D Simplified View", "");
	f3DView->GetGLViewer()->SetStyle(TGLRnrCtx::kOutline);
	f3DView->AddScene(f3DEventScene);
	gEve->AddToListTree(f3DEventScene, kFALSE);
	// XY
	slot = TEveWindow::CreateWindowInTab(gEve->GetBrowser()->GetTabRight());
	pack = slot->MakePack();
	pack->SetElementName("X/Y-Projection View");
	pack->SetShowTitleBar(kTRUE);

	pack->NewSlot()->MakeCurrent();
	fXOZView = gEve->SpawnNewViewer("XOZ View", "");
	fXOZView->GetGLViewer()->SetStyle(TGLRnrCtx::kOutline);
	fXOZView->GetGLViewer()->SetCurrentCamera(TGLViewer::kCameraOrthoXOY);
	fXOZView->AddScene(fXOZGeomScene);
	fXOZView->AddScene(fXOZEventScene);

	pack->NewSlot()->MakeCurrent();
	fYOZView = gEve->SpawnNewViewer("YOZ View", "");
	fYOZView->GetGLViewer()->SetCurrentCamera(TGLViewer::kCameraOrthoXOY);
	fYOZView->AddScene(fYOZGeomScene);
	fYOZView->AddScene(fYOZEventScene);

	// U
	slot = TEveWindow::CreateWindowInTab(gEve->GetBrowser()->GetTabRight());
	pack = slot->MakePack();
	pack->SetElementName("U-Projection View");
	pack->SetShowTitleBar(kFALSE);

	pack->NewSlot()->MakeCurrent();
	fUpUOZView = gEve->SpawnNewViewer("Up-UOZ View", "");
	fUpUOZView->GetGLViewer()->SetCurrentCamera(TGLViewer::kCameraOrthoXOY);
	fUpUOZView->AddScene(fUpUOZGeomScene);
	fUpUOZView->AddScene(fUpUOZEventScene);

	pack->NewSlot()->MakeCurrent();
	fDownUOZView = gEve->SpawnNewViewer("Down-UOZ View", "");
	fDownUOZView->GetGLViewer()->SetCurrentCamera(TGLViewer::kCameraOrthoXOY);
	fDownUOZView->AddScene(fDownUOZGeomScene);
	fDownUOZView->AddScene(fDownUOZEventScene);

	// default camerera setting
	UseDefaultCamera();
}

MultiView::~MultiView()
{

}

void MultiView::UseDefaultCamera()
{
	TGLViewer::ECameraType camera;
	TGLViewer* gv;
	Double_t zoom, center[3], hRotate, vRotate, dolly;

	gv = f3DView->GetGLViewer();
	gv->SetGuideState(TGLUtil::kAxesEdge, kTRUE, kFALSE, 0);
	// gv->ResetCameras();
	camera = TGLViewer::kCameraPerspXOY;
	// zoom=10;dolly=0;center[0]=0;center[1]=0;center[2]=29.7;
	// hRotate=-TMath::Pi()/20;vRotate=-TMath::Pi()/3*2;
	gv->SetCurrentCamera(camera);
	// gv->CurrentCamera().SetExternalCenter(kTRUE);
	// gv->SetPerspectiveCamera(camera, zoom, dolly, center, hRotate, vRotate);

	gv = fXOZView->GetGLViewer();
	// gv->SetGuideState(TGLUtil::kAxesOrigin, kTRUE, kFALSE, 0);
	// gv->ResetCameras();
	camera = TGLViewer::kCameraOrthoXOY;
	zoom = 7.5; dolly = 0; center[0] = 0; center[1] = 0; center[2] = (lrange + hrange) / 2;
	hRotate = 0; vRotate = 0;
	gv->SetCurrentCamera(camera);
	gv->SetOrthoCamera(camera, zoom, dolly, center, hRotate, vRotate);

	gv = fYOZView->GetGLViewer();
	// gv->SetGuideState(TGLUtil::kAxesOrigin, kTRUE, kFALSE, 0);
	// gv->ResetCameras();
	camera = TGLViewer::kCameraOrthoXOY;
	zoom = 7.5; dolly = 0; center[0] = 0; center[1] = 0; center[2] = (lrange + hrange) / 2;
	hRotate = 0; vRotate = 0;
	gv->SetCurrentCamera(camera);
	gv->SetOrthoCamera(camera, zoom, dolly, center, hRotate, vRotate);

	gv = fUpUOZView->GetGLViewer();
	// gv->SetGuideState(TGLUtil::kAxesOrigin, kTRUE, kFALSE, 0);
	// gv->ResetCameras();
	camera = TGLViewer::kCameraOrthoXOY;
	zoom = 6; dolly = 0; center[0] = 0; center[1] = 0; center[2] = (lrange + hrange) / 2;
	hRotate = 0; vRotate = 0;
	gv->SetCurrentCamera(camera);
	gv->SetOrthoCamera(camera, zoom, dolly, center, hRotate, vRotate);

	gv = fDownUOZView->GetGLViewer();
	// gv->SetGuideState(TGLUtil::kAxesOrigin, kTRUE, kFALSE, 0);
	// gv->ResetCameras();
	camera = TGLViewer::kCameraOrthoXOY;
	zoom = 6; dolly = 0; center[0] = 0; center[1] = 0; center[2] = (lrange + hrange) / 2;
	hRotate = 0; vRotate = 0;
	gv->SetCurrentCamera(camera);
	gv->SetOrthoCamera(camera, zoom, dolly, center, hRotate, vRotate);
}

void MultiView::SetDepth(Float_t d)
{
	// Set current depth on all projection managers.
	fXOZMgr->SetCurrentDepth(d);
	fYOZMgr->SetCurrentDepth(d);

	fUpUOZMgr->SetCurrentDepth(d);
	fDownUOZMgr->SetCurrentDepth(d);
}

void MultiView::ImportGeomXOZ(TEveElement* el)
{
	fXOZMgr->ImportElements(el, fXOZGeomScene);
}

void MultiView::ImportGeomYOZ(TEveElement* el)
{
	fYOZMgr->ImportElements(el, fYOZGeomScene);
}

void MultiView::ImportGeomUpUOZ(TEveElement* el)
{
	fUpUOZMgr->ImportElements(el, fUpUOZGeomScene);
}

void MultiView::ImportGeomDownUOZ(TEveElement* el)
{
	fDownUOZMgr->ImportElements(el, fDownUOZGeomScene);
}

void MultiView::ImportEventXOZ(TEveElement* el)
{
	fXOZMgr->ImportElements(el, fXOZEventScene);
}

void MultiView::ImportEventYOZ(TEveElement* el)
{
	fYOZMgr->ImportElements(el, fYOZEventScene);
}

void MultiView::ImportEventUpUOZ(TEveElement* el)
{
	fUpUOZMgr->ImportElements(el, fUpUOZEventScene);
}

void MultiView::ImportEventDownUOZ(TEveElement* el)
{
	fDownUOZMgr->ImportElements(el, fDownUOZEventScene);
}

void MultiView::ImportEvent3D(TEveElement* el)
{
	f3DEventScene->AddElement(el);
}

//---------------------------------------------------------------------------
void MultiView::DestroyEvent3D()
{
	f3DEventScene->DestroyElements();
}

void MultiView::DestroyEventXOZ()
{
	fXOZEventScene->DestroyElements();
}

void MultiView::DestroyEventYOZ()
{
	fYOZEventScene->DestroyElements();
}

void MultiView::DestroyEventUpUOZ()
{
	fUpUOZEventScene->DestroyElements();
}

void MultiView::DestroyEventDownUOZ()
{
	fDownUOZEventScene->DestroyElements();
}

void MultiView::UsePreScaleXOZ(Double_t lrange, Double_t hrange) {
	TEveProjection* p = fXOZMgr->GetProjection();
	p->ClearPreScales();
	p->AddPreScaleEntry(1, 0,  0.1);    // z scale 0.1 from origin to lrange
	p->AddPreScaleEntry(1, lrange,  1);    // z scale 1 between lrange and hrange
	p->AddPreScaleEntry(1, hrange,  0.1);    // z scale 0.1 from hrange to infinity
	p->SetUsePreScale(kTRUE);
}

void MultiView::UsePreScaleYOZ(Double_t lrange, Double_t hrange) {
	TEveProjection* p = fYOZMgr->GetProjection();
	p->ClearPreScales();
	p->AddPreScaleEntry(1, 0,  0.1);    // z scale 0.1 from origin to lrange
	p->AddPreScaleEntry(1, lrange,  1);    // z scale 1 between lrange and hrange
	p->AddPreScaleEntry(1, hrange,  0.1);    // z scale 0.1 from hrange to infinity
	p->SetUsePreScale(kTRUE);
}

void MultiView::UsePreScaleUpUOZ(Double_t lrange, Double_t hrange) {
	TEveProjection* p = fUpUOZMgr->GetProjection();
	p->ClearPreScales();
	p->AddPreScaleEntry(1, 0,  0.1);    // z scale 0.1 from origin to lrange
	p->AddPreScaleEntry(1, lrange,  1);    // z scale 1 between lrange and hrange
	p->AddPreScaleEntry(1, hrange,  0.1);    // z scale 0.1 from hrange to infinity
	p->SetUsePreScale(kTRUE);
}

void MultiView::UsePreScaleDownUOZ(Double_t lrange, Double_t hrange) {
	TEveProjection* p = fDownUOZMgr->GetProjection();
	p->ClearPreScales();
	p->AddPreScaleEntry(1, 0,  0.1);    // z scale 0.1 from origin to lrange
	p->AddPreScaleEntry(1, lrange,  1);    // z scale 1 between lrange and hrange
	p->AddPreScaleEntry(1, hrange,  0.1);    // z scale 0.1 from hrange to infinity
	p->SetUsePreScale(kTRUE);
}

void MultiView::SetUsePreScale(Double_t lrange_input, Double_t hrange_input)
{
	if (lrange_input < 0 || hrange_input < 0) {
		UsePreScaleXOZ(lrange, hrange);
		UsePreScaleYOZ(lrange, hrange);
		UsePreScaleUpUOZ(lrange, hrange);
		UsePreScaleDownUOZ(lrange, hrange);
	}
	else {
		lrange = lrange_input; hrange = hrange_input;
		UsePreScaleXOZ(lrange_input, hrange_input);
		UsePreScaleYOZ(lrange_input, hrange_input);
		UsePreScaleUpUOZ(lrange_input, hrange_input);
		UsePreScaleDownUOZ(lrange_input, hrange_input);
	}
}

void MultiView::DisablePreScale()
{
	TEveProjection* p = fXOZMgr->GetProjection();
	p->SetUsePreScale(kFALSE);
	fXOZMgr->ProjectChildren();

	p = fYOZMgr->GetProjection();
	p->SetUsePreScale(kFALSE);
	fYOZMgr->ProjectChildren();

	p = fUpUOZMgr->GetProjection();
	p->SetUsePreScale(kFALSE);
	fUpUOZMgr->ProjectChildren();

	p = fDownUOZMgr->GetProjection();
	p->SetUsePreScale(kFALSE);
	fDownUOZMgr->ProjectChildren();

	// UseDefaultCamera();
	gEve->Redraw3D();
}

void MultiView::EnablePreScale()
{
	UseDefaultCamera();

	TEveProjection* p = fXOZMgr->GetProjection();
	p->SetUsePreScale(kTRUE);
	fXOZMgr->ProjectChildren();

	p = fYOZMgr->GetProjection();
	p->SetUsePreScale(kTRUE);
	fYOZMgr->ProjectChildren();

	p = fUpUOZMgr->GetProjection();
	p->SetUsePreScale(kTRUE);
	fUpUOZMgr->ProjectChildren();

	p = fDownUOZMgr->GetProjection();
	p->SetUsePreScale(kTRUE);
	fDownUOZMgr->ProjectChildren();

	gEve->Redraw3D();
}