################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CC_SRCS += \
../src/IicMaster/IicMaster.cc 

CC_DEPS += \
./src/IicMaster/IicMaster.d 

OBJS += \
./src/IicMaster/IicMaster.o 


# Each subdirectory must supply rules for building sources it contributes
src/IicMaster/%.o: ../src/IicMaster/%.cc
	@echo 'Building file: $<'
	@echo 'Invoking: MicroBlaze g++ compiler'
	mb-g++ -Wall -O2 -c -fmessage-length=0 -MT"$@" -I../../daq24b512kca_bsp/microblaze_0/include -mlittle-endian -mxl-barrel-shift -mxl-pattern-compare -mcpu=v11.0 -mno-xl-soft-mul -mhard-float -Wl,--no-relax -ffunction-sections -fdata-sections -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


