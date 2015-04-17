/*
 T his *program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.
 
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor,
 Boston, MA  02110-1301, USA.
 
 ---
 Copyright (C) 2015, ufan <>
 */

#include "TFile.h"
#include "TTree.h"
#include "TH1F.h"

//--------------------------------------------
int ana_efficiency(char *infile, char *outfile,float psdoffset_x=6.35,float psdoffset_y=-2.21,float mwdc_range=13.0)//mwdc_range >8.0
{
    float x_psd_position[41]={374.28,354.28,334.28,314.28,294.28,274.28,254.28,234.28,214.28,194.28,174.28,154.28,134.28,114.28,94.28,74.28,54.28,34.28,14.28,-5.72,-25.72,-45.72,-65.72,-85.72,-105.72,-125.72,-145.72,-165.72,-185.72,-205.72,-225.72,-245.72,-265.72,-285.72,-305.72,-325.72,-345.72,-365.72,-385.72,-405.72,-425.72};

    float y_psd_position[41]={407.96,387.96,367.96,347.96,327.96,307.96,287.96,267.96,247.96,227.96,207.96,187.96,167.96,147.96,127.96,107.96,87.96,67.96,47.96,27.96,7.96,-12.04,-32.04,-52.04,-72.04,-92.04,-112.04,-132.04,-152.04,-172.04,-192.04,-212.04,-232.04,-252.04,-272.04,-292.04,-312.04,-332.04,-352.04,-372.04,-392.04};

    for(int i=0;i<41;i++){
      x_psd_position[i]+=psdoffset_x;
      y_psd_position[i]+=psdoffset_y;
    }
    //----------------------------------------------------------------------------------------------
    TFile *f_in=new TFile(infile,"update");
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


    //TEntryList* list_nodetected=new TEntryList("list_nodetected","list_nodetected");
    //list_nodetected->SetTree(tree_final);

    int entries=tree_final->GetEntries();
    //----------------------------------------------------
    float mwdc_cell=mwdc_range/2.0;
    int xwrongevent_flag,ywrongevent_flag,detect_flag;
    int xpsd_hitted_ch,ypsd_hitted_ch,xpsd_detected_ch,ypsd_detected_ch;
    int xpsd_hitted[41],xpsd_detected[41];
    int ypsd_hitted[41],ypsd_detected[41];
    int total_hit,total_detected;
    for(int i=0;i<41;i++){
        xpsd_hitted[i]=0;
        xpsd_detected[i]=0;
        ypsd_hitted[i]=0;
        ypsd_detected[i]=0;
    }
    total_detected=0;total_hit=0;

    for(int i=0;i<entries;i++){
        tree_final->GetEntry(i);
        //-----------------------------------------------
        xwrongevent_flag=0;ywrongevent_flag=0;
        xpsd_hitted_ch=-1;ypsd_hitted_ch=-1;
        xpsd_detected_ch=-1;ypsd_detected_ch=-1;

        for(int j=1;j<40;j++){
            if((ypsdhit_position[j%2]+mwdc_cell)<(y_psd_position[j]+14) && (ypsdhit_position[j%2]-mwdc_cell)>(y_psd_position[j]-14))
            {
                ypsd_hitted[j]++;
                detect_flag=0;
                ypsd_hitted_ch=j;
                for(int k=0;k<ypsd_num;k++){
                    if(ypsd_ch[k]==(j+1)){
                        ypsd_detected[j]++;
                        detect_flag=1;
                        ypsd_detected_ch=j;
                        break;
                    }
                }
                if((detect_flag!=1) && ypsd_num>0){
                    //ypsd_hitted[j]--;
                    ywrongevent_flag=1;
                }
            }
            if((xpsdhit_position[j%2+2]+mwdc_cell)<(x_psd_position[j]+14) && (xpsdhit_position[j%2+2]-mwdc_cell)>(x_psd_position[j]-14))
             {
                xpsd_hitted[j]++;
                detect_flag=0;
                xpsd_hitted_ch=j;
                for(int k=0;k<xpsd_num;k++){
                    if(xpsd_ch[k]==(j+1)){
                        xpsd_detected[j]++;
                        detect_flag=1;
                        xpsd_detected_ch=j;
                        break;
                    }
                }
                if((detect_flag!=1) && xpsd_num>0){
                    //xpsd_hitted[j]--;
                    xwrongevent_flag=1;
                }
            }
        }
        if((ypsdhit_position[0]+mwdc_cell)<(y_psd_position[0]+11) && (ypsdhit_position[0]-mwdc_cell)>(y_psd_position[0]-14)
                && (xpsdhit_position[0]+mwdc_cell)<(x_psd_position[0]+11) && (xpsdhit_position[0]-mwdc_cell)>(x_psd_position[40]-11))
        {
            ypsd_hitted[0]++;
            detect_flag=0;
            ypsd_hitted_ch=0;
            for(int k=0;k<ypsd_num;k++){
                if(ypsd_ch[k]==1){
                    ypsd_detected[0]++;
                    detect_flag=1;
                    ypsd_detected_ch=0;
                    break;
                }
            }
            if((detect_flag!=1) && ypsd_num>0){
                //ypsd_hitted[0]--;
                ywrongevent_flag=1;
            }
            else if((detect_flag!=1) && ypsd_num==0){
                if(xpsd_num==0){
                    ypsd_hitted[0]--;
                }
            }
        }
        if((ypsdhit_position[0]+mwdc_cell)<(y_psd_position[40]+14) && (ypsdhit_position[0]-mwdc_cell)>(y_psd_position[40]-11)
                && (xpsdhit_position[0]+mwdc_cell)<(x_psd_position[0]+11) && (xpsdhit_position[0]-mwdc_cell)>(x_psd_position[40]-11))
        {

            ypsd_hitted[40]++;
            detect_flag=0;
            ypsd_hitted_ch=40;
            for(int k=0;k<ypsd_num;k++){
                if(ypsd_ch[k]==41){
                    ypsd_detected[40]++;
                    detect_flag=1;
                    ypsd_detected_ch=40;
                    break;
                }
            }
            if((detect_flag!=1) && ypsd_num>0){
                //ypsd_hitted[40]--;
                ywrongevent_flag=1;
            }
            else if((detect_flag!=1) && ypsd_num==0){
                if(xpsd_num==0){
                    ypsd_hitted[40]--;
                }
            }
        }
        if((xpsdhit_position[2]+mwdc_cell)<(x_psd_position[0]+11) && (xpsdhit_position[2]-mwdc_cell)>(x_psd_position[0]-14)
                && (ypsdhit_position[2]+mwdc_cell)<(y_psd_position[0]+11) && (ypsdhit_position[2]-mwdc_cell)>(y_psd_position[40]-11))
        {
            xpsd_hitted[0]++;
            detect_flag=0;
            xpsd_hitted_ch=0;
            for(int k=0;k<xpsd_num;k++){
                if(xpsd_ch[k]==1){
                    xpsd_detected[0]++;
                    detect_flag=1;
                    xpsd_detected_ch=0;
                    break;
                }
            }
            if((detect_flag!=1) && xpsd_num>0){
                //xpsd_hitted[0]--;
                xwrongevent_flag=1;
            }
            else if((detect_flag!=1 && xpsd_num == 0)){
                if(ypsd_num==0){
                    xpsd_hitted[0]--;
                }
            }
        }
        if((xpsdhit_position[2]+mwdc_cell)<(x_psd_position[40]+14) && (xpsdhit_position[2]-mwdc_cell)>(x_psd_position[40]-11)
                && (ypsdhit_position[2]+mwdc_cell)<(y_psd_position[0]+11) && (ypsdhit_position[2]-mwdc_cell)>(y_psd_position[40]-11))
        {
            xpsd_hitted[40]++;
            detect_flag=0;
            xpsd_hitted_ch=40;
            for(int k=0;k<xpsd_num;k++){
                if(xpsd_ch[k]==41){
                    xpsd_detected[40]++;
                    detect_flag=1;
                    xpsd_detected_ch=40;
                    break;
                }
            }
            if((detect_flag!=1) && xpsd_num>0){
                //xpsd_hitted[40]--;
                xwrongevent_flag=1;
            }
            else if((detect_flag!=1 && xpsd_num == 0)){
                if(ypsd_num==0){
                    xpsd_hitted[40]--;
                }
            }
        }
        //---------------------------------------
        if(xwrongevent_flag==1 || ywrongevent_flag==1){
            if(xpsd_hitted_ch>-1)
                xpsd_hitted[xpsd_hitted_ch]--;
            if(xpsd_detected_ch>-1)
                xpsd_detected[xpsd_detected_ch]--;
            if(ypsd_hitted_ch>-1)
                ypsd_hitted[ypsd_hitted_ch]--;
            if(ypsd_detected_ch>-1)
                ypsd_detected[ypsd_detected_ch]--;
        }
        //else
        {

            int hit_flag=0;
            for(int j=0;j<4;j++){
                if((xpsdhit_position[j]+mwdc_cell)<(x_psd_position[0]+11) && (xpsdhit_position[j]-mwdc_cell)>(x_psd_position[40]-11)
                        && (ypsdhit_position[j]+mwdc_cell)<(y_psd_position[0]+11) && (ypsdhit_position[j]-mwdc_cell)>(y_psd_position[40]-11))
                {
                    hit_flag++;
                }
            }

            if(hit_flag==4){
                total_hit++;
                if(xpsd_num >0 || ypsd_num >0){
                    total_detected++;
                }
                /*
                else{
                    list_nodetected->Enter(entrynum);
                }
                */
            }

        }
    }

    f_in->cd();

    TH1F* heff_x=new TH1F("heff_x","X layer efficiency distribution",43,-0.5,42.5);
    TH1F* hdet_x=new TH1F("hdet_x","X layer detected_mip distribution",43,-0.5,42.5);
    TH1F* hhit_x=new TH1F("hhit_x","X layer hit_mip destribution",43,-0.5,42.5);
    TH1F* heff_y=new TH1F("heff_y","Y layer efficiency distribution",43,-0.5,42.5);
    TH1F* hdet_y=new TH1F("hdet_y","Y layer detected_mip distribution",43,-0.5,42.5);
    TH1F* hhit_y=new TH1F("hhit_y","Y layer hit_mip destribution",43,-0.5,42.5);

    FILE* fp=fopen(outfile,"w");
    for(int i=0;i<41;i++){
        fprintf(fp,"x%d : %d, %d, %.5f%%\t\t y%d : %d, %d ,%.5f%%\n",i+1,xpsd_hitted[i],xpsd_detected[i],100.0*xpsd_detected[i]/xpsd_hitted[i]
                ,i+1,ypsd_hitted[i],ypsd_detected[i],100.0*ypsd_detected[i]/ypsd_hitted[i]);

        heff_x->SetBinContent(i+2,1.0*xpsd_detected[i]/xpsd_hitted[i]);
        hdet_x->SetBinContent(i+2,xpsd_detected[i]);
        hhit_x->SetBinContent(i+2,xpsd_hitted[i]);
        heff_y->SetBinContent(i+2,1.0*ypsd_detected[i]/ypsd_hitted[i]);
        hdet_y->SetBinContent(i+2,ypsd_detected[i]);
        hhit_y->SetBinContent(i+2,ypsd_hitted[i]);
    }

    heff_x->GetYaxis()->SetRangeUser(0.9,1.01);
    heff_y->GetYaxis()->SetRangeUser(0.9,1.01);
    heff_x->Write(0,TObject::kOverwrite);
    hdet_x->Write(0,TObject::kOverwrite);
    hhit_x->Write(0,TObject::kOverwrite);
    heff_y->Write(0,TObject::kOverwrite);
    hdet_y->Write(0,TObject::kOverwrite);
    hhit_y->Write(0,TObject::kOverwrite);
    //list_nodetected->Write(0,TObject::kOverwrite);
    //list_xout->Write(0,TObject::kOverwrite);
    //list_yout->Write(0,TObject::kOverwrite);

    delete f_in;

    printf("\ntotal hit: %d\n",total_hit);
    fprintf(fp,"total hit: %d\n",total_hit);
    printf("total_detected: %d \n",total_detected);
    fprintf(fp,"total_detected: %d \n",total_detected);
    printf("total efficiency: %f%%\n",100.0*total_detected/total_hit);
    fprintf(fp,"total efficiency: %f%%\n",100.0*total_detected/total_hit);

    fclose(fp);

    return 0;
}