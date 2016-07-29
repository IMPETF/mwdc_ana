#ifndef _TrackFit_
#define _TrackFit_

#include <Math/IFunction.h>
#include <TVector3.h>
#include <map>
#include <TMath.h>
#include "GeometryInfo.h"

namespace TrackFit{

// line to represent any line not parrallel to x-y plane
class Line
{
public:
	Line(): fx0(0,0,0),fx1(1,1,1){ 
		Standardize(); 
	}
	Line(const double* p) : fx0(p[0],p[2],0), fx1(p[0]+p[1],p[2]+p[3],1), fdirection(p[1],p[3],1) { funitdirection= fdirection.Unit();}
	Line(TVector3 point,TVector3 direction,Bool_t flag=true){
		if(flag){//point + direction
			fx0=point;
			fx1=fx0+direction;
		}
		else{//point + point
			fx0=point;
			fx1=direction;
		}
		Standardize();
	}
	~Line() {}
	
	void Reset(const double* p){
		fx0.SetXYZ(p[0],p[2],0);
		fx1.SetXYZ(p[0]+p[1],p[2]+p[3],1);
		fdirection.SetXYZ(p[1],p[3],1);
		funitdirection= fdirection.Unit();
	}

	void Reset(TVector3 point,TVector3 direction,Bool_t flag=true){
		printf("\nin line\n");
		point.Print();direction.Print();
		if(flag){
			fx0=point;
			fx1=fx0+direction;
		}
		else{
			fx0=point;
			fx1=direction;
		}
		Standardize();
	}

	TVector3 GetPoint(Double_t t){
		return fx0 + t*fdirection;
	}

	void GetParameter(Double_t* p){
		p[0]=fx0.X();
		p[1]=fdirection.X();
		p[2]=fx0.Y();
		p[3]=fdirection.Y();
	}

	TVector3 GetUnitDirection(){
		return funitdirection;
	}

	Double_t DistanceToPoint(TVector3 point){
		TVector3 tmp= point-fx0;
		return tmp.Cross(funitdirection).Mag();
	}

	Double_t DistanceToPoint2(TVector3 point){
		TVector3 tmp= point-fx0;
		return tmp.Cross(funitdirection).Mag2();
	}

	TVector3 Cross(Line line){
		TVector3 dir= fdirection.Cross(line.GetUnitDirection());
		return dir.Unit();
	}

	Double_t DistanceToLine(Line line){
		TVector3 dir=Cross(line);

		TVector3 tmp=line.GetPoint(1)-fx0;
		return TMath::Abs(tmp*dir);
	}	

private:
	void Standardize(){
		if(fx1.Z() == fx0.Z()){
			printf("ERROR: the class line can't represent line parrallel to x-y plane, %.2f, %.2f\n",fx1.Z(),fx0.Z());
			exit(1);
		}

		Double_t t=fx1.Z()/(fx1.Z()-fx0.Z());
		Double_t x0=fx1.X() - t*(fx1.X()-fx0.X());
		Double_t y0=fx1.Y() - t*(fx1.Y()-fx0.Y());
		Double_t z0=0;
		// 
		t=(fx1.Z()-1)/(fx1.Z()-fx0.Z());
		Double_t x1=fx1.X() - t*(fx1.X()-fx0.X());
		Double_t y1=fx1.Y() - t*(fx1.Y()-fx0.Y());
		Double_t z1=1;
		// 
		fx0.SetXYZ(x0,y0,z0);
		fx1.SetXYZ(x1,y1,z1);
		fdirection=fx1-fx0;
		funitdirection=fdirection.Unit();
	}

	TVector3 fx0;
	TVector3 fx1;
	TVector3 fdirection;
	TVector3 funitdirection;
};

class LineFit: public ROOT::Math::IBaseFunctionMultiDim
{
public:
	LineFit(GeometryInfo info): fWirePositions(info) {}
	LineFit() {}
	~LineFit() {}
	// 
	ROOT::Math::IBaseFunctionMultiDim* Clone() const {
		return new LineFit(*this);
	}
	unsigned int NDim() const { return 4; }

	//  
	void AddHit(UInt_t gid, Double_t distance){
		fDistances[gid]=distance;
	}

	void Reset(){
		fDistances.clear();
	}

private:
	double DoEval(const double * p) const {
		Line track(p);
		Line wire;
		// 
		Double_t sum=0;
		Int_t size=fDistances.size();
		assert(size==6);
		// 
		std::map<UInt_t,Double_t>::const_iterator it;
		UInt_t gid;
		Double_t distance,tmpdistance;
		for(it = fDistances.begin();it!=fDistances.end();++it){
			gid=it->first;distance=it->second;
			// 
			wire.Reset(fWirePositions.GetPoint(gid),fWirePositions.GetDirection(gid));
			// 
			tmpdistance=track.DistanceToLine(wire);

			sum+=TMath::Power(tmpdistance-distance,2);
		}

		return sum;
	}


private:
	std::map<UInt_t, Double_t> fDistances;//gid -> drift_distance
	GeometryInfo               fWirePositions;
};

}

#endif