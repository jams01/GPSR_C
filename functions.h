#ifndef FUNCTIONS_H
#define FUNCTIONS_H
#define    PI       3.141592652598793
#define    SQH      0.707106781186547
#define    SQRT_2   1.414213562373095
#include<math.h>
#include<fstream>
#include<iostream>
#include<stdio.h>
#include<stdarg.h>
#include<mex.h>
#include<matrix.h>
#include<mkl.h>
#include<cmath>
#include"mkl_lapacke.h"
#include "omp.h"
#include<mkl_spblas.h>
//#include<Eigen/Sparse>
#include<type_traits>
#include<limits>
#include<fftw3.h>
//#include<wavelet2s.h>
//#include<mcheck.h>


//using namespace Eigen;


using namespace std;
//typedef Matrix<double,Dynamic,1> MatrixType;
//typedef Map<MatrixType> MapType;
//typedef SparseMatrix<double,ColMajor,std::make_signed<mwIndex>::type> MatlabSparse;

void kronerDCTinverse(double *x, double *lpf, size_t M, size_t N, size_t L, int lenfil, double *x1);
void kronerDCTdirect(double *x, double *lpf, size_t M, size_t N, size_t L, int lenfil,double *x1);
void mexFunction(int nlhs, mxArray *plhs[],int nrhs, const mxArray *prhs[]);
void convertandcopy(mwSize n,mwIndex *input1,mwIndex *input2,MKL_INT *output1,MKL_INT *output2,mwSize nz,mwIndex *input3,MKL_INT *output3);
sparse_matrix_t matlab_to_spcblas(const mxArray * mat);
//Map<MatlabSparse> matlab_to_eigen_sparse(const mxArray * mat);
void mTimes(sparse_matrix_t A, double *x, double *res, double *G, mwSize tm, mwSize tmy, mwSize m, mwSize n, mwSize l, bool trans, int par);
void eval(double* m1, double val, size_t size, int flag, double* result);
void maxi(double *m, double val, size_t size, int flag, double *result);
void maxi(double *m, double *m1, size_t size, int flag, double *result);
void GPSR_BB(double* y, sparse_matrix_t A, double tau, double* x, double tolA , double* G , mwSize tmy, mwSize tm, mwSize m, mwSize n, mwSize l, int par, mwSize maxiter1, bool verbose1);

void idpwt2(double *wc,size_t nr,size_t nc,int ell,int J,double *hpf,double *lpf,int lenfil,double *img,double *temp);
void copydouble(double *x,double *y,size_t n);

void uplo(double *x,int n,double *lpf,int m,double *y);
void uphi(double *x,int n,double *hpf,int m,double *y);
void dpwt2(double *sig,int nr,int nc,int ell,int J,double *hpf,double *lpf,int lenfil,double *wc,double *temp);
void downlo(double *x,int n,double *lpf,int m,double *y);
void downhi(double *x,int n,double *hpf,int m,double *y);
void mirrorfilt(double *lpf,double *hpf,int lenfil);

#endif // FUNCTIONS_H
