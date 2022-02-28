################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/window/Camera.cpp \
../src/window/Events.cpp \
../src/window/Window.cpp 

OBJS += \
./src/window/Camera.o \
./src/window/Events.o \
./src/window/Window.o 

CPP_DEPS += \
./src/window/Camera.d \
./src/window/Events.d \
./src/window/Window.d 


# Each subdirectory must supply rules for building sources it contributes
src/window/%.o: ../src/window/%.cpp src/window/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


