################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Sources/CANTP.c \
../Sources/CAN_standard.c \
../Sources/DTC.c \
../Sources/UDS_ECU.c \
../Sources/main.c 

OBJS += \
./Sources/CANTP.o \
./Sources/CAN_standard.o \
./Sources/DTC.o \
./Sources/UDS_ECU.o \
./Sources/main.o 

C_DEPS += \
./Sources/CANTP.d \
./Sources/CAN_standard.d \
./Sources/DTC.d \
./Sources/UDS_ECU.d \
./Sources/main.d 


# Each subdirectory must supply rules for building sources it contributes
Sources/%.o: ../Sources/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Standard S32DS C Compiler'
	arm-none-eabi-gcc "@Sources/CANTP.args" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


