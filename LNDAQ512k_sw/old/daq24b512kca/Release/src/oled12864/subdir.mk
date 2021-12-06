################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CC_SRCS += \
../src/oled12864/lq12864.cc 

C_SRCS += \
../src/oled12864/oled_fonts.c 

CC_DEPS += \
./src/oled12864/lq12864.d 

OBJS += \
./src/oled12864/lq12864.o \
./src/oled12864/oled_fonts.o 

C_DEPS += \
./src/oled12864/oled_fonts.d 


# Each subdirectory must supply rules for building sources it contributes
src/oled12864/%.o: ../src/oled12864/%.cc
	@echo 'Building file: $<'
	@echo 'Invoking: MicroBlaze g++ compiler'
	mb-g++ -Wall -O2 -c -fmessage-length=0 -MT"$@" -I../../daq24b512kca_bsp/microblaze_0/include -mlittle-endian -mxl-barrel-shift -mxl-pattern-compare -mcpu=v11.0 -mno-xl-soft-mul -mhard-float -Wl,--no-relax -ffunction-sections -fdata-sections -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/oled12864/%.o: ../src/oled12864/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: MicroBlaze g++ compiler'
	mb-g++ -Wall -O2 -c -fmessage-length=0 -MT"$@" -I../../daq24b512kca_bsp/microblaze_0/include -mlittle-endian -mxl-barrel-shift -mxl-pattern-compare -mcpu=v11.0 -mno-xl-soft-mul -mhard-float -Wl,--no-relax -ffunction-sections -fdata-sections -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


