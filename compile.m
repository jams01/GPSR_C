MY_LFLAGS = '';
MY_OPTIM_FLAGS = '-O3';
MKLROOT = '/opt/intel/mkl';
EIGENROOT = '/usr/include/eigen3/';
SRC = ['gpsr.cpp'];
MKL_LFLAGS = ['-Wl,--start-group ' MKLROOT '/lib/intel64/libmkl_intel_ilp64.a '...
    MKLROOT '/lib/intel64/libmkl_gnu_thread.a '...
    MKLROOT '/lib/intel64/libmkl_core.a -Wl,--end-group -lgomp -lpthread -lm -ldl'];
mex('-v','-g','-R2018a',['COPTIMFLAGS="' MY_OPTIM_FLAGS '"'], ['CFLAGS=" -fopenmp -m64 -I'...
    MKLROOT '/include -I' EIGENROOT ...
    '  -I"/usr/local/MATLAB/R2018b/extern/include" -I"/usr/local/MATLAB/R2018b/simulink/include" -I"/home/jams/GPSR" -fPIC"'],...
    ['CLIBS="' MY_LFLAGS ' -fopenmp -shared -O -lstdc++ -lmex ' MKL_LFLAGS '"'], SRC);
%a=['mex -v COPTIMFLAGS="' MY_OPTIM_FLAGS '" CFLAGS=" -fopenmp -m64 -I' MKLROOT '/include -I' EIGENROOT '  -I"/usr/local/MATLAB/R2018a/extern/include" -I"/usr/local/MATLAB/R2018a/simulink/include" -I"/home/jams/GPSR" -fPIC" CLIBS="' MY_LFLAGS ' -fopenmp -shared -O -lstdc++ -lmex ' MKL_LFLAGS '"']



/usr/bin/g++ -DMX_COMPAT_64  -DMKL_ILP64 -m64 -shared -O3 -Wl,--version-script,"/usr/local/MATLAB/R2018b/extern/lib/glnxa64/c_exportsmexfileversion.map" gpsr.o cpp_mexapi_version.o -lfftw3_omp -lfftw3 -lgomp  -lpthread  -lm  -ldl   -L/opt/intel/mkl/lib/intel64   -Wl,-rpath-link,/usr/local/MATLAB/R2018b/bin/glnxa64 -L"/usr/local/MATLAB/R2018b/bin/glnxa64" -lmx -lmex -lmat -lstdc++ -Wl,--start-group  /opt/intel/mkl/lib/intel64/libmkl_intel_ilp64.a /opt/intel/mkl/lib/intel64/libmkl_gnu_thread.a /opt/intel/mkl/lib/intel64/libmkl_core.a -Wl,--end-group -o gpsr.mexa64
/usr/bin/g++ -c -DMX_COMPAT_64  -DUSE_MEX_CMD   -D_GNU_SOURCE -DMATLAB_MEX_FILE -DMATLAB_MEXCMD_RELEASE=R2018a  -I"/opt/intel/mkl/include"  -I"/usr/local/MATLAB/R2018b/extern/include" -I"/usr/local/MATLAB/R2018b/simulink/include" -fwrapv -fPIC -fno-omit-frame-pointer  -lpthread -DMKL_ILP64 -m64 -lgomp -std=c++11 -O3 -DNDEBUG "/usr/local/MATLAB/R2018b/extern/version/cpp_mexapi_version.cpp" -o cpp_mexapi_version.o
/usr/bin/g++ -c -DMX_COMPAT_64  -DUSE_MEX_CMD -D_GNU_SOURCE -DMATLAB_MEX_FILE -DMATLAB_MEXCMD_RELEASE=R2018a  -I"/opt/intel/mkl/include"  -I"/usr/local/MATLAB/R2018b/extern/include" -I"/usr/local/MATLAB/R2018b/simulink/include" -fwrapv -fPIC -fno-omit-frame-pointer  -lpthread -DMKL_ILP64 -m64 -lfftw3_omp -lfftw3 -lgomp -std=c++11 -O3  -DNDEBUG "./gpsr.cpp" -o gpsr.o
