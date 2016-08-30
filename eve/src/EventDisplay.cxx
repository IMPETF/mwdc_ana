#include "EventDisplay.h"
#include "MultiView.h"
#include "EventHandler.h"
#include <TEveGeoShape.h>
#include <TEveGeoShapeExtract.h>
#include <TEveManager.h>
#include <TEveEventManager.h>
#include <TEveElement.h>
#include <TFile.h>
#include <TString.h>
#include <TEveBrowser.h>
#include <TGFrame.h>
#include <TGButton.h>
#include <TEveText.h>
#include <TEveArrow.h>
#include <TEveTrans.h>
#include <TGTab.h>
#include <TGClient.h>
#include <TGTextEntry.h>
#include <TGLabel.h>
#include <TSystem.h>
#include <TGNumberEntry.h>


ClassImp(EventDisplay)

EventDisplay* gEvtDisplay = 0;

EventDisplay* EventDisplay::Create()
{
	if (!gEvtDisplay) {
		gEvtDisplay = new EventDisplay();
	}

	return gEvtDisplay;
}

EventDisplay::EventDisplay()
: fEventHandler(0)
{
	TEveManager::Create();

	fMultiView = new MultiView();
}

EventDisplay::~EventDisplay()
{
	delete fMultiView;

	// TEveManager::Terminate();
}

/**
 * [EventDisplay::ImportGeometry description]
 * @param  filename root file name which include the mwdc geometry
 * @param  geo_name object name in the root file
 * @return          whether the import process is successful
 *
 * Note: the Geometry should be TEveGeoShapeExtract
 */
bool EventDisplay::ImportGeometry(const char* filename, const char* geo_name)
{
	printf("in:%s\n", filename );
	Bool_t flag = kFALSE;
	TEveGeoShapeExtract* gse;
	TFile* file = new TFile(filename);
	if (file) {
		file->GetObject(geo_name, gse);
		if (gse) {
			fGeometry = TEveGeoShape::ImportShapeExtract(gse, 0);
			flag = kTRUE;
		}
		delete file;
	}

	return flag;
}

void EventDisplay::Initialize()
{
	AddGeometry();
	AddGeometryGuides();

	MakeGui();

	UseDefaultSetting();

	gEve->Redraw3D();
}

void EventDisplay::Draw(Bool_t resetCameras, Bool_t dropLogicals)
{
	gEve->Redraw3D(resetCameras, dropLogicals);
}

void EventDisplay::AddGuides()
{
	fMultiView->ImportEvent3D(fTextUp);
	fMultiView->ImportEvent3D(fTextDown);
	fMultiView->ImportEvent3D(fTextX);
	fMultiView->ImportEvent3D(fArrowX);
	fMultiView->ImportEvent3D(fTextY);
	fMultiView->ImportEvent3D(fArrowY);
}

void EventDisplay::AddToAll(TEveElement* el)
{
	AddToDefaultView(el);
	AddTo3D(el);
	AddToXOZ(el);
	AddToYOZ(el);
	AddToUpUOZ(el);
	AddToDownUOZ(el);
}

void EventDisplay::AddToDefaultView(TEveElement* el)
{
	gEve->AddElement(el);
}

void EventDisplay::AddTo3D(TEveElement* el)
{
	fMultiView->ImportEvent3D(el);
}

void EventDisplay::AddToXOZ(TEveElement* el)
{
	fMultiView->ImportEventXOZ(el);
}

void EventDisplay::AddToYOZ(TEveElement* el)
{
	fMultiView->ImportEventYOZ(el);
}

void EventDisplay::AddToUpUOZ(TEveElement* el)
{
	fMultiView->ImportEventUpUOZ(el);
}

void EventDisplay::AddToDownUOZ(TEveElement* el)
{
	fMultiView->ImportEventDownUOZ(el);
}

void EventDisplay::Clean()
{
	gEve->GetViewers()->DeleteAnnotations();

	CleanXOZ();
	CleanYOZ();
	CleanUpUOZ();
	CleanDownUOZ();
	Clean3D();
	CleanDefaultView();
}

void EventDisplay::CleanXOZ()
{
	fMultiView->DestroyEventXOZ();
}

void EventDisplay::CleanYOZ()
{
	fMultiView->DestroyEventYOZ();
}

void EventDisplay::CleanUpUOZ()
{
	fMultiView->DestroyEventUpUOZ();
}

void EventDisplay::CleanDownUOZ()
{
	fMultiView->DestroyEventDownUOZ();
}

void EventDisplay::Clean3D()
{
	fMultiView->DestroyEvent3D();
}

void EventDisplay::CleanDefaultView()
{
	gEve->GetCurrentEvent()->DestroyElements();
}

void EventDisplay::AddGeometryGuides()
{
	// add text and arrows
	Double_t t_pos[3];
	fTextUp = new TEveText("Up");
	fTextUp->SetFontSize(20); fTextUp->SetMainColor(kBlue);
	TEveElement* x_up = GetCell(1, 0, 39);
	x_up->RefMainTrans().GetPos(t_pos);
	t_pos[0] -= 70; t_pos[2] += 10;
	fTextUp->RefMainTrans().SetPos(t_pos);
	gEve->AddGlobalElement(fTextUp);

	Double_t t_pos_down[3];
	fTextDown = new TEveText("Down");
	fTextDown->SetFontSize(20); fTextDown->SetMainColor(kBlue);
	TEveElement* x_down = GetCell(0, 0, 39);
	x_down->RefMainTrans().GetPos(t_pos_down);
	t_pos_down[0] -= 70; t_pos_down[2] -= 10;
	fTextDown->RefMainTrans().SetPos(t_pos_down);
	gEve->AddGlobalElement(fTextDown);

	fArrowX = new TEveArrow(15., 0., 0., t_pos[0], t_pos[1], (t_pos[2] + t_pos_down[2]) / 2);
	fArrowX->SetMainColor(kRed); fArrowX->SetTubeR(0.05); fArrowX->SetConeR(0.15); fArrowX->SetConeL(0.2);
	fArrowX->SetPickable(kTRUE);
	gEve->AddGlobalElement(fArrowX);
	fTextX = new TEveText("+X");
	fTextX->SetFontSize(15); fTextX->SetMainColor(kRed);
	fTextX->RefMainTrans().SetPos(t_pos[0] + 15, t_pos[1], (t_pos[2] + t_pos_down[2]) / 2);
	gEve->AddGlobalElement(fTextX);

	fArrowY = new TEveArrow(0., 15., 0., t_pos[0], t_pos[1], (t_pos[2] + t_pos_down[2]) / 2);
	fArrowY->SetMainColor(kGreen); fArrowY->SetTubeR(0.05); fArrowY->SetConeR(0.15); fArrowY->SetConeL(0.2);
	fArrowY->SetPickable(kTRUE);
	gEve->AddGlobalElement(fArrowY);
	fTextY = new TEveText("+Y");
	fTextY->SetFontSize(15); fTextY->SetMainColor(kGreen);
	fTextY->RefMainTrans().SetPos(t_pos[0], t_pos[1] + 15, (t_pos[2] + t_pos_down[2]) / 2);
	gEve->AddGlobalElement(fTextY);
}

void EventDisplay::AddGeometry()
{
	gEve->AddGlobalElement(fGeometry);

	fMultiView->SetDepth(-10);
	fMultiView->ImportGeomXOZ(fGeometry);
	fMultiView->ImportGeomYOZ(fGeometry);
	fMultiView->ImportGeomUpUOZ(fGeometry);
	fMultiView->ImportGeomDownUOZ(fGeometry);
	fMultiView->SetDepth(0);
}

void EventDisplay::UseDefaultSetting()
{
	gEve->GetViewers()->SwitchColorSet();
	gEve->GetDefaultGLViewer()->SetStyle(TGLRnrCtx::kOutline);
	gEve->GetDefaultGLViewer()->SetGuideState(TGLUtil::kAxesEdge, kTRUE, kFALSE, 0);

	gEve->GetBrowser()->GetTabRight()->SetTab(1);
}

/**
 * [EventDisplay::MakeGui description]
 * Contruct other GUI components(except Viewers)
 * in the EveBrowser.
 */
void EventDisplay::MakeGui()
{
	TEveBrowser* browser = gEve->GetBrowser();
	browser->StartEmbedding(TRootBrowser::kLeft);

	TGMainFrame* frmMain = new TGMainFrame(gClient->GetRoot(), 1000, 600);
	frmMain->SetWindowName("XX GUI");
	frmMain->SetCleanup(kDeepCleanup);

	// Event Status
	// TGGroupFrame* statusf = new TGGroupFrame(frmMain, "Event Status");
	{
		TGLabel* fLabelStatus = new TGLabel(frmMain, "Event Status");
		frmMain->AddFrame(fLabelStatus, new TGLayoutHints(kLHintsTop | kLHintsCenterX | kLHintsExpandX, 2, 10, 10, 10));
		fTextEntryStatus = new TGTextEntry(frmMain);
		fTextEntryStatus->SetEnabled(kFALSE);
		frmMain->AddFrame(fTextEntryStatus, new TGLayoutHints(kLHintsCenterX | kLHintsTop | kLHintsExpandX, 2, 10, 10, 10));
	}
	// frmMain->AddFrame(statusf, new TGLayoutHints(kLHintsExpandX, 2, 2, 5, 1));

	// Next/Previous Navigation
	TGGroupFrame* navf = new TGGroupFrame(frmMain, "Event Navigation");
	{
		TGHorizontalFrame* hf_nav = new TGHorizontalFrame(navf);
		TString icondir( Form("%s/icons/", gSystem->Getenv("ROOTSYS")) );
		TGPictureButton* b = 0;

		TGLabel* fLabelPre=new TGLabel(hf_nav,"Previous:");
		hf_nav->AddFrame(fLabelPre, new TGLayoutHints(kLHintsLeft | kLHintsCenterY, 10, 2, 10, 10));
		b = new TGPictureButton(hf_nav, gClient->GetPicture(icondir + "GoBack.gif"));
		hf_nav->AddFrame(b, new TGLayoutHints(kLHintsLeft | kLHintsCenterY, 10, 2, 10, 10));
		b->Connect("Clicked()", "EventHandler", fEventHandler, "PreviousEvent()");

		
		b = new TGPictureButton(hf_nav, gClient->GetPicture(icondir + "GoForward.gif"));
		hf_nav->AddFrame(b, new TGLayoutHints(kLHintsRight | kLHintsCenterY, 2, 10, 10, 10));
		b->Connect("Clicked()", "EventHandler", fEventHandler, "NextEvent()");
		TGLabel* fLabelNext=new TGLabel(hf_nav,"Next:");
		hf_nav->AddFrame(fLabelNext, new TGLayoutHints(kLHintsRight | kLHintsCenterY, 10, 2, 10, 10));

		navf->AddFrame(hf_nav, new TGLayoutHints(kLHintsExpandX, 2, 2, 5, 1) );
	}
	// Goto Navigation
	{
		TGHorizontalFrame* hf_goto = new TGHorizontalFrame(navf);

		TGLabel* fLabelGoto=new TGLabel(hf_goto,"Goto Event:");
		hf_goto->AddFrame(fLabelGoto, new TGLayoutHints(kLHintsLeft | kLHintsCenterY, 10, 2, 10, 10));

		fNumberEntryGoto = new TGNumberEntry(hf_goto, 0, 9,998, TGNumberFormat::kNESInteger,TGNumberFormat::kNEANonNegative, TGNumberFormat::kNELNoLimits);
		fNumberEntryGoto->Connect("ValueSet(Long_t)", "EventDisplay", this, "DoGotoEvent()");
		// (fNumberEntryGoto->GetNumberEntry())->Connect("ReturnPressed()", "EventDisplay", this, "DoGotoEvent()");
		hf_goto->AddFrame(fNumberEntryGoto, new TGLayoutHints(kLHintsLeft | kLHintsCenterY, 10, 2, 10, 10));

		navf->AddFrame(hf_goto, new TGLayoutHints(kLHintsExpandX, 2, 2, 5, 1) );
	}
	frmMain->AddFrame(navf, new TGLayoutHints(kLHintsExpandX, 2, 2, 5, 1));

	// Navigation Config
	TGGroupFrame* configf = new TGGroupFrame(frmMain, "Navigation Config");
	{
		TGHorizontalFrame* hf_config = new TGHorizontalFrame(configf);

		TGLabel* fLabelConfig=new TGLabel(hf_config,"Step:");
		hf_config->AddFrame(fLabelConfig, new TGLayoutHints(kLHintsLeft | kLHintsCenterY, 10, 2, 10, 10));

		fNumberEntryConfig = new TGNumberEntry(hf_config, 1, 9,997, TGNumberFormat::kNESInteger,TGNumberFormat::kNEANonNegative, TGNumberFormat::kNELLimitMin, 1);
		fNumberEntryConfig->Connect("ValueSet(Long_t)", "EventDisplay", this, "DoConfigStep()");
		// (fNumberEntryConfig->GetNumberEntry())->Connect("ReturnPressed()", "EventDisplay", this, "DoConfigStep()");
		hf_config->AddFrame(fNumberEntryConfig, new TGLayoutHints(kLHintsLeft | kLHintsCenterY, 10, 2, 10, 10));

		configf->AddFrame(hf_config, new TGLayoutHints(kLHintsExpandX, 2, 2, 5, 1) );
	}
	frmMain->AddFrame(configf, new TGLayoutHints(kLHintsExpandX, 2, 2, 5, 1));

	// Scale Management
	TGGroupFrame* scalef = new TGGroupFrame(frmMain, "Scale Projection");
	TGHorizontalFrame* hf = new TGHorizontalFrame(scalef);
	{
		TGTextButton* b = 0;

		b = new TGTextButton(hf, "EnableScale");
		hf->AddFrame(b, new TGLayoutHints(kLHintsTop | kLHintsExpandX, 2, 0, 2, 2));
		b->Connect("Clicked()", "MultiView", fMultiView, "EnablePreScale()");

		b = new TGTextButton(hf, "DisableScale");
		hf->AddFrame(b, new TGLayoutHints(kLHintsTop | kLHintsExpandX, 3, 2, 2, 2));
		b->Connect("Clicked()", "MultiView", fMultiView, "DisablePreScale()");
	}
	scalef->AddFrame(hf, new TGLayoutHints(kLHintsExpandX, 2, 2, 5, 1));
	frmMain->AddFrame(scalef, new TGLayoutHints(kLHintsExpandX, 2, 2, 5, 1));


	frmMain->MapSubwindows();
	frmMain->Resize();
	frmMain->MapWindow();

	browser->StopEmbedding();
	browser->SetTabTitle("Management", 0);
}

/**
 * [EventDisplay::GetCell description]
 * @param  l     layer: 0 down, 1 up
 * @param  p     plane: 0 X, 1 Y, 2 U
 * @param  index wire:  0-79 X/Y, 0-105 U
 * @return       the eve element(box) corresponds to the l,p,index.
 *
 * Note: l,p,index definition is the same as in global.h
 */
TEveElement* EventDisplay::GetCell(Int_t l, Int_t p, Int_t index)
{
	const char name[3][3] = {"x", "y", "u"};

	TString plane_name = TString::Format("%s_plane_%d", name[p], l);
	TEveElement* plane = fGeometry->FindChild(plane_name);

	TString cell_name;
	if (p == 2) {
		switch (l) {
		case 0:
			cell_name = TString::Format("u_chamber%d_0", 105 - index);
			break;
		case 1:
			cell_name = TString::Format("u_chamber%d_0", index);
			break;
		}
	}
	else {
		switch (l) {
		case 0:
			cell_name = TString::Format("%s_cell_%d", name[p], index + 1);
			break;
		case 1:
			cell_name = TString::Format("%s_cell_%d", name[p], 80 - index);
			break;
		}
	}
	TEveElement* cell = plane->FindChild(cell_name);

	return cell;
}

/**
 * [EventDisplay::GetWire description]
 * @param  l     layer: 0 down, 1 up
 * @param  p     plane: 0 X, 1 Y, 2 U
 * @param  index wire:  0-79 X/Y, 0-105 U
 * @return       the eve element(tube) corresponds to the l,p,index.
 *
 * Note: l,p,index definition is the same as in global.h
 */
TEveElement* EventDisplay::GetWire(Int_t l, Int_t p, Int_t index)
{
	const char name[3][3] = {"x", "y", "u"};

	TString plane_name = TString::Format("%s_plane_%d", name[p], l);
	TEveElement* plane = fGeometry->FindChild(plane_name);

	TString cell_name;
	if (p == 2) {
		switch (l) {
		case 0:
			cell_name = TString::Format("u_chamber%d_0", 105 - index);
			break;
		case 1:
			cell_name = TString::Format("u_chamber%d_0", index);
			break;
		}
	}
	else {
		switch (l) {
		case 0:
			cell_name = TString::Format("%s_cell_%d", name[p], index + 1);
			break;
		case 1:
			cell_name = TString::Format("%s_cell_%d", name[p], 80 - index);
			break;
		}
	}
	TEveElement* cell = plane->FindChild(cell_name);

	TEveElement* wire = cell->FirstChild();

	return wire;
}

void EventDisplay::UpdateDriftRadius(Double_t value[2][3])
{
	fMultiView->UpdateDriftRadius(value);
}

void EventDisplay::UpdateInitialFittedDistance(Double_t value[2][3])
{
	fMultiView->UpdateInitialFittedDistance(value);
}

void EventDisplay::UpdateFinalFittedDistance(Double_t value[2][3])
{
	fMultiView->UpdateFinalFittedDistance(value);
}

void EventDisplay::DoGotoEvent()
{
	UInt_t cur=fNumberEntryGoto->GetNumberEntry()->GetIntNumber();
	printf("DoGotoEvent: %d\n",cur);

	fEventHandler->GotoEvent(cur);
}

void EventDisplay::DoConfigStep()
{
	Int_t step=fNumberEntryConfig->GetNumberEntry()->GetIntNumber();
	printf("DoConfigStep: %d\n",step);
	
	fEventHandler->SetNavigationStep(step);
}