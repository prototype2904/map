################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/CONCURRENT_HOPSCOTCH_HASHING.cpp \
../src/ConcurrentHopscotchHashSet.cpp 

OBJS += \
./src/CONCURRENT_HOPSCOTCH_HASHING.o \
./src/ConcurrentHopscotchHashSet.o 

CPP_DEPS += \
./src/CONCURRENT_HOPSCOTCH_HASHING.d \
./src/ConcurrentHopscotchHashSet.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


