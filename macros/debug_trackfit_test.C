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
#include "TProfile.h"
#include "DriftInfo.h"
#include "TSpline.h"
#include "TGraph.h"
#include "TStopwatch.h"
#include <Fit/Fitter.h>
#include "Math/ChebyshevPol.h"
#include "Math/Polynomial.h"
#include "global.h"
#include "stdio.h"
#include "TF1.h"

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

int calib_test(const char* datadir,const char* outfile,const char* dir_rtseed,const char* dir_rtcorrected,bool seed_round=false)
{
	// open the file 
	TString file_data=TString(datadir)+"/"+outfile;  
	TFile* file_drifttime=new TFile(file_data,"update");
	if(!file_drifttime){
	  printf("open file error: %s\n",outfile);
	  exit(1);
	}

	// get drifttime
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
	// get Drift info, i.e. t0 and T0
	DriftInfo* driftinfo;
	file_drifttime->GetObject("minrising_drifttime/init_edge_fitting_result",driftinfo);
	assert(driftinfo);

	// get rt-relation
	const char hist[6][40]={"h1d_minrising_drifttime_mwdc_Down_X_40"
							,"h1d_minrising_drifttime_mwdc_Down_Y_40"
							,"h1d_minrising_drifttime_mwdc_Down_U_50"
							,"h1d_minrising_drifttime_mwdc_Up_X_40"
							,"h1d_minrising_drifttime_mwdc_Up_Y_40"
							,"h1d_minrising_drifttime_mwdc_Up_U_50"};

	double xmin = -10; double xmax = 450; 
	int order=7;
	ROOT::Math::ChebyshevPol * cheb = new ROOT::Math::ChebyshevPol(order);
	TF1* rt_relation[g_mwdc_location][g_mwdc_wireplane];
	if(seed_round){
		// TSpline5* rt_relation_spline[g_mwdc_location][g_mwdc_wireplane];
		TGraph* g_rt_relation[g_mwdc_location][g_mwdc_wireplane];
		
		for(int l=0;l<g_mwdc_location;l++){
		  for(int p=0;p<g_mwdc_wireplane;p++){
		  	file_drifttime->GetObject(Form("minrising_drifttime/trackfit_test/%s/grt_%s",dir_rtseed,hist[l*3+p]),g_rt_relation[l][p]);
		 	assert(g_rt_relation[l][p]);
		 	
		 	// rt_relation[l][p] = new TF1(Form("frt_%s",hist[l*3+p]),cheb,xmin,xmax,order+1);
		 	// for (int i = 0; i <=order; ++i) 
		 	// 	rt_relation[l][p]->SetParameter(i,1);  
		 	rt_relation[l][p] = new TF1(Form("frt_%s",hist[l*3+p]),Pol,xmin,xmax,8);
		 	for (int i = 0; i <8; ++i) 
		 		rt_relation[l][p]->SetParameter(i,1);
		 	// rt_relation[l][p]->SetParameter(0,0);
		 	rt_relation[l][p]->SetParameter(6,140);
		 	rt_relation[l][p]->SetParLimits(6,120,160);
		 	// rt_relation[l][p]->SetParameter(7,(7.433-5.5)/260.);  
		 	g_rt_relation[l][p]->Fit(rt_relation[l][p],"","",0,400);

		 	rt_relation[l][p]->SetRange(xmin,xmax);
		  }
		}
	}
	else{
		for(int l=0;l<g_mwdc_location;l++){
		  for(int p=0;p<g_mwdc_wireplane;p++){
			file_drifttime->GetObject(Form("minrising_drifttime/trackfit_test/%s/frt_%s",dir_rtseed,hist[l*3+p]),rt_relation[l][p]);		 	
		  	assert(rt_relation[l][p]);
		  }
		}
	}

	// new histograms
	TH2F* h2d_residuals[g_mwdc_location][g_mwdc_wireplane];
	TH2F* h2d_rtrelation[g_mwdc_location][g_mwdc_wireplane];
	for(int l=0;l<g_mwdc_location;l++){
		for(int p=0;p<g_mwdc_wireplane;p++){
			h2d_residuals[l][p]=new TH2F(Form("h2d_%s_%s_residual",g_str_location[l],g_str_plane[p]),Form("h2d_%s_%s_residual",g_str_location[l],g_str_plane[p]),400,0,400,8000,-8,8);
			h2d_rtrelation[l][p]=new TH2F(Form("h2d_%s_%s_rtrelatioin",g_str_location[l],g_str_plane[p]),Form("h2d_%s_%s_rtrelatioin",g_str_location[l],g_str_plane[p]),400,0,400,1600,0,8);
		}
	}

	TH1F* h1d_init_fitteddistances[g_mwdc_location][g_mwdc_wireplane];
	TH1F* h1d_final_fitteddistances[g_mwdc_location][g_mwdc_wireplane];
	for(int l=0;l<g_mwdc_location;l++){
		for(int p=0;p<g_mwdc_wireplane;p++){
			h1d_init_fitteddistances[l][p]=new TH1F(Form("h1d_%s_%s_Init_Fitted_Distance",g_str_location[l],g_str_plane[p]),Form("h1d_%s_%s_Init_Fitted_Distance",g_str_location[l],g_str_plane[p]),400,-50,50);
			h1d_final_fitteddistances[l][p]=new TH1F(Form("h1d_%s_%s_Final_Fitted_Distance",g_str_location[l],g_str_plane[p]),Form("h1d_%s_%s_Final_Fitted_Distance",g_str_location[l],g_str_plane[p]),400,-50,50);
		}
	}

	TH1F* h1d_init_distance_square=new TH1F("h1d_init_distance_square","h1d_init_distance_square",200,0,2000);
	TH1F* h1d_final_distance_square=new TH1F("h1d_final_distance_square","h1d_final_distance_square",100,0,100);

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

	// don't use boundary wires
	UShort_t boundary_wire_low[g_mwdc_wireplane]={3,3,6};
	UShort_t boundary_wire_high[g_mwdc_wireplane]={77,77,100};

	// declare fitting variables
	Bool_t valid_event;
	Double_t t0,T0,tm,Tm;
	UShort_t wireindex;
	TVector3 hitwireUpX,hitwireUpY,hitwireDownX,hitwireDownY,hitposUp,hitposDown;
	UChar_t l,p;

	GeometryInfo gm_info;
	Double_t p0_start[4];
	const Double_t* p0_stop;
	double p0_step=0.01;

	TrackFit::LineFit linefit(gm_info);
	TrackFit::Line init_track;
	ROOT::Fit::Fitter  fitter;
	ROOT::Fit::FitResult result;
	std::map<UInt_t, Double_t> residuals;
	std::map<UInt_t, Double_t> fitted_distances;
	std::map<UInt_t,Double_t>::const_iterator it;

	// event loop
	Int_t entries=tree_minrising_drifttime->GetEntries();
	Int_t failed_evenum=0;
	Int_t valid_evenum=0;

	TStopwatch timer;
	timer.Start();
	// for(int i=0;i<100000;i++){
	for(int i=0;i<entries;i++){
		if(!((i+1)%5000)){
		  printf("%d events analyzed\n",i+1);
		}
		tree_minrising_drifttime->GetEntry(i);
		
		// check whether this event can be used for fitting
		valid_event=true;
		for(l=0;l<g_mwdc_location;l++){
		  for(p=0;p<g_mwdc_wireplane;p++){
		  	if(valid_event){
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
		  	if(minrising_drifttime[l][p] == drifttime_limt || minrising_tot[l][p]== tot_limit){
		  		valid_event=false;
		  		break;	
		  	}
		  	}
		  	else{
		  		break;
		  	}
		  }
		}

		if(valid_event){
			valid_evenum++;

			linefit.Reset();
			for(l=0;l<g_mwdc_location;l++){
			  for(p=0;p<g_mwdc_wireplane;p++){
			  	t0=driftinfo->Get_t0(minrising_gid[l][p]);
			  	T0=driftinfo->Get_T0(minrising_gid[l][p]);
			  	// tm=driftinfo->Get_tm(minrising_gid[l][p]);
			  	// Tm=driftinfo->Get_Tm(minrising_gid[l][p]);
			  	minrising_drifttime[l][p]-=(t0-2*T0);
			  	// 
			  	linefit.AddHit(minrising_gid[l][p],rt_relation[l][p]->Eval(minrising_drifttime[l][p]));
			  }
			}

			// get initial track parameters 
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
			
			// get init residuals
			linefit.CalcResiduals(p0_start);
			residuals=linefit.GetResiduals();
			for(it = residuals.begin();it!=residuals.end();++it){
				l=Encoding::DecodeLocation(it->first);
				p=Encoding::DecodeDirection(it->first);
				h1d_init_fitteddistances[l][p]->Fill(it->second);
			}
			
			// fitting configuration
			fitter.SetFCN(linefit,p0_start);
			for (int j = 0; j < 4; ++j) 
				fitter.Config().ParSettings(j).SetStepSize(p0_step);
			
			// fitter.EvalFCN();
			// printf("Init distance square: %.4f %.4f\n",fitter.Result().MinFcnValue(), linefit(p0_start));
			// fill init distance square
			h1d_init_distance_square->Fill(linefit(p0_start));

			// fitting
			if (!fitter.FitFCN()) {
			   // Error("line3Dfit","Line3D Fit failed");

			   // printf("Event No: %d\n",i+1 );
			   // hitwireUpX.Print();
			   // hitwireUpY.Print();
			   // hitwireDownX.Print();
			   // hitwireDownY.Print();
			   // hitposUp.Print();
			   // hitposDown.Print();
			   // for(int j=0;j<4;j++){
			   // 	printf("%.2f\t", p0_start[j]);
			   // }

			   failed_evenum++;
			   continue;
			   // return 1;
			}

			// get fit result
			result = fitter.Result();
			// std::cout << "Total final distance square " << result.MinFcnValue() << std::endl;
			// fill final distance square
			h1d_final_distance_square->Fill(result.MinFcnValue());
			// result.Print(std::cout);
			
			// get final residuals
			p0_stop=result.GetParams();
			// calculate residuals
			linefit.CalcResiduals(p0_stop);
			residuals=linefit.GetResiduals();
			fitted_distances=linefit.GetFittedDistances();
			
			for(it = residuals.begin();it!=residuals.end();++it){
				if(it->second < 5 && it->second > -5){
					l=Encoding::DecodeLocation(it->first);
					p=Encoding::DecodeDirection(it->first);
					h2d_residuals[l][p]->Fill(minrising_drifttime[l][p], it->second);
					h2d_rtrelation[l][p]->Fill(minrising_drifttime[l][p], fitted_distances[it->first]);
					h1d_final_fitteddistances[l][p]->Fill(it->second);
				}
			}
		}
		
	}

	timer.Stop();
	printf("%d events processed totally; %d events valid, %d events failed\n",entries,valid_evenum,failed_evenum);
	Double_t cputime = timer.CpuTime();
	printf("RT=%7.3f s, Cpu=%7.3f s\n",timer.RealTime(),cputime);

	// correct rt relation
	TProfile* h2d_residuals_pfx[g_mwdc_location][g_mwdc_wireplane];
	TGraph* grt[g_mwdc_location][g_mwdc_wireplane];
	// TSpline5* spline[g_mwdc_location][g_mwdc_wireplane];
	TF1* rt_relation_corrected[g_mwdc_location][g_mwdc_wireplane];

	Int_t nbin;
	Double_t bincenter,bincontent;
	for(int l=0;l<g_mwdc_location;l++){
		for(int p=0;p<g_mwdc_wireplane;p++){
			h2d_residuals[l][p]->Rebin2D(8,5);
			h2d_residuals_pfx[l][p] =h2d_residuals[l][p]->ProfileX();
			
			grt[l][p]=new TGraph();
			grt[l][p]->SetName(Form("grt_%s",hist[l*3+p]));
			grt[l][p]->SetTitle(Form("grt_%s",hist[l*3+p]));
			grt[l][p]->SetPoint(0,0,0);
			// 
			nbin=h2d_residuals_pfx[l][p]->GetNbinsX();
			// nbin=h2d_residuals_pfx[l][p]->FindBin(150);
			for(int j=1;j<=nbin;j++){
				bincenter = h2d_residuals_pfx[l][p]->GetBinCenter(j);
				bincontent = h2d_residuals_pfx[l][p]->GetBinContent(j);
				grt[l][p]->SetPoint(j,bincenter,rt_relation[l][p]->Eval(bincenter)+bincontent);
			}
			// 
			// spline[l][p]=new TSpline5(Form("spline_%s",hist[l*3+p]),grt[l][p]);
			// spline[l][p]->SetName(Form("spline_%s",hist[l*3+p]));
			// spline[l][p]->SetLineColor(kRed);
			// rt_relation_corrected[l][p] = new TF1(Form("frt_%s",hist[l*3+p]),cheb,xmin,xmax,order+1);
		 // 	for (int i = 0; i <=order; ++i) 
		 // 		rt_relation_corrected[l][p]->SetParameter(i,1); 
		 	rt_relation_corrected[l][p] = new TF1(Form("frt_%s",hist[l*3+p]),Pol,xmin,xmax,8);
		 	for (int i = 0; i <8; ++i) 
		 		rt_relation_corrected[l][p]->SetParameter(i,1);
		 	// rt_relation_corrected[l][p]->SetParameter(0,0);
		 	rt_relation_corrected[l][p]->SetParameter(6,140);
		 	rt_relation_corrected[l][p]->SetParLimits(6,120,160);
		 	// rt_relation_corrected[l][p]->SetParameter(7,(7.433-5.5)/260.);  
		 		  
		 	grt[l][p]->Fit(rt_relation_corrected[l][p],"","",0,400);	
		 	rt_relation_corrected[l][p]->SetRange(xmin,xmax);
		}
	}

	// make output directory
	TDirectory* dir_output=file_drifttime->GetDirectory(Form("minrising_drifttime/trackfit_test/%s",dir_rtcorrected));
	if(!dir_output){
	  dir_output=file_drifttime->mkdir(Form("minrising_drifttime/trackfit_test/%s",dir_rtcorrected));
	  if(!dir_output){
	    printf("error!can't mkdir \"/minrising_drifttime/trackfit_test/%s\"in %s\n",outfile,dir_rtcorrected);
	    exit(1);
	  }
	  dir_output=file_drifttime->GetDirectory(Form("minrising_drifttime/trackfit_test/%s",dir_rtcorrected));
	}
	dir_output->cd();

	for(l=0;l<g_mwdc_location;l++){
		for(p=0;p<g_mwdc_wireplane;p++){
			h2d_residuals[l][p]->Write(0,TObject::kOverwrite);
			h2d_residuals_pfx[l][p]->Write(0,TObject::kOverwrite);
			h2d_rtrelation[l][p]->Write(0,TObject::kOverwrite);
			h1d_init_fitteddistances[l][p]->Write(0,TObject::kOverwrite);
			h1d_final_fitteddistances[l][p]->Write(0,TObject::kOverwrite);

			grt[l][p]->Write(0,TObject::kOverwrite);
			// spline[l][p]->Write(0,TObject::kOverwrite);
			rt_relation_corrected[l][p]->Write(0,TObject::kOverwrite);
		}
	}

	h1d_init_distance_square->Write(0,TObject::kOverwrite);
	h1d_final_distance_square->Write(0,TObject::kOverwrite);

	// 
	for(l=0;l<g_mwdc_location;l++){
		for(p=0;p<g_mwdc_wireplane;p++){
			delete h2d_residuals[l][p];
			delete h2d_rtrelation[l][p];
			delete h1d_init_fitteddistances[l][p];
			delete h1d_final_fitteddistances[l][p];
			delete grt[l][p];
			delete rt_relation_corrected[l][p];
			delete rt_relation[l][p];
			// delete spline[l][p];
		}
	}
	delete h1d_init_distance_square;
	delete h1d_final_distance_square;	

	delete file_drifttime;
}

void calib_iteration()
{
	// TString seed,corrected;
	char seed[100],corrected[100];

	// calib_test("./","drifttime_test.root","seed","round1",true);
	for(int i=4;i<13;i++){
		std::cout<< "Round: "<<i+1<<std::endl;

		// seed.Format("round%d",i+1);
		// corrected.Format("round%d",i+2);

		sprintf(seed,"round%d",i+1);
		sprintf(corrected,"round%d",i+2);

		calib_test("./","drifttime_test.root",seed,corrected);
	}
}