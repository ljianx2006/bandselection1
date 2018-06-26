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
	//Ӱ���ļ�
	GDALDataset* m_pDataset;//��ǰӰ��
	long m_imgWidth; //Ӱ��ĸ߶ȣ����
	long m_imgHeight;
	long m_bandNum;//���ε���Ŀ
	double m_GeoLeft;//����Χ
	double m_GeoTop;
	double m_GeoRight;
	double m_GeoBottom;
	double m_ResolutionY ;//�ֱ���
	double m_ResolutionX ;
	CString  m_pszFormat ;//Ӱ���ʽ	
	
	CString m_Projection ;//ͶӰ��Ϣ
	CString m_DataType;//��������
	double m_AdfGeoTransform[6];//ͶӰת������	
	CString FileName;//�ļ�����


public:
	//�ڴ�


	long m_XOff;//���뵽�ڴ��Ӱ����ʼλ��
	long m_YOff;
	long m_buffWidth;//����ĸ߶ȣ����
	long m_buffHeight;
	int m_CurrentBand;//��ǰ�����ݵĲ���������
	double m_maxVale;//��ǰ���ε��������ֵ
	double m_minVale;//��ǰ���ε���С����ֵ

	long m_buffSize;

	long m_xSrc;
	long m_ySrc;
	long m_srcWidth;
	long m_srcHeight;

public:
	//���ƴ���
	long m_WndWidth;//��������ĸ߶ȣ����
	long m_WnDHeight;
	int m_WndXOff;//���Ƶ���ʼλ��
	int m_WndYOff;
	HDC m_hdc;//��ͼ���
	int m_red,m_blue ,m_green;//����ʱ���RGBͨ��
	CDC m_DC;

public :
	double m_filter33[9];//3*3���˲���

public:
	void AdjustHSLDraw(CDC *pDC, float Hue, float Saturation, float Luminosity);//ͬʱ����HSL����
	void AdjustLuminosityDraw(CDC *pDC, float Percentage);//��������Ȼ���
	void AdjustSaturationDraw(CDC *pDC, float Percentage);//�������ͶȻ���
	void AdjustContrastDraw(CDC *pDC, float Percentage);//�����ԱȶȻ���
	void AdjustBrightnessDraw(CDC *pDC, float Percentage);//�������Ȼ���
	void AdjustHueDraw(CDC *pDC, float Percentage);//����ɫ�Ȼ���
	void NegateDraw(CDC *pDC);//��ɫ����

	void MidFilterDraw(CDC* pDC);//��ֵ�˲�����
	void RobertsFilterDraw(CDC* pDC);//�޲����˲�����
	void PrewittFilterDraw(CDC *pDC);//����Τ���˲�����
	void SobelFilterDraw(CDC *pDC);//�������˲�����
	void AvgFilterDraw(CDC *pDC);//��ֵ�˲�����
	void DiagonallyDetectDraw(CDC* pDC);//�Խ��߼���˲�����
	void HorizontallyDetectDraw(CDC* pDC);//ˮƽ����˲�����
	void VerticallyDetectDraw(CDC *pDC);//��ֱ����˲�����
	void LaplaceDraw(CDC* pDC);//������˹����˲�����
	void Filter33Draw(CDC *pDC);//3��3���˲�����ƣ�����m_filter33[9]
	bool IsGray();//�Ƿ��ǻҶ�ͼ
	void HistequealDraw(CDC *pDC);//ֱ��ͼ�������
	void GrayDraw(CDC *pDC, long xOff, long yOff, long width, long height,long GrayBand,CString colorTable);//�ٲ�ɫ�ĻҶ�ͼ
	bool split(CString table, CString splt, CString **reslut, long *cont);
	void GrayDraw(CDC *pDC, long xOff, long yOff, long width, long height, long xSrc, long ySrc, long srcWidth, long srcHeight,long GrayBand,CString  RangeColorTable);//�ٲ�ɫ�ĻҶ�ͼ
	void GrayDraw(CDC *pDC, long xOff, long yOff, long width, long height,long GrayBand);//�Ҷ�ͼ
	void GrayDraw(CDC *pDC, long xOff, long yOff, long width, long height, long xSrc, long ySrc, long srcWidth, long srcHeight,long GrayBand);//�Ҷ�ͼ

	bool SetHDC(HDC hdc);
	void Draw(long xOff, long yOff, long width, long height,  long rband, long gband, long bband);
	void Draw(long xOff, long yOff, long width, long height, long xSrc, long ySrc, long srcWidth, long srcHeight, long rband, long gband, long bband);
	bool DetachHdc();


	void Draw(CDC *pDC, long xOff, long yOff, long width, long height,  long rband, long gband, long bband);
	void Draw(HDC m_hDC, long xOff, long yOff, long width, long height,  long rband, long gband, long bband);
	void Draw(HDC m_hDC,long x,long y,long width,long height,long xSrc,long ySrc,long srcWidth,long srcHeight,long rband,long gband,long bband);
	void Draw(CDC* pDC,long xOff,long yOff,long width,long height,long xSrc,long ySrc,long srcWidth,long srcHeight,long rband,long gband,long bband);
	//����ת�����ļ������ڣ�������γ�ȣ�
	bool GeoToWnd(double GeoX,double GeoY, long & WndX, long & WndY);//�������굽��������
	bool WndToGeo(long WndX, long WndY, double & GeoX,double & GeoY);//�������굽��������
	bool FileToWnd(long Xpixel,long Yline,long & WndX, long& WndY);//�ļ����굽��������
	bool WndToFile(long WndX,long WndY,long & Xpixel,long& Yline);//�������굽�ļ�����
	bool FileToGeo(long Xpixel, long Yline, double & geoX, double & geoY);//�ļ����굽��������
	bool GeoToFile(double geoX, double geoY, long&  Xpixel, long & Yline);//�������굽�ļ�����
	bool WndToLatLong(long x, long y, CString & Lat ,CString & Long);//�������굽��γ��
	bool GeoToLatLong(double GeoX,double Geoy ,CString & Lat,CString & Long);//�������굽��γ��
	
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
	void prbub(int n, double p[]);  //ð������
	void midFilter(long width, long height, float *InBuff, float *OutBuff);
	void RobertsFilter(long width, long height, float *InBuff, float *OutBuff);
	void PrewittFilter(long width, long height, float *InBuff, float *OutBuff);
	void SobelFilter(long width, long height, float *InBuff, float *OutBuff);
	void filter33(long width ,long height, float *InBuff, float *OutBuff);
	void WaterDetectFunction(long width, long height, float *InBuff, float *OutBuff);
	void Histequal(int iWidth, int iHeight, byte *source, byte *result);
};

#endif // !defined(AFX_IMAGE_H__FC9B05CF_4475_4B10_BC04_10F154EDFB4B__INCLUDED_)
