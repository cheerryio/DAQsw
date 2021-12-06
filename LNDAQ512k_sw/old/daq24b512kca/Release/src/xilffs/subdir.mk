################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/xilffs/cc932.c \
../src/xilffs/ccsbcs.c \
../src/xilffs/diskio.c \
../src/xilffs/ff.c \
../src/xilffs/syscall.c 

OBJS += \
./src/xilffs/cc932.o \
./src/xilffs/ccsbcs.o \
./src/xilffs/diskio.o \
./src/xilffs/ff.o \
./src/xilffs/syscall.o 

C_DEPS += \
./src/xilffs/cc932.d \
./src/xilffs/ccsbcs.d \
./src/xilffs/diskio.d \
./src/xilffs/ff.d \
./src/xilffs/syscall.d 


# Each subdirectory must supply rules for building sources it contributes
src/xilffs/%.o: ../src/xilffs/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: MicroBlaze g++ compiler'
	mb-g++ -Wall -O2 -c -fmessage-length=0 -MT"$@" -I../../daq24b512kca_bsp/microblaze_0/include -mlittle-endian -mxl-barrel-shift -mxl-pattern-compare -mcpu=v11.0 -mno-xl-soft-mul -mhard-float -Wl,--no-relax -ffunction-sections -fdata-sections -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


