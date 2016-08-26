#ifdef __CINT__

// global.h
#pragma link C++ all typedef;
#pragma link C++ namespace Encoding;
#pragma link C++ enum EBoardType;
#pragma link C++ enum ELocation;
#pragma link C++ enum EDirection;
#pragma link C++ class std::vector<int>+;
#pragma link C++ class std::map<UInt_t,std::vector<int> >+;
#pragma link C++ global g_range_highprecision;
#pragma link C++ global g_range_precision;

// utility.h
#pragma link C++ namespace Utility;

// BoardInfo.h
#pragma link C++ class BoardInfo+;
#pragma link C++ class MWDCBoard+;
#pragma link C++ class TOFBoard+;

// CrateInfo.h
#pragma link C++ class CrateInfo+;

// DriftInfo.h
#pragma link C++ class DriftInfo+;

// GeometryInfo.h
#pragma link C++ class GeometryInfo;

#endif