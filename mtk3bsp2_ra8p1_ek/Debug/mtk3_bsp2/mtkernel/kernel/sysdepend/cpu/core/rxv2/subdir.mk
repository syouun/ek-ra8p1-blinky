################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
S_UPPER_SRCS += \
../mtk3_bsp2/mtkernel/kernel/sysdepend/cpu/core/rxv2/dispatch.S \
../mtk3_bsp2/mtkernel/kernel/sysdepend/cpu/core/rxv2/int_asm.S \
../mtk3_bsp2/mtkernel/kernel/sysdepend/cpu/core/rxv2/reset_hdl.S 

C_SRCS += \
../mtk3_bsp2/mtkernel/kernel/sysdepend/cpu/core/rxv2/cpu_cntl.c \
../mtk3_bsp2/mtkernel/kernel/sysdepend/cpu/core/rxv2/exc_hdr.c \
../mtk3_bsp2/mtkernel/kernel/sysdepend/cpu/core/rxv2/interrupt.c \
../mtk3_bsp2/mtkernel/kernel/sysdepend/cpu/core/rxv2/reset_main.c \
../mtk3_bsp2/mtkernel/kernel/sysdepend/cpu/core/rxv2/vector_tbl.c 

C_DEPS += \
./mtk3_bsp2/mtkernel/kernel/sysdepend/cpu/core/rxv2/cpu_cntl.d \
./mtk3_bsp2/mtkernel/kernel/sysdepend/cpu/core/rxv2/exc_hdr.d \
./mtk3_bsp2/mtkernel/kernel/sysdepend/cpu/core/rxv2/interrupt.d \
./mtk3_bsp2/mtkernel/kernel/sysdepend/cpu/core/rxv2/reset_main.d \
./mtk3_bsp2/mtkernel/kernel/sysdepend/cpu/core/rxv2/vector_tbl.d 

OBJS += \
./mtk3_bsp2/mtkernel/kernel/sysdepend/cpu/core/rxv2/cpu_cntl.o \
./mtk3_bsp2/mtkernel/kernel/sysdepend/cpu/core/rxv2/dispatch.o \
./mtk3_bsp2/mtkernel/kernel/sysdepend/cpu/core/rxv2/exc_hdr.o \
./mtk3_bsp2/mtkernel/kernel/sysdepend/cpu/core/rxv2/int_asm.o \
./mtk3_bsp2/mtkernel/kernel/sysdepend/cpu/core/rxv2/interrupt.o \
./mtk3_bsp2/mtkernel/kernel/sysdepend/cpu/core/rxv2/reset_hdl.o \
./mtk3_bsp2/mtkernel/kernel/sysdepend/cpu/core/rxv2/reset_main.o \
./mtk3_bsp2/mtkernel/kernel/sysdepend/cpu/core/rxv2/vector_tbl.o 

SREC += \
mtk3bsp2_ra8p1_ek.srec 

S_UPPER_DEPS += \
./mtk3_bsp2/mtkernel/kernel/sysdepend/cpu/core/rxv2/dispatch.d \
./mtk3_bsp2/mtkernel/kernel/sysdepend/cpu/core/rxv2/int_asm.d \
./mtk3_bsp2/mtkernel/kernel/sysdepend/cpu/core/rxv2/reset_hdl.d 

MAP += \
mtk3bsp2_ra8p1_ek.map 


# Each subdirectory must supply rules for building sources it contributes
mtk3_bsp2/mtkernel/kernel/sysdepend/cpu/core/rxv2/%.o: ../mtk3_bsp2/mtkernel/kernel/sysdepend/cpu/core/rxv2/%.c
	$(file > $@.in,-mthumb -mfloat-abi=hard -mcpu=cortex-m85+nopacbti -O0 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-strict-aliasing -Wall -g -D_RENESAS_RA_ -D_RA_CORE=CPU0 -D_RA_ORDINAL=1 -D_RAFSP_EK_RA8P1_ -I"C:/Users/Makoto/e2_studio/workspace/mtk3bsp2_ra8p1_ek/ra_cfg/fsp_cfg/bsp" -I"." -I"C:/Users/Makoto/e2_studio/workspace/mtk3bsp2_ra8p1_ek/ra_gen" -I"C:/Users/Makoto/e2_studio/workspace/mtk3bsp2_ra8p1_ek/ra_cfg/fsp_cfg" -I"C:/Users/Makoto/e2_studio/workspace/mtk3bsp2_ra8p1_ek/src" -I"C:/Users/Makoto/e2_studio/workspace/mtk3bsp2_ra8p1_ek/ra/fsp/inc" -I"C:/Users/Makoto/e2_studio/workspace/mtk3bsp2_ra8p1_ek/ra/fsp/inc/api" -I"C:/Users/Makoto/e2_studio/workspace/mtk3bsp2_ra8p1_ek/ra/fsp/inc/instances" -I"C:/Users/Makoto/e2_studio/workspace/mtk3bsp2_ra8p1_ek/ra/arm/CMSIS_6/CMSIS/Core/Include" -I"C:/Users/Makoto/e2_studio/workspace/mtk3bsp2_ra8p1_ek/mtk3_bsp2" -I"C:/Users/Makoto/e2_studio/workspace/mtk3bsp2_ra8p1_ek/mtk3_bsp2/config" -I"C:/Users/Makoto/e2_studio/workspace/mtk3bsp2_ra8p1_ek/mtk3_bsp2/include" -I"C:/Users/Makoto/e2_studio/workspace/mtk3bsp2_ra8p1_ek/mtk3_bsp2/mtkernel/kernel/knlinc" -std=c99 -Wno-stringop-overflow -Wno-format-truncation -flax-vector-conversions --param=min-pagesize=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" -x c "$<")
	@echo Building file: $< && arm-none-eabi-gcc @"$@.in"
mtk3_bsp2/mtkernel/kernel/sysdepend/cpu/core/rxv2/%.o: ../mtk3_bsp2/mtkernel/kernel/sysdepend/cpu/core/rxv2/%.S
	$(file > $@.in,-mthumb -mfloat-abi=hard -mcpu=cortex-m85+nopacbti -O0 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-strict-aliasing -Wall -g -x assembler-with-cpp -D_RENESAS_RA_ -D_RA_CORE=CPU0 -D_RA_ORDINAL=1 -D_RAFSP_EK_RA8P1_ -I"C:/Users/Makoto/e2_studio/workspace/mtk3bsp2_ra8p1_ek/ra_cfg/fsp_cfg/bsp" -I"." -I"C:/Users/Makoto/e2_studio/workspace/mtk3bsp2_ra8p1_ek/ra_gen" -I"C:/Users/Makoto/e2_studio/workspace/mtk3bsp2_ra8p1_ek/ra_cfg/fsp_cfg" -I"C:/Users/Makoto/e2_studio/workspace/mtk3bsp2_ra8p1_ek/src" -I"C:/Users/Makoto/e2_studio/workspace/mtk3bsp2_ra8p1_ek/ra/fsp/inc" -I"C:/Users/Makoto/e2_studio/workspace/mtk3bsp2_ra8p1_ek/ra/fsp/inc/api" -I"C:/Users/Makoto/e2_studio/workspace/mtk3bsp2_ra8p1_ek/ra/fsp/inc/instances" -I"C:/Users/Makoto/e2_studio/workspace/mtk3bsp2_ra8p1_ek/ra/arm/CMSIS_6/CMSIS/Core/Include" -I"C:/Users/Makoto/e2_studio/workspace/mtk3bsp2_ra8p1_ek/mtk3_bsp2" -I"C:/Users/Makoto/e2_studio/workspace/mtk3bsp2_ra8p1_ek/mtk3_bsp2/config" -I"C:/Users/Makoto/e2_studio/workspace/mtk3bsp2_ra8p1_ek/mtk3_bsp2/include" -I"C:/Users/Makoto/e2_studio/workspace/mtk3bsp2_ra8p1_ek/mtk3_bsp2/mtkernel/kernel/knlinc" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<")
	@echo Building file: $< && arm-none-eabi-gcc @"$@.in"

