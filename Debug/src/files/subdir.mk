################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/files/WorldFiles.cpp \
../src/files/files.cpp 

OBJS += \
./src/files/WorldFiles.o \
./src/files/files.o 

CPP_DEPS += \
./src/files/WorldFiles.d \
./src/files/files.d 


# Each subdirectory must supply rules for building sources it contributes
src/files/%.o: ../src/files/%.cpp src/files/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


