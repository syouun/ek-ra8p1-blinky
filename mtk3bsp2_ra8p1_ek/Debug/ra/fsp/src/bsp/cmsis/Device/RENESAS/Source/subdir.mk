################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../ra/fsp/src/bsp/cmsis/Device/RENESAS/Source/startup.c \
../ra/fsp/src/bsp/cmsis/Device/RENESAS/Source/system.c 

C_DEPS += \
./ra/fsp/src/bsp/cmsis/Device/RENESAS/Source/startup.d \
./ra/fsp/src/bsp/cmsis/Device/RENESAS/Source/system.d 

OBJS += \
./ra/fsp/src/bsp/cmsis/Device/RENESAS/Source/startup.o \
./ra/fsp/src/bsp/cmsis/Device/RENESAS/Source/system.o 

SREC += \
mtk3bsp2_ra8p1_ek.srec 

MAP += \
mtk3bsp2_ra8p1_ek.map 


# Each subdirectory must supply rules for building sources it contributes
ra/fsp/src/bsp/cmsis/Device/RENESAS/Source/%.o: ../ra/fsp/src/bsp/cmsis/Device/RENESAS/Source/%.c
	$(file > $@.in,-mthumb -mfloat-abi=hard -mcpu=cortex-m85+nopacbti -O0 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-strict-aliasing -Wall -g -D_RENESAS_RA_ -D_RA_CORE=CPU0 -D_RA_ORDINAL=1 -D_RAFSP_EK_RA8P1_ -I"C:/Users/Makoto/e2_studio/workspace/mtk3bsp2_ra8p1_ek/ra_cfg/fsp_cfg/bsp" -I"." -I"C:/Users/Makoto/e2_studio/workspace/mtk3bsp2_ra8p1_ek/ra_gen" -I"C:/Users/Makoto/e2_studio/workspace/mtk3bsp2_ra8p1_ek/ra_cfg/fsp_cfg" -I"C:/Users/Makoto/e2_studio/workspace/mtk3bsp2_ra8p1_ek/src" -I"C:/Users/Makoto/e2_studio/workspace/mtk3bsp2_ra8p1_ek/ra/fsp/inc" -I"C:/Users/Makoto/e2_studio/workspace/mtk3bsp2_ra8p1_ek/ra/fsp/inc/api" -I"C:/Users/Makoto/e2_studio/workspace/mtk3bsp2_ra8p1_ek/ra/fsp/inc/instances" -I"C:/Users/Makoto/e2_studio/workspace/mtk3bsp2_ra8p1_ek/ra/arm/CMSIS_6/CMSIS/Core/Include" -I"C:/Users/Makoto/e2_studio/workspace/mtk3bsp2_ra8p1_ek/mtk3_bsp2" -I"C:/Users/Makoto/e2_studio/workspace/mtk3bsp2_ra8p1_ek/mtk3_bsp2/config" -I"C:/Users/Makoto/e2_studio/workspace/mtk3bsp2_ra8p1_ek/mtk3_bsp2/include" -I"C:/Users/Makoto/e2_studio/workspace/mtk3bsp2_ra8p1_ek/mtk3_bsp2/mtkernel/kernel/knlinc" -std=c99 -Wno-stringop-overflow -Wno-format-truncation -flax-vector-conversions --param=min-pagesize=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" -x c "$<")
	@echo Building file: $< && arm-none-eabi-gcc @"$@.in"

