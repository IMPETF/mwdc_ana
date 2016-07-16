/*
   Toy simualation of the cosmic ray events, and the bunch id recording process.
   Assumptions:
    1) The average cosmic ray event rate is 25/s, thus the average interval is 4.0e7 ns
    2) Use the 25ns as the time unit which is consistent with bunch id calculation. 
       Thus the average interval is 16.0e5
    3) Use 9 boards and the 5th board as the standard ref phase.
       The phase offset of each board is from the real data, and the values are as follows(in the unit of 25ns):
	   		phase_offset[9]={-0.03406,-0.03305,-0.00736,-0.00249,0,0.00338,0.00763,0.01951,0.02818};
 	4) The clock init from 1
 	5) The bunch id roll-over range is 4096(in the unit of 25ns)
 */
#include "TRandom3.h"
#include "TRandom1.h"
#include "TFile.h"
#include "TTree.h"
#include "stdio.h"

void simulate_bunchunsync(const char* filename,int eventnum=1000000,double jitter_sigma=0.06)
{
	TRandom3* rdm=new TRandom3();
	rdm->SetSeed(0);
	TRandom3* rdm_jitter=new TRandom3();
	//
	Double_t tau=16.0e5;
	Double_t phase_offset[9]={-0.03406,-0.03305,-0.00736,-0.00249,0,0.00338,0.00763,0.01951,0.02818};
	TString  boardname[9]={"tof3","tof4","mwdc_a","mwdc_c","tof2","tof1","mwdc_b","mwdc_d","mwdc_e"};
	Double_t init_clock_cycle=1;
	Int_t rollover=4096;

	Double_t interval;
	Double_t realtime=0;
	Double_t jitter[9];
	Int_t bunchid[9];

	TFile* fout=new  TFile(filename,"recreate");
	TTree* tout[9];
	for(int i=0;i<9;i++){
		tout[i]=new TTree(boardname[i].Data(),boardname[i].Data());
		tout[i]->Branch("bunch_id",bunchid+i);
	}

	for(int i=0;i<eventnum;i++){
		if(i%100 ==0){
			printf("%d events simulated\n", i);
		}
		interval=rdm->Exp(tau);
		realtime+=interval;
		

		// printf("%.4f\n",realtime );

		for(int j=0;j<9;j++){
			jitter[j]=rdm_jitter->Gaus(0,jitter_sigma);
			if(j==6){
				jitter[j]=0;
			}
			//
			bunchid[j]=((Long_t)(phase_offset[j]+init_clock_cycle+realtime+jitter[j]))%rollover;
			// printf("%d\t",bunchid[j]);
			// 
			tout[j]->Fill();
		}
		// printf("\n");
	}

	// for(int j=0;j<9;j++){
	// 	printf("%d\t",((Int_t)(phase_offset[j]+init_clock_cycle+4095.993))%rollover);
	// }
	// printf("\n");
	// 
	fout->cd();
	for(int j=0;j<9;j++){
		tout[j]->Write();
	}

	delete fout;
	delete rdm,rdm_jitter;

	return;
}