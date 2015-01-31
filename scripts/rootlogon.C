{
printf("Load MWDC_ana library\n");
gSystem->AddIncludePath("-I${CMAKE_INSTALL_PREFIX}/include");
gSystem->Load("${CMAKE_INSTALL_PREFIX}/lib/libmwdc.so");
}