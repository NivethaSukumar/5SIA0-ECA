#include "eeg.h"

// Based on code by: Mohammad Tahghighi
int32_t abssum(int np, int32_t *x)
{
    int i;
    int32_t s = 0;

    for (i = 0; i < np; i++) {
        s += abs(x[i]);
    }

    return s;
}

float average(int np, int32_t *x)
{
    int i;
    int32_t s = 0;

    for (i = 0; i < np; i++) {
        s += x[i];
    }

    return ((float) s) / ((float) np);
}

__global__
void gpu_average(int32_t *x, int32_t *blocksums)
{
    //Note that his kernel is merely an example, and is not necessarily the optimal way to calculate the sum/average on a GPU device

    // The id of this thread within our block
    unsigned int threadId = threadIdx.x;

    // The global id if this thread.
    // Since we launch np threads in total, each id maps to one unique element in x
    unsigned int globalId = blockIdx.x*blockDim.x + threadIdx.x;

    // NOTE: for debugging you can print directly from the GPU device
    // however, if you print a lot, or the GPU encounters a serious error, this might fail
    // also performance is horrible, so make sure to disable it for benchmarking
    //printf("Hello from global thread %d, with local id %d\n",globalId,threadId);

    // Lets first copy the data from global GPU memory to shared memory
    // Shared memory is only accesible within a threadblock, but it is much faster to access than global memory once you have data in there
    // Note that by having the keyword "extern" and empty brackets [], the size of the array will be determined at runtime.
    // You will however have to pass the size in bytes as a 3rd argument to the kernel call (see the "sharedMemBytes" variable)
    // If you statically know the size of the shared memory array, it is probably faster to use that (Disclaimer: I did not test this claim)
    extern __shared__ int32_t blockData[];
    blockData[threadId]=x[globalId];

    // We synchronize the threads here, to make sure every thread has copied valid data from global to local memory
    // Otherwise we potentially risk accessing uninitialized data in the shared memory
    __syncthreads();
/*
    extern __shared__ int32_t blockData[];
    unsigned int threadId = threadIdx.x;
    unsigned int globalId = (blockIdx.x)*((blockDim.x)*2)+threadIdx.x;
    blockData[threadId] = x[globalId] + x[globalId+blockDim.x];
    __syncthreads();

*/

    // The next step is summation of the elements in our blockData
    // The summation is done in a tree like fashion, as illustrated below
    // 0 1 2 3 4 5 6 7  (number of parallel summations)
    // |/  |/  |/  |/   (4)
    // 1   5   9   13
    // |__/    |__/     (2)
    // 6       22
    // |______/         (1)
    // 28


      /*Optimization 1*/
  /*  for(unsigned int s=1;s<blockDim.x;s*=2){
        // Because the amount of work reduces, we use the threadId to determine which threads get to execute the summation
        // The other threads will idle in the meantime (They will be masked during the execution of the conditional-part)
        if (threadId % (2*s) == 0 ){
            blockData[threadId] += blockData[threadId+s];
        }

        // For each layer of the tree, we have to make sure all threads finish their computations
        // otherwise we could read unsummed results
        __syncthreads();
    }  */

/****Optimization 2***********/
/*
    for(unsigned int s=1; s<blockDim.x;s*=2){

      int index = 2*s*threadId;
      if(index<blockDim.x){
        blockData[index] += blockData[index+s];
      }
      __syncthreads();

    }
    */

  /*******Optimization 3*********/

for(unsigned int s=blockDim.x/2;s>0;s>>=1){

  if (threadId<s) {
    blockData[threadId] += blockData[threadId+s];
  }
  __syncthreads();

}

/***********Optimization 4******didnt work******/
/*
for(unsigned int s=(blockDim.x)/2;s>32;s>>=1){

  if (threadId<s)
    blockData[threadId] += blockData[threadId+s];
  __syncthreads();

}

if(threadId<32)
{
  blockData[threadId] += blockData[threadId+32];
  blockData[threadId] += blockData[threadId+16];
  blockData[threadId] += blockData[threadId+8];
  blockData[threadId] += blockData[threadId+4];
  blockData[threadId] += blockData[threadId+2];
  blockData[threadId] += blockData[threadId+1];
}

*/

    //we let 1 selected thread per block write out our local sum to the global memory
    if(threadId==0){

        #ifdef DEBUG
        //example debugging, print the partial sum of each block with the block id
        printf("GPU Block %d sum: %d\n",blockIdx.x, blockData[0]);
        #endif

        //write the sum of this block to the blocksums array
        blocksums[blockIdx.x]=blockData[0];
    }

    //this will return the control to the CPU once all threads finish (reach this point)
    return;
}

/********My Code*****************************************/

__global__
void gpu_abssum(int32_t *x, float *blocksums)
{
    unsigned int threadId = threadIdx.x;

    unsigned int globalId = blockIdx.x*blockDim.x + threadIdx.x;

    extern __shared__ float blockData_abs[];
    blockData_abs[threadId]=x[globalId];

    __syncthreads();


/* Optimization 1*/
/*
    for(unsigned int s=1;s<blockDim.x;s*=2){

        if (threadId % (2*s) == 0 ){
              blockData_abs[threadId] += blockData_abs[threadId+s];
        }


        __syncthreads();
    } */


    /****Optimization 2***********/
    /*
        for(unsigned int s=1; s<blockDim.x;s*=2){

          int index = 2*s*threadId;
          if(index<blockDim.x){
            blockData_abs[index] += blockData_abs[index+s];
          }
          __syncthreads();

        }
        */
/* Optimization 3*/
    for(unsigned int s=blockDim.x/2;s>0;s>>=1){

      if (threadId<s) {
        blockData_abs[threadId] += blockData_abs[threadId+s];
      //  printf("%f\n",blockData_abs[threadId] );
      }
      __syncthreads();

    }
/* didnt work
    for(unsigned int s=(blockDim.x)/2;s>32;s>>=1){

      if (threadId<s)
        blockData_abs[threadId] += blockData_abs[threadId+s];
      __syncthreads();

    }

    if(threadId<32)
    {
      blockData_abs[threadId] += blockData_abs[threadId+32];
      blockData_abs[threadId] += blockData_abs[threadId+16];
      blockData_abs[threadId] += blockData_abs[threadId+8];
      blockData_abs[threadId] += blockData_abs[threadId+4];
      blockData_abs[threadId] += blockData_abs[threadId+2];
      blockData_abs[threadId] += blockData_abs[threadId+1];
    } */

    if(threadId==0){


        blocksums[blockIdx.x]=blockData_abs[0];
    }

    return;
}


__global__
void cuda_abs(int32_t *arr_data, int32_t *result){

  result[blockIdx.x] = abs(arr_data[blockIdx.x]);

  return;


}




float variance(int np, int32_t *x, float avg)
{
    int i;
    float s = 0;

    // Variance = Sum((x - avg)^2)
    for (i = 0; i < np; i++) {
        float tmp = x[i] - avg;
        s += (tmp * tmp);
    }

    return s / ((float) np);
}

float stddev(int np, int32_t *x, float avg)
{
    // Stddev = sqrt(variance)
    float var = variance(np, x, avg);
    return sqrt(var);
}

__global__
void cuda_xisubavg(int32_t *arr_data, float *result, float avg){

  result[blockIdx.x] = arr_data[blockIdx.x]-avg;

  return;


}

__global__
void cuda_square(float *arr_data, float *result){

  result[blockIdx.x] = arr_data[blockIdx.x]*arr_data[blockIdx.x];

  return;


}


__global__
void cuda_floataddition(float *x, float *blocksums)
{
    unsigned int threadId = threadIdx.x;

    unsigned int globalId = blockIdx.x*blockDim.x + threadIdx.x;

    extern __shared__ float blockData_abs[];
    blockData_abs[threadId]=x[globalId];

    __syncthreads();

    /* Optimization 1*/
    /*
        for(unsigned int s=1;s<blockDim.x;s*=2){

            if (threadId % (2*s) == 0 ){
                  blockData_abs[threadId] += blockData_abs[threadId+s];
            }


            __syncthreads();
        } */


        /****Optimization 2***********/
        /*
            for(unsigned int s=1; s<blockDim.x;s*=2){

              int index = 2*s*threadId;
              if(index<blockDim.x){
                blockData_abs[index] += blockData_abs[index+s];
              }
              __syncthreads();

            }
            */
      /*Optimization 3*/

    for(unsigned int s=blockDim.x/2;s>0;s>>=1){

      if (threadId<s) {
        blockData_abs[threadId] += blockData_abs[threadId+s];
      //  printf("%f\n",blockData_abs[threadId] );
      }
      __syncthreads();

    }

    if(threadId==0){


        blocksums[blockIdx.x]=blockData_abs[0];
    }

    return;
}


float cuda_stddev(int np, int32_t *x, float avg)
{
  cudaError_t err;
  int32_t *d_stddev;
  float *fltd_stddev;
  float *stddev_result;

  int numBlocks=4;
  int threadsPerBlock=np/numBlocks;

  float sharedMemBytes = threadsPerBlock*sizeof(float);
  float* device_blocksums;

  err=cudaMalloc(&d_stddev, np*sizeof(int32_t));
  cudaCheckError(err);

  err=cudaMalloc(&stddev_result, np*sizeof(float));
  cudaCheckError(err);

  err=cudaMemcpy(d_stddev,x, np*sizeof(int32_t), cudaMemcpyHostToDevice);
  cudaCheckError(err);

  cuda_xisubavg<<<np,1>>>(d_stddev,stddev_result, avg);
  cudaCheckError(cudaPeekAtLastError());

  float xisubavgres[DATAPOINTS];
  err=cudaMemcpy(xisubavgres,stddev_result, np*sizeof(float), cudaMemcpyDeviceToHost);
  cudaCheckError(err);

  err=cudaFree(d_stddev);
  cudaCheckError(err);
  err=cudaFree(stddev_result);
  cudaCheckError(err);

  err=cudaMalloc(&fltd_stddev, np*sizeof(float));
  cudaCheckError(err);

  err=cudaMalloc(&stddev_result, np*sizeof(float));
  cudaCheckError(err);

  err=cudaMemcpy(fltd_stddev,xisubavgres, np*sizeof(float), cudaMemcpyHostToDevice);
  cudaCheckError(err);

  cuda_square<<<np,1>>>(fltd_stddev,stddev_result);
  cudaCheckError(cudaPeekAtLastError());

  float square[DATAPOINTS];
  err=cudaMemcpy(square,stddev_result, np*sizeof(float), cudaMemcpyDeviceToHost);
  cudaCheckError(err);

  err=cudaFree(fltd_stddev);
  cudaCheckError(err);
  err=cudaFree(stddev_result);
  cudaCheckError(err);

  err=cudaMalloc(&fltd_stddev, np*sizeof(float));
  cudaCheckError(err);

  err=cudaMalloc(&device_blocksums, np*sizeof(float));
  cudaCheckError(err);

  err=cudaMemcpy(fltd_stddev,square, np*sizeof(float), cudaMemcpyHostToDevice);
  cudaCheckError(err);

  cuda_floataddition<<<numBlocks,threadsPerBlock, sharedMemBytes>>>(fltd_stddev, device_blocksums);
  cudaCheckError(cudaPeekAtLastError());

  float blocksums_floatadd[numBlocks];
  err=cudaMemcpy(blocksums_floatadd, device_blocksums, numBlocks*sizeof(int32_t), cudaMemcpyDeviceToHost);
  cudaCheckError(err);

  err=cudaFree(fltd_stddev);
  cudaCheckError(err);

  err=cudaFree(device_blocksums);
  cudaCheckError(err);

  float var_sum=0;
  for(uint32_t blk=0;blk<numBlocks;blk++){
    var_sum+=blocksums_floatadd[blk];
  //  printf("%d\n",abs_sum );
  }

  float variance = var_sum/(float)np;

  return sqrt(variance);


}


int mean_crosstimes(int np, int32_t *x, float avg)
{
    int i;
    bool negative = x[0] < avg;
    int count = 0;

    // Count number of zero crossings for (x - avg)
    for (i = 0; i < np; i++) {
        if (negative) {
            if (x[i] > avg) {
                negative = false;
                count++;
            }
        } else {
            if (x[i] < avg) {
                negative = true;
                count++;
            }
        }
    }

    return count;
}

void stafeature(int np, int32_t *x, float *sta)
{
    // Returns sta = [mean, std, abssum, mean_crosstimes)


    #ifdef CPU_ONLY
    //original CPU code
    float avg = average(np, x);
    sta[0] = avg;
    sta[1] = stddev(np, x, avg);
    sta[2] = abssum(np, x);
    sta[3] = mean_crosstimes(np, x, avg);
    #else

    cudaError_t err;
    int32_t *d_abs;
    int32_t *abs_result;

    err=cudaMalloc(&d_abs, np*sizeof(int32_t));
    cudaCheckError(err);

    err=cudaMalloc(&abs_result, np*sizeof(int32_t));
    cudaCheckError(err);

    err=cudaMemcpy(d_abs,x, np*sizeof(int32_t), cudaMemcpyHostToDevice);
    cudaCheckError(err);

    cuda_abs<<<np,1>>>(d_abs,abs_result);
    cudaCheckError(cudaPeekAtLastError());

    int32_t abs_x[DATAPOINTS];
    err=cudaMemcpy(abs_x,abs_result, np*sizeof(int32_t), cudaMemcpyDeviceToHost);
    cudaCheckError(err);

    err=cudaFree(d_abs);
    cudaCheckError(err);
    err=cudaFree(abs_result);
    cudaCheckError(err);

    /*for (int i = 0; i < np; i++) {
      abs_x[i] = abs(x[i]);
    } */


    //GPU code

    /*
     *  Our high level strategy to calculate the average in parallel in this example is to split the input array into into a number of blocks (numBlocks).
     *  Each block contains thus np/numBlocks elements
     *  These blocks will be mapped to the Streaming Multiprocessors of the GPU.
     *  For each block we calculate the sum
     *  Finally the sums of all the blocks are added on the CPU
    */

    //NOTE: take care np is a multiple of numBlocks for this example.
    int numBlocks=256;
    int threadsPerBlock=np/numBlocks; //i.e., this should have remainder==0

    //Ignore the next bit untill you are inspecting the gpu kernel code, then refer back to it. On the first read just ignore it ;)
    //Because in this setup the amount of required shared memory depends on np we assume it is only known at runtime. (although you can of course get it from the input and assume it fixed for this assigment)
    //the number of required shared memory bytes need to be passed as a 3rd argument to the kernel call later on
    //see the remarks in the gpu_average code
    int sharedMemBytes = threadsPerBlock*sizeof(int32_t);

    //variable for holding return values of cuda functions


    //start by allocating room for array "x" on the global memory of the GPU
    int32_t* device_x;
    err=cudaMalloc(&device_x, np*sizeof(int32_t));

    //Here we check for errors of this cuda call
    //See eeg.h for the implementation of this error check (it's not a default cuda function)
    cudaCheckError(err);

    //also allocate room for the all the sums of the blocks
    int32_t* device_blocksums;
    //Note that room is allocated in global memory for the sum of *each* threadblock
    err=cudaMalloc(&device_blocksums, numBlocks*sizeof(int32_t));
    cudaCheckError(err);

    //Now copy array "x" from the CPU to the GPU
    err=cudaMemcpy(device_x,x, np*sizeof(int32_t), cudaMemcpyHostToDevice);
    cudaCheckError(err);

    //Compute the average on the GPU
    gpu_average<<<numBlocks,threadsPerBlock, sharedMemBytes>>>(device_x, device_blocksums);

    //We use "peekatlasterror" since a kernel launch does not return a cudaError_t to check for errors
    cudaCheckError(cudaPeekAtLastError());

    //copy the sums of each block back from GPU global memory to CPU memory
    int32_t blocksums[numBlocks];
    err=cudaMemcpy(blocksums, device_blocksums, numBlocks*sizeof(int32_t), cudaMemcpyDeviceToHost);
    cudaCheckError(err);

    //free the memory on the GPU
    //Optimalisation Hint: if you do not free the memory, the values will be preserved between multiple kernel calls!
    //For example, the x-array will remain in the GPU global memory if you also map other features to the GPU
    err=cudaFree(device_x);
    cudaCheckError(err);
    err=cudaFree(device_blocksums);
    cudaCheckError(err);

    #ifdef DEBUG
    //print all the block sums calculated by CPU
    for(int b=0;b<numBlocks;b++){
        int sum=0;
        for (int i=0;i<threadsPerBlock;i++)
            sum+=x[b*threadsPerBlock+i];
        printf("CPU Block %d sum: %d\n",b,sum);
    }
    #endif

    //Now add all the block sums on the CPU
    //(Note: if you have many blocks, you might consider mapping this to another GPU call of course)
    int32_t sum=0;
    for(uint32_t blk=0;blk<numBlocks;blk++)
        sum+=blocksums[blk];
    float avg = (float)(sum)/(float)(np);


    #ifdef DEBUG
    //Compare total sum of GPU and CPU
    printf("GPU Total sum: %d\n",sum);
    int cpu_sum=0;
    for(int i=0;i<np;i++)
        cpu_sum+=x[i];
    printf("CPU Total sum: %d\n",cpu_sum);
    #endif

    #ifdef DEBUG
    //compare the average
    printf("GPU average: %f\n",avg);
    printf("CPU average: %f\n",(float)cpu_sum/(float)np);
    #endif
/*******My Code***************************************************************************/
//int32_t* device_x;

err=cudaMalloc(&device_x, np*sizeof(int32_t));
cudaCheckError(err);

float* device_blocksums_abs;
err=cudaMalloc(&device_blocksums_abs, numBlocks*sizeof(int32_t));
cudaCheckError(err);

err=cudaMemcpy(device_x,abs_x, np*sizeof(int32_t), cudaMemcpyHostToDevice);
cudaCheckError(err);

gpu_abssum<<<numBlocks,threadsPerBlock, sharedMemBytes>>>(device_x, device_blocksums_abs);

cudaCheckError(cudaPeekAtLastError());

float blocksums_abs[numBlocks];
err=cudaMemcpy(blocksums_abs, device_blocksums_abs, numBlocks*sizeof(int32_t), cudaMemcpyDeviceToHost);
cudaCheckError(err);

err=cudaFree(device_x);
cudaCheckError(err);

err=cudaFree(device_blocksums_abs);
cudaCheckError(err)

int32_t abs_sum=0;
for(uint32_t blk=0;blk<numBlocks;blk++){
  abs_sum+=abs(blocksums_abs[blk]);
//  printf("%d\n",abs_sum );
}


    //calculate all other features on the CPU for this example
    sta[0] = avg;
    //sta[1] = stddev(np, x, avg);
    sta[1] = cuda_stddev(np, x, avg);
  //sta[2] = abssum(np, x);
    sta[2] = abs_sum;
    sta[3] = mean_crosstimes(np, x, avg);

    #endif
}
