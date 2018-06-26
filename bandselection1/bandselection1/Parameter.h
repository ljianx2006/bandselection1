#pragma once
#include <vector>
#include "gdal_priv.h"
//#include <windef.h>
using namespace std;

//�����ࣺ������к�������������Լ��������Ҫ�����ɵ���ģʽ

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
       ������������Ҫ�������
   ******************************************/
	vector<vector<float> >imgdata; //ͼ������
	vector<vector<float> >sample; //ͼ������
	int imgsize;     //ͼ���С
	int nWidth,nHeight;
	int dimens;      //ͼ��ά��
	int classnum;    //����� 
	int bandnum;     //������
	//int kmax,kmin;//���Ԫ�س�ʼ������Ŀ��kmin�ǲ���Ԫ����С����Ŀ����Ϊ�ⲿ������)kmin=classnum
	//float stopth;//����ֹͣ��ֵ
	//int max_iteration;//CEM����������
	void Normaldata();//��һ��ͼ��
     //BOOL parafinsh;
	 bool parafinsh;
   /*****************************************
       ������������Ҫ�������,��Ϊ���ȹ����������
   ******************************************/

	int bestk; //ѡ���Ԫ�ظ���
	float *bestpp,**bestmu;//bestpp�õ��������Ļ�ϸ��ʣ�bestmu��Ԫ�ؾ�ֵ�Ĺ���ֵ������d��bestk��
	float ***bestcov;//Ԫ�ص�Э����Ĺ��ƣ�����һ����ά��3����������
	//float *dlength; //the successive values of the cost function
	int count_iter; //ִ�е���������Ŀ
	vector<vector<float> >minpp_all;
	vector<vector<float> >minmu_all;
	vector<vector<float> >mincov_all;


	 /*****************************************
       ���ȹ���������
   ******************************************/
	vector<float>accuracy;
	vector<float>kappa;
	vector<vector<float> >Precision;
	vector<vector<float> >Recall;
	
   /*****************************************
       �Ľ��㷨�������
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

