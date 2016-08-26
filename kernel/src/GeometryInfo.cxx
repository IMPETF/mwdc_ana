#include "global.h"
#include "GeometryInfo.h"
#include <algorithm>
#include "TMath.h"

GeometryInfo::GeometryInfo()
{
	Init();
}

GeometryInfo::~GeometryInfo()
{
	fPointMap.clear();
	fDirectionMap.clear();
}

void GeometryInfo::Init()
{
	InitUpX();
	InitUpY();
	InitUpZ();
	InitDownX();
	InitDownY();
	InitDownZ();	
}

TVector3 GeometryInfo::GetPoint(UInt_t gid) const
{
	std::map<UInt_t, TVector3>::const_iterator it;
	it=fPointMap.find(gid);
	if(it!=fPointMap.end())
		return it->second;
	else
		exit;
}

TVector3 GeometryInfo::GetDirection(UInt_t gid) const
{
	std::map<UInt_t, TVector3>::const_iterator it;
	it=fDirectionMap.find(gid);
	if(it!=fDirectionMap.end())
		return it->second;
	else
		exit;
}

TVector3 GeometryInfo::GetPoint(UChar_t location,UChar_t direction,UShort_t index) const
{
	UInt_t gid=Encoding::Encode(EMWDC,location,direction,index);
	std::map<UInt_t, TVector3>::const_iterator it;
	it=fPointMap.find(gid);
	if(it!=fPointMap.end())
		return it->second;
	else
		exit;
}

TVector3 GeometryInfo::GetDirection(UChar_t location,UChar_t direction,UShort_t index) const
{
	UInt_t gid=Encoding::Encode(EMWDC,location,direction,index);
	std::map<UInt_t, TVector3>::const_iterator it;
	it=fDirectionMap.find(gid);
	if(it!=fDirectionMap.end())
		return it->second;
	else
		exit;
}

void GeometryInfo::InitUpX()
{
	Double_t x_up_position[80]={-414.75,-404.25,-393.75,-383.25,-372.75,-362.25,-351.75,-341.25,-330.75,-320.25,-309.75,-299.25,-288.75,-278.25,-267.75,-257.25,
	                       -246.75,-236.25,-225.75,-215.25,-204.75,-194.25,-183.75,-173.25,-162.75,-152.25,-141.75,-131.25,-120.75,-110.25,-99.75,-89.25,
	                       -78.75,-68.25,-57.75,-47.25,-36.75,-26.25,-15.75,-5.25,5.25,15.75,26.25,36.75,47.25,57.75,68.25,78.75,
	                       89.25,99.75,110.25,120.75,131.25,141.75,152.25,162.75,173.25,183.75,194.25,204.75,215.25,225.75,236.25,246.75,
	                       257.25,267.75,278.25,288.75,299.25,309.75,320.25,330.75,341.25,351.75,362.25,372.75,383.25,393.75,404.25,414.75};//Y position measuring
	Double_t x_up_z[80];
	std::fill_n(x_up_z,80,773);

	TVector3 point,direction;
	UInt_t gid;
	for(int i=0;i<80;i++){
		gid=Encoding::Encode(EMWDC,EUP,EX,i);
		point.SetXYZ(0,x_up_position[i],x_up_z[i]);
		direction.SetXYZ(1,0,0);
		fPointMap[gid]=point;
		fDirectionMap[gid]=direction;
	}
}

void GeometryInfo::InitUpY()
{
	Double_t y_up_position[80]={-414.75,-404.25,-393.75,-383.25,-372.75,-362.25,-351.75,-341.25,-330.75,-320.25,-309.75,-299.25,-288.75,-278.25,-267.75,-257.25,
	                         -246.75,-236.25,-225.75,-215.25,-204.75,-194.25,-183.75,-173.25,-162.75,-152.25,-141.75,-131.25,-120.75,-110.25,-99.75,-89.25,
	                         -78.75,-68.25,-57.75,-47.25,-36.75,-26.25,-15.75,-5.25,5.25,15.75,26.25,36.75,47.25,57.75,68.25,78.75,
	                         89.25,99.75,110.25,120.75,131.25,141.75,152.25,162.75,173.25,183.75,194.25,204.75,215.25,225.75,236.25,246.75,
	                         257.25,267.75,278.25,288.75,299.25,309.75,320.25,330.75,341.25,351.75,362.25,372.75,383.25,393.75,404.25,414.75};//X position measuring
	Double_t y_up_z[80];
	std::fill_n(y_up_z,80,758);

	TVector3 point,direction;
	UInt_t gid;
	for(int i=0;i<80;i++){
		gid=Encoding::Encode(EMWDC,EUP,EY,i);
		point.SetXYZ(y_up_position[i],0,y_up_z[i]);
		direction.SetXYZ(0,1,0);
		fPointMap[gid]=point;
		fDirectionMap[gid]=direction;
	}
}

void GeometryInfo::InitUpZ()
{
	// init_point is the UpU_01
	TVector3 init_point(-438.4964,-383.0748,743);
	TVector3 wire_direction(TMath::Sqrt(3.0)/2.0,-0.5,0);
	TVector3 moving_direction(0.5,TMath::Sqrt(3.0)/2.0,0);

	TVector3 point;
	UInt_t gid;
	Double_t wire_sparation=10.5;
	for(int i=0;i<106;i++){
		gid=Encoding::Encode(EMWDC,EUP,EU,i);
		point=init_point + wire_sparation*i*moving_direction;
		fPointMap[gid]=point;
		fDirectionMap[gid]=wire_direction;
	}
}

void GeometryInfo::InitDownX()
{
	Double_t x_down_position[80]={-429.75,-419.25,-408.75,-398.25,-387.75,-377.25,-366.75,-356.25,-345.75,-335.25,-324.75,-314.25,-303.75,-293.25,-282.75,-272.25,
	                           -261.75,-251.25,-240.75,-230.25,-219.75,-209.25,-198.75,-188.25,-177.75,-167.25,-156.75,-146.25,-135.75,-125.25,-114.75,-104.25,
	                           -93.75,-83.25,-72.75,-62.25,-51.75,-41.25,-30.75,-20.25,-9.75,0.75,11.25,21.75,32.25,42.75,53.25,63.75,74.25,84.75,95.25,105.75,
	                           116.25,126.75,137.25,147.75,158.25,168.75,179.25,189.75,200.25,210.75,221.25,231.75,242.25,252.75,263.25,273.75,284.25,294.75,
	                           305.25,315.75,326.25,336.75,347.25,357.75,368.25,378.75,389.25,399.75};//X position measuring
	Double_t x_down_z[80];
	std::fill_n(x_down_z,80,-179);

	TVector3 point,direction;
	UInt_t gid;
	for(int i=0;i<80;i++){
		gid=Encoding::Encode(EMWDC,EDOWN,EX,i);
		point.SetXYZ(x_down_position[i],0,x_down_z[i]);
		direction.SetXYZ(0,1,0);
		fPointMap[gid]=point;
		fDirectionMap[gid]=direction;
	}
}

void GeometryInfo::InitDownY()
{
	Double_t y_down_position[80]={-414.75,-404.25,-393.75,-383.25,-372.75,-362.25,-351.75,-341.25,-330.75,-320.25,-309.75,-299.25,-288.75,-278.25,-267.75,-257.25,
	                           -246.75,-236.25,-225.75,-215.25,-204.75,-194.25,-183.75,-173.25,-162.75,-152.25,-141.75,-131.25,-120.75,-110.25,-99.75,-89.25,
	                           -78.75,-68.25,-57.75,-47.25,-36.75,-26.25,-15.75,-5.25,5.25,15.75,26.25,36.75,47.25,57.75,68.25,78.75,
	                           89.25,99.75,110.25,120.75,131.25,141.75,152.25,162.75,173.25,183.75,194.25,204.75,215.25,225.75,236.25,246.75,
	                           257.25,267.75,278.25,288.75,299.25,309.75,320.25,330.75,341.25,351.75,362.25,372.75,383.25,393.75,404.25,414.75};//Y position measuring
	Double_t y_down_z[80];
	std::fill_n(y_down_z,80,-164);

	TVector3 point,direction;
	UInt_t gid;
	for(int i=0;i<80;i++){
		gid=Encoding::Encode(EMWDC,EDOWN,EY,i);
		point.SetXYZ(0,y_down_position[i],y_down_z[i]);
		direction.SetXYZ(1,0,0);
		fPointMap[gid]=point;
		fDirectionMap[gid]=direction;
	}
}

void GeometryInfo::InitDownZ()
{
	// init_point is the DownU_106
	TVector3 init_point(368.0748,438.4964,-149);
	TVector3 wire_direction(0.5,-TMath::Sqrt(3.0)/2.0,0);
	TVector3 moving_direction(-TMath::Sqrt(3.0)/2.0,-0.5,0);

	TVector3 point;
	UInt_t gid;
	Double_t wire_sparation=10.5;
	for(int i=0;i<106;i++){
		gid=Encoding::Encode(EMWDC,EDOWN,EU,105-i);
		point=init_point + wire_sparation*i*moving_direction;
		fPointMap[gid]=point;
		fDirectionMap[gid]=wire_direction;
	}
}