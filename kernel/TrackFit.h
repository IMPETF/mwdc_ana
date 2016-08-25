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
		// printf("\nin line\n");
		// point.Print();direction.Print();
		if(flag){
			fx0=point;
			fx1=fx0+direction;
		}
		else{
			fx0=point;
			fx1=direction;
		}
		Standardize();
		// fx0.Print();fdirection.Print();
	}

	void Reset_Unstandard(TVector3 point,TVector3 direction,Bool_t flag=true){//to represent wires, which are parrallel to the x-y plane
		// printf("\nin line\n");
		// point.Print();direction.Print();
		if(flag){
			fx0=point;
			fx1=fx0+direction;
			fdirection=direction;
			funitdirection=fdirection.Unit();
		}
		else{
			fx0=point;
			fx1=direction;
			fdirection=fx1-fx0;
			funitdirection=fdirection.Unit();
		}
		// fx0.Print();fdirection.Print();
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

	Double_t DistanceToPoint(TVector3& point){
		TVector3 tmp= point-fx0;
		return tmp.Cross(funitdirection).Mag();
	}

	Double_t DistanceToPoint2(TVector3& point){
		TVector3 tmp= point-fx0;
		return tmp.Cross(funitdirection).Mag2();
	}

	Bool_t IsParallel(Line& line){
		TVector3 direction=line.GetUnitDirection();
		if(direction == funitdirection)
			return true;
		else
			return false;
	}

	TVector3 Cross(Line& line){
		TVector3 dir= fdirection.Cross(line.GetUnitDirection());
		return dir.Unit();
	}

	Double_t DistanceToLine(Line& line){

		if(IsParallel(line)){
			TVector3 tmp=line.GetPoint(1)-fx0;
			TVector3 length=funitdirection.Cross(tmp);
			return length.Mag();
		}
		else{
			TVector3 dir=Cross(line);
			TVector3 tmp=line.GetPoint(1)-fx0;
			return TMath::Abs(tmp*dir);
		}
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

		return;
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
		fHittedwires[gid]=Line();
		fHittedwires[gid].Reset_Unstandard(fWirePositions.GetPoint(gid),fWirePositions.GetDirection(gid));
	}

	void Reset(){
		fDistances.clear();
		fHittedwires.clear();
		fDistances_Fitted.clear();
		fResiduals.clear();
		// fFlag_Init=true;
	}

	void CalcResiduals(Line& track){// residual = fitted - original
		fDistances_Fitted.clear();
		fResiduals.clear();
		// 
		Double_t tmpdistance;
		for(fIterator = fHittedwires.begin();fIterator != fHittedwires.end();++fIterator){
			tmpdistance=track.DistanceToLine(fIterator->second);
			fDistances_Fitted[fIterator->first]=tmpdistance;
			fResiduals[fIterator->first]=tmpdistance- fDistances[fIterator->first];
		}
	}

	void CalcResiduals(Double_t *p){
		track.Reset(p);
		// 
		fDistances_Fitted.clear();
		fResiduals.clear();
		// 
		Double_t tmpdistance;
		for(fIterator = fHittedwires.begin();fIterator != fHittedwires.end();++fIterator){
			tmpdistance=track.DistanceToLine(fIterator->second);
			fDistances_Fitted[fIterator->first]=tmpdistance;
			fResiduals[fIterator->first]=tmpdistance- fDistances[fIterator->first];
		}
	}

	std::map<UInt_t, Double_t> GetResiduals(){
		return fResiduals;
	}

	std::map<UInt_t, Double_t> GetFittedDistances(){
		return fDistances_Fitted;
	}

private:
	double DoEval(const double * p) const {
		Line track(p)
		// printf("in LineFit\n");

		// 
		Double_t sum=0;
		// Int_t size=fDistances.size();
		// assert(size==6);
		// 		
		Double_t tmpdistance;
		for(fIterator = fDistances.begin();fIterator!=fDistances.end();++fIterator){
			tmpdistance=track.DistanceToLine(fHittedwires.at(fIterator->first));

			sum+=TMath::Power(tmpdistance - fIterator->second,2);
			// printf("%.4f(%.4f)\n", tmpdistance,distance);
		}
		return sum;
	}


private:
	Line track;

	std::map<UInt_t, Line>     fHittedwires;
	std::map<UInt_t, Double_t> fDistances;//gid -> drift_distance
	std::map<UInt_t, Double_t> fDistances_Fitted;
	std::map<UInt_t, Double_t> fResiduals;
	std::map<UInt_t,Double_t>::const_iterator fIterator;

	GeometryInfo               fWirePositions;
};

}

#endif