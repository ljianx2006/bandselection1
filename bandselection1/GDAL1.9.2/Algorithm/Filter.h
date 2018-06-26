// currentFilter.h: interface for the Filter class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CURRENTFILTER_H__B1919E5F_43D9_47AD_93BB_F58107A212C5__INCLUDED_)
#define AFX_CURRENTFILTER_H__B1919E5F_43D9_47AD_93BB_F58107A212C5__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include "..\\include\\gdal.h"
#include "..\\include\\gdal_priv.h"
	
//	#if !defined(GDAL_LIB)
//	#define GDAL_LIB
//#pragma comment (lib,"..\\lib\\gdal_i-vc8.lib")
//#pragma comment (lib,"..\\lib\\gdal_id-vc8.lib")
//	#endif
class Filter  
{
public:
	void Histequal(BSTR sourceFile, BSTR targetFile);
	void ratioFilter(BSTR sourceFile, BSTR targetFile);//比值动态滤波
	void percentFilter(BSTR sourceFile, BSTR targetFile);//百分比动态滤波
	void diagonallyDetect(BSTR sourceFile, BSTR targetFile);//监测对角线边界
	void horizontallyDetect(BSTR sourceFile, BSTR targetFile);//监测水平边界
	void verticallyDetect(BSTR sourceFile, BSTR targetFile);//监测垂直边界
	void LaplaceFilter(BSTR sourceFile, BSTR targetFile);//Laplace滤波
	void SobelFilter(BSTR sourceFile, BSTR targetFile);//Sobel滤波
	void PrewittFilter(BSTR sourceFile, BSTR targetFile);//Prewitt滤波
	void robertsFilter(BSTR sourceFile, BSTR targetFile);//roberts滤波
	void midFilter(BSTR sourceFile, BSTR targetFile);//中值动态滤波
	void avgFilter(BSTR sourceFile, BSTR targetFile);//均值滤波
	Filter();
	virtual ~Filter();	
	///////////////////////////////////////
	void Smooth(BSTR sourceFile, BSTR targetFile,long winsize, long threshold);
	
private:
	long Replace(long *bArray, long classes, long iFilterLen, long min_freq);
	void Histequal(int iWidth,int iHeight,byte *source,byte *result);
	void filter33(BSTR sourceFile, BSTR targetFile);//普通的滤波
	void SetFilter33(double* filter33);//设置滤波模板
	void prbub(int n, double p[]);  //冒泡排序
	double m_filter33[9];

};

#endif // !defined(AFX_CURRENTFILTER_H__B1919E5F_43D9_47AD_93BB_F58107A212C5__INCLUDED_)
