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

#pragma once

#include "stdbool.h"
#include "stdint.h"
#include "ParamConfig.h"
#include "General.h"

#define q_t                 int32_t

#define Qxx                 15
#define Qxx_MAX             ((q_t)(1 << Qxx))

#define IFX_Q_mul(x, y)     ((q_t)(((x)*(y)) >> Qxx))
#define IFX_Q(x)            ((q_t)(x*Qxx_MAX))

#define Fs_Hz                   (MOTOR_CTRL_FASTLOOP_FREQ)                          //15000.0f  // [Hz] Sampling frequency, modified by leonard， MOTOR_CTRL_FASTLOOP_FREQ
#define Ts                      (1.0f / Fs_Hz)                      // [s]  Sampling period
#define Fc_PLL                  250.0f
#define Fz_PLL                  25.0f
#define THETA_BASE              PI
#define RPM_BASE                (MOTOR_MAX_SPEED)                           // [rpm] base speed mechanical


#define ONE_OVER_THETA_BASE     (1.0f / THETA_BASE)
#define OMEGA_RE_BASE           ((RPM_BASE*MOTOR_POLE*TWO_PI)/120.0f)   // [rad/s] base angular speed electrical
#define ONE_OVER_OMEGA_RE_BASE  (1.0f / OMEGA_RE_BASE)                  // [1/rad/s]

#define PU_TO_FLOAT(pu, base, qxx_max)  (((float)((q_t)pu))*(base/(float)qxx_max))

typedef struct
{
    float current_u_sensitivity;        // [V/A] used for U phase current sensor type as "Active_Sensor"
    float current_v_sensitivity;        // [V/A] used for V phase current sensor type as "Active_Sensor"
    float current_w_sensitivity;        // [V/A] used for W phase current sensor type as "Active_Sensor"
    float current_dc_sensitivity;       // [V/A] used for DC phase current sensor type as "Active_Sensor"  
} ANALOG_CS_PARAMS_t;

typedef struct {
    q_t Kp;
    q_t Ki;
    q_t k_Omega2dTheta;
    q_t omega1;
    q_t omega2;
    q_t omega;

    uint16_t ThetaU16;
    float Theta_flt;
    float Omega_flt;
} PIPLL_t;

typedef struct {
    uint16_t Theta_MechRaw_U16;         // mechanical angle value
    float    Theta_MechRaw;             // -pi ~ pi, mechanical angle value calculated from read out data directly
    uint16_t Theta_TLI_5012B_U16;       //
    int16_t  TLI5012B_Offset_S16;       // [cnt] TLI-5012 Position Sensor Offset: -32768 ~ 32767[cnt] corresponding to -180.0 ~ 180.0[deg]
    int16_t  Dir_TLI_5012B;             //
    float    Theta_TLI_5012B_flt;       // -pi ~ pi, elec angle value after offset calibration

    uint16_t PLLSmplCnt;
    uint16_t MovingInitialized;
    PIPLL_t PLL;
} TLI_5012B_ABS_POS_t;

typedef struct
{
    uint16_t Count;
   
    bool BrakeHasStarted;                       // 0: Indicate braking has not started yet. Set to 1 after the first step.
    bool TurnOnBrake;                           // 1: turn on brake. 0: turn off brake

    uint16_t    Cnt;                            // Current times of turned on
    uint16_t    OnCnt;                          // Continous allowable times of turn on
    uint16_t    OffCnt;                         // Continuous allowable times of turn off
    uint16_t    Ontimetotal;                    // Continuous on time, if it exceeds BrakeAllowedOnTime, reports an error
    uint16_t    BrakeUsageRate;                 // 0 ~ 100: 0% ~ 100%. Todo, use macro to set
    uint16_t    BrakeAllowedOnTime;             // Allowed turn on time continuously. Unit: s
    float       vdc_chopper_upper_thresh;       // Upper vdc threshold for starting chopper
    float       vdc_chopper_lower_thresh;       // Lower vdc threshold for stopping chopper
} DC_CHOPPER_t;

typedef struct
{
    ANALOG_CS_PARAMS_t cs;                      // TMR current sensor parameters
    TLI_5012B_ABS_POS_t TLI_5012B;              // TLI5012 angle sensor parameters (motor0)
    TLI_5012B_ABS_POS_t TLI_5012B_M1;           // TLI5012 angle sensor parameters (motor1)
    DC_CHOPPER_t DCChopper;                     // DC chopper parameters
    uint16_t    Cal_Speed_Method;               // Choose the actual speed calculation method
} APPPARAMS_t;



extern APPPARAMS_t AppParams;