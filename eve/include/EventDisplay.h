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

#ifndef __EventDisply__
#define __EventDisply__ 

#include <Rtypes.h>
#include <TEveSelection.h>
class EventHandler;
class TEveElement;
class TEveGeoShape;
class TEveText;
class TEveArrow;
class MultiView;
class TGSimpleTable;
class TGTextEntry;
class TGNumberEntry;
class TGSimpleTable;

/**
 * EventDisplay: The Main Class for Event Display
 *   1. It is a singlton class
 *   2. It is responsible for the construction of the
 *   	global geometry view.
 *   3. However, it is NOT responsible for the construction
 *   	of the event view. This part is propagated to the 
 *   	EventHandler class.
 */
class EventDisplay
{
public:
	~EventDisplay();
	static EventDisplay* Create();
	
	// init interface
	bool   ImportGeometry(const char* filename, const char* geo_name="mwdc");
	void   SetEventHandler(EventHandler* evthandler) {fEventHandler=evthandler;}
	void   Initialize();
	void   Draw(Bool_t resetCameras=kFALSE, Bool_t dropLogicals=kFALSE);

	// event scene manipulations
	void   AddGuides();
	void   AddToAll(TEveElement* el);
	template<class T> void   AddToProjection(T* el[2][3]);
	void   AddToXOZ(TEveElement* el);
	void   AddToYOZ(TEveElement* el);
	void   AddToUpUOZ(TEveElement* el);
	void   AddToDownUOZ(TEveElement* el);
	void   AddTo3D(TEveElement* el);
	void   AddToDefaultView(TEveElement* el);

	void   Clean();
	void   CleanXOZ();
	void   CleanYOZ();
	void   CleanUpUOZ();
	void   CleanDownUOZ();
	void   Clean3D();
	void   CleanDefaultView();

	// hight/dehight cells
	void   AddCellToHighlight(int l, int p, int index);
	void   RemoveCellFromHighlight(int l,int p, int index);
	void   RemoveHightedCells();

	// summary table
	void   DefaultColumnName();
	void   DefaultRowName();
	void   UpdateDriftRadius(Double_t value[2][3]);
	void   UpdateInitialFittedDistance(Double_t value[2][3]);
	void   UpdateFinalFittedDistance(Double_t value[2][3]);

	// gui related slots
	void   DoGotoEvent();
	void   DoConfigStep();
	void   DoActivateHighlight();
	void   DoDeactivateHighlight();
	
private:
	EventDisplay();
	void   MakeGui();
	void   AddGeometryGuides();
	void   AddGeometry();
	void   UseDefaultSetting();

public:
	TEveElement* GetCell(int l, int p, int index);
	TEveElement* GetWire(int l, int p, int index);

private:
	MultiView* 		fMultiView;
	TEveGeoShape*   fGeometry;
	EventHandler*   fEventHandler;

private:
	// guides
	TEveText*	fTextUp;
	TEveText*   fTextDown;
	TEveText*   fTextX;
	TEveText*   fTextY;
	TEveArrow*  fArrowX;
	TEveArrow*  fArrowY;

	// gui
	TGTextEntry* fTextEntryStatus;
	TGNumberEntry* fNumberEntryGoto;
	TGNumberEntry* fNumberEntryConfig;

	// summary table
	TGSimpleTable*  fTableDistance;
	Double_t        fTableDistanceBuffer[6][3];
	Double_t*       fTableDistanceBufferTemp[6];

	// highlight
	TEveSelection   fHittedCells;

	ClassDef(EventDisplay, 0);
};

template<class T> void EventDisplay::AddToProjection(T* el[2][3])
{
	for(int l=0;l<2;l++){
		for(int p=0;p<3;p++){
			AddToDefaultView(el[l][p]);
			AddTo3D(el[l][p]);
		}
	}

	AddToXOZ(el[0][0]);
	AddToXOZ(el[1][1]);

	AddToYOZ(el[0][1]);
	AddToYOZ(el[1][0]);

	AddToUpUOZ(el[1][2]);
	AddToDownUOZ(el[0][2]);
}

extern EventDisplay* gEvtDisplay;

#endif