{

gSystem->AddIncludePath("-I/home/ufan/workspace/mwdc/formal_test/pre/install/include");
gSystem->Load("/home/ufan/workspace/mwdc/formal_test/pre/install/lib/libmwdc.so");

gROOT->ProcessLine(".L draw_multihit.C+");
gROOT->ProcessLine(".L draw_noise.C+");
gROOT->ProcessLine(".L draw_mwdctot.C+");
gROOT->ProcessLine(".L draw_tof.C+");
gROOT->ProcessLine(".L draw_drift.C+");

gROOT->ProcessLine(".L draw_trajectory.C+");
gROOT->ProcessLine(".L compare_drift.C+");

gROOT->ProcessLine(".L process.C");
//
printf("Load MWDC analysis library!\n");
}
