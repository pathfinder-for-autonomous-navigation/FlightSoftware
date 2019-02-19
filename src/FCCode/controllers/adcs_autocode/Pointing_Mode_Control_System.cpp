/*
 * Academic License - for use in teaching, academic research, and meeting
 * course requirements at degree granting institutions only.  Not for
 * government, commercial, or other organizational use.
 *
 * File: Pointing_Mode_Control_System.c
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

#include "Pointing_Mode_Control_System.hpp"
#include "Pointing_Mode_Control_System_private.hpp"

/* Exported block parameters */
real_T Kd = 0.0075;                    /* Variable: Kd
                                        * Referenced by: '<S1>/DerivativeGain'
                                        */
real_T Kp = 0.00075;                   /* Variable: Kp
                                        * Referenced by: '<S1>/ProportionalGain'
                                        */

/* External inputs (root inport signals with default storage) */
ExtU rtU;

/* External outputs (root outports fed by signals with default storage) */
ExtY rtY;

/* Real-time model */
RT_MODEL rtM_;
RT_MODEL *const rtM = &rtM_;

/* Model step function */
void Pointing_Mode_Control_System_step(void)
{
  real_T y;
  real_T y_0;
  real_T acc;
  real_T rtb_reverse_0;
  real_T rtb_reverse_idx_0;
  real_T rtb_reverse_idx_1;
  real_T rtb_reverse_idx_2;
  real_T rtb_Switch_idx_0;
  real_T rtb_Switch_idx_1;
  real_T rtb_Switch_idx_2;

  /* Outputs for Atomic SubSystem: '<Root>/Pointing_Mode_Control_System' */
  /* Switch: '<S3>/Switch' incorporates:
   *  Fcn: '<S4>/qerr1'
   *  Fcn: '<S4>/qerr2'
   *  Fcn: '<S4>/qerr3'
   *  Fcn: '<S4>/qerr4'
   *  Fcn: '<S5>/qerr1'
   *  Fcn: '<S5>/qerr2'
   *  Fcn: '<S5>/qerr3'
   *  Gain: '<S3>/reverse'
   *  Gain: '<S3>/reverse1'
   *  Inport: '<Root>/q_body'
   *  Inport: '<Root>/q_cmd'
   */
  if (((-rtU.quat_body[0] * -rtU.q_cmd[0] - rtU.quat_body[1] * -rtU.q_cmd[1]) -
       rtU.quat_body[2] * -rtU.q_cmd[2]) + rtU.quat_body[3] * rtU.q_cmd[3] >=
      0.0) {
    rtb_Switch_idx_0 = ((rtU.quat_body[3] * -rtU.q_cmd[0] + rtU.quat_body[2] *
                         -rtU.q_cmd[1]) - rtU.quat_body[1] * -rtU.q_cmd[2]) +
      rtU.quat_body[0] * rtU.q_cmd[3];
    rtb_Switch_idx_1 = ((-rtU.quat_body[2] * -rtU.q_cmd[0] + rtU.quat_body[3] *
                         -rtU.q_cmd[1]) + rtU.quat_body[0] * -rtU.q_cmd[2]) +
      rtU.quat_body[1] * rtU.q_cmd[3];
    rtb_Switch_idx_2 = ((rtU.quat_body[1] * -rtU.q_cmd[0] - rtU.quat_body[0] *
                         -rtU.q_cmd[1]) + rtU.quat_body[3] * -rtU.q_cmd[2]) +
      rtU.quat_body[2] * rtU.q_cmd[3];
  } else {
    rtb_Switch_idx_0 = ((rtU.quat_body[3] * rtU.q_cmd[0] + rtU.quat_body[2] *
                         rtU.q_cmd[1]) - rtU.quat_body[1] * rtU.q_cmd[2]) +
      rtU.quat_body[0] * -rtU.q_cmd[3];
    rtb_Switch_idx_1 = ((-rtU.quat_body[2] * rtU.q_cmd[0] + rtU.quat_body[3] *
                         rtU.q_cmd[1]) + rtU.quat_body[0] * rtU.q_cmd[2]) +
      rtU.quat_body[1] * -rtU.q_cmd[3];
    rtb_Switch_idx_2 = ((rtU.quat_body[1] * rtU.q_cmd[0] - rtU.quat_body[0] *
                         rtU.q_cmd[1]) + rtU.quat_body[3] * rtU.q_cmd[2]) +
      rtU.quat_body[2] * -rtU.q_cmd[3];
  }

  /* End of Switch: '<S3>/Switch' */

  /* Gain: '<S1>/ProportionalGain' */
  y = Kp * 10.0;

  /* Gain: '<S1>/DerivativeGain' */
  y_0 = Kd * 0.5;

  /* Gain: '<S1>/Gain1' incorporates:
   *  Inport: '<Root>/LDR_h_rw'
   */
  rtY.m_mt_cmd[0] = 1.35E-5 * rtU.rate_wheel[0];

  /* Gain: '<S1>/G2T' incorporates:
   *  Inport: '<Root>/LDR_mag_field_body'
   */
  rtb_reverse_idx_0 = 0.0001 * rtU.magField_body[0];

  /* Gain: '<S1>/Gain1' incorporates:
   *  Inport: '<Root>/LDR_h_rw'
   */
  rtY.m_mt_cmd[1] = 1.35E-5 * rtU.rate_wheel[1];

  /* Gain: '<S1>/G2T' incorporates:
   *  Inport: '<Root>/LDR_mag_field_body'
   */
  rtb_reverse_idx_1 = 0.0001 * rtU.magField_body[1];

  /* Gain: '<S1>/Gain1' incorporates:
   *  Inport: '<Root>/LDR_h_rw'
   */
  rtY.m_mt_cmd[2] = 1.35E-5 * rtU.rate_wheel[2];

  /* Gain: '<S1>/G2T' incorporates:
   *  Inport: '<Root>/LDR_mag_field_body'
   */
  rtb_reverse_idx_2 = 0.0001 * rtU.magField_body[2];

  /* Product: '<S2>/Product2' */
  rtb_reverse_0 = rtb_reverse_idx_0;

  /* Product: '<S2>/Product4' */
  acc = rtb_reverse_idx_1;

  /* SignalConversion: '<S1>/TmpSignal ConversionAtDotProduct1Inport1' incorporates:
   *  Product: '<S2>/Product'
   *  Product: '<S2>/Product1'
   *  Product: '<S2>/Product2'
   *  Product: '<S2>/Product3'
   *  Product: '<S2>/Product4'
   *  Product: '<S2>/Product5'
   *  Sum: '<S2>/Sum'
   *  Sum: '<S2>/Sum1'
   *  Sum: '<S2>/Sum2'
   */
  rtb_reverse_idx_0 = rtY.m_mt_cmd[1] * rtb_reverse_idx_2 - rtY.m_mt_cmd[2] *
    rtb_reverse_idx_1;
  rtb_reverse_idx_1 = rtY.m_mt_cmd[2] * rtb_reverse_0 - rtY.m_mt_cmd[0] *
    rtb_reverse_idx_2;
  rtb_reverse_idx_2 = rtY.m_mt_cmd[0] * acc - rtY.m_mt_cmd[1] * rtb_reverse_0;

  /* S-Function (sdsp2norm2): '<S1>/Normalization' incorporates:
   *  DotProduct: '<S1>/DotProduct1'
   */
  rtb_reverse_0 = (rtb_reverse_idx_0 * rtb_reverse_idx_0 + rtb_reverse_idx_1 *
                   rtb_reverse_idx_1) + rtb_reverse_idx_2 * rtb_reverse_idx_2;
  acc = 1.0 / (sqrt(rtb_reverse_0) + 1.0E-30);
  rtY.m_mt_cmd[0] = rtb_reverse_idx_0 * acc;

  /* Outport: '<Root>/hb_rw_cmd' incorporates:
   *  Gain: '<S1>/DerivativeGain'
   *  Gain: '<S1>/ProportionalGain'
   *  Inport: '<Root>/w_body'
   *  Inport: '<Root>/w_cmd'
   *  Sum: '<S1>/Sum1'
   *  Sum: '<S1>/w_error'
   */
  rtY.hb_rw_cmd[0] = (rtU.w_body[0] - rtU.w_cmd[0]) * y_0 + y * rtb_Switch_idx_0;

  /* S-Function (sdsp2norm2): '<S1>/Normalization' */
  rtY.m_mt_cmd[1] = rtb_reverse_idx_1 * acc;

  /* Outport: '<Root>/hb_rw_cmd' incorporates:
   *  Gain: '<S1>/DerivativeGain'
   *  Gain: '<S1>/ProportionalGain'
   *  Inport: '<Root>/w_body'
   *  Inport: '<Root>/w_cmd'
   *  Sum: '<S1>/Sum1'
   *  Sum: '<S1>/w_error'
   */
  rtY.hb_rw_cmd[1] = (rtU.w_body[1] - rtU.w_cmd[1]) * y_0 + y * rtb_Switch_idx_1;

  /* S-Function (sdsp2norm2): '<S1>/Normalization' */
  rtY.m_mt_cmd[2] = rtb_reverse_idx_2 * acc;

  /* Outport: '<Root>/hb_rw_cmd' incorporates:
   *  Gain: '<S1>/DerivativeGain'
   *  Gain: '<S1>/ProportionalGain'
   *  Inport: '<Root>/w_body'
   *  Inport: '<Root>/w_cmd'
   *  Sum: '<S1>/Sum1'
   *  Sum: '<S1>/w_error'
   */
  rtY.hb_rw_cmd[2] = (rtU.w_body[2] - rtU.w_cmd[2]) * y_0 + y * rtb_Switch_idx_2;

  /* Gain: '<S1>/Gain' */
  rtb_Switch_idx_0 = rtb_reverse_0 * 1.0E+15;

  /* Saturate: '<S1>/Saturation' */
  if (rtb_Switch_idx_0 > 0.08) {
    rtb_Switch_idx_0 = 0.08;
  } else {
    if (rtb_Switch_idx_0 < 0.0) {
      rtb_Switch_idx_0 = 0.0;
    }
  }

  /* End of Saturate: '<S1>/Saturation' */

  /* Outport: '<Root>/m_mt_cmd' incorporates:
   *  Product: '<S1>/Product'
   */
  rtY.m_mt_cmd[0] *= rtb_Switch_idx_0;
  rtY.m_mt_cmd[1] *= rtb_Switch_idx_0;
  rtY.m_mt_cmd[2] *= rtb_Switch_idx_0;

  /* End of Outputs for SubSystem: '<Root>/Pointing_Mode_Control_System' */
}

/* Model initialize function */
void Pointing_Mode_Control_System_initialize(void)
{
  /* (no initialization code required) */
}

/*
 * File trailer for generated code.
 *
 * [EOF]
 */
