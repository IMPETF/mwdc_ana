#include "FitGUI.h"
#include <TRint.h>
#include <TApplication.h>
#include <TGApplication.h>

// invoking: fit_debug <root_file> [geom_file] [init_eventnum]
int main(int argc, char *argv[])
{
  TString geom_filename;
  TString root_filename;
  Int_t   init_evtnum;
  root_filename = argv[1];
  geom_filename = "../geom/mwdc_extract_chamber.root";
  init_evtnum = 0;

  // Use TApplication if you don't need prompt.
  TRint  *app = new TRint("App", &argc, argv);
  // TGApplication  *app = new TGApplication("App", &argc, argv);
  
  // See arguments to Create() and constructor -- you can choose not to show the window
  // or some GUI parts.
  EventDisplay::Create();
  EventHandler* handler=0;

  if(gEvtDisplay->ImportGeometry(geom_filename.Data())){

    handler = new FitGUI(root_filename.Data());
    gEvtDisplay->SetEventHandler(handler); 
    gEvtDisplay->Initialize();

    handler->GotoEvent(init_evtnum);

    app->Run(kTRUE);
    // // Pass kFALSE if you want application to terminate by itself.
    // Then you just need "return 0;" below (to avoid compiler warnings).
}
  // Create custom GUI, if needed.
  
  app->Terminate(0);
  if (handler) {
    delete handler;
  }
 return 0;
}
