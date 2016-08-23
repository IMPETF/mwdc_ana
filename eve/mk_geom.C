void mk_geom(){
	TGeoManager *geom = new TGeoManager("mwdc", 
      "MWDC");

	//--- define some materials
   TGeoMaterial *matVacuum = new TGeoMaterial("Vacuum", 0,0,0);
   TGeoMedium *Vacuum = new TGeoMedium("Vacuum",1, matVacuum);

   //--- make the top container volume
   TGeoVolume *top = geom->MakeBox("TOP", Vacuum, 150., 150., 150.);
   geom->SetTopVolume(top);

   // basic paras; unit:cm
   Double_t plane_x=42,plane_y=42,plane_z=0.5;
   Double_t wire_radius=0.1,wire_length=42;
   Double_t chamber_width=0.525;
   // Down MWDC: x y , volume division
   TGeoVolume* x_wire=geom->MakeTube("x_wire",Vacuum,0,wire_radius,wire_length);
   x_wire->SetLineColor(kRed);
   // x_wire->SetLineWidth(4);
   // x_wire->SetLineStyle(kDotted);
   TGeoVolume* x_plane=geom->MakeBox("x_plane",Vacuum,plane_x,plane_y,plane_z);
   TGeoVolume* x_cell=x_plane->Divide("x_cell",1,80,-plane_x,0);
   x_cell->SetLineColor(kBlue);
   TGeoRotation* x_wire_rot=new TGeoRotation();
   x_wire_rot->RotateX(90);
   // x_cell->AddNode(x_wire,0,x_wire_rot);
   top->AddNode(x_plane,0,new TGeoTranslation(-1.5,0,-17.9));

   TGeoVolume* y_wire=geom->MakeTube("y_wire",Vacuum,0,wire_radius,wire_length);
   y_wire->SetLineColor(kBlue);
   TGeoVolume* y_plane=geom->MakeBox("y_plane",Vacuum,plane_x,plane_y,plane_z);
   TGeoVolume* y_cell=y_plane->Divide("y_cell",2,80,-plane_y,0);
   TGeoRotation* y_wire_rot=new TGeoRotation();
   y_wire_rot->RotateY(90);
   // y_cell->AddNode(y_wire,0,y_wire_rot);
   top->AddNode(y_plane,0,new TGeoTranslation(-1.5,0,-16.4));

   // Down MWDC u; composite solid; flat structure
   TGeoBBox* envelope_box=new TGeoBBox("envelope_box",plane_x,plane_y,1);
   // TGeoTranslation* tr_envelope_box=new TGeoTranslation("tr_envelope_box",-1.5,0,0);
   // tr_envelope_box->RegisterYourself();

   TGeoCompositeShape* u_wires[106];
   TGeoCompositeShape* u_chambers[106];
   
   TGeoBBox* u_prototype_chamber=new TGeoBBox("u_prototype_chamber",chamber_width,90,plane_z);
   TGeoTube* u_prototype_wire=new TGeoTube("u_prototype_wire",0,wire_radius,90);

   TVector3 init_point(36.80748+1.5,43.84964,0);
   TVector3 moving_direction(-TMath::Sqrt(3.0)/2.0,-0.5,0);
   Double_t wire_sparation=1.05;
   TVector3 point;

   TGeoVolume* u_plane=new TGeoVolumeAssembly("u_plane");
   for(int i=0;i<106;i++){
   		// wires
   		TGeoRotation* rot=new TGeoRotation();
   		rot->RotateY(90);
   		rot->RotateZ(-60);
   		point=init_point + wire_sparation*i*moving_direction;
   		TGeoCombiTrans* tr=new TGeoCombiTrans(Form("u_wire_tr%d",i),point.X(),point.Y(),point.Z(),rot);
   		tr->RegisterYourself();
		
		u_wires[i]=new TGeoCompositeShape(Form("u_wire%d",i),Form("u_prototype_wire:u_wire_tr%d * envelope_box",i));
   		TGeoVolume* u_wirevolume=new TGeoVolume(Form("u_wire%d",i),u_wires[i],Vacuum);
   	    u_wirevolume->SetLineColor(kGreen);
   		// chambers
   		TGeoRotation* rot2=new TGeoRotation();
   		rot2->RotateZ(30);
   		TGeoCombiTrans* tr2=new TGeoCombiTrans(Form("u_chamber_tr%d",i),point.X(),point.Y(),point.Z(),rot2);
   		tr2->RegisterYourself();

		u_chambers[i]=new TGeoCompositeShape(Form("u_chamber%d",i),Form("u_prototype_chamber:u_chamber_tr%d * envelope_box",i));   		
   		TGeoVolume* u_chambervolume= new TGeoVolume(Form("u_chamber%d",i),u_chambers[i],Vacuum);
   	    
   	    // place wire into chamber
   	    // u_chambervolume->AddNode(u_wirevolume,0);
   	    u_plane->AddNode(u_chambervolume,0);
   	    // u_chambervolume->SetAttVisibility(kFALSE);
   }	
   top->AddNode(u_plane,0,new TGeoTranslation(-1.5,0,-14.9));

   // Up MWDC
   TGeoRotation* up_rot=new TGeoRotation();
   up_rot->RotateY(180);
   up_rot->RotateZ(90);
   
   top->AddNode(x_plane,1,new TGeoCombiTrans(0,0,77.3,up_rot));
   top->AddNode(y_plane,1,new TGeoCombiTrans(0,0,75.8,up_rot));
   top->AddNode(u_plane,1,new TGeoCombiTrans(0,0,74.3,up_rot));

   //--- close the geometry
   geom->CloseGeometry();
      
   geom->SetVisLevel(6);
   geom->SetVisOption(0);
   top->Draw("ogl");
   // geom->GetVolume("x_plane")->Draw();
   // top->Raytrace();
   // geom->GetMasterVolume()->Draw("ogle");
   // geom->Edit();

   // set camera
   TGLViewer * v = (TGLViewer *)gPad->GetViewer3D();

   // TGLViewer::ECameraType camera = 	TGLViewer::kCameraOrthoXOZ;
   // v->SetCurrentCamera(camera);
   // v->CurrentCamera().SetExternalCenter(kTRUE);
   // TGLOrthoCamera& o = v->CurrentCamera();
   // o.SetEnableRotate(kTRUE);
   // Double_t zoom=0.78,dolly=1;
   // Double_t center[3]={0,0,0};
   // v->SetOrthoCamera(camera, zoom, dolly, center, -30, 0);
   
   // 
   v->SetStyle(TGLRnrCtx::kWireFrame);
   //  show axes
   // TView *view = gPad->GetView();
   // view->ShowAxis();

   // output
   // geom->Export("mwdc_cell.root");
   // x_plane->SaveAs("x_plane.C");
   // x_plane->Export("x_plane.root");
   // 
   std::cout<<"IsVisible():"<<x_plane->IsVisible()<<std::endl;
   std::cout<<" IsVisibleDaughters()"<<x_plane->IsVisibleDaughters()<<std::endl;
   std::cout<<"IsVisContainers()"<<x_plane->IsVisContainers()<<std::endl; 
}

void import()
{
	TGeoManager::Import("mwdc_cell.root");
	gGeoManager->DefaultColors();

	gGeoManager->GetVolume("x_plane")->InvisibleAll();

	gGeoManager->GetVolume("TOP")->Draw("ogl");
}
