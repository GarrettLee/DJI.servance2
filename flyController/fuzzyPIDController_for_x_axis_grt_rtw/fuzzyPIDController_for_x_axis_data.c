/*
 * fuzzyPIDController_for_x_axis_data.c
 *
 * Code generation for model "fuzzyPIDController_for_x_axis".
 *
 * Model version              : 1.8
 * Simulink Coder version : 8.8 (R2015a) 09-Feb-2015
 * C source code generated on : Sun Sep 18 21:56:07 2016
 *
 * Target selection: grt.tlc
 * Note: GRT includes extra infrastructure and instrumentation for prototyping
 * Embedded hardware selection: 32-bit Generic
 * Code generation objectives: Unspecified
 * Validation result: Not run
 */

#include "fuzzyPIDController_for_x_axis.h"
#include "fuzzyPIDController_for_x_axis_private.h"

/* Block parameters (auto storage) */
P_fuzzyPIDController_for_x_axis_T fuzzyPIDController_for_x_axis_P = {
  0.0,                                 /* Mask Parameter: DiscreteDerivative1_ICPrevScale
                                        * Referenced by: '<S2>/UD'
                                        */
  0.0,                                 /* Mask Parameter: DiscreteDerivative_ICPrevScaled
                                        * Referenced by: '<S1>/UD'
                                        */
  0.5,                                 /* Computed Parameter: TSamp_WtEt
                                        * Referenced by: '<S2>/TSamp'
                                        */
  1.0,                                 /* Expression: 1
                                        * Referenced by: '<Root>/Gain2'
                                        */
  2.0,                                 /* Computed Parameter: DiscreteTimeIntegrator_gainval
                                        * Referenced by: '<Root>/Discrete-Time Integrator'
                                        */
  0.0,                                 /* Expression: 0
                                        * Referenced by: '<Root>/Discrete-Time Integrator'
                                        */
  1.0,                                 /* Expression: 1
                                        * Referenced by: '<Root>/Gain3'
                                        */
  0.5,                                 /* Computed Parameter: TSamp_WtEt_h
                                        * Referenced by: '<S1>/TSamp'
                                        */
  1.0                                  /* Expression: 1
                                        * Referenced by: '<Root>/Gain4'
                                        */
};
