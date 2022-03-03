################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/objects/Player.cpp 

OBJS += \
./src/objects/Player.o 

CPP_DEPS += \
./src/objects/Player.d 


# Each subdirectory must supply rules for building sources it contributes
src/objects/%.o: ../src/objects/%.cpp src/objects/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


