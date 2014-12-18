################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/BranchResolver.cpp \
../src/CSE240AProj.cpp \
../src/InstDecode.cpp \
../src/InstFetch.cpp \
../src/InstPipe.cpp \
../src/InstSched.cpp \
../src/ROB.cpp \
../src/TraceOutputLogger.cpp \
../src/instructions.cpp \
../src/userinterface.cpp \
../src/utils.cpp 

OBJS += \
./src/BranchResolver.o \
./src/CSE240AProj.o \
./src/InstDecode.o \
./src/InstFetch.o \
./src/InstPipe.o \
./src/InstSched.o \
./src/ROB.o \
./src/TraceOutputLogger.o \
./src/instructions.o \
./src/userinterface.o \
./src/utils.o 

CPP_DEPS += \
./src/BranchResolver.d \
./src/CSE240AProj.d \
./src/InstDecode.d \
./src/InstFetch.d \
./src/InstPipe.d \
./src/InstSched.d \
./src/ROB.d \
./src/TraceOutputLogger.d \
./src/instructions.d \
./src/userinterface.d \
./src/utils.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -DDEBUG -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


