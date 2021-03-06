/*
 * Initialization.c
 *
 *  Created on: May 26, 2018
 *      Author: markos
 */

#include "DSP28x_Project.h"     // Device Headerfile and Examples Include File
#include "Initialization.h"
#include "Communication.h"
#include "common/include/adc.h"
#include "common/include/clk.h"
#include "common/include/flash.h"
#include "common/include/gpio.h"
#include "common/include/pie.h"
#include "common/include/pll.h"
#include "common/include/pwm.h"
#include "common/include/wdog.h"
#include "common/include/sci.h"



void initialize(){
    //
        // Initialize all the handles needed for this application
        //
        myAdc = ADC_init((void *)ADC_BASE_ADDR, sizeof(ADC_Obj));
        myClk = CLK_init((void *)CLK_BASE_ADDR, sizeof(CLK_Obj));
        myCpu = CPU_init((void *)NULL, sizeof(CPU_Obj));
        myFlash = FLASH_init((void *)FLASH_BASE_ADDR, sizeof(FLASH_Obj));
        myGpio = GPIO_init((void *)GPIO_BASE_ADDR, sizeof(GPIO_Obj));
        myPie = PIE_init((void *)PIE_BASE_ADDR, sizeof(PIE_Obj));
        myPll = PLL_init((void *)PLL_BASE_ADDR, sizeof(PLL_Obj));
        myPwm = PWM_init((void *)PWM_ePWM1_BASE_ADDR, sizeof(PWM_Obj));
        myWDog = WDOG_init((void *)WDOG_BASE_ADDR, sizeof(WDOG_Obj));
        mySci = SCI_init((void *)SCIA_BASE_ADDR, sizeof(SCI_Obj));
        //
        // Perform basic system initialization
        //
        WDOG_disable(myWDog);
        CLK_enableAdcClock(myClk);
        (*Device_cal)();
        //
        // Select the internal oscillator 1 as the clock source
        //
        CLK_setOscSrc(myClk, CLK_OscSrc_Internal);
        //
        // Setup the PLL for x10 /2 which will yield 50Mhz = 10Mhz * 10 / 2
        //
        PLL_setup(myPll, PLL_Multiplier_12, PLL_DivideSelect_ClkIn_by_2);
        //
        // Disable the PIE and all interrupts
        //
        PIE_disable(myPie);
        PIE_disableAllInts(myPie);
        CPU_disableGlobalInts(myCpu);
        CPU_clearIntFlags(myCpu);
        //
        // If running from flash copy RAM only functions to RAM
        //
    #ifdef _FLASH
        memcpy(&RamfuncsRunStart, &RamfuncsLoadStart, (size_t)&RamfuncsLoadSize);
    #endif
        //
        // Setup a debug vector table and enable the PIE
        //
        PIE_setDebugIntVectorTable(myPie);
        PIE_enable(myPie);

        //
        // Register interrupt handlers in the PIE vector table
        //
        PIE_registerPieIntHandler(myPie, PIE_GroupNumber_10, PIE_SubGroupNumber_1,
                                    (intVec_t)&adc_isr);

        //
        // Initialize the ADC
        //
        ADC_enableBandGap(myAdc);
        ADC_enableRefBuffers(myAdc);
        ADC_powerUp(myAdc);
        ADC_enable(myAdc);
        ADC_setVoltRefSrc(myAdc, ADC_VoltageRefSrc_Int);

        //
        // Enable ADCINT1 in PIE
        //
        PIE_enableAdcInt(myPie, ADC_IntNumber_1);
        //
        // Enable CPU Interrupt 1
        //
        CPU_enableInt(myCpu, CPU_IntNumber_10);
        //
        // Enable Global interrupt INTM
        //
        CPU_enableGlobalInts(myCpu);
        //
        // Enable Global realtime interrupt DBGM
        //
        CPU_enableDebugInt(myCpu);
        //
        // Configure ADC
        //
        //
        // Note: Channel ADCINA4  will be double sampled to workaround the ADC 1st
        // sample issue for rev0 silicon errata
        //
        //
        // ADCINT1 trips after AdcResults latch
        //
        ADC_setIntPulseGenMode(myAdc, ADC_IntPulseGenMode_Prior);
        ADC_enableInt(myAdc, ADC_IntNumber_1);              // Enabled ADCINT1
        //
        // Disable ADCINT1 Continuous mode
        //
        ADC_setIntMode(myAdc, ADC_IntNumber_1, ADC_IntMode_EOC);
        //
        // setup EOC2 to trigger ADCINT1 to fire
        //
        ADC_setIntSrc(myAdc, ADC_IntNumber_1, ADC_IntSrc_EOC3);
        //
        // set SOC0 channel select to ADCINA4
        //
        ADC_setSocChanNumber (myAdc, ADC_SocNumber_0, ADC_SocChanNumber_A1);
        ADC_setSocChanNumber (myAdc, ADC_SocNumber_1, ADC_SocChanNumber_B1);
        ADC_setSocChanNumber (myAdc, ADC_SocNumber_2, ADC_SocChanNumber_A2);
        ADC_setSocChanNumber (myAdc, ADC_SocNumber_3, ADC_SocChanNumber_B2);
        // set SOC0 start trigger on EPWM1A, due to round-robin SOC0 converts first
        // then SOC1
        ADC_setSocTrigSrc(myAdc, ADC_SocNumber_0, ADC_SocTrigSrc_EPWM1_ADCSOCA);
        ADC_setSocTrigSrc(myAdc, ADC_SocNumber_2, ADC_SocTrigSrc_EPWM1_ADCSOCA);
        // set SOC0 S/H Window to 14 ADC Clock Cycles, (13 ACQPS plus 1)
        //
        ADC_setSocSampleWindow(myAdc, ADC_SocNumber_0,
                               ADC_SocSampleWindow_14_cycles);
        ADC_setSocSampleWindow(myAdc, ADC_SocNumber_1,
                               ADC_SocSampleWindow_14_cycles);
        ADC_setSocSampleWindow(myAdc, ADC_SocNumber_2,
                               ADC_SocSampleWindow_14_cycles);
        ADC_setSocSampleWindow(myAdc, ADC_SocNumber_3,
                               ADC_SocSampleWindow_14_cycles);
        ADC_setSampleMode(myAdc, ADC_SampleMode_SOC0_and_SOC1_Together);
        ADC_setSampleMode(myAdc, ADC_SampleMode_SOC2_and_SOC3_Together);
        //
        // Enable PWM clock
        //
        CLK_enablePwmClock(myClk, PWM_Number_1);
        //
        // Setup PWM
        //
        PWM_enableSocAPulse(myPwm);                     // Enable SOC on A group
        //
        // Select SOC from from CPMA on upcount
        //
        PWM_setSocAPulseSrc(myPwm, PWM_SocPulseSrc_CounterEqualCmpAIncr);
        //
        // Generate pulse on 1st event
        //
        PWM_setSocAPeriod(myPwm, PWM_SocPeriod_FirstEvent);
        PWM_setCmpA(myPwm, 0x00FF);                      // Set compare A value
        PWM_setPeriod(myPwm, 0x1770);                    // Set period for ePWM1
        PWM_setCounterMode(myPwm, PWM_CounterMode_Up);   // count up and start
        CLK_enableTbClockSync(myClk);
        //setup gpio for TX RX
        GPIO_setPullUp(myGpio, GPIO_Number_28, GPIO_PullUp_Enable);
        GPIO_setPullUp(myGpio, GPIO_Number_29, GPIO_PullUp_Disable);
        GPIO_setQualification(myGpio, GPIO_Number_28, GPIO_Qual_ASync);
        GPIO_setMode(myGpio, GPIO_Number_28, GPIO_28_Mode_SCIRXDA);
        GPIO_setMode(myGpio, GPIO_Number_29, GPIO_29_Mode_SCITXDA);
        GPIO_setMode(myGpio, GPIO_Number_34, GPIO_34_Mode_GeneralPurpose);
        GPIO_setDirection(myGpio,GPIO_Number_34,GPIO_Direction_Output);
        GPIO_setLow(myGpio, GPIO_Number_34);

        GPIO_setMode(myGpio, GPIO_Number_16, GPIO_16_Mode_GeneralPurpose);
        GPIO_setDirection(myGpio,GPIO_Number_16,GPIO_Direction_Output);
        GPIO_setLow(myGpio, GPIO_Number_16);

        scia_echoback_init();           // Initialize SCI for echoback
        scia_fifo_init();               // Initialize the SCI FIFO
        return;
}
