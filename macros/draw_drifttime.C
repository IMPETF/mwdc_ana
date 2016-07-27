#include "TTree.h"
#include "TFile.h"
#include "TROOT.h"
#include "TCanvas.h"
#include "TH1F.h"
#include "CrateInfo.h"
#include "BoardInfo.h"
#include "utility.h"
#include "TMath.h"
#include "TStyle.h"
#include "TH2F.h"

void draw_drifttime(const char* datadir,const char* outfile)
{
	gStyle->SetOptStat(111111);
	// 
	TH1F* h1d_minrising_drifttime_mwdc[g_mwdc_location][g_mwdc_wireplane];
	TH1F* h1d_maxtot_drifttime_mwdc[g_mwdc_location][g_mwdc_wireplane];
	for(int l=0;l<g_mwdc_location;l++){
	  for(int p=0;p<g_mwdc_wireplane;p++){
	  	h1d_minrising_drifttime_mwdc[l][p]=new TH1F(Form("h1d_%s_%s_minrising_drifttime",g_str_location[l],g_str_plane[p]),Form("h1d_%s_%s_minrising_drifttime",g_str_location[l],g_str_plane[p]),600,-100.5,499.5);
	  	h1d_maxtot_drifttime_mwdc[l][p]=new TH1F(Form("h1d_%s_%s_maxtot_drifttime",g_str_location[l],g_str_plane[p]),Form("h1d_%s_%s_maxtot_drifttime",g_str_location[l],g_str_plane[p]),600,-100.5,499.5);
	  }
	}
	// 
	TString file_data=TString(datadir)+"/"+outfile;  
	TFile* file_drifttime=new TFile(file_data,"update");
	if(!file_drifttime){
	  printf("open file error: %s\n",outfile);
	  exit(1);
	}

	TTree* tree_maxtot_drifttime,*tree_minrising_drifttime;
	file_drifttime->GetObject("maxtot_drifttime/maxtot_drifttime",tree_maxtot_drifttime);
	file_drifttime->GetObject("minrising_drifttime/minrising_drifttime",tree_minrising_drifttime);

	UInt_t maxtot_gid[g_mwdc_location][g_mwdc_wireplane],minrising_gid[g_mwdc_location][g_mwdc_wireplane];
	Double_t maxtot_tot[g_mwdc_location][g_mwdc_wireplane],minrising_tot[g_mwdc_location][g_mwdc_wireplane];//unit: ns
	Double_t maxtot_drifttime[g_mwdc_location][g_mwdc_wireplane],minrising_drifttime[g_mwdc_location][g_mwdc_wireplane];
	
	Double_t tot_limit=2775;// if a channel has only rising edge and no falling edge, we think its tot is beyond the limit.
	Double_t drifttime_limt=3000;
	                           // thus assign this value as the tot value of this channel. It's kind of arbitrarily.
	tree_maxtot_drifttime->SetBranchAddress("gid",maxtot_gid);
	tree_maxtot_drifttime->SetBranchAddress("tot",maxtot_tot);
	tree_maxtot_drifttime->SetBranchAddress("drifttime",maxtot_drifttime);

	tree_minrising_drifttime->SetBranchAddress("gid",minrising_gid);
	tree_minrising_drifttime->SetBranchAddress("tot",minrising_tot);
	tree_minrising_drifttime->SetBranchAddress("drifttime",minrising_drifttime);
	
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

	Bool_t maxtot_brokenwireflag,minrising_brokenwireflag;
	// event loop
	Int_t entries=tree_minrising_drifttime->GetEntries();
	for(int i=0;i<entries;i++){
		if(!((i+1)%5000)){
		  printf("%d events analyzed\n",i+1);
		}

		tree_minrising_drifttime->GetEntry(i);
		tree_maxtot_drifttime->GetEntry(i);
		// 
		for(int l=0;l<g_mwdc_location;l++){
		  for(int p=0;p<g_mwdc_wireplane;p++){
		  	if(l==1){
		    	if(Encoding::IsChannelValid(maxtot_gid[l][p])){
		   			h1d_maxtot_drifttime_mwdc[l][p]->Fill(maxtot_drifttime[l][p]);
		    	}
		    	if(Encoding::IsChannelValid(minrising_gid[l][p])){
		    		h1d_minrising_drifttime_mwdc[l][p]->Fill(minrising_drifttime[l][p]);
		    	}
			}
			else{
				if(Encoding::IsChannelValid(maxtot_gid[l][p])){
		   			maxtot_brokenwireflag=false;
		   			for(int brokenwire_index=0;brokenwire_index<11;brokenwire_index++){
		   				if(maxtot_gid[l][p] == brokenwire_gid[brokenwire_index]){
		   					maxtot_brokenwireflag=true;
		   					break;
		   				}
		   			}
		   			if(!maxtot_brokenwireflag){	
		   				h1d_maxtot_drifttime_mwdc[l][p]->Fill(maxtot_drifttime[l][p]);
		   			}
		    	}
		    	if(Encoding::IsChannelValid(minrising_gid[l][p])){
		    		minrising_brokenwireflag=false;
		    		for(int brokenwire_index=0;brokenwire_index<11;brokenwire_index++){
		    			if(minrising_gid[l][p] == brokenwire_gid[brokenwire_index]){
		    				minrising_brokenwireflag=true;
		    				break;
		    			}
		    		}
		    		if(!minrising_brokenwireflag){
						h1d_minrising_drifttime_mwdc[l][p]->Fill(minrising_drifttime[l][p]);		    			
		    		}
		    	}
			}
		  }
		}
	}
	printf("%d events processed totally\n",entries);

	// save histograms
	TDirectory* dir_maxtot=file_drifttime->GetDirectory("maxtot_drifttime");
	if(!dir_maxtot){
	  dir_maxtot=file_drifttime->mkdir("maxtot_drifttime");
	  if(!dir_maxtot){
	    printf("error!can't mkdir \"maxtot_drifttime\" in %s\n",file_data.Data());
	    exit(1);
	  }
	  dir_maxtot=file_drifttime->GetDirectory("maxtot_drifttime");
	}
	dir_maxtot->cd();
	for(int l=0;l<g_mwdc_location;l++){
	  for(int p=0;p<g_mwdc_wireplane;p++){
	  	h1d_maxtot_drifttime_mwdc[l][p]->Write(0,TObject::kOverwrite);
	  }
	}

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
	for(int l=0;l<g_mwdc_location;l++){
	  for(int p=0;p<g_mwdc_wireplane;p++){
	  	h1d_minrising_drifttime_mwdc[l][p]->Write(0,TObject::kOverwrite);

	  	TCanvas* c1=new TCanvas(Form("c%s_%s_minrising_drifttime",g_str_location[l],g_str_plane[p]),Form("c%s_%s_minrising_drifttime",g_str_location[l],g_str_plane[p]));
	  	c1->SetLogy();
	  	h1d_minrising_drifttime_mwdc[l][p]->Draw();
	  }
	}

	// 
	delete file_drifttime;
}