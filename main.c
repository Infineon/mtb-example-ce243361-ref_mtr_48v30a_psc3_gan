/******************************************************************************
* File Name:   main.c
*
* Description: This is the source code for the <ce243361-ref_mtr_48v30a_psc3_gan> Example
*              for ModusToolbox.
*
* Related Document: See README.md
*
*******************************************************************************
* Copyright 2024-2025, Cypress Semiconductor Corporation (an Infineon company) or
* an affiliate of Cypress Semiconductor Corporation.  All rights reserved.
*
* This software, including source code, documentation and related
* materials ("Software") is owned by Cypress Semiconductor Corporation
* or one of its affiliates ("Cypress") and is protected by and subject to
* worldwide patent protection (United States and foreign),
* United States copyright laws and international treaty provisions.
* Therefore, you may use this Software only as provided in the license
* agreement accompanying the software package from which you
* obtained this Software ("EULA").
* If no EULA applies, Cypress hereby grants you a personal, non-exclusive,
* non-transferable license to copy, modify, and compile the Software
* source code solely for use in connection with Cypress's
* integrated circuit products.  Any reproduction, modification, translation,
* compilation, or representation of this Software except as specified
* above is prohibited without the express written permission of Cypress.
*
* Disclaimer: THIS SOFTWARE IS PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, NONINFRINGEMENT, IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. Cypress
* reserves the right to make changes to the Software without notice. Cypress
* does not assume any liability arising out of the application or use of the
* Software or any product or circuit described in the Software. Cypress does
* not authorize its products for use in any products where a malfunction or
* failure of the Cypress product may reasonably be expected to result in
* significant property damage, injury or death ("High Risk Product"). By
* including Cypress's product in a High Risk Product, the manufacturer
* of such system or application assumes all risk of such use and in doing
* so agrees to indemnify Cypress against all liability.
*******************************************************************************/

/*******************************************************************************
* Header Files
*******************************************************************************/
#include "General.h"
#include "HardwareIface.h"
#include "cybsp.h"
#include "Controller.h"
#include "Params.h"
#include "MotorCtrlHWConfig.h"
#include "TLI_5012B.h"
#include "CtrlVars.h"
#include "AppParams.h"

/*******************************************************************************
* Global variable
********************************************************************************/
/* XMC7x - GCC_ARM: EEPROM storage */
#if defined(COMPONENT_CAT1C)
uint8_t Em_Eeprom_Storage[srss_0_eeprom_0_PHYSICAL_SIZE] __attribute__ ((section(".cy_em_eeprom")));
#endif
/*******************************************************************************
* Function Name: main
********************************************************************************
* Summary:
* This is the main function.
*
* Parameters:
*  void
*
* Return:
*  int
*
*******************************************************************************/
int main(void)
{
    cy_rslt_t result;
    
    #if defined(COMPONENT_CAT1C)// Disabled the D-CACHE for XMC7200 device. 
    SCB_DisableDCache();
    #endif
    
    result = cybsp_init();                 /* Initialize the device and board peripherals */
    CY_ASSERT(result == CY_RSLT_SUCCESS);  /* Board init failed. Stop program execution   */
    (void)result;                          /* CY_ASSERT is compiled out in release builds */
   
    // Initialise controller
    HW_IFACE_ConnectFcnPointers();         /* must be called before STATE_MACHINE_Init()  */

    //---------------------------------------------------------
    //--- Initialise the SPI for TLI-5012 sensor interface
    //---------------------------------------------------------
    #if (USING_TLI_5012B)
    uint32_t status = Init_SPI_TLI_5012B();
    if (INIT_FAILURE == status)
    {
        CY_ASSERT(0);
    }

    status = Config_SPI_TLI_5012B_TxDMA();
    if (INIT_FAILURE == status)
    {
        CY_ASSERT(0);
    }

    status = Config_SPI_TLI_5012B_RxDMA();
    if (INIT_FAILURE == status)
    {
        CY_ASSERT(0);
    }
    // PwrUp_Enc1();
    // PwrUp_Enc2();
    
    // Add delay 1sec before initialising state machine
    // to ensure 2 boards can operate at the same time without faults
    // Cy_SysLib_Delay(1000);

    #if 0               //7th FET using PWM
    // Initialize 7th FET and start to output PWM to get a soft start, after a delay, change to 100% fully turn on
    Cy_TCPWM_PWM_Init(PWM_7TH_FET_HW, PWM_7TH_FET_NUM, &PWM_7TH_FET_config);
    Cy_TCPWM_Counter_Enable(PWM_7TH_FET_HW, PWM_7TH_FET_NUM);
    Cy_TCPWM_TriggerStart_Single(PWM_7TH_FET_HW, PWM_7TH_FET_NUM);
    Cy_SysLib_Delay(100);
    uint32_t cc0 = Cy_TCPWM_PWM_GetPeriod0(PWM_7TH_FET_HW, PWM_7TH_FET_NUM); // Get period register and set compare value to 100%
    Cy_TCPWM_PWM_SetCompare0Val(PWM_7TH_FET_HW, PWM_7TH_FET_NUM, cc0+1U);
    #else               //After a delay, then keep 7thFET turning on
    Cy_SysLib_Delay(650);
    // Initialize 7th FET and start to output PWM to get a soft start, after a delay, change to 100% fully turn on
    Cy_TCPWM_PWM_Init(PWM_7TH_FET_HW, PWM_7TH_FET_NUM, &PWM_7TH_FET_config);
    uint32_t cc0 = Cy_TCPWM_PWM_GetPeriod0(PWM_7TH_FET_HW, PWM_7TH_FET_NUM); // Get period register and set compare value to 100%
    Cy_TCPWM_PWM_SetCompare0Val(PWM_7TH_FET_HW, PWM_7TH_FET_NUM, cc0+1U);
    Cy_TCPWM_Counter_Enable(PWM_7TH_FET_HW, PWM_7TH_FET_NUM);                //Start output
    Cy_TCPWM_TriggerStart_Single(PWM_7TH_FET_HW, PWM_7TH_FET_NUM);
    #endif

    STATE_MACHINE_Init();

    #if (OFFSET_CAL_DONE)
    motor[0].params_ptr->sys.fb.mode = Direct; //AqB_Enc;
    // motor[1].params_ptr->sys.fb.mode = Direct; //AqB_Enc;
    
    // motor[0].params_ptr->ctrl.mode   = Speed_Mode_FOC_Encoder_Align_Startup;
    motor[0].params_ptr->ctrl.mode   = Position_Mode_FOC_Encoder_Align_Startup;
    // motor[1].params_ptr->ctrl.mode   = Speed_Mode_FOC_Encoder_Align_Startup;
    #endif
    
    InitPIPLL(&AppParams.TLI_5012B.PLL,    0, TLI_5012B_POS);
    // InitPIPLL(&AppParams.TLI_5012B_M1.PLL, 0, TLI_5012B_POS);
    #endif
    //---------------------------------------------------------

    /* Initialize CAN peripheral */
    #ifdef USING_CAN
    can_fd_init();
    #endif

    //---------------------------------------------------------
    //--- setting for boards in Infineon Mobile Robot (IMR)
    //---------------------------------------------------------
    if (DLM_BRD_POSITION == 0) { // front board
        vars[0].dir =  1;
        // vars[1].dir = -1;
    }
    else if (DLM_BRD_POSITION == 1) { // back board
        vars[0].dir = -1;
        // vars[1].dir =  1;
    }
    //---------------------------------------------------------

#if defined(CTRL_METHOD_RFO)
    // Disable the drive for motors configured in position control mode.
    // MOTOR_CTRL_SetDriveEnable must be called after STATE_MACHINE_Init.
    for (uint8_t i = 0U; i < MOTOR_CTRL_NO_OF_MOTOR; i++)
    {
        if (params[i].ctrl.mode == Position_Mode_FOC_Encoder_Align_Startup)
        {
            MOTOR_CTRL_SetDriveEnable(&motor[i], false);
        }
    }
#endif


    // Enable global interrupts
    __enable_irq();

    //(void) (result);
    for (;;)
    {
        #ifdef USING_CAN
        /* transmit speed & position via CAN regularly */
        manage_extComm_with_CAN();
        #endif

        #if (CPU_LOAD_CALC_ENABLED)
        MCU_CPULoadCalc();
        #endif
        
    }
}
