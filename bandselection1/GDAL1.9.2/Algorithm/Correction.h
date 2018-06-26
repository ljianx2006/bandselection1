// Correction.h: interface for the CCorrection class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CORRECTION_H__91BCA8E3_C477_42BD_BA81_292AD893D929__INCLUDED_)
#define AFX_CORRECTION_H__91BCA8E3_C477_42BD_BA81_292AD893D929__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include<math.h>
#include "..\\include\\gdal.h"
#include "..\\include\\gdal_priv.h"
#include "..\\include\\ogr_srs_api.h"
#include "..\\include\\cpl_string.h"
#include "..\\include\\cpl_conv.h"
//
//	#if !defined(GDAL_LIB)
//	#define GDAL_LIB
//#pragma comment (lib,"..\\lib\\gdal_i-vc8.lib")
//#pragma comment (lib,"..\\lib\\gdal_id-vc8.lib")
//	#endif

class CCorrection  
{
public:
	bool ConsineMethods(CString imgFile,CString DEMFile,CString resultFile,double  SolerAzimuthAngle,double SolarZenithAngle,double K);
	CCorrection();
	virtual ~CCorrection();

};

#endif // !defined(AFX_CORRECTION_H__91BCA8E3_C477_42BD_BA81_292AD893D929__INCLUDED_)
