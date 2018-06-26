//https://blog.csdn.net/u010420283/article/details/53131288 envi裁剪

#include "gdal_priv.h"
#include "Eigen/Dense"
#include "Eigen/Sparse"
#include "Eigen/Core" 
#include <iostream>
#include <qpOASES.hpp>
#include <math.h>
#include <Estimate_para.h>


using namespace std;
using namespace Eigen;
using namespace qpOASES;
typedef Eigen::Matrix<int, Eigen::Dynamic, Eigen::Dynamic>				            Matrix_i;
typedef Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic>						Matrix_t;

//typedef Eigen::Array<double, Eigen::Dynamic, Eigen::Dynamic>						ArrayXXd;


Matrix_t my_corref(Matrix_t Data_input)
{
	int Row_c, Col_c;
	double data_stdI=0.0 , data_stdJ=0.0 , data_stdIJ = 0.0; //标准差
	double cor_ref = 0.0;
	Row_c = Data_input.rows();
	Col_c = Data_input.cols();
	Matrix_t Corref;
	Corref.resize(Row_c, Row_c);
	VectorXd bandselectedI(Col_c);               //[波段]*[行*列],X
	VectorXd bandselectedJ(Col_c);               //[波段]*[行*列],Y
	VectorXd bandselectedIJ(Col_c);              //[波段]*[行*列],X*Y
	VectorXd bandselectedMean(Col_c);            //[波段]*[行*列],均值
	bandselectedMean.setOnes(Col_c);
	VectorXd bandselectedMinusMeanI(Col_c);      //[波段]*[行*列]，X去均值
	VectorXd bandselectedMinusMeanJ(Col_c);      //[波段]*[行*列]，Y去均值
	VectorXd bandselectedMinusMeanIJ(Col_c);     //[波段]*[行*列]，X*Y去均值
	
	for (int i=0;i<Row_c;i++)
	{
		for (int j = i; j < Row_c; j++)
		{
			bandselectedI = Data_input.row(i);
			bandselectedJ = Data_input.row(j);
			// X
			// 去均值
			bandselectedMinusMeanI = bandselectedI - bandselectedMean*bandselectedI.mean();
			//cout << bandselectedMinusMeanI.mean() << endl;
			// 计算标准差
			data_stdI = sqrt((bandselectedMinusMeanI.norm()*bandselectedMinusMeanI.norm()) / (bandselectedMinusMeanI.size() - 1));
			//cout << data_stdI << endl;
			// Y
			// 去均值
			bandselectedMinusMeanJ = bandselectedJ - bandselectedMean*bandselectedJ.mean();
			//cout << bandselectedMinusMeanJ.mean() << endl;
			// 计算标准差
			data_stdJ = sqrt((bandselectedMinusMeanJ.norm()*bandselectedMinusMeanJ.norm()) / (bandselectedMinusMeanJ.size() - 1));
			//cout << data_stdJ << endl;

			// XY
			bandselectedIJ = bandselectedI.array() * bandselectedJ.array();
			//cout << bandselectedIJ.size() << endl;
			//cout << bandselectedIJ.mean() << endl;
			bandselectedMinusMeanIJ = bandselectedIJ - bandselectedMean*bandselectedIJ.mean();//去均值
			//data_stdIJ = sqrt((bandselectedMinusMeanIJ.norm()*bandselectedMinusMeanIJ.norm()) / (bandselectedMinusMeanIJ.size() - 1));
			//cout << data_stdIJ << endl;


			cor_ref = (bandselectedIJ.mean() - bandselectedI.mean() * bandselectedJ.mean()) / data_stdI / data_stdJ;
			//cout << cor_ref << endl;

			Corref(i,j) = cor_ref;
			Corref(j,i) = cor_ref;

		}
	}
	//cout << Corref.row(0) << endl;
	return Corref;

	//bandselectedI = Data_input.row(0);
	//bandselectedJ = Data_input.row(1);

	//// X
	//// 去均值
	//bandselectedMinusMeanI = bandselectedI - bandselectedMean*bandselectedI.mean();
	//cout << bandselectedMinusMeanI.mean() << endl;
	//// 计算标准差
	//data_stdI = sqrt((bandselectedMinusMeanI.norm()*bandselectedMinusMeanI.norm()) / (bandselectedMinusMeanI.size() - 1));
	//cout << data_stdI << endl;
	//// Y
	//// 去均值
	//bandselectedMinusMeanJ = bandselectedJ - bandselectedMean*bandselectedJ.mean();
	//cout << bandselectedMinusMeanJ.mean() << endl;
	//// 计算标准差
	//data_stdJ = sqrt((bandselectedMinusMeanJ.norm()*bandselectedMinusMeanJ.norm()) / (bandselectedMinusMeanJ.size() - 1));
	//cout << data_stdJ << endl;


	//// XY
	//bandselectedIJ = bandselectedI.array() * bandselectedJ.array();
	//cout << bandselectedIJ.size() << endl;
	//cout << bandselectedIJ.mean() << endl;
	//bandselectedMinusMeanIJ = bandselectedIJ - bandselectedMean*bandselectedIJ.mean();//去均值
	//data_stdIJ = sqrt((bandselectedMinusMeanIJ.norm()*bandselectedMinusMeanIJ.norm()) / (bandselectedMinusMeanIJ.size() - 1));
	//cout << data_stdIJ << endl;


	//cor_ref = (bandselectedIJ.mean() - bandselectedI.mean() * bandselectedJ.mean())/ data_stdI /data_stdJ;
	//cout << cor_ref << endl;
}


void my_lsqlin(Matrix_t C_M, Matrix_t d_M, /*Matrix_t A_M, Matrix_t b_M,*/ Matrix_t Aeq_M, Matrix_t beq_M, Matrix_t lb_M, Matrix_t ub_M)
{
	Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic, RowMajor> H_M = C_M.transpose()*C_M;
	Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic, RowMajor> g_M = -C_M.transpose()*d_M;
	real_t* H = H_M.data();
	real_t* g = g_M.data();
	cout << *(H) << ends << *(H + 1) << ends << *(H + 2) << ends << *(H + 3) << *(g + 1) << endl;
	cout << *(g) << ends << *(g + 1) << endl;

	//cout << "H_M" << endl << H_M << endl;
	//cout << "g_M" << endl << g_M << endl;
	real_t* A = Aeq_M.data();
	cout << "Aeq_M" << endl << Aeq_M << endl;
	real_t* lb = lb_M.data();
	cout << "lb_M" << endl << lb_M << endl;
	real_t* ub = ub_M.data();
	cout << "ub_M" << endl << ub_M << endl;
	real_t* lbA = beq_M.data();
	cout << "beq_M" << endl << beq_M << endl;
	real_t* ubA = beq_M.data();
	cout << "beq_M" << endl << beq_M << endl;

	/* Setting up QProblem object with zero Hessian matrix. */
	QProblem example(2, 1);

	Options options;
	//options.setToMPC();
	example.setOptions(options);

	/* Solve first LP. */
	int_t nWSR = 10;
	example.init(H, g, A, lb, ub, lbA, ubA, nWSR, 0);


	/* Get and print solution of second LP. */
	real_t xOpt[2];
	example.getPrimalSolution(xOpt);
	printf("\nxOpt = [ %e, %e ];  objVal = %e\n\n", xOpt[0], xOpt[1], example.getObjVal());


}


#if 0
int main()
{
	Matrix_t C(2, 2), d(2, 1),/* A(1,2),b(1,2),*/Aeq(1, 2), Beq(1, 1), lb(2, 1), ub(2, 1);

	C << 1.0, 0.0,
		0.0, 0.5;
	d << 0.5, 0.25;
	Aeq << 1.0, 1.0;
	Beq << 1.0;
	lb << 0.0, 0.0;
	ub << 1.0, 1.0;
	my_lsqlin(C, d, Aeq, Beq, lb, ub);


	//Matrix_t a(2,2);
	//a << 1, 2, 3, 4;
	//Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic, RowMajor> b = a;
	//cout << *(b.data() + 1);



	return 0;
}

#endif


//int main()
//{
//	ArrayXXd a(2, 2), b(1, 2);
//	a << 1, 2, 3, 4;
//	b << 1, 2;
//	a.col(1) = 6;
//	cout << a;
//	cout << b / a;
//	return 0;
//}
//





#if 1


int main()
{
	/************************************************************************/
	/*                    读取，按照[波段]*[行*列]排序                      */
	/************************************************************************/
	GDALDataset * DataSet;          // 图像文件
	GDALAllRegister();              //注册驱动 
	CPLSetConfigOption("GDAL_FILENAME_IS_UTF8", "NO");//设置可以读入带有中文的路径
	char * filepath("roi.tif");
	DataSet = (GDALDataset *)GDALOpen(filepath, GA_ReadOnly);//文件的打开使用的是GDALOpen函数
	int xlen = DataSet->GetRasterXSize();
	int ylen = DataSet->GetRasterYSize();
	int zlen = DataSet->GetRasterCount();
	int imgsize = xlen * ylen;
	GDALRasterBand **pBand;
	pBand = new GDALRasterBand *[zlen]; //新建波段





	Matrix_t spectrum(zlen, imgsize);//[波段]*[行*列]
	for (int k = 0; k != zlen; ++k)//每一层循环
	{
		vector<float>Image_Gray(imgsize, 0);
		pBand[k] = DataSet->GetRasterBand(k + 1);//选择波段
		if (pBand[k] == NULL)
		{
			cout << "pBand Fail" << endl;
		}
		unsigned short *Image_Gray0 = (unsigned short *)CPLMalloc(sizeof(unsigned short)*imgsize);
		pBand[k]->RasterIO(GF_Read, 0, 0, xlen, ylen, Image_Gray0, xlen, ylen, GDT_UInt16, 0, 0);//提取图像的灰度
		for (int i = 0; i<xlen; i++)
		{
			for (int j = 0; j<ylen; j++)
			{
				spectrum(k, i*ylen + j) = Image_Gray0[i*ylen + j];
				//gsl_matrix_set(spectrum,k,i*ylen+j,(double)Image_Gray0[i*ylen + j]);
			}
		}
		CPLFree(Image_Gray0);
	}

	//std::cout << "Here is the matrix spectrum:\n" << spectrum.block(190,0,1,128*128) << std::endl;
	Matrix_t Corref;
	Corref = my_corref( spectrum );
	cout << Corref << endl;
	cout << Corref.rows() << endl;
	cout << Corref.cols() << endl;
	//Corref.resize(2, 2);
	//Corref << 1.2, 2.1,
	//	2.1, 4;

	SelfAdjointEigenSolver<MatrixXd> es(Corref);
	cout << "The eigenvalues of A are:" << endl << es.eigenvalues() << endl;
	cout << "The matrix of eigenvectors, V, is:" << endl << es.eigenvectors() << endl << endl;

	//MatrixXd A = MatrixXd::Random(6, 6);
	//cout << "Here is a random 6x6 matrix, A:" << endl << A << endl << endl;
	//EigenSolver<MatrixXd> es1(Corref);
	//cout << "The eigenvalues of A are:" << endl << es1.eigenvalues() << endl;
	//cout << "The matrix of eigenvectors, V, is:" << endl << es1.eigenvectors() << endl << endl;



	/*
	Matrix_t bandselected(1, imgsize);//[波段]*[行*列]
	VectorXd bandselectedI(imgsize);//[波段]*[行*列]
	VectorXd bandselectedJ(imgsize);//[波段]*[行*列]
	VectorXd bandselectedIJ(imgsize);//[波段]*[行*列]
	VectorXd bandselectedMean(imgsize);//[波段]*[行*列]
	VectorXd bandselectedMinusMean(imgsize);//[波段]*[行*列]

	VectorXd bandselectedIdx(29);//[波段]*[行*列]
	//bandselected = spectrum.row(0);
	//std::cout << "Here is the matrix spectrum:\n" << bandselected << std::endl;

	bandselectedIdx << 26, 9, 29, 27, 30, 28, 31, 25, 8, 22, 24, 32, 23, 10, 20, 21, 19, 1, 4, 17, 16, 15, 2, 5, 6, 11, 14, 12, 13;
	//std::cout << "Here is the matrix spectrum:\n" << bandselectedIdx << std::endl;
	//std::sort(bandselectedIdx.data(), bandselectedIdx.data() + bandselectedIdx.size());
	//cout << bandselectedIdx;
	//cout << bandselectedIdx.col(1);
	//double Idx = bandselectedIdx(1,1);
	double Idx = bandselectedIdx(1);
	bandselectedI = spectrum.row(1);
	bandselectedJ = spectrum.row(2);
	cout << "Spectrum has rows:"<<spectrum.rows() << endl;
	cout << "Spectrum has columns:" << spectrum.cols() << endl;
	cout << "bandselectedI has size:" << bandselectedI.size() << endl;
	cout << bandselectedI.mean() << endl;
	bandselectedMean.setOnes(imgsize);
	//cout << bandselectedMean << endl;
	
	bandselectedMinusMean = bandselectedI - bandselectedMean*bandselectedI.mean();//去均值
	cout << bandselectedMinusMean.mean() << endl;
	double data_std; //标准差
	// 计算标准差
	data_std = sqrt((bandselectedMinusMean.norm()*bandselectedMinusMean.norm()) / (bandselectedMinusMean.size() - 1));
	cout << data_std << endl;

	bandselectedIJ = bandselectedI.array() * bandselectedJ.array();
	cout << bandselectedIJ.size() << endl;
	cout << bandselectedIJ.mean() << endl;
	bandselectedMinusMean = bandselectedIJ - bandselectedMean*bandselectedIJ.mean();//去均值
	data_std = sqrt((bandselectedMinusMean.norm()*bandselectedMinusMean.norm()) / (bandselectedMinusMean.size() - 1));
	cout << data_std << endl;




	//double fenzi = 
	//cout << bandselected << endl;


	//int Idx = 2;
	//int Idx_band;
	MatrixXd Idx_band;
	Idx_band.setZero(1, zlen);
	cout << Idx_band;
	cout << bandselectedIdx.col(0);
	bandselected = spectrum.row(Idx);
	cout << bandselected << endl;
	*/
	Parameter * para = Parameter::getInstance();
	para->init_GrayImg(DataSet);
	int classnum = 4; //类别数
	CEstimate_para estimate;
	for (int i = 0; i != para->imgdata.size() - 3; ++i)
	{
		int bandmin = classnum;
		int bandmax = classnum + 6;
		vector<vector<float> >imgdatabuf;
		imgdatabuf.push_back(para->imgdata[i]);
		estimate.Mixture(para, imgdatabuf, bandmin, bandmax, 0, 1e-4, 0, 5000);
		//输出估计结果到列表中
		//int col = i*classnum;
		//wchar_t num0[50] = { 0 };
		//wchar_t num1[50] = { 0 };
		//wchar_t num2[50] = { 0 };
		for (int j = 0; j<classnum; ++j)
		{
			cout << para->minmu_all[i][j] << endl;
			cout << para->mincov_all[i][j] << endl;
			cout << para->minpp_all[i][j] << endl;
			//swprintf(num0, L" %f ", para->minmu_all[i][j]);
			//swprintf(num1, L"%f", para->mincov_all[i][j]);
			//swprintf(num2, L"%f", para->minpp_all[i][j]);
			//XList_SetItemText(m_hList, col + j, 2, num0);
			//XList_SetItemText(m_hList, col + j, 3, num1);
			//XList_SetItemText(m_hList, col + j, 4, num2);
			//XList_SetItemText(m_hList, col + j, 5, L"估计完成");

			//if (col + j + classnum<classnum*bandnum)
			//	XList_SetItemText(m_hList, col + j + classnum, 5, L"正在估计");

			////刷新控件
			//XEle_RedrawEle(m_hEle);
		}
	}





	//char fid[]="data.txt";
	//outputMatrix(spectrum,fid);
	/************************************************************************/
	/*                             IEA                                      */
	/************************************************************************/
	int P = 6;
	Matrix_t R = spectrum;
	Matrix_t M = R.rowwise().mean();
	Matrix_t ED = Matrix_t::Zero(P, 1);
	ArrayXXd M_expand(M.rows(), R.cols());
	for (int i = 0; i<R.rows(); i++)
	{
		M_expand.row(i) = M(i, 0);
	}
	ArrayXXd  dR = R.array() - M_expand*M_expand / R.array();






	/************************************************************************/
	/*                             ISMA                                      */
	/************************************************************************/

	return 0;
}


#endif