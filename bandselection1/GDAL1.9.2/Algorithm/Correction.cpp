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
	GDALDataset* pImgDataset=NULL;//Ҫ������Ӱ��
	GDALDataset* pDEMDataset=NULL;//DEMӰ��
	pDEMDataset = (GDALDataset *) GDALOpen(DEMFile,GA_ReadOnly);//DEM��ֻ����ʽ��
	pImgDataset = (GDALDataset *) GDALOpen(imgFile,GA_ReadOnly);//

	if (pImgDataset==NULL && pDEMDataset==NULL)
		return false;

	//�����������Ӱ��, ��Ӱ��
	long  width = pImgDataset->GetRasterXSize();
    long  height = pImgDataset->GetRasterYSize();
	
	int nBand=pImgDataset->GetRasterCount();
	const char *  pszFormat = pImgDataset->GetDriver()->GetDescription();
    GDALDriver *poDriver;  
    poDriver = GetGDALDriverManager()->GetDriverByName(pszFormat);
	GDALDataset *poDstDS; //�������Ӱ��
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
//���DEM�ķֱ���
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
	poBand=pDEMDataset->GetRasterBand(1);//��ȡDEM����
	poBand->RasterIO( GF_Read, 0, 0, DemWidth, DemHeight, DemBuff, DemWidth, DemHeight, GDT_Float32,  0, 0 );

	double geoX=0.0;
	double geoY=0.0;

	long fx=0,fy=0;//DEM���ļ�����
	double x,y;
	double dY,dX;//DEM�е��ݶ�
	double A,S,cosi;
	for(int k=1;k<=nBand;k++)//����ÿһ�����ηֱ��� ���k��Сд��
	{
		poBand=pImgDataset->GetRasterBand(k);//���k��Сд��
		poBandNew=poDstDS->GetRasterBand(k);//���k��Сд��
		poBand->RasterIO( GF_Read, 0, 0, width, height, InBuff, width, height, GDT_Float32,  0, 0 );
		
		for( int i=0;i<height;i++)//���ڳ�������Ԫ�����Ԫ
		{
			for( int j=0;j<width;j++)
			{			
				OutBuff[i*width+j]=InBuff[i*width+j];//��ʼ����Ԫ��ֵ
				geoX = adfGeoTransform[0] + j*adfGeoTransform[1] + i*adfGeoTransform[2];
				geoY = adfGeoTransform[3] + j*adfGeoTransform[4] + i*adfGeoTransform[5];				
							
				y=((adfGeoTransformDEM[4]*geoX-adfGeoTransformDEM[1]*geoY)-(adfGeoTransformDEM[4]*adfGeoTransformDEM[0]-adfGeoTransformDEM[1]*adfGeoTransformDEM[3]))/(adfGeoTransformDEM[4]*adfGeoTransformDEM[2]-adfGeoTransformDEM[1]*adfGeoTransformDEM[5]);
				fx= y-long(y)>0.5?long(y)+1:long(y);// ��
				x=((adfGeoTransformDEM[5]*geoX-adfGeoTransformDEM[2]*geoY)-(adfGeoTransformDEM[0]*adfGeoTransformDEM[5]-adfGeoTransformDEM[3]*adfGeoTransformDEM[2]))/(adfGeoTransformDEM[1]*adfGeoTransformDEM[5]-adfGeoTransformDEM[4]*adfGeoTransformDEM[2]);
				fy=x-long(x)>0.5?long(x)+1:long(x);//��
				//�Դ�����Ӱ�������(i,j)��Ӧ��DEM������(fx,fy)����Ϊ�ļ�����
				if(fx>0 && fx<DemHeight-1 && fy>0 && fy<DemWidth-1)//ȷ������DEMӰ���ڣ�û�����ڷ�Χ�ڵĲ��봦��
				{
					dY=(DemBuff[fx,fy+1]-DemBuff[fx,fy-1])/(2*DEMResolutionY);//��ʽ7.6  ���߳���Y������ݶ�
					dX=(DemBuff[fx+1,fy]-DemBuff[fx-1,fy])/(2*DEMResolutionX);//��ʽ7.5  ���߳���X������ݶ�
					S=atan(sqrt(dY*dY+dX*dX));//��ʽ7.4
					A=atan(dY/dX);//��ʽ7.3
					//SolerAzimuthAngle���춥��,  SolarZenithAngle:��λ��
					cosi=cos(SolarZenithAngle)*cos(S)+sin(S)*sin(SolarZenithAngle)*cos(SolerAzimuthAngle-A);//��ʽ7.2
					if(cosi!=0.0)
					{				
						OutBuff[i*width+j]=InBuff[i*width+j]*pow((cos(SolarZenithAngle)/cosi),K);//��ʽ7.1,���K�Ǵ�д��
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
