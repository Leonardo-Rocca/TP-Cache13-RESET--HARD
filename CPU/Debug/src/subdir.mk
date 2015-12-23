################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/CpuArchivoDeConfiguracion.c \
../src/cpu.c \
../src/cpuConexiones.c 

OBJS += \
./src/CpuArchivoDeConfiguracion.o \
./src/cpu.o \
./src/cpuConexiones.o 

C_DEPS += \
./src/CpuArchivoDeConfiguracion.d \
./src/cpu.d \
./src/cpuConexiones.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


