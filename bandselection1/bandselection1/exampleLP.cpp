/*
 *	This file is part of qpOASES.
 *
 *	qpOASES -- An Implementation of the Online Active Set Strategy.
 *	Copyright (C) 2007-2017 by Hans Joachim Ferreau, Andreas Potschka,
 *	Christian Kirches et al. All rights reserved.
 *
 *	qpOASES is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation; either
 *	version 2.1 of the License, or (at your option) any later version.
 *
 *	qpOASES is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *	See the GNU Lesser General Public License for more details.
 *
 *	You should have received a copy of the GNU Lesser General Public
 *	License along with qpOASES; if not, write to the Free Software
 *	Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */


/**
 *	\file examples/exampleLP.cpp
 *	\author Hans Joachim Ferreau
 *	\version 3.2
 *	\date 2008-2017
 *
 *	Very simple example for solving a LP sequence using qpOASES.
 */



#include <qpOASES.hpp>
#include <iostream>
#include "Eigen/Dense"
#include "Eigen/Sparse"
#include "Eigen/Core" 
using namespace std;
using namespace Eigen;
USING_NAMESPACE_QPOASES
typedef Eigen::Matrix<int, Eigen::Dynamic, Eigen::Dynamic>				            Matrix_i;
typedef Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic>						Matrix_t;

#if 0
/** Example for qpOASES main function solving LPs. */
int main( )
{


	/* Setup data of first LP. */
	real_t A[1*2] = { 1.0, 1.0 };
	real_t g[2] = { 1.5, 1.0 };
	real_t lb[2] = { 0.5, -2.0 };
	real_t ub[2] = { 5.0, 2.0 };
	real_t lbA[1] = { -1.0 };
	real_t ubA[1] = { 2.0 };

	/* Setup data of second LP. */
	real_t g_new[2] = { 1.0, 1.5 };
	real_t lb_new[2] = { 0.0, -1.0 };
	real_t ub_new[2] = { 5.0, -0.5 };
	real_t lbA_new[1] = { -2.0 };
	real_t ubA_new[1] = { 1.0 };


	/* Setting up QProblem object with zero Hessian matrix. */
	QProblem example( 2,1,HST_ZERO );

	Options options;
 	//options.setToMPC();
	example.setOptions( options );

	/* Solve first LP. */
	int_t nWSR = 10;
	example.init( 0,g,A,lb,ub,lbA,ubA, nWSR,0 );

	///* Solve second LP. */
	//nWSR = 10;
	//example.hotstart( g_new,lb_new,ub_new,lbA_new,ubA_new, nWSR,0 );


	/* Get and print solution of second LP. */
	real_t xOpt[2];
	example.getPrimalSolution( xOpt );
	printf( "\nxOpt = [ %e, %e ];  objVal = %e\n\n", xOpt[0],xOpt[1],example.getObjVal() );

	return 0;
}
#endif


/*
 *	end of file
 */



 //--------------------LinearLeastSquaresProblemSolver (lsqlin in Matlab) --------------------------------------------------------//
 // similar to lsqlin in Matlab
 //
 // find x that minimizes 0.5*||C*x - d||^2  
 //                  <=>  0.5*x'*(C'*C)*x - x'*(C'*d) + 0.5*d'*d  
 //                  <=>  0.5 * x'*H*x + x'*g with H = C'*C, and g = -C'*d;
 //
 // subject to:
 // lb_A <= A*x <= ub_A
 // lb_x <= x <= ub_x
 
 ////matlab  lsqlin
// lsqlin(C,d,A,b,Aeq,beq,lb,ub)



////qpoase  
//min   1 / 2 * x'Hx + x'g
//s.t.lb <= x <= ub
//lbA <= Ax <= ubA
//



//数组转eigen
Matrix_t eigen2matrix(double* input,int rows,int cols)
{
	Matrix_t output(rows,cols);
	for (int i=0;i<rows;i++)
	{
		for (int j=0;j<cols;j++)
		{
			output(i, j) = input[i*rows+j];
		}
	}
	return output;
}
//eigen转数组
void eigen2matrix(Matrix_t input,double** output)
{
	
	for (int i = 0; i < input.rows(); i++)
	{
		for (int j = 0; j < input.cols(); j++)
		{
			output[i][j] = input(i,j);
		}
	}
}


void my_lsqlin(Matrix_t C_M, Matrix_t d_M, /*Matrix_t A_M, Matrix_t b_M,*/ Matrix_t Aeq_M, Matrix_t beq_M, Matrix_t lb_M, Matrix_t ub_M)
{	
	Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic, RowMajor> H_M = C_M.transpose()*C_M;	
	Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic, RowMajor> g_M = -C_M.transpose()*d_M;
	real_t* H = H_M.data();
	real_t* g = g_M.data();
	cout << *(H ) << ends << *(H + 1) << ends << *(H + 2) << ends << *(H+3 )<<*(g+1)<<endl;
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
	QProblem example(2, 1, HST_ZERO);

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


#if 1
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
