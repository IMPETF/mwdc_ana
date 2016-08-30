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

#ifndef _EventHandler_
#define _EventHandler_

#include "Rtypes.h"

class EventHandler
{
public:
	EventHandler(): fTotalEvents(0),fCurrentEvent(0),fNavigationStep(1){}
	~EventHandler(){}

	void NextEvent(){
		printf("NextEvent: %d \n", fCurrentEvent+1);

		fCurrentEvent++;
	}
	void PreviousEvent(){ 
		printf("PreviousEvent: %d\n", fCurrentEvent-1);

		fCurrentEvent--;
	}
	Bool_t GotoEvent(UInt_t ev){ 
		printf("GotoEvent: %d\n",ev);
		fCurrentEvent = ev;
	}

	void SetNavigationStep(Int_t step) {
		printf("SetNavigationStep: %d\n",step);
		fNavigationStep=step;
	}

	virtual void DropEvent() {};
	virtual void AddEvent() {};

private:
	UInt_t fTotalEvents;
	UInt_t fCurrentEvent;
	Int_t  fNavigationStep;

ClassDef(EventHandler,0);
};

#endif