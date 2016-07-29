#ifndef _DriftInfo_
#define _DriftInfo_

#include "TNamed.h"
#include <map>
#include "TF1.h"

class DriftInfo : public TNamed
{
public:
	DriftInfo() : TNamed("init_edge_fitting_result","init_edge_fitting_result") {};
	DriftInfo(const char* name,const char* title) : TNamed(name,title){};
	~DriftInfo() {};
	
	void Insert_t0(UInt_t gid, Double_t t0);
	void Insert_T0(UInt_t gid, Double_t T0);
	void Insert_tm(UInt_t gid, Double_t tm);
	void Insert_Tm(UInt_t gid, Double_t Tm);
	void Insert_frising(UInt_t gid, TF1* frising);
	void Insert_ffalling(UInt_t gid, TF1* ffalling);

	Double_t Get_t0(UInt_t gid);
	Double_t Get_T0(UInt_t gid);
	Double_t Get_tm(UInt_t gid);
	Double_t Get_Tm(UInt_t gid);
	TF1  Get_frising(UInt_t gid);
	TF1  Get_ffalling(UInt_t gid);

private:
	std::map<UInt_t,Double_t> fCollection_t0;
	std::map<UInt_t,Double_t> fCollection_T0;
	std::map<UInt_t,Double_t> fCollection_tm;
	std::map<UInt_t,Double_t> fCollection_Tm;
	std::map<UInt_t,TF1> 	  fCollection_frising;
	std::map<UInt_t,TF1> 	  fCollection_ffalling;

ClassDef(DriftInfo,1)

};
#endif //_DriftInfo_