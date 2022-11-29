
#include "functions.h"

// receive y,A,tau,toalA,maxiter,G makeOnFilter,par,tmx,m,n,l,verbose;

//typedef SparseMatrix<double,ColMajor,std::make_signed<mwIndex>::type> MatlabSparse;

void mexFunction(int nlhs, mxArray *plhs[],
                 int nrhs, const mxArray *prhs[])
{
    if(nrhs != 12) {
        mexErrMsgIdAndTxt("MyToolbox:arrayProduct:nrhs",
                          "12 inputs required.");
    }
    if(nlhs != 1) {
        mexErrMsgIdAndTxt("MyToolbox:arrayProduct:nlhs",
                          "One output required.");
    }
    mwSize maxIter;
    double* y;
    double tau,tolA;
    double* x;
    double* G;
    //Map<MatlabSparse> A = matlab_to_eigen_sparse(prhs[1]);
    sparse_matrix_t A = matlab_to_spcblas(prhs[1]);
    bool verbose;
    mwSize tmy;
    mwSize tmx;
    int par;
    // Si la función en Matlab recibe funcion(A,B)
    // aca recibo A así 
    // A =static_cast<double*>(mxGetDoubles(prhs[0]));
    // B = static_cast<double*>(mxGetDoubles(prhs[1]));
    y =static_cast<double*>(mxGetDoubles(prhs[0]));
    tmy =static_cast<mwSize>(mxGetM(prhs[0]));
    tau = mxGetScalar(prhs[2]);
    tolA = mxGetScalar(prhs[3]);
    maxIter =static_cast<mwSize>(mxGetScalar(prhs[4]));
    G=static_cast<double*>(mxGetDoubles(prhs[5]));
    par = static_cast<int>(mxGetScalar(prhs[6]));
    tmx=static_cast<mwSize>(mxGetScalar(prhs[7]));
    mwSize m=static_cast<mwSize>(mxGetScalar(prhs[8]));
    mwSize n=static_cast<mwSize>(mxGetScalar(prhs[9]));
    mwSize l=static_cast<mwSize>(mxGetScalar(prhs[10]));
    verbose = static_cast<bool>(mxGetScalar(prhs[11]));
    plhs[0] = mxCreateDoubleMatrix(tmx,1,mxREAL);
    x = static_cast<double*>(mxGetDoubles(plhs[0]));
    //omp_set_nested(1);
    // Y aca llamo la función de C++
    GPSR_BB(y,A,tau,x,tolA,G,tmy,tmx,m,n,l,par,maxIter,verbose);
    return;

}


void unpackdouble(double *x,int n,int nc,int k,double *y){
   int i;
   for( i=0; i < n; i++)
        *y++ = x[k+nc*i];
 }

void packdouble(double *x,int n,int nc,int k,double *y){
   int i;
   //cblas_dcopy(n,x,1,&y[k],nc);
   for( i=0; i < n; i++)
         y[k+nc*i] = *x++;
 }

void downlo(double *x,int n,double *lpf,int m,double *y){
           int n2, mlo, mhi, h, j;
           double s;
           double *t1,*t2,*t3,*t4;
           /*lowpass version */
           n2 = n/2;
           mlo = m /2;
           mhi = n2-mlo;
           if(2*mhi + (m-1) >= n) --mhi;
           if(mhi < 0) mhi = -1;
           //t1=y;
           t4=x;
           for( t1=y; t1<=y+mhi; t1++){
                s = 0.;
                //t2=lpf;
                t3=t4;
                t4=t4+2;
                for( t2=lpf; t2 < lpf + m; t2++){
                    s += *t2*(*t3);
                    //t2++;
                    t3++;
                }
                //y[i] = s;
                *t1=s;
                //t1++;
            }

            /* fix up edge values */
            h=(mhi+1);
            t4=&x[2*h];
            for( t1= &y[mhi+1]; t1<&y[mhi+1]+n2; t1++){
                s = 0.;
                j = 2*h;
                //j=j+(2);
                t3=t4;
                t4=t4+2;
                for( t2=lpf; t2 <lpf + m; t2++){
                    if(j >= n){
                        j -= n;
                        t3=&x[j];
                    }
                    s += (*t2)*(*t3);
                    j++;
                    t3++;
                }
                *t1 = s;
                //j=j+2;
                h++;
            }



}

/*void adddouble(double *x,double *y,int n,double *z){
   while(n--) *z++ = *x++ + *y++;
}*/

void downhi(double *x,int n,double *hpf,int m,double *y){
       int n2, mlo, i, h, j;
       double s;

           /* highpass version */
           n2 = n/2;
           mlo = m/2-1;
       if(2*mlo+1 - (m-1) < 0) mlo++;
           for( i= mlo; i<n2; i++) {
                s = 0.;
                for( h=0; h < m; h++)
                     s += hpf[h]*x[2*i+1-h];
                y[i] = s;
           }
           if(mlo > n2) mlo = n2;
        /* fix up edge values */
           for( i= 0; i<mlo; i++) {
                s = 0.;
                j = 2*i+1;
                for( h=0; h < m; h++) {
                     if(j < 0) j += n;
                     s += hpf[h]*x[j];
                     --j;
                }
                y[i] = s;
           }
}


void dpwt2(double *sig,int nr,int nc,int ell,int J,double *hpf,double *lpf,int lenfil,double *wc,double *temp){
        double *wcplo,*wcphi,*templo,*temphi;
        int k,j,nj;
        cblas_dcopy(nr*nc,sig,1,wc,1);
        //copy(sig,nr*nc,wc);
        templo = &temp[nr];
        temphi = &temp[2*nr];
               nj = nr;
               for( j=(J-1); j >= ell; --j){
                       for( k=0; k < nj; k++){
                           wcplo = &wc[k*nr];
                           wcphi = &wc[k*nr + nj/2];
                           //copydouble(wcplo,temp,nj);
                           cblas_dcopy(nj,wcplo,1,temp,1);
                           downlo(temp, nj, lpf,lenfil,wcplo);
                           downhi(temp, nj, hpf,lenfil,wcphi);
                       }
                       for( k=0; k < nj; k++){
                           //cblas_dcopy(nj,&wc[k],nc,temp,1);
                           unpackdouble(wc,nj,nc,k,temp);
                           downlo(temp, nj, lpf,lenfil,templo);
                           downhi(temp, nj, hpf,lenfil,temphi);
                           cblas_dcopy(nj/2,templo,1,&wc[k],nc);
                           //packdouble(templo,nj/2,nc,k,wc);
                           cblas_dcopy(nj/2,temphi,1,&wc[(nj/2*nr)+k],nc);
                           //packdouble(temphi,nj/2,nc,k,&wc[nj/2*nr]);
                       }
                       nj = nj/2;
               }
}




void idpwt2(double *wc,size_t nr,size_t nc,int ell,int J,double *hpf,double *lpf,int lenfil,double *img,double *temp){
        //mirrorfilt(lpf,hpf,lenfil);
        double *wcplo,*wcphi,*templo,*temphi,*temptop;
        size_t nj,tmp;
        int k,j;
        tmp=nr*nc;
        //copydouble(wc,img,tmp);
        cblas_dcopy(tmp,wc,1,img,1);
        templo = &temp[nr];
        temphi = &temp[2*nr];
        temptop = &temp[3*nr];

               nj = 1;
               for( k=0; k < ell; k++) nj *=2;

               for( j=ell; j < J; j++){
                        for( k=0; k < 2*nj; k++){
                           cblas_dcopy(nj,&img[k],nc,templo,1);
                           //unpackdouble(img,nj,nc,k,templo);
                           cblas_dcopy(nj,&img[(nj*nr)+k],nc,temphi,1);
                           //unpackdouble(&img[nj*nr],nj,nc,k,temphi);
                           uplo(templo, nj, lpf,lenfil,temp);
                           uphi(temphi, nj, hpf,lenfil,temptop);
                           vdAdd(nj*2,temp,temptop,temp);
                           //adddouble(temp,temptop,nj*2,temp);
                           cblas_dcopy(nj*2,temp,1,&img[k],nc);
                           //packdouble(temp,nj*2,nc,k,img);
                       }

                       for( k=0; k < 2*nj; k++){
                           wcplo = &img[k*nr];
                           wcphi = &img[k*nr + nj];
                           //copydouble(wcplo,temp,nj);
                           cblas_dcopy(nj,wcplo,1,temp,1);
                           uplo(wcplo, nj, lpf,lenfil,templo);
                           uphi(wcphi, nj, hpf,lenfil,temphi);
                           vdAdd(nj*2,templo,temphi,wcplo);
                           //adddouble(templo,temphi,nj*2,wcplo);
                       }
                       nj *= 2;
               }
}



void kronerDCTdirect(double *x, double *lpf, size_t M, size_t N, size_t L,int lenfil,double *x1){
    //copydouble(x,x1,M*N*L);
    cblas_dcopy(M*N*L,x,1,x1,1);
    int J = (int)(log((double)M)/log(2.0));
    static double *hpf=new double[lenfil];
    int ell=1;
    size_t n=M*N;
    int m =(int)(log((double)L)/log(2.0));
    //static double *y=new double[L];
    int nutr = 1;
    //static double *tab=new double[L*2];//tabla de senos y coseno
    static double *tempa=static_cast<double*>(mkl_malloc(nutr*N*3*sizeof(double),64));
    static double *tmpa=static_cast<double*>(mkl_malloc(nutr*L*sizeof(double),64));//new double[L];
    static double *tmp1a=static_cast<double*>(mkl_malloc(nutr*L*sizeof(double),64));//new double[L];
    static double *slidea=static_cast<double*>(mkl_malloc(nutr*n*sizeof(double),64));//new double[n];
    mirrorfilt(lpf,hpf,lenfil);
    //#pragma omp parallel num_threads(nutr) shared(tempa,tmpa,tmp1a,slidea)
    //{
    double *tmp2;
    double *temp = &tempa[0*N*3];
    double *tmp = &tmpa[0*L];
    double *tmp1 = &tmp1a[0*L];
    double *slide = &slidea[0*n];
    static fftw_plan plan = fftw_plan_r2r_1d(L, tmp, tmp1, FFTW_REDFT10, FFTW_MEASURE);
    //#pragma omp for
    for(size_t j=0;j<n;j++){
            //initializeVector(tab,L*2,0.0);
            //initializeVector(y,L,0.0);
            //for(size_t i=0;i<L;i++){
            //    tmp[i]=x1[(i*n)+j];
            //}
            cblas_dcopy(L,&x1[j],n,tmp,1);
            //dct(tmp,y,m,tab);
            fftw_execute(plan);
            //fftw_execute_r2r(plan,tmp,tmp1);
            vdLinearFrac(L,tmp1,tmp1,1/sqrt(2.0*L),0.0,0.0,1.0,tmp);
            tmp[0]=tmp[0]/sqrt(2);
            cblas_dcopy(L,tmp,1,&x1[j],n);
            //for(size_t i=0;i<L;i++){
            //    x1[(i*n)+j]=tmp[i];
            //}
    }


    //tmp2=x1;
    //#pragma omp for
    for(size_t i=0;i<n*L;i=i+n){
        tmp2=&x1[i];
        //for(size_t j=0;j<n;j++){
            //tmp2[j]=x1[i+j];
            //for(size_t k=0;k<N;k++){
            //    tmp2[(j*N)+k]=x1[(k*N)+j+i];
            //}
        //}
        dpwt2(tmp2,M,N,ell,J,hpf,lpf,lenfil,slide,temp);
        cblas_dcopy(n,slide,1,tmp2,1);
        //for(size_t j=0;j<n;j++){
        //    x1[i+j]=slide[j];
            //for(size_t k=0;k<N;k++){
            //    x1[(k*N)+j+i]=slide[(j*N)+k];
            //}
        //}
        //tmp2=tmp2+n;
    }
    //delete tmp2;
    //fftw_destroy_plan(plan);
    //mkl_free(slide);
    //mkl_free(tmp);
    //mkl_free(tmp1);
    //mkl_free(temp);
    //}

}

void kronerDCTinverse(double *x, double *lpf, size_t M, size_t N, size_t L,int lenfil,double *x1){

    int J = (int)(log((double)M)/log(2.0));
    static double *hpf=new double[lenfil];
    int ell=1;
    size_t n=M*N;
    //copydouble(x,x1,n*L);
    cblas_dcopy(n*L,x,1,x1,1);
    int m = (int)(log((double)L)/log(2.0));
    //static double *y=new double[L];
    int nutr = 1;
    //static double *tab=new double[L*2];//tabla de senos y coseno
    static double *tempa=(double*)mkl_malloc(nutr*N*4*sizeof(double),64);
    static double *slidea=(double*)mkl_malloc(nutr*n*sizeof(double),64);//new double[n];
    static double *tmpa=(double*)mkl_malloc(nutr*L*sizeof(double),64);//new double[L];
    static double *tmp1a=(double*)mkl_malloc(nutr*L*sizeof(double),64);

    mirrorfilt(lpf,hpf,lenfil);
    //#pragma omp parallel num_threads(nutr) shared(tempa,tmpa,tmp1a,slidea)
    //{
    double *tmp2;
    double *temp = &tempa[0*N*4];
    double *tmp = &tmpa[0*L];
    double *tmp1 = &tmp1a[0*L];
    double *slide = &slidea[0*n];
    //tmp2=x1;
    int rank=1;
    int* Ns = static_cast<int*>(MKL_malloc(3*sizeof(int),64));
    Ns[0]=M;
    Ns[1]=N;
    Ns[2]=L;
    int howmany = M*N;
    int idist = 1;
    int odist = 1;
    int istride = M*N;
    int ostride = M*N; /* distance between two elements in
                                     the same column */
    int *inembed = NULL;
    int *onembed = NULL;
    unsigned flags = FFTW_MEASURE;
    const fftw_r2r_kind kind = FFTW_REDFT01; // DCT-1
    /*fftw_plan_many_r2r(rank, &Ns[2],howmany,
                       x,inembed,
                       istride,idist,
                       x1, onembed,
                       ostride,odist,
                       &kind, flags);*/
    static fftw_plan plani = fftw_plan_r2r_1d(L, tmp, tmp1, FFTW_REDFT01, FFTW_MEASURE);
    //#pragma omp for
    for(size_t i=0;i<n*L;i=i+n){
        tmp2=&x1[i];
        //for(size_t j=0;j<n;j++){
           // tmp2[j]=x1[i+j];
            //for(size_t k=0;k<N;k++){
            //    tmp2[(j*N)+k]=x1[(k*N)+j+i];
            //}
        //}
        idpwt2(tmp2,M,N,ell,J,hpf,lpf,lenfil,slide,temp);
        cblas_dcopy(n,slide,1,tmp2,1);
        //for(size_t j=0;j<n;j++){
            //x1[i+j]=slide[j];
            //for(size_t k=0;k<N;k++){
            //    x1[(k*N)+j+i]=slide[(j*N)+k];
            //}
        //}
        //tmp2=tmp2+n;

    }

    //delete tmp2;
    //#pragma omp for
    for(size_t j=0;j<n;j++){
        //initializeVector(tab,L*2,0.0);
        //initializeVector(y,L,0.0);
        //for(size_t i=0;i<L;i++){
        //    tmp[i]=x1[(i*n)+j];
        //}
      cblas_dcopy(L,&x1[j],n,tmp,1);
      //idct(tmp,y,m,tab);
      //vdLinearFrac(L,tmp,tmp,sqrt(2.0*L),0.0,0.0,1.0,tmp1);
      tmp[0]=tmp[0]*sqrt(2);
      fftw_execute(plani);
      //fftw_execute_r2r(plani,tmp,tmp1);
      vdLinearFrac(L,tmp1,tmp1,1/sqrt(2.0*L),0.0,0.0,1.0,tmp);
      cblas_dcopy(L,tmp,1,&x1[j],n);
      //for(size_t i=0;i<L;i++){
      //    x1[(i*n)+j]=tmp[i];
      //}
    }
    //fftw_destroy_plan(plani);
    //mkl_free(slide);
    //mkl_free(tmp);
    //mkl_free(tmp1);
    //mkl_free(temp);
    //}
}


void mirrorfilt(double *lpf, double *hpf, int lenfil){
    int i,isign;
    isign = 1;
    for(i=0; i < lenfil; i++){
        *hpf++ = isign * *lpf++;
        isign *= -1;
    }
}


void uphi(double *x,int n,double *hpf,int m,double *y){
           int  meven, modd, i, h, j, mmin;
           double s;

           /*hipass version */
           meven = (m+1)/2;
           modd = m/2;

            /* away from edges */
           for( i= 0; i+meven<n; i++){
                s = 0.;
                for( h=0; h < meven; h++)
                    s += hpf[2*h]*x[i+h];
                y[2*i+1] = s;
                s = 0.;
                for( h=0; h < modd; h++)
                    s += hpf[2*h+1]*x[i+h];
                y[2*i] = s;
            }

            /* fix up edge values */
            mmin = n-meven;
            if(mmin < 0) mmin = 0;
            for( i= mmin; i<n; i++){

                s = 0.;
                j = i;
                for( h=0; h < meven; h++){
                    if(j >= n) j -= n;
                    s += hpf[2*h]*x[j];
                    j++;
                }
                y[2*i+1] = s;

                s = 0.;
                j = i;
                for( h=0; h < modd; h++){
                    if(j >= n) j -= n;
                    s += hpf[2*h+1]*x[j];
                    j++;
                }
                y[2*i] = s;
            }
}


void uplo(double *x,int n,double *lpf,int m,double *y){
           int  meven, modd, i, h, j, mmax;
           double s;

           /*lowpass version */

            /* away from edges */
           meven = (m+1)/2; modd = m/2;
           for( i= meven; i<n; i++){
                s = 0.;
                for( h=0; h < meven; h++)
                    s += lpf[2*h]*x[i-h];
                y[2*i] = s;
                s = 0.;
                for( h=0; h < modd; h++)
                    s += lpf[2*h+1]*x[i-h];
                y[2*i+1] = s;
            }

            /* fix up edge values */
            mmax = meven;
            if(mmax > n) mmax = n;
            for( i= 0; i < mmax; i++){

                s = 0.;
                j = i;
                for( h=0; h < meven; h++){
                    if(j < 0) j += n;
                    s += lpf[2*h]*x[j];
                    --j;
                }
                y[2*i] = s;

                s = 0.;
                j = i;
                for( h=0; h < modd; h++){
                    if(j < 0) j += n;
                    s += lpf[2*h+1]*x[j];
                    --j;
                }
                y[2*i+1] = s;
            }
}

void convertandcopy(mwSize n,mwIndex *input1,mwIndex *input2,MKL_INT *output1,MKL_INT *output2,mwSize nz,mwIndex *input3,MKL_INT *output3){
    for(mwSize i=0;i<n;i++){
        output1[i] = static_cast<MKL_INT>(input1[i]);
        output2[i] = static_cast<MKL_INT>(input2[i]);
        output3[i] = static_cast<MKL_INT>(input3[i]);
    }
    for(mwSize i=n;i<nz;i++){
        output3[i] = static_cast<MKL_INT>(input3[i]);
    }
}


sparse_matrix_t matlab_to_spcblas(const mxArray * mat){
    mwSize m = mxGetM(mat);
    mwSize n = mxGetN(mat);
    mwSize nz = mxGetNzmax(mat);
    mwSize *Ir = mxGetIr(mat);
    sparse_matrix_t As;
    mwIndex *pntrB = mxGetJc(mat);
    mwIndex *pntrE = mxGetJc(mat)+1;
    double* A = mxGetDoubles(mat);
    MKL_INT *Ir1 = static_cast<MKL_INT*>(mkl_malloc(nz*sizeof(MKL_INT),64));
    MKL_INT *pntrE1 = static_cast<MKL_INT*>(mkl_malloc(n*sizeof(MKL_INT),64));
    MKL_INT *pntrE2 = static_cast<MKL_INT*>(mkl_malloc(n*sizeof(MKL_INT),64));
    convertandcopy(n,pntrB,pntrE,pntrE1,pntrE2,nz,Ir,Ir1);
    matrix_descr md;
    md.type = SPARSE_MATRIX_TYPE_GENERAL;

    sparse_status_t status = mkl_sparse_d_create_csc(&As, SPARSE_INDEX_BASE_ZERO, m, n, pntrE1, pntrE2, Ir1, A);
    return As;
}


/*Map<MatlabSparse> matlab_to_eigen_sparse(const mxArray * mat)
{
    mxAssert(mxGetClassID(mat) == mxDOUBLE_CLASS,
    "Type of the input matrix isn't double");
    mwSize     m = mxGetM (mat);
    mwSize     n = mxGetN (mat);
    mwSize    nz = mxGetNzmax(mat);
    //Theoretically fails in very very large matrices
    mxAssert(nz <= std::numeric_limits< std::make_signed<mwIndex>::type>::max(),
    "Unsupported Data size."
    );
    double  *pr = mxGetPr (mat);
    MatlabSparse::StorageIndex* ir = reinterpret_cast<MatlabSparse::StorageIndex*>(mxGetIr (mat));
    MatlabSparse::StorageIndex* jc = reinterpret_cast<MatlabSparse::StorageIndex*>(mxGetJc (mat));
    Map<MatlabSparse> result (m, n, nz, jc, ir, pr);
    return result;
}
*/

//
void mTimes(sparse_matrix_t A, double* x, double* res, double* G,mwSize tm,mwSize tmy,mwSize m,mwSize n,mwSize l, bool trans,int par){
    matrix_descr md;
    md.type = SPARSE_MATRIX_TYPE_GENERAL;
    static double durations = 0;
    static double durationk = 0;
    if(!trans){ //A*x
        clock_t t0 = clock();
        static double* x1 = (double*)mkl_malloc(tm*sizeof(double),64);
        kronerDCTinverse(x,G,m,n,l,par,x1);// falta corregir q guarde en x1;
        durationk = durationk + (clock() - t0 ) / (double) CLOCKS_PER_SEC;
        mexPrintf("Time Kroner= %f",durationk);
        //MapType x1e(x1,tm);
        //MapType rese(res,tmy);
        //rese = A*x1e;
        //res=&rese(0);
        t0 = clock();
        mkl_sparse_d_mv(SPARSE_OPERATION_NON_TRANSPOSE, 1.0, A, md, x1, 0.0,res);
        durations = durations + (clock() - t0 ) / (double) CLOCKS_PER_SEC;
        mexPrintf(" Time sparse= %f\n",durations);
    }else{//A'*x
        static double* aux =(double*)mkl_malloc(tm*sizeof(double),64);// new double[tm];
        //MapType x1ea(x,tmy);
        //MapType auxe(aux,tm);
        //auxe = A*x1ea;
        //aux=&auxe(0);
        clock_t t0 = clock();
        mkl_sparse_d_mv(SPARSE_OPERATION_TRANSPOSE, 1.0, A, md, x, 0.0,aux);
        durations = durations + (clock() - t0 ) / (double) CLOCKS_PER_SEC;
        mexPrintf("Time sparse= %f",durations);
        t0 = clock();
        kronerDCTdirect(aux,G,m,n,l,par,res);
        durationk = durationk + (clock() - t0 ) / (double) CLOCKS_PER_SEC;
        mexPrintf(" Time kroner= %f\n",durationk);
    }

}

/**
* @brief eval Evaluate if values are greater of smaller than a value and makes 0 if don't
* @param m1 input vector
* @param val value to be compared
* @param size size of the vector
* @param flag if flag = 0 put a 1 if a position is greater that 0 and 0 otherwise. flag = 1 opposite
* @param result binary vector
*/
void  eval(double *m1, double val, size_t size, int flag, double *result){
    int val1,val2;
    if(flag<2){
        if(flag==0){
            val1=1;
            val2=0;
        }else{
            val1=0;
            val2=1;
        }
        #pragma omp parallel for num_threads(4)
        for(size_t i=0;i<size;i++){
            if (m1[i]>=val)result[i]=val1;
            else result[i]=val2;
        }
    }else{
        #pragma omp parallel for num_threads(4)
        for(size_t i=0;i<size;i++){
            if (m1[i]==val)result[i]=0;
            else result[i]=1;
        }
    }
}
/*void  eval(double* m1, double val, size_t size, int flag, double* result){
    int val1,val2;
    double* result1=result;
    if(flag<2){
        if(flag==0){
            val1=1;
            val2=0;
        }else{
            val1=0;
            val2=1;
        }
        for(register double* i=m1;i<m1+size;i++){
            if (*i>=val){
                *result1=val1;
                result1++;
            }else{
                *result1=val2;
                result1++;
            }
        }
    }else{
        for(register double* i=m1;i<m1+size;i++){
            if (*i==val){
                *result1=0;
                result1++;
            }else{
                *result1=1;
                result1++;
            }
        }
    }
}*/

/*void maxi(double* m, double val, size_t size, int flag, double* result){
    register double* j=result;
    for(register double* i=m;i<m+size;i++){
        if (flag==0){
             *j=(*i>val)?*i:val;
        }else{
             *j=(*i<val)?*i:val;
        }
        j++;
    }
}

void maxi(double* m, double* m1, size_t size, int flag, double* result){// maxi= max if flag=0 and min if flag!=0
    register double* i;
    register double* j=m1;
    register double* k=result;

    for(i=m;i<m+size;i++){
        if (flag==0){
             *k=(*i>*j)?*i:*j;
        }else{
             *k=(*i<*j)?*i:*j;
        }
        j++;
        k++;
    }
}*/

void maxi(double *m, double val, size_t size, int flag, double *result){
    if (flag==0){
        #pragma omp parallel for num_threads(4)
        for(size_t i=0;i<size;i++){
             result[i]=(m[i]>val)?m[i]:val;
        }
    }else{
        #pragma omp parallel for num_threads(4)
        for(size_t i=0;i<size;i++){
         result[i]=(m[i]<val)?m[i]:val;
        }
    }
}
void maxi(double *m, double *m1, size_t size, int flag, double *result){// maxi= max if flag=0 and min if flag!=0
    if (flag==0){
        #pragma omp parallel for num_threads(4)
        for(size_t i=0;i<size;i++){
                 result[i]=(m[i]>m1[i])?m[i]:m1[i];
        }
    }else{
        #pragma omp parallel for num_threads(4)
        for(size_t i=0;i<size;i++){
         result[i]=(m[i]<m1[i])?m[i]:m1[i];
        }
    }
}

void GPSR_BB(double* y, sparse_matrix_t A, double tau,double* x, double tolA ,double* G ,mwSize tmy, mwSize tm,mwSize m,mwSize n,mwSize l,int par,mwSize maxiter1,bool verbose1){

    double f=1.0,tol_debias=0.001,num_nz_x=2.0,alpha=1.0;
    bool cont_debias_cg=true,keep_going;
    static double* RWpvec=0;
    double beta_cg=0.0,alpha_cg=0.0,rTr_cg_plus=0.0,rTr_cg=0.0,final_tau=0.0;
    double final_tolA=0.0001;
    double tmp;
    clock_t t0,t1;;
    int iter=0;
    //MatlabSparse B=A.transpose();
    int final_stopCriterion=4;
    const unsigned Initial_X_supplied=80;
    int stopCriterion = 1;
    double tolD = 0.0001;
    bool debias = false;
    int maxiter = maxiter1;
    int maxiter_debias = 500;
    int miniter = 5;
    int miniter_debias = 5;
    int init = 0;
    int enforceMonotone = 1;
    double alphamin = 1e-30;
    double alphamax = 1e30;
    bool compute_mse = false;
    bool verbose = verbose1;
    bool continuation = false;
    int cont_steps = -1;
    int firstTauFactorGiven = 0;
    double firstTauFactor = 0.0,criterionObjective=0.0,realmin=0.0,lambda0=0.0;
    double lambda=0.001,dGd=0.0,criterionActiveSet=0.0,delta_x_criterion=0.0;
    double prev_f=0.0,dd=0.0,num_changes_active=0.0,criterionLCP=0.0;
    size_t i;
    double* Aty=static_cast<double*>(mkl_malloc(tm*sizeof(double),64));
    double* aux=static_cast<double*>(mkl_malloc(tm*sizeof(double),64));
    //double* Apvec= mkl_malloc(tm*sizeof(double),64);
    //double* resid2=(double*)mkl_malloc(tmy*sizeof(double),64);
    //double* resid_prev = mkl_malloc(tmy*sizeof(double),64);//new double[tmy];
    mTimes(A,y,Aty,G,tm,tmy,m,n,l,true,par);
    double* cont_factors=nullptr;
    //double* rvec= mkl_malloc(tm*sizeof(double),64);//new double[tm];
    double* resid=static_cast<double*>(mkl_malloc(tmy*sizeof(double),64));//new double[tmy];
    double* nz=static_cast<double*>(mkl_malloc(tm*sizeof(double),64));//new double[tm];
    double* true_x=static_cast<double*>( mkl_malloc(tm*sizeof(double),64));//new double[tm];
    double* nz_x=static_cast<double*>(mkl_malloc(tm*sizeof(double),64));//new double[tm];
    double* nz_x_prev=static_cast<double*>(mkl_malloc(tm*sizeof(double),64));//new double[tm];
    double* resid_base=static_cast<double*>(mkl_malloc(tmy*sizeof(double),64));//new double[tmy];
    double* gradu=static_cast<double*>(mkl_malloc(tm*sizeof(double),64));//new double[tm];
    double* u=static_cast<double*>(mkl_malloc(tm*sizeof(double),64));//new double[tm];
    double* gradv=static_cast<double*>(mkl_malloc(tm*sizeof(double),64));//new double[tm];
    double* v=static_cast<double*>(mkl_malloc(tm*sizeof(double),64));//new double[tm];
    double* old_u=static_cast<double*>(mkl_malloc(tm*sizeof(double),64));//new double[tm];
    double* old_v=static_cast<double*>(mkl_malloc(tm*sizeof(double),64));//new double[tm];
    double* dx =static_cast<double*>( mkl_malloc(tm*sizeof(double),64));//new double[tm];
    //double* w = mkl_malloc(2*tm*sizeof(double),64);//new double[2*tm];
    double* gradq=static_cast<double*>(mkl_malloc(tm*sizeof(double),64));//new double[tm];
    double* uvmin =static_cast<double*>(mkl_malloc(tm*sizeof(double),64));// new double[tm];
    double* auv =static_cast<double*>(mkl_malloc(tmy*sizeof(double),64));// new double[tmy];
    //double* err=0;
    double* du=static_cast<double*>(mkl_malloc(tm*sizeof(double),64));//new double[tm];
    double* dv=static_cast<double*>(mkl_malloc(tm*sizeof(double),64));//new double[tm];
    //double* temp=static_cast<double*>(mkl_malloc(tm*sizeof(double),64));//new double[tm];
    double* temp1=static_cast<double*>(mkl_malloc(tm*sizeof(double),64));//new double[tm];
    double* temp2=static_cast<double*>(mkl_malloc(tm*sizeof(double),64));//new double[tm];
    //double* tempy=static_cast<double*>(mkl_malloc(tmy*sizeof(double),64));//new double[tmy];
    //double* temp1y=static_cast<double*>(mkl_malloc(tmy*sizeof(double),64));//new double[tmy];
    double* temp2y=static_cast<double*>(mkl_malloc(tmy*sizeof(double),64));//new double[tmy];
    double* term=static_cast<double*>(mkl_malloc(tm*sizeof(double),64));//new double[tm];
    double temporal;
    double mses;
    double* x_debias=0;
    int debias_start;
    double objective;
    double tempMax;
    double tmp1,tmp2,tmp3;
    size_t indextemporal;
    indextemporal = cblas_idamax(tm,Aty,1);
    temporal=fabs(Aty[indextemporal]); //temporal= max(Aty,tm,1);
    size_t tmp4;
    tau=2*temporal*tau;

    switch(init)
    {
    case 0: {
        //x=new double[tm];
        for(i=0;i<tm;i++){
            x[i]=0.0;
        }
        break;
    };

    case 2: {
        mTimes(A,y,x,G,tm,tmy,m,n,l,true,par);
        break;
    };
    case Initial_X_supplied: // Initial x was given as a function arguments; just check size
    {
        cout<<"Initial x was given"<< endl;
        break;
    };
    default: cout<<"Unknown Initialization option"<<endl;break;
    }

    mTimes(A,y,aux,G,tm,tmy,m,n,l,true,par);

    double max_tau;
    max_tau = 0.0;

    for (size_t i = 0; i < tm;i++) //se calcula el maximo valor absoluto de la matriz
    {
        aux[i] = fabs(aux[i]);

        if (aux[i] > max_tau)
            max_tau=(double)aux[i];
    }
    if (tau >= max_tau)
    {
        for(i=0;i<tm;i++)
            x[i]=0.0;
        if (debias){
            //copy(x,tm,x_debias);
            cblas_dcopy(tm,x,1,x_debias,1);
        }
        //temporal=prod(y,y,tm);
        temporal = cblas_ddot(tmy,y,1,y,1);
        objective=(0.5*temporal); //Revisar tamaño del vector

        if (compute_mse)
        {

            for (i=0;i<tm;i++)
            {
                mses=mses+(true_x[i]*true_x[i]); // calcula el error cuadratico medio
            }
        }
    }
    cblas_dcopy(tm,x,1,temp1,1);
    cblas_dscal(tm,-1.0,temp1,1);
    //scalar_prod(x,-1,tm,temp1);
    eval(x,0.0,tm,0,temp2);
    vdMul( tm, x, temp2, u );
    //element_prod(x, temp2,tm,u);        //eval funcion (x>=0)
    eval(x,0.0,tm,1,temp2);
    vdMul( tm, temp1, temp2, v );
    //element_prod(temp1,temp2,tm,v);  //evalmenor funcion (x<0)


    //define the indicator vector or matrix of nonzeros in x
    eval(x,0.0,tm,2,nz_x); //eval x ~= 0.0
    num_nz_x = cblas_dasum(tm,nz_x,1);
    //num_nz_x = mysum(nz_x,1.0,tm); //calcular la suma
    t0 = clock();

    // store given tau, because we're going to change it in the
    // continuation procedure

    final_tau = tau;
    //store given stopping criterion and threshold, because we're going
    // to change them in the continuation procedure
    final_stopCriterion = stopCriterion;
    final_tolA = tolA;  // valor?
    //set continuation factors

    if (continuation && (cont_steps > 1))
    {
        // If tau is scalar, first check top see if the first factor is
        // too large (i.e., large enough to make the first
        // solution all zeros). If so, make it a little smaller than that.
        // Also set to that value as default

        if ((firstTauFactorGiven == 0)|| firstTauFactor*tau>= max_tau)
        {
            firstTauFactor= (0.5*max_tau)/tau; // como dividir???
            if (verbose)
            {
                mexPrintf("\n setting parameter FirstTauFactor\n");
            }

        }


        tmp1=log10(firstTauFactor);
        tmp2=log10(1/firstTauFactor)/(cont_steps-1);
        tmp3=0.0;
        tmp4=((int)(tmp1/(-1*tmp2)))+1;
        cont_factors=new double[tmp4];
        for(i=0;i<tmp4;i++){
            cont_factors[i]=pow(10,tmp1);
            tmp1=tmp1+tmp2;
        }

    }

    if (!continuation)
    {
        cont_factors=new double[1];

        cont_factors[0]=1.0;
        cont_steps = 1;
    }
    iter = 1;
    if (compute_mse)
    {
        double* tmpo=new double[tm];
        cblas_dcopy(tm,true_x,1,tmpo,1);
        cblas_daxpy(tm,-1.0,x,1,tmpo,1);
        //sum(x,true_x,tm,1,tmpo);
        vdMul(tm,tmpo,tmpo,tmpo);
        mses = cblas_dasum(tm,tmpo,1);
        delete[] tmpo;
    }
    bool keep_continuation = true;
    int cont_loop = 1;

    // Compute and store initial value of the objective function

    while (keep_continuation){

        mTimes(A,x,temp2y,G,tm,tmy,m,n,l,false,par);
        cblas_dcopy(tmy,y,1,resid,1);
        cblas_daxpy(tmy,-1.0,temp2y,1,resid,1); //resid=y-resid;
        //sum(y,temp1y,tmy,1,resid);// dimensionalidad de resid y "y"
        //gradq=AT(resid) //prod(trans(A),resid)
        //A(x) y.assign_temporary(vector x)

        if (cont_steps == -1){
            mTimes(A,resid,gradq,G,tm,tmy,m,n,l,true,par);//AT(resid);
            tau =(final_tau>0.2*gradq[cblas_idamax(tmy,gradq,1)])?final_tau:0.2*gradq[cblas_idamax(tmy,gradq,1)];//tau = max(final_tau,0.2*max(abs(gradq)));funcion max y abs de una matriz???
            if (tau == final_tau){
                stopCriterion = final_stopCriterion;
                tolA = final_tolA;
                keep_continuation = 0;
            }
            else{
                stopCriterion = 1;
                tolA = 1e-5;

            }
        }
        else{
            tau = final_tau * cont_factors[0];//cont_factors(cont_loop) cont_factors es una cte siempre
            if (cont_loop == cont_steps){
                stopCriterion = final_stopCriterion;
                tolA = final_tolA;
                keep_continuation = 0;
            }
            else{
                stopCriterion = 1;
                tolA = 1e-5;
            }
        }


        if (verbose){
            mexPrintf( " Setting tau = %0.5g \n",tau );
        }

        if (cont_loop == 1) {
            alpha = 1.0;
            cblas_dcopy(tm,u,1,temp1,1);
            cblas_dscal(tm,tau,temp1,1);
            //scalar_prod(u,tau,tm,temp1);

            cblas_dcopy(tm,v,1,temp2,1);
            cblas_dscal(tm,tau,temp2,1);
            //scalar_prod(v,tau,tm,temp2);


            f = 0.5 * cblas_ddot(tmy,resid,1,resid,1) + cblas_dasum(tm,temp1,1) + cblas_dasum(tm,temp2,1);
                    //(prod(resid,resid,tmy))+mysum(temp1,1.0,tm,1.0)+mysum(temp2,1.0,tm,1.0);
            objective = f;
            if (compute_mse){

                //sum(x,true_x,tm,1,temp1);
                cblas_dcopy(tm,true_x,1,temp1,1);
                cblas_daxpy(tm,-1.0,x,1,temp1,1);
                //sum(x,true_x,tm,1,tmpo);
                vdMul(tm,temp1,temp1,temp1);
                mses = cblas_dasum(tm,temp1,1);
                //mses = prod(temp1,temp1,tm);
            }
            if (verbose){
                mexPrintf("Initial obj = %f, alpha=%f, nonzeros=%f  \n",f , alpha , num_nz_x );
            }
        }
        // Compute the initial gradient and the useful quantity resid_base
        cblas_dcopy(tmy,resid,1,resid_base,1);
        cblas_daxpy(tmy,-1.0,y,1,resid_base,1); //resid_base=y-resid;
        //sum(y,resid,tmy,1,resid_base);

        // control variable for the outer loop and iteration counter
        keep_going = true;

        if (verbose){
            mexPrintf("\n Initial obj =%f, nonzeros =%f \n", f, num_nz_x);
        }

        while (keep_going)
        {
            mTimes(A,resid_base,term,G,tm,tmy,m,n,l,true,par);
            cblas_daxpy(tm,-1.0,Aty,1,term,1);
            vdLinearFrac(tm,term,gradu,1.0,tau,0.0,1.0,gradu);
            vdLinearFrac(tm,term,term,-1.0,0,0,1,temp1);//temp1=-term
            vdLinearFrac(tm,temp1,gradv,1.0,tau,0.0,1.0,gradv);
            vdLinearFrac(tm,gradu,gradu,alpha,0.0,0.0,1.0,temp1);
            cblas_dcopy(tm,u,1,temp2,1);
            cblas_daxpy(tm,-1.0,temp1,1,temp2,1);
            maxi(temp2, 0.0,tm,0,du);
            cblas_daxpy(tm,-1.0,u,1,du,1);
            vdLinearFrac(tm,gradv,gradv,alpha,0.0,0.0,1.0,temp1);
            cblas_dcopy(tm,v,1,temp2,1);
            cblas_daxpy(tm,-1.0,temp1,1,temp2,1);
            maxi(temp2, 0.0,tm,0,dv);
            cblas_daxpy(tm,-1.0,v,1,dv,1);
            cblas_dcopy(tm,du,1,dx,1);
            cblas_daxpy(tm,-1.0,dv,1,dx,1);
            cblas_dcopy(tm,u,1,old_u,1);
            cblas_dcopy(tm,v,1,old_v,1);
            // calculate useful matrix-vector product involving dx
            mTimes(A,dx,auv,G,tm,tmy,m,n,l,false,par);
            dGd = cblas_ddot(tmy,auv,1,auv,1);

            if (enforceMonotone==1){
                // monotone variant: calculate minimizer along the direction (du,dv)
                //lambda0 = - (prod(gradu,du,tm) + prod(gradv,dv,tm))/(realmin+dGd);
                lambda0 = - (cblas_ddot(tm,gradu,1,du,1) + cblas_ddot(tm,gradv,1,dv,1))/(realmin+dGd);
                if (lambda0 < 0){
                    mexPrintf("ERROR: lambda0 = %10.3e negative. Quit\n", lambda0);

                }
                lambda = (lambda0<1)?lambda0:1;
            }
            else{
                //nonmonotone variant: choose lambda=1
                lambda = 1;
            }

            //scalar_prod(du,lambda,tm,temp1);
            cblas_daxpy(tm,lambda,du,1,u,1);
            cblas_daxpy(tm,lambda,dv,1,v,1);
            maxi(u,v,tm,1,uvmin);
            cblas_daxpy(tm,-1.0,uvmin,1,u,1);
            cblas_daxpy(tm,-1.0,uvmin,1,v,1);//u=u-uvmin
            cblas_dcopy(tm,u,1,x,1);
            cblas_daxpy(tm,-1.0,v,1,x,1);//x=u-v
            cblas_dcopy(tm,nz_x,1,nz_x_prev,1);
            eval(x,0.0,tm,2,nz_x);
            num_nz_x=cblas_dasum(tm,nz_x,1);
            cblas_dcopy(tmy,y,1,resid,1);
            cblas_daxpy(tmy,-lambda,auv,1,resid,1);//y-lambda*auv;
            cblas_daxpy(tmy,-1.0,resid_base,1,resid,1);//y-resid_base-(ant)
            prev_f=f;
            vdLinearFrac(tm,u,u,tau,0.0,0.0,1.0,temp1); //temp1=tau*u;
            vdLinearFrac(tm,v,v,tau,0.0,0.0,1.0,temp2); //temp2=tau*v;
            f=0.5*cblas_ddot(tmy,resid,1,resid,1)+ cblas_dasum(tm,temp1,1)+ cblas_dasum(tm,temp2,1);
            //dd= prod(du,du,tm) + prod(dv,dv,tm);
            dd= cblas_ddot(tm,du,1,du,1) + cblas_ddot(tm,dv,1,dv,1);
            if (dGd <=0 )
            {
                // something wrong if we get to here
                mexPrintf(" dGd=%12.4e, nonpositive curvature detected\n", dGd);
                alpha = alphamax;
            }
            else
            {
                tmp=(alphamin>dd/dGd?alphamin:dd/dGd);
                alpha = (alphamax<tmp)?alphamax:tmp;

            }
            //temp1=resid_base;
            //scalar_prod(auv,lambda,tmy,temp2y);
            //sum(resid_base , temp2y,tmy,0,tempy);
            cblas_daxpy(tmy,lambda,auv,1,resid_base,1);//resid_base = resid_base + lambda*auv;
            //copy(tempy,tmy,resid_base);

            // print out stuff
            if (verbose)
            {
                mexPrintf("It=%4d, obj=%9.5e, alpha=%6.2e, nz=%8.0f  ",iter, f, alpha, num_nz_x);

            }
            // update iteration counts, store results and times
            iter++;
            objective = f;
            if (compute_mse)
            {
                //sum(x,true_x,tm,1,temp1);
                cblas_dcopy(tm,true_x,1,temp1,1);
                cblas_daxpy(tm,-1.0,x,1,temp1,1);
                //sum(x,true_x,tm,1,tmpo);
                vdMul(tm,temp1,temp1,temp1);
                mses = cblas_dasum(tm,temp1,1);
                //mses = prod(temp1,temp1,tm);
            }

            switch(stopCriterion)
            {
            case 0:
            {
                //compute the stopping criterion based on the change
                //of the number of non-zero components of the estimate
                //
                //----num_changes_active = (sum(nz_x(:)~=nz_x_prev(:)));-----
                //---------------------------------
                //evalvec(nz_x,nz_x_prev,tm,temp1);
                vdMul(tm,nz_x,nz_x_prev,temp1);
                //num_changes_active = (mysum(temp1,1.0,tm,1));
                num_changes_active = cblas_dasum(tm,temp1,1);

                //---------------------------------
                //----num_changes_active = (sum(nz_x(:)~=nz_x_prev(:)));
                if (num_nz_x >= 1)
                {
                    criterionActiveSet = num_changes_active;
                }
                else{
                    criterionActiveSet = tolA / 2;
                }
                if (criterionActiveSet > tolA){  //revisar esto
                    keep_going=1;
                }
                else{
                    keep_going=0;
                }
                if (verbose){
                    mexPrintf("Delta n-zeros = %f (target = %e)\n",criterionActiveSet , tolA);
                }
                break;
            }
            case 1:
            {
                //compute the stopping criterion based on the relative
                //variation of the objective function.
                criterionObjective = fabs(f-prev_f)/(prev_f);
                if(criterionObjective > tolA)
                {
                    keep_going=1;
                }
                else{
                    keep_going=0;
                }
                if (verbose){
                    mexPrintf("Delta obj. = %e (target = %e)\n",criterionObjective , tolA);
                }
                break;
            }
            case 2:
            {
                //stopping criterion based on relative norm of step taken
                //----delta_x_criterion = norm(dx(:))/norm((angry):)); -----
                //---------------------------------
                //delta_x_criterion = sqrt(mysum(dx,2.0,tm))/sqrt(mysum(x,2.0,tm));
                delta_x_criterion = cblas_dnrm2(tm,dx,1)/cblas_dnrm2(tm,x,1);
                keep_going = (delta_x_criterion > tolA);
                if (verbose){
                    mexPrintf("Norm(delta x)/norm(x) = %e (target = %e)\n",delta_x_criterion,tolA);
                }
                //---------------------------------
                //----delta_x_criterion = norm(dx(:))/norm((angry):)); -----

                break;
            }
            /*case 3:
            {

                maxi(gradu,old_u,tm,1,temp1);
                maxi(gradu,old_v,tm,1,temp2);
                concatenate(temp1,temp2,tm,tm,w);

                criterionLCP = max(w,2*tm,1);
                tempMax = max(old_u,tm,1);
                tmp=(1.0e-6 > tempMax)?1.0e-6:tempMax;
                criterionLCP = criterionLCP / ((tmp > max(old_v,tm,1))?tmp:max(old_v,tm,1));
                keep_going = (criterionLCP > tolA);
                if (verbose)
                {
                    mexPrintf("LCP = %e (target = %e)\n",criterionLCP,tolA);
                }
                break;
            }*/
            case 4:
            {
                keep_going = (f > tolA);
                if (verbose)
                {
                    mexPrintf("Objective = %e (target = %e)\n",f,tolA);
                }
                break;
            }
            case 5:
            {

                delta_x_criterion = sqrt(dd)/sqrt(cblas_ddot(tm,x,1,x,1));
                keep_going = (delta_x_criterion > tolA);
                if (verbose)
                {
                    mexPrintf("Norm(delta x)/norm(x) = %e (target = %e)\n",delta_x_criterion,tolA);
                }
                break;
            }
            default:
            {
                mexPrintf("Stop Criterion Error");
            }

            }
            if (iter<=miniter)
            {
                keep_going = true;
            }
            else if (iter > maxiter) //and no more than maxiter iterations
            {

                keep_going = 0;
            }
        }
        cont_loop++;


    }


    /*for (i=0;i<tm;i++)
    {
        if (x[i] != 0.0){
            nz_x[i]=1.0;
        }
    }

    num_nz_x = mysum(nz_x,1.0,tm);
    */

    eval(x,0.0,tm,2,nz_x);
    //num_nz_x=mysum(nz_x,1.0,tm,0);
    num_nz_x=cblas_dasum(tm,nz_x,1);

    //if (verbose)
    //{


        mexPrintf("\nFinished the main algorithm!\nResults:\n");
        mexPrintf("||A x - y ||_2^2 = %e\n",cblas_ddot(tmy,resid,1,resid,1));
        mexPrintf("||x||_1 = %e \n",cblas_dasum(tm,x,1));
        mexPrintf("Objective function = %10.3f\n",f);
        //mexPrintf("ti1=%f, ti2=%f, ti3=%f, ti4=%f\n",ti1/CLOCKS_PER_SEC,ti2/CLOCKS_PER_SEC,ti3/CLOCKS_PER_SEC,ti4/CLOCKS_PER_SEC);
        mexPrintf("Number of non-zero components = %2.0f\n",num_nz_x);
        mexPrintf("CPU time so far = %10.3f\n", (float)(clock()-t0)/CLOCKS_PER_SEC);
        mexPrintf("\n");

    //}

    /*if (debias && (num_nz_x != 0.0))
    {
        if (num_nz_x > tmy)
        {
            if (verbose)
            {
                mexPrintf("\n");
                mexPrintf("Debiasing requested, but not performed\n");
                mexPrintf("There are too many nonzeros in x\n\n");
                mexPrintf("nonzeros in x: %6.4f, length of y: %6d\n",num_nz_x,(int)tmy);
            }
        }

        else if(num_nz_x == 0.0)
        {
            if (verbose){
                mexPrintf("\n");
                mexPrintf("Debiasing requested, but not performed\n");
                mexPrintf("x has no nonzeros\n\n");
            }
        }
        else
        {
            if (verbose){
                mexPrintf("\n");
                mexPrintf("Starting the debiasing phase...\n\n");
            }

            copy(x,tm,x_debias);
            copy(nz_x,tm,zeroind);
            cont_debias_cg = true;
            debias_start = iter;

            // calculate initial residual
            mTimes(A,x_debias,temp1y,coord1,coord2,coord3,false);
            sum(temp1y,y,tmy,1,resid);

            for(i=0;i<tmy;i++){
                resid2[i]=1.0;
            }

            scalar_prod(resid2,2e-16,tmy,resid_prev);

            mTimes(A,resid,temp1,coord1,coord2,coord3,true);

            // mask out the zeros
            element_prod(temp1,zeroind,tm,rvec);
            rTr_cg = prod(rvec,rvec,tm);

            // set convergence threshold for the residual || RW x_debias - y ||_2
            tol_debias = tolD * prod(rvec,rvec,tm);//prod se podria cambiar por rTr_cg?

            // initialize pvec
            scalar_prod(rvec,-1.0,tm,pvec);

            while(cont_debias_cg)
            {
                mTimes(A,pvec,RWpvec,coord1,coord2,coord3,false);//funcion de multiplicacion
                mTimes(A,RWpvec,temp1,coord1,coord2,coord3,true);
                element_prod(temp1,zeroind,tm,Apvec);
                alpha_cg = rTr_cg/prod(pvec,Apvec,tm);
                scalar_prod(pvec,alpha,tm,temp1);
                copy(x_debias,tm,temp2);
                sum(temp2 , temp1,tm,0,x_debias);
                scalar_prod(RWpvec,alpha_cg,tmy,temp1y);
                copy(resid,tmy,temp2y);
                sum(temp2y , temp1y,tmy,0,resid);
                scalar_prod(Apvec,alpha_cg,tm,temp1);
                copy(rvec,tm,temp2);
                sum(temp2,temp1,tm,0,rvec);
                rTr_cg_plus = prod(rvec,rvec,tm);//hay q vectorizar rvec
                beta_cg = rTr_cg_plus / rTr_cg;
                scalar_prod(rvec,-1.0,tm,temp1);
                scalar_prod(pvec,beta_cg,tm,temp2);
                sum(temp1 , temp2,tm,0,pvec);//pvec delete?
                rTr_cg = rTr_cg_plus;
                iter++;
                scalar_prod(x_debias,tau,tm,temp1);
                objective = (0.5*prod(resid,resid,tmy)) + mysum(temp1,1.0,tm,1);
                if(compute_mse)
                {
                    for (i=0;i<tm;i++){
                        true_x[i]=1.0;
                    }
                    sum(true_x , x_debias,tm,1,err);
                    mses = prod(err,err,tm);//vectorizar
                }
                if (verbose)
                {
                    mexPrintf(" Iter = %5d, debias resid = %13.8f, convergence = %8.3f\n",iter, prod(resid,resid,tm), (rTr_cg / tol_debias));
                }
                cont_debias_cg = ((iter-debias_start) <= miniter_debias )|((rTr_cg > tol_debias) & ((iter-debias_start) <= maxiter_debias));

                break;
            }
            if (verbose) {
                mexPrintf("\nFinished the debiasing phase!\nResults:\n");
                mexPrintf("||A x - y ||_2^2 = %5.20f\n",prod(resid,resid,tmy));
                mexPrintf("||x||_1 = %10.3f\n",mysum(x,1.0,tm,1));
                mexPrintf("Objective function = %10.3f\n",f);
                for (i=0;i<tm;i++){
                    if (x_debias[i] != 0.0){
                        nz[i]=1;
                    }
                }
                mexPrintf("Number of non-zero components = %8.0f\n",mysum(nz));
                mexPrintf("CPU time so far = %10.3f\n", (float)(clock()-t0)/CLOCKS_PER_SEC);
                mexPrintf("\n");
            }

        }

    }*/

    mkl_free(Aty);
    mkl_free(aux);

    mkl_free(resid);//new double[tmy];
    mkl_free(nz);//new double[tm];
    mkl_free(true_x);//new double[tm];
    mkl_free(nz_x);//new double[tm];
    mkl_free(nz_x_prev);//new double[tm];
    mkl_free(resid_base);//new double[tmy];
    mkl_free(gradu);//new double[tm];
    mkl_free(u);//new double[tm];
    mkl_free(gradv);//new double[tm];
    mkl_free(v);//new double[tm];
    mkl_free(old_u);//new double[tm];
    mkl_free(old_v);//new double[tm];
    mkl_free(dx);//new double[tm];
    //double* w = mkl_malloc(2*tm*sizeof(double),64);//new double[2*tm];
    mkl_free(gradq);//new double[tm];
    mkl_free(uvmin);// new double[tm];
    mkl_free(auv);// new double[tmy];
    //double* err=0;
    mkl_free(du);//new double[tm];
    mkl_free(dv);//new double[tm];
    //double* temp=(double*)mkl_malloc(tm*sizeof(double),64);//new double[tm];
    mkl_free(temp1);//new double[tm];
    mkl_free(temp2);//new double[tm];
    //double* tempy=(double*)mkl_malloc(tmy*sizeof(double),64);//new double[tmy];
    //double* temp1y=(double*)mkl_malloc(tmy*sizeof(double),64);//new double[tmy];
    mkl_free(temp2y);//new double[tmy];
    mkl_free(term);//new double[tm];
}
