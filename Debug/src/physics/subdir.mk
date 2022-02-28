################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/physics/Hitbox.cpp \
../src/physics/PhysicsSolver.cpp 

OBJS += \
./src/physics/Hitbox.o \
./src/physics/PhysicsSolver.o 

CPP_DEPS += \
./src/physics/Hitbox.d \
./src/physics/PhysicsSolver.d 


# Each subdirectory must supply rules for building sources it contributes
src/physics/%.o: ../src/physics/%.cpp src/physics/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


