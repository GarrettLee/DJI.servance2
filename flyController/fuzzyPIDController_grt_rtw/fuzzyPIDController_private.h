/*
 * fuzzyPIDController_private.h
 *
 * Code generation for model "fuzzyPIDController".
 *
 * Model version              : 1.7
 * Simulink Coder version : 8.8 (R2015a) 09-Feb-2015
 * C source code generated on : Mon Sep 12 23:47:06 2016
 *
 * Target selection: grt.tlc
 * Note: GRT includes extra infrastructure and instrumentation for prototyping
 * Embedded hardware selection: 32-bit Generic
 * Code generation objectives: Unspecified
 * Validation result: Not run
 */

#ifndef RTW_HEADER_fuzzyPIDController_private_h_
#define RTW_HEADER_fuzzyPIDController_private_h_
#include "rtwtypes.h"
#include "builtin_typeid_types.h"
#include "multiword_types.h"
#include "fis.h"
#ifdef __cplusplus

extern "C" {

#endif

  extern void fisAssignMfPointer(FIS *fis);
  extern void fisCheckDataStructure(FIS *fis);
  extern void fisAssignFunctionPointer(FIS *fis);
  extern IO *fisBuildIoList(int_T node_n, int_T *mf_n);
  extern void fisComputeOutputMfValueArray(FIS *fis, int numofpoints);
  extern char **fisCreateMatrix(int row_n, int col_n, int element_size);
  extern void fisFreeFisNode(FIS *fis);
  extern void fisEvaluate(FIS *fis, int numofpoints);

#ifdef __cplusplus

}
#endif

/* Private macros used by the generated code to access rtModel */
#ifndef rtmSetTFinal
# define rtmSetTFinal(rtm, val)        ((rtm)->Timing.tFinal = (val))
#endif

#ifndef rtmGetTPtr
# define rtmGetTPtr(rtm)               (&(rtm)->Timing.taskTime0)
#endif
#endif                                 /* RTW_HEADER_fuzzyPIDController_private_h_ */
