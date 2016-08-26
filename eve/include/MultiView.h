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

#ifndef __MultiView__
#define __MultiView__

#include <TEveManager.h>
#include <TEveViewer.h>
#include <TGLViewer.h>
#include <TEveScene.h>
#include <TEveProjectionManager.h>
#include <TEveProjectionAxes.h>
#include <TEveBrowser.h>
#include <TEveWindow.h>

///////////////////////////////////////////
// MultiView: Cutomized Eve Browser Tabs //
///////////////////////////////////////////
class MultiView
{
public:
	MultiView();
	~MultiView();

	void UseDefaultCamera();

	void SetDepth(Float_t d);

	void ImportGeomXOZ(TEveElement* el);
	void ImportGeomYOZ(TEveElement* el);
	void ImportGeomUpUOZ(TEveElement* el);
	void ImportGeomDownUOZ(TEveElement* el);

	void ImportEventXOZ(TEveElement* el);
	void ImportEventYOZ(TEveElement* el);
	void ImportEventUpUOZ(TEveElement* el);
	void ImportEventDownUOZ(TEveElement* el);	
	void ImportEvent3D(TEveElement* el);

	void DestroyEventXOZ();
	void DestroyEventYOZ();
	void DestroyEventUpUOZ();
	void DestroyEventDownUOZ();
	void DestroyEvent3D();

	void UsePreScaleXOZ(Double_t lrange, Double_t hrange);
	void UsePreScaleYOZ(Double_t lrange, Double_t hrange);
	void UsePreScaleUpUOZ(Double_t lrange, Double_t hrange);
	void UsePreScaleDownUOZ(Double_t lrange, Double_t hrange);
	void SetUsePreScale(Double_t lrange_input=-1, Double_t hrange_input=-1);
	void DisablePreScale();
	void EnablePreScale();

	TEveProjectionManager *fUpUOZMgr;
	TEveProjectionManager *fDownUOZMgr;
	TEveProjectionManager *fXOZMgr;
	TEveProjectionManager *fYOZMgr;

	TEveViewer            *f3DView;
	TEveViewer            *fUpUOZView;
	TEveViewer            *fDownUOZView;
	TEveViewer            *fXOZView;
	TEveViewer            *fYOZView;

	TEveScene             *fUpUOZGeomScene;
	TEveScene             *fDownUOZGeomScene;
	TEveScene             *fXOZGeomScene;
	TEveScene             *fYOZGeomScene;

	TEveScene             *f3DEventScene;
	TEveScene             *fUpUOZEventScene;
	TEveScene             *fDownUOZEventScene;
	TEveScene             *fXOZEventScene;
	TEveScene             *fYOZEventScene;

private:
	Double_t lrange;
	Double_t hrange;//z-axis in the range [lrange,hrange] will be scaled
	
	ClassDef(MultiView, 0);
};
#endif