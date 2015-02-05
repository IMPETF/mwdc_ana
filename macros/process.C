void pre_process(const char* datadir,const char* outfile,const char* prefix,const char* logfile,int bunchid_diff)
{
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