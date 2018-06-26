// Correction.cpp: implementation of the CCorrection class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Correction.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CCorrection::CCorrection()
{
	GDALAllRegister();
}

CCorrection::~CCorrection()
{

}

bool CCorrection::ConsineMethods(CString imgFile, CString DEMFile, CString resultFile, double SolerAzimuthAngle, double SolarZenithAngle,double K)
{
	GDALDataset* pImgDataset=NULL;//要纠正的影像
	GDALDataset* pDEMDataset=NULL;//DEM影像
	pDEMDataset = (GDALDataset *) GDALOpen(DEMFile,GA_ReadOnly);//DEM以只读方式打开
	pImgDataset = (GDALDataset *) GDALOpen(imgFile,GA_ReadOnly);//

	if (pImgDataset==NULL && pDEMDataset==NULL)
		return false;

	//创建纠正后的影像, 空影像
	long  width = pImgDataset->GetRasterXSize();
    long  height = pImgDataset->GetRasterYSize();
	
	int nBand=pImgDataset->GetRasterCount();
	const char *  pszFormat = pImgDataset->GetDriver()->GetDescription();
    GDALDriver *poDriver;  
    poDriver = GetGDALDriverManager()->GetDriverByName(pszFormat);
	GDALDataset *poDstDS; //纠正后的影像
	char **papszOptions = NULL;    
    poDstDS = poDriver->Create(resultFile, width, height, nBand, GDT_Float32 , 
		papszOptions );
	if(poDstDS==NULL)
		return false;
	
	double    adfGeoTransform[6];
    CPLErr hr=pImgDataset->GetGeoTransform( adfGeoTransform );
	if(hr==CE_None)
	{
		poDstDS->SetGeoTransform( adfGeoTransform );
	}
	
	const char  * strPJ=pImgDataset->GetProjectionRef();
	CString CStrPJ=strPJ;
	if(CStrPJ!="")
    {
		poDstDS->SetProjection( strPJ );
	}
////////
//获得DEM的分辨率
	double    adfGeoTransformDEM[6];
	hr=pDEMDataset->GetGeoTransform( adfGeoTransformDEM );
	double 	DEMResolutionX;
	double 	DEMResolutionY;
	DEMResolutionX=adfGeoTransformDEM[1];
	DEMResolutionY=-adfGeoTransformDEM[5] ;
	if(hr!=CE_None)
	{
		return false;
	}
	DEMResolutionX=fabs(adfGeoTransformDEM[1]);
	DEMResolutionY=fabs(adfGeoTransformDEM[5]);
	
	GDALRasterBand  *poBand;
	GDALRasterBand *poBandNew;

	float *InBuff=new float[width*height];
	float *OutBuff=new float[width*height];
	long  DemWidth = pDEMDataset->GetRasterXSize();
    long  DemHeight = pDEMDataset->GetRasterYSize();

 	float *DemBuff=new float[DemWidth* DemHeight];
	poBand=pDEMDataset->GetRasterBand(1);//读取DEM数据
	poBand->RasterIO( GF_Read, 0, 0, DemWidth, DemHeight, DemBuff, DemWidth, DemHeight, GDT_Float32,  0, 0 );

	double geoX=0.0;
	double geoY=0.0;

	long fx=0,fy=0;//DEM的文件坐标
	double x,y;
	double dY,dX;//DEM中的梯度
	double A,S,cosi;
	for(int k=1;k<=nBand;k++)//对于每一个波段分别处理 这个k是小写的
	{
		poBand=pImgDataset->GetRasterBand(k);//这个k是小写的
		poBandNew=poDstDS->GetRasterBand(k);//这个k是小写的
		poBand->RasterIO( GF_Read, 0, 0, width, height, InBuff, width, height, GDT_Float32,  0, 0 );
		
		for( int i=0;i<height;i++)//对于除中心像元外的像元
		{
			for( int j=0;j<width;j++)
			{			
				OutBuff[i*width+j]=InBuff[i*width+j];//初始化像元的值
				geoX = adfGeoTransform[0] + j*adfGeoTransform[1] + i*adfGeoTransform[2];
				geoY = adfGeoTransform[3] + j*adfGeoTransform[4] + i*adfGeoTransform[5];				
							
				y=((adfGeoTransformDEM[4]*geoX-adfGeoTransformDEM[1]*geoY)-(adfGeoTransformDEM[4]*adfGeoTransformDEM[0]-adfGeoTransformDEM[1]*adfGeoTransformDEM[3]))/(adfGeoTransformDEM[4]*adfGeoTransformDEM[2]-adfGeoTransformDEM[1]*adfGeoTransformDEM[5]);
				fx= y-long(y)>0.5?long(y)+1:long(y);// 行
				x=((adfGeoTransformDEM[5]*geoX-adfGeoTransformDEM[2]*geoY)-(adfGeoTransformDEM[0]*adfGeoTransformDEM[5]-adfGeoTransformDEM[3]*adfGeoTransformDEM[2]))/(adfGeoTransformDEM[1]*adfGeoTransformDEM[5]-adfGeoTransformDEM[4]*adfGeoTransformDEM[2]);
				fy=x-long(x)>0.5?long(x)+1:long(x);//列
				//对代纠正影像的象素(i,j)对应着DEM的象素(fx,fy)，均为文件坐标
				if(fx>0 && fx<DemHeight-1 && fy>0 && fy<DemWidth-1)//确保落在DEM影像内，没有落在范围内的不与处理
				{
					dY=(DemBuff[fx,fy+1]-DemBuff[fx,fy-1])/(2*DEMResolutionY);//公式7.6  ；高程在Y方向的梯度
					dX=(DemBuff[fx+1,fy]-DemBuff[fx-1,fy])/(2*DEMResolutionX);//公式7.5  ；高程在X方向的梯度
					S=atan(sqrt(dY*dY+dX*dX));//公式7.4
					A=atan(dY/dX);//公式7.3
					//SolerAzimuthAngle：天顶角,  SolarZenithAngle:方位角
					cosi=cos(SolarZenithAngle)*cos(S)+sin(S)*sin(SolarZenithAngle)*cos(SolerAzimuthAngle-A);//公式7.2
					if(cosi!=0.0)
					{				
						OutBuff[i*width+j]=InBuff[i*width+j]*pow((cos(SolarZenithAngle)/cosi),K);//公式7.1,这个K是大写的
					}
				}
			
			}
		}
		poBandNew->RasterIO( GF_Write, 0, 0, width, height, OutBuff, width, height, GDT_Float32, 0, 0 );
	}

////////////
	if(InBuff)
	{
		delete InBuff;
		InBuff=NULL;
	}
	if(DemBuff)
	{
		delete DemBuff;
		DemBuff=NULL;
	}
	if(OutBuff)
	{
		delete OutBuff;
		OutBuff=NULL;
	}
	if(pImgDataset)
	{
		delete pImgDataset;
		pImgDataset=NULL;
	}
	if(pDEMDataset)
	{
		delete pDEMDataset;
		pDEMDataset=NULL;
	}
	
	return true;

}
