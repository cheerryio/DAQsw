################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../xilffs/cc932.c \
../xilffs/ccsbcs.c \
../xilffs/diskio.c \
../xilffs/ff.c \
../xilffs/syscall.c 

OBJS += \
./xilffs/cc932.o \
./xilffs/ccsbcs.o \
./xilffs/diskio.o \
./xilffs/ff.o \
./xilffs/syscall.o 

C_DEPS += \
./xilffs/cc932.d \
./xilffs/ccsbcs.d \
./xilffs/diskio.d \
./xilffs/ff.d \
./xilffs/syscall.d 


# Each subdirectory must supply rules for building sources it contributes
xilffs/%.o: ../xilffs/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: MicroBlaze g++ compiler'
	mb-g++ -Wall -O0 -g3 -c -fmessage-length=0 -MT"$@" -I../../LNDAQ512k_bsp/microblaze_0/include -mlittle-endian -mxl-barrel-shift -mxl-pattern-compare -mcpu=v11.0 -mno-xl-soft-mul -mhard-float -Wl,--no-relax -ffunction-sections -fdata-sections -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


