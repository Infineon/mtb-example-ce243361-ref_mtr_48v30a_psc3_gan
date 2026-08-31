/*******************************************************************************
* Copyright 2021-2024, Cypress Semiconductor Corporation (an Infineon company) or
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


#include "HardwareIface.h"


#define    MOTOR_CTRL_NO_OF_MOTOR_BITPOSITION               ((1U<<MOTOR_CTRL_NO_OF_MOTOR) -1)
/*******************************************************************************/
/*Hardware initialization  function are all the configured motor are called together*/
void HW_IFACE_Init(uint8_t motor_id)
{

  if(motor_id == MOTOR_CTRL_ID_MOTOR0)  /*Motor 0*/
  {
      MCU_Init();
  }
#if (MOTOR_CTRL_MOTOR1_ENABLED)
  if(motor_id == MOTOR_CTRL_ID_MOTOR1) /*Motor 1*/
  {
      MCU_Init_M1();
  }
#endif
}

/*******************************************************************************/
/*Start  Peripherals function are all the configured motor are called together*/
void HW_IFACE_StartPeripherals(uint8_t motor_id)
{

  static uint8_t motor_ctrl_hw_start =0;
  motor_ctrl_hw_start |= 1<< motor_id;
  if(motor_ctrl_hw_start == MOTOR_CTRL_NO_OF_MOTOR_BITPOSITION)  
  {
      MCU_StartPeripherals();
#if (MOTOR_CTRL_MOTOR1_ENABLED)
      /*Cy_SysLib_DelayUs(25);*/
      MCU_StartPeripherals_M1();
#endif
    motor_ctrl_hw_start=0;
   }    
}

/*******************************************************************************/

void HW_IFACE_StopPeripherals(uint8_t motor_id)
{
  if(motor_id == MOTOR_CTRL_ID_MOTOR0)  /*Motor 0*/
  {
      MCU_StopPeripherals();
  }
#if (MOTOR_CTRL_MOTOR1_ENABLED)
  if(motor_id == MOTOR_CTRL_ID_MOTOR1) /*Motor 1*/
  {
      MCU_StopPeripherals_M1();
  }
#endif
}

/*******************************************************************************/

void HW_IFACE_GateDriverEnterHighZ(uint8_t motor_id)
{
  uint8_t phase_control = (motor_id>>5)&0x7;  //Special handling to control  driver per phase, Bit 7 - Phase W, Bit 6 - Phase V, Bit 5 - Phase U
  uint8_t motor_inst    =  motor_id &0x1F;
   
  if ((phase_control == 0) ||(phase_control == 0x7) ) //Control all the 3 phases 
  {
    if(motor_inst == MOTOR_CTRL_ID_MOTOR0)  /*Motor 0*/
    {
      MCU_GateDriverEnterHighZ();
    }
#if (MOTOR_CTRL_MOTOR1_ENABLED)
    if(motor_inst == MOTOR_CTRL_ID_MOTOR1) /*Motor 1*/
    {
      MCU_GateDriverEnterHighZ_M1();
    }
#endif
  }
  else 
  {
    if(motor_inst == MOTOR_CTRL_ID_MOTOR0)  /*Motor 0*/
    {
        if(phase_control&0x1){MCU_PhaseUEnterHighZ(); }  //Phase U control enabled
        if(phase_control&0x2){MCU_PhaseVEnterHighZ(); }  //Phase V control enabled
        if(phase_control&0x4){MCU_PhaseWEnterHighZ(); }  //Phase W control enabled
    }
#if (MOTOR_CTRL_MOTOR1_ENABLED)
    if(motor_inst == MOTOR_CTRL_ID_MOTOR1) /*Motor 1*/
    {
        if(phase_control&0x1){MCU_PhaseUEnterHighZ_M1(); }  //Phase U control enabled
        if(phase_control&0x2){MCU_PhaseVEnterHighZ_M1(); }  //Phase V control enabled
        if(phase_control&0x4){MCU_PhaseWEnterHighZ_M1(); }  //Phase W control enabled
    }
#endif            
  }
  
}

/*******************************************************************************/

void HW_IFACE_GateDriverExitHighZ(uint8_t motor_id)
{
  uint8_t phase_control = (motor_id>>5)&0x7;  //Special handling to control gate driver per phase separately, Bit 7 - Phase W, Bit 6 - Phase V, Bit 5 - Phase U
  uint8_t motor_inst    =  motor_id &0x1F;
       
  if ((phase_control == 0) ||(phase_control == 0x7) ) //Control all the 3 phases 
  {
    if(motor_inst == MOTOR_CTRL_ID_MOTOR0)  /*Motor 0*/
    {
      MCU_GateDriverExitHighZ();
    }
#if (MOTOR_CTRL_MOTOR1_ENABLED)
    if(motor_inst == MOTOR_CTRL_ID_MOTOR1) /*Motor 1*/
    {
      MCU_GateDriverExitHighZ_M1();
    }
#endif
  }
  else 
  {
    if(motor_inst == MOTOR_CTRL_ID_MOTOR0)  /*Motor 0*/
    {
        if(phase_control&0x1){MCU_PhaseUExitHighZ(); }  //Phase U control enabled
        if(phase_control&0x2){MCU_PhaseVExitHighZ(); }  //Phase V control enabled
        if(phase_control&0x4){MCU_PhaseWExitHighZ(); }  //Phase W control enabled
    }
#if (MOTOR_CTRL_MOTOR1_ENABLED)
    if(motor_inst == MOTOR_CTRL_ID_MOTOR1) /*Motor 1*/
    {
        if(phase_control&0x1){MCU_PhaseUExitHighZ_M1(); }  //Phase U control enabled
        if(phase_control&0x2){MCU_PhaseVExitHighZ_M1(); }  //Phase V control enabled
        if(phase_control&0x4){MCU_PhaseWExitHighZ_M1(); }  //Phase W control enabled
    }
#endif            
  }
}

/*******************************************************************************/
// Connecting HW-agnostic function pointers to their corresponding HW-dependent functions
void HW_IFACE_ConnectFcnPointers(void)
{

    hw_fcn.HardwareIfaceInit        = HW_IFACE_Init;   /* used in FCN, state machine init*/
    hw_fcn.EnterCriticalSection     = MCU_EnterCriticalSection;
    hw_fcn.ExitCriticalSection      = MCU_ExitCriticalSection;
    hw_fcn.GateDriverEnterHighZ     = HW_IFACE_GateDriverEnterHighZ; /*Used in Profiler, State Machine*/
    hw_fcn.GateDriverExitHighZ      = HW_IFACE_GateDriverExitHighZ;
    hw_fcn.StartPeripherals         = HW_IFACE_StartPeripherals;
    hw_fcn.StopPeripherals          = HW_IFACE_StopPeripherals;
    hw_fcn.FlashRead                = MCU_FlashRead;
    hw_fcn.FlashWrite               = MCU_FlashWrite;
    hw_fcn.ArePhaseVoltagesMeasured = MCU_ArePhaseVoltagesMeasured;

}
