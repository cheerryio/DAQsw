################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/sdc_ctrl/mmc.c \
../src/sdc_ctrl/ocsdc.c 

OBJS += \
./src/sdc_ctrl/mmc.o \
./src/sdc_ctrl/ocsdc.o 

C_DEPS += \
./src/sdc_ctrl/mmc.d \
./src/sdc_ctrl/ocsdc.d 


# Each subdirectory must supply rules for building sources it contributes
src/sdc_ctrl/%.o: ../src/sdc_ctrl/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: MicroBlaze g++ compiler'
	mb-g++ -Wall -O2 -c -fmessage-length=0 -MT"$@" -I../../daq24b512kca_bsp/microblaze_0/include -mlittle-endian -mxl-barrel-shift -mxl-pattern-compare -mcpu=v11.0 -mno-xl-soft-mul -mhard-float -Wl,--no-relax -ffunction-sections -fdata-sections -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


