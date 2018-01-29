# OS Name (Linux or Darwin)
OSUPPER = $(shell uname -s 2>/dev/null | tr [:lower:] [:upper:])
OSLOWER = $(shell uname -s 2>/dev/null | tr [:upper:] [:lower:])

# Flags to detect 32-bit or 64-bit OS platform
OS_SIZE = $(shell uname -m | sed -e "s/i.86/32/" -e "s/x86_64/64/")
OS_ARCH = $(shell uname -m | sed -e "s/i386/i686/")

# These flags will override any settings
ifeq ($(i386),1)
	OS_SIZE = 32
	OS_ARCH = i686
endif

ifeq ($(x86_64),1)
	OS_SIZE = 64
	OS_ARCH = x86_64
endif

# Flags to detect either a Linux system (linux) or Mac OSX (darwin)
DARWIN = $(strip $(findstring DARWIN, $(OSUPPER)))

# Location of the CUDA Toolkit binaries and libraries
CUDA_PATH       ?= /usr/local/cuda
CUDA_INC_PATH   ?= $(CUDA_PATH)/include
CUDA_BIN_PATH   ?= $(CUDA_PATH)/bin
ifneq ($(DARWIN),)
  CUDA_LIB_PATH  ?= $(CUDA_PATH)/lib
else
  ifeq ($(OS_SIZE),32)
    CUDA_LIB_PATH  ?= $(CUDA_PATH)/lib
  else
    CUDA_LIB_PATH  ?= $(CUDA_PATH)/lib64
  endif
endif


# Common binaries
#NVCC            ?= $(CUDA_BIN_PATH)/nvcc
NVCC            ?= nvcc
CC 				=  g++

# Extra user flags
EXTRA_NVCCFLAGS ?=
EXTRA_LDFLAGS   ?=
#CFLAGS = -O3 -Wall std=gnu99
CFLAGS=-O3 -Wall

# CUDA code generation flags
GENCODE_SM10    := -Wno-deprecated-gpu-targets -gencode arch=compute_10,code=sm_10
GENCODE_SM20    := -Wno-deprecated-gpu-targets -gencode arch=compute_20,code=sm_20
GENCODE_SM30    := -gencode arch=compute_30,code=sm_30

#NOTE: Set this based on your device!!
GENCODE_FLAGS   := $(GENCODE_SM20)


# OS-specific build flags
ifneq ($(DARWIN),)
      ifneq ($(CU_SRCS),)
          LDFLAGS   := -Xlinker -rpath $(CUDA_LIB_PATH) -L$(CUDA_LIB_PATH) -lcudart
      endif
      CCFLAGS   := -arch $(OS_ARCH)
else
  ifeq ($(OS_SIZE),32)
      ifneq ($(CU_SRCS),)
          LDFLAGS   := -L$(CUDA_LIB_PATH) -lcudart
      endif
      CCFLAGS   := -m32
  else
      ifneq ($(CU_SRCS),)
          LDFLAGS   := -L$(CUDA_LIB_PATH) -lcudart
      endif
      CCFLAGS   := -m64
  endif
endif

# OS-architecture specific flags
ifeq ($(OS_SIZE),32)
      NVCCFLAGS := -m32
else
      NVCCFLAGS := -m64
endif

# Common includes and paths for CUDA
INCLUDES      := -I$(CUDA_INC_PATH) -I.

#openCL libs
CL_LIBS=OpenCL

SRCS=$(wildcard *.c)
CPP_SRCS=$(wildcard *.cpp)
CU_SRCS=$(wildcard *.cu)
OBJS=$(SRCS:.c=.o) $(CPP_SRCS:.cpp=.o) $(CU_SRCS:.cu=.o)
DEPEND=$(OBJS:%.o=%.d)
EXE=eeg

#if there are any cuda sources, we need to add lcudart to the linker flags
ifneq ($(CU_SRCS),)
LDFLAGS+=-lcudart
endif

#Target Rules
check:output.txt reference.txt
	@cat output.txt; cmp output.txt reference.txt && echo "Output is Correct!" || echo "Output MISMATCH!!"

output.txt:$(EXE)
	./$(EXE) 2> output.txt

run:$(EXE)
	./$(EXE)

$(EXE):$(OBJS)
	$(CC) $(OBJS) -L $(CUDA_LIB_PATH) -l$(CL_LIBS) $(LDFLAGS) $(EXTRA_LDFLAGS) -o $(EXE)

%.o:%.cpp
	$(CC) -MMD -MF $(subst .o,.d,$@) $(INCLUDES) -c $(CFLAGS) $< -o $@

%.o:%.c
	$(CC) -MMD -MF $(subst .o,.d,$@) $(INCLUDES) -c $(CFLAGS) $< -o $@

%.o:%.cu
	@$(NVCC) $(NVCCFLAGS) --generate-dependencies $< -o $(subst .o,.d,$@)
	$(NVCC) $(NVCCFLAGS) $(EXTRA_NVCCFLAGS) $(GENCODE_FLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -rf $(OBJS) $(CU_OBJS) $(DEPEND) $(EXE) $(USER).zip output.txt

zip:$(USER).zip
$(USER).zip:clean
	zip $(USER).zip ./* -x EEG.csv -x reference.txt

-include $(DEPEND)
