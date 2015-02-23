/*
void pre_process(const char* datadir,const char* outfile,const char* prefix,const char* logfile,int bunchid_diff)
{
  //Utility::convert_hptdc(datadir,outfile,prefix,".dat");
  Utility::convert_hptdc(datadir,outfile,prefix);
  Utility::merge_hptdc(datadir,outfile);
  Utility::print_info(datadir,outfile,logfile,bunchid_diff,prefix);
  Utility::check(datadir,outfile);
}

void process(const char* datadir,const char* outfile)
{
  gROOT->ProcessLine(".L draw_multihit.C+");
  gROOT->ProcessLine(".L draw_noise.C+");
  gROOT->ProcessLine(".L draw_mwdctot.C+");
  gROOT->ProcessLine(".L draw_tof.C+");
  gROOT->ProcessLine(".L draw_drift.C+");

  draw_multihit(datadir,outfile);
  draw_noise_merge(datadir,outfile);
  draw_mwdctot(datadir,outfile);
  draw_tof(datadir,outfile);
  draw_drift(datadir,outfile);
}
*/

int process_mwdc(const char* datadir,const char* outfile,const char* prefix)
{
  TString configdir=TString(datadir)+"/../../";
  Utility::convert_hptdc_ungrouped(datadir,outfile,prefix,configdir.Data());
  int mwdc_synflag=Utility::check_ungrouped(datadir,outfile,configdir.Data());
  int mwdc_eventsamenum;
  if(mwdc_synflag<0){
    printf("ERROR: MWDC boards not synchronized! Contact Me as soon as possible!\n");
  }
  else{
    mwdc_eventsamenum=Utility::merge_hptdc_ungrouped(datadir,outfile,configdir.Data());
    //
    draw_multihit(datadir,outfile);
    draw_noise_merge(datadir,outfile);
    draw_mwdctot(datadir,outfile);
    draw_tof(datadir,outfile);
    draw_drift(datadir,outfile);
    //
    if(!mwdc_eventsamenum){
      printf("WARING: All of MWDC boards should have same number of events\n");
    }
  }
  
  return mwdc_synflag;
}

int process_psd(const char* datadir,const char* rawfile,const char* outfile,const char* stdpedfile)
{
  Utility::convert_psd(datadir,rawfile,datadir,outfile);
  //
  Utility::draw_channels(datadir,outfile,datadir,"mip.root");
  
  TString newpedfile=TString(datadir)+"/new_ped.root";
  Utility::draw_relp(newpedfile.Data(),stdpedfile,datadir,"mip.root");
  
  TString scifile=TString(datadir)+"/"+TString(outfile);
  Utility::draw_mip(scifile.Data(),newpedfile.Data(),datadir,"mip.root");
  
  Utility::draw_mapping(datadir,outfile,datadir);
  
  Utility::fit_dy58(scifile.Data(),newpedfile.Data(),datadir,"dy58");
  
  return 0;
}

int mwdc_validation(const char* mwdcfile)
{
  TString refile=TString("../../data/20150221/test12/mwdc12.root");
  compare_drift(mwdcfile,refile.Data());
  
  return 0;
}

int system_validation(const char* datadir,const char* mwdc_file,const char* psd_file)
{
  TString mwdcfullname=TString(datadir)+"/"+TString(mwdc_file);
  TString psdfullname=TString(datadir)+"/"+TString(psd_file);
  TString pedfullname=TString(datadir)+"/new_ped.root";
  draw_trajectory(mwdcfullname.Data(),psdfullname.Data(),pedfullname.Data());
  
  return 0;
}