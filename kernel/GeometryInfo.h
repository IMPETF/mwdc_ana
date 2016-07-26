#include <map>
#include "TVector3.h"

class GeometryInfo
{
public:
	GeometryInfo();
	~GeometryInfo();

	void GetPoint(UInt_t gid,TVector3& point);
	void GetDirection(UInt_t gid, TVector3& direction);
	TVector3 GetPoint(UChar_t location,UChar_t direction,UShort_t index);
	TVector3 GetDirection(UChar_t location,UChar_t direction,UShort_t index);

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