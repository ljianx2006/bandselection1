//#include "stdafx.h"
#include "Parameter.h"
//#include "Improved_weighted_classify.h"
#include <vector>
#include <iostream>
using namespace std;


Parameter::Parameter(void)
{
	parafinsh =FALSE;
}


Parameter::~Parameter(void)
{
}
void Parameter::init_GrayImg( GDALDataset *pdataset)
{
	int xlen = pdataset->GetRasterXSize();
	int ylen = pdataset->GetRasterYSize();
	int zlen = pdataset->GetRasterCount();

	nWidth = xlen;
	nHeight = ylen;
	imgsize = xlen * ylen;
	bandnum = zlen;
	imgdata.clear();
	GDALRasterBand **pBand;
	pBand = new GDALRasterBand * [zlen]; //新建波段
	for (int k=0;k!=zlen;++k)
	{ 
		vector<float>Image_Gray(imgsize,0);
		pBand[k]=pdataset->GetRasterBand(k+1);//选择波段
		if (pBand[k]== NULL)  
		{  
			cout<<"pBand Fail"<<endl;  
		} 

		unsigned short *Image_Gray0 = ( unsigned short *)CPLMalloc(sizeof( unsigned short)*imgsize);
		pBand[k]->RasterIO(GF_Read,0,0,xlen,ylen,Image_Gray0,xlen,ylen,GDT_UInt16 ,0,0);//提取图像的灰度

		for (int i=0;i<imgsize;i++)
		{
			Image_Gray[i]=float(Image_Gray0[i]);
		}
		imgdata.push_back(Image_Gray);
		CPLFree(Image_Gray0); 
	}
	//预留band+1行为参考图
	vector<float>label(imgsize,0);
	imgdata.push_back(label);
	//计算图像的坐标:band+2行为行坐标，band+3行为列坐标
	vector<float>linenum(imgsize,0);
	vector<float>colnum(imgsize,0);
	imgdata.push_back(linenum);
	imgdata.push_back(colnum);
	for (int i=0;i<ylen;i++)
	{
		for(int j=0;j<xlen;j++) 
		{
			imgdata[zlen+1][i*xlen+j]=i;
			imgdata[zlen+2][i*xlen+j]=j;
		}
	}
	
}

void Parameter::Initparameter(int num)
{
	dimens = imgdata.size();
    classnum =num;

}
void Parameter::Normaldata()
{	
	vector<float>minbuf;
	vector<float>maxbuf;
	//求出每一行数据的最大最小值，并放入minbuf，maxbuf中
	float min,max;
	for (int i =0;i!=imgdata.size()-3;++i)
	{
		max = min = imgdata [i][0];
		for (int j=1;j!=imgdata[0].size();++j)
		{
			if(imgdata[i][j]<min)
				min = imgdata[i][j];
			if(imgdata[i][j]>max)
				max=imgdata[i][j];
		}
		minbuf.push_back(min);
		maxbuf.push_back(max);
	}
	for (int i =0;i!=imgdata.size()-3;++i)
		for (int j=0;j!=imgdata[0].size()-2;++j)
		{
			imgdata[i][j] = 255*(imgdata[i][j] -minbuf[i])/(maxbuf[i]-minbuf[i]);
		}
}
int  Parameter::findindex(vector<float>&index,int k)
{
	for (int i=0;i!=classnum;++i)
	{
		if (index[i]==k)
		{
			return i;
		}
	}
}
//void Parameter::IteraClassRankMapping(vector<vector<float>>&sample)
//{
//	vector<vector<float>>samplemu;
//	//计算样本的均值
//	vector<float>mubuf(classnum,0);
//	vector<int>num(classnum,0);
//	for(int i=0;i!=bandnum;++i)
//	{
//		mubuf.assign(classnum,0);
//		num.assign(classnum,0);
//		for (int j=0;j!=sample[0].size();++j)
//		{
//			mubuf[sample[bandnum][j]-1] +=sample[i][j];
//			num[sample[bandnum][j]-1]++;
//		}
//		for (int j=0;j!=classnum;++j)
//		{
//			mubuf[j] = mubuf[j]/num[j];
//		}
//		samplemu.push_back(mubuf);
//	}
//	//对均值排序
//	for (int i=0;i!=bandnum;++i)
//	{
//		vector<float>S_index(classnum,0);
//		vector<float>E_index(classnum,0);
//		for (int j=0;j!=classnum;j++)
//		{
//			S_index[j] = j;
//			E_index[j] = j;
//		}
//		CImproved_weighted_classify classier;
//		classier.quicksort(samplemu[i],S_index,0,classnum-1);
//		classier.quicksort(minmu_all[i],E_index,0,classnum-1);
//		vector<float>Newmubuf(classnum,0);vector<float>Newcovbuf(classnum,0);vector<float>Newppbuf(classnum,0);
//		vector<float>NewPrebuf(classnum,0);vector<float>NewRebuf(classnum,0);
//		for (int j=0;j!=classnum;++j)
//		{
//			int k =findindex(S_index,j); 
//			Newmubuf[j] = minmu_all[i][E_index[k]];
//			Newcovbuf[j] = mincov_all[i][E_index[k]];
//			Newppbuf[j] = minpp_all[i][E_index[k]];
//			NewPrebuf[j] = Precision[i][E_index[k]];
//			NewRebuf[j] = Recall[i][E_index[k]];
//		}
//		Newmu.push_back(Newmubuf);Newcov.push_back(Newcovbuf);Newpp.push_back(Newppbuf);
//		NewPrecision.push_back(NewPrebuf);NewRecall.push_back(NewRebuf);
//	}
//}
Parameter *Parameter::Image =NULL;