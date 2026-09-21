################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
S_UPPER_SRCS += \
../mtk3_bsp2/mtkernel/kernel/sysdepend/cpu/core/armv7a/dispatch.S \
../mtk3_bsp2/mtkernel/kernel/sysdepend/cpu/core/armv7a/exc_entry.S \
../mtk3_bsp2/mtkernel/kernel/sysdepend/cpu/core/armv7a/int_asm.S \
../mtk3_bsp2/mtkernel/kernel/sysdepend/cpu/core/armv7a/reset_hdl.S \
../mtk3_bsp2/mtkernel/kernel/sysdepend/cpu/core/armv7a/vector_tbl.S 

C_SRCS += \
../mtk3_bsp2/mtkernel/kernel/sysdepend/cpu/core/armv7a/cpu_cntl.c \
../mtk3_bsp2/mtkernel/kernel/sysdepend/cpu/core/armv7a/exc_hdl.c \
../mtk3_bsp2/mtkernel/kernel/sysdepend/cpu/core/armv7a/interrupt.c \
../mtk3_bsp2/mtkernel/kernel/sysdepend/cpu/core/armv7a/reset_main.c 

C_DEPS += \
./mtk3_bsp2/mtkernel/kernel/sysdepend/cpu/core/armv7a/cpu_cntl.d \
./mtk3_bsp2/mtkernel/kernel/sysdepend/cpu/core/armv7a/exc_hdl.d \
./mtk3_bsp2/mtkernel/kernel/sysdepend/cpu/core/armv7a/interrupt.d \
./mtk3_bsp2/mtkernel/kernel/sysdepend/cpu/core/armv7a/reset_main.d 

OBJS += \
./mtk3_bsp2/mtkernel/kernel/sysdepend/cpu/core/armv7a/cpu_cntl.o \
./mtk3_bsp2/mtkernel/kernel/sysdepend/cpu/core/armv7a/dispatch.o \
./mtk3_bsp2/mtkernel/kernel/sysdepend/cpu/core/armv7a/exc_entry.o \
./mtk3_bsp2/mtkernel/kernel/sysdepend/cpu/core/armv7a/exc_hdl.o \
./mtk3_bsp2/mtkernel/kernel/sysdepend/cpu/core/armv7a/int_asm.o \
./mtk3_bsp2/mtkernel/kernel/sysdepend/cpu/core/armv7a/interrupt.o \
./mtk3_bsp2/mtkernel/kernel/sysdepend/cpu/core/armv7a/reset_hdl.o \
./mtk3_bsp2/mtkernel/kernel/sysdepend/cpu/core/armv7a/reset_main.o \
./mtk3_bsp2/mtkernel/kernel/sysdepend/cpu/core/armv7a/vector_tbl.o 

SREC += \
mtk3bsp2_ra8p1_ek.srec 

S_UPPER_DEPS += \
./mtk3_bsp2/mtkernel/kernel/sysdepend/cpu/core/armv7a/dispatch.d \
./mtk3_bsp2/mtkernel/kernel/sysdepend/cpu/core/armv7a/exc_entry.d \
./mtk3_bsp2/mtkernel/kernel/sysdepend/cpu/core/armv7a/int_asm.d \
./mtk3_bsp2/mtkernel/kernel/sysdepend/cpu/core/armv7a/reset_hdl.d \
./mtk3_bsp2/mtkernel/kernel/sysdepend/cpu/core/armv7a/vector_tbl.d 

MAP += \
mtk3bsp2_ra8p1_ek.map 


# Each subdirectory must supply rules for building sources it contributes
mtk3_bsp2/mtkernel/kernel/sysdepend/cpu/core/armv7a/%.o: ../mtk3_bsp2/mtkernel/kernel/sysdepend/cpu/core/armv7a/%.c
	$(file > $@.in,-mthumb -mfloat-abi=hard -mcpu=cortex-m85+nopacbti -O0 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-strict-aliasing -Wall -g -D_RENESAS_RA_ -D_RA_CORE=CPU0 -D_RA_ORDINAL=1 -D_RAFSP_EK_RA8P1_ -I"C:/Users/Makoto/e2_studio/workspace/mtk3bsp2_ra8p1_ek/ra_cfg/fsp_cfg/bsp" -I"." -I"C:/Users/Makoto/e2_studio/workspace/mtk3bsp2_ra8p1_ek/ra_gen" -I"C:/Users/Makoto/e2_studio/workspace/mtk3bsp2_ra8p1_ek/ra_cfg/fsp_cfg" -I"C:/Users/Makoto/e2_studio/workspace/mtk3bsp2_ra8p1_ek/src" -I"C:/Users/Makoto/e2_studio/workspace/mtk3bsp2_ra8p1_ek/ra/fsp/inc" -I"C:/Users/Makoto/e2_studio/workspace/mtk3bsp2_ra8p1_ek/ra/fsp/inc/api" -I"C:/Users/Makoto/e2_studio/workspace/mtk3bsp2_ra8p1_ek/ra/fsp/inc/instances" -I"C:/Users/Makoto/e2_studio/workspace/mtk3bsp2_ra8p1_ek/ra/arm/CMSIS_6/CMSIS/Core/Include" -I"C:/Users/Makoto/e2_studio/workspace/mtk3bsp2_ra8p1_ek/mtk3_bsp2" -I"C:/Users/Makoto/e2_studio/workspace/mtk3bsp2_ra8p1_ek/mtk3_bsp2/config" -I"C:/Users/Makoto/e2_studio/workspace/mtk3bsp2_ra8p1_ek/mtk3_bsp2/include" -I"C:/Users/Makoto/e2_studio/workspace/mtk3bsp2_ra8p1_ek/mtk3_bsp2/mtkernel/kernel/knlinc" -std=c99 -Wno-stringop-overflow -Wno-format-truncation -flax-vector-conversions --param=min-pagesize=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" -x c "$<")
	@echo Building file: $< && arm-none-eabi-gcc @"$@.in"
mtk3_bsp2/mtkernel/kernel/sysdepend/cpu/core/armv7a/%.o: ../mtk3_bsp2/mtkernel/kernel/sysdepend/cpu/core/armv7a/%.S
	$(file > $@.in,-mthumb -mfloat-abi=hard -mcpu=cortex-m85+nopacbti -O0 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-strict-aliasing -Wall -g -x assembler-with-cpp -D_RENESAS_RA_ -D_RA_CORE=CPU0 -D_RA_ORDINAL=1 -D_RAFSP_EK_RA8P1_ -I"C:/Users/Makoto/e2_studio/workspace/mtk3bsp2_ra8p1_ek/ra_cfg/fsp_cfg/bsp" -I"." -I"C:/Users/Makoto/e2_studio/workspace/mtk3bsp2_ra8p1_ek/ra_gen" -I"C:/Users/Makoto/e2_studio/workspace/mtk3bsp2_ra8p1_ek/ra_cfg/fsp_cfg" -I"C:/Users/Makoto/e2_studio/workspace/mtk3bsp2_ra8p1_ek/src" -I"C:/Users/Makoto/e2_studio/workspace/mtk3bsp2_ra8p1_ek/ra/fsp/inc" -I"C:/Users/Makoto/e2_studio/workspace/mtk3bsp2_ra8p1_ek/ra/fsp/inc/api" -I"C:/Users/Makoto/e2_studio/workspace/mtk3bsp2_ra8p1_ek/ra/fsp/inc/instances" -I"C:/Users/Makoto/e2_studio/workspace/mtk3bsp2_ra8p1_ek/ra/arm/CMSIS_6/CMSIS/Core/Include" -I"C:/Users/Makoto/e2_studio/workspace/mtk3bsp2_ra8p1_ek/mtk3_bsp2" -I"C:/Users/Makoto/e2_studio/workspace/mtk3bsp2_ra8p1_ek/mtk3_bsp2/config" -I"C:/Users/Makoto/e2_studio/workspace/mtk3bsp2_ra8p1_ek/mtk3_bsp2/include" -I"C:/Users/Makoto/e2_studio/workspace/mtk3bsp2_ra8p1_ek/mtk3_bsp2/mtkernel/kernel/knlinc" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<")
	@echo Building file: $< && arm-none-eabi-gcc @"$@.in"

