#include "DriftInfo.h"

void DriftInfo::Insert_t0(UInt_t gid, Double_t t0)
{
	fCollection_t0[gid]=t0;
}

void DriftInfo::Insert_T0(UInt_t gid, Double_t T0)
{
	fCollection_T0[gid]=T0;
}

void DriftInfo::Insert_tm(UInt_t gid, Double_t tm)
{
	fCollection_tm[gid]=tm;
}

void DriftInfo::Insert_Tm(UInt_t gid, Double_t Tm)
{
	fCollection_Tm[gid]=Tm;
}

void DriftInfo::Insert_frising(UInt_t gid, TF1* frising)
{
	fCollection_frising[gid]=(*frising);

	Insert_t0(gid,frising->GetParameter("t0"));
	Insert_T0(gid,frising->GetParameter("T0"));
}

void DriftInfo::Insert_ffalling(UInt_t gid, TF1* ffalling)
{
	fCollection_ffalling[gid]=(*ffalling);

	Insert_tm(gid,ffalling->GetParameter("tm"));
	Insert_Tm(gid,ffalling->GetParameter("Tm"));
}

Double_t DriftInfo::Get_t0(UInt_t gid)
{
	return fCollection_t0[gid];
}

Double_t DriftInfo::Get_T0(UInt_t gid)
{
	return fCollection_T0[gid];
}

Double_t DriftInfo::Get_tm(UInt_t gid)
{
	return fCollection_tm[gid];
}

Double_t DriftInfo::Get_Tm(UInt_t gid)
{
	return fCollection_Tm[gid];
}

TF1 DriftInfo::Get_frising(UInt_t gid)
{
	return fCollection_frising[gid];
}

TF1 DriftInfo::Get_ffalling(UInt_t gid)
{
	return fCollection_ffalling[gid];
}