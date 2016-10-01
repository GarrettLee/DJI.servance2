/*
 * fuzzyPIDController_for_x_axis.h
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

#ifndef RTW_HEADER_fuzzyPIDController_for_x_axis_h_
#define RTW_HEADER_fuzzyPIDController_for_x_axis_h_
#include <float.h>
#include <string.h>
#include <stddef.h>
#ifndef fuzzyPIDController_for_x_axis_COMMON_INCLUDES_
# define fuzzyPIDController_for_x_axis_COMMON_INCLUDES_
#include "rtwtypes.h"
#include "rtw_continuous.h"
#include "rtw_solver.h"
#include "rt_logging.h"
#endif                                 /* fuzzyPIDController_for_x_axis_COMMON_INCLUDES_ */

#include "fuzzyPIDController_for_x_axis_types.h"

/* Shared type includes */
#include "multiword_types.h"
#include "rt_nonfinite.h"

/* Macros for accessing real-time model data structure */
#ifndef rtmGetFinalTime
# define rtmGetFinalTime(rtm)          ((rtm)->Timing.tFinal)
#endif

#ifndef rtmGetRTWLogInfo
# define rtmGetRTWLogInfo(rtm)         ((rtm)->rtwLogInfo)
#endif

#ifndef rtmGetErrorStatus
# define rtmGetErrorStatus(rtm)        ((rtm)->errorStatus)
#endif

#ifndef rtmSetErrorStatus
# define rtmSetErrorStatus(rtm, val)   ((rtm)->errorStatus = (val))
#endif

#ifndef rtmGetStopRequested
# define rtmGetStopRequested(rtm)      ((rtm)->Timing.stopRequestedFlag)
#endif

#ifndef rtmSetStopRequested
# define rtmSetStopRequested(rtm, val) ((rtm)->Timing.stopRequestedFlag = (val))
#endif

#ifndef rtmGetStopRequestedPtr
# define rtmGetStopRequestedPtr(rtm)   (&((rtm)->Timing.stopRequestedFlag))
#endif

#ifndef rtmGetT
# define rtmGetT(rtm)                  ((rtm)->Timing.taskTime0)
#endif

#ifndef rtmGetTFinal
# define rtmGetTFinal(rtm)             ((rtm)->Timing.tFinal)
#endif

#ifndef P_PARAMETER_X
# define P_PARAMETER_X fuzzyPIDController_for_x_axis_U.In4
#endif

#ifndef I_PARAMETER_X
# define I_PARAMETER_X fuzzyPIDController_for_x_axis_U.In2
#endif

#ifndef D_PARAMETER_X
# define D_PARAMETER_X fuzzyPIDController_for_x_axis_U.In3
#endif

#ifndef KP_PARAMETER_X 
# define KP_PARAMETER_X fuzzyPIDController_for_x_axis_U.In5
#endif

#ifndef KD_PARAMETER_X 
# define KD_PARAMETER_X fuzzyPIDController_for_x_axis_U.In6
#endif

#ifndef CONTROLLER_INPUT_X 
# define CONTROLLER_INPUT_X fuzzyPIDController_for_x_axis_U.In1
#endif

#ifndef FUZZY_KP_X 
# define FUZZY_KP_X fuzzyPIDController_for_x_axis_Y.fkp
#endif

#ifndef FUZZY_KI_Y
# define FUZZY_KI_X fuzzyPIDController_for_x_axis_Y.fki
#endif

#ifndef FUZZY_KD_X 
# define FUZZY_KD_X fuzzyPIDController_for_x_axis_Y.fkd
#endif

#ifndef FUZZY_DX_X 
# define FUZZY_DX_X fuzzyPIDController_for_x_axis_Y.dx
#endif

#ifndef FUZZY_KP_X 
# define FUZZY_KP_X fuzzyPIDController_for_x_axis_Y.fuzzy_kp
#endif

#ifndef FUZZY_KI_X 
# define FUZZY_KI_X fuzzyPIDController_for_x_axis_Y.fuzzy_ki
#endif

#ifndef FUZZY_KD_X 
# define FUZZY_KD_X fuzzyPIDController_for_x_axis_Y.fuzzy_kd
#endif


#ifndef CONTROLLER_OUTPUT_X 
# define CONTROLLER_OUTPUT_X fuzzyPIDController_for_x_axis_Y.Out1
#endif

/* Block signals (auto storage) */
typedef struct {
  real_T TmpSignalConversionAtAnimation1[2];
  real_T FISSfunction[3];              /* '<S4>/FIS S-function' */
} B_fuzzyPIDController_for_x_axis_T;

/* Block states (auto storage) for system '<Root>' */
typedef struct {
  real_T UD_DSTATE;                    /* '<S2>/UD' */
  real_T DiscreteTimeIntegrator_DSTATE;/* '<Root>/Discrete-Time Integrator' */
  real_T UD_DSTATE_f;                  /* '<S1>/UD' */
  struct {
    void *FISPointer;
  } FISSfunction_PWORK;                /* '<S4>/FIS S-function' */
} DW_fuzzyPIDController_for_x_axis_T;

/* External inputs (root inport signals with auto storage) */
typedef struct {
  real_T In1;                          /* '<Root>/In1' */
  real_T In2;                          /* '<Root>/In2' */
  real_T In3;                          /* '<Root>/In3' */
  real_T In4;                          /* '<Root>/In4' */
  real_T In5;                          /* '<Root>/In5' */
  real_T In6;                          /* '<Root>/In6' */
  real_T fki;                          /* '<Root>/fki' */
  real_T fkp;                          /* '<Root>/fkp' */
  real_T fkd;                          /* '<Root>/fkd' */
} ExtU_fuzzyPIDController_for_x_axis_T;

/* External outputs (root outports fed by signals with auto storage) */
typedef struct {
  real_T Out1;                         /* '<Root>/Out1' */
  real_T dx;                           /* '<Root>/dx' */
  real_T fuzzy_kp;                     /* '<Root>/fuzzy_kp' */
  real_T fuzzy_ki;                     /* '<Root>/fuzzy_ki' */
  real_T fuzzy_kd;                     /* '<Root>/fuzzy_kd' */
} ExtY_fuzzyPIDController_for_x_axis_T;

/* Parameters (auto storage) */
struct P_fuzzyPIDController_for_x_axis_T_ {
  real_T DiscreteDerivative1_ICPrevScale;/* Mask Parameter: DiscreteDerivative1_ICPrevScale
                                          * Referenced by: '<S2>/UD'
                                          */
  real_T DiscreteDerivative_ICPrevScaled;/* Mask Parameter: DiscreteDerivative_ICPrevScaled
                                          * Referenced by: '<S1>/UD'
                                          */
  real_T TSamp_WtEt;                   /* Computed Parameter: TSamp_WtEt
                                        * Referenced by: '<S2>/TSamp'
                                        */
  real_T Gain2_Gain;                   /* Expression: 1
                                        * Referenced by: '<Root>/Gain2'
                                        */
  real_T DiscreteTimeIntegrator_gainval;/* Computed Parameter: DiscreteTimeIntegrator_gainval
                                         * Referenced by: '<Root>/Discrete-Time Integrator'
                                         */
  real_T DiscreteTimeIntegrator_IC;    /* Expression: 0
                                        * Referenced by: '<Root>/Discrete-Time Integrator'
                                        */
  real_T Gain3_Gain;                   /* Expression: 1
                                        * Referenced by: '<Root>/Gain3'
                                        */
  real_T TSamp_WtEt_h;                 /* Computed Parameter: TSamp_WtEt_h
                                        * Referenced by: '<S1>/TSamp'
                                        */
  real_T Gain4_Gain;                   /* Expression: 1
                                        * Referenced by: '<Root>/Gain4'
                                        */
};

/* Real-time Model Data Structure */
struct tag_RTM_fuzzyPIDController_for_x_axis_T {
  const char_T *errorStatus;
  RTWLogInfo *rtwLogInfo;

  /*
   * Timing:
   * The following substructure contains information regarding
   * the timing information for the model.
   */
  struct {
    time_T taskTime0;
    uint32_T clockTick0;
    uint32_T clockTickH0;
    time_T stepSize0;
    time_T tFinal;
    boolean_T stopRequestedFlag;
  } Timing;
};

/* Block parameters (auto storage) */
extern P_fuzzyPIDController_for_x_axis_T fuzzyPIDController_for_x_axis_P;

/* Block signals (auto storage) */
extern B_fuzzyPIDController_for_x_axis_T fuzzyPIDController_for_x_axis_B;

/* Block states (auto storage) */
extern DW_fuzzyPIDController_for_x_axis_T fuzzyPIDController_for_x_axis_DW;

/* External inputs (root inport signals with auto storage) */
extern ExtU_fuzzyPIDController_for_x_axis_T fuzzyPIDController_for_x_axis_U;

/* External outputs (root outports fed by signals with auto storage) */
extern ExtY_fuzzyPIDController_for_x_axis_T fuzzyPIDController_for_x_axis_Y;

/* Model entry point functions */
extern void fuzzyPIDController_for_x_axis_initialize(void);
extern void fuzzyPIDController_for_x_axis_step(void);
extern void fuzzyPIDController_for_x_axis_terminate(void);

/* Real-time Model object */
extern RT_MODEL_fuzzyPIDController_for_x_axis_T *const
  fuzzyPIDController_for_x_axis_M;

/*-
 * The generated code includes comments that allow you to trace directly
 * back to the appropriate location in the model.  The basic format
 * is <system>/block_name, where system is the system number (uniquely
 * assigned by Simulink) and block_name is the name of the block.
 *
 * Use the MATLAB hilite_system command to trace the generated code back
 * to the model.  For example,
 *
 * hilite_system('<S3>')    - opens system 3
 * hilite_system('<S3>/Kp') - opens and selects block Kp which resides in S3
 *
 * Here is the system hierarchy for this model
 *
 * '<Root>' : 'fuzzyPIDController_for_x_axis'
 * '<S1>'   : 'fuzzyPIDController_for_x_axis/Discrete Derivative'
 * '<S2>'   : 'fuzzyPIDController_for_x_axis/Discrete Derivative1'
 * '<S3>'   : 'fuzzyPIDController_for_x_axis/Fuzzy Logic  Controller  with Ruleviewer'
 * '<S4>'   : 'fuzzyPIDController_for_x_axis/Fuzzy Logic  Controller  with Ruleviewer/Fuzzy Logic Controller'
 */
#endif                                 /* RTW_HEADER_fuzzyPIDController_for_x_axis_h_ */
