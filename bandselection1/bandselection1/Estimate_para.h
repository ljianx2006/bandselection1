#pragma once
#include "Parameter.h"
class CEstimate_para
{

public:
	CEstimate_para(void);
	~CEstimate_para(void);

public:
	void Mixture(Parameter *para,vector<vector<float>>data,int kmin,int kmax,float regulatize,float iterthreshold,int covoption,int Maxiteration);

	/********************¸¨Öúº¯Êý********************/
	void Getcov(vector<vector<float> >&data,float **covresult);
	void Elipsnorm(float *mu,float **cov,int level,int dashed =0) ;
	void multinorm(vector<vector<float> >&data,vector<float>&mu,vector<vector<float> >&cov,vector<float>&result);
	void uninorm(float *data,int len,float mu,float cov,float *result);
};

