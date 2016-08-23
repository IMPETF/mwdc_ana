#include <vector>
#include "TGraph.h"
#include "TFile.h"
#include "TCanvas.h"
#include "TF1.h"
#include "TH1F.h"
#include "TSpline.h"
#include "Math/ChebyshevPol.h"
#include "Math/Polynomial.h"
#include <iostream>
// class Chebyshev {
// public: 
//    Chebyshev(int n, double xmin, double xmax) : 
//       fA(xmin), fB(xmax),
//       fT(std::vector<double>(n) )  {}

//    double operator() (const double * xx, const double *p) { 
//       double x = (2.0*xx[0] - fA -fB)/(fB-fA);
//       int order = fT.size(); 
//       if (order == 1) return p[0]; 
//       if (order == 2) return p[0] + x*p[1]; 
//       // build the polynomials
//       fT[0] = 1;
//       fT[1] = x; 
//       for (int i = 1; i< order; ++i) { 
//          fT[i+1] =  2 *x * fT[i] - fT[i-1]; 
//       }
//       double sum = p[0]*fT[0]; 
//       for (int i = 1; i<= order; ++i) { 
//          sum += p[i] * fT[i]; 
//       }
//       return sum; 
//    }

// private: 
//    double fA; 
//    double fB; 
//    std::vector<double> fT; // polynomial
//    std::vector<double> fC; // coefficients
// };


// void  exampleFitChebyshev() { 

//    TH1 * h1 = new TH1D("h1","h1",100,-2,2);
//    h1->FillRandom("gaus");

//    double xmin = -2; double xmax = 2; 
//    double n = 4; 
//    Chebyshev * cheb = new Chebyshev(n,xmin,xmax);
//    TF1 * f1 = new TF1("f1",cheb,xmin,xmax,n+1,"Chebyshev");
//    for (int i = 0; i <=n; ++i) f1->SetParameter(i,1);  

//    h1->Fit(f1); 
// }

const char file[6][30]={"drifttime_down_x_40.root","drifttime_down_y_40.root","drifttime_down_u_50.root"
						,"drifttime_up_x_40.root","drifttime_up_y_40.root","drifttime_up_u_50.root"};
const char hist[6][40]={"h1d_minrising_drifttime_mwdc_Down_X_40"
						,"h1d_minrising_drifttime_mwdc_Down_Y_40"
						,"h1d_minrising_drifttime_mwdc_Down_U_50"
						,"h1d_minrising_drifttime_mwdc_Up_X_40"
						,"h1d_minrising_drifttime_mwdc_Up_Y_40"
						,"h1d_minrising_drifttime_mwdc_Up_U_50"};

using namespace std;

void get_rt_chebyshev(Int_t order)
{
	TFile* f=new TFile("drifttime_down_x_40.root");
	TH1F* h1=(TH1F*)f->Get("h1d_minrising_drifttime_mwdc_Down_X_40");
	// h1->SetDirectory(0);
	assert(h1);

	Int_t t0_bin,tmax_bin,binnum;
	t0_bin=h1->FindBin(0);
	tmax_bin=h1->FindBin(400);
	binnum=tmax_bin-t0_bin+1;

	Double_t area,integral,tmp_s,tmp_t;
	Double_t half_cell_length=5.5;// unit: mm
	Double_t half_diagnol_length=7.433;
	area=h1->Integral(t0_bin,tmax_bin);

	TGraph* g_rt=new TGraph();
	g_rt->SetName(Form("grt_%s",h1->GetName()));
	g_rt->SetTitle(Form("grt_%s",h1->GetTitle()));
	g_rt->SetPoint(0,0,0);
	for(int i=0;i<binnum;i++){
		integral=h1->Integral(t0_bin,t0_bin+i);

		tmp_t= h1->GetBinCenter(t0_bin+i);
		tmp_s= half_diagnol_length * integral / area;
		g_rt->SetPoint(i+1,tmp_t,tmp_s);
	}
	g_rt->Draw("A*");

	double xmin = -10; double xmax = 450; 
	ROOT::Math::ChebyshevPol * cheb = new ROOT::Math::ChebyshevPol(order);
	TF1 * f1 = new TF1("f1",cheb,xmin,xmax,order+1,"Chebyshev");
	for (int i = 0; i <=order; ++i) 
		f1->SetParameter(i,1);  
	g_rt->Fit(f1,"R","",0,400);	
	f1->SetRange(xmin,xmax);
	f1->Draw("same");

	cout<<"0:"<<f1->Eval(0)<<"\t"<<"400:"<<f1->Eval(400)<<endl;
}

// pol5 + pol1, total free parameters 8
Double_t Pol(Double_t* x, Double_t* par)
{
	ROOT::Math::Polynomial pol5(5);

	pol5.SetParameters(par);

	Double_t x0=par[6];
	Double_t y0=pol5(par[6]);
	ROOT::Math::Polynomial pol1(par[7],y0-par[7]*x0);

	if(x[0]<=x0){
		return pol5(x[0]);
	}
	else{
		return pol1(x[0]);
	}
}

void get_rt_pol(int index)
{
	TFile* f=new TFile(file[index]);
	TH1F* h1=(TH1F*)f->Get(hist[index]);
	// h1->SetDirectory(0);
	assert(h1);

	Int_t t0_bin,tmax_bin,binnum;
	t0_bin=h1->FindBin(0);
	tmax_bin=h1->FindBin(400);
	binnum=tmax_bin-t0_bin+1;

	Double_t area,integral,tmp_s,tmp_t;
	Double_t half_cell_length=5.5;// unit: mm
	Double_t half_diagnol_length=7.433;
	area=h1->Integral(t0_bin,tmax_bin);

	TGraph* g_rt=new TGraph();
	g_rt->SetName(Form("grt_%s",h1->GetName()));
	g_rt->SetTitle(Form("grt_%s",h1->GetTitle()));
	g_rt->SetPoint(0,0,0);
	for(int i=0;i<binnum;i++){
		integral=h1->Integral(t0_bin,t0_bin+i);

		tmp_t= h1->GetBinCenter(t0_bin+i);
		tmp_s= half_diagnol_length * integral / area;
		g_rt->SetPoint(i+1,tmp_t,tmp_s);
	}
	g_rt->Draw("A*");

	double xmin = -10; double xmax = 450; 
	TF1 * f1 = new TF1("f1",Pol,xmin,xmax,8,"Pol");
	
	// Double_t p0_start[8];
	// g_rt->Fit("pol5");
	// g_rt->GetFunction("pol5")->GetParameters(p0_start);
	// p0_start[6]=140;
	// p0_start[7]=0.0006;
	// f1->SetParameters(p0_start);

	// for(int i=0;i<8;i++){
	// 	f1->SetParameter(i,1);
	// }
	// f1->SetParameter(6,140);
	for (int i = 0; i <8; ++i) 
		f1->SetParameter(i,1);
	// f1->SetParameter(0,0);
	f1->SetParameter(6,140);
	f1->SetParLimits(6,120,160);
	// f1->SetParameter(7,(7.433-5.5)/260.);  

	g_rt->Fit(f1,"","",0,400);	
	f1->SetRange(xmin,xmax);
	f1->Draw("same");

	cout<<"0:"<<f1->Eval(0)<<"\t"<<"400:"<<f1->Eval(400)<<endl;
}

void get_rt_seed(const char* filename, const char* h1dname, TGraph& g_output,TSpline5& spline_output)
{
	TFile* f=new TFile(filename);
	TH1F* h1=(TH1F*)f->Get(h1dname);
	// h1->SetDirectory(0);
	assert(h1);

	Int_t t0_bin,tmax_bin,binnum;
	t0_bin=h1->FindBin(0);
	tmax_bin=h1->FindBin(400);
	binnum=tmax_bin-t0_bin+1;

	Double_t area,integral,tmp_s,tmp_t;
	Double_t half_cell_length=5.5;// unit: mm
	Double_t half_diagnol_length=7.433;
	area=h1->Integral(t0_bin,tmax_bin);

	TGraph* g_rt=new TGraph();
	g_rt->SetName(Form("grt_%s",h1->GetName()));
	g_rt->SetTitle(Form("grt_%s",h1->GetTitle()));
	// g_rt->SetPoint(0,0,0);
	for(int i=0;i<binnum;i++){
		integral=h1->Integral(t0_bin,t0_bin+i);

		tmp_t= h1->GetBinCenter(t0_bin+i);
		tmp_s= half_diagnol_length * integral / area;
		g_rt->SetPoint(i,tmp_t,tmp_s);
	}

	// Int_t degree[7]={6,7,8,9,10,11,12};
	// TF1* fpoly[7];
	// for(int i=0;i<7;i++){
	// 	fpoly[i]=new TF1(Form("fpoly%d",degree[i]),Form("pol%d",degree[i]),0,400);
	// 	g_rt->Fit(fpoly[i],"R0","",0,400);
	// 	fpoly[i]->SetRange(-20,450);
	// }

	// TCanvas* c=new TCanvas("c","c");
	// TH2F* h2axis=new TH2F("h2axis","h2axis",10,-20,450,10,-1.5,7.5);
	// h2axis->Draw();
	// c->DrawFrame(-20,-1.5,450,7.5);
	// h1->Draw();
	// g_rt->Draw("*");

	TSpline5* s3=new TSpline5(Form("spline_%s",h1->GetTitle()),g_rt);
	s3->SetName(Form("spline_%s",h1->GetName()));
	s3->SetLineColor(kRed);
	// s3->Draw("same");

	printf("%.2f, %.2f\n",s3->Eval(0),s3->Eval(450));

	g_output=(*g_rt);
	spline_output=(*s3);
	// double xmin = -10; double xmax = 450; 
	// double n = 7; 
	// Chebyshev * cheb = new Chebyshev(n,xmin,xmax);
	// TF1 * f1 = new TF1("f1",cheb,xmin,xmax,n+1,"Chebyshev");
	// for (int i = 0; i <=n; ++i) f1->SetParameter(i,1);  
	// g_rt->Fit(f1,"R","",0,400);	
	// f1->SetRange(xmin,xmax);
	// f1->Draw("same");
	
	// for(int i=0;i<7;i++){
	// 	fpoly[i]->SetLineColor(2+i);
	// 	fpoly[i]->Draw("same");
	// }
	// c->BuildLegend();

	delete f;
}

void get_rt_test(const char* datadir,const char* outfile)
{
	TString file_data=TString(datadir)+"/"+outfile;  
	TFile* file_drifttime=new TFile(file_data,"update");
	if(!file_drifttime){
	  printf("open file error: %s\n",outfile);
	  exit(1);
	}
	// 
	TDirectory* dir_minrising=file_drifttime->GetDirectory("minrising_drifttime");
	if(!dir_minrising){
	  dir_minrising=file_drifttime->mkdir("minrising_drifttime");
	  if(!dir_minrising){
	    printf("error!can't mkdir \"minrising_drifttime\" in %s\n",file_data.Data());
	    exit(1);
	  }
	  dir_minrising=file_drifttime->GetDirectory("minrising_drifttime");
	}
	dir_minrising->cd();

	TDirectory* dir_trackfit=dir_minrising->GetDirectory("trackfit_test");
	if(!dir_trackfit){
	  dir_trackfit=dir_minrising->mkdir("trackfit_test");
	  if(!dir_trackfit){
	    printf("error!can't mkdir \"trackfit_test\" in %s\n",outfile);
	    exit(1);
	  }
	  dir_trackfit=dir_minrising->GetDirectory("trackfit_test");
	}
	dir_trackfit->cd();

	TDirectory* dir_rtseed=dir_trackfit->GetDirectory("seed");
	if(!dir_rtseed){
	  dir_rtseed=dir_trackfit->mkdir("seed");
	  if(!dir_rtseed){
	    printf("error!can't mkdir \"seed\" in %s\n",outfile);
	    exit(1);
	  }
	  dir_rtseed=dir_trackfit->GetDirectory("seed");
	}
	dir_rtseed->cd();
	//
	TGraph gr;
	TSpline5 spline;
	for(int i=0;i<6;i++){
		get_rt_seed(file[i],hist[i],gr,spline);
		dir_rtseed->cd();
		gr.Write(0,TObject::kOverwrite);
		spline.Write(0,TObject::kOverwrite);
	}

	delete file_drifttime;
}