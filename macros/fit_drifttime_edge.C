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
#include "TGraph.h"

TF1* fit_rising(TH1* hinput,const char* options,double low=-30,double high=40,Int_t type=0)
{
	TString FunName;
	FunName.Form("frising_%s",hinput->GetName());
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
	FunName.Form("ffalling_%s",hinput->GetName());
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

void fit_drifttime_edge(const char* datadir,const char* outfile)
{
	gStyle->SetOptStat(111111);
	// new histograms
	TH1F* h1d_minrising_drifttime_mwdc[g_mwdc_location][g_mwdc_wireplane];
	std::map<UInt_t,TH1F*> h1d_minrising_drifttime_mwdc_single;

	// const Int_t rising_nparams=6;
	// TString rising_paranames[rising_nparams]={"p0","A0","T1","t1","t0","T0"};
	// Int_t rising_para_nbin[rising_nparams]={200,1400,100,60,60,80};
	// Double_t rising_para_lowbin[rising_nparams]={-16,0,0,-15,-10,-20};
	// Double_t rising_para_highbin[rising_nparams]={4,14000,100,15,15,20};
	const Int_t rising_nparams=5;
	TString rising_paranames[rising_nparams]={"A0","T1","t1","t0","T0"};
	Int_t rising_para_nbin[rising_nparams]={1400,120,40,60,60};
	Double_t rising_para_lowbin[rising_nparams]={0,20,-5,-10,-5};
	Double_t rising_para_highbin[rising_nparams]={14000,140,15,15,10};

	TH1F* h1d_rising_param[rising_nparams];
	for(int i=0;i<rising_nparams;i++){
		h1d_rising_param[i]=new TH1F(Form("h1d_rising_param_%s",rising_paranames[i].Data()),Form("h1d_rising_param_%s",rising_paranames[i].Data()),rising_para_nbin[i],rising_para_lowbin[i],rising_para_highbin[i]);
	}

	const Int_t falling_nparams=5;
	TString falling_paranames[falling_nparams]={"pm","#alpha_{m}","Am","tm","Tm"};
	Int_t falling_para_nbin[falling_nparams]={20,42,500,100,50};
	Double_t falling_para_lowbin[falling_nparams]={0,-10,0,50,0};
	Double_t falling_para_highbin[falling_nparams]={20,0.5,10000,150,50};
	
	// const Int_t falling_nparams=4;
	// TString falling_paranames[falling_nparams]={"#alpha_{m}","Am","tm","Tm"};
	// Int_t falling_para_nbin[falling_nparams]={40,500,150,50};
	// Double_t falling_para_lowbin[falling_nparams]={-1,0,0,0};
	// Double_t falling_para_highbin[falling_nparams]={1,10000,150,50};
	// 
	// const Int_t falling_nparams=3;
	// TString falling_paranames[falling_nparams]={"Am","tm","Tm"};
	// Int_t falling_para_nbin[falling_nparams]={500,150,50};
	// Double_t falling_para_lowbin[falling_nparams]={0,0,0};
	// Double_t falling_para_highbin[falling_nparams]={10000,150,50};
	TH1F* h1d_falling_param[falling_nparams];
	for(int i=0;i<falling_nparams;i++){
		h1d_falling_param[i]=new TH1F(Form("h1d_falling_param_%s",falling_paranames[i].Data()),Form("h1d_falling_param_%s",falling_paranames[i].Data()),falling_para_nbin[i],falling_para_lowbin[i],falling_para_highbin[i]);
	}

	TH1F* h1d_drifttime_interval=new TH1F("h1d_drifttime_interval","h1d_drifttime_interval",200,0,200);
	TH1F* h1d_drifttime_t0_minus_2T0=new TH1F("h1d_drifttime_t0_minus_2T0","h1d_drifttime_t0_minus_2T0",70,-20,15);
	TH1F* h1d_drifttime_tm_plus_2Tm=new TH1F("h1d_drifttime_tm_plus_2Tm","h1d_drifttime_tm_plus_2Tm",150,50,200);
	TH1F* h1d_drifttime_interval_new=new TH1F("h1d_drifttime_interval_new","h1d_drifttime_interval_new",150,50,200);
	TH2F* h2d_t0_vs_tm=new TH2F("h2d_t0_vs_tm","h2d_t0_vs_tm",60,-10,15,150,0,150);
	TH2F* h2d_t0_vs_tm_new=new TH2F("h2d_t0_vs_tm_new","h2d_t0_vs_tm_new",70,-20,15,150,50,200);
	TH2F* h2d_location_vs_tm=new TH2F("h2d_location_vs_tm","h2d_location_vs_tm",4,-1.5,2.5,100,50,150);
	TH2F* h2d_location_vs_t0=new TH2F("h2d_location_vs_t0","h2d_location_vs_t0",4,-1.5,2.5,60,-10,15);
	// read histograms
	TString file_data=TString(datadir)+"/"+outfile;  
	TFile* file_drifttime=new TFile(file_data,"update");
	if(!file_drifttime){
	  printf("open file error: %s\n",outfile);
	  exit(1);
	}

	for(int l=0;l<g_mwdc_location;l++){
	  for(int p=0;p<g_mwdc_wireplane;p++){
	  	file_drifttime->GetObject(Form("minrising_drifttime/h1d_%s_%s_minrising_drifttime",g_str_location[l],g_str_plane[p]),h1d_minrising_drifttime_mwdc[l][p]);
	  	if(!h1d_minrising_drifttime_mwdc[l][p]){
	  		printf("Can't read histogram\n");
	  		exit(1);
	  	}
	  	h1d_minrising_drifttime_mwdc[l][p]->SetDirectory(0);
	  }
	}
	for(int l=0;l<g_mwdc_location;l++){
	  for(int p=0;p<g_mwdc_wireplane;p++){
	      for(int w=0;w<g_mwdc_wireindex[p];w++){
	          UInt_t gid=Encoding::Encode(EMWDC,l,p,w);
	          file_drifttime->GetObject(Form("minrising_drifttime/histogram/h1d_minrising_drifttime_mwdc_%s_%s_%d",g_str_location[l],g_str_plane[p],w+1),h1d_minrising_drifttime_mwdc_single[gid]);
			  if(!h1d_minrising_drifttime_mwdc_single[gid]){
			  	printf("Can't read histogram\n");
			  	exit(1);
			  }	          
	        }
	    }
	}

	// fit histograms
	// broken wires
	UInt_t brokenwire_gid[11];
	// DownX
	brokenwire_gid[0]=Encoding::Encode(EMWDC,EDOWN,EX,45);
	brokenwire_gid[1]=Encoding::Encode(EMWDC,EDOWN,EX,46);
	brokenwire_gid[2]=Encoding::Encode(EMWDC,EDOWN,EX,47);
	brokenwire_gid[3]=Encoding::Encode(EMWDC,EDOWN,EX,74);
	brokenwire_gid[4]=Encoding::Encode(EMWDC,EDOWN,EX,75);
	brokenwire_gid[5]=Encoding::Encode(EMWDC,EDOWN,EX,76);
	// DownY
	brokenwire_gid[6]=Encoding::Encode(EMWDC,EDOWN,EY,31);
	brokenwire_gid[7]=Encoding::Encode(EMWDC,EDOWN,EY,32);
	brokenwire_gid[8]=Encoding::Encode(EMWDC,EDOWN,EY,47);
	// DownU
	brokenwire_gid[9]=Encoding::Encode(EMWDC,EDOWN,EU,94);
	brokenwire_gid[10]=Encoding::Encode(EMWDC,EDOWN,EU,95);
	brokenwire_gid[11]=Encoding::Encode(EMWDC,EDOWN,EU,96);

	Bool_t minrising_brokenwireflag;
	// 
	TF1* ffit_rising,*ffit_falling;

	TDirectory* dir_hist=file_drifttime->GetDirectory("minrising_drifttime/histogram");
	dir_hist->cd();
	for(int l=0;l<g_mwdc_location;l++){
	  for(int p=0;p<g_mwdc_wireplane;p++){
	      for(int w=0;w<g_mwdc_wireindex[p];w++){
	      	UInt_t gid=Encoding::Encode(EMWDC,l,p,w);
	      	minrising_brokenwireflag=false;
	      	for(int brokenwire_index=0;brokenwire_index<11;brokenwire_index++){
	      		if(gid == brokenwire_gid[brokenwire_index]){
	      			minrising_brokenwireflag=true;
	      			break;
	      		}
	      	}
	      	if(!minrising_brokenwireflag){
	      		ffit_rising=fit_rising(h1d_minrising_drifttime_mwdc_single[gid],"QB+",-30,40,1);
	      		for(int paramindex=0;paramindex<rising_nparams;paramindex++){
	      			h1d_rising_param[paramindex]->Fill(ffit_rising->GetParameter(paramindex));
	      			
	      		}
	      		h1d_drifttime_t0_minus_2T0->Fill(ffit_rising->GetParameter("t0")-2*ffit_rising->GetParameter("T0"));
	      		// 
	      		ffit_falling=fit_falling(h1d_minrising_drifttime_mwdc_single[gid],"QB+",80,150,0);
	      		for(int paramindex=0;paramindex<falling_nparams;paramindex++){
	      			h1d_falling_param[paramindex]->Fill(ffit_falling->GetParameter(paramindex));
	      		}
	      		h1d_drifttime_tm_plus_2Tm->Fill(ffit_falling->GetParameter("tm")+2*ffit_falling->GetParameter("Tm"));
	      		// 
	      		h1d_minrising_drifttime_mwdc_single[gid]->Write(0,TObject::kOverwrite);
	      		h2d_location_vs_tm->Fill(Encoding::DecodeLocation(gid),ffit_falling->GetParameter("tm"));
	      		h2d_location_vs_t0->Fill(Encoding::DecodeLocation(gid),ffit_rising->GetParameter("t0"));
	      		//
	      		h1d_drifttime_interval->Fill(ffit_falling->GetParameter("tm") - ffit_rising->GetParameter("t0"));
	      		h1d_drifttime_interval_new->Fill(ffit_falling->GetParameter("tm")+2*ffit_falling->GetParameter("Tm")-ffit_rising->GetParameter("t0")+2*ffit_rising->GetParameter("T0"));
	      		h2d_t0_vs_tm->Fill(ffit_rising->GetParameter("t0"),ffit_falling->GetParameter("tm"));
	      		h2d_t0_vs_tm_new->Fill(ffit_rising->GetParameter("t0")-2*ffit_rising->GetParameter("T0"),ffit_falling->GetParameter("tm")+2*ffit_falling->GetParameter("Tm"));
	      	}
	      }
	  }
	}

	//
	Double_t t0,tm,Tm,T0;
	Int_t bin_t0,bin_tm; 
	TCanvas* c[g_mwdc_location][g_mwdc_wireplane];
	TCanvas* c1;

	for(int l=0;l<g_mwdc_location;l++){
	  for(int p=0;p<g_mwdc_wireplane;p++){
	  	ffit_rising=fit_rising(h1d_minrising_drifttime_mwdc[l][p],"QB+",-30,40,1);
	  	ffit_falling=fit_falling(h1d_minrising_drifttime_mwdc[l][p],"QB+",80,150,0);
	  	c1=(TCanvas*)gROOT->FindObject("c1");
		delete c1;
	  	// 
	  	// c[l][p]=new TCanvas(Form("c%s_%s_minrising_drifttime",g_str_location[l],g_str_plane[p]),Form("c%s_%s_minrising_drifttime",g_str_location[l],g_str_plane[p]));
	  	// c[l][p]->cd();
	  	// h1d_minrising_drifttime_mwdc[l][p]->Draw();
	  	h1d_minrising_drifttime_mwdc[l][p]->Print();

	  	t0=ffit_rising->GetParameter("t0");T0=ffit_rising->GetParameter("T0");tm=ffit_falling->GetParameter("tm");Tm=ffit_falling->GetParameter("Tm");
	  	bin_t0=h1d_minrising_drifttime_mwdc[l][p]->FindBin(t0);bin_tm=h1d_minrising_drifttime_mwdc[l][p]->FindBin(tm);
	  	printf("MWDC_%s_%s: t0=%.2f, tm=%.2f, Tm=%.2f, tm-t0=%.2f\n",g_str_location[l],g_str_plane[p],t0,tm,Tm,tm-t0);
	  	printf("\tevent_proportion: <t0=%.4f, tm-t0=%.4f, >tm=%.4f\n", h1d_minrising_drifttime_mwdc[l][p]->Integral(0,bin_t0-1)/h1d_minrising_drifttime_mwdc[l][p]->Integral(),h1d_minrising_drifttime_mwdc[l][p]->Integral(bin_t0,bin_tm)/h1d_minrising_drifttime_mwdc[l][p]->Integral(),h1d_minrising_drifttime_mwdc[l][p]->Integral(bin_tm+1,600)/h1d_minrising_drifttime_mwdc[l][p]->Integral());

	  	bin_t0=h1d_minrising_drifttime_mwdc[l][p]->FindBin(t0-2*T0);bin_tm=h1d_minrising_drifttime_mwdc[l][p]->FindBin(tm+2*Tm);
	  	printf("\tevent_proportion: <t0-2T0=%.4f, tm-t0=%.4f, >tm+2Tm=%.4f\n", h1d_minrising_drifttime_mwdc[l][p]->Integral(0,bin_t0-1)/h1d_minrising_drifttime_mwdc[l][p]->Integral(),h1d_minrising_drifttime_mwdc[l][p]->Integral(bin_t0,bin_tm)/h1d_minrising_drifttime_mwdc[l][p]->Integral(),h1d_minrising_drifttime_mwdc[l][p]->Integral(bin_tm+1,600)/h1d_minrising_drifttime_mwdc[l][p]->Integral());
	  	// TLine* line_t0=new TLine(t0,0,t0,ffit_rising->GetParameter(1));
	  	// line_t0->SetLineWidth(2);
	  	// line_t0->SetLineColor(kBlack);
	  	// line_t0->DrawClone("same");
	  	// TLine* line_tm=new TLine(tm,0,tm,ffit_falling->GetParameter(2));
	  	// line_tm->SetLineWidth(2);
	  	// line_tm->SetLineColor(kGreen);
	  	// line_tm->DrawClone("same");
	  }
	}

	// save param histogram
	TDirectory* dir_drift=file_drifttime->GetDirectory("minrising_drifttime");
	dir_drift->cd();
	for(int paramindex=0;paramindex<rising_nparams;paramindex++){
		h1d_rising_param[paramindex]->Write(0,TObject::kOverwrite);
	}
	for(int paramindex=0;paramindex<falling_nparams;paramindex++){
		h1d_falling_param[paramindex]->Write(0,TObject::kOverwrite);
	}
	h1d_drifttime_interval->Write(0,TObject::kOverwrite);
	h1d_drifttime_interval_new->Write(0,TObject::kOverwrite);
	h1d_drifttime_t0_minus_2T0->Write(0,TObject::kOverwrite);
	h1d_drifttime_tm_plus_2Tm->Write(0,TObject::kOverwrite);
	h2d_t0_vs_tm->Write(0,TObject::kOverwrite);
	h2d_t0_vs_tm_new->Write(0,TObject::kOverwrite);
	h2d_location_vs_tm->Write(0,TObject::kOverwrite);
	h2d_location_vs_t0->Write(0,TObject::kOverwrite);
	// 
	delete file_drifttime;
}