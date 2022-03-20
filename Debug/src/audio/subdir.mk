################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/audio/Audio.cpp \
../src/audio/audioutil.cpp 

OBJS += \
./src/audio/Audio.o \
./src/audio/audioutil.o 

CPP_DEPS += \
./src/audio/Audio.d \
./src/audio/audioutil.d 


# Each subdirectory must supply rules for building sources it contributes
src/audio/%.o: ../src/audio/%.cpp src/audio/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


