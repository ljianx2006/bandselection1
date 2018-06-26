// currentFilter.cpp: implementation of the Filter class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Filter.h"
#include <comdef.h>
#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

Filter::Filter()
{

}

Filter::~Filter()
{

}
void Filter::SetFilter33(double* filter33)
{
	for(int i=0;i<9;i++)
	{
		m_filter33[i]=filter33[i];
	}
}

void Filter::filter33(BSTR sourceFile, BSTR targetFile)
{
		GDALAllRegister();
	GDALDataset  *poDataset;
	poDataset = (GDALDataset *) GDALOpen( _bstr_t(sourceFile), GA_ReadOnly );//GA_ReadOnly or GA_Update
	if(poDataset==NULL)
	{
		AfxMessageBox((LPCTSTR)"faild to open file");
		exit(0);
	}
	long  width = poDataset->GetRasterXSize();
    long  height = poDataset->GetRasterYSize();
	float *InBuff=new float[width*height];
	float *OutBuff=new float[width*height];
	long i,j;
	for( i=0;i<width;i++)//处理中心区域像元
	{
		for( j=0;j<height;j++)
		{	
			OutBuff[i*height+j]=0;
			InBuff[i*height+j]=0;
		}
	}
	int nBand=poDataset->GetRasterCount();
	const char *  pszFormat = poDataset->GetDriver()->GetDescription();
    GDALDriver *poDriver;  
    poDriver = GetGDALDriverManager()->GetDriverByName(pszFormat);
	GDALDataset *poDstDS; 
	char **papszOptions = NULL;    
    poDstDS = poDriver->Create(_bstr_t(targetFile), width, height, nBand, GDT_Float32 , 
		papszOptions );
	
	double    adfGeoTransform[6];
    CPLErr hr=poDataset->GetGeoTransform( adfGeoTransform );
	if(hr==CE_None)
	{
		poDstDS->SetGeoTransform( adfGeoTransform );
	}

	const char  * strPJ=poDataset->GetProjectionRef();
	CString CStrPJ=strPJ;
	if(CStrPJ!="")
    {
		poDstDS->SetProjection( strPJ );
	}
	
	GDALRasterBand  *poBand;
	GDALRasterBand *poBandNew;
	////////////////////////
	//////////////////////////
	double sum=0;
	int jj=0;
	
	for(int k=1;k<=nBand;k++)
	{
		poBand=poDataset->GetRasterBand(k);
		poBandNew=poDstDS->GetRasterBand(k);
		poBand->RasterIO( GF_Read, 0, 0, width, height, InBuff, width, height, GDT_Float32,  0, 0 );
		//double filter[9];
		
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
	
		
        
		poBandNew->RasterIO( GF_Write, 0, 0, width, height, OutBuff, width, height, GDT_Float32, 0, 0 );
		
	}
	if( poDstDS != NULL )
	{
		delete poDstDS;
		poDstDS=NULL;
	}
	if(poDataset!=NULL)
	{
		delete poDataset;
		poDataset=NULL;
	}
	
	delete []InBuff;
	delete []OutBuff;
}

void Filter::avgFilter(BSTR sourceFile, BSTR targetFile)
{
		double ff[9]={1,1,1,
					  1,1,1,
			          1,1,1};
		this->SetFilter33(ff);
		this->filter33(sourceFile,targetFile);

}

void Filter::midFilter(BSTR sourceFile, BSTR targetFile)
{
		GDALAllRegister();
	GDALDataset  *poDataset;
	poDataset = (GDALDataset *) GDALOpen( _bstr_t(sourceFile), GA_ReadOnly );//GA_ReadOnly or GA_Update
	if(poDataset==NULL)
	{
		AfxMessageBox("faild to open file");
		exit(0);
	}
	long  width = poDataset->GetRasterXSize();
    long  height = poDataset->GetRasterYSize();
	float *InBuff=new float[width*height];
	float *OutBuff=new float[width*height];
	long i,j;
	for( i=0;i<width;i++)//处理中心区域像元
	{
		for( j=0;j<height;j++)
		{	
			OutBuff[i*height+j]=0;
			InBuff[i*height+j]=0;
		}
	}
	int nBand=poDataset->GetRasterCount();
	const char *  pszFormat = poDataset->GetDriver()->GetDescription();
    GDALDriver *poDriver;  
    poDriver = GetGDALDriverManager()->GetDriverByName(pszFormat);
	GDALDataset *poDstDS; 
	char **papszOptions = NULL;    
    poDstDS = poDriver->Create(_bstr_t(targetFile), width, height, nBand, GDT_Float32 , 
		papszOptions );
	
	double    adfGeoTransform[6];
    CPLErr hr=poDataset->GetGeoTransform( adfGeoTransform );
	if(hr==CE_None)
	{
		poDstDS->SetGeoTransform( adfGeoTransform );
	}

	const char  * strPJ=poDataset->GetProjectionRef();
	 CString CStrPJ=strPJ;
	if(CStrPJ!="")
    {
		poDstDS->SetProjection( strPJ );
	}
	
	GDALRasterBand  *poBand;
	GDALRasterBand *poBandNew;
	////////////////////////
	//////////////////////////
	double sum=0;
	int jj=0;
	double filter[9];
	
	for(int k=1;k<=nBand;k++)
	{
		poBand=poDataset->GetRasterBand(k);
		poBandNew=poDstDS->GetRasterBand(k);
		poBand->RasterIO( GF_Read, 0, 0, width, height, InBuff, width, height, GDT_Float32,  0, 0 );
		//double filter[9];
		
		
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
	
		
        
		poBandNew->RasterIO( GF_Write, 0, 0, width, height, OutBuff, width, height, GDT_Float32, 0, 0 );
		
	}
	if( poDstDS != NULL )
	{
		delete poDstDS;
		poDstDS=NULL;
	}
	if(poDataset!=NULL)
	{
		delete poDataset;
		poDataset=NULL;
	}
	
	delete []InBuff;
	delete []OutBuff;

}


void Filter::prbub(int n, double p[])  
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

void Filter::robertsFilter(BSTR sourceFile, BSTR targetFile)
{
		GDALAllRegister();
	GDALDataset  *poDataset;
	poDataset = (GDALDataset *) GDALOpen( _bstr_t(sourceFile), GA_ReadOnly );//GA_ReadOnly or GA_Update
	if(poDataset==NULL)
	{
		AfxMessageBox("faild to open file");
		exit(0);
	}
	long  width = poDataset->GetRasterXSize();
    long  height = poDataset->GetRasterYSize();
	float *InBuff=new float[width*height];
	float *OutBuff=new float[width*height];
	long i,j;
	for( i=0;i<width;i++)//处理中心区域像元
	{
		for( j=0;j<height;j++)
		{	
			OutBuff[i*height+j]=0;
			InBuff[i*height+j]=0;
		}
	}
	int nBand=poDataset->GetRasterCount();
	const char *  pszFormat = poDataset->GetDriver()->GetDescription();
    GDALDriver *poDriver;  
    poDriver = GetGDALDriverManager()->GetDriverByName(pszFormat);
	GDALDataset *poDstDS; 
	char **papszOptions = NULL;    
    poDstDS = poDriver->Create(_bstr_t(targetFile), width, height, nBand, GDT_Float32 , 
		papszOptions );
	
	double    adfGeoTransform[6];
    CPLErr hr=poDataset->GetGeoTransform( adfGeoTransform );
	if(hr==CE_None)
	{
		poDstDS->SetGeoTransform( adfGeoTransform );
	}

	const char  * strPJ=poDataset->GetProjectionRef();
	 CString CStrPJ=strPJ;
	if(CStrPJ!="")
    {
		poDstDS->SetProjection( strPJ );
	}
	
	GDALRasterBand  *poBand;
	GDALRasterBand *poBandNew;
	////////////////////////
	//////////////////////////
	double sum=0;
	int jj=0;
	
	for(int k=1;k<=nBand;k++)
	{
		poBand=poDataset->GetRasterBand(k);
		poBandNew=poDstDS->GetRasterBand(k);
		poBand->RasterIO( GF_Read, 0, 0, width, height, InBuff, width, height, GDT_Float32,  0, 0 );
	
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
	
		
        
		poBandNew->RasterIO( GF_Write, 0, 0, width, height, OutBuff, width, height, GDT_Float32, 0, 0 );
		
	}
	if( poDstDS != NULL )
	{
		delete poDstDS;
		poDstDS=NULL;
	}
	if(poDataset!=NULL)
	{
		delete poDataset;
		poDataset=NULL;
	}
	
	delete []InBuff;
	delete []OutBuff;
}

void Filter::PrewittFilter(BSTR sourceFile, BSTR targetFile)
{

			GDALAllRegister();
	GDALDataset  *poDataset;
	poDataset = (GDALDataset *) GDALOpen( _bstr_t(sourceFile), GA_ReadOnly );//GA_ReadOnly or GA_Update
	if(poDataset==NULL)
	{
		AfxMessageBox("faild to open file");
		exit(0);
	}
	long  width = poDataset->GetRasterXSize();
    long  height = poDataset->GetRasterYSize();
	float *InBuff=new float[width*height];
	float *OutBuff=new float[width*height];
	long i,j;
	for( i=0;i<width;i++)//处理中心区域像元
	{
		for( j=0;j<height;j++)
		{	
			OutBuff[i*height+j]=0;
			InBuff[i*height+j]=0;
		}
	}
	int nBand=poDataset->GetRasterCount();
	const char *  pszFormat = poDataset->GetDriver()->GetDescription();
    GDALDriver *poDriver;  
    poDriver = GetGDALDriverManager()->GetDriverByName(pszFormat);
	GDALDataset *poDstDS; 
	char **papszOptions = NULL;    
    poDstDS = poDriver->Create(_bstr_t(targetFile), width, height, nBand, GDT_Float32 , 
		papszOptions );
	
	double    adfGeoTransform[6];
    CPLErr hr=poDataset->GetGeoTransform( adfGeoTransform );
	if(hr==CE_None)
	{
		poDstDS->SetGeoTransform( adfGeoTransform );
	}

	const char  * strPJ=poDataset->GetProjectionRef();
	 CString CStrPJ=strPJ;
	if(CStrPJ!="")
    {
		poDstDS->SetProjection( strPJ );
	}
	
	GDALRasterBand  *poBand;
	GDALRasterBand *poBandNew;
	////////////////////////
	//////////////////////////
	double sum=0;
	int jj=0;
	double filter33[9];
	double t1,t2;
	for(int k=1;k<=nBand;k++)
	{
		poBand=poDataset->GetRasterBand(k);
		poBandNew=poDstDS->GetRasterBand(k);
		poBand->RasterIO( GF_Read, 0, 0, width, height, InBuff, width, height, GDT_Float32,  0, 0 );
	
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
	
		
        
		poBandNew->RasterIO( GF_Write, 0, 0, width, height, OutBuff, width, height, GDT_Float32, 0, 0 );
		
	}
	if( poDstDS != NULL )
	{
		delete poDstDS;
		poDstDS=NULL;
	}
	if(poDataset!=NULL)
	{
		delete poDataset;
		poDataset=NULL;
	}
	
	delete []InBuff;
	delete []OutBuff;
}

void Filter::SobelFilter(BSTR sourceFile, BSTR targetFile)
{
	GDALAllRegister();
	GDALDataset  *poDataset;
	poDataset = (GDALDataset *) GDALOpen( _bstr_t(sourceFile), GA_ReadOnly );//GA_ReadOnly or GA_Update
	if(poDataset==NULL)
	{
		AfxMessageBox("faild to open file");
		exit(0);
	}
	long  width = poDataset->GetRasterXSize();
    long  height = poDataset->GetRasterYSize();
	float *InBuff=new float[width*height];
	float *OutBuff=new float[width*height];
	long i,j;
	for( i=0;i<width;i++)//处理中心区域像元
	{
		for( j=0;j<height;j++)
		{	
			OutBuff[i*height+j]=0;
			InBuff[i*height+j]=0;
		}
	}
	int nBand=poDataset->GetRasterCount();
	const char *  pszFormat = poDataset->GetDriver()->GetDescription();
    GDALDriver *poDriver;  
    poDriver = GetGDALDriverManager()->GetDriverByName(pszFormat);
	GDALDataset *poDstDS; 
	char **papszOptions = NULL;    
    poDstDS = poDriver->Create(_bstr_t(targetFile), width, height, nBand, GDT_Float32 , 
		papszOptions );
	
	double    adfGeoTransform[6];
    CPLErr hr=poDataset->GetGeoTransform( adfGeoTransform );
	if(hr==CE_None)
	{
		poDstDS->SetGeoTransform( adfGeoTransform );
	}

	const char  * strPJ=poDataset->GetProjectionRef();
	 CString CStrPJ=strPJ;
	if(CStrPJ!="")
    {
		poDstDS->SetProjection( strPJ );
	}
	
	GDALRasterBand  *poBand;
	GDALRasterBand *poBandNew;
	////////////////////////
	//////////////////////////
	double sum=0;
	int jj=0;
	double filter33[9];
	double t1,t2;
	for(int k=1;k<=nBand;k++)
	{
		poBand=poDataset->GetRasterBand(k);
		poBandNew=poDstDS->GetRasterBand(k);
		poBand->RasterIO( GF_Read, 0, 0, width, height, InBuff, width, height, GDT_Float32,  0, 0 );
	
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
	
		
        
		poBandNew->RasterIO( GF_Write, 0, 0, width, height, OutBuff, width, height, GDT_Float32, 0, 0 );
		
	}
	if( poDstDS != NULL )
	{
		delete poDstDS;
		poDstDS=NULL;
	}
	if(poDataset!=NULL)
	{
		delete poDataset;
		poDataset=NULL;
	}
	
	delete []InBuff;
	delete []OutBuff;

}

void Filter::LaplaceFilter(BSTR sourceFile, BSTR targetFile)
{
		double ff[9]={0,1,0,
					  1,-4,1,
			          0,1,0};
		this->SetFilter33(ff);
		this->filter33(sourceFile,targetFile);
}

void Filter::verticallyDetect(BSTR sourceFile, BSTR targetFile)
{
	double ff[9]={-1,0,1,
				 -1,0,1,
			       -1,0,1};
		this->SetFilter33(ff);
		this->filter33(sourceFile,targetFile);

}

void Filter::horizontallyDetect(BSTR sourceFile, BSTR targetFile)
{
	double ff[9]={-1,-1,-1,
				 0,0,0,
			       1,1,1};
		this->SetFilter33(ff);
		this->filter33(sourceFile,targetFile);

}

void Filter::diagonallyDetect(BSTR sourceFile, BSTR targetFile)
{
		double ff[9]={0,1,1,
						-1,0,1,
						-1,-1,0};
		this->SetFilter33(ff);
		this->filter33(sourceFile,targetFile);

}

void Filter::percentFilter(BSTR sourceFile, BSTR targetFile)
{
	GDALAllRegister();
	GDALDataset  *poDataset;
	poDataset = (GDALDataset *) GDALOpen( _bstr_t(sourceFile), GA_ReadOnly );//GA_ReadOnly or GA_Update
	if(poDataset==NULL)
	{
		AfxMessageBox("faild to open file");
		exit(0);
	}
	long  width = poDataset->GetRasterXSize();
    long  height = poDataset->GetRasterYSize();
	float *InBuff=new float[width*height];
	float *OutBuff=new float[width*height];
	long i,j;
	for( i=0;i<width;i++)//处理中心区域像元
	{
		for( j=0;j<height;j++)
		{	
			OutBuff[i*height+j]=10;
			InBuff[i*height+j]=10;
		}
	}
	int nBand=poDataset->GetRasterCount();
	const char *  pszFormat = poDataset->GetDriver()->GetDescription();
    GDALDriver *poDriver;  
    poDriver = GetGDALDriverManager()->GetDriverByName(pszFormat);
	GDALDataset *poDstDS; 
	char **papszOptions = NULL;    
    poDstDS = poDriver->Create(_bstr_t(targetFile), width, height, nBand, GDT_Float32 , 
		papszOptions );
	
	double    adfGeoTransform[6];
    CPLErr hr=poDataset->GetGeoTransform( adfGeoTransform );
	if(hr==CE_None)
	{
		poDstDS->SetGeoTransform( adfGeoTransform );
	}

	const char  * strPJ=poDataset->GetProjectionRef();
	 CString CStrPJ=strPJ;
	if(CStrPJ!="")
    {
		poDstDS->SetProjection( strPJ );
	}
	
	GDALRasterBand  *poBand;
	GDALRasterBand *poBandNew;
	////////////////////////
	//////////////////////////
	double sum=0;
	int jj=0;
	float  filter[9];
	
	for(int k=1;k<=nBand;k++)
	{
		poBand=poDataset->GetRasterBand(k);
		poBandNew=poDstDS->GetRasterBand(k);
		poBand->RasterIO( GF_Read, 0, 0, width, height, InBuff, width, height, GDT_Float32,  0, 0 );
		//double filter[9];
		
		
		for( i=0;i<height;i++)//处理中心区域像元
		{
			for( j=0;j<width;j++)
			{
				
				if(i!=0&&i!=height-1&&j!=0&&j!=width-1)
				{
					///*******************************************
					//动态生成滤波器filer
			
						//为模板初始化
						filter[0]=InBuff[(i-1)*width+(j-1)];
						filter[1]=InBuff[(i-1)*width+(j)];
						filter[2]=InBuff[(i-1)*width+(j+1)];
						filter[3]=InBuff[(i)*width+(j-1)];
						filter[4]=InBuff[(i)*width+(j)];
						filter[5]=InBuff[(i)*width+(j+1)];
						filter[6]=InBuff[(i+1)*width+(j-1)];
						filter[7]=InBuff[(i+1)*width+(j)];
						filter[8]=InBuff[(i+1)*width+(j+1)];
					
						//与中心像元之差
						filter[0]-=filter[4];
						filter[1]-=filter[4];
						filter[2]-=filter[4];
						filter[3]-=filter[4];
						filter[4]-=filter[4];
						filter[5]-=filter[4];
						filter[6]-=filter[4];
						filter[7]-=filter[4];
						filter[8]-=filter[4];	
						
						//差的平方
						filter[0]*=filter[0];
						filter[1]*=filter[1];
						filter[2]*=filter[2];
						filter[3]*=filter[3];
						filter[4]*=filter[4];
						filter[5]*=filter[5];
						filter[6]*=filter[6];
						filter[7]*=filter[7];
						filter[8]*=filter[8];

						//平方和
						for(jj=0;jj<9;jj++)
						{
							sum+=filter[jj];
						}
						if(sum==0)
							sum=filter[4];
						//用距离的比例作为模板
						filter[0]=filter[0]/sum;
						filter[1]=filter[1]/sum;
						filter[2]=filter[2]/sum;
						filter[3]=filter[3]/sum;
						filter[4]=filter[4]/sum;
						filter[5]=filter[5]/sum;
						filter[6]=filter[6]/sum;
						filter[7]=filter[7]/sum;
						filter[8]=filter[8]/sum;
						
				
					
					OutBuff[i*width+j]=(InBuff[(i-1)*width+(j-1)]*m_filter33[0] +		InBuff[(i-1)*width+(j)]*m_filter33[1]+		InBuff[(i-1)*width+(j+1)]*m_filter33[2]
					                   +InBuff[(i)*width+(j-1)]  *m_filter33[3] +													InBuff[(i)*width+(j+1)]*m_filter33[5]
								       +InBuff[(i+1)*width+(j-1)]*m_filter33[6] +		InBuff[(i+1)*width+(j)]*m_filter33[7]+		InBuff[(i+1)*width+(j+1)]*m_filter33[8]);
			 		if(OutBuff[i*width+j]==0)
					{
						OutBuff[i*width+j]=InBuff[i*width+j];
					}				
				}
			}
		}
		poBandNew->RasterIO( GF_Write, 0, 0, width, height, OutBuff, width, height, GDT_Float32, 0, 0 );
		
	}
	if( poDstDS != NULL )
	{
		delete poDstDS;
		poDstDS=NULL;
	}
	if(poDataset!=NULL)
	{
		delete poDataset;
		poDataset=NULL;
	}
	
	delete []InBuff;
	delete []OutBuff;

}

void Filter::ratioFilter(BSTR sourceFile, BSTR targetFile)
{
	GDALAllRegister();
	GDALDataset  *poDataset;
	poDataset = (GDALDataset *) GDALOpen( _bstr_t(sourceFile), GA_ReadOnly );//GA_ReadOnly or GA_Update
	if(poDataset==NULL)
	{
		AfxMessageBox("faild to open file");
		exit(0);
	}
	long  width = poDataset->GetRasterXSize();
    long  height = poDataset->GetRasterYSize();
	float *InBuff=new float[width*height];
	float *OutBuff=new float[width*height];
	long i,j;
	for( i=0;i<width;i++)//处理中心区域像元
	{
		for( j=0;j<height;j++)
		{	
			OutBuff[i*height+j]=0;
			InBuff[i*height+j]=0;
		}
	}
	int nBand=poDataset->GetRasterCount();
	const char *  pszFormat = poDataset->GetDriver()->GetDescription();
    GDALDriver *poDriver;  
    poDriver = GetGDALDriverManager()->GetDriverByName(pszFormat);
	GDALDataset *poDstDS; 
	char **papszOptions = NULL;    
    poDstDS = poDriver->Create(_bstr_t(targetFile), width, height, nBand, GDT_Float32 , 
		papszOptions );
	
	double    adfGeoTransform[6];
    CPLErr hr=poDataset->GetGeoTransform( adfGeoTransform );
	if(hr==CE_None)
	{
		poDstDS->SetGeoTransform( adfGeoTransform );
	}

	const char  * strPJ=poDataset->GetProjectionRef();
	 CString CStrPJ=strPJ;
	if(CStrPJ!="")
    {
		poDstDS->SetProjection( strPJ );
	}
	
	GDALRasterBand  *poBand;
	GDALRasterBand *poBandNew;
	////////////////////////
	//////////////////////////
	double sum=0;
	int jj=0;
	double  filter[9];
	
	for(int k=1;k<=nBand;k++)
	{
		poBand=poDataset->GetRasterBand(k);
		poBandNew=poDstDS->GetRasterBand(k);
		poBand->RasterIO( GF_Read, 0, 0, width, height, InBuff, width, height, GDT_Float32,  0, 0 );
		//double filter[9];
		
		
		for( i=0;i<height;i++)//处理中心区域像元
		{
			for( j=0;j<width;j++)
			{
				if(i!=0&&i!=height-1&&j!=0&&j!=width-1)
				{
					///*******************************************
					//动态生成滤波器filer
		
				
						filter[0]=InBuff[(i-1)*width+(j-1)];
						filter[1]=InBuff[(i-1)*width+(j)];
						filter[2]=InBuff[(i-1)*width+(j+1)];
						filter[3]=InBuff[(i)*width+(j-1)];
						filter[4]=InBuff[(i)*width+(j)];
						filter[5]=InBuff[(i)*width+(j+1)];
						filter[6]=InBuff[(i+1)*width+(j-1)];
						filter[7]=InBuff[(i+1)*width+(j)];
						filter[8]=InBuff[(i+1)*width+(j+1)];

					
						// 细部和纹理非常清晰
						filter[0]=filter[4]/filter[0];
						filter[1]=filter[4]/filter[1];
						filter[2]=filter[4]/filter[2];
						filter[3]=filter[4]/filter[3];
						filter[4]=filter[4]/filter[4];
						filter[5]=filter[4]/filter[5];
						filter[6]=filter[4]/filter[6];
						filter[7]=filter[4]/filter[7];
						filter[8]=filter[4]/filter[8];
				

						//////////////////////////////////
						///////////////////////////////////
						for(jj=0;jj<9;jj++)
						{
							sum+=filter[jj];
						}
						if(sum==0)
							sum=1;
					SetFilter33(filter);
					//	****************************************///	
					OutBuff[i*width+j]=(InBuff[(i-1)*width+(j-1)]*m_filter33[0] +		InBuff[(i-1)*width+(j)]*m_filter33[1]+		InBuff[(i-1)*width+(j+1)]*m_filter33[2]
					                   +InBuff[(i)*width+(j-1)]  *m_filter33[3] +		InBuff[(i)*width+(j)]  *m_filter33[4]+		InBuff[(i)*width+(j+1)]*m_filter33[5]
								       +InBuff[(i+1)*width+(j-1)]*m_filter33[6] +		InBuff[(i+1)*width+(j)]*m_filter33[7]+		InBuff[(i+1)*width+(j+1)]*m_filter33[8])/9;
			 		OutBuff[i*width+j]/=sum ;
				}
		
		
				
				
				
			}
		}
	
		
        
		poBandNew->RasterIO( GF_Write, 0, 0, width, height, OutBuff, width, height, GDT_Float32, 0, 0 );
		
	}
	if( poDstDS != NULL )
	{
		delete poDstDS;
		poDstDS=NULL;
	}
	if(poDataset!=NULL)
	{
		delete poDataset;
		poDataset=NULL;
	}
	
	delete []InBuff;
	delete []OutBuff;
}


long Filter::Replace(long *bArray, long classes, long iFilterLen, long min_freq)
{

	long	* frequency;
	long temp_member=0;
	long max_freq_value=0;
	long max_freq_index=0;
	
	
	frequency=new long[classes+1];
	long j=0;


	for(j;j<classes+1;j++)
	{
		frequency[j]=0;
	}

	for(long i=0; i<iFilterLen*iFilterLen;i++)
	{
		//数值循环起点选择为0，故令象元值起点也为0
		temp_member=bArray[i];
		frequency[temp_member]=frequency[temp_member]+1;
	}

	for( j=0;j<classes+1;j++)
	{
		if(frequency[j]>max_freq_value)
		{
			//找出窗口内各类别象元的最大频率
			max_freq_value=frequency[j];

			//最大频率象元值
			max_freq_index=j;
		}
	}

    temp_member=bArray[(iFilterLen/2)*iFilterLen+iFilterLen/2];
	
	//如果象元出现频率高于给定的最小频率；
	//如果低于给定的最小频率

		if(frequency[temp_member]<min_freq)
		{
		                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                  
			//从内存中删除指针
			delete []frequency;
			return max_freq_index;
		}
		else
		{
			

			//从内存中删除指针
			delete []frequency;
			return temp_member;
			
		}

}

void Filter::Smooth(BSTR sourceFile, BSTR targetFile,long winsize, long threshold )
{
		long line=0;//the height of the example file "cluster"=1396;
	long column=0;//the width of the example file "cluster"=1428;

//	FILE  *stream1,*stream2,*stream3;
//	FILE  *stream2;
	long *temp1;


	long *aValue;
	long *result;
	long classnum;

	 GDALAllRegister();
	GDALDataset  *poDataset;
	poDataset = (GDALDataset *) GDALOpen(_bstr_t(sourceFile), GA_ReadOnly );
	
//	long width, height, pixCount;

	long  lWidth = poDataset->GetRasterXSize();
    long  lHeight = poDataset->GetRasterYSize();
	column=lWidth;
	line=lHeight;
	
	


		//为各个指针开辟内存

	temp1=new long[(line-winsize+1)*(column-winsize+1)];
	aValue=new long[winsize*winsize];
	result=new long[column];
	long *buf = new long[column*line];

	if(buf==NULL)
	{
		::MessageBox(NULL, "Failure in memory applying.","OK",MB_OK);
        exit(0);
	}
	
	GDALRasterBand  *poBand;
	GDALRasterBand *poBandNew;
	const	char * pszFormat = poDataset->GetDriver()->GetDescription();
    GDALDriver *poDriver;  
    poDriver = GetGDALDriverManager()->GetDriverByName(pszFormat);
	GDALDataset *poDstDS; 
	char **papszOptions = NULL;    
	int nBand=poDataset->GetRasterCount();
    poDstDS = poDriver->Create(_bstr_t(targetFile), lWidth, lHeight,nBand, GDT_Int32 , 
		papszOptions );
	double    adfGeoTransform[6];
	CPLErr hr=poDataset->GetGeoTransform( adfGeoTransform );
	if(hr==CE_None)
	{
		poDstDS->SetGeoTransform( adfGeoTransform );
	}

	const char  * strPJ=poDataset->GetProjectionRef();
	 CString CStrPJ=strPJ;
	if(CStrPJ!="")
    {
		poDstDS->SetProjection( strPJ );
	}

for(int kkk=1;kkk<=nBand;kkk++)
{
	poBand=poDataset->GetRasterBand( kkk );
	poBand->RasterIO( GF_Read, 0, 0, lWidth, lHeight, buf, lWidth, lHeight, GDT_Int32,  0, 0 );

	//读取图像的类别数
	long num=line*column;
	for(long m=0;m<column*line;m++)
	{
		for(long n=m+1;n<column*line;n++)
		{
			if (buf[m] != buf[n])
			continue;
			else num--;
			break;
		}		
	}
	classnum=num+1;
	long i=0;

	for (i;i<line-winsize+1;i++)
	{
	
		for (long j=0;j<column-winsize+1;j++)
		{
			
			// 读取滤波器数组
			for (long k = 0; k < winsize; k++)
			{
				for (long l = 0; l < winsize; l++)
				{
				
					// 保存象素值
				aValue[k*winsize+l] =buf[i*column+j+k*column+l];
				
		
				}
			}
			
			// 获取winsize*winsize窗口中频率最大的象元值,赋给出现频率小于阈值的象元

			result[j]= Replace(aValue,classnum,winsize,threshold);
			temp1[i*(column-winsize+1)+j]=result[j];
			//此处可以设置 classnum（11）,winsize（5）,threshold（9）
		
		}
	
	}



	//下面是将图像处理时漏掉的四行值补上
	for ( i=0;i<line-winsize+1;i++)
	{
		
		for (long j= 0; j <column-winsize+1; j++)
		{
						
				
			buf[(i+winsize/2)*column+j+winsize/2]=temp1[i*(column-winsize+1)+j];
					
			
		}
			
		
	}
			
//	fwrite(buf,sizeof(unsigned char),column*line, stream2);

 

	poBandNew = poDstDS->GetRasterBand(kkk);
	poBandNew->RasterIO( GF_Write, 0, 0, lWidth, lHeight, 
		buf, lWidth, lHeight, GDT_Int32, 0, 0 );
}

	if( poDstDS != NULL )
	{
		delete poDstDS;
		poDstDS=NULL;
	}
	if(poDataset!=NULL)//??????,????????,???c++???
	{
		delete poDataset;
		poDataset=NULL;
	}

	
//	fclose(stream1);
	//fclose(stream2);


	delete [] buf;
	delete [] aValue;
	delete [] result;
	delete [] temp1;

	::MessageBox(NULL,"处理完毕","OK",MB_OK);


}


void Filter::Histequal(int iWidth, int iHeight, byte *source, byte *result)
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
			
			bMap[i] = (byte)(iTemp * 255 / (iHeight * iWidth));
			
			if (bMap[i] < 0)
				bMap[i] = 0;
			else if (bMap[i] > 255)
				bMap[i] = 255;
		}
		
		for ( x = 0; x < iHeight; x++)
			for ( y = 0; y < iWidth; y++)	
				result[x* iWidth+y] = bMap[source[x* iWidth+y]%256];	

}

void Filter::Histequal(BSTR sourceFile, BSTR targetFile)
{
	GDALAllRegister();
	GDALDataset  *poDataset;
	poDataset = (GDALDataset *) GDALOpen( _bstr_t(sourceFile), GA_ReadOnly );//GA_ReadOnly or GA_Update
	if(poDataset==NULL)
	{
		AfxMessageBox("faild to open file");
		exit(0);
	}
	long  width = poDataset->GetRasterXSize();
    long  height = poDataset->GetRasterYSize();	
	byte *InBuff=new byte[width*height];
	byte *OutBuff=new byte[width*height];
		
	int nBand=poDataset->GetRasterCount();
	const char *  pszFormat = poDataset->GetDriver()->GetDescription();
    GDALDriver *poDriver;  
    poDriver = GetGDALDriverManager()->GetDriverByName(pszFormat);
	GDALDataset *poDstDS; 
	char **papszOptions = NULL;    
    poDstDS = poDriver->Create(_bstr_t(targetFile), width, height, nBand, GDT_Byte , 
		papszOptions );
	
	double    adfGeoTransform[6];
    CPLErr hr=poDataset->GetGeoTransform( adfGeoTransform );
	if(hr==CE_None)
	{
		poDstDS->SetGeoTransform( adfGeoTransform );
	}

	const char  * strPJ=poDataset->GetProjectionRef();
	 CString CStrPJ=strPJ;
	if(CStrPJ!="")
    {
		poDstDS->SetProjection( strPJ );
	}
	
	GDALRasterBand  *poBand;
	GDALRasterBand *poBandNew;
	////////////////////////
	//////////////////////////
	double sum=0;
	for(int k=1;k<=nBand;k++)
	{
		poBand=poDataset->GetRasterBand(k);
		poBandNew=poDstDS->GetRasterBand(k);
		poBand->RasterIO( GF_Read, 0, 0, width, height, InBuff, width, height, GDT_Byte,  0, 0 );
		this->Histequal(width, height, InBuff,OutBuff);
		poBandNew->RasterIO( GF_Write, 0, 0, width, height, OutBuff, width, height, GDT_Byte, 0, 0 );
		
	}
	if( poDstDS != NULL )
	{
		delete poDstDS;
		poDstDS=NULL;
	}
	if(poDataset!=NULL)
	{
		delete poDataset;
		poDataset=NULL;
	}
	
	delete []InBuff;
	delete []OutBuff;

}
