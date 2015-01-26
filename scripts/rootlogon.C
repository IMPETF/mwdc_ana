{
printf("Load MWDC_ana library\n");
gSystem->AddIncludePath("-I${kernel_SOURCE_DIR}");
gSystem->Load("libmwdc.so");
}