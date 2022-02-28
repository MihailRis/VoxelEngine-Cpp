################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/voxels/Block.cpp \
../src/voxels/Chunk.cpp \
../src/voxels/Chunks.cpp \
../src/voxels/ChunksController.cpp \
../src/voxels/ChunksLoader.cpp \
../src/voxels/WorldGenerator.cpp \
../src/voxels/voxel.cpp 

OBJS += \
./src/voxels/Block.o \
./src/voxels/Chunk.o \
./src/voxels/Chunks.o \
./src/voxels/ChunksController.o \
./src/voxels/ChunksLoader.o \
./src/voxels/WorldGenerator.o \
./src/voxels/voxel.o 

CPP_DEPS += \
./src/voxels/Block.d \
./src/voxels/Chunk.d \
./src/voxels/Chunks.d \
./src/voxels/ChunksController.d \
./src/voxels/ChunksLoader.d \
./src/voxels/WorldGenerator.d \
./src/voxels/voxel.d 


# Each subdirectory must supply rules for building sources it contributes
src/voxels/%.o: ../src/voxels/%.cpp src/voxels/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


