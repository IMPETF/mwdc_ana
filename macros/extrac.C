{
TFile* file=new TFile("/home/ufan/workspace/mwdc/formal_test/data/test43/mwdc43.root");

TTree* mwdc=(TTree*)file->Get("merge/mwdc");
TTree* tof=(TTree*)file->Get("merge/tof");
TTree* mwdc_multihit=(TTree*)file->Get("merge/mwdc_multihit");
/*
ChannelMap *mwdc_leading=0,*mwdc_trailing=0;
tree_mwdc->SetBranchAddress("leading_raw",&mwdc_leading);
tree_mwdc->SetBranchAddress("trailing_raw",&mwdc_trailing);
ChannelMap *tof_timeleading=0,*tof_timetrailing=0,*tof_totleading=0,*tof_tottrailing=0;
tree_tof->SetBranchAddress("time_leading_raw",&tof_timeleading);
tree_tof->SetBranchAddress("time_trailing_raw",&tof_timetrailing);
tree_tof->SetBranchAddress("tot_leading_raw",&tof_totleading);
tree_tof->SetBranchAddress("tot_trailing_raw",&tof_tottrailing);
Int_t multihit[2][3];
tree_multihit->SetBranchAddress("multihit",multihit);
*/
TFile* fileout=new TFile("example.root","recreate");

TDirectory* dir=fileout->mkdir("merge");
dir->cd();

TTree* mwdc_clone=(TTree*)mwdc->CloneTree(10000);
TTree* tof_clone=(TTree*)tof->CloneTree(10000);
TTree* mwdc_multihit_clone=(TTree*)mwdc_multihit->CloneTree(10000);

mwdc_clone->Write();
tof_clone->Write();
mwdc_multihit_clone->Write();

delete fileout;

delete file;
}
