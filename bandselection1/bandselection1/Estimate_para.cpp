//#include "stdafx.h"
#include "Estimate_para.h"
#include <Windows.h>
#include <stdio.h>
#include <iostream>
#include <time.h>
#include <string>
#include <vector>
using namespace std;

#define PI 3.1416
#define REAL_MIN 3.40e-30

CEstimate_para::CEstimate_para(void)
{
}


CEstimate_para::~CEstimate_para(void)
{
}

void CEstimate_para::Getcov(vector<vector<float> >&data,float **covresult)
{ 
	int xlen = data.size();
	int ylen = data[0].size();
	float *line_mu = new float[xlen];
	memset(line_mu,0,sizeof(float)*xlen);
	//求每行的均值
	for(int i=0;i!=xlen;++i)
	{
		for (int j=0;j!=ylen;++j)
		{
			line_mu[i]+=data[i][j];
		}
		line_mu[i] = line_mu[i]/ylen;
	}
	for (int i=0;i!=xlen;++i)
		for (int j=0;j!=xlen;++j)
		{
			for (int k = 0;k!=ylen;++k)
			{
				covresult[i][j] +=(data[i][k]-line_mu[i])*(data[j][k]-line_mu[j]);
			} 
			covresult[i][j] = covresult[i][j]/ylen;
		}
	delete[]line_mu;
}
void CEstimate_para::Elipsnorm(float *mu,float **cov,int level,int dashed)    //covb本身是对角阵，不用对其做奇异值分解，只需对其对角线元素进行排序即可
{
	//mu为2x1的矩阵转化来，分别为estcv[0][comp],esrcv[1][comp],cov为三维矩阵estcov[0][0][comp]——[1][1][comp]转化来的2x2的对角矩阵
	bool tflag =false;
	if (cov[0][0]<cov[1][1])
	{
		float temp = cov[0][0];
		cov[0][0] =cov[1][1];
		cov[1][1] =temp;
		tflag =true;
	}
	float a = sqrt(cov[0][0]*level*level);
	float b =sqrt(cov[1][1]*level*level);
	int thcount = int(2*PI/0.01)+1;
	float *theta =new float[thcount];
	float *xx = new float [thcount];
	float *yy = new float[thcount];
	for (int i =0;i!=thcount;++i)
	{
		theta[i] =(float)0.01*i;
		xx[i] = a*cos(theta[i]);
		yy[i] = b*sin(theta[i]);
	}
	float **cord=new float *[2];
	if (tflag)
	{
		cord[0] = xx;
		cord[1] = yy;
	}
	else
	{
		cord[0] = yy;
		cord[1] = xx;
	}
	//plot部分是什么意思////
	return;
}
//此函数中数据为一维
void CEstimate_para::multinorm(vector<vector<float> >&data,vector<float>&mu,vector<vector<float> >&cov,vector<float>&result)
{
	int dimens =data.size();
	int imgsize =data[0].size();
	float dd = 1;//dd为对角阵cov的行列式
	float **inv_cov = new float *[dimens];//inv_cov为cov的逆矩阵
	for(int i=0;i!=dimens;++i)
	{
		inv_cov[i] = new float [dimens];
		memset(inv_cov[i],0,sizeof(float)*dimens);
	}
	for (int i=0;i!=dimens;++i)
	{
		if (cov[i][i]<0)
		{
			cov[i][i] = abs(cov[i][i]);
		}
		if(cov[i][i] ==0)
			cov[i][i] +=REAL_MIN;
		dd *=cov[i][i];
		inv_cov[i][i] = 1/cov[i][i];
	}

	float ff = 1/sqrt(2*PI*abs(dd));
	float **centered = new float *[dimens];
	for(int i=0;i!=dimens;++i)
		centered[i] = new float[imgsize];
	for (int i=0;i!=dimens;++i)
		for (int j=0;j!=imgsize;++j)
		{
			centered[i][j] =data[i][j]-mu[i];
		}
	if (dimens!=1)
	{
		for (int i=0;i!=dimens;++i)
			for (int j=0;j!=imgsize;++j)
			{
				centered[i][j] = centered[i][j]*centered[i][j]*inv_cov[i][i];
			}
		for (int i =0;i!=imgsize;++i)
			for (int j =0;j!=dimens;j++)
			{
				result[i] +=centered[j][i];
			}
		for(int i =0;i!=imgsize;++i)
			result[i] = ff*exp(-0.5*result[i]);
	}
	else
	{
		for (int i=0;i!=imgsize;++i)
			{
				centered[0][i] =pow(centered[0][i],2);
				result[i] =ff*exp(-0.5*inv_cov[0][0]*centered[0][i]);
				if(result[i]==0)
					result[i]=REAL_MIN;
			}
		}

		for (int i=0;i!=dimens;++i)
		{
			delete []inv_cov[i];
			delete []centered[i];
		}
		delete[]centered;	
		delete[]inv_cov;

}
void CEstimate_para::uninorm(float *data,int len,float mu,float cov,float *result)
{
	float ff;
	ff =pow(2*PI*(cov+REAL_MIN),0.5);
	for (int i=0;i!=len;++i)
	{
		result[i] =exp((-1/(2*cov))*pow((data[i]-mu),2))/ff;
	}
}

//此函数中数据为一维
void CEstimate_para::Mixture(Parameter *para,vector<vector<float>>data,int kmin,int kmax,float regulatize,float iterthreshold,int covoption,int Maxiteration)//valmin，valmax 分别为kmin 和kmax
{
	//变量定义
	int imgsize = data[0].size();
	int dimens = data.size();
	int npars;
	int k = kmax;
	/*int **indic =new int *[k];
	for(int i = 0;i!= k;++i)
	{
		indic[i] = new int [para->imgsize];
		memset(indic[i],0,sizeof(int)*para->imgsize);
	}*/
	float **estmu = new float *[dimens];
	for(int i=0;i!=dimens;++i)
	{
		estmu[i] = new float [k];
	}
	int *randindex = new int[k];
	srand(time(0));
	for (int i=0;i!=k;++i)
	{
		randindex[i] = rand()%imgsize; //随机产生k个0~para.imgsize之间的数
		for (int j=0;j!=dimens;++j)	
			estmu [j][i] = data[j][randindex[i]];
	}
	delete[]randindex;
	//操作
	switch (covoption)
	{
	case 0:
		npars = (dimens + dimens*(dimens+1)/2);
		break;
	case 1:
		npars = 2*dimens;
		break;
	case 2:
		npars = dimens;
		break;
	case 3:
		npars = dimens;
		break;
	default:
		npars = (dimens + dimens*(dimens+1)/2);
		break;
	}
	int nparscover2 = npars/2;

	//求出全图的协方差
	float **globcov = new float *[dimens];
	for (int i = 0;i!=dimens;++i)
	{
		globcov[i] = new float[dimens];
		memset(globcov[i],0,sizeof(float)*dimens);
	}
	Getcov(data,globcov);

	float ***estcov = new float**[dimens];
	for (int i =0;i!=dimens;++i)
		estcov[i] =new float *[dimens];

	for (int i=0;i!=dimens;++i)
		for (int j = 0;j!=dimens;++j)
		{
			estcov[i][j] = new float [k];
			memset(estcov[i][j],0,sizeof(float)*k);
		}

	//找出glocov对角线中的最大值
	float maxbuf = globcov[0][0];
	for (int i = 0;i!=dimens;++i)
	{
		if (globcov[i][i]>maxbuf)
			maxbuf = globcov[i][i];
	}
	
	//对estcov赋值
	for (int i=0;i!=k;++i)
		for(int j=0;j!=dimens;++j)
			estcov[j][j][i] =maxbuf/k;

	//初始化estpp
	float *estpp =new float[k];
		for(int i=0;i!=k;++i)
			estpp[i] =1/(float)k;

	/*int size_p =201;
	float *plotgrid=new float [size_p];
	float *mix =new float[size_p];
	memset(mix,0,sizeof(float)*size_p);
	if (dimens == 1)
	{
		//找出同一个坐标点中最大的和最小的维数的灰度值
		float mindata,maxdata; 
		mindata = data[0][0];
		maxdata = data[0][0];
		for (int i =0;i!=imgsize;++i)
		{
			if (data[0][i]<mindata)
			{
				mindata = data[0][i];
			}
			if (data[0][i]>maxdata)
			{
				maxdata= data[0][i];
			}
		}

		//测试用estmu
		estmu[0][0] = 15.3614;estmu[0][1] = 9.2169;estmu[0][2] = 132.1084;estmu[0][3] = 138.2530;estmu[0][4] = 193.5542;
		estmu[0][5] =255.0000;estmu[0][6] = 187.4096;estmu[0][7] = 73.7349;estmu[0][8] = 144.3976;estmu[0][9] =95.2410;

		for (int i =0;i!=201;++i)
			plotgrid[i] = mindata+i*(maxdata-mindata)/200;
		float * mixresult =new float[size_p];
		memset(mixresult,0,sizeof(float)*size_p);
		for (int j=0;j!=k;++j)
		{
			uninorm(plotgrid,size_p,estmu[0][j],estcov[0][0][j],mixresult);
			for (int i =0;i!=201;++i)
				mix[i] =mix[i]+estpp[j]*mixresult[i];
		}
		delete[]mixresult;
	}
	else
	{
			float *mubuf =new float[2];
			float **covbuf=new float *[2];
			for (int i=0;i!=2;++i)
				covbuf[i] =new float[2];

			for (int i =0;i!=k;++i)
			{
				//给mubuf和covbuf赋值
				mubuf[0] = estmu[0][i];
				mubuf[1] = estmu[1][i];
				covbuf[0][0] =estcov[0][0][i];
				covbuf[0][1] =estcov[0][1][i];
				covbuf[1][0] =estcov[1][0][i];
				covbuf[1][1] =estcov[1][1][i];
				//调用Elipsnorm_w()函数
				Elipsnorm(mubuf,covbuf,2);
			}
			for(int i=0;i!=2;++i)
				delete []covbuf[i];
			delete []covbuf;
			delete[]mubuf;	
	}*/

	//matlab第89行
	//float **semi_indic =new float *[k];
	//float **all_indic = new float *[k];
	//for (int i=0;i!=k;++i)
	//{
	//	semi_indic[i] =new float[imgsize];
	//	all_indic[i] = new float[imgsize];
	//	memset(semi_indic[i],0,imgsize*sizeof(float));
	//}
	vector<vector<float>>semi_indic(k,vector<float>(imgsize,0));
	vector<vector<float>>all_indic(k,vector<float>(imgsize,0));
	for(int i=0;i!=k;++i)
	{
		/*float *mubuf = new float[dimens];
		float **covbuf = new float*[dimens];*/
		vector<float>mubuf(dimens,0);
		vector<vector<float>>covbuf(dimens,vector<float>(dimens,0));
		for (int j =0;j!=dimens;++j)
		{
			/*covbuf[j] =new float [dimens];
			memset(covbuf[j],0,dimens*sizeof(float));*/
			mubuf[j] =estmu[j][i];    //取estmu的第i列赋值给mubuf
		}
		//将estcov[][][i]赋值给covbuf
		for (int j=0;j!=dimens;++j)
			covbuf[j][j] =estcov[j][j][i];
		multinorm(data,mubuf,covbuf,semi_indic[i]);
		//将semi_indic每个元素除k
		for (int j=0;j!=imgsize;++j)
		{
			//cout<<j<<": "<<semi_indic[i][j]<<endl;
			all_indic[i][j] =semi_indic[i][j]/k;
		}
		/*for(int j=0;j!=dimens;++j)
			delete[]covbuf[j];
		delete[]mubuf;
		delete[]covbuf;*/
	}
		//是否对输出bestk，bestcov等赋初值
		para->bestk =k;
		
		int countf =1;int k_cont =1;
		vector<float>loglike;
		vector<float>dlength;
		vector<float>kappas;

		float loglikebuf = 0;
		for (int i=0;i!=imgsize;++i)
		{
			float sumbuf =0;
			for(int j=0;j!=k;++j)
				sumbuf +=all_indic[j][i];
			if(sumbuf==0)
				sumbuf = REAL_MIN;
			loglikebuf +=log(sumbuf);
		}
		loglike.push_back(loglikebuf);
		float dlengthbuf = -loglikebuf+(nparscover2*k*log(1/float(k)))+(nparscover2+0.5)*k*log(float(imgsize));
		dlength.push_back(dlengthbuf);
		kappas.push_back(k);
		float mindl = dlength[0];
		vector<int>subclass_index(kmax,1);
		vector<vector<int> >subclass_index0;
		subclass_index0.push_back(subclass_index);

		while(k_cont)
		{
			int cont = 1;
			int iteration_step =1;
			while (cont&&(iteration_step<Maxiteration)&&k>=kmin)
			{
				int comp=0;
				while(comp<=k-1)
				{
					//为all_indic重新赋值
					for (int i = 0;i!=k;++i)
						for (int j=0;j!=imgsize;++j)
						{
							all_indic[i][j] = semi_indic[i][j]*estpp[i];
						}
					//归一化all_indic
					vector<vector<float>>normindic(k,vector<float>(imgsize,0));
			
					vector<double>sum(imgsize,0);
					for (int i=0;i!=imgsize;++i)
						for (int j=0;j!=k;++j)
							sum[i] +=all_indic[j][i];

					for(int i=0;i!=k;++i)
						for (int j=0;j!=imgsize;++j)
						{
							if (sum[j]==0)
								sum[j]=REAL_MIN;
							normindic[i][j] =all_indic[i][j]/sum[j];
						}

					float normalized = 0;
					for(int j=0;j!=imgsize;++j)
						normalized +=normindic[comp][j];
					normalized =1/normalized;
					//aux是中间矩阵
					vector<vector<float>>aux(dimens,vector<float>(imgsize,0));
					for(int i=0;i!=dimens;++i)
						for (int j=0;j!=imgsize;++j)
						{
							aux[i][j] =normindic[comp][j]*data[i][j];
						}

					for(int j=0;j!=dimens;++j)
					{
						double sum =0;
						for(int i=0;i!=imgsize;++i)
							sum +=aux[j][i];
						estmu[j][comp] =normalized*sum;
					}

					//matlab第121行  更新estcov的值
					if ((covoption==0)||(covoption==2))
					{
						vector<vector<float>>bufdata(dimens,vector<float>(imgsize,0));
						for (int i=0;i!=dimens;++i)
							for (int j=0;j!=imgsize;++j)
							{
								bufdata[i][j] =normindic[comp][j]*(data[i][j]-estmu[i][comp]);
							}

						for (int i=0;i!=dimens;++i)
							for (int j=0;j!=dimens;++j)
							{
								float sumda =0;
								for(int m=0;m!=imgsize;++m)
									sumda +=bufdata[i][m]*data[j][m];
								estcov[i][j][comp] =normalized*sumda;
							}
					}
					else
					{
						vector<float>sumbuf(dimens,0);
						for (int i=0;i!=dimens;++i)
							for (int j=0;j!=imgsize;++j)
							{
								sumbuf [i]+=normalized*aux[i][j]*data[i][j];
							}
							for (int i=0;i!=dimens;++i)
							{
								estcov[i][i][comp] =sumbuf[i]-estmu[i][comp]*estmu[i][comp]; 
							}
					}
					if (covoption==2)
					{
						float **comcov = new float *[dimens];
						for (int i =0;i!=dimens;++i)
						{
							comcov[i] =new float[dimens];
							memset(comcov[i],0,dimens*sizeof(float));
						}

						for (int i =0;i!=dimens;++i)
							for (int j =0;j!=dimens;++j)
							{
								for (int comp2=0;comp2!=k;++comp2)
									comcov[i][j] +=estpp[comp2]*estcov[i][j][comp2];
							}
							//将comcov的值赋给estcov
							for (int comp2=0;comp2!=k;++comp2)
								for (int i =0;i!=dimens;++i)
									for (int j =0;j!=dimens;++j)
										estcov[i][j][comp2] = comcov[i][j];

							for (int i =0;i!=dimens;++i)
								delete[]comcov[i];
							delete[]comcov;
					}
					if (covoption==3)
					{
						float **comcov = new float *[dimens];
						for (int i =0;i!=dimens;++i)
						{
							comcov[i] =new float[dimens];
							memset(comcov[i],0,dimens*sizeof(float));
						}

						for (int i =0;i!=dimens;++i)
							for (int comp2=0;comp2!=k;++comp2)
								comcov[i][i] +=estpp[comp2]*estcov[i][i][comp2];

						//将comcov的值赋给estcov
						for (int comp2=0;comp2!=k;++comp2)
							for (int i =0;i!=dimens;++i)
								estcov[i][i][comp2] = comcov[i][i];

						for (int i =0;i!=dimens;++i)
							delete[]comcov[i];
						delete[]comcov;
					}
					//更新estpp的值
					float sum0 =0;
					for (int i=0;i!=imgsize;++i)
						sum0 += normindic[comp][i];
					estpp[comp] =abs(sum0-nparscover2)/imgsize;
					//对estpp做归一化
					sum0 =0;
					for (int i=0;i!=k;++i)
						sum0 +=estpp[i];
					for (int i=0;i!=k;++i)
						estpp[i] =estpp[i]/sum0;
					
					bool killflag =0;
					if (estpp[comp]==0)
					{
						killflag =1;
						vector<int>sub_pos;
						for(int i =0;i!=subclass_index.size();++i)
						{
							if(subclass_index[i]==1)
								sub_pos.push_back(i);
						}
						subclass_index[sub_pos[comp]]=0;
						subclass_index0.push_back(subclass_index);
						//去除estmu的comp列，estcov的第3维的comp，estpp的comp个，semi_indic的comp行
						for (int i=comp;i!=k-1;++i)
						{
							estpp[i]=estpp[i+1];
							for(int j=0;j!=dimens;++j)
							{
								estmu[j][i] =estmu[j][i+1];
								for(int m=0;m!=dimens;++m)
									estcov[j][m][i] = estcov[j][m][i+1];
							}
							for(int j=0;j!=imgsize;++j)
								semi_indic[i][j] =semi_indic[i+1][j];
						}
						if (k>kmin)
						{
							k=k-1;
						}
						else
						{
							k_cont=0;
							//将estpp，estmu，estcov的最小值保存到minpp，minmu，mincov
							vector<float>minpp_buf;
							vector<float>minmu_buf;
							vector<float>mincov_buf;
							//将minpp,minmu，mincov放入到minpp_all,minmu_all，mincov_all
							for (int i=0;i!=k;++i)
							{
								minpp_buf.push_back(estpp[i]);
								minmu_buf.push_back(estmu[0][i]);
								mincov_buf.push_back(estcov[0][0][i]);
							}
							para->minpp_all.push_back(minpp_buf);
							para->minmu_all.push_back(minmu_buf);
							para->mincov_all.push_back(mincov_buf);
							break;
						}
					}
					if ((covoption==2)||(covoption==3))
					{
						for(int i=0;i!=k;++i)
						{
							/*float *mubuf = new float[dimens];
							float **covbuf = new float*[dimens];*/
							vector<float>mubuf(dimens,0);
							vector<vector<float>>covbuf(dimens,vector<float>(dimens,0));
							for (int j =0;j!=dimens;++j)
							{
								/*covbuf[j] =new float [dimens];
								memset(covbuf[j],0,dimens*sizeof(float));*/
								mubuf[j] =estmu[j][i];    //取estmu的第i列赋值给mubuf
							}
							//将estcov[][][i]赋值给covbuf
							for (int j=0;j!=dimens;++j)
								covbuf[j][j] =estcov[j][j][i];
							multinorm(data,mubuf,covbuf,semi_indic[i]);

							//释放空间
							/*for (int j=0;j!=dimens;++j)
							{
								delete[]covbuf[j];
							}
							delete[]mubuf;
							delete[]covbuf;*/
						}
					}
					if (killflag==0)
					{
						/*float *mubuf = new float[dimens];
						float **covbuf = new float*[dimens];*/
						vector<float>mubuf(dimens,0);
						vector<vector<float>>covbuf(dimens,vector<float>(dimens,0));
						for (int j =0;j!=dimens;++j)
						{
							/*covbuf[j] =new float [dimens];
							memset(covbuf[j],0,dimens*sizeof(float));*/
							mubuf[j] =estmu[j][comp];    //取estmu的第i列赋值给mubuf
						}
						//将estcov[][][i]赋值给covbuf
						for (int j=0;j!=dimens;++j)
							covbuf[j][j] =estcov[j][j][comp];
						multinorm(data,mubuf,covbuf,semi_indic[comp]);
						
						comp =comp+1;
						//释放空间
				/*		for (int j=0;j!=dimens;++j)
						{
							delete[]covbuf[j];
						}
						delete[]mubuf;
						delete[]covbuf;*/
					}
				}
				countf++;
				iteration_step++;
				//将semi_indic置0
				/*for(int i=0;i!=k;++i)
					memset(semi_indic[i],0,imgsize*sizeof(float));*/
				for (int i=0;i!=k;++i)
				{
				/*	float *mubuf = new float[dimens];
					float **covbuf = new float*[dimens];*/
					vector<float>mubuf(dimens,0);
					vector<vector<float>>covbuf(dimens,vector<float>(dimens,0));
					for (int j =0;j!=dimens;++j)
					{
						/*covbuf[j] =new float [dimens];
						memset(covbuf[j],0,dimens*sizeof(float));*/
						mubuf[j] =estmu[j][i];    //取estmu的第i列赋值给mubuf
					}
					//将estcov[][][i]赋值给covbuf
					for (int j=0;j!=dimens;++j)
						covbuf[j][j] =estcov[j][j][i];
					multinorm(data,mubuf,covbuf,semi_indic[i]);
					
					for (int j=0;j!=imgsize;++j)
					{
						all_indic[i][j] =semi_indic[i][j]*estpp[i];
						if (all_indic[i][j]<REAL_MIN)
							all_indic[i][j] =REAL_MIN;
					}
					//释放空间
				/*	for (int j=0;j!=dimens;++j)
						delete[]covbuf[j];
					delete[]mubuf;
					delete[]covbuf;*/
				}
				//添加loglike新元素
				loglikebuf=0;
				for (int i=0;i!=imgsize;++i)
				{
					float sumbuf =0;
					for(int j=0;j!=k;++j)
						sumbuf +=all_indic[j][i];
					loglikebuf +=log(REAL_MIN+sumbuf);
				}
				loglike.push_back(loglikebuf);
				//求log（estpp）的和
				float sumpp =0;
				for (int i=0;i!=k;++i)
				{
					sumpp+=log(estpp[i]);
				}

				dlengthbuf = -loglikebuf+(nparscover2*sumpp)+(nparscover2+0.5)*k*log(float(imgsize));
				dlength.push_back(dlengthbuf);
				kappas.push_back(k);

				if (abs((loglike[countf-1]-loglike[countf-2])/loglike[countf-2])<iterthreshold)
				{
					cont =0;
				}
           }
			/*if (dimens==1)
			{
				float * mixresult =new float[size_p];
				memset(mixresult,0,sizeof(float)*size_p);
				for (int j=0;j!=k;++j)
				{
					uninorm(plotgrid,size_p,estmu[0][j],estcov[0][0][j],mixresult);
					for (int i =0;i!=201;++i)
						mix[i] =mix[i]+mixresult[i];
				}
				delete[]mixresult;
			}
			else
			{
				float *mubuf =new float[2];
				float **covbuf=new float *[2];
				for (int i=0;i!=2;++i)
					covbuf[i] =new float[2];

				for (int i =0;i!=k;++i)
				{
					//给mubuf和covbuf赋值
					mubuf[0] = estmu[0][i];
					mubuf[1] = estmu[1][i];
					covbuf[0][0] =estcov[0][0][i];
					covbuf[0][1] =estcov[0][1][i];
					covbuf[1][0] =estcov[1][0][i];
					covbuf[1][1] =estcov[1][1][i];
					//调用Elipsnorm_w()函数
					Elipsnorm(mubuf,covbuf,2);
				}
				for(int i=0;i!=2;++i)
					delete []covbuf[i];
				delete []covbuf;
				delete[]mubuf;	
			}*/
			if (dlength[countf-1]<mindl)
			{
				para->bestk =k;
				mindl = dlength[countf-1];
				//将estpp，estmu，estcov保存
				/*para.bestpp =new float [k];
				para.bestmu =new float *[dimens];
				para.bestcov =new float **[dimens];
				for (int i =0;i!=dimens;++i)
				{
					para.bestcov[i] =new float *[dimens];
					para.bestmu[i] =new float [dimens];
				}

				for (int i=0;i!=dimens;++i)
					for (int j = 0;j!=dimens;++j)
					{
						para.bestcov[i][j] = new float [k];
					}
					for (int i =0;i!=k;++i)
					{
						para.bestpp[i] =estpp[i];
						for (int j=0;j!=dimens;++j)
						{
							para.bestmu[j][i] =estmu[j][i];
							for(int m=0;m!=dimens;++m)
								para.bestcov[m][j][i] =estcov[m][j][i];
						}
					}*/
			}
			if (k>kmin)
			{
				float minval =estpp[0];
				int minpos=0;
				for (int i=0;i!=k;++i)
				{
					if (estpp[i]<minval)
					{
						minval =estpp[i];
						minpos = i;
					}
				}
				vector<int>min_pos;
				for(int i =0;i!=subclass_index.size();++i)
				{
					if(subclass_index[i]==1)
						min_pos.push_back(i);
				}
				subclass_index[min_pos[minpos]]=0;
				subclass_index0.push_back(subclass_index);
				//去除estmu的comp列，estcov的第3维的comp，estpp的comp个，semi_indic的comp行
				for (int i=minpos;i!=k-1;++i)
				{
					estpp[i]=estpp[i+1];
					for(int j=0;j!=dimens;++j)
					{
						estmu[j][i] =estmu[j][i+1];
						for(int m=0;m!=dimens;++m)
							estcov[j][m][i] = estcov[j][m][i+1];
					}
					for(int j=0;j!=imgsize;++j)
						semi_indic[i][j] =semi_indic[i+1][j];
				}
				k=k-1;
				float sumpp =0;
				for(int i=0;i!=k;++i)
					sumpp+=estpp[i];
				for(int i=0;i!=k;++i)
				{
					estpp[i] =estpp[i]/sumpp;
					//memset(semi_indic[i],0,imgsize*sizeof(float));
				}
				countf++;
				//重置semi_indic
				for (int i=0;i!=k;++i)
				{
					/*float *mubuf = new float[dimens];
					float **covbuf = new float*[dimens];*/
					vector<float>mubuf(dimens,0);
					vector<vector<float>>covbuf(dimens,vector<float>(dimens,0));
					for (int j =0;j!=dimens;++j)
					{
						/*covbuf[j] =new float [dimens];
						memset(covbuf[j],0,dimens*sizeof(float));*/
						mubuf[j] =estmu[j][i];    //取estmu的第i列赋值给mubuf
					}
					//将estcov[][][i]赋值给covbuf
					for (int j=0;j!=dimens;++j)
						covbuf[j][j] =estcov[j][j][i];
					multinorm(data,mubuf,covbuf,semi_indic[i]);

					for (int j=0;j!=imgsize;++j)
					{
						all_indic[i][j] =semi_indic[i][j]*estpp[i];
						if (all_indic[i][j]<REAL_MIN)
							all_indic[i][j] =REAL_MIN;
					}
					//释放空间
					/*for (int j=0;j!=dimens;++j)
						delete[]covbuf[j];
					delete[]mubuf;
					delete[]covbuf;*/
				}
				//添加loglike新元素
				loglikebuf=0;
				for (int i=0;i!=imgsize;++i)
				{
					float sumbuf =0;
					for(int j=0;j!=k;++j)
						sumbuf +=all_indic[j][i];
					if(sumbuf==0)
						sumbuf = REAL_MIN;
					loglikebuf +=log(sumbuf);
				}
				loglike.push_back(loglikebuf);
				//求log（estpp）的和
				float sumppl =0;
				for (int i=0;i!=k;++i)
				{
					sumppl+=log(estpp[i]);
				}

				dlengthbuf = -loglikebuf+(nparscover2*sumppl)+(nparscover2+0.5)*k*log(float(imgsize));
				dlength.push_back(dlengthbuf);
				kappas.push_back(k);
			}
			else if(k==kmin)
			{
				k_cont=0;
				vector<float>minpp_buf;
				vector<float>minmu_buf;
				vector<float>mincov_buf;
				//将minpp,minmu，mincov放入到minpp_all,minmu_all，mincov_all
				for (int i=0;i!=k;++i)
				{
					minpp_buf.push_back(estpp[i]);
					minmu_buf.push_back(estmu[0][i]);
					mincov_buf.push_back(estcov[0][0][i]);
				}
				para->minpp_all.push_back(minpp_buf);
				para->minmu_all.push_back(minmu_buf);
				para->mincov_all.push_back(mincov_buf);
			}

		}


		//释放内存
		for (int i=0;i!=dimens;++i)
		{
			delete[]estmu[i];
		}
		delete[]estmu;
		for (int i=0;i!=dimens;++i)
		{
			delete[]globcov[i];
		}
		delete[]globcov;
		for (int i=0;i!=dimens;++i)
			for (int j = 0;j!=dimens;++j)
				delete[]estcov[i][j];
	
		for(int i=0;i!=dimens;++i)
			delete[]estcov[i];

		delete[]estcov;
		delete[]estpp;
		//delete []mix;

}