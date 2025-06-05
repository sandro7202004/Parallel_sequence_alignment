NVCC = /usr/local/cuda/bin/nvcc

all:
	make DW
	make BW 
	make COL
	$(NVCC) NW_Diagonal_GPU.cu -o NW_CUDA -arch=sm_60 -std=c++11 -I/usr/local/cuda/include

NW_diag:
	$(NVCC) NW_Diagonal_GPU.cu -o NW_CUDA -arch=sm_60 -std=c++11 -I/usr/local/cuda/include

clean:
	rm -f BW
	rm -f DW
	rm -f COL
	rm -f NW_CUDA
