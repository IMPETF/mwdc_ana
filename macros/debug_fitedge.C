#include "TTree.h"
#include "TFile.h"
#include "TROOT.h"
#include "TCanvas.h"
#include "TH1F.h"
#include "utility.h"
#include "TMath.h"
#include "TStyle.h"
#include "TF1.h"
#include "TString.h"
#include <map>
#include "global.h"
#include "TLine.h"
#include "TH2F.h"

TF1* fit_rising(TH1* hinput,const char* options,double low=-30,double high=40,Int_t type=0)
{
	TString FunName;
	FunName.Form("frising_%s_type%d_range(%.1f,%.2f)",hinput->GetName(),type,low,high);
	TF1 *f_rising = (TF1*)gROOT->GetListOfFunctions()->FindObject(FunName.Data());
	if (f_rising) delete f_rising;

	switch(type){
		case 0:
			f_rising=new TF1(FunName.Data(),"[0]+[1]*TMath::Exp(-(x-[3])/[2])/(1+TMath::Exp(-(x-[4])/[5]))",low,high);
			f_rising->SetParName(0,"p0");
			f_rising->SetParLimits(0,0,10);
			f_rising->SetParameter(0,1);
			f_rising->SetParName(1,"A0");
			f_rising->SetParameter(1, hinput->GetBinContent(hinput->GetMaximumBin()));
			f_rising->SetParName(2,"T1");
			f_rising->SetParameter(2,50);
			f_rising->SetParName(3,"t1");
			f_rising->SetParameter(3,3);
			f_rising->SetParName(4,"t0");
			f_rising->SetParameter(4,2);
			f_rising->SetParName(5,"T0");
			f_rising->SetParameter(5,3);
			break;
		case 1:
			f_rising=new TF1(FunName.Data(),"[0]*TMath::Exp(-(x-[2])/[1])/(1+TMath::Exp(-(x-[3])/[4]))",low,high);
			f_rising->SetParName(0,"A0");
			f_rising->SetParameter(0, hinput->GetBinContent(hinput->GetMaximumBin()));
			f_rising->SetParName(1,"T1");
			f_rising->SetParameter(1,50);
			f_rising->SetParName(2,"t1");
			f_rising->SetParameter(2,3);
			f_rising->SetParName(3,"t0");
			f_rising->SetParameter(3,2);
			f_rising->SetParName(4,"T0");
			f_rising->SetParameter(4,3);
			break;
	}
	
	hinput->Fit(f_rising,options,"",low,high);

	return f_rising;
}

TF1* fit_falling(TH1* hinput,const char* options,double low=80,double high=150,Int_t type=0)
{
	TString FunName;
	FunName.Form("ffalling_%s_type%d_range(%.1f,%.2f)",hinput->GetName(),type,low,high);
	TF1 *f_falling = (TF1*)gROOT->GetListOfFunctions()->FindObject(FunName.Data());
	if (f_falling) delete f_falling;

	switch(type){
		case 0:
			f_falling=new TF1("f_falling","[0]+([1]*x+[2])/(1+TMath::Exp((x-[3])/[4]))",low,high);
			f_falling->SetParName(0,"pm");
			f_falling->SetParameter(0,0);
			f_falling->SetParLimits(0,0,10);
			f_falling->SetParName(1,"#alpha_{m}");
			f_falling->SetParameter(1,-0.5);
			f_falling->SetParLimits(1,-10,0.1);
			f_falling->SetParName(2,"Am");
			f_falling->SetParameter(2,0.3*hinput->GetBinContent(hinput->GetMaximumBin()));
			f_falling->SetParName(3,"tm");
			f_falling->SetParameter(3,80);
			f_falling->SetParName(4,"Tm");
			f_falling->SetParameter(4,20);
			break;
		case 1:
			f_falling=new TF1("f_falling","([0]*x+[1])/(1+TMath::Exp((x-[2])/[3]))",low,high);
			f_falling->SetParName(0,"#alpha_{m}");
			f_falling->SetParameter(0,-0.5);
			f_falling->SetParLimits(0,-10,0.1);
			f_falling->SetParName(1,"Am");
			f_falling->SetParameter(1,0.3*hinput->GetBinContent(hinput->GetMaximumBin()));
			f_falling->SetParName(2,"tm");
			f_falling->SetParameter(2,80);
			f_falling->SetParName(3,"Tm");
			f_falling->SetParameter(3,20);
			break;
		case 2:
			f_falling=new TF1("f_falling","[0]/(1+TMath::Exp((x-[1])/[2]))",low,high);
			f_falling->SetParName(0,"Am");
			f_falling->SetParameter(0,0.3*hinput->GetBinContent(hinput->GetMaximumBin()));
			f_falling->SetParName(1,"tm");
			f_falling->SetParameter(1,80);
			f_falling->SetParName(2,"Tm");
			f_falling->SetParameter(2,20);
			break;
		case 3:
			f_falling=new TF1("f_falling","[0]+[1]/(1+TMath::Exp((x-[2])/[3]))",low,high);
			f_falling->SetParName(0,"pm");
			f_falling->SetParameter(0,0);
			f_falling->SetParLimits(0,0,10);
			f_falling->SetParName(1,"Am");
			f_falling->SetParameter(1,0.3*hinput->GetBinContent(hinput->GetMaximumBin()));
			f_falling->SetParName(2,"tm");
			f_falling->SetParameter(2,80);
			f_falling->SetParName(3,"Tm");
			f_falling->SetParameter(3,20);
			break;
	}
	
	hinput->Fit(f_falling,options,"",low,high);

	return f_falling;
}

const char file[4][20]={"drifttime_1.root","drifttime_2.root","drifttime_3.root","drifttime_sum.root"};
const char hist[4][40]={"h1d_minrising_drifttime_mwdc_Down_X_3"
						,"h1d_minrising_drifttime_mwdc_Down_X_40"
						,"h1d_minrising_drifttime_mwdc_Down_Y_44"
						,"h1d_Down_Y_minrising_drifttime"};

void debug_fitedge_rising(const char* filename, const char* hname)
{
	TFile* fin=new TFile(filename);
	TH1F* hinput=(TH1F*)fin->Get(hname);
	hinput->SetDirectory(0);

	TCanvas* c=(TCanvas*)gROOT->FindObject("c");
	if(!c){
		c=new TCanvas("c","c");
	}
	c->cd();
	c->SetLogy();

	hinput->Draw();
	TF1* ffit_rising0=fit_rising(hinput,"NB",-30,40,1);
	ffit_rising0->SetLineColor(kBlack);
	ffit_rising0->Draw("same");

	TF1* ffit_rising1=fit_rising(hinput,"NB",-30,30,1);
	ffit_rising1->SetLineColor(kRed);
	ffit_rising1->Draw("same");

	delete fin;
}

void debug_fitedge_falling(const char* filename, const char* hname)
{
	TFile* fin=new TFile(filename);
	TH1F* hinput=(TH1F*)fin->Get(hname);
	hinput->SetDirectory(0);

	TCanvas* c=(TCanvas*)gROOT->FindObject("c");
	if(!c){
		c=new TCanvas("c","c");
	}
	c->cd();
	// c->SetLogy();

	hinput->Draw();
	TF1* ffit_falling0=fit_falling(hinput,"NB",60,150,0);
	ffit_falling0->SetLineColor(kBlack);
	ffit_falling0->SetRange(50,450);
	ffit_falling0->Draw("same");

	TF1* ffit_falling1=fit_falling(hinput,"NB",70,150,0);
	ffit_falling1->SetLineColor(kRed);
	ffit_falling1->SetRange(50,450);
	ffit_falling1->Draw("same");

	TF1* ffit_falling2=fit_falling(hinput,"NB",80,150,0);
	ffit_falling2->SetLineColor(kGreen);
	ffit_falling2->SetRange(50,450);
	ffit_falling2->Draw("same");

	// TF1* ffit_falling3=fit_falling(hinput,"NB",90,150,0);
	// ffit_falling3->SetLineColor(kYellow);
	// ffit_falling3->SetRange(50,450);
	// ffit_falling3->Draw("same");

	delete fin;
}