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
#include <algorithm>

int draw_trajectory(const char* mwdc_file,const char* psd_file,const char* psd_pedfile)
{
    int x_hitted,y_hitted;
    char input;
    int next_entry,range;
    next_entry=2;range=10;
    TGraph *gx=new TGraph;
    gx->SetTitle("X Profile");
    TGraph *gy=new TGraph;
    gy->SetTitle("Y Profile");

    TCanvas* can=new TCanvas("can","can",800,420);
    can->Divide(2,1);
    //
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
    float x_psd_position[41]={374.28,354.28,334.28,314.28,294.28,274.28,254.28,234.28,214.28,194.28,174.28,154.28,134.28,114.28,94.28,74.28,54.28,34.28,14.28,-5.72,-25.72,-45.72,-65.72,-85.72,-105.72,-125.72,-145.72,-165.72,-185.72,-205.72,-225.72,-245.72,-265.72,-285.72,-305.72,-325.72,-345.72,-365.72,-385.72,-405.72,-425.72
};
    float x_psd_z[41]={208.2,222.2,208.2,222.2,208.2,222.2,208.2,222.2,208.2,222.2,208.2,222.2,
                       208.2,222.2,208.2,222.2,208.2,222.2,208.2,222.2,208.2,222.2,208.2,222.2,
                       208.2,222.2,208.2,222.2,208.2,222.2,208.2,222.2,208.2,222.2,208.2,222.2,
                       208.2,222.2,208.2,222.2,208.2};
    
    /*float y_psd_position[41]={394.7,374.7,354.7,334.7,314.7,294.7,274.7,254.7,234.7,214.7,194.7,
                              174.7,154.7,134.7,114.7,94.7,74.7,54.7,34.7,14.7,-5.3,-25.3,-45.3,
                              -65.3,-85.3,-105.3,-125.3,-145.3,-165.3,-185.3,-205.3,-225.3,-245.3,
                              -265.3,-285.3,-305.3,-325.3,-345.3,-365.3,-385.3,-405.3};//last time*/
    float y_psd_position[41]={407.96,387.96,367.96,347.96,327.96,307.96,287.96,267.96,247.96,227.96,207.96,187.96,167.96,147.96,127.96,107.96,87.96,67.96,47.96,27.96,7.96,-12.04,-32.04,-52.04,-72.04,-92.04,-112.04,-132.04,-152.04,-172.04,-192.04,-212.04,-232.04,-252.04,-272.04,-292.04,-312.04,-332.04,-352.04,-372.04,-392.04
};
    float y_psd_z[41]={182,196,182,196,182,196,182,196,182,196,182,196,182,196,182,196,182,196,182,
                       196,182,196,182,196,182,196,182,196,182,196,182,196,182,196,182,196,182,196,
                       182,196,182};
  //
    int id8[41]={0,1,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,22,46,47,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64,65,66};
    int id5[41]={23,24,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,45,68,69,71,72,73,74,75,76,77,78,79,80,81,82,83,84,85,86,87,88};

    int channel;
    float mean,sigma;
    float xpedmean_dy8_pos[41],xpedmean_dy5_pos[41],xpedmean_dy8_neg[41],xpedmean_dy5_neg[41];
    float ypedmean_dy8_pos[41],ypedmean_dy5_pos[41],ypedmean_dy8_neg[41],ypedmean_dy5_neg[41];
    float xpedsigma_dy8_pos[41],xpedsigma_dy5_pos[41],xpedsigma_dy8_neg[41],xpedsigma_dy5_neg[41];
    float ypedsigma_dy8_pos[41],ypedsigma_dy5_pos[41],ypedsigma_dy8_neg[41],ypedsigma_dy5_neg[41];
    TFile *f_ped=new TFile(psd_pedfile);

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
//------------------------------------------------------------------------------------------------------------
    TFile* file_psd=new TFile(psd_file);
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
  TString label_location[2]={"Down","Up"};
  TString label_direction[3]={"X","Y","U"};
/*
  //define histogram
  std::vector<TH1F*> histrepo[2];
  TH1* htemp;
  for(int i=0;i<2;i++){
    for(int j=0;j<3;j++){
      htemp=(TH1*)gROOT->FindObject("h"+label_direction[j]+"_"+label_location[i]+"_"+"multihit");
      if(htemp)	{
	delete htemp;
      }
      histrepo[i].push_back(new TH1F("h"+label_direction[j]+"_"+label_location[i]+"_"+"multihit",label_direction[j]+"_"+label_location[i]+"_"+"multihit",11,-0.5,10.5));
    }
  }
*/
  //
  TFile* file_out=new TFile(mwdc_file);
  if(!file_out){
    printf("open file error: %s\n",mwdc_file);
    exit(1);
  }
  //
  TTree *tree_mwdc,*tree_tof,*tree_multihit;
  file_out->GetObject("merge/mwdc",tree_mwdc);
  file_out->GetObject("merge/tof",tree_tof);
  file_out->GetObject("merge/mwdc_multihit",tree_multihit);

  ChannelMap *mwdc_leading=0,*mwdc_trailing=0;
  tree_mwdc->SetBranchAddress("leading_raw",&mwdc_leading);
  tree_mwdc->SetBranchAddress("trailing_raw",&mwdc_trailing);
  /*
  ChannelMap *tof_timeleading=0,*tof_timetrailing=0,*tof_totleading=0,*tof_tottrailing=0;
  tree_tof->SetBranchAddress("time_leading_raw",&tof_timeleading);
  tree_tof->SetBranchAddress("time_trailing_raw",&tof_timetrailing);
  tree_tof->SetBranchAddress("tot_leading_raw",&tof_totleading);
  tree_tof->SetBranchAddress("tot_trailing_raw",&tof_tottrailing);
  */
  Int_t multihit[2][3];
  tree_multihit->SetBranchAddress("multihit",multihit);
  //
  int entries=tree_mwdc->GetEntriesFast();
  if(entries > (tree_psd->GetEntriesFast())){
    entries=tree_psd->GetEntriesFast();
  }
  ChannelMap::iterator it;
  UChar_t type,location,direction;
  UShort_t index;
  //for(int i=0;i<100;i++){
  for(int i=0;i<entries;i++){
    tree_mwdc->GetEntry(i);
    //tree_tof->GetEntry(i);
    tree_multihit->GetEntry(i);
    tree_psd->GetEntry(i);
    //
    if(multihit[0][0]==1 && multihit[0][1]==1 && multihit[1][0]==1 && multihit[1][1]==1){
      x_hitted=0;y_hitted=0;
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
		gx->SetPoint(x_hitted,x_down_position[index],x_down_z[index]);
		x_hitted++;
	      }
	      else if(direction==1){
		gy->SetPoint(y_hitted,y_down_position[index],y_down_z[index]);
		y_hitted++;
	      }
	      break;
	    case 1:
	      if(direction==0){
		gy->SetPoint(y_hitted,x_up_position[index],x_up_z[index]);
		y_hitted++;
	      }
	      else if(direction==1){
		gx->SetPoint(x_hitted,y_up_position[index],y_up_z[index]);
		x_hitted++;
	      }
	      break;
	    default:
	      break;
	  }
	}
      }
      //
      for(int j=0;j<41;j++){
            if(x_dy8_pos[j] > (xpedmean_dy8_pos[j]+5*xpedsigma_dy8_pos[j])
                    || x_dy8_neg[j] > (xpedmean_dy8_neg[j]+5*xpedsigma_dy8_neg[j]))
            {
                gx->SetPoint(x_hitted,x_psd_position[j],x_psd_z[j]);
                x_hitted++;
            }
      }
        
      for(int j=0;j<41;j++){
           if(y_dy8_pos[j] > (ypedmean_dy8_pos[j]+5*ypedsigma_dy8_pos[j])
                    || y_dy8_neg[j] > (ypedmean_dy8_neg[j]+5*ypedsigma_dy8_neg[j]))
            {
                gy->SetPoint(y_hitted,y_psd_position[j],y_psd_z[j]);
                y_hitted++;
            }
      }
        
    }
    else{
      continue;
    }
    /*
    for(it=mwdc_leading->begin();it!=mwdc_leading->end();it++){
      Encoding::Decode(it->first,type,location,direction,index);
      if (type!=EMWDC) {
	printf("event_%d:MWDC unmatched type\n",i+1);
      }
    }
    for(it=mwdc_trailing->begin();it!=mwdc_trailing->end();it++){
      Encoding::Decode(it->first,type,location,direction,index);
      if (type!=EMWDC) {
	printf("event_%d:MWDC unmatched type\n",i+1);
      }
    }
    //
    for(it=tof_timeleading->begin();it!=tof_timeleading->end();it++){
      Encoding::Decode(it->first,type,location,direction,index);
      if (type!=ETOF) {
	printf("event_%d:TOF unmatched type\n",i+1);
      }
    }
    for(it=tof_timetrailing->begin();it!=tof_timetrailing->end();it++){
      Encoding::Decode(it->first,type,location,direction,index);
      if (type!=ETOF) {
	printf("event_%d:TOF unmatched type\n",i+1);
      }
    }
    for(it=tof_totleading->begin();it!=tof_totleading->end();it++){
      Encoding::Decode(it->first,type,location,direction,index);
      if (type!=ETOF) {
	printf("event_%d:TOF unmatched type\n",i+1);
      }
    }
    for(it=tof_tottrailing->begin();it!=tof_tottrailing->end();it++){
      Encoding::Decode(it->first,type,location,direction,index);
      if (type!=ETOF) {
	printf("event_%d:TOF unmatched type\n",i+1);
      }
    }
    */
    gx->Set(x_hitted);
    can->cd(1);
    gPad->DrawFrame(-500,-300,500,800,"X");
    gx->Draw("*");
    
    gy->Set(y_hitted);
    can->cd(2);
    gPad->DrawFrame(-500,-300,500,800,"Y");
    gy->Draw("*");
    can->Update();
    //------------------------------------------
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
  
  printf("%d events processed totally\n",entries);
  
  //
  delete file_out;
  delete file_psd;
  delete can;
  
  return 0;
}

