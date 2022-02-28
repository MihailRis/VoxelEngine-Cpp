################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/lighting/LightSolver.cpp \
../src/lighting/Lighting.cpp \
../src/lighting/Lightmap.cpp 

OBJS += \
./src/lighting/LightSolver.o \
./src/lighting/Lighting.o \
./src/lighting/Lightmap.o 

CPP_DEPS += \
./src/lighting/LightSolver.d \
./src/lighting/Lighting.d \
./src/lighting/Lightmap.d 


# Each subdirectory must supply rules for building sources it contributes
src/lighting/%.o: ../src/lighting/%.cpp src/lighting/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


