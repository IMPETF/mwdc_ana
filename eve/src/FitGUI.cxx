#include "global.h"
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

FitGUI::FitGUI(const char* filename, Long_t evt)
{
  fLineFitAlg = new LineFit(GeometryInfo());
	//
	SetFile(filename);
	SetFirstEvent(evt);
	//
	Init();
}

FitGUI::~FitGUI()
{
	delete fLineFitAlg;
	delete fFileInput;
}

void FitGUI::SetFile(const char* filename)
{
	fFileNameInput = filename;
}

void FitGUI::SetFirstEvent(Long_t evt)
{
	fCurrentEvent = evt;
}

void FitGUI::Init()
{
	//
	fFileInput = new TFile(fFileNameInput);
	if (!fFileInput) {
		PrintWarningMessage("Open File Error!");
		return;
	}

	//
	fFileInput->GetObject("minrising_drifttime/minrising_drifttime", fTreeDriftTime);
	if (!fTreeDriftTime) {
		PrintWarningMessage("Drifttime tree not exist");
	}
	fTreeDriftTime->SetBranchAddress("gid", fGid);
	fTreeDriftTime->SetBranchAddress("tot", fTot);
	fTreeDriftTime->SetBranchAddress("drifttime", fRawDriftTime);
	fTotalEvents = fTreeDriftTime->GetEntries();

	//
	fFileInput->GetObject("minrising_drifttime/init_edge_fitting_result", fDriftInfo);
	if (!fDriftInfo) {
		PrintWarningMessage("DriftInfo not exist");
	}

	//
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

}

Bool_t FitGUI::ReadEvent()
{
	fTreeDriftTime->GetEntry(fCurrentEvent);

	// check event validity
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
				if (!Encoding::IsChannelValid(fGid[l][p])) {
					valid_event = false;
					break;
				}
				//
				wireindex = Encoding::DecodeIndex(fGid[l][p]);
				if (wireindex < boundary_wire_low[p] || wireindex > boundary_wire_high[p]) {
					valid_event = false;
					break;
				}
				//
				for (int brokenwire_index = 0; brokenwire_index < 11; brokenwire_index++) {
					if (fGid[l][p] == brokenwire_gid[brokenwire_index]) {
						valid_event = false;
						break;
					}
				}
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

	// correct drifttime and get drift distance
	Double_t t0,T0;
	if(valid_event){
		for(l=0;l<g_mwdc_location;l++){
		  for(p=0;p<g_mwdc_wireplane;p++){
		  	t0=fDriftInfo->Get_t0(fGid[l][p]);
		  	T0=fDriftInfo->Get_T0(fGid[l][p]);
		  	fCorrectedDriftTime[l][p] = fRawDriftTime[l][p] - (t0-2*T0);
		  	fDriftRadius[l][p] = fRtRelation_Spline[l][p]->Eval(fCorrectedDriftTime[l][p]);
		  }
		}
	}

	// get init track
	
	// Fit
	
	// 
}

void FitGUI::AddEvent()
{

}

Bool_t FitGUI::Fit()
{
	fLineFitAlg->Reset();

}
