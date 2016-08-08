#include "TrackFit.h"
#include "TTree.h"
#include "TFile.h"
#include "TROOT.h"
#include "TCanvas.h"
#include "TH1F.h"
#include "utility.h"
#include "TMath.h"
#include "TStyle.h"
#include "TH2F.h"
#include "TH1F.h"
#include "DriftInfo.h"
#include "TSpline.h"
#include "TGraph.h"
#include "TStopwatch.h"
#include <Fit/Fitter.h>
#include "global.h"

int calib(const char* datadir,const char* outfile)
{
	TH2F* h2d_residuals[g_mwdc_location][g_mwdc_wireplane];
	for(int l=0;l<g_mwdc_location;l++){
		for(int p=0;p<g_mwdc_wireplane;p++){
			h2d_residuals[l][p]=new TH2F(Form("h2d_%s_%s_residual",g_str_location[l],g_str_plane[p]),Form("h2d_%s_%s_residual",g_str_location[l],g_str_plane[p]),400,0,400,4000,0,8);
		}
	}

	TH1F* h1d_fitteddistances[g_mwdc_location][g_mwdc_wireplane];
	for(int l=0;l<g_mwdc_location;l++){
		for(int p=0;p<g_mwdc_wireplane;p++){
			h1d_fitteddistances[l][p]=new TH1F(Form("h1d_%s_%s_Init_Fitted_Distance",g_str_location[l],g_str_plane[p]),Form("h1d_%s_%s_Init_Fitted_Distance",g_str_location[l],g_str_plane[p]),200,0,50);
		}
	}
	TH1F* h1d_init_distance_square=new TH1F("h1d_init_distance_square","h1d_init_distance_square",200,0,2000);
	TH1F* h1d_final_distance_square=new TH1F("h1d_final_distance_square","h1d_final_distance_square",100,0,100);
	// 
	TString file_data=TString(datadir)+"/"+outfile;  
	TFile* file_drifttime=new TFile(file_data,"update");
	if(!file_drifttime){
	  printf("open file error: %s\n",outfile);
	  exit(1);
	}

	// drifttime
	TTree* tree_minrising_drifttime;
	file_drifttime->GetObject("minrising_drifttime/minrising_drifttime",tree_minrising_drifttime);

	UInt_t minrising_gid[g_mwdc_location][g_mwdc_wireplane];
	Double_t minrising_tot[g_mwdc_location][g_mwdc_wireplane];//unit: ns
	Double_t minrising_drifttime[g_mwdc_location][g_mwdc_wireplane];
	
	Double_t tot_limit=2775;// if a channel has only rising edge and no falling edge, we think its tot is beyond the limit.
	Double_t drifttime_limt=3000;
	                           // thus assign this value as the tot value of this channel. It's kind of arbitrarily.
	tree_minrising_drifttime->SetBranchAddress("gid",minrising_gid);
	tree_minrising_drifttime->SetBranchAddress("tot",minrising_tot);
	tree_minrising_drifttime->SetBranchAddress("drifttime",minrising_drifttime);
	// Drift info
	DriftInfo* driftinfo;
	file_drifttime->GetObject("minrising_drifttime/init_edge_fitting_result",driftinfo);
	assert(driftinfo);

	// rt-relation
	const char hist[6][40]={"h1d_minrising_drifttime_mwdc_Down_X_40"
							,"h1d_minrising_drifttime_mwdc_Down_Y_40"
							,"h1d_minrising_drifttime_mwdc_Down_U_50"
							,"h1d_minrising_drifttime_mwdc_Up_X_40"
							,"h1d_minrising_drifttime_mwdc_Up_Y_40"
							,"h1d_minrising_drifttime_mwdc_Up_U_50"};
	TSpline5* rt_relation[g_mwdc_location][g_mwdc_wireplane];
	for(int l=0;l<g_mwdc_location;l++){
	  for(int p=0;p<g_mwdc_wireplane;p++){
	  	file_drifttime->GetObject(Form("minrising_drifttime/trackfit_test/seed/spline_%s",hist[l*3+p]),rt_relation[l][p]);
	 	assert(rt_relation[l][p]);
	  }
	}
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

	UShort_t boundary_wire_low[g_mwdc_wireplane]={3,3,6};
	UShort_t boundary_wire_high[g_mwdc_wireplane]={77,77,100};

	Bool_t valid_event;
	Double_t t0,T0,tm,Tm;
	UShort_t wireindex;
	TVector3 hitwireUpX,hitwireUpY,hitwireDownX,hitwireDownY,hitposUp,hitposDown;

	GeometryInfo gm_info;
	Double_t p0_start[4];
	const Double_t* p0_stop;
	double p0_step=0.01;

	TrackFit::LineFit linefit(gm_info);
	TrackFit::Line init_track;
	ROOT::Fit::Fitter  fitter;
	ROOT::Fit::FitResult result;
	std::map<UInt_t, Double_t> residuals;
	std::map<UInt_t,Double_t>::const_iterator it;
	// event loop
	Int_t entries=tree_minrising_drifttime->GetEntries();
	Int_t failed_evenum=0;
	Int_t valid_evenum=0;

	 TStopwatch timer;
	 timer.Start();
	// for(int i=0;i<10000;i++){
	for(int i=0;i<entries;i++){
		if(!((i+1)%5000)){
		  printf("%d events analyzed\n",i+1);
		}

		tree_minrising_drifttime->GetEntry(i);
		valid_event=true;
		//
		for(int l=0;l<g_mwdc_location;l++){
		  for(int p=0;p<g_mwdc_wireplane;p++){
		  	if(!Encoding::IsChannelValid(minrising_gid[l][p])){
		  		valid_event=false;
		  		break;
		  	}
		  	// 
		  	wireindex=Encoding::DecodeIndex(minrising_gid[l][p]);
		  	if(wireindex< boundary_wire_low[p]|| wireindex> boundary_wire_high[p]){
		  		valid_event=false;
		  		break;
		  	}
		  	// 
		  	for(int brokenwire_index=0;brokenwire_index<11;brokenwire_index++){
		  		if(minrising_gid[l][p] == brokenwire_gid[brokenwire_index]){
		  			valid_event=false;
		  			break;
		  		}
		  	}
		  }
		}

		if(valid_event){
			valid_evenum++;

			linefit.Reset();
			for(int l=0;l<g_mwdc_location;l++){
			  for(int p=0;p<g_mwdc_wireplane;p++){
			  	t0=driftinfo->Get_t0(minrising_gid[l][p]);
			  	T0=driftinfo->Get_T0(minrising_gid[l][p]);
			  	// tm=driftinfo->Get_tm(minrising_gid[l][p]);
			  	// Tm=driftinfo->Get_Tm(minrising_gid[l][p]);
			  	minrising_drifttime[l][p]-=(t0-2*T0);
			  	// 
			  	linefit.AddHit(minrising_gid[l][p],rt_relation[l][p]->Eval(minrising_drifttime[l][p]));
			  }
			}

			// start parameters 
			hitwireUpX=gm_info.GetPoint(minrising_gid[1][0]);
			// hitwireUpX.Print();
			hitwireUpY=gm_info.GetPoint(minrising_gid[1][1]);
			// hitwireUpY.Print();
			hitwireDownX=gm_info.GetPoint(minrising_gid[0][0]);
			// hitwireDownX.Print();
			hitwireDownY=gm_info.GetPoint(minrising_gid[0][1]);
			// hitwireDownY.Print();

			hitposUp.SetXYZ(hitwireUpY.X(),hitwireUpX.Y(),(hitwireUpY.Z()+hitwireUpX.Z())/2);
			// hitposUp.Print();
			hitposDown.SetXYZ(hitwireDownX.X(),hitwireDownY.Y(),(hitwireDownX.Z()+hitwireDownY.Z())/2);
			// hitposDown.Print();
			
			init_track.Reset(hitposUp,hitposDown,false);
			init_track.GetParameter(p0_start);
			
			linefit.CalcResiduals(p0_start);
			residuals=linefit.GetResiduals();
			for(it = residuals.begin();it!=residuals.end();++it){
				UChar_t l,p;
				l=Encoding::DecodeLocation(it->first);
				p=Encoding::DecodeDirection(it->first);
				h1d_fitteddistances[l][p]->Fill(it->second);
			}
			
			// fit config
			fitter.SetFCN(linefit,p0_start);
			for (int j = 0; j < 4; ++j) 
				fitter.Config().ParSettings(j).SetStepSize(p0_step);
			
			// fitter.EvalFCN();
			// printf("Init distance square: %.4f %.4f\n",fitter.Result().MinFcnValue(), linefit(p0_start));
			h1d_init_distance_square->Fill(linefit(p0_start));

			// fitting
			if (!fitter.FitFCN()) {
			   Error("line3Dfit","Line3D Fit failed");

			   printf("Event No: %d\n",i+1 );
			   hitwireUpX.Print();
			   hitwireUpY.Print();
			   hitwireDownX.Print();
			   hitwireDownY.Print();
			   hitposUp.Print();
			   hitposDown.Print();
			   for(int j=0;j<4;j++){
			   	printf("%.2f\t", p0_start[j]);
			   }

			   failed_evenum++;
			   // return 1;
			}

			// fit result
			result = fitter.Result();
			// std::cout << "Total final distance square " << result.MinFcnValue() << std::endl;
			h1d_final_distance_square->Fill(result.MinFcnValue());
			// result.Print(std::cout);
			p0_stop=result.GetParams();

			// calculate residuals
			linefit.CalcResiduals(p0_stop);
			residuals=linefit.GetResiduals();
			
			for(it = residuals.begin();it!=residuals.end();++it){
				UChar_t l,p;
				l=Encoding::DecodeLocation(it->first);
				p=Encoding::DecodeDirection(it->first);
				h2d_residuals[l][p]->Fill(minrising_drifttime[l][p], it->second);
			}
		}
		
	}
	timer.Stop();
	printf("%d events processed totally; %d events valid, %d events failed\n",entries,valid_evenum,failed_evenum);
	Double_t cputime = timer.CpuTime();
	printf("RT=%7.3f s, Cpu=%7.3f s\n",timer.RealTime(),cputime);

	for(int l=0;l<g_mwdc_location;l++){
		for(int p=0;p<g_mwdc_wireplane;p++){
			TCanvas* c=new TCanvas(Form("c%s_%s",g_str_location[l],g_str_plane[p]),Form("c%s_%s",g_str_location[l],g_str_plane[p]));
			// c->SetLogy();
			h2d_residuals[l][p]->Draw();
			// h1d_fitteddistances[l][p]->Draw();
		}
	}
	for(int l=0;l<g_mwdc_location;l++){
		for(int p=0;p<g_mwdc_wireplane;p++){
			TCanvas* c=new TCanvas(Form("cfitteddistances%s_%s",g_str_location[l],g_str_plane[p]),Form("c%s_%s",g_str_location[l],g_str_plane[p]));
			// c->SetLogy();
			h1d_fitteddistances[l][p]->Draw();
		}
	}
	TCanvas* cinit_distance_square=new TCanvas("cinit_distance_square","cinit_distance_square");
	// cinit_distance_square->SetLogy();
	h1d_init_distance_square->Draw();
	TCanvas* cfinal_distance_square=new TCanvas("cfinal_distance_square","cfinal_distance_square");
	// cfinal_distance_square->SetLogy();
	h1d_final_distance_square->Draw();
	// 
	delete file_drifttime;
}