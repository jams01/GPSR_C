Gradient Projection for Sparse Reconstruction: Application to Compressed Sensing and Other Inverse Problems" by Mario A. T. Figueiredo, Robert D. Nowak, Stephen J. Wright, Journal of Selected Topics on Signal Processing, December 2007

Implementation of the GPSR algorithm in C++ for Matlab (Mex) (single core)

This implementations uses Intel MKL as linear algebra library for fast computations. 

The sparsity basis are DCT along the spectral domain and Wavelet 2D in the spatial domain.

The compilation step using g++ are shown next.

First create the GPSR object 

/usr/bin/g++ -c -DMX_COMPAT_64  -DUSE_MEX_CMD -D_GNU_SOURCE -DMATLAB_MEX_FILE -DMATLAB_MEXCMD_RELEASE=R2018a  -I"/opt/intel/mkl/include"  -I"/usr/local/MATLAB/R2018b/extern/include" -I"/usr/local/MATLAB/R2018b/simulink/include" -fwrapv -fPIC -fno-omit-frame-pointer  -lpthread -DMKL_ILP64 -m64 -lfftw3_omp -lfftw3 -lgomp -std=c++11 -O3  -DNDEBUG "./gpsr.cpp" -o gpsr.o

Then create the object for cpp_mexapi_version which comes with Matlab

/usr/bin/g++ -c -DMX_COMPAT_64  -DUSE_MEX_CMD   -D_GNU_SOURCE -DMATLAB_MEX_FILE -DMATLAB_MEXCMD_RELEASE=R2018a  -I"/opt/intel/mkl/include"  -I"/usr/local/MATLAB/R2018b/extern/include" -I"/usr/local/MATLAB/R2018b/simulink/include" -fwrapv -fPIC -fno-omit-frame-pointer  -lpthread -DMKL_ILP64 -m64 -lgomp -std=c++11 -O3 -DNDEBUG "/usr/local/MATLAB/R2018b/extern/version/cpp_mexapi_version.cpp" -o cpp_mexapi_version.o

Finally link and create the mex file

/usr/bin/g++ -DMX_COMPAT_64  -DMKL_ILP64 -m64 -shared -O3 -Wl,--version-script,"/usr/local/MATLAB/R2018b/extern/lib/glnxa64/c_exportsmexfileversion.map" gpsr.o cpp_mexapi_version.o -lfftw3_omp -lfftw3 -lgomp  -lpthread  -lm  -ldl   -L/opt/intel/mkl/lib/intel64   -Wl,-rpath-link,/usr/local/MATLAB/R2018b/bin/glnxa64 -L"/usr/local/MATLAB/R2018b/bin/glnxa64" -lmx -lmex -lmat -lstdc++ -Wl,--start-group  /opt/intel/mkl/lib/intel64/libmkl_intel_ilp64.a /opt/intel/mkl/lib/intel64/libmkl_gnu_thread.a /opt/intel/mkl/lib/intel64/libmkl_core.a -Wl,--end-group -o gpsr.mexa64


Note that all the path must be changed according to your system. 

To run the code puth the mex file in the matlab path and run

result =gpsr(y,H,tau,1e-12,maxiter,G,16,M*N*L,M,N,L,1);

where y are a vector with the measurements, H is an sparse matrix with the sensing matrix such that y=Hx, and x is the signal, tau is the l1 regularizer parameter (sparsity term), 1e-12 is the tolerance, maxiter the number of iterations, G and 16 are the filters of the wavelet transform (usually generated using wavelab package with MakeOnFilter function). M,N are the spatial dimensions and L the spectral dimension.
