/*
 * fuzzyPIDController_for_x_axis.c
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

/* Block signals (auto storage) */
B_fuzzyPIDController_for_x_axis_T fuzzyPIDController_for_x_axis_B;

/* Block states (auto storage) */
DW_fuzzyPIDController_for_x_axis_T fuzzyPIDController_for_x_axis_DW;

/* External inputs (root inport signals with auto storage) */
ExtU_fuzzyPIDController_for_x_axis_T fuzzyPIDController_for_x_axis_U;

/* External outputs (root outports fed by signals with auto storage) */
ExtY_fuzzyPIDController_for_x_axis_T fuzzyPIDController_for_x_axis_Y;

/* Real-time model */
RT_MODEL_fuzzyPIDController_for_x_axis_T fuzzyPIDController_for_x_axis_M_;
RT_MODEL_fuzzyPIDController_for_x_axis_T *const fuzzyPIDController_for_x_axis_M =
  &fuzzyPIDController_for_x_axis_M_;

/* Model step function */
void fuzzyPIDController_for_x_axis_step(void)
{
  real_T rtb_Product1;
  real_T rtb_Diff_g;
  real_T rtb_TSamp;
  real_T rtb_Diff;
  real_T rtb_Product;
  real_T rtb_TSamp_g;
  real_T rtb_Product2;
  real_T rtb_Add;

  /* Product: '<Root>/Product5' incorporates:
   *  Inport: '<Root>/In1'
   *  Inport: '<Root>/In4'
   */
  rtb_Product1 = fuzzyPIDController_for_x_axis_U.In4 *
    fuzzyPIDController_for_x_axis_U.In1;

  /* SampleTimeMath: '<S2>/TSamp' incorporates:
   *  Inport: '<Root>/In1'
   *
   * About '<S2>/TSamp':
   *  y = u * K where K = 1 / ( w * Ts )
   */
  rtb_TSamp = fuzzyPIDController_for_x_axis_U.In1 *
    fuzzyPIDController_for_x_axis_P.TSamp_WtEt;

  /* Sum: '<S2>/Diff' incorporates:
   *  UnitDelay: '<S2>/UD'
   */
  rtb_Diff = rtb_TSamp - fuzzyPIDController_for_x_axis_DW.UD_DSTATE;

  /* SignalConversion: '<S3>/TmpSignal ConversionAtAnimation1Inport1' incorporates:
   *  Inport: '<Root>/In1'
   *  Inport: '<Root>/In5'
   *  Inport: '<Root>/In6'
   *  Product: '<Root>/Product6'
   *  Product: '<Root>/Product7'
   */
  fuzzyPIDController_for_x_axis_B.TmpSignalConversionAtAnimation1[0] =
    fuzzyPIDController_for_x_axis_U.In5 * fuzzyPIDController_for_x_axis_U.In1;
  fuzzyPIDController_for_x_axis_B.TmpSignalConversionAtAnimation1[1] =
    fuzzyPIDController_for_x_axis_U.In6 * rtb_Diff;

  /* S-Function Block: <S4>/FIS S-function (sffis) */
  {
    FIS *fis = (FIS *)
      fuzzyPIDController_for_x_axis_DW.FISSfunction_PWORK.FISPointer;
    fis->input[0]->value =
      fuzzyPIDController_for_x_axis_B.TmpSignalConversionAtAnimation1[0];
    fis->input[1]->value =
      fuzzyPIDController_for_x_axis_B.TmpSignalConversionAtAnimation1[1];
    fisEvaluate(fis, 101);
    fuzzyPIDController_for_x_axis_B.FISSfunction[0] = fis->output[0]->value;
    fuzzyPIDController_for_x_axis_B.FISSfunction[1] = fis->output[1]->value;
    fuzzyPIDController_for_x_axis_B.FISSfunction[2] = fis->output[2]->value;
  }

  /* Product: '<Root>/Product' incorporates:
   *  Gain: '<Root>/Gain2'
   *  Inport: '<Root>/fkp'
   *  Product: '<Root>/Product9'
   */
  rtb_Product = fuzzyPIDController_for_x_axis_P.Gain2_Gain *
    fuzzyPIDController_for_x_axis_B.FISSfunction[0] *
    fuzzyPIDController_for_x_axis_U.fkp * rtb_Product1;

  /* Sum: '<Root>/Add' */
  rtb_Add = rtb_Product1 + rtb_Product;

  /* Product: '<Root>/Product1' incorporates:
   *  DiscreteIntegrator: '<Root>/Discrete-Time Integrator'
   *  Gain: '<Root>/Gain3'
   *  Inport: '<Root>/fki'
   *  Product: '<Root>/Product8'
   */
  rtb_Product1 = fuzzyPIDController_for_x_axis_P.Gain3_Gain *
    fuzzyPIDController_for_x_axis_B.FISSfunction[1] *
    fuzzyPIDController_for_x_axis_U.fki *
    fuzzyPIDController_for_x_axis_DW.DiscreteTimeIntegrator_DSTATE;

  /* SampleTimeMath: '<S1>/TSamp' incorporates:
   *  Inport: '<Root>/In1'
   *  Inport: '<Root>/In3'
   *  Product: '<Root>/Product3'
   *
   * About '<S1>/TSamp':
   *  y = u * K where K = 1 / ( w * Ts )
   */
  rtb_TSamp_g = fuzzyPIDController_for_x_axis_U.In3 *
    fuzzyPIDController_for_x_axis_U.In1 *
    fuzzyPIDController_for_x_axis_P.TSamp_WtEt_h;

  /* Sum: '<S1>/Diff' incorporates:
   *  UnitDelay: '<S1>/UD'
   */
  rtb_Diff_g = rtb_TSamp_g - fuzzyPIDController_for_x_axis_DW.UD_DSTATE_f;

  /* Product: '<Root>/Product2' incorporates:
   *  Gain: '<Root>/Gain4'
   *  Inport: '<Root>/fkd'
   *  Product: '<Root>/Product10'
   */
  rtb_Product2 = fuzzyPIDController_for_x_axis_P.Gain4_Gain *
    fuzzyPIDController_for_x_axis_B.FISSfunction[2] *
    fuzzyPIDController_for_x_axis_U.fkd * rtb_Diff_g;

  /* Outport: '<Root>/Out1' incorporates:
   *  DiscreteIntegrator: '<Root>/Discrete-Time Integrator'
   *  Sum: '<Root>/Add1'
   *  Sum: '<Root>/Add2'
   *  Sum: '<Root>/Add3'
   */
  fuzzyPIDController_for_x_axis_Y.Out1 =
    ((fuzzyPIDController_for_x_axis_DW.DiscreteTimeIntegrator_DSTATE +
      rtb_Product1) + rtb_Add) + (rtb_Diff_g + rtb_Product2);

  /* Outport: '<Root>/dx' */
  fuzzyPIDController_for_x_axis_Y.dx = rtb_Diff;

  /* Outport: '<Root>/fuzzy_kp' */
  fuzzyPIDController_for_x_axis_Y.fuzzy_kp = rtb_Product;

  /* Outport: '<Root>/fuzzy_ki' */
  fuzzyPIDController_for_x_axis_Y.fuzzy_ki = rtb_Product1;

  /* Outport: '<Root>/fuzzy_kd' */
  fuzzyPIDController_for_x_axis_Y.fuzzy_kd = rtb_Product2;

  /* Update for UnitDelay: '<S2>/UD' */
  fuzzyPIDController_for_x_axis_DW.UD_DSTATE = rtb_TSamp;

  /* Update for DiscreteIntegrator: '<Root>/Discrete-Time Integrator' incorporates:
   *  Inport: '<Root>/In1'
   *  Inport: '<Root>/In2'
   *  Product: '<Root>/Product4'
   */
  fuzzyPIDController_for_x_axis_DW.DiscreteTimeIntegrator_DSTATE +=
    fuzzyPIDController_for_x_axis_U.In2 * fuzzyPIDController_for_x_axis_U.In1 *
    fuzzyPIDController_for_x_axis_P.DiscreteTimeIntegrator_gainval;

  /* Update for UnitDelay: '<S1>/UD' */
  fuzzyPIDController_for_x_axis_DW.UD_DSTATE_f = rtb_TSamp_g;

  /* Matfile logging */
  rt_UpdateTXYLogVars(fuzzyPIDController_for_x_axis_M->rtwLogInfo,
                      (&fuzzyPIDController_for_x_axis_M->Timing.taskTime0));

  /* signal main to stop simulation */
  {                                    /* Sample time: [2.0s, 0.0s] */
    if ((rtmGetTFinal(fuzzyPIDController_for_x_axis_M)!=-1) &&
        !((rtmGetTFinal(fuzzyPIDController_for_x_axis_M)-
           fuzzyPIDController_for_x_axis_M->Timing.taskTime0) >
          fuzzyPIDController_for_x_axis_M->Timing.taskTime0 * (DBL_EPSILON))) {
      rtmSetErrorStatus(fuzzyPIDController_for_x_axis_M, "Simulation finished");
    }
  }

  /* Update absolute time for base rate */
  /* The "clockTick0" counts the number of times the code of this task has
   * been executed. The absolute time is the multiplication of "clockTick0"
   * and "Timing.stepSize0". Size of "clockTick0" ensures timer will not
   * overflow during the application lifespan selected.
   * Timer of this task consists of two 32 bit unsigned integers.
   * The two integers represent the low bits Timing.clockTick0 and the high bits
   * Timing.clockTickH0. When the low bit overflows to 0, the high bits increment.
   */
  if (!(++fuzzyPIDController_for_x_axis_M->Timing.clockTick0)) {
    ++fuzzyPIDController_for_x_axis_M->Timing.clockTickH0;
  }

  fuzzyPIDController_for_x_axis_M->Timing.taskTime0 =
    fuzzyPIDController_for_x_axis_M->Timing.clockTick0 *
    fuzzyPIDController_for_x_axis_M->Timing.stepSize0 +
    fuzzyPIDController_for_x_axis_M->Timing.clockTickH0 *
    fuzzyPIDController_for_x_axis_M->Timing.stepSize0 * 4294967296.0;
}

/* Model initialize function */
void fuzzyPIDController_for_x_axis_initialize(void)
{
  /* Registration code */

  /* initialize non-finites */
  rt_InitInfAndNaN(sizeof(real_T));

  /* initialize real-time model */
  (void) memset((void *)fuzzyPIDController_for_x_axis_M, 0,
                sizeof(RT_MODEL_fuzzyPIDController_for_x_axis_T));
  rtmSetTFinal(fuzzyPIDController_for_x_axis_M, -1);
  fuzzyPIDController_for_x_axis_M->Timing.stepSize0 = 2.0;

  /* Setup for data logging */
  {
    static RTWLogInfo rt_DataLoggingInfo;
    fuzzyPIDController_for_x_axis_M->rtwLogInfo = &rt_DataLoggingInfo;
  }

  /* Setup for data logging */
  {
    rtliSetLogXSignalInfo(fuzzyPIDController_for_x_axis_M->rtwLogInfo, (NULL));
    rtliSetLogXSignalPtrs(fuzzyPIDController_for_x_axis_M->rtwLogInfo, (NULL));
    rtliSetLogT(fuzzyPIDController_for_x_axis_M->rtwLogInfo, "tout");
    rtliSetLogX(fuzzyPIDController_for_x_axis_M->rtwLogInfo, "");
    rtliSetLogXFinal(fuzzyPIDController_for_x_axis_M->rtwLogInfo, "");
    rtliSetLogVarNameModifier(fuzzyPIDController_for_x_axis_M->rtwLogInfo, "rt_");
    rtliSetLogFormat(fuzzyPIDController_for_x_axis_M->rtwLogInfo, 0);
    rtliSetLogMaxRows(fuzzyPIDController_for_x_axis_M->rtwLogInfo, 1000);
    rtliSetLogDecimation(fuzzyPIDController_for_x_axis_M->rtwLogInfo, 1);

    /*
     * Set pointers to the data and signal info for each output
     */
    {
      static void * rt_LoggedOutputSignalPtrs[] = {
        &fuzzyPIDController_for_x_axis_Y.Out1,
        &fuzzyPIDController_for_x_axis_Y.dx,
        &fuzzyPIDController_for_x_axis_Y.fuzzy_kp,
        &fuzzyPIDController_for_x_axis_Y.fuzzy_ki,
        &fuzzyPIDController_for_x_axis_Y.fuzzy_kd
      };

      rtliSetLogYSignalPtrs(fuzzyPIDController_for_x_axis_M->rtwLogInfo,
                            ((LogSignalPtrsType)rt_LoggedOutputSignalPtrs));
    }

    {
      static int_T rt_LoggedOutputWidths[] = {
        1,
        1,
        1,
        1,
        1
      };

      static int_T rt_LoggedOutputNumDimensions[] = {
        1,
        1,
        1,
        1,
        1
      };

      static int_T rt_LoggedOutputDimensions[] = {
        1,
        1,
        1,
        1,
        1
      };

      static boolean_T rt_LoggedOutputIsVarDims[] = {
        0,
        0,
        0,
        0,
        0
      };

      static void* rt_LoggedCurrentSignalDimensions[] = {
        (NULL),
        (NULL),
        (NULL),
        (NULL),
        (NULL)
      };

      static int_T rt_LoggedCurrentSignalDimensionsSize[] = {
        4,
        4,
        4,
        4,
        4
      };

      static BuiltInDTypeId rt_LoggedOutputDataTypeIds[] = {
        SS_DOUBLE,
        SS_DOUBLE,
        SS_DOUBLE,
        SS_DOUBLE,
        SS_DOUBLE
      };

      static int_T rt_LoggedOutputComplexSignals[] = {
        0,
        0,
        0,
        0,
        0
      };

      static const char_T *rt_LoggedOutputLabels[] = {
        "",
        "",
        "",
        "",
        "" };

      static const char_T *rt_LoggedOutputBlockNames[] = {
        "fuzzyPIDController_for_x_axis/Out1",
        "fuzzyPIDController_for_x_axis/dx",
        "fuzzyPIDController_for_x_axis/fuzzy_kp",
        "fuzzyPIDController_for_x_axis/fuzzy_ki",
        "fuzzyPIDController_for_x_axis/fuzzy_kd" };

      static RTWLogDataTypeConvert rt_RTWLogDataTypeConvert[] = {
        { 0, SS_DOUBLE, SS_DOUBLE, 0, 0, 0, 1.0, 0, 0.0 },

        { 0, SS_DOUBLE, SS_DOUBLE, 0, 0, 0, 1.0, 0, 0.0 },

        { 0, SS_DOUBLE, SS_DOUBLE, 0, 0, 0, 1.0, 0, 0.0 },

        { 0, SS_DOUBLE, SS_DOUBLE, 0, 0, 0, 1.0, 0, 0.0 },

        { 0, SS_DOUBLE, SS_DOUBLE, 0, 0, 0, 1.0, 0, 0.0 }
      };

      static RTWLogSignalInfo rt_LoggedOutputSignalInfo[] = {
        {
          5,
          rt_LoggedOutputWidths,
          rt_LoggedOutputNumDimensions,
          rt_LoggedOutputDimensions,
          rt_LoggedOutputIsVarDims,
          rt_LoggedCurrentSignalDimensions,
          rt_LoggedCurrentSignalDimensionsSize,
          rt_LoggedOutputDataTypeIds,
          rt_LoggedOutputComplexSignals,
          (NULL),

          { rt_LoggedOutputLabels },
          (NULL),
          (NULL),
          (NULL),

          { rt_LoggedOutputBlockNames },

          { (NULL) },
          (NULL),
          rt_RTWLogDataTypeConvert
        }
      };

      rtliSetLogYSignalInfo(fuzzyPIDController_for_x_axis_M->rtwLogInfo,
                            rt_LoggedOutputSignalInfo);

      /* set currSigDims field */
      rt_LoggedCurrentSignalDimensions[0] = &rt_LoggedOutputWidths[0];
      rt_LoggedCurrentSignalDimensions[1] = &rt_LoggedOutputWidths[1];
      rt_LoggedCurrentSignalDimensions[2] = &rt_LoggedOutputWidths[2];
      rt_LoggedCurrentSignalDimensions[3] = &rt_LoggedOutputWidths[3];
      rt_LoggedCurrentSignalDimensions[4] = &rt_LoggedOutputWidths[4];
    }

    rtliSetLogY(fuzzyPIDController_for_x_axis_M->rtwLogInfo, "yout");
  }

  /* block I/O */
  (void) memset(((void *) &fuzzyPIDController_for_x_axis_B), 0,
                sizeof(B_fuzzyPIDController_for_x_axis_T));

  /* states (dwork) */
  (void) memset((void *)&fuzzyPIDController_for_x_axis_DW, 0,
                sizeof(DW_fuzzyPIDController_for_x_axis_T));

  /* external inputs */
  (void) memset((void *)&fuzzyPIDController_for_x_axis_U, 0,
                sizeof(ExtU_fuzzyPIDController_for_x_axis_T));

  /* external outputs */
  (void) memset((void *)&fuzzyPIDController_for_x_axis_Y, 0,
                sizeof(ExtY_fuzzyPIDController_for_x_axis_T));

  /* Matfile logging */
  rt_StartDataLoggingWithStartTime(fuzzyPIDController_for_x_axis_M->rtwLogInfo,
    0.0, rtmGetTFinal(fuzzyPIDController_for_x_axis_M),
    fuzzyPIDController_for_x_axis_M->Timing.stepSize0, (&rtmGetErrorStatus
    (fuzzyPIDController_for_x_axis_M)));

  /* S-Function Block: <S4>/FIS S-function (sffis) */
  {
    FIS *fis;
    if ((fis = (FIS *)calloc(1, sizeof(FIS))) == NULL ) {
      rtmSetErrorStatus(fuzzyPIDController_for_x_axis_M,
                        "Memory Allocation Error");
      return;
    }

    (void)strcpy(fis->name, "fuzzf1");
    (void)strcpy(fis->type, "mamdani");
    (void)strcpy(fis->andMethod, "min");
    (void)strcpy(fis->orMethod, "max");
    (void)strcpy(fis->defuzzMethod, "mom");
    (void)strcpy(fis->impMethod, "min");
    (void)strcpy(fis->aggMethod, "max");

    {
      IO *io_list;
      int_T *in_mf_n;
      int_T i;
      if ((in_mf_n = (int_T *)calloc(2, sizeof(int_T))) == NULL ) {
        rtmSetErrorStatus(fuzzyPIDController_for_x_axis_M,
                          "Memory Allocation Error");
        return;
      }

      in_mf_n[0] = 7;
      in_mf_n[1] = 7;
      io_list = fisBuildIoList(2, in_mf_n);
      free(in_mf_n);
      fis->in_n = 2;
      if ((fis->input = (IO **)calloc(2, sizeof(IO *))) == NULL ) {
        rtmSetErrorStatus(fuzzyPIDController_for_x_axis_M,
                          "Memory Allocation Error");
        return;
      }

      for (i=0; i< 2; i++) {
        fis->input[i] = io_list+i;
      }

      (void) strcpy(fis->input[0]->name,"e");
      fis->input[0]->bound[0] = -3.0;
      fis->input[0]->bound[1] = 3.0;
      fis->input[0]->mf_n = 7;
      (void) strcpy(fis->input[0]->mf[0]->label, "NB");
      (void) strcpy(fis->input[0]->mf[0]->type, "zmf");
      fis->input[0]->mf[0]->params = (real_T *)calloc(2, sizeof(real_T));
      if (fis->input[0]->mf[0]->params == NULL) {
        rtmSetErrorStatus(fuzzyPIDController_for_x_axis_M,
                          "Memory Allocation Error");
        return;
      }

      fis->input[0]->mf[0]->params[0] = -3.0;
      fis->input[0]->mf[0]->params[1] = -1.0;
      (void) strcpy(fis->input[0]->mf[1]->label, "NM");
      (void) strcpy(fis->input[0]->mf[1]->type, "trimf");
      fis->input[0]->mf[1]->params = (real_T *)calloc(3, sizeof(real_T));
      if (fis->input[0]->mf[1]->params == NULL) {
        rtmSetErrorStatus(fuzzyPIDController_for_x_axis_M,
                          "Memory Allocation Error");
        return;
      }

      fis->input[0]->mf[1]->params[0] = -3.0;
      fis->input[0]->mf[1]->params[1] = -2.0;
      fis->input[0]->mf[1]->params[2] = 0.0;
      (void) strcpy(fis->input[0]->mf[2]->label, "NS");
      (void) strcpy(fis->input[0]->mf[2]->type, "trimf");
      fis->input[0]->mf[2]->params = (real_T *)calloc(3, sizeof(real_T));
      if (fis->input[0]->mf[2]->params == NULL) {
        rtmSetErrorStatus(fuzzyPIDController_for_x_axis_M,
                          "Memory Allocation Error");
        return;
      }

      fis->input[0]->mf[2]->params[0] = -3.0;
      fis->input[0]->mf[2]->params[1] = -1.0;
      fis->input[0]->mf[2]->params[2] = 1.0;
      (void) strcpy(fis->input[0]->mf[3]->label, "Z");
      (void) strcpy(fis->input[0]->mf[3]->type, "trimf");
      fis->input[0]->mf[3]->params = (real_T *)calloc(3, sizeof(real_T));
      if (fis->input[0]->mf[3]->params == NULL) {
        rtmSetErrorStatus(fuzzyPIDController_for_x_axis_M,
                          "Memory Allocation Error");
        return;
      }

      fis->input[0]->mf[3]->params[0] = -2.0;
      fis->input[0]->mf[3]->params[1] = 0.0;
      fis->input[0]->mf[3]->params[2] = 2.0;
      (void) strcpy(fis->input[0]->mf[4]->label, "PS");
      (void) strcpy(fis->input[0]->mf[4]->type, "trimf");
      fis->input[0]->mf[4]->params = (real_T *)calloc(3, sizeof(real_T));
      if (fis->input[0]->mf[4]->params == NULL) {
        rtmSetErrorStatus(fuzzyPIDController_for_x_axis_M,
                          "Memory Allocation Error");
        return;
      }

      fis->input[0]->mf[4]->params[0] = -1.0;
      fis->input[0]->mf[4]->params[1] = 1.0;
      fis->input[0]->mf[4]->params[2] = 3.0;
      (void) strcpy(fis->input[0]->mf[5]->label, "PM");
      (void) strcpy(fis->input[0]->mf[5]->type, "trimf");
      fis->input[0]->mf[5]->params = (real_T *)calloc(3, sizeof(real_T));
      if (fis->input[0]->mf[5]->params == NULL) {
        rtmSetErrorStatus(fuzzyPIDController_for_x_axis_M,
                          "Memory Allocation Error");
        return;
      }

      fis->input[0]->mf[5]->params[0] = 0.0;
      fis->input[0]->mf[5]->params[1] = 2.0;
      fis->input[0]->mf[5]->params[2] = 3.0;
      (void) strcpy(fis->input[0]->mf[6]->label, "PB");
      (void) strcpy(fis->input[0]->mf[6]->type, "smf");
      fis->input[0]->mf[6]->params = (real_T *)calloc(2, sizeof(real_T));
      if (fis->input[0]->mf[6]->params == NULL) {
        rtmSetErrorStatus(fuzzyPIDController_for_x_axis_M,
                          "Memory Allocation Error");
        return;
      }

      fis->input[0]->mf[6]->params[0] = 1.0;
      fis->input[0]->mf[6]->params[1] = 3.0;
      (void) strcpy(fis->input[1]->name,"ec");
      fis->input[1]->bound[0] = -3.0;
      fis->input[1]->bound[1] = 3.0;
      fis->input[1]->mf_n = 7;
      (void) strcpy(fis->input[1]->mf[0]->label, "NB");
      (void) strcpy(fis->input[1]->mf[0]->type, "zmf");
      fis->input[1]->mf[0]->params = (real_T *)calloc(2, sizeof(real_T));
      if (fis->input[1]->mf[0]->params == NULL) {
        rtmSetErrorStatus(fuzzyPIDController_for_x_axis_M,
                          "Memory Allocation Error");
        return;
      }

      fis->input[1]->mf[0]->params[0] = -3.0;
      fis->input[1]->mf[0]->params[1] = -1.0;
      (void) strcpy(fis->input[1]->mf[1]->label, "NM");
      (void) strcpy(fis->input[1]->mf[1]->type, "trimf");
      fis->input[1]->mf[1]->params = (real_T *)calloc(3, sizeof(real_T));
      if (fis->input[1]->mf[1]->params == NULL) {
        rtmSetErrorStatus(fuzzyPIDController_for_x_axis_M,
                          "Memory Allocation Error");
        return;
      }

      fis->input[1]->mf[1]->params[0] = -3.0;
      fis->input[1]->mf[1]->params[1] = -2.0;
      fis->input[1]->mf[1]->params[2] = 0.0;
      (void) strcpy(fis->input[1]->mf[2]->label, "NS");
      (void) strcpy(fis->input[1]->mf[2]->type, "trimf");
      fis->input[1]->mf[2]->params = (real_T *)calloc(3, sizeof(real_T));
      if (fis->input[1]->mf[2]->params == NULL) {
        rtmSetErrorStatus(fuzzyPIDController_for_x_axis_M,
                          "Memory Allocation Error");
        return;
      }

      fis->input[1]->mf[2]->params[0] = -3.0;
      fis->input[1]->mf[2]->params[1] = -1.0;
      fis->input[1]->mf[2]->params[2] = 1.0;
      (void) strcpy(fis->input[1]->mf[3]->label, "Z");
      (void) strcpy(fis->input[1]->mf[3]->type, "trimf");
      fis->input[1]->mf[3]->params = (real_T *)calloc(3, sizeof(real_T));
      if (fis->input[1]->mf[3]->params == NULL) {
        rtmSetErrorStatus(fuzzyPIDController_for_x_axis_M,
                          "Memory Allocation Error");
        return;
      }

      fis->input[1]->mf[3]->params[0] = -2.0;
      fis->input[1]->mf[3]->params[1] = 0.0;
      fis->input[1]->mf[3]->params[2] = 2.0;
      (void) strcpy(fis->input[1]->mf[4]->label, "PS");
      (void) strcpy(fis->input[1]->mf[4]->type, "trimf");
      fis->input[1]->mf[4]->params = (real_T *)calloc(3, sizeof(real_T));
      if (fis->input[1]->mf[4]->params == NULL) {
        rtmSetErrorStatus(fuzzyPIDController_for_x_axis_M,
                          "Memory Allocation Error");
        return;
      }

      fis->input[1]->mf[4]->params[0] = -1.0;
      fis->input[1]->mf[4]->params[1] = 1.0;
      fis->input[1]->mf[4]->params[2] = 3.0;
      (void) strcpy(fis->input[1]->mf[5]->label, "PM");
      (void) strcpy(fis->input[1]->mf[5]->type, "trimf");
      fis->input[1]->mf[5]->params = (real_T *)calloc(3, sizeof(real_T));
      if (fis->input[1]->mf[5]->params == NULL) {
        rtmSetErrorStatus(fuzzyPIDController_for_x_axis_M,
                          "Memory Allocation Error");
        return;
      }

      fis->input[1]->mf[5]->params[0] = 0.0;
      fis->input[1]->mf[5]->params[1] = 2.0;
      fis->input[1]->mf[5]->params[2] = 3.0;
      (void) strcpy(fis->input[1]->mf[6]->label, "PB");
      (void) strcpy(fis->input[1]->mf[6]->type, "smf");
      fis->input[1]->mf[6]->params = (real_T *)calloc(2, sizeof(real_T));
      if (fis->input[1]->mf[6]->params == NULL) {
        rtmSetErrorStatus(fuzzyPIDController_for_x_axis_M,
                          "Memory Allocation Error");
        return;
      }

      fis->input[1]->mf[6]->params[0] = 1.0;
      fis->input[1]->mf[6]->params[1] = 3.0;
    }

    {
      IO *io_list;
      int_T *out_mf_n;
      int_T i;
      if ((out_mf_n = (int_T *)calloc(3, sizeof(int_T))) == NULL) {
        rtmSetErrorStatus(fuzzyPIDController_for_x_axis_M,
                          "Memory Allocation Error");
        return;
      }

      out_mf_n[0] = 7;
      out_mf_n[1] = 7;
      out_mf_n[2] = 7;
      io_list = fisBuildIoList(3, out_mf_n);
      free(out_mf_n);
      fis->out_n = 3;
      if ((fis->output = (IO **)calloc(3, sizeof(IO *))) == NULL ) {
        rtmSetErrorStatus(fuzzyPIDController_for_x_axis_M,
                          "Memory Allocation Error");
        return;
      }

      for (i=0; i< 3; i++) {
        fis->output[i] = io_list+i;
      }

      (void) strcpy(fis->output[0]->name,"KP");
      fis->output[0]->bound[0] = -0.3;
      fis->output[0]->bound[1] = 0.3;
      fis->output[0]->mf_n = 7;
      (void) strcpy(fis->output[0]->mf[0]->label, "NB");
      (void) strcpy(fis->output[0]->mf[0]->type, "zmf");
      fis->output[0]->mf[0]->value_array = (real_T *)calloc(101, sizeof(real_T));
      if (fis->output[0]->mf[0]->value_array == NULL) {
        rtmSetErrorStatus(fuzzyPIDController_for_x_axis_M,
                          "Memory Allocation Error");
        return;
      }

      fis->output[0]->mf[0]->params = (real_T *)calloc(2, sizeof(real_T));
      if (fis->output[0]->mf[0]->params == NULL) {
        rtmSetErrorStatus(fuzzyPIDController_for_x_axis_M,
                          "Memory Allocation Error");
        return;
      }

      fis->output[0]->mf[0]->params[0] = -0.3;
      fis->output[0]->mf[0]->params[1] = -0.1;
      (void) strcpy(fis->output[0]->mf[1]->label, "NM");
      (void) strcpy(fis->output[0]->mf[1]->type, "trimf");
      fis->output[0]->mf[1]->value_array = (real_T *)calloc(101, sizeof(real_T));
      if (fis->output[0]->mf[1]->value_array == NULL) {
        rtmSetErrorStatus(fuzzyPIDController_for_x_axis_M,
                          "Memory Allocation Error");
        return;
      }

      fis->output[0]->mf[1]->params = (real_T *)calloc(3, sizeof(real_T));
      if (fis->output[0]->mf[1]->params == NULL) {
        rtmSetErrorStatus(fuzzyPIDController_for_x_axis_M,
                          "Memory Allocation Error");
        return;
      }

      fis->output[0]->mf[1]->params[0] = -0.3;
      fis->output[0]->mf[1]->params[1] = -0.2;
      fis->output[0]->mf[1]->params[2] = 0.0;
      (void) strcpy(fis->output[0]->mf[2]->label, "NS");
      (void) strcpy(fis->output[0]->mf[2]->type, "trimf");
      fis->output[0]->mf[2]->value_array = (real_T *)calloc(101, sizeof(real_T));
      if (fis->output[0]->mf[2]->value_array == NULL) {
        rtmSetErrorStatus(fuzzyPIDController_for_x_axis_M,
                          "Memory Allocation Error");
        return;
      }

      fis->output[0]->mf[2]->params = (real_T *)calloc(3, sizeof(real_T));
      if (fis->output[0]->mf[2]->params == NULL) {
        rtmSetErrorStatus(fuzzyPIDController_for_x_axis_M,
                          "Memory Allocation Error");
        return;
      }

      fis->output[0]->mf[2]->params[0] = -0.3;
      fis->output[0]->mf[2]->params[1] = -0.1;
      fis->output[0]->mf[2]->params[2] = 0.1;
      (void) strcpy(fis->output[0]->mf[3]->label, "Z");
      (void) strcpy(fis->output[0]->mf[3]->type, "trimf");
      fis->output[0]->mf[3]->value_array = (real_T *)calloc(101, sizeof(real_T));
      if (fis->output[0]->mf[3]->value_array == NULL) {
        rtmSetErrorStatus(fuzzyPIDController_for_x_axis_M,
                          "Memory Allocation Error");
        return;
      }

      fis->output[0]->mf[3]->params = (real_T *)calloc(3, sizeof(real_T));
      if (fis->output[0]->mf[3]->params == NULL) {
        rtmSetErrorStatus(fuzzyPIDController_for_x_axis_M,
                          "Memory Allocation Error");
        return;
      }

      fis->output[0]->mf[3]->params[0] = -0.2;
      fis->output[0]->mf[3]->params[1] = 0.0;
      fis->output[0]->mf[3]->params[2] = 0.2;
      (void) strcpy(fis->output[0]->mf[4]->label, "PS");
      (void) strcpy(fis->output[0]->mf[4]->type, "trimf");
      fis->output[0]->mf[4]->value_array = (real_T *)calloc(101, sizeof(real_T));
      if (fis->output[0]->mf[4]->value_array == NULL) {
        rtmSetErrorStatus(fuzzyPIDController_for_x_axis_M,
                          "Memory Allocation Error");
        return;
      }

      fis->output[0]->mf[4]->params = (real_T *)calloc(3, sizeof(real_T));
      if (fis->output[0]->mf[4]->params == NULL) {
        rtmSetErrorStatus(fuzzyPIDController_for_x_axis_M,
                          "Memory Allocation Error");
        return;
      }

      fis->output[0]->mf[4]->params[0] = -0.1;
      fis->output[0]->mf[4]->params[1] = 0.1;
      fis->output[0]->mf[4]->params[2] = 0.3;
      (void) strcpy(fis->output[0]->mf[5]->label, "PM");
      (void) strcpy(fis->output[0]->mf[5]->type, "trimf");
      fis->output[0]->mf[5]->value_array = (real_T *)calloc(101, sizeof(real_T));
      if (fis->output[0]->mf[5]->value_array == NULL) {
        rtmSetErrorStatus(fuzzyPIDController_for_x_axis_M,
                          "Memory Allocation Error");
        return;
      }

      fis->output[0]->mf[5]->params = (real_T *)calloc(3, sizeof(real_T));
      if (fis->output[0]->mf[5]->params == NULL) {
        rtmSetErrorStatus(fuzzyPIDController_for_x_axis_M,
                          "Memory Allocation Error");
        return;
      }

      fis->output[0]->mf[5]->params[0] = 0.0;
      fis->output[0]->mf[5]->params[1] = 0.2;
      fis->output[0]->mf[5]->params[2] = 0.3;
      (void) strcpy(fis->output[0]->mf[6]->label, "PB");
      (void) strcpy(fis->output[0]->mf[6]->type, "smf");
      fis->output[0]->mf[6]->value_array = (real_T *)calloc(101, sizeof(real_T));
      if (fis->output[0]->mf[6]->value_array == NULL) {
        rtmSetErrorStatus(fuzzyPIDController_for_x_axis_M,
                          "Memory Allocation Error");
        return;
      }

      fis->output[0]->mf[6]->params = (real_T *)calloc(2, sizeof(real_T));
      if (fis->output[0]->mf[6]->params == NULL) {
        rtmSetErrorStatus(fuzzyPIDController_for_x_axis_M,
                          "Memory Allocation Error");
        return;
      }

      fis->output[0]->mf[6]->params[0] = 0.1;
      fis->output[0]->mf[6]->params[1] = 0.3;
      (void) strcpy(fis->output[1]->name,"KI");
      fis->output[1]->bound[0] = -0.3;
      fis->output[1]->bound[1] = 0.3;
      fis->output[1]->mf_n = 7;
      (void) strcpy(fis->output[1]->mf[0]->label, "NB");
      (void) strcpy(fis->output[1]->mf[0]->type, "zmf");
      fis->output[1]->mf[0]->value_array = (real_T *)calloc(101, sizeof(real_T));
      if (fis->output[1]->mf[0]->value_array == NULL) {
        rtmSetErrorStatus(fuzzyPIDController_for_x_axis_M,
                          "Memory Allocation Error");
        return;
      }

      fis->output[1]->mf[0]->params = (real_T *)calloc(2, sizeof(real_T));
      if (fis->output[1]->mf[0]->params == NULL) {
        rtmSetErrorStatus(fuzzyPIDController_for_x_axis_M,
                          "Memory Allocation Error");
        return;
      }

      fis->output[1]->mf[0]->params[0] = -0.3;
      fis->output[1]->mf[0]->params[1] = -0.1;
      (void) strcpy(fis->output[1]->mf[1]->label, "NM");
      (void) strcpy(fis->output[1]->mf[1]->type, "trimf");
      fis->output[1]->mf[1]->value_array = (real_T *)calloc(101, sizeof(real_T));
      if (fis->output[1]->mf[1]->value_array == NULL) {
        rtmSetErrorStatus(fuzzyPIDController_for_x_axis_M,
                          "Memory Allocation Error");
        return;
      }

      fis->output[1]->mf[1]->params = (real_T *)calloc(3, sizeof(real_T));
      if (fis->output[1]->mf[1]->params == NULL) {
        rtmSetErrorStatus(fuzzyPIDController_for_x_axis_M,
                          "Memory Allocation Error");
        return;
      }

      fis->output[1]->mf[1]->params[0] = -0.3;
      fis->output[1]->mf[1]->params[1] = -0.2;
      fis->output[1]->mf[1]->params[2] = 0.0;
      (void) strcpy(fis->output[1]->mf[2]->label, "NS");
      (void) strcpy(fis->output[1]->mf[2]->type, "trimf");
      fis->output[1]->mf[2]->value_array = (real_T *)calloc(101, sizeof(real_T));
      if (fis->output[1]->mf[2]->value_array == NULL) {
        rtmSetErrorStatus(fuzzyPIDController_for_x_axis_M,
                          "Memory Allocation Error");
        return;
      }

      fis->output[1]->mf[2]->params = (real_T *)calloc(3, sizeof(real_T));
      if (fis->output[1]->mf[2]->params == NULL) {
        rtmSetErrorStatus(fuzzyPIDController_for_x_axis_M,
                          "Memory Allocation Error");
        return;
      }

      fis->output[1]->mf[2]->params[0] = -0.3;
      fis->output[1]->mf[2]->params[1] = -0.1;
      fis->output[1]->mf[2]->params[2] = 0.1;
      (void) strcpy(fis->output[1]->mf[3]->label, "Z");
      (void) strcpy(fis->output[1]->mf[3]->type, "trimf");
      fis->output[1]->mf[3]->value_array = (real_T *)calloc(101, sizeof(real_T));
      if (fis->output[1]->mf[3]->value_array == NULL) {
        rtmSetErrorStatus(fuzzyPIDController_for_x_axis_M,
                          "Memory Allocation Error");
        return;
      }

      fis->output[1]->mf[3]->params = (real_T *)calloc(3, sizeof(real_T));
      if (fis->output[1]->mf[3]->params == NULL) {
        rtmSetErrorStatus(fuzzyPIDController_for_x_axis_M,
                          "Memory Allocation Error");
        return;
      }

      fis->output[1]->mf[3]->params[0] = -0.2;
      fis->output[1]->mf[3]->params[1] = 0.0;
      fis->output[1]->mf[3]->params[2] = 0.2;
      (void) strcpy(fis->output[1]->mf[4]->label, "PS");
      (void) strcpy(fis->output[1]->mf[4]->type, "trimf");
      fis->output[1]->mf[4]->value_array = (real_T *)calloc(101, sizeof(real_T));
      if (fis->output[1]->mf[4]->value_array == NULL) {
        rtmSetErrorStatus(fuzzyPIDController_for_x_axis_M,
                          "Memory Allocation Error");
        return;
      }

      fis->output[1]->mf[4]->params = (real_T *)calloc(3, sizeof(real_T));
      if (fis->output[1]->mf[4]->params == NULL) {
        rtmSetErrorStatus(fuzzyPIDController_for_x_axis_M,
                          "Memory Allocation Error");
        return;
      }

      fis->output[1]->mf[4]->params[0] = -0.1;
      fis->output[1]->mf[4]->params[1] = 0.1;
      fis->output[1]->mf[4]->params[2] = 0.3;
      (void) strcpy(fis->output[1]->mf[5]->label, "PM");
      (void) strcpy(fis->output[1]->mf[5]->type, "trimf");
      fis->output[1]->mf[5]->value_array = (real_T *)calloc(101, sizeof(real_T));
      if (fis->output[1]->mf[5]->value_array == NULL) {
        rtmSetErrorStatus(fuzzyPIDController_for_x_axis_M,
                          "Memory Allocation Error");
        return;
      }

      fis->output[1]->mf[5]->params = (real_T *)calloc(3, sizeof(real_T));
      if (fis->output[1]->mf[5]->params == NULL) {
        rtmSetErrorStatus(fuzzyPIDController_for_x_axis_M,
                          "Memory Allocation Error");
        return;
      }

      fis->output[1]->mf[5]->params[0] = 0.0;
      fis->output[1]->mf[5]->params[1] = 0.2;
      fis->output[1]->mf[5]->params[2] = 0.3;
      (void) strcpy(fis->output[1]->mf[6]->label, "PB");
      (void) strcpy(fis->output[1]->mf[6]->type, "smf");
      fis->output[1]->mf[6]->value_array = (real_T *)calloc(101, sizeof(real_T));
      if (fis->output[1]->mf[6]->value_array == NULL) {
        rtmSetErrorStatus(fuzzyPIDController_for_x_axis_M,
                          "Memory Allocation Error");
        return;
      }

      fis->output[1]->mf[6]->params = (real_T *)calloc(2, sizeof(real_T));
      if (fis->output[1]->mf[6]->params == NULL) {
        rtmSetErrorStatus(fuzzyPIDController_for_x_axis_M,
                          "Memory Allocation Error");
        return;
      }

      fis->output[1]->mf[6]->params[0] = 0.1;
      fis->output[1]->mf[6]->params[1] = 0.3;
      (void) strcpy(fis->output[2]->name,"KD");
      fis->output[2]->bound[0] = -0.3;
      fis->output[2]->bound[1] = 0.3;
      fis->output[2]->mf_n = 7;
      (void) strcpy(fis->output[2]->mf[0]->label, "NB");
      (void) strcpy(fis->output[2]->mf[0]->type, "zmf");
      fis->output[2]->mf[0]->value_array = (real_T *)calloc(101, sizeof(real_T));
      if (fis->output[2]->mf[0]->value_array == NULL) {
        rtmSetErrorStatus(fuzzyPIDController_for_x_axis_M,
                          "Memory Allocation Error");
        return;
      }

      fis->output[2]->mf[0]->params = (real_T *)calloc(2, sizeof(real_T));
      if (fis->output[2]->mf[0]->params == NULL) {
        rtmSetErrorStatus(fuzzyPIDController_for_x_axis_M,
                          "Memory Allocation Error");
        return;
      }

      fis->output[2]->mf[0]->params[0] = -0.3;
      fis->output[2]->mf[0]->params[1] = -0.1;
      (void) strcpy(fis->output[2]->mf[1]->label, "NM");
      (void) strcpy(fis->output[2]->mf[1]->type, "trimf");
      fis->output[2]->mf[1]->value_array = (real_T *)calloc(101, sizeof(real_T));
      if (fis->output[2]->mf[1]->value_array == NULL) {
        rtmSetErrorStatus(fuzzyPIDController_for_x_axis_M,
                          "Memory Allocation Error");
        return;
      }

      fis->output[2]->mf[1]->params = (real_T *)calloc(3, sizeof(real_T));
      if (fis->output[2]->mf[1]->params == NULL) {
        rtmSetErrorStatus(fuzzyPIDController_for_x_axis_M,
                          "Memory Allocation Error");
        return;
      }

      fis->output[2]->mf[1]->params[0] = -0.3;
      fis->output[2]->mf[1]->params[1] = -0.2;
      fis->output[2]->mf[1]->params[2] = 0.0;
      (void) strcpy(fis->output[2]->mf[2]->label, "NS");
      (void) strcpy(fis->output[2]->mf[2]->type, "trimf");
      fis->output[2]->mf[2]->value_array = (real_T *)calloc(101, sizeof(real_T));
      if (fis->output[2]->mf[2]->value_array == NULL) {
        rtmSetErrorStatus(fuzzyPIDController_for_x_axis_M,
                          "Memory Allocation Error");
        return;
      }

      fis->output[2]->mf[2]->params = (real_T *)calloc(3, sizeof(real_T));
      if (fis->output[2]->mf[2]->params == NULL) {
        rtmSetErrorStatus(fuzzyPIDController_for_x_axis_M,
                          "Memory Allocation Error");
        return;
      }

      fis->output[2]->mf[2]->params[0] = -0.3;
      fis->output[2]->mf[2]->params[1] = -0.1;
      fis->output[2]->mf[2]->params[2] = 0.1;
      (void) strcpy(fis->output[2]->mf[3]->label, "Z");
      (void) strcpy(fis->output[2]->mf[3]->type, "trimf");
      fis->output[2]->mf[3]->value_array = (real_T *)calloc(101, sizeof(real_T));
      if (fis->output[2]->mf[3]->value_array == NULL) {
        rtmSetErrorStatus(fuzzyPIDController_for_x_axis_M,
                          "Memory Allocation Error");
        return;
      }

      fis->output[2]->mf[3]->params = (real_T *)calloc(3, sizeof(real_T));
      if (fis->output[2]->mf[3]->params == NULL) {
        rtmSetErrorStatus(fuzzyPIDController_for_x_axis_M,
                          "Memory Allocation Error");
        return;
      }

      fis->output[2]->mf[3]->params[0] = -0.2;
      fis->output[2]->mf[3]->params[1] = 0.0;
      fis->output[2]->mf[3]->params[2] = 0.2;
      (void) strcpy(fis->output[2]->mf[4]->label, "PS");
      (void) strcpy(fis->output[2]->mf[4]->type, "trimf");
      fis->output[2]->mf[4]->value_array = (real_T *)calloc(101, sizeof(real_T));
      if (fis->output[2]->mf[4]->value_array == NULL) {
        rtmSetErrorStatus(fuzzyPIDController_for_x_axis_M,
                          "Memory Allocation Error");
        return;
      }

      fis->output[2]->mf[4]->params = (real_T *)calloc(3, sizeof(real_T));
      if (fis->output[2]->mf[4]->params == NULL) {
        rtmSetErrorStatus(fuzzyPIDController_for_x_axis_M,
                          "Memory Allocation Error");
        return;
      }

      fis->output[2]->mf[4]->params[0] = -0.1;
      fis->output[2]->mf[4]->params[1] = 0.1;
      fis->output[2]->mf[4]->params[2] = 0.3;
      (void) strcpy(fis->output[2]->mf[5]->label, "PM");
      (void) strcpy(fis->output[2]->mf[5]->type, "trimf");
      fis->output[2]->mf[5]->value_array = (real_T *)calloc(101, sizeof(real_T));
      if (fis->output[2]->mf[5]->value_array == NULL) {
        rtmSetErrorStatus(fuzzyPIDController_for_x_axis_M,
                          "Memory Allocation Error");
        return;
      }

      fis->output[2]->mf[5]->params = (real_T *)calloc(3, sizeof(real_T));
      if (fis->output[2]->mf[5]->params == NULL) {
        rtmSetErrorStatus(fuzzyPIDController_for_x_axis_M,
                          "Memory Allocation Error");
        return;
      }

      fis->output[2]->mf[5]->params[0] = 0.0;
      fis->output[2]->mf[5]->params[1] = 0.2;
      fis->output[2]->mf[5]->params[2] = 0.3;
      (void) strcpy(fis->output[2]->mf[6]->label, "PB");
      (void) strcpy(fis->output[2]->mf[6]->type, "smf");
      fis->output[2]->mf[6]->value_array = (real_T *)calloc(101, sizeof(real_T));
      if (fis->output[2]->mf[6]->value_array == NULL) {
        rtmSetErrorStatus(fuzzyPIDController_for_x_axis_M,
                          "Memory Allocation Error");
        return;
      }

      fis->output[2]->mf[6]->params = (real_T *)calloc(2, sizeof(real_T));
      if (fis->output[2]->mf[6]->params == NULL) {
        rtmSetErrorStatus(fuzzyPIDController_for_x_axis_M,
                          "Memory Allocation Error");
        return;
      }

      fis->output[2]->mf[6]->params[0] = 0.1;
      fis->output[2]->mf[6]->params[1] = 0.3;
      fis->rule_n = 49;
      fis->rule_list = (int_T **) fisCreateMatrix(49, fis->in_n + fis->out_n,
        sizeof(int_T));
      if ((fis->rule_weight = (real_T *)calloc(49,sizeof(real_T))) == NULL ) {
        rtmSetErrorStatus(fuzzyPIDController_for_x_axis_M,
                          "Memory Allocation Error");
        return;
      }

      if ((fis->and_or = (int_T *)calloc(49,sizeof(int_T))) == NULL ) {
        rtmSetErrorStatus(fuzzyPIDController_for_x_axis_M,
                          "Memory Allocation Error");
        return;
      }

      fis->rule_list[0][0] = 1;
      fis->rule_list[0][1] = 1;
      fis->rule_list[0][2] = 7;
      fis->rule_list[0][3] = 1;
      fis->rule_list[0][4] = 5;
      fis->rule_weight[0] = 1.0;
      fis->and_or[0] = 1;
      fis->rule_list[1][0] = 1;
      fis->rule_list[1][1] = 2;
      fis->rule_list[1][2] = 7;
      fis->rule_list[1][3] = 1;
      fis->rule_list[1][4] = 2;
      fis->rule_weight[1] = 1.0;
      fis->and_or[1] = 1;
      fis->rule_list[2][0] = 1;
      fis->rule_list[2][1] = 3;
      fis->rule_list[2][2] = 7;
      fis->rule_list[2][3] = 1;
      fis->rule_list[2][4] = 1;
      fis->rule_weight[2] = 1.0;
      fis->and_or[2] = 1;
      fis->rule_list[3][0] = 1;
      fis->rule_list[3][1] = 4;
      fis->rule_list[3][2] = 7;
      fis->rule_list[3][3] = 2;
      fis->rule_list[3][4] = 1;
      fis->rule_weight[3] = 1.0;
      fis->and_or[3] = 1;
      fis->rule_list[4][0] = 1;
      fis->rule_list[4][1] = 5;
      fis->rule_list[4][2] = 6;
      fis->rule_list[4][3] = 3;
      fis->rule_list[4][4] = 1;
      fis->rule_weight[4] = 1.0;
      fis->and_or[4] = 1;
      fis->rule_list[5][0] = 1;
      fis->rule_list[5][1] = 6;
      fis->rule_list[5][2] = 5;
      fis->rule_list[5][3] = 4;
      fis->rule_list[5][4] = 2;
      fis->rule_weight[5] = 1.0;
      fis->and_or[5] = 1;
      fis->rule_list[6][0] = 1;
      fis->rule_list[6][1] = 7;
      fis->rule_list[6][2] = 4;
      fis->rule_list[6][3] = 4;
      fis->rule_list[6][4] = 5;
      fis->rule_weight[6] = 1.0;
      fis->and_or[6] = 1;
      fis->rule_list[7][0] = 2;
      fis->rule_list[7][1] = 1;
      fis->rule_list[7][2] = 7;
      fis->rule_list[7][3] = 1;
      fis->rule_list[7][4] = 5;
      fis->rule_weight[7] = 1.0;
      fis->and_or[7] = 1;
      fis->rule_list[8][0] = 2;
      fis->rule_list[8][1] = 2;
      fis->rule_list[8][2] = 7;
      fis->rule_list[8][3] = 1;
      fis->rule_list[8][4] = 3;
      fis->rule_weight[8] = 1.0;
      fis->and_or[8] = 1;
      fis->rule_list[9][0] = 2;
      fis->rule_list[9][1] = 3;
      fis->rule_list[9][2] = 7;
      fis->rule_list[9][3] = 2;
      fis->rule_list[9][4] = 1;
      fis->rule_weight[9] = 1.0;
      fis->and_or[9] = 1;
      fis->rule_list[10][0] = 2;
      fis->rule_list[10][1] = 4;
      fis->rule_list[10][2] = 7;
      fis->rule_list[10][3] = 3;
      fis->rule_list[10][4] = 2;
      fis->rule_weight[10] = 1.0;
      fis->and_or[10] = 1;
      fis->rule_list[11][0] = 2;
      fis->rule_list[11][1] = 5;
      fis->rule_list[11][2] = 6;
      fis->rule_list[11][3] = 3;
      fis->rule_list[11][4] = 2;
      fis->rule_weight[11] = 1.0;
      fis->and_or[11] = 1;
      fis->rule_list[12][0] = 2;
      fis->rule_list[12][1] = 6;
      fis->rule_list[12][2] = 4;
      fis->rule_list[12][3] = 4;
      fis->rule_list[12][4] = 3;
      fis->rule_weight[12] = 1.0;
      fis->and_or[12] = 1;
      fis->rule_list[13][0] = 2;
      fis->rule_list[13][1] = 7;
      fis->rule_list[13][2] = 4;
      fis->rule_list[13][3] = 4;
      fis->rule_list[13][4] = 5;
      fis->rule_weight[13] = 1.0;
      fis->and_or[13] = 1;
      fis->rule_list[14][0] = 3;
      fis->rule_list[14][1] = 1;
      fis->rule_list[14][2] = 6;
      fis->rule_list[14][3] = 1;
      fis->rule_list[14][4] = 4;
      fis->rule_weight[14] = 1.0;
      fis->and_or[14] = 1;
      fis->rule_list[15][0] = 3;
      fis->rule_list[15][1] = 2;
      fis->rule_list[15][2] = 6;
      fis->rule_list[15][3] = 2;
      fis->rule_list[15][4] = 3;
      fis->rule_weight[15] = 1.0;
      fis->and_or[15] = 1;
      fis->rule_list[16][0] = 3;
      fis->rule_list[16][1] = 3;
      fis->rule_list[16][2] = 6;
      fis->rule_list[16][3] = 3;
      fis->rule_list[16][4] = 2;
      fis->rule_weight[16] = 1.0;
      fis->and_or[16] = 1;
      fis->rule_list[17][0] = 3;
      fis->rule_list[17][1] = 4;
      fis->rule_list[17][2] = 6;
      fis->rule_list[17][3] = 3;
      fis->rule_list[17][4] = 2;
      fis->rule_weight[17] = 1.0;
      fis->and_or[17] = 1;
      fis->rule_list[18][0] = 3;
      fis->rule_list[18][1] = 5;
      fis->rule_list[18][2] = 4;
      fis->rule_list[18][3] = 4;
      fis->rule_list[18][4] = 3;
      fis->rule_weight[18] = 1.0;
      fis->and_or[18] = 1;
      fis->rule_list[19][0] = 3;
      fis->rule_list[19][1] = 6;
      fis->rule_list[19][2] = 3;
      fis->rule_list[19][3] = 5;
      fis->rule_list[19][4] = 3;
      fis->rule_weight[19] = 1.0;
      fis->and_or[19] = 1;
      fis->rule_list[20][0] = 3;
      fis->rule_list[20][1] = 7;
      fis->rule_list[20][2] = 3;
      fis->rule_list[20][3] = 5;
      fis->rule_list[20][4] = 4;
      fis->rule_weight[20] = 1.0;
      fis->and_or[20] = 1;
      fis->rule_list[21][0] = 4;
      fis->rule_list[21][1] = 1;
      fis->rule_list[21][2] = 6;
      fis->rule_list[21][3] = 2;
      fis->rule_list[21][4] = 4;
      fis->rule_weight[21] = 1.0;
      fis->and_or[21] = 1;
      fis->rule_list[22][0] = 4;
      fis->rule_list[22][1] = 2;
      fis->rule_list[22][2] = 6;
      fis->rule_list[22][3] = 3;
      fis->rule_list[22][4] = 3;
      fis->rule_weight[22] = 1.0;
      fis->and_or[22] = 1;
      fis->rule_list[23][0] = 4;
      fis->rule_list[23][1] = 3;
      fis->rule_list[23][2] = 5;
      fis->rule_list[23][3] = 3;
      fis->rule_list[23][4] = 3;
      fis->rule_weight[23] = 1.0;
      fis->and_or[23] = 1;
      fis->rule_list[24][0] = 4;
      fis->rule_list[24][1] = 4;
      fis->rule_list[24][2] = 4;
      fis->rule_list[24][3] = 4;
      fis->rule_list[24][4] = 3;
      fis->rule_weight[24] = 1.0;
      fis->and_or[24] = 1;
      fis->rule_list[25][0] = 4;
      fis->rule_list[25][1] = 5;
      fis->rule_list[25][2] = 3;
      fis->rule_list[25][3] = 5;
      fis->rule_list[25][4] = 3;
      fis->rule_weight[25] = 1.0;
      fis->and_or[25] = 1;
      fis->rule_list[26][0] = 4;
      fis->rule_list[26][1] = 6;
      fis->rule_list[26][2] = 3;
      fis->rule_list[26][3] = 6;
      fis->rule_list[26][4] = 3;
      fis->rule_weight[26] = 1.0;
      fis->and_or[26] = 1;
      fis->rule_list[27][0] = 4;
      fis->rule_list[27][1] = 7;
      fis->rule_list[27][2] = 2;
      fis->rule_list[27][3] = 6;
      fis->rule_list[27][4] = 4;
      fis->rule_weight[27] = 1.0;
      fis->and_or[27] = 1;
      fis->rule_list[28][0] = 5;
      fis->rule_list[28][1] = 1;
      fis->rule_list[28][2] = 5;
      fis->rule_list[28][3] = 3;
      fis->rule_list[28][4] = 4;
      fis->rule_weight[28] = 1.0;
      fis->and_or[28] = 1;
      fis->rule_list[29][0] = 5;
      fis->rule_list[29][1] = 2;
      fis->rule_list[29][2] = 5;
      fis->rule_list[29][3] = 3;
      fis->rule_list[29][4] = 3;
      fis->rule_weight[29] = 1.0;
      fis->and_or[29] = 1;
      fis->rule_list[30][0] = 5;
      fis->rule_list[30][1] = 3;
      fis->rule_list[30][2] = 4;
      fis->rule_list[30][3] = 4;
      fis->rule_list[30][4] = 4;
      fis->rule_weight[30] = 1.0;
      fis->and_or[30] = 1;
      fis->rule_list[31][0] = 5;
      fis->rule_list[31][1] = 4;
      fis->rule_list[31][2] = 3;
      fis->rule_list[31][3] = 5;
      fis->rule_list[31][4] = 4;
      fis->rule_weight[31] = 1.0;
      fis->and_or[31] = 1;
      fis->rule_list[32][0] = 5;
      fis->rule_list[32][1] = 5;
      fis->rule_list[32][2] = 2;
      fis->rule_list[32][3] = 5;
      fis->rule_list[32][4] = 4;
      fis->rule_weight[32] = 1.0;
      fis->and_or[32] = 1;
      fis->rule_list[33][0] = 5;
      fis->rule_list[33][1] = 6;
      fis->rule_list[33][2] = 2;
      fis->rule_list[33][3] = 6;
      fis->rule_list[33][4] = 4;
      fis->rule_weight[33] = 1.0;
      fis->and_or[33] = 1;
      fis->rule_list[34][0] = 5;
      fis->rule_list[34][1] = 7;
      fis->rule_list[34][2] = 2;
      fis->rule_list[34][3] = 7;
      fis->rule_list[34][4] = 5;
      fis->rule_weight[34] = 1.0;
      fis->and_or[34] = 1;
      fis->rule_list[35][0] = 6;
      fis->rule_list[35][1] = 1;
      fis->rule_list[35][2] = 5;
      fis->rule_list[35][3] = 4;
      fis->rule_list[35][4] = 7;
      fis->rule_weight[35] = 1.0;
      fis->and_or[35] = 1;
      fis->rule_list[36][0] = 6;
      fis->rule_list[36][1] = 2;
      fis->rule_list[36][2] = 4;
      fis->rule_list[36][3] = 4;
      fis->rule_list[36][4] = 3;
      fis->rule_weight[36] = 1.0;
      fis->and_or[36] = 1;
      fis->rule_list[37][0] = 6;
      fis->rule_list[37][1] = 3;
      fis->rule_list[37][2] = 3;
      fis->rule_list[37][3] = 5;
      fis->rule_list[37][4] = 5;
      fis->rule_weight[37] = 1.0;
      fis->and_or[37] = 1;
      fis->rule_list[38][0] = 6;
      fis->rule_list[38][1] = 4;
      fis->rule_list[38][2] = 2;
      fis->rule_list[38][3] = 5;
      fis->rule_list[38][4] = 5;
      fis->rule_weight[38] = 1.0;
      fis->and_or[38] = 1;
      fis->rule_list[39][0] = 6;
      fis->rule_list[39][1] = 5;
      fis->rule_list[39][2] = 2;
      fis->rule_list[39][3] = 6;
      fis->rule_list[39][4] = 5;
      fis->rule_weight[39] = 1.0;
      fis->and_or[39] = 1;
      fis->rule_list[40][0] = 6;
      fis->rule_list[40][1] = 6;
      fis->rule_list[40][2] = 2;
      fis->rule_list[40][3] = 7;
      fis->rule_list[40][4] = 5;
      fis->rule_weight[40] = 1.0;
      fis->and_or[40] = 1;
      fis->rule_list[41][0] = 6;
      fis->rule_list[41][1] = 7;
      fis->rule_list[41][2] = 1;
      fis->rule_list[41][3] = 7;
      fis->rule_list[41][4] = 7;
      fis->rule_weight[41] = 1.0;
      fis->and_or[41] = 1;
      fis->rule_list[42][0] = 7;
      fis->rule_list[42][1] = 1;
      fis->rule_list[42][2] = 4;
      fis->rule_list[42][3] = 4;
      fis->rule_list[42][4] = 7;
      fis->rule_weight[42] = 1.0;
      fis->and_or[42] = 1;
      fis->rule_list[43][0] = 7;
      fis->rule_list[43][1] = 2;
      fis->rule_list[43][2] = 4;
      fis->rule_list[43][3] = 4;
      fis->rule_list[43][4] = 6;
      fis->rule_weight[43] = 1.0;
      fis->and_or[43] = 1;
      fis->rule_list[44][0] = 7;
      fis->rule_list[44][1] = 3;
      fis->rule_list[44][2] = 2;
      fis->rule_list[44][3] = 5;
      fis->rule_list[44][4] = 6;
      fis->rule_weight[44] = 1.0;
      fis->and_or[44] = 1;
      fis->rule_list[45][0] = 7;
      fis->rule_list[45][1] = 4;
      fis->rule_list[45][2] = 2;
      fis->rule_list[45][3] = 6;
      fis->rule_list[45][4] = 6;
      fis->rule_weight[45] = 1.0;
      fis->and_or[45] = 1;
      fis->rule_list[46][0] = 7;
      fis->rule_list[46][1] = 5;
      fis->rule_list[46][2] = 2;
      fis->rule_list[46][3] = 7;
      fis->rule_list[46][4] = 5;
      fis->rule_weight[46] = 1.0;
      fis->and_or[46] = 1;
      fis->rule_list[47][0] = 7;
      fis->rule_list[47][1] = 6;
      fis->rule_list[47][2] = 1;
      fis->rule_list[47][3] = 7;
      fis->rule_list[47][4] = 5;
      fis->rule_weight[47] = 1.0;
      fis->and_or[47] = 1;
      fis->rule_list[48][0] = 7;
      fis->rule_list[48][1] = 7;
      fis->rule_list[48][2] = 1;
      fis->rule_list[48][3] = 7;
      fis->rule_list[48][4] = 7;
      fis->rule_weight[48] = 1.0;
      fis->and_or[48] = 1;
      fisAssignMfPointer(fis);
      fisAssignFunctionPointer(fis);
      if ((fis->firing_strength = (real_T *)calloc(49, sizeof(real_T))) == NULL )
      {
        rtmSetErrorStatus(fuzzyPIDController_for_x_axis_M,
                          "Memory Allocation Error");
        return;
      }

      if ((fis->rule_output = (real_T *)calloc(49, sizeof(real_T))) == NULL ) {
        rtmSetErrorStatus(fuzzyPIDController_for_x_axis_M,
                          "Memory Allocation Error");
        return;
      }

      if ((fis->mfs_of_rule = (real_T *)calloc(2, sizeof(real_T))) == NULL ) {
        rtmSetErrorStatus(fuzzyPIDController_for_x_axis_M,
                          "Memory Allocation Error");
        return;
      }

      fis->BigOutMfMatrix = (real_T *) calloc(4949, sizeof(real_T));
      if (fis->BigOutMfMatrix == NULL) {
        rtmSetErrorStatus(fuzzyPIDController_for_x_axis_M,
                          "Memory Allocation Error");
        return;
      }

      fis->BigWeightMatrix = (real_T *) calloc(4949, sizeof(real_T));
      if (fis->BigWeightMatrix == NULL) {
        rtmSetErrorStatus(fuzzyPIDController_for_x_axis_M,
                          "Memory Allocation Error");
        return;
      }

      for (i = 0; i < 3; i++) {
        fisComputeOutputMfValueArray(fis, 101);
      }

      fisCheckDataStructure(fis);
    }

    fis->next = NULL;
    fuzzyPIDController_for_x_axis_DW.FISSfunction_PWORK.FISPointer = fis;
  }

  /* InitializeConditions for UnitDelay: '<S2>/UD' */
  fuzzyPIDController_for_x_axis_DW.UD_DSTATE =
    fuzzyPIDController_for_x_axis_P.DiscreteDerivative1_ICPrevScale;

  /* InitializeConditions for DiscreteIntegrator: '<Root>/Discrete-Time Integrator' */
  fuzzyPIDController_for_x_axis_DW.DiscreteTimeIntegrator_DSTATE =
    fuzzyPIDController_for_x_axis_P.DiscreteTimeIntegrator_IC;

  /* InitializeConditions for UnitDelay: '<S1>/UD' */
  fuzzyPIDController_for_x_axis_DW.UD_DSTATE_f =
    fuzzyPIDController_for_x_axis_P.DiscreteDerivative_ICPrevScaled;
}

/* Model terminate function */
void fuzzyPIDController_for_x_axis_terminate(void)
{
  /* S-Function Block: <S4>/FIS S-function (sffis) */
  {
    FIS *fis = (FIS *)
      fuzzyPIDController_for_x_axis_DW.FISSfunction_PWORK.FISPointer;
    fisFreeFisNode(fis);
  }
}
