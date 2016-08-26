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

#ifndef __GEOMETRYINFO__
#define __GEOMETRYINFO__ 

#include <map>
#include "TVector3.h"

class GeometryInfo
{
public:
	GeometryInfo();
	~GeometryInfo();

	TVector3 GetPoint(UInt_t gid) const;
	TVector3 GetDirection(UInt_t gid) const;
	TVector3 GetPoint(UChar_t location,UChar_t direction,UShort_t index) const;
	TVector3 GetDirection(UChar_t location,UChar_t direction,UShort_t index) const;

protected:
	void Init();
	// X,Y,Z is the wireplane label, which is not consistent with the coordinate system definition
	// According to the coordination system's definition:
	// 		1) UpX wire is oriented along x axis, thus measures y position
	// 		2) UpY wire is oriented along y axis, thus measures x position
	// 		3) The orientation angle between UpX and UpU wire is 30 degree
	// 		4) DownX wire is oriented along y axis, thus measures x position
	// 		5) DownY wire is oriented along x axis, thus measures y position
	// 		6) The orientation angle between DownX and DownU wire is 30 degree
	void InitUpX();
	void InitUpY();
	void InitUpZ();
	void InitDownX();
	void InitDownY();
	void InitDownZ();

private:
	std::map<UInt_t, TVector3> fPointMap;
	std::map<UInt_t, TVector3> fDirectionMap;
};
#endif