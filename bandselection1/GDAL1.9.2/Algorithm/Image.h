// Image.h: interface for the CImage class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_IMAGE_H__FC9B05CF_4475_4B10_BC04_10F154EDFB4B__INCLUDED_)
#define AFX_IMAGE_H__FC9B05CF_4475_4B10_BC04_10F154EDFB4B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "..\\include\\gdal.h"
#include "..\\include\\gdal_priv.h"
#include "..\\include\\ogr_srs_api.h"
#include "..\\include\\cpl_string.h"
#include "..\\include\\cpl_conv.h"


class CImage  
{
public:
	//影像文件
	GDALDataset* m_pDataset;//当前影像
	long m_imgWidth; //影响的高度，宽度
	long m_imgHeight;
	long m_bandNum;//波段的数目
	double m_GeoLeft;//地理范围
	double m_GeoTop;
	double m_GeoRight;
	double m_GeoBottom;
	double m_ResolutionY ;//分辨率
	double m_ResolutionX ;
	CString  m_pszFormat ;//影像格式	
	
	CString m_Projection ;//投影信息
	CString m_DataType;//数据类型
	double m_AdfGeoTransform[6];//投影转换参数	
	CString FileName;//文件名字


public:
	//内存


	long m_XOff;//读入到内存的影像起始位置
	long m_YOff;
	long m_buffWidth;//缓存的高度，宽度
	long m_buffHeight;
	int m_CurrentBand;//当前所操纵的波段索引号
	double m_maxVale;//当前波段的最大象素值
	double m_minVale;//当前波段的最小象素值

	long m_buffSize;

	long m_xSrc;
	long m_ySrc;
	long m_srcWidth;
	long m_srcHeight;

public:
	//绘制窗口
	long m_WndWidth;//绘制区域的高度，宽度
	long m_WnDHeight;
	int m_WndXOff;//绘制的起始位置
	int m_WndYOff;
	HDC m_hdc;//绘图句柄
	int m_red,m_blue ,m_green;//绘制时候的RGB通道
	CDC m_DC;

public :
	double m_filter33[9];//3*3的滤波器

public:
	void AdjustHSLDraw(CDC *pDC, float Hue, float Saturation, float Luminosity);//同时调整HSL绘制
	void AdjustLuminosityDraw(CDC *pDC, float Percentage);//调整发光度绘制
	void AdjustSaturationDraw(CDC *pDC, float Percentage);//调整饱和度绘制
	void AdjustContrastDraw(CDC *pDC, float Percentage);//调整对比度绘制
	void AdjustBrightnessDraw(CDC *pDC, float Percentage);//调整亮度绘制
	void AdjustHueDraw(CDC *pDC, float Percentage);//调整色度绘制
	void NegateDraw(CDC *pDC);//反色绘制

	void MidFilterDraw(CDC* pDC);//中值滤波绘制
	void RobertsFilterDraw(CDC* pDC);//罗伯特滤波绘制
	void PrewittFilterDraw(CDC *pDC);//博锐韦特滤波绘制
	void SobelFilterDraw(CDC *pDC);//索博尔滤波绘制
	void AvgFilterDraw(CDC *pDC);//均值滤波绘制
	void DiagonallyDetectDraw(CDC* pDC);//对角线监测滤波绘制
	void HorizontallyDetectDraw(CDC* pDC);//水平监测滤波绘制
	void VerticallyDetectDraw(CDC *pDC);//垂直监测滤波绘制
	void LaplaceDraw(CDC* pDC);//拉普拉斯监测滤波绘制
	void Filter33Draw(CDC *pDC);//3×3阶滤波监绘制，根据m_filter33[9]
	bool IsGray();//是否是灰度图
	void HistequealDraw(CDC *pDC);//直方图均衡绘制
	void GrayDraw(CDC *pDC, long xOff, long yOff, long width, long height,long GrayBand,CString colorTable);//假彩色的灰度图
	bool split(CString table, CString splt, CString **reslut, long *cont);
	void GrayDraw(CDC *pDC, long xOff, long yOff, long width, long height, long xSrc, long ySrc, long srcWidth, long srcHeight,long GrayBand,CString  RangeColorTable);//假彩色的灰度图
	void GrayDraw(CDC *pDC, long xOff, long yOff, long width, long height,long GrayBand);//灰度图
	void GrayDraw(CDC *pDC, long xOff, long yOff, long width, long height, long xSrc, long ySrc, long srcWidth, long srcHeight,long GrayBand);//灰度图

	bool SetHDC(HDC hdc);
	void Draw(long xOff, long yOff, long width, long height,  long rband, long gband, long bband);
	void Draw(long xOff, long yOff, long width, long height, long xSrc, long ySrc, long srcWidth, long srcHeight, long rband, long gband, long bband);
	bool DetachHdc();


	void Draw(CDC *pDC, long xOff, long yOff, long width, long height,  long rband, long gband, long bband);
	void Draw(HDC m_hDC, long xOff, long yOff, long width, long height,  long rband, long gband, long bband);
	void Draw(HDC m_hDC,long x,long y,long width,long height,long xSrc,long ySrc,long srcWidth,long srcHeight,long rband,long gband,long bband);
	void Draw(CDC* pDC,long xOff,long yOff,long width,long height,long xSrc,long ySrc,long srcWidth,long srcHeight,long rband,long gband,long bband);
	//坐标转换（文件，窗口，地理，经纬度）
	bool GeoToWnd(double GeoX,double GeoY, long & WndX, long & WndY);//地理坐标到窗口坐标
	bool WndToGeo(long WndX, long WndY, double & GeoX,double & GeoY);//窗口坐标到地理坐标
	bool FileToWnd(long Xpixel,long Yline,long & WndX, long& WndY);//文件坐标到窗口坐标
	bool WndToFile(long WndX,long WndY,long & Xpixel,long& Yline);//窗口坐标到文件坐标
	bool FileToGeo(long Xpixel, long Yline, double & geoX, double & geoY);//文件坐标到地理坐标
	bool GeoToFile(double geoX, double geoY, long&  Xpixel, long & Yline);//地理坐标到文件坐标
	bool WndToLatLong(long x, long y, CString & Lat ,CString & Long);//窗口坐标到经纬度
	bool GeoToLatLong(double GeoX,double Geoy ,CString & Lat,CString & Long);//地理坐标到经纬度
	
	void Close();
	bool IsOpen();
	//bool Open(CString pszFileName);
	bool Open(const char* pszFileName);
	CImage();
	virtual ~CImage();

private:
	void HLStoRGB(float H, float L, float S, float & r,float & g,float & b);
	double HuetoRGB(double m1, double m2, double h);
	void RGBtoHSL(float r,float g ,float b, float & H,float & S,float & L);
	void prbub(int n, double p[]);  //冒泡排序
	void midFilter(long width, long height, float *InBuff, float *OutBuff);
	void RobertsFilter(long width, long height, float *InBuff, float *OutBuff);
	void PrewittFilter(long width, long height, float *InBuff, float *OutBuff);
	void SobelFilter(long width, long height, float *InBuff, float *OutBuff);
	void filter33(long width ,long height, float *InBuff, float *OutBuff);
	void WaterDetectFunction(long width, long height, float *InBuff, float *OutBuff);
	void Histequal(int iWidth, int iHeight, byte *source, byte *result);
};

#endif // !defined(AFX_IMAGE_H__FC9B05CF_4475_4B10_BC04_10F154EDFB4B__INCLUDED_)
