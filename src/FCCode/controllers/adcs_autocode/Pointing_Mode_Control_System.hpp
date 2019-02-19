/*
 * Academic License - for use in teaching, academic research, and meeting
 * course requirements at degree granting institutions only.  Not for
 * government, commercial, or other organizational use.
 *
 * File: Pointing_Mode_Control_System.h
 *
 * Code generated for Simulink model 'Pointing_Mode_Control_System'.
 *
 * Model version                  : 1.302
 * Simulink Coder version         : 8.14 (R2018a) 06-Feb-2018
 * C/C++ source code generated on : Thu Jan  3 01:55:40 2019
 *
 * Target selection: ert.tlc
 * Embedded hardware selection: ARM Compatible->ARM Cortex
 * Code generation objectives:
 *    1. RAM efficiency
 *    2. Execution efficiency
 * Validation result: Not run
 */

#ifndef RTW_HEADER_Pointing_Mode_Control_System_h_
#define RTW_HEADER_Pointing_Mode_Control_System_h_
#include <math.h>
#ifndef Pointing_Mode_Control_System_COMMON_INCLUDES_
# define Pointing_Mode_Control_System_COMMON_INCLUDES_
#include "rtwtypes.hpp"
#endif                                 /* Pointing_Mode_Control_System_COMMON_INCLUDES_ */

#include "Pointing_Mode_Control_System_types.hpp"

/* Macros for accessing real-time model data structure */
#ifndef rtmGetErrorStatus
# define rtmGetErrorStatus(rtm)        ((rtm)->errorStatus)
#endif

#ifndef rtmSetErrorStatus
# define rtmSetErrorStatus(rtm, val)   ((rtm)->errorStatus = (val))
#endif

/* External inputs (root inport signals with default storage) */
typedef struct {
  real_T q_cmd[4];                     /* '<Root>/q_cmd' */
  real_T quat_body[4];                 /* '<Root>/q_body' */
  real_T w_cmd[3];                     /* '<Root>/w_cmd' */
  real_T w_body[3];                    /* '<Root>/w_body' */
  real_T rate_wheel[3];                /* '<Root>/LDR_h_rw' */
  real_T magField_body[3];             /* '<Root>/LDR_mag_field_body' */
} ExtU;

/* External outputs (root outports fed by signals with default storage) */
typedef struct {
  real_T hb_rw_cmd[3];                 /* '<Root>/hb_rw_cmd' */
  real_T m_mt_cmd[3];                  /* '<Root>/m_mt_cmd' */
} ExtY;

/* Real-time Model Data Structure */
struct tag_RTM {
  const char_T * volatile errorStatus;
};

/* External inputs (root inport signals with default storage) */
extern ExtU rtU;

/* External outputs (root outports fed by signals with default storage) */
extern ExtY rtY;

/*
 * Exported Global Parameters
 *
 * Note: Exported global parameters are tunable parameters with an exported
 * global storage class designation.  Code generation will declare the memory for
 * these parameters and exports their symbols.
 *
 */
extern real_T Kd;                      /* Variable: Kd
                                        * Referenced by: '<S1>/DerivativeGain'
                                        */
extern real_T Kp;                      /* Variable: Kp
                                        * Referenced by: '<S1>/ProportionalGain'
                                        */

/* Model entry point functions */
extern void Pointing_Mode_Control_System_initialize(void);
extern void Pointing_Mode_Control_System_step(void);

/* Real-time Model object */
extern RT_MODEL *const rtM;

/*-
 * The generated code includes comments that allow you to trace directly
 * back to the appropriate location in the model.  The basic format
 * is <system>/block_name, where system is the system number (uniquely
 * assigned by Simulink) and block_name is the name of the block.
 *
 * Note that this particular code originates from a subsystem build,
 * and has its own system numbers different from the parent model.
 * Refer to the system hierarchy for this subsystem below, and use the
 * MATLAB hilite_system command to trace the generated code back
 * to the parent model.  For example,
 *
 * hilite_system('single_sat_attitude/Controller1/Pointing_Mode_Control_System/Pointing_Controller')    - opens subsystem single_sat_attitude/Controller1/Pointing_Mode_Control_System/Pointing_Controller
 * hilite_system('single_sat_attitude/Controller1/Pointing_Mode_Control_System/Pointing_Controller/Kp') - opens and selects block Kp
 *
 * Here is the system hierarchy for this model
 *
 * '<Root>' : 'single_sat_attitude/Controller1/Pointing_Mode_Control_System'
 * '<S2>'   : 'single_sat_attitude/Controller1/Pointing_Mode_Control_System/Pointing_Mode_Control_System/Cross3'
 * '<S3>'   : 'single_sat_attitude/Controller1/Pointing_Mode_Control_System/Pointing_Mode_Control_System/compute_attitude_error'
 * '<S4>'   : 'single_sat_attitude/Controller1/Pointing_Mode_Control_System/Pointing_Mode_Control_System/compute_attitude_error/quat_multiply'
 * '<S5>'   : 'single_sat_attitude/Controller1/Pointing_Mode_Control_System/Pointing_Mode_Control_System/compute_attitude_error/quat_multiply1'
 */
#endif                                 /* RTW_HEADER_Pointing_Mode_Control_System_h_ */

/*
 * File trailer for generated code.
 *
 * [EOF]
 */
