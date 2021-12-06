################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CC_SRCS += \
../src/MiniDmaFifo2Mem/MiniDmaFifo2Mem.cc 

CC_DEPS += \
./src/MiniDmaFifo2Mem/MiniDmaFifo2Mem.d 

OBJS += \
./src/MiniDmaFifo2Mem/MiniDmaFifo2Mem.o 


# Each subdirectory must supply rules for building sources it contributes
src/MiniDmaFifo2Mem/%.o: ../src/MiniDmaFifo2Mem/%.cc
	@echo 'Building file: $<'
	@echo 'Invoking: MicroBlaze g++ compiler'
	mb-g++ -Wall -O0 -g3 -c -fmessage-length=0 -MT"$@" -I../../LNDAQ512k_bsp/microblaze_0/include -mlittle-endian -mxl-barrel-shift -mxl-pattern-compare -mcpu=v11.0 -mno-xl-soft-mul -mhard-float -Wl,--no-relax -ffunction-sections -fdata-sections -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


