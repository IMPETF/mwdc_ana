{
	TFile *fin=new TFile("x_residual.root");
	TH2F* h2d_residual;
	fin->GetObject("h2d_Down_X_residual",h2d_residual);
	if(!h2d_residual)
		exit(1);

	// 
	h2d_residual->RebinY(8);
	THStack hstack(h2d_residual,"y");

	// hstack.Draw("stack");

	TList* hists=hstack.GetHists();
	
	TIter next(hists);
	TObject* hist;

	// TCanvas* c1=new TCanvas("c1","c1");
	// c1->Print("debug_fitslice.pdf[");
	// while(hist=next()){
	// 	hist->Draw();
	// 	c1->Print("debug_fitslice.pdf");
	// }
	// c1->Print("debug_fitslice.pdf]");



	// 
	h2d_residual->FitSlicesY(0,1,400,100,"QNRSG2");
	TH1D* h2d_mean=(TH1D*)gDirectory->Get("h2d_Down_X_residual_1");
	TH1D* h2d_sigma=(TH1D*)gDirectory->Get("h2d_Down_X_residual_2");
	TH1D* h2d_chi2=(TH1D*)gDirectory->Get("h2d_Down_X_residual_chi2");

	TProfile* hprofile=h2d_residual->ProfileX();
	std::cout<<"Number of bins:" <<hprofile->GetNbinsX()<<std::endl;
	std::cout<<"Number of bins:" <<hprofile->GetBinWidth(1)<<std::endl;	
	std::cout<<"Center of first bin:"<< hprofile->GetBinCenter(1)<<std::endl;
	std::cout<<"Content of first bin:"<< hprofile->GetBinContent(1)<<std::endl;

	TCanvas* c2=new TCanvas("c2","c2");
	c2->Divide(2,2);
	c2->cd(1);
	h2d_residual->Draw("colz");
	c2->cd(2);
	h2d_mean->Draw();
	hprofile->SetLineColor(kRed);
	hprofile->Draw("same");
	c2->cd(3);
	h2d_sigma->Draw();
	c2->cd(4);
	h2d_chi2->Draw();



	// delete fin;
}