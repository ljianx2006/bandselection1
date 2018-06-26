#pragma once
#include <vector>
#include "gdal_priv.h"
//#include <windef.h>
using namespace std;

//参数类：存放所有函数所需的输入以及输出，需要建立成单例模式

class Parameter
{
	friend class CEstimate_para;
	friend class CKappa_analysia;
	friend class CImproved_weighted_classify;


private:

	Parameter(void);
public:
	
	


public:

   /*****************************************
       参数估计类需要输入参数
   ******************************************/
	vector<vector<float> >imgdata; //图像数据
	vector<vector<float> >sample; //图像数据
	int imgsize;     //图像大小
	int nWidth,nHeight;
	int dimens;      //图像维数
	int classnum;    //类别数 
	int bandnum;     //波段数
	//int kmax,kmin;//混合元素初始最大的数目，kmin是测试元素最小的数目（作为外部的输入)kmin=classnum
	//float stopth;//迭代停止阈值
	//int max_iteration;//CEM最大迭代次数
	void Normaldata();//归一化图像
     //BOOL parafinsh;
	 bool parafinsh;
   /*****************************************
       参数估计类需要输出参数,作为精度估计类的输入
   ******************************************/

	int bestk; //选择的元素个数
	float *bestpp,**bestmu;//bestpp得到的向量的混合概率，bestmu是元素均值的估计值，包括d行bestk列
	float ***bestcov;//元素的协方差的估计，它是一个三维（3索引）阵列
	//float *dlength; //the successive values of the cost function
	int count_iter; //执行迭代的总数目
	vector<vector<float> >minpp_all;
	vector<vector<float> >minmu_all;
	vector<vector<float> >mincov_all;


	 /*****************************************
       精度估计类的输出
   ******************************************/
	vector<float>accuracy;
	vector<float>kappa;
	vector<vector<float> >Precision;
	vector<vector<float> >Recall;
	
   /*****************************************
       改进算法类的输入
   ******************************************/
	vector<vector<float>>Newmu;
	vector<vector<float>>Newcov;
	vector<vector<float> >Newpp;
	vector<vector<float> >NewPrecision;
	vector<vector<float> >NewRecall;

	vector<vector<float> >train_data;
	vector<vector<float> >test_data;
	int num_neighbors;

	vector<int>testresult;
	static Parameter * Image;

public:
	static Parameter *getInstance()
	{
		if (Image==NULL)
			Image =new Parameter();
		return Image;
	}
	~Parameter(void);
	void Initparameter(int num);
	void init_GrayImg(GDALDataset* pdataset);
	void IteraClassRankMapping(vector<vector<float>>&sample);
	int findindex(vector<float>&index,int k);
	
};

