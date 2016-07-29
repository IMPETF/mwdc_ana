#include <Math/IFunction.h>
#include <TVector3.h>
#include <vector>
#include <TMath.h>
#include <TGraph2D.h>
#include <TRandom2.h>
#include <TStyle.h>
#include <TCanvas.h>
#include <Fit/Fitter.h>
#include <TPolyLine3D.h>
#include "TrackFit.h"


class Line
{
public:
	Line(): fx0(0,0,0),fx1(1,1,1),fdirection(1,1,1), funitdirection(1,1,1) {}
	Line(const double* p) : fx0(p[0],p[2],0), fx1(p[0]+p[1],p[2]+p[3],1), fdirection(p[1],p[3],1) { funitdirection= fdirection.Unit();}
	~Line() {}
	
	void Reset(const double* p){
		fx0.SetXYZ(p[0],p[2],0);
		fx1.SetXYZ(p[0]+p[1],p[2]+p[3],1);
		fdirection.SetXYZ(p[1],p[3],1);
		funitdirection= fdirection.Unit();
	}

	TVector3 GetPoint(Double_t t){
		return fx0 + t*fdirection;
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
	TVector3 fx0;
	TVector3 fx1;
	TVector3 fdirection;
	TVector3 funitdirection;
};

bool first = true; 

class LineFit: public ROOT::Math::IBaseFunctionMultiDim
{
public:
	LineFit() {}
	~LineFit() {}
	
	ROOT::Math::IBaseFunctionMultiDim* Clone() const {
		return new LineFit(*this);
	}
	unsigned int NDim() const { return 4; }

	void AddPoint(TVector3 newpoint){
		fPoints.push_back(newpoint);
	}

	void Reset(){
		fPoints.clear();
	}

private:
	double DoEval(const double * p) const {
		Line line(p);

		Double_t sum=0;
		Int_t size=fPoints.size();
		for(int i=0;i<size;i++){
			sum+=line.DistanceToPoint2(fPoints[i]);
		}

		if (first)
		   std::cout << "Total Initial distance square = " << sum << std::endl;
		first = false;

		return sum;
	}


private:
	std::vector<TVector3> fPoints;
};



Int_t debug_fit_track()
{
   gStyle->SetOptStat(0);
   gStyle->SetOptFit();


   //double e = 0.1;
   Int_t nd = 1000;

   ROOT::Fit::Fitter  fitter;
   LineFit linefit;

   // Fill the 2D graph
   double p0[4] = {10+9894*0.5,20+9894*0.5,1+9894*0.5,2+9894*0.5};
   // set the function and the initial parameter values
   double pStart[4] = {1,1,1,1};
   double p0_step=0.5;
   Line line;

   TVector3 tmppoint;
   for(int round_id=0;round_id<1;round_id++){
   	printf("round_id=%d\n", round_id+1);
   	for(int j=0;j<4;j++){
   		p0[j]+=p0_step;
   		pStart[j]=p0[j]+10;
   	}
   	line.Reset(p0);
   	// generate graph with the 3d points
   	linefit.Reset();
   	for (Int_t N=0; N<nd; N++) {
   	   double x,y,z = 0;
   	   // Generate a random number 
   	   double t = gRandom->Uniform(0,10);
   	   tmppoint=line.GetPoint(t);
   	   x=tmppoint.X();y=tmppoint.Y();z=tmppoint.Z();

   	   double err = 1;
   	 // do a gaussian smearing around the points in all coordinates
   	   x += gRandom->Gaus(0,err);  
   	   y += gRandom->Gaus(0,err);  
   	   z += gRandom->Gaus(0,err);  

   	   linefit.AddPoint(TVector3(x,y,z));
   	}
   	
   	// fitting
   	fitter.SetFCN(linefit,pStart,nd);
   	// set step sizes different than default ones (0.3 times parameter values)
   	for (int i = 0; i < 4; ++i) fitter.Config().ParSettings(i).SetStepSize(0.01);
   	bool ok = fitter.FitFCN();
   	if (!ok) {
   	   Error("line3Dfit","Line3D Fit failed");
   	   return 1;
   	}

   	const ROOT::Fit::FitResult & result = fitter.Result();
   	std::cout << "Total final distance square " << result.MinFcnValue() << std::endl;
   	result.Print(std::cout);
   }
   
   // get fit parameters
   // const double * parFit = result.GetParams();


   return 0;
}

void trackfit()
{
	TrackFit::LineFit linefit;
}