/******************************************************************************
* File Name: DCChopper.c
*
* Description: DC Chopper to use PWM to suppress the DC bus voltage overshoot
*
* Related Document: See README.md
*
*
*******************************************************************************/
/*******************************************************************************
* Copyright 2025, Cypress Semiconductor Corporation (an Infineon company) or
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

#include "DCChopper.h"
#include "ParamConfig.h"


void DCChopper_Braking(MOTOR_t* motor_ptr);

void DCChopper_Braking(MOTOR_t* motor_ptr)          // Control the DC Chopper braking PWM in fast loop, to control the VDC voltage
{
    DC_CHOPPER_t* dcchopper_ptr = &AppParams.DCChopper;
    STATE_MACHINE_t* sm_ptr = motor_ptr->sm_ptr;
    SENSOR_IFACE_t*  sensor_iface_ptr = motor_ptr->sensor_iface_ptr;

    if((sm_ptr->current == Init) || (sm_ptr->current == Brake_Boot) || (sm_ptr->current == Fault))    //Turn off brake in below cases
    {
        Cy_GPIO_Write(BRAKE_PWM_PORT, BRAKE_PWM_PIN, 0UL);              // Turn off the brake
        dcchopper_ptr->Ontimetotal = 0;
    }
    else
    {
        if(dcchopper_ptr->BrakeUsageRate == 100)                                       //100%
        {
            dcchopper_ptr->OnCnt = 65535;
            dcchopper_ptr->OffCnt = 0;
        }
        else if(dcchopper_ptr->BrakeUsageRate == 0)                                    //0%
        {
            dcchopper_ptr->OnCnt = 0;
            dcchopper_ptr->OffCnt = 65535;
        }
        else if(dcchopper_ptr->BrakeUsageRate <= 50)                                   // BrakeUsageRate <= 50%
        {
            dcchopper_ptr->OnCnt = 1;
            dcchopper_ptr->OffCnt = (100 - dcchopper_ptr->BrakeUsageRate) / dcchopper_ptr->BrakeUsageRate; // Inaccurate, but simple
        }
        else                                                            // 50% < BrakeUsageRate < 100%
        {
            dcchopper_ptr->OnCnt = dcchopper_ptr->BrakeUsageRate / (100 - dcchopper_ptr->BrakeUsageRate);
            dcchopper_ptr->OffCnt = 1;
        }
        
        if(dcchopper_ptr->OnCnt == 0)
        {
            Cy_GPIO_Write(BRAKE_PWM_PORT, BRAKE_PWM_PIN, 0UL);              // Turn off the brake
            dcchopper_ptr->Ontimetotal = 0;
            return;
        }
       
        // Judge whether it should be turned on or off this time
        if(sensor_iface_ptr->v_dc.raw < dcchopper_ptr->vdc_chopper_lower_thresh)
        {               
            dcchopper_ptr->BrakeHasStarted = false;                     //Clear
            dcchopper_ptr->TurnOnBrake = false;              
            dcchopper_ptr->Cnt = 0;              
        }               
        else if(!dcchopper_ptr->BrakeHasStarted)                         //The first time to turn on, set the flags
        {               
            if(sensor_iface_ptr->v_dc.raw > dcchopper_ptr->vdc_chopper_upper_thresh)                    
            {               
                dcchopper_ptr->BrakeHasStarted = true;               
                dcchopper_ptr->TurnOnBrake = true;               
                dcchopper_ptr->Cnt = 0;              
            }               
        }               
        else                                                        // Brake has started
        {               
            dcchopper_ptr->Cnt++;                
            if(!dcchopper_ptr->TurnOnBrake)                              // Turn off
            {               
                if(dcchopper_ptr->Cnt > dcchopper_ptr->OffCnt)                
                {               
                    dcchopper_ptr->TurnOnBrake = true;               
                    dcchopper_ptr->Cnt = 0;                              // Change to accumulate the ON time              
                }               
            }               
            else                                                    // Turn on
            {
                if(dcchopper_ptr->Cnt > dcchopper_ptr->OnCnt)
                {
                    dcchopper_ptr->TurnOnBrake = false;
                    dcchopper_ptr->Cnt = 0;                              // Change to accumulate the off time
                }
            }
        }

        // Execute the turn on or off action
        if(dcchopper_ptr->TurnOnBrake)
        {
            Cy_GPIO_Write(BRAKE_PWM_PORT, BRAKE_PWM_PIN, 1UL);      // Turn on the brake    
        }
        else
        {
            Cy_GPIO_Write(BRAKE_PWM_PORT, BRAKE_PWM_PIN, 0UL);      // Turn off the brake       
            dcchopper_ptr->Ontimetotal = 0;                         // Clear the accumulated continous on time
        }
    }
}



