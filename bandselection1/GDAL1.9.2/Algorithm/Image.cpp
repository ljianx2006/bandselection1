// Image.cpp: implementation of the CImage class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Image.h"
#include<math.h>
#include<vector>//注意这个一定要放在下面这段段话的前面，不然头文件包含出错
using namespace std;
//
#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif
//
#using "System.Windows.Forms.dll"
#using "System.Drawing.dll"
using namespace System::Windows::Forms;
using namespace System::Drawing;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CImage::CImage()
{
	GDALAllRegister();
	
	 m_pDataset=NULL;//当前影像
	 m_imgWidth=0; //影响的高度，宽度
	 m_imgHeight=0;
	 m_bandNum=0;//波段的数目
	 m_GeoLeft=0.0;//地理范围
	 m_GeoTop=0.0;
	 m_GeoRight=0.0;
	 m_GeoBottom=0.0;
	 m_ResolutionY=0.0 ;//分辨率
	 m_ResolutionX=0.0;
	 m_pszFormat="" ;//影像格式
	
	 m_Projection="" ;//投影信息
	 m_DataType="";//数据类型

	FileName="";//文件名字
	int i=0;
	for (i=0;i<6;i++)
	{
		m_AdfGeoTransform[i]=0.0;
	}

	

	//内存
	 m_XOff=0;//读入到内存的影像起始位置
	 m_YOff=0;
	 m_buffWidth=0;//缓存的高度，宽度
	 m_buffHeight=0;
	 m_CurrentBand=0;//当前所操纵的波段索引号


	//绘制窗口
	 m_WndWidth=0;//绘制区域的高度，宽度
	 m_WnDHeight=0;
	m_WndXOff=0;//绘制的起始位置
	m_WndYOff=0;
	m_hdc=0;//绘图句柄
	m_red=0;m_blue =0;m_green=0;//绘制时候的RGB通道

}

CImage::~CImage()
{
	Close();
}
inline const char * CString2ConstCharP(CString s)
{
	const char* c=" ";
	strcpy((char*)s.GetBuffer(s.GetLength()+1),c);
	s.ReleaseBuffer();
	return c;
}

bool CImage::Open(const char* pszFileName)
{
	if (IsOpen())
		Close();
	
	m_pDataset = (GDALDataset *) GDALOpen(pszFileName,GA_ReadOnly);
	if (m_pDataset)
	{
		m_imgWidth=m_pDataset->GetRasterXSize(); //影响的高度，宽度
		m_imgHeight=m_pDataset->GetRasterYSize();
		m_bandNum=m_pDataset->GetRasterCount();//波段的数目

		if( m_pDataset->GetProjectionRef()  != NULL )
		{
			m_Projection=m_pDataset->GetProjectionRef(); 
		}
	
		if( m_pDataset->GetGeoTransform( m_AdfGeoTransform ) == CE_None )
		{		
			m_GeoLeft=m_AdfGeoTransform[0];
			m_GeoTop=m_AdfGeoTransform[3] ;
			m_ResolutionX=m_AdfGeoTransform[1];
			m_ResolutionY=m_AdfGeoTransform[5] ;
		}
		
	  double geox=0.0;
	  double geoy=0.0;
	  if(FileToGeo(m_imgWidth,m_imgHeight,geox,geoy))
	  {
		  m_GeoRight=geox;
		  m_GeoBottom=geoy;
		  
	  }
		
	
		m_pszFormat=m_pDataset->GetDriver()->GetDescription();
		
		GDALRasterBand  *poBand=NULL;
		if(m_bandNum>0)
		{
			m_CurrentBand=1;
			poBand = m_pDataset->GetRasterBand( 1 );	
			if(poBand)
			{
				m_DataType=GDALGetDataTypeName(poBand->GetRasterDataType());//数据类型
			}	

		}
		poBand=NULL;
		FileName=pszFileName;//文件名字


		return true;
	}

	return false;

}

bool CImage::IsOpen()
{
	return (m_pDataset != NULL);
}

void CImage::Close()
{
	if (!IsOpen())
		return;
	if (m_pDataset) {
		delete m_pDataset;
		m_pDataset = NULL;
		
	}

	m_imgWidth=0; //影响的高度，宽度
	m_imgHeight=0;
	m_bandNum=0;//波段的数目
	m_GeoLeft=0.0;//地理范围
	m_GeoTop=0.0;
	m_GeoRight=0.0;
	m_GeoBottom=0.0;
	m_ResolutionY=0.0 ;//分辨率
	m_ResolutionX=0.0;
	m_pszFormat="" ;//影像格式	
	
	m_Projection="" ;//投影信息
	m_DataType="";//数据类型
	
	FileName="";//文件名字

	int i=0;
	for (i=0;i<6;i++)
	{
		m_AdfGeoTransform[i]=0.0;
	}
	
	//内存
	m_XOff=0;//读入到内存的影像起始位置
	m_YOff=0;
	m_buffWidth=0;//缓存的高度，宽度
	m_buffHeight=0;
	m_CurrentBand=0;//当前所操纵的波段索引号
	
	//绘制窗口
	m_WndWidth=0;//绘制区域的高度，宽度
	m_WnDHeight=0;
	m_WndXOff=0;//绘制的起始位置
	m_WndYOff=0;
	m_hdc=0;//绘图句柄
	m_red=0;m_blue =0;m_green=0;//绘制时候的RGB通道

	if(m_DC.m_hDC>0)
		m_DC.Detach();	
}


bool CImage::GeoToFile(double geoX, double geoY, long &Xpixel, long &Yline)
{
	if(!m_pDataset) return false;
	double x,y;
	if( m_pDataset->GetGeoTransform( m_AdfGeoTransform ) == CE_None )
	{ 
		y=((m_AdfGeoTransform[4]*geoX-m_AdfGeoTransform[1]*geoY)-(m_AdfGeoTransform[4]*m_AdfGeoTransform[0]-m_AdfGeoTransform[1]*m_AdfGeoTransform[3]))/(m_AdfGeoTransform[4]*m_AdfGeoTransform[2]-m_AdfGeoTransform[1]*m_AdfGeoTransform[5]);
		Yline= y-long(y)>0.5?long(y)+1:long(y);
		x=((m_AdfGeoTransform[5]*geoX-m_AdfGeoTransform[2]*geoY)-(m_AdfGeoTransform[0]*m_AdfGeoTransform[5]-m_AdfGeoTransform[3]*m_AdfGeoTransform[2]))/(m_AdfGeoTransform[1]*m_AdfGeoTransform[5]-m_AdfGeoTransform[4]*m_AdfGeoTransform[2]);
		Xpixel=x-long(x)>0.5?long(x)+1:long(x);
		return true;
	}else
	{
	   return false;
	}
		   
}

bool CImage::FileToGeo(long Xpixel, long Yline, double &geoX, double &geoY)
{
	if(!m_pDataset) return false;
	if( m_pDataset->GetGeoTransform( m_AdfGeoTransform ) == CE_None )
	{ 
		geoX = m_AdfGeoTransform[0] + Xpixel*m_AdfGeoTransform[1] + Yline*m_AdfGeoTransform[2];
		geoY = m_AdfGeoTransform[3] + Xpixel*m_AdfGeoTransform[4] + Yline*m_AdfGeoTransform[5];
		return true;
	}else
	{
		return false;
	}
}

bool CImage::WndToFile(long WndX, long WndY, long & Xpixel, long& Yline)
{
	if(m_WndWidth==0 || m_WnDHeight==0 || m_srcWidth==0 || m_srcHeight==0)
		return false;

	double x=double(m_srcWidth)/double(m_WndWidth)*(WndX-m_WndXOff)+m_xSrc;
	Xpixel=x-long(x)>0.5?long(x)+1:long(x);
	double y=double(m_srcHeight)/double(m_WnDHeight)*(WndY-m_WndYOff)+m_ySrc;
	Yline=y-long(y)>0.5?long(y)+1:long(y);
	if(Xpixel<m_xSrc || Xpixel>m_srcWidth || Yline<m_ySrc ||Yline>m_srcHeight )
		return false;
	return true;

}

bool CImage::FileToWnd(long Xpixel, long Yline, long &WndX, long& WndY)
{
	if(m_WndWidth==0 || m_WnDHeight==0 || m_srcWidth==0 || m_srcWidth==0)
		return false;
	double x=double(m_WndWidth)/double(m_srcWidth)*(Xpixel-m_xSrc)+m_WndXOff;
	double y=double(m_WnDHeight)/double(m_srcHeight)*(Yline-m_ySrc)+m_WndYOff;
	WndX=x-long(x)>0.5?long(x)+1:long(x);
	WndY=y-long(y)>0.5?long(y)+1:long(y);
	
	if(WndX<m_WndXOff || WndX >m_WndWidth || WndY< m_WndYOff || WndY> m_WnDHeight)
		return false;
	return true;
}

bool CImage::WndToGeo(long WndX, long WndY, double &GeoX, double &GeoY)
{
	long Xpixel=0;
	long Yline=0;
	if(WndToFile(WndX,WndY,Xpixel,Yline))
	{
		return FileToGeo(Xpixel,Yline,GeoX, GeoY);
	}

	return false;

}

bool CImage::GeoToWnd(double GeoX, double GeoY, long &WndX, long &WndY)	//地理坐标到窗口坐标
{
	long Xpixel=0;
	long Yline=0;
	if(GeoToFile(GeoX,GeoY,Xpixel,Yline))
	{
		return FileToWnd(Xpixel,Yline,WndX, WndY);
	}
	return false;

}

void CImage::Draw(CDC *pDC, long xOff, long yOff, long width, long height, long xSrc, long ySrc, long srcWidth, long srcHeight, long rband, long gband, long bband)
{
	
	if(pDC==NULL)
		return;	
	if (!m_pDataset)
		return;

	if(m_DC.m_hDC>0)
		m_DC.Detach();
	m_DC.Attach(pDC->m_hDC);
	m_hdc=pDC->m_hDC;

	if (rband <= 0 || rband>m_bandNum)
		rband = m_CurrentBand;
	if (gband <= 0 || gband>m_bandNum)
		gband = m_CurrentBand;
	if (bband <= 0 || bband>m_bandNum)
		bband = m_CurrentBand;

	m_red=rband;
	m_green=gband;
	m_blue=bband;
		
	if(xSrc<0 ||xSrc >m_imgWidth) xSrc=0;
	if(ySrc<0 ||ySrc >m_imgHeight) ySrc=0;
	if(srcWidth<1 || srcWidth > m_imgWidth) srcWidth=m_imgWidth;
	if(srcHeight<1 || srcHeight > m_imgHeight) srcHeight=m_imgHeight;	
	  m_XOff=xSrc;//读入到内存的影像起始位置
	  m_YOff=ySrc;
	  m_buffWidth=srcWidth;//缓存的高度，宽度
	  m_buffHeight=srcHeight;
	if(double(width)/ double(height)>double(srcWidth)/double(srcHeight))//保证长宽不畸变
	{
		double tmp=height*double(srcWidth)/double(srcHeight);
		width=tmp-long(tmp)>0.5?long(tmp)+1:long(tmp);	
	}
	else
	{
		double tmp=width*double(srcHeight)/double(srcWidth);
		height=tmp-long(tmp)>0.5?long(tmp)+1:long(tmp);	
	}

	m_WndWidth=width;
	m_WnDHeight=height;
	m_WndXOff=xOff;
	m_WndYOff=yOff;
	 m_xSrc=xSrc;
	 m_ySrc=ySrc;
	 m_srcWidth=srcWidth;
	 m_srcHeight= srcHeight;

	//byte* buf = new byte[srcWidth*srcHeight];
	int dataType=0;
	if(m_DataType=="Byte")dataType=0;
	if(m_DataType=="UInt16")dataType=1;
	if(m_DataType=="Int16")dataType=2;
	if(m_DataType=="UInt32")dataType=3;
	if(m_DataType=="Int32")dataType=4;
	if(m_DataType=="Float32")dataType=5;
	if(m_DataType=="Float64")dataType=6;
	if(m_DataType=="CInt16")dataType=7;
	if(m_DataType=="CInt32")dataType=8;
	if(m_DataType=="CFloat32")dataType=9;
	if(m_DataType=="CFloat64")dataType=10;
	
	GDALRasterBand  *m_pBand=NULL;
	byte* m_ByteBuff;
	float* m_FloatBuff;
	byte* buf =NULL;
	float* buffFloat=NULL;
	int x,y;
	CDC dcMem;
	dcMem.CreateCompatibleDC(pDC);
	CBitmap bitmap;
	bitmap.CreateCompatibleBitmap(pDC,width,height);
	dcMem.SelectObject(&bitmap);
	int i;
	switch(dataType)
	{
		case 0://byte就byte		
			
				m_ByteBuff = new byte[width*height*3];			
				m_buffHeight=height;
				m_buffWidth=width;				
			m_buffSize=height*width*3;
			buf = new byte[width*height];
			
			m_pBand = m_pDataset->GetRasterBand(rband);	
			if (m_pBand)
			{
				if (CE_None==m_pBand->RasterIO( GF_Read, xSrc,ySrc, srcWidth, srcHeight, buf, width,height, GDT_Byte, 0, 0 ))
				{
					for ( i = 0; i < width*height; i ++) 
						m_ByteBuff[3*i+2] = buf[i];
				}
			}	
			m_pBand = m_pDataset->GetRasterBand(gband);
			if (m_pBand)
			{	
				if (CE_None==m_pBand->RasterIO( GF_Read, xSrc,ySrc, srcWidth, srcHeight, buf, width,height, GDT_Byte, 0, 0 ))
				{
					for ( i = 0; i <width*height; i ++) 
						m_ByteBuff[3*i+1] = buf[i];
				}
			}
			
			m_pBand = m_pDataset->GetRasterBand(bband);
			if (m_pBand)
			{	
				if (CE_None==m_pBand->RasterIO( GF_Read, xSrc,ySrc, srcWidth, srcHeight, buf, width,height, GDT_Byte, 	0, 0 ))
				{
					for ( i = 0; i < width*height; i ++) 
						m_ByteBuff[3*i+0] = buf[i];
				}
			}
			for ( x = 0; x < width; x ++)
			{
				for ( y = 0; y < height; y ++) 
				{
					dcMem.SetPixel(x,y,RGB(m_ByteBuff[(y*width+x)*3+2],m_ByteBuff[(y*width+x)*3+1],m_ByteBuff[(y*width+x)*3+0]));
				}
			}
			pDC->BitBlt(xOff,yOff,width,height,&dcMem,0,0,SRCCOPY);
			delete buf;
			delete  m_ByteBuff;
			m_ByteBuff=NULL;
			buf=NULL;
			break;
		default://其他类型的都用float
			
			
				m_FloatBuff = new float[width*height*3];			
				m_buffHeight=height;
				m_buffWidth=width;
			
			m_buffSize=height*width*3;
			buffFloat = new float[width*height];
			
			m_pBand = m_pDataset->GetRasterBand(rband);	
			if (m_pBand)
			{
				if (CE_None==m_pBand->RasterIO( GF_Read, xSrc,ySrc, srcWidth, srcHeight, buffFloat, width,height, GDT_Float32, 0, 0 ))
				{
					for ( i = 0; i < width*height; i ++) 
						m_FloatBuff[3*i+2] = buffFloat[i];
				}
			}	
			m_pBand = m_pDataset->GetRasterBand(gband);
			if (m_pBand)
			{	
				if (CE_None==m_pBand->RasterIO( GF_Read, xSrc,ySrc, srcWidth, srcHeight, buffFloat, width,height, GDT_Float32, 0, 0 ))
				{
					for ( i = 0; i <width*height; i ++) 
						m_FloatBuff[3*i+1] = buffFloat[i];
				}
			}
			
			m_pBand = m_pDataset->GetRasterBand(bband);
			if (m_pBand)
			{	
				if (CE_None==m_pBand->RasterIO( GF_Read, xSrc,ySrc, srcWidth, srcHeight, buffFloat, width,height, GDT_Float32, 	0, 0 ))
				{
					for ( i = 0; i < width*height; i ++) 
						m_FloatBuff[3*i+0] = buffFloat[i];
				}
			}
			for ( x = 0; x < width; x ++)
			{
				for ( y = 0; y < height; y ++) 
				{
					dcMem.SetPixel(x,y,RGB(m_FloatBuff[(y*width+x)*3+2],m_FloatBuff[(y*width+x)*3+1],m_FloatBuff[(y*width+x)*3+0]));
				}
			}
			pDC->BitBlt(xOff,yOff,width,height,&dcMem,0,0,SRCCOPY);				
			delete buffFloat;
			delete  m_FloatBuff;
			m_FloatBuff=NULL;
			buffFloat=NULL;
			break;
	}
	

}



void CImage::Draw(HDC m_hDC, long x, long y, long width, long height, long xSrc, long ySrc, long srcWidth, long srcHeight, long rband, long gband, long bband)
{
	CDC dc;
	dc.Attach(m_hDC);
	Draw(&dc,x,y,width,height,xSrc,ySrc,srcWidth,srcHeight,rband,gband,bband);	
	dc.Detach();
}

void CImage::Draw(CDC *pDC, long x, long y, long width, long height,  long rband, long gband, long bband)
{
	long xSrc=0;
	long ySrc=0;
	long srcWidth=m_imgWidth;
	long srcHeight=m_imgHeight;
	Draw(pDC,x,y,width,height,xSrc,ySrc,srcWidth,srcHeight,rband,gband,bband);
}

void CImage::Draw(HDC m_hDC, long x, long y, long width, long height,  long rband, long gband, long bband)
{
	CDC dc;
	dc.Attach(m_hDC);
	Draw(&dc,x,y,width,height,rband,gband,bband);	
	dc.Detach();
}

bool CImage::GeoToLatLong(double GeoX, double Geoy, CString & Lat, CString & Long)
{
	if( m_Projection && m_Projection.GetLength()>0 )
	{
		OGRSpatialReferenceH hProj, hLatLong = NULL;
		OGRCoordinateTransformationH hTransform = NULL;
		
		hProj = OSRNewSpatialReference( CString2ConstCharP(m_Projection) );
		if( hProj != NULL )
			hLatLong = OSRCloneGeogCS( hProj );
		else
			return false;
	
		if( hLatLong != NULL )
		 {
			CPLPushErrorHandler( CPLQuietErrorHandler );
			hTransform = OCTNewCoordinateTransformation( hProj, hLatLong );
			CPLPopErrorHandler();
			OSRDestroySpatialReference( hLatLong );
		}
		else
		{
			return false;
		}

		if( hProj != NULL )
		OSRDestroySpatialReference( hProj );

		if( hTransform != NULL && OCTTransform(hTransform,1,&GeoX,&Geoy,NULL) )
		{
		  Lat=GDALDecToDMS( GeoX, "Long", 2 ) ;
		  Long=GDALDecToDMS( Geoy, "Lat", 2 ) ;
		}
		
		if( hTransform != NULL )
			OCTDestroyCoordinateTransformation( hTransform );
		if(Lat.GetLength()==0 ||Long.GetLength()==0 )
			return false;
		return true;
	}
	return false;

}

bool CImage::WndToLatLong(long x, long y, CString & Lat, CString & Long)
{
	double GeoX , GeoY;
	if(WndToGeo(x,y,GeoX,GeoY))
	{
		return GeoToLatLong(GeoX,GeoY,Lat,Long);
	}
	return true;
}



bool CImage::SetHDC(HDC hdc)
{
	BOOL bResult;
	if(m_DC.m_hDC>0)
		m_DC.Detach();
	bResult=m_DC.Attach(hdc);

	if(bResult)
	{
		m_hdc=m_DC.m_hDC;
		return true;
	}
	return false;
}

bool CImage::DetachHdc()
{
	HDC result;
	if(m_DC.m_hDC>0)
	{
		result=m_DC.Detach();
		if(result>0)
		{
			m_hdc=0;
			return true;
		}
		else
			return false;
	}
	return true;

}

void CImage::Draw(long xOff, long yOff, long width, long height, long xSrc, long ySrc, long srcWidth, long srcHeight, long rband, long gband, long bband)
{
	if(m_DC.m_hDC<=0)
		return;	
	Draw(&m_DC,  xOff,  yOff,  width,  height,  xSrc,  ySrc,  srcWidth,  srcHeight,  rband,  gband,  bband);

}

void CImage::Draw(long xOff, long yOff, long width, long height, long rband, long gband, long bband)
{
	long xSrc=0;
	long ySrc=0;
	long srcWidth=m_imgWidth;
	long srcHeight=m_imgHeight;
	this->Draw( xOff,  yOff,  width,  height,  xSrc,  ySrc,  srcWidth,  srcHeight,  rband,  gband,  bband);
}



void CImage::GrayDraw(CDC *pDC, long xOff, long yOff, long width, long height, long xSrc, long ySrc, long srcWidth, long srcHeight,long GrayBand)
{
	//Draw(CDC *pDC,  xOff,  yOff,  width,  height,  xSrc,  ySrc,  srcWidth,  srcHeight,  rband,  gband,  bband);

	if(pDC==NULL)
		return;	
	if(m_DC.m_hDC>0)
		m_DC.Detach();
	m_DC.Attach(pDC->m_hDC);
	m_hdc=pDC->m_hDC;
	if (!m_pDataset)
		return;
	if(GrayBand>m_bandNum || GrayBand<=0)
		GrayBand=1;
	m_CurrentBand=GrayBand;
	m_red=m_CurrentBand;
	m_green=m_CurrentBand;
	m_blue=m_CurrentBand;	
	
	if(xSrc<0 ||xSrc >m_imgWidth) xSrc=0;
	if(ySrc<0 ||ySrc >m_imgHeight) ySrc=0;
	if(srcWidth<1 || srcWidth > m_imgWidth) srcWidth=m_imgWidth;
	if(srcHeight<1 || srcHeight > m_imgHeight) srcHeight=m_imgHeight;	
	  m_XOff=xSrc;//读入到内存的影像起始位置
	  m_YOff=ySrc;
	  m_buffWidth=srcWidth;//缓存的高度，宽度
	  m_buffHeight=srcHeight;
	if(double(width)/ double(height)>double(srcWidth)/double(srcHeight))//保证长宽不畸变
	{
		double tmp=height*double(srcWidth)/double(srcHeight);
		width=tmp-long(tmp)>0.5?long(tmp)+1:long(tmp);	
	}
	else
	{
		double tmp=width*double(srcHeight)/double(srcWidth);
		height=tmp-long(tmp)>0.5?long(tmp)+1:long(tmp);	
	}

	m_WndWidth=width;
	m_WnDHeight=height;
	m_WndXOff=xOff;
	m_WndYOff=yOff;
	m_xSrc=xSrc;
	m_ySrc=ySrc;
	m_srcWidth=srcWidth;
	m_srcHeight= srcHeight;

	int dataType=0;
	if(m_DataType=="Byte")dataType=0;
	if(m_DataType=="UInt16")dataType=1;
	if(m_DataType=="Int16")dataType=2;
	if(m_DataType=="UInt32")dataType=3;
	if(m_DataType=="Int32")dataType=4;
	if(m_DataType=="Float32")dataType=5;
	if(m_DataType=="Float64")dataType=6;
	if(m_DataType=="CInt16")dataType=7;
	if(m_DataType=="CInt32")dataType=8;
	if(m_DataType=="CFloat32")dataType=9;
	if(m_DataType=="CFloat64")dataType=10;
	
	GDALRasterBand  *m_pBand=NULL;
	m_pBand = m_pDataset->GetRasterBand(m_CurrentBand);	
	if(!m_pBand)return;//没有取得数据
	m_minVale=m_pBand->GetMaximum();
	m_minVale=m_pBand->GetMinimum();
	int x,y;
	CDC dcMem;
	dcMem.CreateCompatibleDC(pDC);
	CBitmap bitmap;
	bitmap.CreateCompatibleBitmap(pDC,width,height);
	dcMem.SelectObject(&bitmap);
	
	byte* m_ByteBuff;
	float* m_FloatBuff;

	switch(dataType)
	{
	
	case 0:	
				m_ByteBuff = new byte[width*height];			
				m_buffHeight=height;
				m_buffWidth=width;
		
			if (m_pBand)
			{
				if (CE_None==m_pBand->RasterIO( GF_Read, xSrc,ySrc, srcWidth, srcHeight, m_ByteBuff, width,height, GDT_Byte, 0, 0 ))
				{				
				}
			}			
			
			for ( x = 0; x < width; x ++)
			{
				for ( y = 0; y < height; y ++) 
				{
					dcMem.SetPixel(x,y,RGB(m_ByteBuff[y*width+x],m_ByteBuff[y*width+x],m_ByteBuff[y*width+x]));
				}
			}
			pDC->BitBlt(xOff,yOff,width,height,&dcMem,0,0,SRCCOPY);	
				delete  m_ByteBuff;
				m_ByteBuff=NULL;
			break;
		default://其他类型的都用float

				m_FloatBuff = new float[width*height];			
				m_buffHeight=height;
				m_buffWidth=width;
		
			m_buffSize=height*width;

			if (m_pBand)
			{
				if (CE_None==m_pBand->RasterIO( GF_Read, xSrc,ySrc, srcWidth, srcHeight, m_FloatBuff, width,height, GDT_Float32, 0, 0 ))
				{				
				}
			}		
		
			for ( x = 0; x < width; x ++)
			{
				for ( y = 0; y < height; y ++) 
				{
					dcMem.SetPixel(x,y,RGB(m_FloatBuff[y*width+x],m_FloatBuff[y*width+x],m_FloatBuff[y*width+x]));
				}
			}
			pDC->BitBlt(xOff,yOff,width,height,&dcMem,0,0,SRCCOPY);	
			
			
			delete  m_FloatBuff;
			m_FloatBuff=NULL;
			break;
	}

}

void CImage::GrayDraw(CDC *pDC, long xOff, long yOff, long width, long height,long GrayBand)
{
	long xSrc=0;
	long ySrc=0;
	long srcWidth=m_imgWidth;
	long srcHeight=m_imgHeight;
	GrayDraw(pDC,  xOff,  yOff,  width,  height,  xSrc,  ySrc,  srcWidth,  srcHeight,GrayBand);

}



void CImage::GrayDraw(CDC *pDC, long xOff, long yOff, long width, long height, long xSrc, long ySrc, long srcWidth, long srcHeight, long GrayBand, CString RangeColorTable)
{//"a,b,color;b,c,color;c,d,color;d,f,color;f,g,color;g,h,color;"
	//一定要注意最后有个；像元值所处的范围[a,b]，color是该范围的颜色值
	if(pDC==NULL)
		return;	
	if(RangeColorTable.GetLength()==0)
		return;
	if (!m_pDataset)
		return;

	if(m_DC.m_hDC>0)
		m_DC.Detach();
	m_DC.Attach(pDC->m_hDC);
	m_hdc=pDC->m_hDC;
	if(GrayBand>m_bandNum || GrayBand<=0)
		GrayBand=1;
	m_CurrentBand=GrayBand;
	m_red=m_CurrentBand;
	m_green=m_CurrentBand;
	m_blue=m_CurrentBand;	
	
	if(xSrc<0 ||xSrc >m_imgWidth) xSrc=0;
	if(ySrc<0 ||ySrc >m_imgHeight) ySrc=0;
	if(srcWidth<1 || srcWidth > m_imgWidth) srcWidth=m_imgWidth;
	if(srcHeight<1 || srcHeight > m_imgHeight) srcHeight=m_imgHeight;	
	  m_XOff=xSrc;//读入到内存的影像起始位置
	  m_YOff=ySrc;
	  m_buffWidth=srcWidth;//缓存的高度，宽度
	  m_buffHeight=srcHeight;
	if(double(width)/ double(height)>double(srcWidth)/double(srcHeight))//保证长宽不畸变
	{
		double tmp=height*double(srcWidth)/double(srcHeight);
		width=tmp-long(tmp)>0.5?long(tmp)+1:long(tmp);	
	}
	else
	{
		double tmp=width*double(srcHeight)/double(srcWidth);
		height=tmp-long(tmp)>0.5?long(tmp)+1:long(tmp);	
	}

	m_WndWidth=width;
	m_WnDHeight=height;
	m_WndXOff=xOff;
	m_WndYOff=yOff;

	m_xSrc=xSrc;
	m_ySrc=ySrc;
	m_srcWidth=srcWidth;
	m_srcHeight= srcHeight;

	int dataType=0;
	if(m_DataType=="Byte")dataType=0;
	if(m_DataType=="UInt16")dataType=1;
	if(m_DataType=="Int16")dataType=2;
	if(m_DataType=="UInt32")dataType=3;
	if(m_DataType=="Int32")dataType=4;
	if(m_DataType=="Float32")dataType=5;
	if(m_DataType=="Float64")dataType=6;
	if(m_DataType=="CInt16")dataType=7;
	if(m_DataType=="CInt32")dataType=8;
	if(m_DataType=="CFloat32")dataType=9;
	if(m_DataType=="CFloat64")dataType=10;
	
	GDALRasterBand  *m_pBand=NULL;
	m_pBand = m_pDataset->GetRasterBand(m_CurrentBand);	
	if(!m_pBand)return;//没有取得数据
	m_minVale=m_pBand->GetMaximum();
	m_minVale=m_pBand->GetMinimum();
	
	int x,y;
	CDC dcMem;
	dcMem.CreateCompatibleDC(pDC);
	CBitmap bitmap;
	bitmap.CreateCompatibleBitmap(pDC,width,height);
	dcMem.SelectObject(&bitmap);
	
	CString * Rangecolores;
	long count;
	double *a,*b;
	long *color;
	split(RangeColorTable,(CString)";",&Rangecolores,&count);
	a=new double[count];
	b=new double[count];
	color=new long[count];
	CString *colorRange;
	long tmpcount;
	int i=0;
	for(i=0;i< count;i++)
	{
		split(Rangecolores[i]+(CString)",",(CString)",",&colorRange,&tmpcount);


		a[i]=atof(CString2ConstCharP(colorRange[0]));
		//a[i]=atof(colorRange[0]);

		b[i]=atof(CString2ConstCharP(colorRange[1]));
		color[i]=atol(CString2ConstCharP(colorRange[2]));
			
	/*	b[i]=(LPCTSTR)atof(colorRange[1]);
		color[i]=atol(colorRange[2]);*/
		delete []colorRange;
	}

	byte* m_ByteBuff;
	float* m_FloatBuff;
	
	switch(dataType)
	{
	case 0://byte就byte
	
			
			m_ByteBuff = new byte[width*height];			
			m_buffHeight=height;
			m_buffWidth=width;
	
	
		m_buffSize=height*width;
		if (m_pBand)
		{
			if (CE_None==m_pBand->RasterIO( GF_Read, xSrc,ySrc, srcWidth, srcHeight, m_ByteBuff, width,height, GDT_Byte, 0, 0 ))
			{				
			}
		}			
		
		for ( x = 0; x < width; x ++)
		{
			for ( y = 0; y < height; y ++) 
			{
				for(i=0;i<count;i++)
				{
					if(m_ByteBuff[y*width+x]>a[i] && m_ByteBuff[y*width+x]<b[i])
					{
						dcMem.SetPixel(x,y,color[i]);
						break;
					}
				}			
			}
		}
		pDC->BitBlt(xOff,yOff,width,height,&dcMem,0,0,SRCCOPY);	
		delete  m_ByteBuff;
		m_ByteBuff=NULL;
		break;
	default://其他类型的都用float
		
			m_FloatBuff = new float[width*height];			
			m_buffHeight=height;
			m_buffWidth=width;
	
		m_buffSize=height*width;
		if (m_pBand)
		{
			if (CE_None==m_pBand->RasterIO( GF_Read, xSrc,ySrc, srcWidth, srcHeight, m_FloatBuff, width,height, GDT_Float32, 0, 0 ))
			{				
			}
		}		
		
		for ( x = 0; x < width; x ++)
		{
			for ( y = 0; y < height; y ++) 
			{
				for(i=0;i<count;i++)
				{
					if(m_FloatBuff[y*width+x]>a[i] && m_FloatBuff[y*width+x]<b[i])
					{
						dcMem.SetPixel(x,y,color[i]);
						break;
					}
				}			
			}
		}
		pDC->BitBlt(xOff,yOff,width,height,&dcMem,0,0,SRCCOPY);	
		delete  m_FloatBuff;
		m_FloatBuff=NULL;
		break;
	}

	delete[]a;
	delete[]b;
	delete[]color;
	delete[]Rangecolores;

	
}
bool CImage::split(CString table, CString splt, CString **reslut, long *cont)
{
	* cont=0;
	int i=0;
	CString content;
	CString tmp(table);
	i=tmp.Find(splt);
	while(i>0)
	{
		content=tmp.Left(i);
		(* cont)++;
		if(tmp.GetLength()-i-splt.GetLength()<0)
			break;
		tmp=tmp.Right(tmp.GetLength()-i-splt.GetLength());		
		i=tmp.Find(splt);		
	}
	
	*reslut=new CString [(* cont)];
	tmp=table;
	i=tmp.Find(splt);
	int k=0;
	while(i>0)
	{
		(*reslut)[k]=tmp.Left(i);
		k++;		
		if(tmp.GetLength()-i-splt.GetLength()<0)
			break;
		tmp=tmp.Right(tmp.GetLength()-i-splt.GetLength());		
		i=tmp.Find(splt);		
	}
	
	return true;
}

void CImage::GrayDraw(CDC *pDC, long xOff, long yOff, long width, long height, long GrayBand, CString colorTable)
{
	long xSrc=0;
	long ySrc=0;
	long srcWidth=m_imgWidth;
	long srcHeight=m_imgHeight;
	GrayDraw(pDC,  xOff,  yOff,  width,  height,  xSrc,  ySrc,  srcWidth,  srcHeight,  GrayBand,  colorTable);
}




void CImage::Histequal(int iWidth, int iHeight, byte *source, byte *result)
{
	int i, j;
	
	int icount[256];
	int x,y;
	for (i = 0; i < 256; i++)
		icount[i] = 0;
	for ( x = 0; x < iHeight; x++)
		for ( y = 0; y < iWidth; y++)	
		{
			icount[source[x* iWidth+y]%256]++;			
		}
		
		byte bMap[256];
		int iTemp;
		for (i = 0; i < 256; i++) {
			iTemp = 0;
			
			for (j = 0; j < i; j++)
				iTemp += icount[j];
			if(iHeight * iWidth>0)
			{
				bMap[i] = (byte)(iTemp * 255 / (iHeight * iWidth));
			}
		
			
			if (bMap[i] < 0)
				bMap[i] = 0;
			else if (bMap[i] > 255)
				bMap[i] = 255;
		}
		
		for ( x = 0; x < iHeight; x++)
			for ( y = 0; y < iWidth; y++)	
				result[x* iWidth+y] = bMap[source[x* iWidth+y]%256];	
			
}

void CImage::HistequealDraw(CDC *pDC)
{
	if(pDC==NULL)
		return;	
	if (!m_pDataset)
		return;

	if(m_DC.m_hDC>0)
		m_DC.Detach();
	m_DC.Attach(pDC->m_hDC);
	m_hdc=pDC->m_hDC;

	
	GDALRasterBand  *pBand=NULL;

	byte* buf =NULL;
	byte* buftmp=NULL;
	byte* ByteBuff=NULL;
	int x,y;

	
	CDC dcMem;
	dcMem.CreateCompatibleDC(pDC);
	CBitmap bitmap;
	bitmap.CreateCompatibleBitmap(pDC,m_WndWidth,m_WnDHeight);
	dcMem.SelectObject(&bitmap);

	int i;
	
	if(m_red==0 || m_red>m_bandNum) m_red=m_CurrentBand;
	if(m_blue==0 || m_blue>m_bandNum) m_blue=m_CurrentBand;
	if(m_green==0 || m_green>m_bandNum) m_green=m_CurrentBand;
	
	if(!IsGray())
	{
		ByteBuff = new byte[m_buffHeight*m_buffWidth*3];		
					
		m_buffSize=m_buffHeight*m_buffWidth*3;
		buf = new byte[m_buffHeight*m_buffWidth];
		buftmp = new byte[m_buffHeight*m_buffWidth];
		
		pBand = m_pDataset->GetRasterBand(m_red);	
		if (pBand)
		{
			if (CE_None==pBand->RasterIO( GF_Read, m_xSrc,m_ySrc, m_srcWidth, m_srcHeight, buftmp, m_buffWidth,m_buffHeight, GDT_Byte, 0, 0 ))
			{
				Histequal(m_buffWidth,m_buffHeight,buftmp,buf);
				for ( i = 0; i < m_buffHeight*m_buffWidth; i ++) 
					ByteBuff[3*i+2] = buf[i];
			}
		}	
		pBand = m_pDataset->GetRasterBand(m_green);
		if (pBand)
		{	
		if (CE_None==pBand->RasterIO( GF_Read, m_xSrc,m_ySrc, m_srcWidth, m_srcHeight, buftmp, m_buffWidth,m_buffHeight, GDT_Byte, 0, 0 ))
			{
				Histequal(m_buffWidth,m_buffHeight,buftmp,buf);
				for ( i = 0; i <m_buffHeight*m_buffWidth; i ++) 
					ByteBuff[3*i+1] = buf[i];
			}
		}
		
		pBand = m_pDataset->GetRasterBand(m_blue);
		if (pBand)
		{	
			if (CE_None==pBand->RasterIO( GF_Read, m_xSrc,m_ySrc, m_srcWidth, m_srcHeight, buftmp, m_buffWidth,m_buffHeight, GDT_Byte, 0, 0 ))
			{
				Histequal(m_buffWidth,m_buffHeight,buftmp,buf);
				for ( i = 0; i <m_buffHeight*m_buffWidth; i ++) 
					ByteBuff[3*i+0] = buf[i];
			}
		}
		for ( x = 0; x < m_buffWidth; x ++)
		{
			for ( y = 0; y < m_buffHeight; y ++) 
			{
				dcMem.SetPixel(x,y,RGB(ByteBuff[(y*m_buffWidth+x)*3+2],ByteBuff[(y*m_buffWidth+x)*3+1],ByteBuff[(y*m_buffWidth+x)*3+0]));
			}
		}
		pDC->BitBlt(m_WndXOff,m_WndYOff,m_WndWidth,m_WnDHeight,&dcMem,0,0,SRCCOPY);
	}
	else
	{
	
		m_buffSize=m_buffHeight*m_buffWidth;
		buf = new byte[m_buffHeight*m_buffWidth];
		buftmp = new byte[m_buffHeight*m_buffWidth];
		
		pBand = m_pDataset->GetRasterBand(m_CurrentBand);	
		if (pBand)
		{
			if (CE_None==pBand->RasterIO( GF_Read, m_xSrc,m_ySrc, m_srcWidth, m_srcHeight, buftmp, m_buffWidth,m_buffHeight, GDT_Byte, 0, 0 ))
			{
				Histequal(m_buffWidth,m_buffHeight,buftmp,buf);
				
			}
		}	
	
		for ( x = 0; x < m_buffWidth; x ++)
		{
			for ( y = 0; y < m_buffHeight; y ++) 
			{
				dcMem.SetPixel(x,y,RGB(buf[(y*m_buffWidth+x)],buf[(y*m_buffWidth+x)],buf[(y*m_buffWidth+x)]));
			}
		}
		pDC->BitBlt(m_WndXOff,m_WndYOff,m_WndWidth,m_WnDHeight,&dcMem,0,0,SRCCOPY);

	}
	if(buf)
	{
		delete buf;
	}
	if(buftmp)
	{
		delete buftmp;
	}
	if(ByteBuff)
	{
		delete  ByteBuff;
	}
	buf=NULL;
	buftmp=NULL;
	ByteBuff=NULL;

}

bool CImage::IsGray()
{
	if(m_red==m_green && m_green==m_blue)
		return true;
	return false;
}

void CImage::filter33(long width, long height, float *InBuff, float *OutBuff)
{
	long i,j,jj;
	double sum=0;
	for(jj=0;jj<9;jj++)
	{
		sum+=m_filter33[jj];
	}
	if(sum==0)
		sum=1;
	for( i=0;i<height;i++)//处理中心区域像元
	{
		for( j=0;j<width;j++)
		{
			if(i!=0&&i!=height-1&&j!=0&&j!=width-1)
			{
				OutBuff[i*width+j]=(InBuff[(i-1)*width+(j-1)]*m_filter33[0] +		InBuff[(i-1)*width+(j)]*m_filter33[1]+		InBuff[(i-1)*width+(j+1)]*m_filter33[2]
					+InBuff[(i)*width+(j-1)]  *m_filter33[3] +		InBuff[(i)*width+(j)]  *m_filter33[4]+		InBuff[(i)*width+(j+1)]*m_filter33[5]
					+InBuff[(i+1)*width+(j-1)]*m_filter33[6] +		InBuff[(i+1)*width+(j)]*m_filter33[7]+		InBuff[(i+1)*width+(j+1)]*m_filter33[8]);
				OutBuff[i*width+j]/=sum ;
			}
			
		}
	}

}

void CImage::Filter33Draw(CDC *pDC)
{
	if(pDC==NULL)
		return;	
	if (!m_pDataset)
		return;

	if(m_DC.m_hDC>0)
		m_DC.Detach();
	m_DC.Attach(pDC->m_hDC);
	m_hdc=pDC->m_hDC;

	
	GDALRasterBand  *pBand=NULL;

	float* buf =NULL;
	float* buftmp=NULL;
	float * ByteBuff=NULL;
	int x,y;

	
	CDC dcMem;
	dcMem.CreateCompatibleDC(pDC);
	CBitmap bitmap;
	bitmap.CreateCompatibleBitmap(pDC,m_WndWidth,m_WnDHeight);
	dcMem.SelectObject(&bitmap);

	int i;
	
	if(m_red==0 || m_red>m_bandNum) m_red=m_CurrentBand;
	if(m_blue==0 || m_blue>m_bandNum) m_blue=m_CurrentBand;
	if(m_green==0 || m_green>m_bandNum) m_green=m_CurrentBand;
	
	if(!IsGray())
	{
		ByteBuff = new float[m_buffHeight*m_buffWidth*3];		
					
		m_buffSize=m_buffHeight*m_buffWidth*3;
		buf = new float[m_buffHeight*m_buffWidth];
		buftmp = new float[m_buffHeight*m_buffWidth];
		
		pBand = m_pDataset->GetRasterBand(m_red);	
		if (pBand)
		{
			if (CE_None==pBand->RasterIO( GF_Read, m_xSrc,m_ySrc, m_srcWidth, m_srcHeight, buftmp, m_buffWidth,m_buffHeight, GDT_Float32, 0, 0 ))
			{
				filter33(m_buffWidth,m_buffHeight,buftmp, buf);
				//Histequal(m_buffWidth,m_buffHeight,buftmp,buf);
				for ( i = 0; i < m_buffHeight*m_buffWidth; i ++) 
					ByteBuff[3*i+2] = buf[i];
			}
		}	
		pBand = m_pDataset->GetRasterBand(m_green);
		if (pBand)
		{	
		if (CE_None==pBand->RasterIO( GF_Read, m_xSrc,m_ySrc, m_srcWidth, m_srcHeight, buftmp, m_buffWidth,m_buffHeight, GDT_Float32, 0, 0 ))
			{
			//	Histequal(m_buffWidth,m_buffHeight,buftmp,buf);
				filter33(m_buffWidth,m_buffHeight,buftmp, buf);
				for ( i = 0; i <m_buffHeight*m_buffWidth; i ++) 
					ByteBuff[3*i+1] = buf[i];
			}
		}
		
		pBand = m_pDataset->GetRasterBand(m_blue);
		if (pBand)
		{	
			if (CE_None==pBand->RasterIO( GF_Read, m_xSrc,m_ySrc, m_srcWidth, m_srcHeight, buftmp, m_buffWidth,m_buffHeight, GDT_Float32, 0, 0 ))
			{
				//Histequal(m_buffWidth,m_buffHeight,buftmp,buf);
				filter33(m_buffWidth,m_buffHeight,buftmp, buf);
				for ( i = 0; i <m_buffHeight*m_buffWidth; i ++) 
					ByteBuff[3*i+0] = buf[i];
			}
		}
		for ( x = 0; x < m_buffWidth; x ++)
		{
			for ( y = 0; y < m_buffHeight; y ++) 
			{
				dcMem.SetPixel(x,y,RGB(ByteBuff[(y*m_buffWidth+x)*3+2],ByteBuff[(y*m_buffWidth+x)*3+1],ByteBuff[(y*m_buffWidth+x)*3+0]));
			}
		}
		pDC->BitBlt(m_WndXOff,m_WndYOff,m_WndWidth,m_WnDHeight,&dcMem,0,0,SRCCOPY);
	}
	else
	{
	
		m_buffSize=m_buffHeight*m_buffWidth;
		buf = new float[m_buffHeight*m_buffWidth];
		buftmp = new float[m_buffHeight*m_buffWidth];
		
		pBand = m_pDataset->GetRasterBand(m_CurrentBand);	
		if (pBand)
		{
			if (CE_None==pBand->RasterIO( GF_Read, m_xSrc,m_ySrc, m_srcWidth, m_srcHeight, buftmp, m_buffWidth,m_buffHeight, GDT_Float32, 0, 0 ))
			{
				//Histequal(m_buffWidth,m_buffHeight,buftmp,buf);
				filter33(m_buffWidth,m_buffHeight,buftmp, buf);
				
			}
		}	
	
		for ( x = 0; x < m_buffWidth; x ++)
		{
			for ( y = 0; y < m_buffHeight; y ++) 
			{
				dcMem.SetPixel(x,y,RGB(buf[(y*m_buffWidth+x)],buf[(y*m_buffWidth+x)],buf[(y*m_buffWidth+x)]));
			}
		}
		pDC->BitBlt(m_WndXOff,m_WndYOff,m_WndWidth,m_WnDHeight,&dcMem,0,0,SRCCOPY);

	}
	if(buf)
	{
		delete buf;
	}
	if(buftmp)
	{
		delete buftmp;
	}
	if(ByteBuff)
	{
		delete  ByteBuff;
	}
	buf=NULL;
	buftmp=NULL;
	ByteBuff=NULL;
}

void CImage::LaplaceDraw(CDC *pDC)
{
	m_filter33[0]=0;m_filter33[1]=1;m_filter33[2]=0;
	m_filter33[3]=1;m_filter33[4]=-4;m_filter33[5]=1;
	m_filter33[6]=0;m_filter33[7]=1;m_filter33[8]=0;

	this->Filter33Draw(pDC);
}

void CImage::VerticallyDetectDraw(CDC *pDC)
{

	m_filter33[0]=-1;m_filter33[1]=0;m_filter33[2]=1;
	m_filter33[3]=-1;m_filter33[4]=0;m_filter33[5]=1;
	m_filter33[6]=-1;m_filter33[7]=0;m_filter33[8]=1;
	
	this->Filter33Draw(pDC);

}

void CImage::HorizontallyDetectDraw(CDC *pDC)
{
	m_filter33[0]=-1;m_filter33[1]=-1;m_filter33[2]=-1;
	m_filter33[3]=0;m_filter33[4]=0;m_filter33[5]=0;
	m_filter33[6]=1;m_filter33[7]=1;m_filter33[8]=1;
	
	this->Filter33Draw(pDC);
}

void CImage::DiagonallyDetectDraw(CDC *pDC)
{

	m_filter33[0]=0;m_filter33[1]=1;m_filter33[2]=1;
	m_filter33[3]=-1;m_filter33[4]=0;m_filter33[5]=1;
	m_filter33[6]=-1;m_filter33[7]=-1;m_filter33[8]=0;
	
	this->Filter33Draw(pDC);

}

void CImage::AvgFilterDraw(CDC *pDC)
{
	m_filter33[0]=1;m_filter33[1]=1;m_filter33[2]=1;
	m_filter33[3]=1;m_filter33[4]=1;m_filter33[5]=1;
	m_filter33[6]=1;m_filter33[7]=1;m_filter33[8]=1;
	
	this->Filter33Draw(pDC);
}

void CImage::SobelFilterDraw(CDC *pDC)
{
		if(pDC==NULL)
		return;	
	if (!m_pDataset)
		return;

	if(m_DC.m_hDC>0)
		m_DC.Detach();
	m_DC.Attach(pDC->m_hDC);
	m_hdc=pDC->m_hDC;

	
	GDALRasterBand  *pBand=NULL;

	float* buf =NULL;
	float* buftmp=NULL;
	float * ByteBuff=NULL;
	int x,y;

	
	CDC dcMem;
	dcMem.CreateCompatibleDC(pDC);
	CBitmap bitmap;
	bitmap.CreateCompatibleBitmap(pDC,m_WndWidth,m_WnDHeight);
	dcMem.SelectObject(&bitmap);

	int i;
	
	if(m_red==0 || m_red>m_bandNum) m_red=m_CurrentBand;
	if(m_blue==0 || m_blue>m_bandNum) m_blue=m_CurrentBand;
	if(m_green==0 || m_green>m_bandNum) m_green=m_CurrentBand;
	
	if(!IsGray())
	{
		ByteBuff = new float[m_buffHeight*m_buffWidth*3];		
					
		m_buffSize=m_buffHeight*m_buffWidth*3;
		buf = new float[m_buffHeight*m_buffWidth];
		buftmp = new float[m_buffHeight*m_buffWidth];
		
		pBand = m_pDataset->GetRasterBand(m_red);	
		if (pBand)
		{
			if (CE_None==pBand->RasterIO( GF_Read, m_xSrc,m_ySrc, m_srcWidth, m_srcHeight, buftmp, m_buffWidth,m_buffHeight, GDT_Float32, 0, 0 ))
			{
				SobelFilter(m_buffWidth,m_buffHeight,buftmp, buf);
				//Histequal(m_buffWidth,m_buffHeight,buftmp,buf);
				for ( i = 0; i < m_buffHeight*m_buffWidth; i ++) 
					ByteBuff[3*i+2] = buf[i];
			}
		}	
		pBand = m_pDataset->GetRasterBand(m_green);
		if (pBand)
		{	
		if (CE_None==pBand->RasterIO( GF_Read, m_xSrc,m_ySrc, m_srcWidth, m_srcHeight, buftmp, m_buffWidth,m_buffHeight, GDT_Float32, 0, 0 ))
			{
			//	Histequal(m_buffWidth,m_buffHeight,buftmp,buf);
				SobelFilter(m_buffWidth,m_buffHeight,buftmp, buf);
				for ( i = 0; i <m_buffHeight*m_buffWidth; i ++) 
					ByteBuff[3*i+1] = buf[i];
			}
		}
		
		pBand = m_pDataset->GetRasterBand(m_blue);
		if (pBand)
		{	
			if (CE_None==pBand->RasterIO( GF_Read, m_xSrc,m_ySrc, m_srcWidth, m_srcHeight, buftmp, m_buffWidth,m_buffHeight, GDT_Float32, 0, 0 ))
			{
				//Histequal(m_buffWidth,m_buffHeight,buftmp,buf);
				SobelFilter(m_buffWidth,m_buffHeight,buftmp, buf);
				for ( i = 0; i <m_buffHeight*m_buffWidth; i ++) 
					ByteBuff[3*i+0] = buf[i];
			}
		}
		for ( x = 0; x < m_buffWidth; x ++)
		{
			for ( y = 0; y < m_buffHeight; y ++) 
			{
				dcMem.SetPixel(x,y,RGB(ByteBuff[(y*m_buffWidth+x)*3+2],ByteBuff[(y*m_buffWidth+x)*3+1],ByteBuff[(y*m_buffWidth+x)*3+0]));
			}
		}
		pDC->BitBlt(m_WndXOff,m_WndYOff,m_WndWidth,m_WnDHeight,&dcMem,0,0,SRCCOPY);
	}
	else
	{
	
		m_buffSize=m_buffHeight*m_buffWidth;
		buf = new float[m_buffHeight*m_buffWidth];
		buftmp = new float[m_buffHeight*m_buffWidth];
		
		pBand = m_pDataset->GetRasterBand(m_CurrentBand);	
		if (pBand)
		{
			if (CE_None==pBand->RasterIO( GF_Read, m_xSrc,m_ySrc, m_srcWidth, m_srcHeight, buftmp, m_buffWidth,m_buffHeight, GDT_Float32, 0, 0 ))
			{
				//Histequal(m_buffWidth,m_buffHeight,buftmp,buf);
				SobelFilter(m_buffWidth,m_buffHeight,buftmp, buf);
				
			}
		}	
	
		for ( x = 0; x < m_buffWidth; x ++)
		{
			for ( y = 0; y < m_buffHeight; y ++) 
			{
				dcMem.SetPixel(x,y,RGB(buf[(y*m_buffWidth+x)],buf[(y*m_buffWidth+x)],buf[(y*m_buffWidth+x)]));
			}
		}
		pDC->BitBlt(m_WndXOff,m_WndYOff,m_WndWidth,m_WnDHeight,&dcMem,0,0,SRCCOPY);

	}
	if(buf)
	{
		delete buf;
	}
	if(buftmp)
	{
		delete buftmp;
	}
	if(ByteBuff)
	{
		delete  ByteBuff;
	}
	buf=NULL;
	buftmp=NULL;
	ByteBuff=NULL;

}

void CImage::SobelFilter(long width, long height, float *InBuff, float *OutBuff)
{
	long i,j;
	double sum=0;
	int jj=0;
	double filter33[9];
	double t1,t2;
	for( i=0;i<height;i++)//处理中心区域像元
	{
		for( j=0;j<width;j++)
		{
			if(i!=0&&i!=height-1&&j!=0&&j!=width-1)
			{
				filter33[0]=-1;	filter33[1]=-2;filter33[2]=-1;
				filter33[3]=0;	filter33[4]=0;filter33[5]=0;
				filter33[6]=1;	filter33[7]=2;filter33[8]=1;
				t1=(InBuff[(i-1)*width+(j-1)]*filter33[0] +		InBuff[(i-1)*width+(j)]*filter33[1]+		InBuff[(i-1)*width+(j+1)]*filter33[2]
					+InBuff[(i)*width+(j-1)]  *filter33[3] +		InBuff[(i)*width+(j)]  *filter33[4]+		InBuff[(i)*width+(j+1)]*filter33[5]
					+InBuff[(i+1)*width+(j-1)]*filter33[6] +		InBuff[(i+1)*width+(j)]*filter33[7]+		InBuff[(i+1)*width+(j+1)]*filter33[8])/9;
				
				filter33[0]=-1;	filter33[1]=0;filter33[2]=1;
				filter33[3]=-2;	filter33[4]=0;filter33[5]=2;
				filter33[6]=-1;	filter33[7]=0;filter33[8]=1;
				t2=(InBuff[(i-1)*width+(j-1)]*filter33[0] +		InBuff[(i-1)*width+(j)]*filter33[1]+		InBuff[(i-1)*width+(j+1)]*filter33[2]
					+InBuff[(i)*width+(j-1)]  *filter33[3] +		InBuff[(i)*width+(j)]  *filter33[4]+		InBuff[(i)*width+(j+1)]*filter33[5]
					+InBuff[(i+1)*width+(j-1)]*filter33[6] +		InBuff[(i+1)*width+(j)]*filter33[7]+		InBuff[(i+1)*width+(j+1)]*filter33[8])/9;
				OutBuff[i*width+j]=	fabs(t1)+fabs(t2);
				
			}				
			
		}
	}


}

void CImage::PrewittFilter(long width, long height, float *InBuff, float *OutBuff)
{
	double sum=0;
	int jj=0;
	double filter33[9];
	double t1,t2;
	long i,j;
	for( i=0;i<height;i++)//处理中心区域像元
	{
		for( j=0;j<width;j++)
		{
			if(i!=0&&i!=height-1&&j!=0&&j!=width-1)
			{
				filter33[0]=-1;	filter33[1]=-1;filter33[2]=-1;
				filter33[3]=0;	filter33[4]=0;filter33[5]=0;
				filter33[6]=1;	filter33[7]=1;filter33[8]=1;
				t1=(InBuff[(i-1)*width+(j-1)]*filter33[0] +		InBuff[(i-1)*width+(j)]*filter33[1]+		InBuff[(i-1)*width+(j+1)]*filter33[2]
					+InBuff[(i)*width+(j-1)]  *filter33[3] +		InBuff[(i)*width+(j)]  *filter33[4]+		InBuff[(i)*width+(j+1)]*filter33[5]
					+InBuff[(i+1)*width+(j-1)]*filter33[6] +		InBuff[(i+1)*width+(j)]*filter33[7]+		InBuff[(i+1)*width+(j+1)]*filter33[8])/9;
				
				filter33[0]=-1;	filter33[1]=0;filter33[2]=1;
				filter33[3]=-1;	filter33[4]=0;filter33[5]=1;
				filter33[6]=-1;	filter33[7]=0;filter33[8]=1;
				t2=(InBuff[(i-1)*width+(j-1)]*filter33[0] +		InBuff[(i-1)*width+(j)]*filter33[1]+		InBuff[(i-1)*width+(j+1)]*filter33[2]
					+InBuff[(i)*width+(j-1)]  *filter33[3] +		InBuff[(i)*width+(j)]  *filter33[4]+		InBuff[(i)*width+(j+1)]*filter33[5]
					+InBuff[(i+1)*width+(j-1)]*filter33[6] +		InBuff[(i+1)*width+(j)]*filter33[7]+		InBuff[(i+1)*width+(j+1)]*filter33[8])/9;
				OutBuff[i*width+j]=	fabs(t1)+fabs(t2);
				
			}
		}
	}
}

void CImage::PrewittFilterDraw(CDC *pDC)
{
			if(pDC==NULL)
		return;	
	if (!m_pDataset)
		return;

	if(m_DC.m_hDC>0)
		m_DC.Detach();
	m_DC.Attach(pDC->m_hDC);
	m_hdc=pDC->m_hDC;

	
	GDALRasterBand  *pBand=NULL;

	float* buf =NULL;
	float* buftmp=NULL;
	float * ByteBuff=NULL;
	int x,y;

	
	CDC dcMem;
	dcMem.CreateCompatibleDC(pDC);
	CBitmap bitmap;
	bitmap.CreateCompatibleBitmap(pDC,m_WndWidth,m_WnDHeight);
	dcMem.SelectObject(&bitmap);

	int i;
	
	if(m_red==0 || m_red>m_bandNum) m_red=m_CurrentBand;
	if(m_blue==0 || m_blue>m_bandNum) m_blue=m_CurrentBand;
	if(m_green==0 || m_green>m_bandNum) m_green=m_CurrentBand;
	
	if(!IsGray())
	{
		ByteBuff = new float[m_buffHeight*m_buffWidth*3];		
					
		m_buffSize=m_buffHeight*m_buffWidth*3;
		buf = new float[m_buffHeight*m_buffWidth];
		buftmp = new float[m_buffHeight*m_buffWidth];
		
		pBand = m_pDataset->GetRasterBand(m_red);	
		if (pBand)
		{
			if (CE_None==pBand->RasterIO( GF_Read, m_xSrc,m_ySrc, m_srcWidth, m_srcHeight, buftmp, m_buffWidth,m_buffHeight, GDT_Float32, 0, 0 ))
			{
				PrewittFilter(m_buffWidth,m_buffHeight,buftmp, buf);
				//Histequal(m_buffWidth,m_buffHeight,buftmp,buf);
				for ( i = 0; i < m_buffHeight*m_buffWidth; i ++) 
					ByteBuff[3*i+2] = buf[i];
			}
		}	
		pBand = m_pDataset->GetRasterBand(m_green);
		if (pBand)
		{	
		if (CE_None==pBand->RasterIO( GF_Read, m_xSrc,m_ySrc, m_srcWidth, m_srcHeight, buftmp, m_buffWidth,m_buffHeight, GDT_Float32, 0, 0 ))
			{
			//	Histequal(m_buffWidth,m_buffHeight,buftmp,buf);
				PrewittFilter(m_buffWidth,m_buffHeight,buftmp, buf);
				for ( i = 0; i <m_buffHeight*m_buffWidth; i ++) 
					ByteBuff[3*i+1] = buf[i];
			}
		}
		
		pBand = m_pDataset->GetRasterBand(m_blue);
		if (pBand)
		{	
			if (CE_None==pBand->RasterIO( GF_Read, m_xSrc,m_ySrc, m_srcWidth, m_srcHeight, buftmp, m_buffWidth,m_buffHeight, GDT_Float32, 0, 0 ))
			{
				//Histequal(m_buffWidth,m_buffHeight,buftmp,buf);
				PrewittFilter(m_buffWidth,m_buffHeight,buftmp, buf);
				for ( i = 0; i <m_buffHeight*m_buffWidth; i ++) 
					ByteBuff[3*i+0] = buf[i];
			}
		}
		for ( x = 0; x < m_buffWidth; x ++)
		{
			for ( y = 0; y < m_buffHeight; y ++) 
			{
				dcMem.SetPixel(x,y,RGB(ByteBuff[(y*m_buffWidth+x)*3+2],ByteBuff[(y*m_buffWidth+x)*3+1],ByteBuff[(y*m_buffWidth+x)*3+0]));
			}
		}
		pDC->BitBlt(m_WndXOff,m_WndYOff,m_WndWidth,m_WnDHeight,&dcMem,0,0,SRCCOPY);
	}
	else
	{
	
		m_buffSize=m_buffHeight*m_buffWidth;
		buf = new float[m_buffHeight*m_buffWidth];
		buftmp = new float[m_buffHeight*m_buffWidth];
		
		pBand = m_pDataset->GetRasterBand(m_CurrentBand);	
		if (pBand)
		{
			if (CE_None==pBand->RasterIO( GF_Read, m_xSrc,m_ySrc, m_srcWidth, m_srcHeight, buftmp, m_buffWidth,m_buffHeight, GDT_Float32, 0, 0 ))
			{
				//Histequal(m_buffWidth,m_buffHeight,buftmp,buf);
				PrewittFilter(m_buffWidth,m_buffHeight,buftmp, buf);
				
			}
		}	
	
		for ( x = 0; x < m_buffWidth; x ++)
		{
			for ( y = 0; y < m_buffHeight; y ++) 
			{
				dcMem.SetPixel(x,y,RGB(buf[(y*m_buffWidth+x)],buf[(y*m_buffWidth+x)],buf[(y*m_buffWidth+x)]));
			}
		}
		pDC->BitBlt(m_WndXOff,m_WndYOff,m_WndWidth,m_WnDHeight,&dcMem,0,0,SRCCOPY);

	}
	if(buf)
	{
		delete buf;
	}
	if(buftmp)
	{
		delete buftmp;
	}
	if(ByteBuff)
	{
		delete  ByteBuff;
	}
	buf=NULL;
	buftmp=NULL;
	ByteBuff=NULL;
}

void CImage::RobertsFilter(long width, long height, float *InBuff, float *OutBuff)
{
	long i,j;
	for( i=0;i<height;i++)//处理中心区域像元
	{
		for( j=0;j<width;j++)
		{
			if(i!=0&&i!=height-1&&j!=0&&j!=width-1)
			{
				OutBuff[i*width+j]=	fabs(InBuff[(i)*width+(j)] -InBuff[(i+1)*width+(j+1)])+fabs(InBuff[(i+1)*width+(j)] -InBuff[(i)*width+(j+1)]);
				
			}
		}
	}

}

void CImage::RobertsFilterDraw(CDC *pDC)
{
	if(pDC==NULL)
		return;	
	if (!m_pDataset)
		return;

	if(m_DC.m_hDC>0)
		m_DC.Detach();
	m_DC.Attach(pDC->m_hDC);
	m_hdc=pDC->m_hDC;

	
	GDALRasterBand  *pBand=NULL;

	float* buf =NULL;
	float* buftmp=NULL;
	float * ByteBuff=NULL;
	int x,y;

	
	CDC dcMem;
	dcMem.CreateCompatibleDC(pDC);
	CBitmap bitmap;
	bitmap.CreateCompatibleBitmap(pDC,m_WndWidth,m_WnDHeight);
	dcMem.SelectObject(&bitmap);

	int i;
	
	if(m_red==0 || m_red>m_bandNum) m_red=m_CurrentBand;
	if(m_blue==0 || m_blue>m_bandNum) m_blue=m_CurrentBand;
	if(m_green==0 || m_green>m_bandNum) m_green=m_CurrentBand;
	
	if(!IsGray())
	{
		ByteBuff = new float[m_buffHeight*m_buffWidth*3];		
					
		m_buffSize=m_buffHeight*m_buffWidth*3;
		buf = new float[m_buffHeight*m_buffWidth];
		buftmp = new float[m_buffHeight*m_buffWidth];
		
		pBand = m_pDataset->GetRasterBand(m_red);	
		if (pBand)
		{
			if (CE_None==pBand->RasterIO( GF_Read, m_xSrc,m_ySrc, m_srcWidth, m_srcHeight, buftmp, m_buffWidth,m_buffHeight, GDT_Float32, 0, 0 ))
			{
				RobertsFilter(m_buffWidth,m_buffHeight,buftmp, buf);
				//Histequal(m_buffWidth,m_buffHeight,buftmp,buf);
				for ( i = 0; i < m_buffHeight*m_buffWidth; i ++) 
					ByteBuff[3*i+2] = buf[i];
			}
		}	
		pBand = m_pDataset->GetRasterBand(m_green);
		if (pBand)
		{	
		if (CE_None==pBand->RasterIO( GF_Read, m_xSrc,m_ySrc, m_srcWidth, m_srcHeight, buftmp, m_buffWidth,m_buffHeight, GDT_Float32, 0, 0 ))
			{
			//	Histequal(m_buffWidth,m_buffHeight,buftmp,buf);
				RobertsFilter(m_buffWidth,m_buffHeight,buftmp, buf);
				for ( i = 0; i <m_buffHeight*m_buffWidth; i ++) 
					ByteBuff[3*i+1] = buf[i];
			}
		}
		
		pBand = m_pDataset->GetRasterBand(m_blue);
		if (pBand)
		{	
			if (CE_None==pBand->RasterIO( GF_Read, m_xSrc,m_ySrc, m_srcWidth, m_srcHeight, buftmp, m_buffWidth,m_buffHeight, GDT_Float32, 0, 0 ))
			{
				//Histequal(m_buffWidth,m_buffHeight,buftmp,buf);
				RobertsFilter(m_buffWidth,m_buffHeight,buftmp, buf);
				for ( i = 0; i <m_buffHeight*m_buffWidth; i ++) 
					ByteBuff[3*i+0] = buf[i];
			}
		}
		for ( x = 0; x < m_buffWidth; x ++)
		{
			for ( y = 0; y < m_buffHeight; y ++) 
			{
				dcMem.SetPixel(x,y,RGB(ByteBuff[(y*m_buffWidth+x)*3+2],ByteBuff[(y*m_buffWidth+x)*3+1],ByteBuff[(y*m_buffWidth+x)*3+0]));
			}
		}
		pDC->BitBlt(m_WndXOff,m_WndYOff,m_WndWidth,m_WnDHeight,&dcMem,0,0,SRCCOPY);
	}
	else
	{
	
		m_buffSize=m_buffHeight*m_buffWidth;
		buf = new float[m_buffHeight*m_buffWidth];
		buftmp = new float[m_buffHeight*m_buffWidth];
		
		pBand = m_pDataset->GetRasterBand(m_CurrentBand);	
		if (pBand)
		{
			if (CE_None==pBand->RasterIO( GF_Read, m_xSrc,m_ySrc, m_srcWidth, m_srcHeight, buftmp, m_buffWidth,m_buffHeight, GDT_Float32, 0, 0 ))
			{
				//Histequal(m_buffWidth,m_buffHeight,buftmp,buf);
				RobertsFilter(m_buffWidth,m_buffHeight,buftmp, buf);
				
			}
		}	
	
		for ( x = 0; x < m_buffWidth; x ++)
		{
			for ( y = 0; y < m_buffHeight; y ++) 
			{
				dcMem.SetPixel(x,y,RGB(buf[(y*m_buffWidth+x)],buf[(y*m_buffWidth+x)],buf[(y*m_buffWidth+x)]));
			}
		}
		pDC->BitBlt(m_WndXOff,m_WndYOff,m_WndWidth,m_WnDHeight,&dcMem,0,0,SRCCOPY);

	}
	if(buf)
	{
		delete buf;
	}
	if(buftmp)
	{
		delete buftmp;
	}
	if(ByteBuff)
	{
		delete  ByteBuff;
	}
	buf=NULL;
	buftmp=NULL;
	ByteBuff=NULL;
}

void CImage::midFilter(long width, long height, float *InBuff, float *OutBuff)
{
	double filter[9];
	long i,j;
	
	for( i=0;i<height;i++)//处理中心区域像元
	{
		for( j=0;j<width;j++)
		{
			if(i!=0&&i!=height-1&&j!=0&&j!=width-1)
			{
				
				filter[0]=InBuff[(i-1)*width+(j-1)];
				filter[1]=InBuff[(i-1)*width+(j)];
				filter[2]=InBuff[(i-1)*width+(j+1)];
				filter[3]=InBuff[(i)*width+(j-1)];
				filter[4]=InBuff[(i)*width+(j)];
				filter[5]=InBuff[(i)*width+(j+1)];
				filter[6]=InBuff[(i+1)*width+(j-1)];
				filter[7]=InBuff[(i+1)*width+(j)];
				filter[8]=InBuff[(i+1)*width+(j+1)];
				prbub(9,filter)  ;		
				
				OutBuff[i*width+j]=filter[4];
			}
		}
	}
}
void CImage::prbub(int n, double p[])  
{ 
	int m,k,j,i;
    double d;
    k=0; m=n-1;
    while (k<m)
	{ j=m-1; m=0;
	for (i=k; i<=j; i++)
		if (p[i]>p[i+1])
		{ d=p[i]; p[i]=p[i+1]; p[i+1]=d; m=i;}
        j=k+1; k=0;
        for (i=m; i>=j; i--)
			if (p[i-1]>p[i])
            { d=p[i]; p[i]=p[i-1]; p[i-1]=d; k=i;}
	}
    return;
  }

void CImage::MidFilterDraw(CDC *pDC)
{
	if(pDC==NULL)
		return;	
	if (!m_pDataset)
		return;

	if(m_DC.m_hDC>0)
		m_DC.Detach();
	m_DC.Attach(pDC->m_hDC);
	m_hdc=pDC->m_hDC;

	
	GDALRasterBand  *pBand=NULL;

	float* buf =NULL;
	float* buftmp=NULL;
	float * ByteBuff=NULL;
	int x,y;

	
	CDC dcMem;
	dcMem.CreateCompatibleDC(pDC);
	CBitmap bitmap;
	bitmap.CreateCompatibleBitmap(pDC,m_WndWidth,m_WnDHeight);
	dcMem.SelectObject(&bitmap);

	int i;
	
	if(m_red==0 || m_red>m_bandNum) m_red=m_CurrentBand;
	if(m_blue==0 || m_blue>m_bandNum) m_blue=m_CurrentBand;
	if(m_green==0 || m_green>m_bandNum) m_green=m_CurrentBand;
	
	if(!IsGray())
	{
		ByteBuff = new float[m_buffHeight*m_buffWidth*3];		
					
		m_buffSize=m_buffHeight*m_buffWidth*3;
		buf = new float[m_buffHeight*m_buffWidth];
		buftmp = new float[m_buffHeight*m_buffWidth];
		
		pBand = m_pDataset->GetRasterBand(m_red);	
		if (pBand)
		{
			if (CE_None==pBand->RasterIO( GF_Read, m_xSrc,m_ySrc, m_srcWidth, m_srcHeight, buftmp, m_buffWidth,m_buffHeight, GDT_Float32, 0, 0 ))
			{
				midFilter(m_buffWidth,m_buffHeight,buftmp, buf);
				//Histequal(m_buffWidth,m_buffHeight,buftmp,buf);
				for ( i = 0; i < m_buffHeight*m_buffWidth; i ++) 
					ByteBuff[3*i+2] = buf[i];
			}
		}	
		pBand = m_pDataset->GetRasterBand(m_green);
		if (pBand)
		{	
		if (CE_None==pBand->RasterIO( GF_Read, m_xSrc,m_ySrc, m_srcWidth, m_srcHeight, buftmp, m_buffWidth,m_buffHeight, GDT_Float32, 0, 0 ))
			{
			//	Histequal(m_buffWidth,m_buffHeight,buftmp,buf);
				midFilter(m_buffWidth,m_buffHeight,buftmp, buf);
				for ( i = 0; i <m_buffHeight*m_buffWidth; i ++) 
					ByteBuff[3*i+1] = buf[i];
			}
		}
		
		pBand = m_pDataset->GetRasterBand(m_blue);
		if (pBand)
		{	
			if (CE_None==pBand->RasterIO( GF_Read, m_xSrc,m_ySrc, m_srcWidth, m_srcHeight, buftmp, m_buffWidth,m_buffHeight, GDT_Float32, 0, 0 ))
			{
				//Histequal(m_buffWidth,m_buffHeight,buftmp,buf);
				midFilter(m_buffWidth,m_buffHeight,buftmp, buf);
				for ( i = 0; i <m_buffHeight*m_buffWidth; i ++) 
					ByteBuff[3*i+0] = buf[i];
			}
		}
		for ( x = 0; x < m_buffWidth; x ++)
		{
			for ( y = 0; y < m_buffHeight; y ++) 
			{
				dcMem.SetPixel(x,y,RGB(ByteBuff[(y*m_buffWidth+x)*3+2],ByteBuff[(y*m_buffWidth+x)*3+1],ByteBuff[(y*m_buffWidth+x)*3+0]));
			}
		}
		pDC->BitBlt(m_WndXOff,m_WndYOff,m_WndWidth,m_WnDHeight,&dcMem,0,0,SRCCOPY);
	}
	else
	{
	
		m_buffSize=m_buffHeight*m_buffWidth;
		buf = new float[m_buffHeight*m_buffWidth];
		buftmp = new float[m_buffHeight*m_buffWidth];
		
		pBand = m_pDataset->GetRasterBand(m_CurrentBand);	
		if (pBand)
		{
			if (CE_None==pBand->RasterIO( GF_Read, m_xSrc,m_ySrc, m_srcWidth, m_srcHeight, buftmp, m_buffWidth,m_buffHeight, GDT_Float32, 0, 0 ))
			{
				//Histequal(m_buffWidth,m_buffHeight,buftmp,buf);
				midFilter(m_buffWidth,m_buffHeight,buftmp, buf);
				
			}
		}	
	
		for ( x = 0; x < m_buffWidth; x ++)
		{
			for ( y = 0; y < m_buffHeight; y ++) 
			{
				dcMem.SetPixel(x,y,RGB(buf[(y*m_buffWidth+x)],buf[(y*m_buffWidth+x)],buf[(y*m_buffWidth+x)]));
			}
		}
		pDC->BitBlt(m_WndXOff,m_WndYOff,m_WndWidth,m_WnDHeight,&dcMem,0,0,SRCCOPY);

	}
	if(buf)
	{
		delete buf;
	}
	if(buftmp)
	{
		delete buftmp;
	}
	if(ByteBuff)
	{
		delete  ByteBuff;
	}
	buf=NULL;
	buftmp=NULL;
	ByteBuff=NULL;
}



void CImage::NegateDraw(CDC *pDC)
{
	if(pDC==NULL)
		return;	
	if (!m_pDataset)
		return;

	if(m_DC.m_hDC>0)
		m_DC.Detach();
	m_DC.Attach(pDC->m_hDC);
	m_hdc=pDC->m_hDC;

	
	GDALRasterBand  *pBand=NULL;

	float* buf =NULL;
	float * ByteBuff=NULL;
	int x,y;

	
	CDC dcMem;
	dcMem.CreateCompatibleDC(pDC);
	CBitmap bitmap;
	bitmap.CreateCompatibleBitmap(pDC,m_WndWidth,m_WnDHeight);
	dcMem.SelectObject(&bitmap);

	int i;
	
	if(m_red==0 || m_red>m_bandNum) m_red=m_CurrentBand;
	if(m_blue==0 || m_blue>m_bandNum) m_blue=m_CurrentBand;
	if(m_green==0 || m_green>m_bandNum) m_green=m_CurrentBand;
	
	if(!IsGray())
	{
		ByteBuff = new float[m_buffHeight*m_buffWidth*3];		
					
		m_buffSize=m_buffHeight*m_buffWidth*3;
		buf = new float[m_buffHeight*m_buffWidth];	
		
		pBand = m_pDataset->GetRasterBand(m_red);	
		if (pBand)
		{
			if (CE_None==pBand->RasterIO( GF_Read, m_xSrc,m_ySrc, m_srcWidth, m_srcHeight, buf, m_buffWidth,m_buffHeight, GDT_Float32, 0, 0 ))
			{
				
				for ( i = 0; i < m_buffHeight*m_buffWidth; i ++) 
					ByteBuff[3*i+2] = buf[i];
			}
		}	
		pBand = m_pDataset->GetRasterBand(m_green);
		if (pBand)
		{	
		if (CE_None==pBand->RasterIO( GF_Read, m_xSrc,m_ySrc, m_srcWidth, m_srcHeight, buf, m_buffWidth,m_buffHeight, GDT_Float32, 0, 0 ))
			{		
				for ( i = 0; i <m_buffHeight*m_buffWidth; i ++) 
					ByteBuff[3*i+1] = buf[i];
			}
		}
		
		pBand = m_pDataset->GetRasterBand(m_blue);
		if (pBand)
		{	
			if (CE_None==pBand->RasterIO( GF_Read, m_xSrc,m_ySrc, m_srcWidth, m_srcHeight, buf, m_buffWidth,m_buffHeight, GDT_Float32, 0, 0 ))
			{
				
				for ( i = 0; i <m_buffHeight*m_buffWidth; i ++) 
					ByteBuff[3*i+0] = buf[i];
			}
		}
		for ( x = 0; x < m_buffWidth; x ++)
		{
			for ( y = 0; y < m_buffHeight; y ++) 
			{
				dcMem.SetPixel(x,y,RGB(255-ByteBuff[(y*m_buffWidth+x)*3+2],255-ByteBuff[(y*m_buffWidth+x)*3+1],255-ByteBuff[(y*m_buffWidth+x)*3+0]));
			}
		}
		pDC->BitBlt(m_WndXOff,m_WndYOff,m_WndWidth,m_WnDHeight,&dcMem,0,0,SRCCOPY);
	}
	else
	{
	
		m_buffSize=m_buffHeight*m_buffWidth;
		buf = new float[m_buffHeight*m_buffWidth];
	
		pBand = m_pDataset->GetRasterBand(m_CurrentBand);	
		if (pBand)
		{
			if (CE_None==pBand->RasterIO( GF_Read, m_xSrc,m_ySrc, m_srcWidth, m_srcHeight, buf, m_buffWidth,m_buffHeight, GDT_Float32, 0, 0 ))
			{
			
			}
		}	
	
		for ( x = 0; x < m_buffWidth; x ++)
		{
			for ( y = 0; y < m_buffHeight; y ++) 
			{
				dcMem.SetPixel(x,y,RGB(255-buf[(y*m_buffWidth+x)],255-buf[(y*m_buffWidth+x)],255-buf[(y*m_buffWidth+x)]));
			}
		}
		pDC->BitBlt(m_WndXOff,m_WndYOff,m_WndWidth,m_WnDHeight,&dcMem,0,0,SRCCOPY);

	}
	if(buf)
	{
		delete buf;
	}

	if(ByteBuff)
	{
		delete  ByteBuff;
	}
	buf=NULL;
	ByteBuff=NULL;
}

void CImage::RGBtoHSL(float r, float g, float b, float &H, float &S, float &L)
{
	 double delta;
	 double cmax = max(r,max(g,b));
	 double cmin = min(r,min(g,b));
	 L = (cmax+cmin)/2.0;
	 if (cmax == cmin) 
	 {
	 	 S = 0;
	 	 H = 0;
	 } 
	 else 
	 {
	 	 if (L < 0.5) 
	 	 	 S = (cmax-cmin)/(cmax+cmin);
	 	 else
	 	 	 S = (cmax-cmin)/(2.0-cmax-cmin);
	 	 delta = cmax - cmin;
	 	 if (r==cmax)
	 	 	 H = (g-b)/delta;
	 	 else if (g==cmax)
	 	 	 H = 2.0 +(b-r)/delta;
	 	 else
	 	 	 H = 4.0+(r-g)/delta;
	 	 H /= 6.0;
	 	 if (H < 0.0)
	 	 	 H += 1.0;
	
	 	 else if(H > 1.0)
	 	 	 H -= 1.0;	
	 }
}

double CImage::HuetoRGB(double m1, double m2, double h)
{
	if (h < 0) 
		h += 1.0;
	if (h > 1) 
		h -= 1.0;
	if (6.0*h < 1)
		return (m1+(m2-m1)*h*6.0);
	if (2.0*h < 1)
		return m2;
	if (3.0*h < 2.0)
		return (m1+(m2-m1)*((2.0/3.0)-h)*6.0);
	return m1;
}

void CImage::HLStoRGB(float H, float L, float S, float &r, float &g, float &b)
{
	double m1, m2;
	if (S==0) 
	{
		r=g=b=L;
	} 
	else 
	{
		if (L <= 0.5)
			m2 = L*(1.0+S);
		else
			m2 = L+S-L*S;
		m1 = 2.0*L-m2;
		r = HuetoRGB(m1, m2, H+1.0/3.0);
		g = HuetoRGB(m1, m2, H);
		b = HuetoRGB(m1, m2, H-1.0/3.0);
	}


}

void CImage::AdjustHueDraw(CDC *pDC, float Percentage)
{
	//if(Percentage>100.0 && Percentage<0 )
	//		return;
	
	if(pDC==NULL)
		return;	
	if (!m_pDataset)
		return;
	
	if(m_DC.m_hDC>0)
		m_DC.Detach();
	m_DC.Attach(pDC->m_hDC);
	m_hdc=pDC->m_hDC;
	
	
	GDALRasterBand  *pBand=NULL;
	
	float* buf =NULL;

	float * ByteBuff=NULL;
	int x,y;
	
	
	CDC dcMem;
	dcMem.CreateCompatibleDC(pDC);
	CBitmap bitmap;
	bitmap.CreateCompatibleBitmap(pDC,m_WndWidth,m_WnDHeight);
	dcMem.SelectObject(&bitmap);
	
	int i;
	
	if(m_red==0 || m_red>m_bandNum) m_red=m_CurrentBand;
	if(m_blue==0 || m_blue>m_bandNum) m_blue=m_CurrentBand;
	if(m_green==0 || m_green>m_bandNum) m_green=m_CurrentBand;
	
	if(!IsGray())
	{
		ByteBuff = new float[m_buffHeight*m_buffWidth*3];		
		
		m_buffSize=m_buffHeight*m_buffWidth*3;
		buf = new float[m_buffHeight*m_buffWidth];
	
		
		pBand = m_pDataset->GetRasterBand(m_red);	
		if (pBand)
		{
			if (CE_None==pBand->RasterIO( GF_Read, m_xSrc,m_ySrc, m_srcWidth, m_srcHeight, buf, m_buffWidth,m_buffHeight, GDT_Float32, 0, 0 ))
			{
				
				for ( i = 0; i < m_buffHeight*m_buffWidth; i ++) 
					ByteBuff[3*i+2] = buf[i];
			}
		}	
		pBand = m_pDataset->GetRasterBand(m_green);
		if (pBand)
		{	
			if (CE_None==pBand->RasterIO( GF_Read, m_xSrc,m_ySrc, m_srcWidth, m_srcHeight, buf, m_buffWidth,m_buffHeight, GDT_Float32, 0, 0 ))
			{
			
				for ( i = 0; i <m_buffHeight*m_buffWidth; i ++) 
					ByteBuff[3*i+1] = buf[i];
			}
		}
		
		pBand = m_pDataset->GetRasterBand(m_blue);
		if (pBand)
		{	
			if (CE_None==pBand->RasterIO( GF_Read, m_xSrc,m_ySrc, m_srcWidth, m_srcHeight, buf, m_buffWidth,m_buffHeight, GDT_Float32, 0, 0 ))
			{
				
				for ( i = 0; i <m_buffHeight*m_buffWidth; i ++) 
					ByteBuff[3*i+0] = buf[i];
			}
		}
		float r,g,b ,H,L,S;
		for ( x = 0; x < m_buffWidth; x ++)
		{
			for ( y = 0; y < m_buffHeight; y ++) 
			{
				r=ByteBuff[(y*m_buffWidth+x)*3+2];
				g=ByteBuff[(y*m_buffWidth+x)*3+1];
				b=ByteBuff[(y*m_buffWidth+x)*3+0];
				RGBtoHSL( r,g,b,H,S,L);
				H = max(0.0,H*Percentage/100.0);
				H = min(H,1.0);
				HLStoRGB(H,L,S,r,g,b);
				dcMem.SetPixel(x,y,RGB(r,g,b));
			}
		}
		pDC->BitBlt(m_WndXOff,m_WndYOff,m_WndWidth,m_WnDHeight,&dcMem,0,0,SRCCOPY);
	}

	if(buf)
	{
		delete buf;
	}

	if(ByteBuff)
	{
		delete  ByteBuff;
	}
	buf=NULL;

	ByteBuff=NULL;

}
void CImage::AdjustBrightnessDraw(CDC *pDC, float Percentage)
{
//	if(Percentage>100.0 && Percentage<0 )
//		return;

	if(pDC==NULL)
		return;	
	if (!m_pDataset)
		return;
	
	if(m_DC.m_hDC>0)
		m_DC.Detach();
	m_DC.Attach(pDC->m_hDC);
	m_hdc=pDC->m_hDC;
	
	
	GDALRasterBand  *pBand=NULL;
	
	float* buf =NULL;
	
	float * ByteBuff=NULL;
	int x,y;
	
	
	CDC dcMem;
	dcMem.CreateCompatibleDC(pDC);
	CBitmap bitmap;
	bitmap.CreateCompatibleBitmap(pDC,m_WndWidth,m_WnDHeight);
	dcMem.SelectObject(&bitmap);
	
	int i;
	
	if(m_red==0 || m_red>m_bandNum) m_red=m_CurrentBand;
	if(m_blue==0 || m_blue>m_bandNum) m_blue=m_CurrentBand;
	if(m_green==0 || m_green>m_bandNum) m_green=m_CurrentBand;
	
	if(!IsGray())
	{
		ByteBuff = new float[m_buffHeight*m_buffWidth*3];		
		
		m_buffSize=m_buffHeight*m_buffWidth*3;
		buf = new float[m_buffHeight*m_buffWidth];
		
		
		pBand = m_pDataset->GetRasterBand(m_red);	
		if (pBand)
		{
			if (CE_None==pBand->RasterIO( GF_Read, m_xSrc,m_ySrc, m_srcWidth, m_srcHeight, buf, m_buffWidth,m_buffHeight, GDT_Float32, 0, 0 ))
			{
				
				for ( i = 0; i < m_buffHeight*m_buffWidth; i ++) 
					ByteBuff[3*i+2] = buf[i];
			}
		}	
		pBand = m_pDataset->GetRasterBand(m_green);
		if (pBand)
		{	
			if (CE_None==pBand->RasterIO( GF_Read, m_xSrc,m_ySrc, m_srcWidth, m_srcHeight, buf, m_buffWidth,m_buffHeight, GDT_Float32, 0, 0 ))
			{
				
				for ( i = 0; i <m_buffHeight*m_buffWidth; i ++) 
					ByteBuff[3*i+1] = buf[i];
			}
		}
		
		pBand = m_pDataset->GetRasterBand(m_blue);
		if (pBand)
		{	
			if (CE_None==pBand->RasterIO( GF_Read, m_xSrc,m_ySrc, m_srcWidth, m_srcHeight, buf, m_buffWidth,m_buffHeight, GDT_Float32, 0, 0 ))
			{
				
				for ( i = 0; i <m_buffHeight*m_buffWidth; i ++) 
					ByteBuff[3*i+0] = buf[i];
			}
		}
		float r,g,b;
		for ( x = 0; x < m_buffWidth; x ++)
		{
			for ( y = 0; y < m_buffHeight; y ++) 
			{
				r=ByteBuff[(y*m_buffWidth+x)*3+2];
				g=ByteBuff[(y*m_buffWidth+x)*3+1];
				b=ByteBuff[(y*m_buffWidth+x)*3+0];
				r=min(r*Percentage/100.0,255);
				g=min(g*Percentage/100.0,255);
				b=min(b*Percentage/100.0,255);
				r=max(r,0);
				g=max(g,0);
				b=max(b,0);
				dcMem.SetPixel(x,y,RGB(r,g,b));
			}
		}
		pDC->BitBlt(m_WndXOff,m_WndYOff,m_WndWidth,m_WnDHeight,&dcMem,0,0,SRCCOPY);
	}
	else//灰度图
	{
		buf = new float[m_buffHeight*m_buffWidth];		
		float r;
		m_buffSize=m_buffHeight*m_buffWidth;
		pBand = m_pDataset->GetRasterBand(m_CurrentBand);	
		if (pBand)
		{
			if (CE_None==pBand->RasterIO( GF_Read, m_xSrc,m_ySrc, m_srcWidth, m_srcHeight, buf, m_buffWidth,m_buffHeight, GDT_Float32, 0, 0 ))
			{
				for ( x = 0; x < m_buffWidth; x ++)
				{
					for ( y = 0; y < m_buffHeight; y ++) 
					{
						r=buf[(y*m_buffWidth+x)];					
						r=min(r*Percentage/100.0,255);					
						r=max(r,0);					
						dcMem.SetPixel(x,y,RGB(r,r,r));
					}
				}
				pDC->BitBlt(m_WndXOff,m_WndYOff,m_WndWidth,m_WnDHeight,&dcMem,0,0,SRCCOPY);
			}
		}	

	}
	
	if(buf)
	{
		delete buf;
	}
	
	if(ByteBuff)
	{
		delete  ByteBuff;
	}
	buf=NULL;
	
	ByteBuff=NULL;

}

void CImage::AdjustContrastDraw(CDC *pDC, float Percentage)
{
	//if(Percentage>100.0 && Percentage<0 )
	//		return;
	
	if(pDC==NULL)
		return;	
	if (!m_pDataset)
		return;
	
	if(m_DC.m_hDC>0)
		m_DC.Detach();
	m_DC.Attach(pDC->m_hDC);
	m_hdc=pDC->m_hDC;
	
	
	GDALRasterBand  *pBand=NULL;
	
	float* buf =NULL;
	
	float * ByteBuff=NULL;
	int x,y;
	
	
	CDC dcMem;
	dcMem.CreateCompatibleDC(pDC);
	CBitmap bitmap;
	bitmap.CreateCompatibleBitmap(pDC,m_WndWidth,m_WnDHeight);
	dcMem.SelectObject(&bitmap);
	
	int i;
	
	if(m_red==0 || m_red>m_bandNum) m_red=m_CurrentBand;
	if(m_blue==0 || m_blue>m_bandNum) m_blue=m_CurrentBand;
	if(m_green==0 || m_green>m_bandNum) m_green=m_CurrentBand;
	
	if(!IsGray())
	{
		ByteBuff = new float[m_buffHeight*m_buffWidth*3];		
		
		m_buffSize=m_buffHeight*m_buffWidth*3;
		buf = new float[m_buffHeight*m_buffWidth];
		
		
		pBand = m_pDataset->GetRasterBand(m_red);	
		if (pBand)
		{
			if (CE_None==pBand->RasterIO( GF_Read, m_xSrc,m_ySrc, m_srcWidth, m_srcHeight, buf, m_buffWidth,m_buffHeight, GDT_Float32, 0, 0 ))
			{
				
				for ( i = 0; i < m_buffHeight*m_buffWidth; i ++) 
					ByteBuff[3*i+2] = buf[i];
			}
		}	
		pBand = m_pDataset->GetRasterBand(m_green);
		if (pBand)
		{	
			if (CE_None==pBand->RasterIO( GF_Read, m_xSrc,m_ySrc, m_srcWidth, m_srcHeight, buf, m_buffWidth,m_buffHeight, GDT_Float32, 0, 0 ))
			{
				
				for ( i = 0; i <m_buffHeight*m_buffWidth; i ++) 
					ByteBuff[3*i+1] = buf[i];
			}
		}
		
		pBand = m_pDataset->GetRasterBand(m_blue);
		if (pBand)
		{	
			if (CE_None==pBand->RasterIO( GF_Read, m_xSrc,m_ySrc, m_srcWidth, m_srcHeight, buf, m_buffWidth,m_buffHeight, GDT_Float32, 0, 0 ))
			{
				
				for ( i = 0; i <m_buffHeight*m_buffWidth; i ++) 
					ByteBuff[3*i+0] = buf[i];
			}
		}
		float r,g,b ;
		for ( x = 0; x < m_buffWidth; x ++)
		{
			for ( y = 0; y < m_buffHeight; y ++) 
			{
				r=ByteBuff[(y*m_buffWidth+x)*3+2];
				g=ByteBuff[(y*m_buffWidth+x)*3+1];
				b=ByteBuff[(y*m_buffWidth+x)*3+0];
				r=min(128+(r-128)*Percentage/100.0,255);
				g=min(128+(g-128)*Percentage/100.0,255);
				b=min(128+(b-128)*Percentage/100.0,255);
				r=max(r,0);
				g=max(g,0);
				b=max(b,0);
				dcMem.SetPixel(x,y,RGB(r,g,b));
			}
		}
		pDC->BitBlt(m_WndXOff,m_WndYOff,m_WndWidth,m_WnDHeight,&dcMem,0,0,SRCCOPY);
	}
	else//灰度图
	{
		buf = new float[m_buffHeight*m_buffWidth];		
		float r;
		m_buffSize=m_buffHeight*m_buffWidth;
		pBand = m_pDataset->GetRasterBand(m_CurrentBand);	
		if (pBand)
		{
			if (CE_None==pBand->RasterIO( GF_Read, m_xSrc,m_ySrc, m_srcWidth, m_srcHeight, buf, m_buffWidth,m_buffHeight, GDT_Float32, 0, 0 ))
			{
				for ( x = 0; x < m_buffWidth; x ++)
				{
					for ( y = 0; y < m_buffHeight; y ++) 
					{
						r=buf[(y*m_buffWidth+x)];					
						r=min(128+(r-128)*Percentage/100.0,255);				
						r=max(r,0);					
						dcMem.SetPixel(x,y,RGB(r,r,r));
					}
				}
				pDC->BitBlt(m_WndXOff,m_WndYOff,m_WndWidth,m_WnDHeight,&dcMem,0,0,SRCCOPY);
			}
		}	

	}
	
	if(buf)
	{
		delete buf;
	}
	
	if(ByteBuff)
	{
		delete  ByteBuff;
	}
	buf=NULL;
	
	ByteBuff=NULL;
}

void CImage::AdjustSaturationDraw(CDC *pDC, float Percentage)
{
//	if(Percentage>100.0 && Percentage<0 )
//		return;

	if(pDC==NULL)
		return;	
	if (!m_pDataset)
		return;
	
	if(m_DC.m_hDC>0)
		m_DC.Detach();
	m_DC.Attach(pDC->m_hDC);
	m_hdc=pDC->m_hDC;
	
	
	GDALRasterBand  *pBand=NULL;
	
	float* buf =NULL;

	float * ByteBuff=NULL;
	int x,y;
	
	
	CDC dcMem;
	dcMem.CreateCompatibleDC(pDC);
	CBitmap bitmap;
	bitmap.CreateCompatibleBitmap(pDC,m_WndWidth,m_WnDHeight);
	dcMem.SelectObject(&bitmap);
	
	int i;
	
	if(m_red==0 || m_red>m_bandNum) m_red=m_CurrentBand;
	if(m_blue==0 || m_blue>m_bandNum) m_blue=m_CurrentBand;
	if(m_green==0 || m_green>m_bandNum) m_green=m_CurrentBand;
	
	if(!IsGray())
	{
		ByteBuff = new float[m_buffHeight*m_buffWidth*3];		
		
		m_buffSize=m_buffHeight*m_buffWidth*3;
		buf = new float[m_buffHeight*m_buffWidth];
	
		
		pBand = m_pDataset->GetRasterBand(m_red);	
		if (pBand)
		{
			if (CE_None==pBand->RasterIO( GF_Read, m_xSrc,m_ySrc, m_srcWidth, m_srcHeight, buf, m_buffWidth,m_buffHeight, GDT_Float32, 0, 0 ))
			{
				
				for ( i = 0; i < m_buffHeight*m_buffWidth; i ++) 
					ByteBuff[3*i+2] = buf[i];
			}
		}	
		pBand = m_pDataset->GetRasterBand(m_green);
		if (pBand)
		{	
			if (CE_None==pBand->RasterIO( GF_Read, m_xSrc,m_ySrc, m_srcWidth, m_srcHeight, buf, m_buffWidth,m_buffHeight, GDT_Float32, 0, 0 ))
			{
			
				for ( i = 0; i <m_buffHeight*m_buffWidth; i ++) 
					ByteBuff[3*i+1] = buf[i];
			}
		}
		
		pBand = m_pDataset->GetRasterBand(m_blue);
		if (pBand)
		{	
			if (CE_None==pBand->RasterIO( GF_Read, m_xSrc,m_ySrc, m_srcWidth, m_srcHeight, buf, m_buffWidth,m_buffHeight, GDT_Float32, 0, 0 ))
			{
				
				for ( i = 0; i <m_buffHeight*m_buffWidth; i ++) 
					ByteBuff[3*i+0] = buf[i];
			}
		}
		float r,g,b ,H,L,S;
		for ( x = 0; x < m_buffWidth; x ++)
		{
			for ( y = 0; y < m_buffHeight; y ++) 
			{
				r=ByteBuff[(y*m_buffWidth+x)*3+2];
				g=ByteBuff[(y*m_buffWidth+x)*3+1];
				b=ByteBuff[(y*m_buffWidth+x)*3+0];
				RGBtoHSL( r,g,b,H,S,L);
				S = fabs(S*Percentage/100.0);
				S = min(S,1.0);
				if(long(Percentage)!=100)
					HLStoRGB(H,L,S,r,g,b);
				dcMem.SetPixel(x,y,RGB(r,g,b));
			}
		}
		pDC->BitBlt(m_WndXOff,m_WndYOff,m_WndWidth,m_WnDHeight,&dcMem,0,0,SRCCOPY);
	}

	if(buf)
	{
		delete buf;
	}

	if(ByteBuff)
	{
		delete  ByteBuff;
	}
	buf=NULL;

	ByteBuff=NULL;
}

void CImage::AdjustLuminosityDraw(CDC *pDC, float Percentage)
{
	//	if(Percentage>100.0 && Percentage<0 )
	//		return;
	
	if(pDC==NULL)
		return;	
	if (!m_pDataset)
		return;
	
	if(m_DC.m_hDC>0)
		m_DC.Detach();
	m_DC.Attach(pDC->m_hDC);
	m_hdc=pDC->m_hDC;
	
	
	GDALRasterBand  *pBand=NULL;
	
	float* buf =NULL;

	float * ByteBuff=NULL;
	int x,y;
	
	
	CDC dcMem;
	dcMem.CreateCompatibleDC(pDC);
	CBitmap bitmap;
	bitmap.CreateCompatibleBitmap(pDC,m_WndWidth,m_WnDHeight);
	dcMem.SelectObject(&bitmap);
	
	int i;
	
	if(m_red==0 || m_red>m_bandNum) m_red=m_CurrentBand;
	if(m_blue==0 || m_blue>m_bandNum) m_blue=m_CurrentBand;
	if(m_green==0 || m_green>m_bandNum) m_green=m_CurrentBand;
	
	if(!IsGray())
	{
		ByteBuff = new float[m_buffHeight*m_buffWidth*3];		
		
		m_buffSize=m_buffHeight*m_buffWidth*3;
		buf = new float[m_buffHeight*m_buffWidth];
	
		
		pBand = m_pDataset->GetRasterBand(m_red);	
		if (pBand)
		{
			if (CE_None==pBand->RasterIO( GF_Read, m_xSrc,m_ySrc, m_srcWidth, m_srcHeight, buf, m_buffWidth,m_buffHeight, GDT_Float32, 0, 0 ))
			{
				
				for ( i = 0; i < m_buffHeight*m_buffWidth; i ++) 
					ByteBuff[3*i+2] = buf[i];
			}
		}	
		pBand = m_pDataset->GetRasterBand(m_green);
		if (pBand)
		{	
			if (CE_None==pBand->RasterIO( GF_Read, m_xSrc,m_ySrc, m_srcWidth, m_srcHeight, buf, m_buffWidth,m_buffHeight, GDT_Float32, 0, 0 ))
			{
			
				for ( i = 0; i <m_buffHeight*m_buffWidth; i ++) 
					ByteBuff[3*i+1] = buf[i];
			}
		}
		
		pBand = m_pDataset->GetRasterBand(m_blue);
		if (pBand)
		{	
			if (CE_None==pBand->RasterIO( GF_Read, m_xSrc,m_ySrc, m_srcWidth, m_srcHeight, buf, m_buffWidth,m_buffHeight, GDT_Float32, 0, 0 ))
			{
				
				for ( i = 0; i <m_buffHeight*m_buffWidth; i ++) 
					ByteBuff[3*i+0] = buf[i];
			}
		}
		float r,g,b ,H,L,S;
		for ( x = 0; x < m_buffWidth; x ++)
		{
			for ( y = 0; y < m_buffHeight; y ++) 
			{
				r=ByteBuff[(y*m_buffWidth+x)*3+2];
				g=ByteBuff[(y*m_buffWidth+x)*3+1];
				b=ByteBuff[(y*m_buffWidth+x)*3+0];
				RGBtoHSL( r,g,b,H,S,L);
				L = max(0.0,L*Percentage/100.0);
			//	L = min(L,1.0);
				HLStoRGB(H,L,S,r,g,b);
				dcMem.SetPixel(x,y,RGB(r,g,b));
			}
		}
		pDC->BitBlt(m_WndXOff,m_WndYOff,m_WndWidth,m_WnDHeight,&dcMem,0,0,SRCCOPY);
	}

	if(buf)
	{
		delete buf;
	}

	if(ByteBuff)
	{
		delete  ByteBuff;
	}
	buf=NULL;

	ByteBuff=NULL;
}

void CImage::AdjustHSLDraw(CDC *pDC, float PercentHue, float PercentSaturation, float PercentLuminosity)
{
	//if(PercentHue>100.0 && PercentHue<0 && PercentSaturation>100.0 && PercentSaturation<0 && PercentLuminosity>100.0 && PercentLuminosity<0)
	//		return;
	
	if(pDC==NULL)
		return;	
	if (!m_pDataset)
		return;
	
	if(m_DC.m_hDC>0)
		m_DC.Detach();
	m_DC.Attach(pDC->m_hDC);
	m_hdc=pDC->m_hDC;
	
	
	GDALRasterBand  *pBand=NULL;
	
	float* buf =NULL;

	float * ByteBuff=NULL;
	int x,y;
	
	
	CDC dcMem;
	dcMem.CreateCompatibleDC(pDC);
	CBitmap bitmap;
	bitmap.CreateCompatibleBitmap(pDC,m_WndWidth,m_WnDHeight);
	dcMem.SelectObject(&bitmap);
	
	int i;
	
	if(m_red==0 || m_red>m_bandNum) m_red=m_CurrentBand;
	if(m_blue==0 || m_blue>m_bandNum) m_blue=m_CurrentBand;
	if(m_green==0 || m_green>m_bandNum) m_green=m_CurrentBand;
	
	if(!IsGray())
	{
		ByteBuff = new float[m_buffHeight*m_buffWidth*3];		
		
		m_buffSize=m_buffHeight*m_buffWidth*3;
		buf = new float[m_buffHeight*m_buffWidth];
	
		
		pBand = m_pDataset->GetRasterBand(m_red);	
		if (pBand)
		{
			if (CE_None==pBand->RasterIO( GF_Read, m_xSrc,m_ySrc, m_srcWidth, m_srcHeight, buf, m_buffWidth,m_buffHeight, GDT_Float32, 0, 0 ))
			{
				
				for ( i = 0; i < m_buffHeight*m_buffWidth; i ++) 
					ByteBuff[3*i+2] = buf[i];
			}
		}	
		pBand = m_pDataset->GetRasterBand(m_green);
		if (pBand)
		{	
			if (CE_None==pBand->RasterIO( GF_Read, m_xSrc,m_ySrc, m_srcWidth, m_srcHeight, buf, m_buffWidth,m_buffHeight, GDT_Float32, 0, 0 ))
			{
			
				for ( i = 0; i <m_buffHeight*m_buffWidth; i ++) 
					ByteBuff[3*i+1] = buf[i];
			}
		}
		
		pBand = m_pDataset->GetRasterBand(m_blue);
		if (pBand)
		{	
			if (CE_None==pBand->RasterIO( GF_Read, m_xSrc,m_ySrc, m_srcWidth, m_srcHeight, buf, m_buffWidth,m_buffHeight, GDT_Float32, 0, 0 ))
			{
				
				for ( i = 0; i <m_buffHeight*m_buffWidth; i ++) 
					ByteBuff[3*i+0] = buf[i];
			}
		}
		float r,g,b ,H,L,S;
		for ( x = 0; x < m_buffWidth; x ++)
		{
			for ( y = 0; y < m_buffHeight; y ++) 
			{
				r=ByteBuff[(y*m_buffWidth+x)*3+2];
				g=ByteBuff[(y*m_buffWidth+x)*3+1];
				b=ByteBuff[(y*m_buffWidth+x)*3+0];
				RGBtoHSL( r,g,b,H,S,L);
				S = (S*PercentSaturation/100.0);
				H = (H*PercentHue/100.0);
				L = (L*PercentLuminosity/100.0);
				HLStoRGB(H,L,S,r,g,b);
				dcMem.SetPixel(x,y,RGB(r,g,b));
			}
		}
		pDC->BitBlt(m_WndXOff,m_WndYOff,m_WndWidth,m_WnDHeight,&dcMem,0,0,SRCCOPY);
	}

	if(buf)
	{
		delete buf;
	}

	if(ByteBuff)
	{
		delete  ByteBuff;
	}
	buf=NULL;

	ByteBuff=NULL;
}

void CImage::WaterDetectFunction(long width, long height, float *InBuff, float *OutBuff)
{
	//GDALRasterBand  *pBand=NULL;
	//
	//float* buf =NULL;

	//float * ByteBuff=NULL;
	//ByteBuff = new float[m_buffHeight*m_buffWidth*3];		

	//m_buffSize=m_buffHeight*m_buffWidth*3;
	//buf = new float[m_buffHeight*m_buffWidth];

	//
	//pBand = m_pDataset->GetRasterBand(m_red);
	//if (pBand)
	//{
	//	if (CE_None==pBand->RasterIO( GF_Read, m_xSrc,m_ySrc, m_srcWidth, m_srcHeight, buf, m_buffWidth,m_buffHeight, GDT_Float32, 0, 0 ))
	//	{
	//		
	//		for ( i = 0; i < m_buffHeight*m_buffWidth; i ++) 	



	//								if(iB03[j] > iB04[j] && iB04[j] > iB01[j] && iB01[j] > iB02[j] && iB02[j] > iB07[j])
	//					iOutputFlag[iDestPos] = 1; //判断水

}