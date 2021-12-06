################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/xilffs/diskio.c \
../src/xilffs/ff.c \
../src/xilffs/ffsystem.c \
../src/xilffs/ffunicode.c 

OBJS += \
./src/xilffs/diskio.o \
./src/xilffs/ff.o \
./src/xilffs/ffsystem.o \
./src/xilffs/ffunicode.o 

C_DEPS += \
./src/xilffs/diskio.d \
./src/xilffs/ff.d \
./src/xilffs/ffsystem.d \
./src/xilffs/ffunicode.d 


# Each subdirectory must supply rules for building sources it contributes
src/xilffs/%.o: ../src/xilffs/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: MicroBlaze g++ compiler'
	mb-g++ -Wall -O0 -g3 -c -fmessage-length=0 -MT"$@" -I../../LNDAQ512k_bsp/microblaze_0/include -mlittle-endian -mxl-barrel-shift -mxl-pattern-compare -mcpu=v11.0 -mno-xl-soft-mul -mhard-float -Wl,--no-relax -ffunction-sections -fdata-sections -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


