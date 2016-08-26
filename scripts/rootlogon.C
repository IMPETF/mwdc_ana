{

gSystem->AddIncludePath("-I${CMAKE_INSTALL_PREFIX}/include");
gSystem->Load("${CMAKE_INSTALL_PREFIX}/lib/libmwdc_kernel.so");
gSystem->Load("${CMAKE_INSTALL_PREFIX}/lib/libmwdc_eve.so");
  
gROOT->ProcessLine(".L draw_multihit.C+");
gROOT->ProcessLine(".L draw_noise.C+");
gROOT->ProcessLine(".L draw_mwdctot.C+");
gROOT->ProcessLine(".L draw_tof.C+");
gROOT->ProcessLine(".L draw_drift.C+");

gROOT->ProcessLine(".L draw_trajectory.C+");
gROOT->ProcessLine(".L compare_drift.C+");

gROOT->ProcessLine(".L process.C");

gROOT->ProcessLine(".L extract_event.C+");
gROOT->ProcessLine(".L ana_efficiency.C+");
gROOT->ProcessLine(".L ana_dy58.C+");
gROOT->ProcessLine(".L ana_mips.C+");
gROOT->ProcessLine(".L ana_position_resolution.C+");
gROOT->ProcessLine(".L ana_batch.C+");
//
printf("Load MWDC analysis library!\n");

gStyle->SetOptStat(111111);

}
