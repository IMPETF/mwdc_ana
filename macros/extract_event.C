/*   $Id: draw_trajectory.h, 2015-02-16 17:51:16+08:00 MWDC_ana $
 *--------------------------------------------------------
 *  Author(s):
 *
 *--------------------------------------------------------
*/

#include "TTree.h"
#include "TFile.h"
#include "TROOT.h"
#include "TCanvas.h"
#include "TH1F.h"
#include "CrateInfo.h"
#include "BoardInfo.h"
#include "utility.h"
#include "TGraph.h"
#include "TMath.h"
#include "TMultiGraph.h"
#include <algorithm>

int extract_event(const char* datadir,int testindex,const char* outdir,const char* psd_pedfile,float psdoffset_x=6.35,float psdoffset_y=-2.21,int ped_cut=5)
{
    //COMMENT: 
    TString outfile=TString::Format("%s/run_simple_%d.root",outdir,testindex);
    TString mwdcfile=TString::Format("%s/test%d/mwdc%d.root",datadir,testindex,testindex);
    TString psdfile=TString::Format("%s/test%d/psd%d.root",datadir,testindex,testindex);
    
    //COMMEMT: coordinate system info
    float x_up_position[80]={-414.75,-404.25,-393.75,-383.25,-372.75,-362.25,-351.75,-341.25,-330.75,-320.25,-309.75,-299.25,-288.75,-278.25,-267.75,-257.25,
                           -246.75,-236.25,-225.75,-215.25,-204.75,-194.25,-183.75,-173.25,-162.75,-152.25,-141.75,-131.25,-120.75,-110.25,-99.75,-89.25,
                           -78.75,-68.25,-57.75,-47.25,-36.75,-26.25,-15.75,-5.25,5.25,15.75,26.25,36.75,47.25,57.75,68.25,78.75,
                           89.25,99.75,110.25,120.75,131.25,141.75,152.25,162.75,173.25,183.75,194.25,204.75,215.25,225.75,236.25,246.75,
                           257.25,267.75,278.25,288.75,299.25,309.75,320.25,330.75,341.25,351.75,362.25,372.75,383.25,393.75,404.25,414.75};//Y position measuring
    float x_up_z[80];
    std::fill_n(x_up_z,80,773);
    float y_up_position[80]={-414.75,-404.25,-393.75,-383.25,-372.75,-362.25,-351.75,-341.25,-330.75,-320.25,-309.75,-299.25,-288.75,-278.25,-267.75,-257.25,
                             -246.75,-236.25,-225.75,-215.25,-204.75,-194.25,-183.75,-173.25,-162.75,-152.25,-141.75,-131.25,-120.75,-110.25,-99.75,-89.25,
                             -78.75,-68.25,-57.75,-47.25,-36.75,-26.25,-15.75,-5.25,5.25,15.75,26.25,36.75,47.25,57.75,68.25,78.75,
                             89.25,99.75,110.25,120.75,131.25,141.75,152.25,162.75,173.25,183.75,194.25,204.75,215.25,225.75,236.25,246.75,
                             257.25,267.75,278.25,288.75,299.25,309.75,320.25,330.75,341.25,351.75,362.25,372.75,383.25,393.75,404.25,414.75};//X position measuring
    float y_up_z[80];
    std::fill_n(y_up_z,80,758);
    float x_down_position[80]={-429.75,-419.25,-408.75,-398.25,-387.75,-377.25,-366.75,-356.25,-345.75,-335.25,-324.75,-314.25,-303.75,-293.25,-282.75,-272.25,
                               -261.75,-251.25,-240.75,-230.25,-219.75,-209.25,-198.75,-188.25,-177.75,-167.25,-156.75,-146.25,-135.75,-125.25,-114.75,-104.25,
                               -93.75,-83.25,-72.75,-62.25,-51.75,-41.25,-30.75,-20.25,-9.75,0.75,11.25,21.75,32.25,42.75,53.25,63.75,74.25,84.75,95.25,105.75,
                               116.25,126.75,137.25,147.75,158.25,168.75,179.25,189.75,200.25,210.75,221.25,231.75,242.25,252.75,263.25,273.75,284.25,294.75,
                               305.25,315.75,326.25,336.75,347.25,357.75,368.25,378.75,389.25,399.75};//X position measuring
    float x_down_z[80];
    std::fill_n(x_down_z,80,-179);
    float y_down_position[80]={-414.75,-404.25,-393.75,-383.25,-372.75,-362.25,-351.75,-341.25,-330.75,-320.25,-309.75,-299.25,-288.75,-278.25,-267.75,-257.25,
                               -246.75,-236.25,-225.75,-215.25,-204.75,-194.25,-183.75,-173.25,-162.75,-152.25,-141.75,-131.25,-120.75,-110.25,-99.75,-89.25,
                               -78.75,-68.25,-57.75,-47.25,-36.75,-26.25,-15.75,-5.25,5.25,15.75,26.25,36.75,47.25,57.75,68.25,78.75,
                               89.25,99.75,110.25,120.75,131.25,141.75,152.25,162.75,173.25,183.75,194.25,204.75,215.25,225.75,236.25,246.75,
                               257.25,267.75,278.25,288.75,299.25,309.75,320.25,330.75,341.25,351.75,362.25,372.75,383.25,393.75,404.25,414.75};//Y position measuring
    float y_down_z[80];
    std::fill_n(y_down_z,80,-164);
    //
    /*float x_psd_position[41]={-394,-374,-354,-334,-314,-294,-274,-254,-234,-214,-194,-174,-154,
                              -134,-114,-94,-74,-54,-34,-14,6,26,46,66,86,106,126,146,166,186,
                              206,226,246,266,286,306,326,346,366,386,406};//last time*/
    float x_psd_position[41]={374.28,354.28,334.28,314.28,294.28,274.28,254.28,234.28,214.28,194.28,174.28,154.28,134.28,114.28,94.28,74.28,54.28,34.28,14.28,-5.72,-25.72,-45.72,-65.72,-85.72,-105.72,-125.72,-145.72,-165.72,-185.72,-205.72,-225.72,-245.72,-265.72,-285.72,-305.72,-325.72,-345.72,-365.72,-385.72,-405.72,-425.72};
    float x_psd_z[41]={208.2,222.2,208.2,222.2,208.2,222.2,208.2,222.2,208.2,222.2,208.2,222.2,
                       208.2,222.2,208.2,222.2,208.2,222.2,208.2,222.2,208.2,222.2,208.2,222.2,
                       208.2,222.2,208.2,222.2,208.2,222.2,208.2,222.2,208.2,222.2,208.2,222.2,
                       208.2,222.2,208.2,222.2,208.2};
    
    /*float y_psd_position[41]={394.7,374.7,354.7,334.7,314.7,294.7,274.7,254.7,234.7,214.7,194.7,
                              174.7,154.7,134.7,114.7,94.7,74.7,54.7,34.7,14.7,-5.3,-25.3,-45.3,
                              -65.3,-85.3,-105.3,-125.3,-145.3,-165.3,-185.3,-205.3,-225.3,-245.3,
                              -265.3,-285.3,-305.3,-325.3,-345.3,-365.3,-385.3,-405.3};//last time*/
    float y_psd_position[41]={407.96,387.96,367.96,347.96,327.96,307.96,287.96,267.96,247.96,227.96,207.96,187.96,167.96,147.96,127.96,107.96,87.96,67.96,47.96,27.96,7.96,-12.04,-32.04,-52.04,-72.04,-92.04,-112.04,-132.04,-152.04,-172.04,-192.04,-212.04,-232.04,-252.04,-272.04,-292.04,-312.04,-332.04,-352.04,-372.04,-392.04};
    float y_psd_z[41]={182,196,182,196,182,196,182,196,182,196,182,196,182,196,182,196,182,196,182,
                       196,182,196,182,196,182,196,182,196,182,196,182,196,182,196,182,196,182,196,
                       182,196,182};
		       
    float zpsdhit_position[4]={182,196,208.2,222.2};	       
    //COMMEMT: read psd pedestal into buffer for later use
    int id8[41]={0,1,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,22,46,47,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64,65,66};
    int id5[41]={23,24,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,45,68,69,71,72,73,74,75,76,77,78,79,80,81,82,83,84,85,86,87,88};

    int channel;
    float mean,sigma;
    float xpedmean_dy8_pos[41],xpedmean_dy5_pos[41],xpedmean_dy8_neg[41],xpedmean_dy5_neg[41];
    float ypedmean_dy8_pos[41],ypedmean_dy5_pos[41],ypedmean_dy8_neg[41],ypedmean_dy5_neg[41];
    float xpedsigma_dy8_pos[41],xpedsigma_dy5_pos[41],xpedsigma_dy8_neg[41],xpedsigma_dy5_neg[41];
    float ypedsigma_dy8_pos[41],ypedsigma_dy5_pos[41],ypedsigma_dy8_neg[41],ypedsigma_dy5_neg[41];
    TFile *f_ped=new TFile(psd_pedfile);
    if(!f_ped){
      printf("open file error: %s\n",psd_pedfile);
      exit(1);
    }

    TTree *tree_ped=(TTree*)f_ped->Get("xpos_ped");
    tree_ped->SetBranchAddress("channel",&channel);
    tree_ped->SetBranchAddress("mean",&mean);
    tree_ped->SetBranchAddress("sigma",&sigma);
    tree_ped->BuildIndex("channel");
    for(int i=0;i<41;i++){
        tree_ped->GetEntryWithIndex(id8[i]+1);
        xpedmean_dy8_pos[i]=mean;
        xpedsigma_dy8_pos[i]=sigma;

        tree_ped->GetEntryWithIndex(id5[i]+1);
        xpedmean_dy5_pos[i]=mean;
        xpedsigma_dy5_pos[i]=sigma;
    }

    tree_ped=(TTree*)f_ped->Get("xneg_ped");
    tree_ped->SetBranchAddress("channel",&channel);
    tree_ped->SetBranchAddress("mean",&mean);
    tree_ped->SetBranchAddress("sigma",&sigma);
    tree_ped->BuildIndex("channel");
    for(int i=0;i<41;i++){
        tree_ped->GetEntryWithIndex(id8[40-i]+1);
        xpedmean_dy8_neg[i]=mean;
        xpedsigma_dy8_neg[i]=sigma;

        tree_ped->GetEntryWithIndex(id5[40-i]+1);
        xpedmean_dy5_neg[i]=mean;
        xpedsigma_dy5_neg[i]=sigma;
    }

    tree_ped=(TTree*)f_ped->Get("ypos_ped");
    tree_ped->SetBranchAddress("channel",&channel);
    tree_ped->SetBranchAddress("mean",&mean);
    tree_ped->SetBranchAddress("sigma",&sigma);
    tree_ped->BuildIndex("channel");
    for(int i=0;i<41;i++){
        tree_ped->GetEntryWithIndex(id8[40-i]+1);
        ypedmean_dy8_pos[i]=mean;
        ypedsigma_dy8_pos[i]=sigma;

        tree_ped->GetEntryWithIndex(id5[40-i]+1);
        ypedmean_dy5_pos[i]=mean;
        ypedsigma_dy5_pos[i]=sigma;
    }

    tree_ped=(TTree*)f_ped->Get("yneg_ped");
    tree_ped->SetBranchAddress("channel",&channel);
    tree_ped->SetBranchAddress("mean",&mean);
    tree_ped->SetBranchAddress("sigma",&sigma);
    tree_ped->BuildIndex("channel");
    for(int i=0;i<41;i++){
        tree_ped->GetEntryWithIndex(id8[i]+1);
        ypedmean_dy8_neg[i]=mean;
        ypedsigma_dy8_neg[i]=sigma;

        tree_ped->GetEntryWithIndex(id5[i]+1);
        ypedmean_dy5_neg[i]=mean;
        ypedsigma_dy5_neg[i]=sigma;
    }
    delete f_ped;
    
    
    //COMMENT: ------------------------------------------------------------------------------------------------------------
    TFile* file_psd=new TFile(psdfile.Data());
    if(!file_psd){
      printf("open file error: %s\n",psdfile.Data());
      exit(1);
    }
    TTree *tree_psd=(TTree*)file_psd->Get("psd_hitinfo");

    int x_dy8_pos[41],x_dy5_pos[41],x_dy8_neg[41],x_dy5_neg[41];
    int y_dy8_pos[41],y_dy5_pos[41],y_dy8_neg[41],y_dy5_neg[41];

    tree_psd->SetBranchAddress("xpos_dy8",x_dy8_pos);
    tree_psd->SetBranchAddress("xpos_dy5",x_dy5_pos);
    tree_psd->SetBranchAddress("xneg_dy8",x_dy8_neg);
    tree_psd->SetBranchAddress("xneg_dy5",x_dy5_neg);
    tree_psd->SetBranchAddress("ypos_dy8",y_dy8_pos);
    tree_psd->SetBranchAddress("ypos_dy5",y_dy5_pos);
    tree_psd->SetBranchAddress("yneg_dy8",y_dy8_neg);
    tree_psd->SetBranchAddress("yneg_dy5",y_dy5_neg);
    
    //
    TFile* file_mwdc=new TFile(mwdcfile.Data());
    if(!file_mwdc){
      printf("open file error: %s\n",mwdcfile.Data());
      exit(1);
    }
    TTree *tree_mwdc,*tree_multihit;
    file_mwdc->GetObject("merge/mwdc",tree_mwdc);
    file_mwdc->GetObject("merge/mwdc_multihit",tree_multihit);
    
    ChannelMap *mwdc_trailing=0;
    tree_mwdc->SetBranchAddress("trailing_raw",&mwdc_trailing);
    
    Int_t multihit[2][3];
    tree_multihit->SetBranchAddress("multihit",multihit);
    //
    ChannelMap::iterator it;
    UChar_t type,location,direction;
    UShort_t index;
    
    //COMMENT: output tree structure
    float xuphit_position,xuphit_z,yuphit_position,yuphit_z;
    float xdownhit_position,xdownhit_z,ydownhit_position,ydownhit_z;
    float xpsdhit_position[4],ypsdhit_position[4];
    float track_angle=0;
    //info from mwdc
    int xpsd_num,ypsd_num;
    int xpsd_ch[41],ypsd_ch[41];
    float xpsd_x[41],xpsd_z[41];
    float ypsd_y[41],ypsd_z[41];
    float xpsd_energy_pos[41],xpsd_energy_neg[41];
    float ypsd_energy_pos[41],ypsd_energy_neg[41];
    //info from psd
    
    TFile *file_out=new TFile(outfile.Data(),"recreate");
    TTree *tree_final=new TTree("event_simple","event_simple");

    tree_final->Branch("xpsdhit_x",xpsdhit_position,"xpsdhit_x[4]/F");
    tree_final->Branch("ypsdhit_y",ypsdhit_position,"ypsdhit_y[4]/F");
    tree_final->Branch("zpsdhit_z",zpsdhit_position,"zpsdhit_z[4]/F");//expected hit pos in PSD's 4 layers
    tree_final->Branch("xuphit_x",&xuphit_position,"xuphit_x/F");
    tree_final->Branch("xuphit_z",&xuphit_z,"xuphit_z/F");//X of the  hit pos of UP-layer of MWDC in global coordinate system
    tree_final->Branch("yuphit_y",&yuphit_position,"yuphit_y/F");
    tree_final->Branch("yuphit_z",&yuphit_z,"yuphit_z/F");//Y of the  hit pos of UP-layer of MWDC in global coordinate system
    tree_final->Branch("xdownhit_x",&xdownhit_position,"xdownhit_x/F");
    tree_final->Branch("xdownhit_z",&xdownhit_z,"xdownhit_z/F");//X of the  hit pos of Down-layer of MWDC in global coordinate system
    tree_final->Branch("ydownhit_y",&ydownhit_position,"ydownhit_y/F");
    tree_final->Branch("ydownhit_z",&ydownhit_z,"ydownhit_z/F");//Y of the  hit pos of Down-layer of MWDC in global coordinate system
    tree_final->Branch("track_angle",&track_angle,"track_angle/F");//value of the sin(incident_angle)

    tree_final->Branch("xpsd_num",&xpsd_num,"xpsd_num/I");
    tree_final->Branch("xpsd_ch",xpsd_ch,"xpsd_ch[xpsd_num]/I");
    tree_final->Branch("xpsd_x",xpsd_x,"xpsd_x[xpsd_num]/F");//Layer X measures position in X-coordinate
    tree_final->Branch("xpsd_z",xpsd_z,"xpsd_z[xpsd_num]/F");
    tree_final->Branch("xpsd_energy_pos",xpsd_energy_pos,"xpsd_energy_pos[xpsd_num]/F");
    tree_final->Branch("xpsd_energy_neg",xpsd_energy_neg,"xpsd_energy_neg[xpsd_num]/F");
    tree_final->Branch("ypsd_num",&ypsd_num,"ypsd_num/I");
    tree_final->Branch("ypsd_ch",ypsd_ch,"ypsd_ch[ypsd_num]/I");
    tree_final->Branch("ypsd_y",ypsd_y,"ypsd_y[ypsd_num]/F");//Layer Y measures position in Y-coordinate
    tree_final->Branch("ypsd_z",ypsd_z,"ypsd_z[ypsd_num]/F");
    tree_final->Branch("ypsd_energy_pos",ypsd_energy_pos,"ypsd_energy_pos[ypsd_num]/F");
    tree_final->Branch("ypsd_energy_neg",ypsd_energy_neg,"ypsd_energy_neg[ypsd_num]/F");
    
    //COMMEMT: begin the analyze
    UInt_t single_eventnum=0;
    int entries=tree_mwdc->GetEntriesFast();
    if(entries > (tree_psd->GetEntriesFast())){
      entries=tree_psd->GetEntriesFast();
    }
    //for(int i=0;i<100;i++){
    for(int i=0;i<entries;i++){
      //
      tree_mwdc->GetEntry(i);
      tree_multihit->GetEntry(i);
      tree_psd->GetEntry(i);
      //
      if(multihit[0][0]==1 && multihit[0][1]==1 && multihit[1][0]==1 && multihit[1][1]==1){
	//
	single_eventnum++;
	//
	for(it=mwdc_trailing->begin();it!=mwdc_trailing->end();it++){
	  Encoding::Decode(it->first,type,location,direction,index);
	  if (type!=EMWDC) {
	    printf("event_%d:MWDC unmatched type\n",i+1);
	  }
	  if(direction!=2){
	    switch (location) {
	      case 0:
		if(direction==0){
		  xdownhit_position=x_down_position[index];
		  xdownhit_z=x_down_z[index];
		}
		else if(direction==1){
		  ydownhit_position=y_down_position[index];
		  ydownhit_z=y_down_z[index];
		}
		break;
	      case 1:
		if(direction==0){
		  yuphit_position=x_up_position[index];
		  yuphit_z=x_up_z[index];
		}
		else if(direction==1){
		  xuphit_position=y_up_position[index];
		  xuphit_z=y_up_z[index];
		}
		break;
	      default:
		break;
	    }
	  }
	}
	//------------------------------------------------
	for(int j=0;j<4;j++){
	  xpsdhit_position[j]=xdownhit_position+(zpsdhit_position[j]-xdownhit_z)/(xuphit_z-xdownhit_z)*(xuphit_position-xdownhit_position);
	  ypsdhit_position[j]=ydownhit_position+(zpsdhit_position[j]-ydownhit_z)/(yuphit_z-ydownhit_z)*(yuphit_position-ydownhit_position);
	  
	}//psd down to up (0-->3)
	
	//track angle
	track_angle=((xuphit_z+yuphit_z)/2.0-(xdownhit_z+ydownhit_z)/2.0)/
	TMath::Sqrt(TMath::Power(((xuphit_z+yuphit_z)/2.0-(xdownhit_z+ydownhit_z)/2.0),2)+TMath::Power((xuphit_position-xdownhit_position),2)+TMath::Power((yuphit_position-ydownhit_position),2));
	//COMMENT: PSD info extraction
	xpsd_num=0;ypsd_num=0;
	for(int j=0;j<41;j++){
	  if(x_dy8_pos[j] > (xpedmean_dy8_pos[j]+ped_cut*xpedsigma_dy8_pos[j])
	    || x_dy8_neg[j] > (xpedmean_dy8_neg[j]+ped_cut*xpedsigma_dy8_neg[j]))
	  {
	    xpsd_ch[xpsd_num]=j+1;
	    xpsd_x[xpsd_num]=x_psd_position[j]+psdoffset_x;
	    xpsd_z[xpsd_num]=x_psd_z[j];
	    xpsd_energy_pos[xpsd_num]=x_dy8_pos[j]-xpedmean_dy8_pos[j];
	    xpsd_energy_neg[xpsd_num]=x_dy8_neg[j]-xpedmean_dy8_neg[j];
	    xpsd_num++;
	  }
	}
	
	for(int j=0;j<41;j++){
	  if(y_dy8_pos[j] > (ypedmean_dy8_pos[j]+ped_cut*ypedsigma_dy8_pos[j])
	    || y_dy8_neg[j] > (ypedmean_dy8_neg[j]+ped_cut*ypedsigma_dy8_neg[j]))
	  {
	    ypsd_ch[ypsd_num]=j+1;
	    ypsd_y[ypsd_num]=y_psd_position[j]+psdoffset_y;
	    ypsd_z[ypsd_num]=y_psd_z[j];
	    ypsd_energy_pos[ypsd_num]=y_dy8_pos[j]-ypedmean_dy8_pos[j];
	    ypsd_energy_neg[ypsd_num]=y_dy8_neg[j]-ypedmean_dy8_neg[j];
	    ypsd_num++;
	  }
	}
	
	//COMMEMT: fill the tree
	tree_final->Fill();
      }
    }
    
    printf("%d events processed totally\n",entries);
    printf("about %.4f%% percent are single hit events\n",(float)single_eventnum/entries);
    //
    file_out->cd();
    tree_final->Write(0,TObject::kOverwrite);
    //
    delete file_mwdc;
    delete file_psd;
    delete file_out;
    
    return 0;
}

int draw_trajectory_final(char* infile)
{
    TFile *f_in=new TFile(infile);
    TTree *tree_final=(TTree*)f_in->Get("event_simple");

    float xuphit_position,xuphit_z,yuphit_position,yuphit_z;
    float xdownhit_position,xdownhit_z,ydownhit_position,ydownhit_z;
    float xpsdhit_position[4],ypsdhit_position[4],zpsdhit_position[4];
    float track_angle=0;
    //info from mwdc
    int xpsd_num,ypsd_num;
    int xpsd_ch[41],ypsd_ch[41];
    float xpsd_x[41],xpsd_z[41];
    float ypsd_y[41],ypsd_z[41];
    float xpsd_energy_pos[41],xpsd_energy_neg[41];
    float ypsd_energy_pos[41],ypsd_energy_neg[41];
    //info from psd

    tree_final->SetBranchAddress("xpsdhit_x",xpsdhit_position);
    tree_final->SetBranchAddress("ypsdhit_y",ypsdhit_position);
    tree_final->SetBranchAddress("zpsdhit_z",zpsdhit_position);
    tree_final->SetBranchAddress("xuphit_x",&xuphit_position);
    tree_final->SetBranchAddress("xuphit_z",&xuphit_z);
    tree_final->SetBranchAddress("yuphit_y",&yuphit_position);
    tree_final->SetBranchAddress("yuphit_z",&yuphit_z);
    tree_final->SetBranchAddress("xdownhit_x",&xdownhit_position);
    tree_final->SetBranchAddress("xdownhit_z",&xdownhit_z);
    tree_final->SetBranchAddress("ydownhit_y",&ydownhit_position);
    tree_final->SetBranchAddress("ydownhit_z",&ydownhit_z);
    tree_final->SetBranchAddress("track_angle",&track_angle);

    tree_final->SetBranchAddress("xpsd_num",&xpsd_num);
    tree_final->SetBranchAddress("xpsd_ch",xpsd_ch);
    tree_final->SetBranchAddress("xpsd_x",xpsd_x);
    tree_final->SetBranchAddress("xpsd_z",xpsd_z);
    tree_final->SetBranchAddress("xpsd_energy_pos",xpsd_energy_pos);
    tree_final->SetBranchAddress("xpsd_energy_neg",xpsd_energy_neg);
    tree_final->SetBranchAddress("ypsd_num",&ypsd_num);
    tree_final->SetBranchAddress("ypsd_ch",ypsd_ch);
    tree_final->SetBranchAddress("ypsd_y",ypsd_y);
    tree_final->SetBranchAddress("ypsd_z",ypsd_z);
    tree_final->SetBranchAddress("ypsd_energy_pos",ypsd_energy_pos);
    tree_final->SetBranchAddress("ypsd_energy_neg",ypsd_energy_neg);


    TCanvas *can=new TCanvas("can","can",900,460);
    can->Divide(2,1);
    TGraph* gx_truehit=new TGraph;
    TGraph* gy_truehit=new TGraph;
    gx_truehit->SetMarkerStyle(25);
    gx_truehit->SetMarkerColor(kRed);
    gx_truehit->SetTitle("X Profile");
    gy_truehit->SetMarkerStyle(25);
    gy_truehit->SetMarkerColor(kRed);
    gy_truehit->SetTitle("Y Profile");

    TGraph* gx_expected=new TGraph(4);
    TGraph* gy_expected=new TGraph(4);
    gx_expected->SetMarkerStyle(3);
    gx_expected->SetMarkerColor(kBlue);
    gy_expected->SetMarkerStyle(3);
    gy_expected->SetMarkerColor(kBlue);

    TMultiGraph* gmultix=new TMultiGraph("multix","multix");
    gmultix->Add(gx_truehit);
    gmultix->Add(gx_expected);
    TMultiGraph* gmultiy=new TMultiGraph("multiy","multiy");
    gmultiy->Add(gy_truehit);
    gmultiy->Add(gy_expected);

    int next_entry,range;
    next_entry=2;range=10;
    char input;
    int hitnum_x,hitnum_y;

    int entries;
    entries=tree_final->GetEntriesFast();
    printf("entries= %d",entries);
    for(int i=0;i<entries;i++){
        tree_final->GetEntry(i);
        //--------------------------------------
        hitnum_x=0;hitnum_y=0;
        gx_truehit->SetPoint(hitnum_x,xuphit_position,xuphit_z);hitnum_x++;
        gx_truehit->SetPoint(hitnum_x,xdownhit_position,xdownhit_z);hitnum_x++;
        for(int j=0;j<xpsd_num;j++){
            gx_truehit->SetPoint(hitnum_x,xpsd_x[j],xpsd_z[j]);
            hitnum_x++;
        }
        gx_truehit->Set(hitnum_x);

        gy_truehit->SetPoint(hitnum_y,yuphit_position,yuphit_z);hitnum_y++;
        gy_truehit->SetPoint(hitnum_y,ydownhit_position,ydownhit_z);hitnum_y++;
        for(int j=0;j<ypsd_num;j++){
            gy_truehit->SetPoint(hitnum_y,ypsd_y[j],ypsd_z[j]);
            hitnum_y++;
        }
        gy_truehit->Set(hitnum_y);

        for(int j=0;j<4;j++){
            gx_expected->SetPoint(j,xpsdhit_position[j],zpsdhit_position[j]);
            gy_expected->SetPoint(j,ypsdhit_position[j],zpsdhit_position[j]);
        }

        can->cd(1);
        gPad->DrawFrame(-500,-300,500,800,"X");
        //gx_truehit->Draw("P");
        gmultix->Draw("P");

        can->cd(2);
        gPad->DrawFrame(-500,-300,500,800,"Y");
        //gy_truehit->Draw("P");
        gmultiy->Draw("P");

        can->Update();

        printf("total event:%d,  current event: %d\n",entries,i+1);
        if((i-next_entry)>range){
            printf("input the next event you wanna see(event_init,range):\n");
            scanf("%d %d",&next_entry,&range);
            while(next_entry-2 > entries ){
                printf("error! there are only %d events,please enter another eventnum:\n",entries+1);
                scanf("%d %d",&next_entry,&range);
            }
            i=next_entry-2;
        }
        else{
            input=getchar();
            if(input == 'q')
                break;
        }

    }

    return 0;
}
