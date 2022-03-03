# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/graphics/LineBatch.cpp \
../src/graphics/Mesh.cpp \
../src/graphics/Shader.cpp \
../src/graphics/Texture.cpp \
../src/graphics/VoxelRenderer.cpp \
../src/graphics/Batch2D.cpp

OBJS += \
./src/graphics/LineBatch.o \
./src/graphics/Mesh.o \
./src/graphics/Shader.o \
./src/graphics/Texture.o \
./src/graphics/VoxelRenderer.o \
./src/graphics/Batch2D.o 

CPP_DEPS += \
./src/graphics/LineBatch.d \
./src/graphics/Mesh.d \
./src/graphics/Shader.d \
./src/graphics/Texture.d \
./src/graphics/VoxelRenderer.d \
./src/graphics/Batch2D.d


# Each subdirectory must supply rules for building sources it contributes
src/graphics/%.o: ../src/graphics/%.cpp src/graphics/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


