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
#include "TMath.h"
#include "TH1F.h"
#include "TH2F.h"


int ana_position_resolution(char* infile)
{
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

    TH1F* hx=new TH1F("hx_resolution","X position resolution",400,-200,200);
    TH1F* hy=new TH1F("hy_resolution","Y position resolution",400,-200,200);
    TH2F* hup=new TH2F("hmwdc_up","MWDC_up Hit Distribution ",82,-430.5,430.5,82,-430.5,430.5);
    TH2F* hdown=new TH2F("hmwdc_down","MWDC_down Hit Distribution",82,-445.5,415.5,82,-430.5,430.5);
    TH2F* hpsd_true=new TH2F("hpsd_true","PSD recontructed Hit Distribution",180,-435.3,434.7,180,-424,436);
    TH2F* hpsd_expected=new TH2F("hpsd_expected","PSD expected hit distribution",180,-435.3,434.7,180,-424,436);

    float x_truehit,y_truehit;
    float x_expected,y_expected;
    int x_flag,y_flag;
    int entries=tree_final->GetEntries();
    printf("entries= %d",entries);
    for(int i=0;i<entries;i++){
        tree_final->GetEntry(i);

        x_flag=0;y_flag=0;
        if(xpsd_num>2 || ypsd_num>2 || xpsd_num == 0 || ypsd_num == 0)
            continue;
        else{

            switch(ypsd_num){
            case 1:
                y_truehit=ypsd_y[0];
                y_expected=ypsdhit_position[(ypsd_ch[0]-1)%2];
                //y_expected=ypsdhit_position[2];
                y_flag=1;
                break;
            case 2:
                if(TMath::Abs(ypsd_ch[0]-ypsd_ch[1]) == 1){
                    y_truehit=(ypsd_y[0]+ypsd_y[1])/2.0;
                    y_expected=(ypsdhit_position[0]+ypsdhit_position[1])/2.0;
                    y_flag=1;
                }
                break;
            }

            switch(xpsd_num){
            case 1:
                x_truehit=xpsd_x[0];
                x_expected=xpsdhit_position[(xpsd_ch[0]-1)%2+2];
                //x_expected=xpsdhit_position[0];
                x_flag=1;
                break;
            case 2:
                if(TMath::Abs(xpsd_ch[0]-xpsd_ch[1]) == 1){
                    x_truehit=(xpsd_x[0]+xpsd_x[1])/2.0;
                    x_expected=(xpsdhit_position[2]+xpsdhit_position[3])/2.0;
                    x_flag=1;
                }
                break;
            }

            hup->Fill(xuphit_position,yuphit_position);
            hdown->Fill(xdownhit_position,ydownhit_position);
            if(x_flag==1 && y_flag==1){
                hx->Fill((x_expected-x_truehit));
                hy->Fill((y_expected-y_truehit));

                hpsd_true->Fill(x_truehit,y_truehit);
                hpsd_expected->Fill(x_expected,y_expected);
            }
        }
    }

    hx->Write(0,TObject::kOverwrite);
    hy->Write(0,TObject::kOverwrite);
    hup->Write(0,TObject::kOverwrite);
    hdown->Write(0,TObject::kOverwrite);
    hpsd_expected->Write(0,TObject::kOverwrite);
    hpsd_true->Write(0,TObject::kOverwrite);

    delete f_in;

    return 0;
}
