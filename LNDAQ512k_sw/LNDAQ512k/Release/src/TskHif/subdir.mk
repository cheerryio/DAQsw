################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CC_SRCS += \
../src/TskHif/MenuDefines.cc \
../src/TskHif/PCF8563.cc \
../src/TskHif/TskHif.cc 

CC_DEPS += \
./src/TskHif/MenuDefines.d \
./src/TskHif/PCF8563.d \
./src/TskHif/TskHif.d 

OBJS += \
./src/TskHif/MenuDefines.o \
./src/TskHif/PCF8563.o \
./src/TskHif/TskHif.o 


# Each subdirectory must supply rules for building sources it contributes
src/TskHif/%.o: ../src/TskHif/%.cc
	@echo 'Building file: $<'
	@echo 'Invoking: MicroBlaze g++ compiler'
	mb-g++ -Wall -O2 -c -fmessage-length=0 -MT"$@" -I../../LNDAQ512k_bsp/microblaze_0/include -mlittle-endian -mxl-barrel-shift -mxl-pattern-compare -mcpu=v11.0 -mno-xl-soft-mul -mhard-float -Wl,--no-relax -ffunction-sections -fdata-sections -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


