///////////////////////////////////////////////////////////////////////////////
// FILE:          ASIPiezo.h
// PROJECT:       Micro-Manager
// SUBSYSTEM:     DeviceAdapters
//-----------------------------------------------------------------------------
// DESCRIPTION:   ASI motorized one-axis stage device adapter
//
// COPYRIGHT:     Applied Scientific Instrumentation, Eugene OR
//
// LICENSE:       This file is distributed under the BSD license.
//
//                This file is distributed in the hope that it will be useful,
//                but WITHOUT ANY WARRANTY; without even the implied warranty
//                of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
//
//                IN NO EVENT SHALL THE COPYRIGHT OWNER OR
//                CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
//                INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES.
//
// AUTHOR:        Jon Daniels (jon@asiimaging.com) 09/2013
//
// BASED ON:      ASIStage.h and others
//

#ifndef ASIPIEZO_H
#define ASIPIEZO_H

#include "ASIPeripheralBase.h"
#include "MMDevice.h"
#include "DeviceBase.h"

class CPiezo : public ASIPeripheralBase<CStageBase, CPiezo>
{
public:
   CPiezo(const char* name);
   ~CPiezo() { }
  
   // Device API
   int Initialize();
   bool Busy();

   // Piezo API
   int Stop();
   int Home();

   // the step size is the programming unit for dimensions and is integer
   // see http://micro-manager.3463995.n2.nabble.com/what-are-quot-steps-quot-for-stages-td7580724.html
   double GetStepSize() {return stepSizeUm_;}
   int GetPositionSteps(long& steps);
   int SetPositionSteps(long steps);
   int SetPositionUm(double pos);
   int GetPositionUm(double& pos);
   int SetRelativePositionUm(double d);
   int GetLimits(double& min, double& max);
   int SetOrigin();

   bool IsContinuousFocusDrive() const { return false; }  // todo figure out what this means and if it's accurate

   int IsStageSequenceable(bool& isSequenceable) const { isSequenceable = ttl_trigger_enabled_; return DEVICE_OK; }
   int GetStageSequenceMaxLength(long& nrEvents) const { nrEvents = ring_buffer_capacity_; return DEVICE_OK; }

   // special firmware required for sequence (TTL triggering)
   int StartStageSequence();
   int StopStageSequence();
   int ClearStageSequence();
   int AddToStageSequence(double position);
   int SendStageSequence();

   // action interface
   int OnSaveCardSettings     (MM::PropertyBase* pProp, MM::ActionType eAct);
   int OnRefreshProperties    (MM::PropertyBase* pProp, MM::ActionType eAct);
   int OnLowerLim             (MM::PropertyBase* pProp, MM::ActionType eAct);
   int OnUpperLim             (MM::PropertyBase* pProp, MM::ActionType eAct);
   int OnPiezoMode            (MM::PropertyBase* pProp, MM::ActionType eAct);
   int OnJoystickFastSpeed    (MM::PropertyBase* pProp, MM::ActionType eAct);
   int OnJoystickSlowSpeed    (MM::PropertyBase* pProp, MM::ActionType eAct);
   int OnJoystickMirror       (MM::PropertyBase* pProp, MM::ActionType eAct);
   int OnJoystickSelect       (MM::PropertyBase* pProp, MM::ActionType eAct);
   int OnWheelFastSpeed       (MM::PropertyBase* pProp, MM::ActionType eAct);
   int OnWheelSlowSpeed       (MM::PropertyBase* pProp, MM::ActionType eAct);
   int OnWheelMirror          (MM::PropertyBase* pProp, MM::ActionType eAct);
   int OnMotorControl         (MM::PropertyBase* pProp, MM::ActionType eAct);
   int OnAxisPolarity         (MM::PropertyBase* pProp, MM::ActionType eAct);
   int OnMaintainMode         (MM::PropertyBase* pProp, MM::ActionType eAct);
   int OnMaintainOneOvershoot (MM::PropertyBase* pProp, MM::ActionType eAct);
   int OnMaintainOneMaxTime   (MM::PropertyBase* pProp, MM::ActionType eAct);
   int OnRunPiezoCalibration  (MM::PropertyBase* pProp, MM::ActionType eAct);
   int OnAutoSleepDelay       (MM::PropertyBase* pProp, MM::ActionType eAct);
   // single axis properties
   int OnSAAmplitude          (MM::PropertyBase* pProp, MM::ActionType eAct);
   int OnSAOffset             (MM::PropertyBase* pProp, MM::ActionType eAct);
   int OnSAPeriod             (MM::PropertyBase* pProp, MM::ActionType eAct);
   int OnSAMode               (MM::PropertyBase* pProp, MM::ActionType eAct);
   int OnSAPattern            (MM::PropertyBase* pProp, MM::ActionType eAct);
   int OnSAAdvanced           (MM::PropertyBase* pProp, MM::ActionType eAct);
   int OnSAClkSrc             (MM::PropertyBase* pProp, MM::ActionType eAct);
   int OnSAClkPol             (MM::PropertyBase* pProp, MM::ActionType eAct);
   int OnSATTLOut             (MM::PropertyBase* pProp, MM::ActionType eAct);
   int OnSATTLPol             (MM::PropertyBase* pProp, MM::ActionType eAct);
   int OnSAPatternByte        (MM::PropertyBase* pProp, MM::ActionType eAct);
   // SPIM properties
   int OnSetHomeHere          (MM::PropertyBase* pProp, MM::ActionType eAct);
   int OnHomePosition         (MM::PropertyBase* pProp, MM::ActionType eAct);
   int OnSPIMNumSlices        (MM::PropertyBase* pProp, MM::ActionType eAct);
   int OnSPIMState            (MM::PropertyBase* pProp, MM::ActionType eAct);
   // ring buffer properties
   int OnRBDelayBetweenPoints (MM::PropertyBase* pProp, MM::ActionType eAct);
   int OnRBMode               (MM::PropertyBase* pProp, MM::ActionType eAct);
   int OnRBTrigger            (MM::PropertyBase* pProp, MM::ActionType eAct);
   int OnRBRunning            (MM::PropertyBase* pProp, MM::ActionType eAct);
   int OnUseSequence          (MM::PropertyBase* pProp, MM::ActionType eAct);
   int OnFastSequence         (MM::PropertyBase* pProp, MM::ActionType eAct);
   //Others
   int OnVector				  (MM::PropertyBase* pProp, MM::ActionType eAct);

private:
   double unitMult_;
   double stepSizeUm_;
   std::string axisLetter_;
   bool ring_buffer_supported_;
   long ring_buffer_capacity_;
   bool ttl_trigger_supported_;
   bool ttl_trigger_enabled_;
   bool runningFastSequence_;
   std::vector<double> sequence_;

   int OnSaveJoystickSettings();
};

#endif // ASIPIEZO_H
