///////////////////////////////////////////////////////////////////////////////
// FILE:          ASIXYStage.cpp
// PROJECT:       Micro-Manager
// SUBSYSTEM:     DeviceAdapters
//-----------------------------------------------------------------------------
// DESCRIPTION:   ASI XY Stage device adapter
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
// BASED ON:      ASIStage.cpp and others
//

#include "ASIXYStage.h"
#include "ASITiger.h"
#include "ASIHub.h"
#include "ModuleInterface.h"
#include "DeviceUtils.h"
#include "DeviceBase.h"
#include "MMDevice.h"
#include <iostream>
#include <cmath>
#include <sstream>
#include <string>


// TODO faster busy check for typical case where axes are on same card by just querying card busy

///////////////////////////////////////////////////////////////////////////////
// CXYStage
//
CXYStage::CXYStage(const char* name) :
   ASIPeripheralBase< ::CXYStageBase, CXYStage >(name),
   unitMultX_(g_StageDefaultUnitMult),  // later will try to read actual setting
   unitMultY_(g_StageDefaultUnitMult),  // later will try to read actual setting
   stepSizeXUm_(g_StageMinStepSize),    // we'll use 1 nm as our smallest possible step size, this is somewhat arbitrary
   stepSizeYUm_(g_StageMinStepSize),    //   and doesn't change during the program
   axisLetterX_(g_EmptyAxisLetterStr),    // value determined by extended name
   axisLetterY_(g_EmptyAxisLetterStr),    // value determined by extended name
   advancedPropsEnabled_(false),
   speedTruth_(false),
   lastSpeedX_(1.0),
   lastSpeedY_(1.0),
   ring_buffer_supported_(false),
   ring_buffer_capacity_(0),
   ttl_trigger_supported_(false),
   ttl_trigger_enabled_(false)
{
   if (IsExtendedName(name))  // only set up these properties if we have the required information in the name
   {
      axisLetterX_ = GetAxisLetterFromExtName(name);
      CreateProperty(g_AxisLetterXPropertyName, axisLetterX_.c_str(), MM::String, true);
      axisLetterY_ = GetAxisLetterFromExtName(name,1);
      CreateProperty(g_AxisLetterYPropertyName, axisLetterY_.c_str(), MM::String, true);
   }
}

int CXYStage::Initialize()
{
   // call generic Initialize first, this gets hub
   RETURN_ON_MM_ERROR( PeripheralInitialize() );

   // read the unit multiplier for X and Y axes
   // ASI's unit multiplier is how many units per mm, so divide by 1000 here to get units per micron
   // we store the micron-based unit multiplier for MM use, not the mm-based one ASI uses
   std::ostringstream command;
   double tmp;
   command << "UM " << axisLetterX_ << "?";
   RETURN_ON_MM_ERROR ( hub_->QueryCommandVerify(command.str(),":") );
   RETURN_ON_MM_ERROR( hub_->ParseAnswerAfterEquals(tmp) );
   unitMultX_ = tmp/1000;
   command.str("");
   command << "UM " << axisLetterY_ << "?";
   RETURN_ON_MM_ERROR ( hub_->QueryCommandVerify(command.str(),":") );
   RETURN_ON_MM_ERROR( hub_->ParseAnswerAfterEquals(tmp) );
   unitMultY_ = tmp/1000;

   // set controller card to return positions with 1 decimal places (3 is max allowed currently, 1 gives 10nm resolution)
   command.str("");
   command << addressChar_ << "VB Z=1";
   RETURN_ON_MM_ERROR ( hub_->QueryCommand(command.str()) );

   // expose the step size to user as read-only property (no need for action handler)
   command.str("");
   command << g_StageMinStepSize;
   CreateProperty(g_StepSizeXPropertyName , command.str().c_str(), MM::Float, true);
   CreateProperty(g_StepSizeYPropertyName , command.str().c_str(), MM::Float, true);

   // create MM description; this doesn't work during hardware configuration wizard but will work afterwards
   command.str("");
   command << g_XYStageDeviceDescription << " Xaxis=" << axisLetterX_ << " Yaxis=" << axisLetterY_ << " HexAddr=" << addressString_;
   CreateProperty(MM::g_Keyword_Description, command.str().c_str(), MM::String, true);

   // max motor speed - read only property; do this way instead of via to-be-created properties to minimize serial
   //   traffic with updating speed based on speedTruth_ (and seems to do a better job of preserving decimal points)
   double minSpeedX, maxSpeedX;
   RETURN_ON_MM_ERROR ( getMinMaxSpeed(axisLetterX_, minSpeedX, maxSpeedX) );
   double minSpeedY, maxSpeedY;
   RETURN_ON_MM_ERROR ( getMinMaxSpeed(axisLetterY_, minSpeedY, maxSpeedY) );
   command.str("");
   command << (minSpeedX*1000);
   CreateProperty(g_MinMotorSpeedXPropertyName, command.str().c_str(), MM::Float, true);
   command.str("");
   command << maxSpeedX;
   CreateProperty(g_MaxMotorSpeedXPropertyName, command.str().c_str(), MM::Float, true);
   command.str("");
   command << (minSpeedY*1000);
   CreateProperty(g_MinMotorSpeedYPropertyName, command.str().c_str(), MM::Float, true);
   command.str("");
   command << maxSpeedY;
   CreateProperty(g_MaxMotorSpeedYPropertyName, command.str().c_str(), MM::Float, true);

   // now for properties that are read-write, mostly parameters that set aspects of stage behavior
   // our approach to parameters: read in value for X, if user changes it in MM then change for both X and Y
   // if user wants different ones for X and Y then he/she should set outside MM (using terminal program)
   //    and then not change in MM (and realize that Y isn't being shown by MM)
   // parameters exposed for user to set easily: SL, SU, PC, E, S, AC, WT, MA, JS X=, JS Y=, JS mirror
   // parameters maybe exposed with some hurdle to user: B, OS, AA, AZ, KP, KI, KD, AZ, CCA Y (in OnAdvancedProperties())

   CPropertyAction* pAct;

   // refresh properties from controller every time - default is not to refresh (speeds things up by not redoing so much serial comm)
   pAct = new CPropertyAction (this, &CXYStage::OnRefreshProperties);
   CreateProperty(g_RefreshPropValsPropertyName, g_NoState, MM::String, false, pAct);
   AddAllowedValue(g_RefreshPropValsPropertyName, g_NoState);
   AddAllowedValue(g_RefreshPropValsPropertyName, g_YesState);

   // save settings to controller if requested
   pAct = new CPropertyAction (this, &CXYStage::OnSaveCardSettings);
   CreateProperty(g_SaveSettingsPropertyName, g_SaveSettingsOrig, MM::String, false, pAct);
   AddAllowedValue(g_SaveSettingsPropertyName, g_SaveSettingsX);
   AddAllowedValue(g_SaveSettingsPropertyName, g_SaveSettingsY);
   AddAllowedValue(g_SaveSettingsPropertyName, g_SaveSettingsZ);
   AddAllowedValue(g_SaveSettingsPropertyName, g_SaveSettingsZJoystick);
   AddAllowedValue(g_SaveSettingsPropertyName, g_SaveSettingsOrig);
   AddAllowedValue(g_SaveSettingsPropertyName, g_SaveSettingsDone);

   // Motor speed (S) for X and Y
   pAct = new CPropertyAction (this, &CXYStage::OnSpeedXMicronsPerSec);  // allow reading actual speed at higher precision by using different units
   CreateProperty(g_MotorSpeedXMicronsPerSecPropertyName , "1000", MM::Float, true, pAct);  // read-only property updated when X speed is set
   pAct = new CPropertyAction (this, &CXYStage::OnSpeedYMicronsPerSec);  // allow reading actual speed at higher precision by using different units
   CreateProperty(g_MotorSpeedYMicronsPerSecPropertyName , "1000", MM::Float, true, pAct);  // read-only property updated when Y speed is set
   pAct = new CPropertyAction (this, &CXYStage::OnSpeedX);
   CreateProperty(g_MotorSpeedXPropertyName, "1", MM::Float, false, pAct);
   SetPropertyLimits(g_MotorSpeedXPropertyName, minSpeedX, maxSpeedX);
   UpdateProperty(g_MotorSpeedXPropertyName);
   pAct = new CPropertyAction (this, &CXYStage::OnSpeedY);
   CreateProperty(g_MotorSpeedYPropertyName, "1", MM::Float, false, pAct);
   SetPropertyLimits(g_MotorSpeedYPropertyName, minSpeedY, maxSpeedY);
   UpdateProperty(g_MotorSpeedYPropertyName);

   // Backlash (B) for X and Y
   pAct = new CPropertyAction (this, &CXYStage::OnBacklashX);
   CreateProperty(g_BacklashXPropertyName, "0", MM::Float, false, pAct);
   UpdateProperty(g_BacklashXPropertyName);
   pAct = new CPropertyAction (this, &CXYStage::OnBacklashY);
   CreateProperty(g_BacklashYPropertyName, "0", MM::Float, false, pAct);
   UpdateProperty(g_BacklashYPropertyName);

   // drift error (E) for both X and Y
   pAct = new CPropertyAction (this, &CXYStage::OnDriftErrorX);
   CreateProperty(g_DriftErrorXPropertyName, "0", MM::Float, false, pAct);
   UpdateProperty(g_DriftErrorXPropertyName);
   pAct = new CPropertyAction (this, &CXYStage::OnDriftErrorY);
   CreateProperty(g_DriftErrorYPropertyName, "0", MM::Float, false, pAct);
   UpdateProperty(g_DriftErrorYPropertyName);

   // finish error (PC) for X and Y
   pAct = new CPropertyAction (this, &CXYStage::OnFinishErrorX);
   CreateProperty(g_FinishErrorXPropertyName, "0", MM::Float, false, pAct);
   UpdateProperty(g_FinishErrorXPropertyName);
   pAct = new CPropertyAction (this, &CXYStage::OnFinishErrorY);
   CreateProperty(g_FinishErrorYPropertyName, "0", MM::Float, false, pAct);
   UpdateProperty(g_FinishErrorYPropertyName);

   // acceleration (AC) for X and Y
   pAct = new CPropertyAction (this, &CXYStage::OnAccelerationX);
   CreateProperty(g_AccelerationXPropertyName, "0", MM::Integer, false, pAct);
   UpdateProperty(g_AccelerationXPropertyName);
   pAct = new CPropertyAction (this, &CXYStage::OnAccelerationY);
   CreateProperty(g_AccelerationYPropertyName, "0", MM::Integer, false, pAct);
   UpdateProperty(g_AccelerationYPropertyName);

   // upper and lower limits (SU and SL) for X and Y
   pAct = new CPropertyAction (this, &CXYStage::OnLowerLimX);
   CreateProperty(g_LowerLimXPropertyName, "0", MM::Float, false, pAct);
   UpdateProperty(g_LowerLimXPropertyName);
   pAct = new CPropertyAction (this, &CXYStage::OnLowerLimY);
   CreateProperty(g_LowerLimYPropertyName, "0", MM::Float, false, pAct);
   UpdateProperty(g_LowerLimYPropertyName);
   pAct = new CPropertyAction (this, &CXYStage::OnUpperLimX);
   CreateProperty(g_UpperLimXPropertyName, "0", MM::Float, false, pAct);
   UpdateProperty(g_UpperLimXPropertyName);
   pAct = new CPropertyAction (this, &CXYStage::OnUpperLimY);
   CreateProperty(g_UpperLimYPropertyName, "0", MM::Float, false, pAct);
   UpdateProperty(g_UpperLimYPropertyName);

   // maintain behavior (MA) for X and Y
   pAct = new CPropertyAction (this, &CXYStage::OnMaintainStateX);
   CreateProperty(g_MaintainStateXPropertyName, g_StageMaintain_0, MM::String, false, pAct);
   AddAllowedValue(g_MaintainStateXPropertyName, g_StageMaintain_0);
   AddAllowedValue(g_MaintainStateXPropertyName, g_StageMaintain_1);
   AddAllowedValue(g_MaintainStateXPropertyName, g_StageMaintain_2);
   AddAllowedValue(g_MaintainStateXPropertyName, g_StageMaintain_3);
   UpdateProperty(g_MaintainStateXPropertyName);
   pAct = new CPropertyAction (this, &CXYStage::OnMaintainStateY);
   CreateProperty(g_MaintainStateYPropertyName, g_StageMaintain_0, MM::String, false, pAct);
   AddAllowedValue(g_MaintainStateYPropertyName, g_StageMaintain_0);
   AddAllowedValue(g_MaintainStateYPropertyName, g_StageMaintain_1);
   AddAllowedValue(g_MaintainStateYPropertyName, g_StageMaintain_2);
   AddAllowedValue(g_MaintainStateYPropertyName, g_StageMaintain_3);
   UpdateProperty(g_MaintainStateYPropertyName);

   // Motor enable/disable (MC) for X and Y
   pAct = new CPropertyAction (this, &CXYStage::OnMotorControlX);
   CreateProperty(g_MotorControlXPropertyName, g_OnState, MM::String, false, pAct);
   AddAllowedValue(g_MotorControlXPropertyName, g_OnState);
   AddAllowedValue(g_MotorControlXPropertyName, g_OffState);
   UpdateProperty(g_MotorControlPropertyName);
   pAct = new CPropertyAction (this, &CXYStage::OnMotorControlY);
   CreateProperty(g_MotorControlYPropertyName, g_OnState, MM::String, false, pAct);
   AddAllowedValue(g_MotorControlYPropertyName, g_OnState);
   AddAllowedValue(g_MotorControlYPropertyName, g_OffState);
   UpdateProperty(g_MotorControlYPropertyName);

   // Wait time, default is 0 (WT)
   pAct = new CPropertyAction (this, &CXYStage::OnWaitTime);
   CreateProperty(g_StageWaitTimePropertyName, "0", MM::Integer, false, pAct);
   UpdateProperty(g_StageWaitTimePropertyName);

   // joystick fast speed (JS X=)
   pAct = new CPropertyAction (this, &CXYStage::OnJoystickFastSpeed);
   CreateProperty(g_JoystickFastSpeedPropertyName, "100", MM::Float, false, pAct);
   SetPropertyLimits(g_JoystickFastSpeedPropertyName, 0, 100);
   UpdateProperty(g_JoystickFastSpeedPropertyName);

   // joystick slow speed (JS Y=)
   pAct = new CPropertyAction (this, &CXYStage::OnJoystickSlowSpeed);
   CreateProperty(g_JoystickSlowSpeedPropertyName, "10", MM::Float, false, pAct);
   SetPropertyLimits(g_JoystickSlowSpeedPropertyName, 0, 100);
   UpdateProperty(g_JoystickSlowSpeedPropertyName);

   // joystick mirror (changes joystick fast/slow speeds to negative)
   pAct = new CPropertyAction (this, &CXYStage::OnJoystickMirror);
   CreateProperty(g_JoystickMirrorPropertyName, g_NoState, MM::String, false, pAct);
   AddAllowedValue(g_JoystickMirrorPropertyName, g_NoState);
   AddAllowedValue(g_JoystickMirrorPropertyName, g_YesState);
   UpdateProperty(g_JoystickMirrorPropertyName);

   // joystick rotate (interchanges X and Y axes, useful if camera is rotated
   pAct = new CPropertyAction (this, &CXYStage::OnJoystickRotate);
   CreateProperty(g_JoystickRotatePropertyName, g_NoState, MM::String, false, pAct);
   AddAllowedValue(g_JoystickRotatePropertyName, g_NoState);
   AddAllowedValue(g_JoystickRotatePropertyName, g_YesState);
   UpdateProperty(g_JoystickRotatePropertyName);

   // joystick enable/disable
   pAct = new CPropertyAction (this, &CXYStage::OnJoystickEnableDisable);
   CreateProperty(g_JoystickEnabledPropertyName, g_YesState, MM::String, false, pAct);
   AddAllowedValue(g_JoystickEnabledPropertyName, g_NoState);
   AddAllowedValue(g_JoystickEnabledPropertyName, g_YesState);
   UpdateProperty(g_JoystickEnabledPropertyName);

   if (FirmwareVersionAtLeast(2.87))  // changed behavior of JS F and T as of v2.87
   {
      // fast wheel speed (JS F) (per-card, not per-axis)
      pAct = new CPropertyAction (this, &CXYStage::OnWheelFastSpeed);
      CreateProperty(g_WheelFastSpeedPropertyName, "10", MM::Float, false, pAct);
      SetPropertyLimits(g_WheelFastSpeedPropertyName, 0, 100);
      UpdateProperty(g_WheelFastSpeedPropertyName);

      // slow wheel speed (JS T) (per-card, not per-axis)
      pAct = new CPropertyAction (this, &CXYStage::OnWheelSlowSpeed);
      CreateProperty(g_WheelSlowSpeedPropertyName, "5", MM::Float, false, pAct);
      SetPropertyLimits(g_WheelSlowSpeedPropertyName, 0, 100);
      UpdateProperty(g_WheelSlowSpeedPropertyName);

      // wheel mirror (changes wheel fast/slow speeds to negative) (per-card, not per-axis)
      pAct = new CPropertyAction (this, &CXYStage::OnWheelMirror);
      CreateProperty(g_WheelMirrorPropertyName, g_NoState, MM::String, false, pAct);
      AddAllowedValue(g_WheelMirrorPropertyName, g_NoState);
      AddAllowedValue(g_WheelMirrorPropertyName, g_YesState);
      UpdateProperty(g_WheelMirrorPropertyName);
   }

   // generates a set of additional advanced properties that are rarely used
   pAct = new CPropertyAction (this, &CXYStage::OnAdvancedProperties);
   CreateProperty(g_AdvancedPropertiesPropertyName, g_NoState, MM::String, false, pAct);
   AddAllowedValue(g_AdvancedPropertiesPropertyName, g_NoState);
   AddAllowedValue(g_AdvancedPropertiesPropertyName, g_YesState);
   UpdateProperty(g_AdvancedPropertiesPropertyName);

   // invert axis by changing unitMult in Micro-manager's eyes (not actually on controller)
   pAct = new CPropertyAction (this, &CXYStage::OnAxisPolarityX);
   CreateProperty(g_AxisPolarityX, g_AxisPolarityNormal, MM::String, false, pAct);
   AddAllowedValue(g_AxisPolarityX, g_AxisPolarityReversed);
   AddAllowedValue(g_AxisPolarityX, g_AxisPolarityNormal);
   pAct = new CPropertyAction (this, &CXYStage::OnAxisPolarityY);
   CreateProperty(g_AxisPolarityY, g_AxisPolarityNormal, MM::String, false, pAct);
   AddAllowedValue(g_AxisPolarityY, g_AxisPolarityReversed);
   AddAllowedValue(g_AxisPolarityY, g_AxisPolarityNormal);

   // get build info so we can add optional properties
   FirmwareBuild build;
   RETURN_ON_MM_ERROR( hub_->GetBuildInfo(addressChar_, build) );

   // populate speedTruth_, which is whether the controller will tell us the actual speed
   if (FirmwareVersionAtLeast(3.27))
   {
      speedTruth_ = ! hub_->IsDefinePresent(build, "SPEED UNTRUTH");
   }
   else  // before v3.27
   {
      speedTruth_ = hub_->IsDefinePresent(build, "SPEED TRUTH");
   }

   // add ring buffer properties if supported (starting version 2.81)
   if (FirmwareVersionAtLeast(2.81) && (build.vAxesProps[0] & BIT1))
   {
      // get the number of ring buffer positions from the BU X output
      std::string rb_define = hub_->GetDefineString(build, "RING BUFFER");

      ring_buffer_capacity_ = 0;
      if (rb_define.size() > 12)
      {
         ring_buffer_capacity_ = atol(rb_define.substr(11).c_str());
      }

      if (ring_buffer_capacity_ != 0)
      {
         ring_buffer_supported_ = true;

         pAct = new CPropertyAction (this, &CXYStage::OnRBMode);
         CreateProperty(g_RB_ModePropertyName, g_RB_OnePoint_1, MM::String, false, pAct);
         AddAllowedValue(g_RB_ModePropertyName, g_RB_OnePoint_1);
         AddAllowedValue(g_RB_ModePropertyName, g_RB_PlayOnce_2);
         AddAllowedValue(g_RB_ModePropertyName, g_RB_PlayRepeat_3);
         UpdateProperty(g_RB_ModePropertyName);

         pAct = new CPropertyAction (this, &CXYStage::OnRBDelayBetweenPoints);
         CreateProperty(g_RB_DelayPropertyName, "0", MM::Integer, false, pAct);
         UpdateProperty(g_RB_DelayPropertyName);

         // "do it" property to do TTL trigger via serial
         pAct = new CPropertyAction (this, &CXYStage::OnRBTrigger);
         CreateProperty(g_RB_TriggerPropertyName, g_IdleState, MM::String, false, pAct);
         AddAllowedValue(g_RB_TriggerPropertyName, g_IdleState, 0);
         AddAllowedValue(g_RB_TriggerPropertyName, g_DoItState, 1);
         AddAllowedValue(g_RB_TriggerPropertyName, g_DoneState, 2);
         UpdateProperty(g_RB_TriggerPropertyName);

         pAct = new CPropertyAction (this, &CXYStage::OnRBRunning);
         CreateProperty(g_RB_AutoplayRunningPropertyName, g_NoState, MM::String, false, pAct);
         AddAllowedValue(g_RB_AutoplayRunningPropertyName, g_NoState);
         AddAllowedValue(g_RB_AutoplayRunningPropertyName, g_YesState);
         UpdateProperty(g_RB_AutoplayRunningPropertyName);

         pAct = new CPropertyAction (this, &CXYStage::OnUseSequence);
         CreateProperty(g_UseSequencePropertyName, g_NoState, MM::String, false, pAct);
         AddAllowedValue(g_UseSequencePropertyName, g_NoState);
         AddAllowedValue(g_UseSequencePropertyName, g_YesState);
         ttl_trigger_enabled_ = false;
      }

   }

   if (FirmwareVersionAtLeast(3.09) && (hub_->IsDefinePresent(build, "IN0_INT"))
         && ring_buffer_supported_)
   {
      ttl_trigger_supported_ = true;
   }

   // add SCAN properties if supported
   if (build.vAxesProps[0] & BIT2)
   {
      pAct = new CPropertyAction (this, &CXYStage::OnScanState);
      CreateProperty(g_ScanStatePropertyName, g_ScanStateIdle, MM::String, false, pAct);
      AddAllowedValue(g_ScanStatePropertyName, g_ScanStateIdle);
      AddAllowedValue(g_ScanStatePropertyName, g_ScanStateRunning);
      UpdateProperty(g_ScanStatePropertyName);

      pAct = new CPropertyAction (this, &CXYStage::OnScanFastAxis);
      CreateProperty(g_ScanFastAxisPropertyName, g_ScanAxisX, MM::String, false, pAct);
      AddAllowedValue(g_ScanFastAxisPropertyName, g_ScanAxisX);
      AddAllowedValue(g_ScanFastAxisPropertyName, g_ScanAxisY);
      UpdateProperty(g_ScanFastAxisPropertyName);

      pAct = new CPropertyAction (this, &CXYStage::OnScanSlowAxis);
      CreateProperty(g_ScanSlowAxisPropertyName, g_ScanAxisX, MM::String, false, pAct);
      AddAllowedValue(g_ScanSlowAxisPropertyName, g_ScanAxisX);
      AddAllowedValue(g_ScanSlowAxisPropertyName, g_ScanAxisY);
      AddAllowedValue(g_ScanSlowAxisPropertyName, g_ScanAxisNull);
      UpdateProperty(g_ScanSlowAxisPropertyName);

      pAct = new CPropertyAction (this, &CXYStage::OnScanPattern);
      CreateProperty(g_ScanPatternPropertyName, g_ScanPatternRaster, MM::String, false, pAct);
      AddAllowedValue(g_ScanPatternPropertyName, g_ScanPatternRaster);
      AddAllowedValue(g_ScanPatternPropertyName, g_ScanPatternSerpentine);
      UpdateProperty(g_ScanPatternPropertyName);

      pAct = new CPropertyAction (this, &CXYStage::OnScanFastStartPosition);
      CreateProperty(g_ScanFastAxisStartPositionPropertyName, "0", MM::Float, false, pAct);
      UpdateProperty(g_ScanFastAxisStartPositionPropertyName);

      pAct = new CPropertyAction (this, &CXYStage::OnScanFastStopPosition);
      CreateProperty(g_ScanFastAxisStopPositionPropertyName, "0", MM::Float, false, pAct);
      UpdateProperty(g_ScanFastAxisStopPositionPropertyName);

      pAct = new CPropertyAction (this, &CXYStage::OnScanSlowStartPosition);
      CreateProperty(g_ScanSlowAxisStartPositionPropertyName, "0", MM::Float, false, pAct);
      UpdateProperty(g_ScanSlowAxisStartPositionPropertyName);

      pAct = new CPropertyAction (this, &CXYStage::OnScanSlowStopPosition);
      CreateProperty(g_ScanSlowAxisStopPositionPropertyName, "0", MM::Float, false, pAct);
      UpdateProperty(g_ScanSlowAxisStopPositionPropertyName);

      pAct = new CPropertyAction (this, &CXYStage::OnScanNumLines);
      CreateProperty(g_ScanNumLinesPropertyName, "1", MM::Integer, false, pAct);
      SetPropertyLimits(g_ScanNumLinesPropertyName, 1, 100);  // upper limit is arbitrary, have limits to enforce > 0
      UpdateProperty(g_ScanNumLinesPropertyName);

      pAct = new CPropertyAction (this, &CXYStage::OnScanSettlingTime);
      CreateProperty(g_ScanSettlingTimePropertyName, "1", MM::Float, false, pAct);
      SetPropertyLimits(g_ScanSettlingTimePropertyName, 0., 5000.);  // limits are arbitrary really, just give a reasonable range
      UpdateProperty(g_ScanSettlingTimePropertyName);

      if (FirmwareVersionAtLeast(3.17)) {
         pAct = new CPropertyAction (this, &CXYStage::OnScanOvershootDistance);
         CreateProperty(g_ScanOvershootDistancePropertyName, "0", MM::Integer, false, pAct);  // on controller it is float but <1um precision isn't important and easier to deal with integer
         SetPropertyLimits(g_ScanOvershootDistancePropertyName, 0, 500);  // limits are arbitrary really, just give a reasonable range
         UpdateProperty(g_ScanOvershootDistancePropertyName);
      }

      if (FirmwareVersionAtLeast(3.30)) {
         pAct = new CPropertyAction (this, &CXYStage::OnScanRetraceSpeedPercent);
         CreateProperty(g_ScanRetraceSpeedPercentPropertyName, "67", MM::Float, false, pAct);
         SetPropertyLimits(g_ScanRetraceSpeedPercentPropertyName, 0.01, 100);
         UpdateProperty(g_ScanRetraceSpeedPercentPropertyName);
      }

   }

   //Vector Move VE X=### Y=###
   pAct = new CPropertyAction (this, &CXYStage::OnVectorX);
   CreateProperty(g_VectorXPropertyName, "0", MM::Float, false, pAct);
   SetPropertyLimits(g_VectorXPropertyName, maxSpeedX*-1, maxSpeedX);
   UpdateProperty(g_VectorXPropertyName);
   pAct = new CPropertyAction (this, &CXYStage::OnVectorY);
   CreateProperty(g_VectorYPropertyName, "0", MM::Float, false, pAct);
   SetPropertyLimits(g_VectorYPropertyName, maxSpeedY*-1 , maxSpeedY);
   UpdateProperty(g_VectorYPropertyName);

   initialized_ = true;
   return DEVICE_OK;
}

int CXYStage::getMinMaxSpeed(const std::string& axisLetter, double& minSpeed, double& maxSpeed)
{
   std::ostringstream command;
   command << "S " << axisLetter << "?";
   RETURN_ON_MM_ERROR( hub_->QueryCommandVerify(command.str(), ":A"));
   double origSpeed;
   RETURN_ON_MM_ERROR( hub_->ParseAnswerAfterEquals(origSpeed) );
   std::ostringstream command2;
   command2 << "S " << axisLetter << "=10000";
   RETURN_ON_MM_ERROR( hub_->QueryCommandVerify(command2.str(), ":A")); // set too high
   RETURN_ON_MM_ERROR( hub_->QueryCommandVerify(command.str(), ":A"));  // read actual max
   RETURN_ON_MM_ERROR( hub_->ParseAnswerAfterEquals(maxSpeed) );
   command2.str("");
   command2 << "S " << axisLetter << "=0.000001";
   RETURN_ON_MM_ERROR( hub_->QueryCommandVerify(command2.str(), ":A")); // set too low
   RETURN_ON_MM_ERROR( hub_->QueryCommandVerify(command.str(), ":A"));  // read actual min
   RETURN_ON_MM_ERROR( hub_->ParseAnswerAfterEquals(minSpeed) );
   command2.str("");
   command2 << "S " << axisLetter << "=" << origSpeed;
   RETURN_ON_MM_ERROR( hub_->QueryCommandVerify(command2.str(), ":A")); // restore
   return DEVICE_OK;
}

int CXYStage::GetPositionSteps(long& x, long& y)
{
   std::ostringstream command;
   command << "W " << axisLetterX_;
   RETURN_ON_MM_ERROR ( hub_->QueryCommandVerify(command.str(),":A") );
   double tmp;
   RETURN_ON_MM_ERROR ( hub_->ParseAnswerAfterPosition2(tmp) );
   x = (long)(tmp/unitMultX_/stepSizeXUm_);
   command.str("");
   command << "W " << axisLetterY_;
   RETURN_ON_MM_ERROR ( hub_->QueryCommandVerify(command.str(),":A") );
   RETURN_ON_MM_ERROR ( hub_->ParseAnswerAfterPosition2(tmp) );
   y = (long)(tmp/unitMultY_/stepSizeYUm_);
   return DEVICE_OK;
}

//rewritten to get 2 position from one serial command query, require half the time
//But may cause problem for cases where xy axis are on different cards , reply may not be in correct order
//int CXYStage::GetPositionSteps(long& x, long& y)
//{
//	 ostringstream command;	 command.str("");
//	 command << "W " << axisLetterX_<<" "<<axisLetterY_;
//	 RETURN_ON_MM_ERROR ( hub_->QueryCommandVerify(command.str(),":A") );
//	 vector<string> elems=hub_->SplitAnswerOnDelim(" ");
//	 //check if reply is in correct format
//	 //has exactly 3 strings after split, and first string is ":A"
//	 //W X Y
//	 //:A 123 123
//	 if(elems.size()<3 || elems[0].find(":A")== string::npos)
//	 {
//		RETURN_ON_MM_ERROR(DEVICE_SERIAL_INVALID_RESPONSE);
//	 }
//	 double xtmp,ytmp;
//	 xtmp=atoi(elems[1].c_str());
//	 ytmp=atoi(elems[2].c_str());
//	 x = (long)(xtmp/unitMultX_/stepSizeXUm_);
//	 y = (long)(ytmp/unitMultY_/stepSizeYUm_);
//
//	  return DEVICE_OK;
//}

int CXYStage::SetPositionSteps(long x, long y)
{
   std::ostringstream command;
   command << "M " << axisLetterX_ << "=" << x*unitMultX_*stepSizeXUm_ << " " << axisLetterY_ << "=" << y*unitMultY_*stepSizeYUm_;
   return hub_->QueryCommandVerify(command.str(),":A");
}

int CXYStage::SetRelativePositionSteps(long x, long y)
{
   std::ostringstream command;
   if ( (x == 0) && (y != 0) )
   {
      command << "R " << axisLetterY_ << "=" << y*unitMultY_*stepSizeYUm_;
   }
   else if ( (x != 0) && (y == 0) )
   {
      command << "R " << axisLetterX_ << "=" << x*unitMultX_*stepSizeXUm_;
   }
   else
   {
      command << "R " << axisLetterX_ << "=" << x*unitMultX_*stepSizeXUm_ << " " << axisLetterY_ << "=" << y*unitMultY_*stepSizeYUm_;
   }
   return hub_->QueryCommandVerify(command.str(),":A");
}

int CXYStage::GetStepLimits(long& xMin, long& xMax, long& yMin, long& yMax)
{
   // limits are always represented in terms of mm, independent of unit multiplier
   std::ostringstream command;
   command << "SL " << axisLetterX_ << "?";
   double tmp;
   RETURN_ON_MM_ERROR ( hub_->QueryCommandVerify(command.str(),":A") );
   RETURN_ON_MM_ERROR( hub_->ParseAnswerAfterEquals(tmp) );
   xMin = (long) (tmp*1000/stepSizeXUm_);
   command.str("");
   command << "SU " << axisLetterX_ << "?";
   RETURN_ON_MM_ERROR ( hub_->QueryCommandVerify(command.str(),":A") );
   RETURN_ON_MM_ERROR ( hub_->ParseAnswerAfterEquals(tmp) );
   xMax = (long) (tmp*1000/stepSizeXUm_);
   command.str("");
   command << "SL " << axisLetterY_ << "?";
   RETURN_ON_MM_ERROR ( hub_->QueryCommandVerify(command.str(),":A") );
   RETURN_ON_MM_ERROR( hub_->ParseAnswerAfterEquals(tmp) );
   yMin = (long) (tmp*1000/stepSizeYUm_);
   command.str("");
   command << "SU " << axisLetterY_ << "?";
   RETURN_ON_MM_ERROR ( hub_->QueryCommandVerify(command.str(),":A") );
   RETURN_ON_MM_ERROR ( hub_->ParseAnswerAfterEquals(tmp) );
   yMax = (long) (tmp*1000/stepSizeYUm_);
   return DEVICE_OK;
}

int CXYStage::Stop()
{
   // note this stops the card which usually is synonymous with the stage, \ stops all stages
   std::ostringstream command;
   command.str("");
   command << addressChar_ << "HALT";
   RETURN_ON_MM_ERROR ( hub_->QueryCommand(command.str()) );
   return DEVICE_OK;
}

bool CXYStage::Busy()
{
   std::ostringstream command;
   if (FirmwareVersionAtLeast(2.7)) // can use more accurate RS <axis>?
   {
      command << "RS " << axisLetterX_ << "?";
      if (hub_->QueryCommandVerify(command.str(),":A") != DEVICE_OK)  // say we aren't busy if we can't communicate
         return false;
      char c;
      if (hub_->GetAnswerCharAtPosition3(c) != DEVICE_OK)
         return false;
      if (c == 'B')
         return true;
      command.str("");
      command << "RS " << axisLetterY_ << "?";
      if (hub_->QueryCommandVerify(command.str(),":A") != DEVICE_OK)  // say we aren't busy if we can't communicate
         return false;
      if (hub_->GetAnswerCharAtPosition3(c) != DEVICE_OK)
         return false;
      return (c == 'B');
   }
   else  // use LSB of the status byte as approximate status, not quite equivalent
   {
      command << "RS " << axisLetterX_;
      if (hub_->QueryCommandVerify(command.str(),":A") != DEVICE_OK)  // say we aren't busy if we can't communicate
         return false;
      unsigned int i;
      if (hub_->ParseAnswerAfterPosition2(i) != DEVICE_OK)  // say we aren't busy if we can't communicate
         return false;
      if (i & (unsigned int)BIT0)  // mask everything but LSB
         return true; // don't bother checking other axis
      command.str("");
      command << "RS " << axisLetterY_;
      if (hub_->QueryCommandVerify(command.str(),":A") != DEVICE_OK)  // say we aren't busy if we can't communicate
         return false;
      if (hub_->ParseAnswerAfterPosition2(i) != DEVICE_OK)  // say we aren't busy if we can't communicate
         return false;
      return (i & (unsigned int)BIT0);  // mask everything but LSB
   }
}

int CXYStage::SetOrigin()
{
   std::ostringstream command;
   command << "H " << axisLetterX_ << "=0 " << axisLetterY_ << "=0";
   return hub_->QueryCommandVerify(command.str(),":A");
}

int CXYStage::SetXOrigin()
{
   std::ostringstream command;
   command << "H " << axisLetterX_ << "=0 ";
   return hub_->QueryCommandVerify(command.str(),":A");
}

int CXYStage::SetYOrigin()
{
   std::ostringstream command;
   command << "H " << axisLetterY_ << "=0";
   return hub_->QueryCommandVerify(command.str(),":A");
}

int CXYStage::Home()
{
   std::ostringstream command;
   command << "! " << axisLetterX_ << " " << axisLetterY_;
   return hub_->QueryCommandVerify(command.str(),":A");
}

int CXYStage::SetHome()
{
   if (FirmwareVersionAtLeast(2.7)) {
      std::ostringstream command;
      command << "HM " << axisLetterX_ << "+" << " " << axisLetterY_ << "+";
      return hub_->QueryCommandVerify(command.str(),":A");
   }
   else
   {
      return DEVICE_UNSUPPORTED_COMMAND;
   }
}

// Disables TTL triggering; doesn't actually stop anything already happening on controller
int CXYStage::StopXYStageSequence()
{
   std::ostringstream command;
   if (!ttl_trigger_supported_)
   {
      return DEVICE_UNSUPPORTED_COMMAND;
   }
   command << addressChar_ << "TTL X=0";  // switch off TTL triggering
   RETURN_ON_MM_ERROR ( hub_->QueryCommandVerify(command.str(),":A") );
   return DEVICE_OK;
}

// Enables TTL triggering; doesn't actually start anything going on controller
int CXYStage::StartXYStageSequence()
{
   std::ostringstream command;
   if (!ttl_trigger_supported_)
   {
      return DEVICE_UNSUPPORTED_COMMAND;
   }
   // ensure that ringbuffer pointer points to first entry
   // for now leave the axis_byte unchanged (hopefully default)
   // for now leave mode (RM F) unchanged; would normally be set to 1 and is done in OnRBMode = property "RingBufferMode"
   command << addressChar_ << "RM Z=0";
   RETURN_ON_MM_ERROR ( hub_->QueryCommandVerify(command.str(),":A") );

   command.str("");
   command << addressChar_ << "TTL X=1";  // switch on TTL triggering
   RETURN_ON_MM_ERROR ( hub_->QueryCommandVerify(command.str(),":A") );
   return DEVICE_OK;
}

int CXYStage::SendXYStageSequence()
{
   std::ostringstream command;
   if (!ttl_trigger_supported_)
   {
      return DEVICE_UNSUPPORTED_COMMAND;
   }
   command << addressChar_ << "RM X=0"; // clear ring buffer
   RETURN_ON_MM_ERROR ( hub_->QueryCommandVerify(command.str(),":A") );
   for (unsigned i=0; i< sequenceX_.size(); i++)  // send new points
   {
      command.str("");
      command << "LD " << axisLetterX_ << "=" << sequenceX_[i]*unitMultX_ << " " << axisLetterY_ << "=" << sequenceY_[i]*unitMultY_;
      RETURN_ON_MM_ERROR ( hub_->QueryCommandVerify(command.str(),":A") );
   }

   return DEVICE_OK;
}

int CXYStage::ClearXYStageSequence()
{
   std::ostringstream command;
   if (!ttl_trigger_supported_)
   {
      return DEVICE_UNSUPPORTED_COMMAND;
   }
   sequenceX_.clear();
   sequenceY_.clear();
   command << addressChar_ << "RM X=0";  // clear ring buffer
   RETURN_ON_MM_ERROR ( hub_->QueryCommandVerify(command.str(),":A") );
   return DEVICE_OK;
}

int CXYStage::AddToXYStageSequence(double positionX, double positionY)
{
   if (!ttl_trigger_supported_)
   {
      return DEVICE_UNSUPPORTED_COMMAND;
   }
   sequenceX_.push_back(positionX);
   sequenceY_.push_back(positionY);
   return DEVICE_OK;
}


int CXYStage::Move(double vx, double vy)
{
   std::ostringstream command;
   command << "VE " << axisLetterX_ << "=" << vx <<" "<< axisLetterY_ << "=" << vy ;
   return hub_->QueryCommandVerify(command.str(), ":A") ;
}

// action handlers

// redoes the joystick settings so they can be saved using SS Z
int CXYStage::OnSaveJoystickSettings()
{
   long tmp;
   std::string tmpstr;
   std::ostringstream command; command.str("");
   std::ostringstream response; response.str("");
   command << "J " << axisLetterX_ << "?";
   response << ":A " << axisLetterX_ << "=";
   RETURN_ON_MM_ERROR( hub_->QueryCommandVerify(command.str(), response.str()));
   RETURN_ON_MM_ERROR( hub_->ParseAnswerAfterEquals(tmp) );
   tmp += 100;
   command.str("");
   command << "J " << axisLetterX_ << "=" << tmp;
   RETURN_ON_MM_ERROR( hub_->QueryCommandVerify(command.str(), ":A"));
   command.str("");
   response.str("");
   command << "J " << axisLetterY_ << "?";
   response << ":A " << axisLetterY_ << "=";
   RETURN_ON_MM_ERROR( hub_->QueryCommandVerify(command.str(), response.str()));
   RETURN_ON_MM_ERROR( hub_->ParseAnswerAfterEquals(tmp) );
   tmp += 100;
   command.str("");
   command << "J " << axisLetterY_ << "=" << tmp;
   RETURN_ON_MM_ERROR( hub_->QueryCommandVerify(command.str(), ":A"));
   return DEVICE_OK;
}

int CXYStage::OnSaveCardSettings(MM::PropertyBase* pProp, MM::ActionType eAct)
{
   std::string tmpstr;
   std::ostringstream command;
   if (eAct == MM::AfterSet) {
      command.str("");
      command << addressChar_ << "SS ";
      pProp->Get(tmpstr);
      if (tmpstr == g_SaveSettingsOrig)
         return DEVICE_OK;
      if (tmpstr == g_SaveSettingsDone)
         return DEVICE_OK;
      if (tmpstr == g_SaveSettingsX)
         command << 'X';
      else if (tmpstr == g_SaveSettingsY)
         command << 'X';
      else if (tmpstr == g_SaveSettingsZ)
         command << 'Z';
      else if (tmpstr == g_SaveSettingsZJoystick)
      {
         command << 'Z';
         // do save joystick settings first
         RETURN_ON_MM_ERROR (OnSaveJoystickSettings());
      }
      RETURN_ON_MM_ERROR (hub_->QueryCommandVerify(command.str(), ":A", (long)200));  // note added 200ms delay
      pProp->Set(g_SaveSettingsDone);
      command.str(""); command << g_SaveSettingsDone;
      RETURN_ON_MM_ERROR ( hub_->UpdateSharedProperties(addressChar_, pProp->GetName(), command.str()) );
   }
   return DEVICE_OK;
}

int CXYStage::OnRefreshProperties(MM::PropertyBase* pProp, MM::ActionType eAct)
{
    if (eAct == MM::AfterSet)
    {
        std::string tmpstr;
        pProp->Get(tmpstr);
        refreshProps_ = (tmpstr == g_YesState) ? true : false;
    }
    return DEVICE_OK;
}

int CXYStage::OnAdvancedProperties(MM::PropertyBase* pProp, MM::ActionType eAct)
// special property, when set to "yes" it creates a set of little-used properties that can be manipulated thereafter
// these parameters exposed with some hurdle to user: B, OS, AA, AZ, KP, KI, KD, AZ
{
   if (eAct == MM::BeforeGet)
   {
      return DEVICE_OK; // do nothing
   }
   else if (eAct == MM::AfterSet) {
      std::string tmpstr;
      pProp->Get(tmpstr);
      if (tmpstr == g_YesState && !advancedPropsEnabled_) // after creating advanced properties once no need to repeat
      {
         CPropertyAction* pAct;
         advancedPropsEnabled_ = true;

         // make sure that the new properties are initialized, set to true at the end of creating them
         initialized_ = false;

         // overshoot (OS)
         pAct = new CPropertyAction (this, &CXYStage::OnOvershoot);
         CreateProperty(g_OvershootPropertyName, "0", MM::Float, false, pAct);
         UpdateProperty(g_OvershootPropertyName);

         // servo integral term (KI)
         pAct = new CPropertyAction (this, &CXYStage::OnKIntegral);
         CreateProperty(g_KIntegralPropertyName, "0", MM::Integer, false, pAct);
         UpdateProperty(g_KIntegralPropertyName);

         // servo proportional term (KP)
         pAct = new CPropertyAction (this, &CXYStage::OnKProportional);
         CreateProperty(g_KProportionalPropertyName, "0", MM::Integer, false, pAct);
         UpdateProperty(g_KProportionalPropertyName);

         // servo derivative term (KD)
         pAct = new CPropertyAction (this, &CXYStage::OnKDerivative);
         CreateProperty(g_KDerivativePropertyName, "0", MM::Integer, false, pAct);
         UpdateProperty(g_KDerivativePropertyName);

         // motor proportional term (KV)
         pAct = new CPropertyAction (this, &CXYStage::OnKDrive);
         CreateProperty(g_KDrivePropertyName, "0", MM::Integer, false, pAct);
         UpdateProperty(g_KDrivePropertyName);

         // motor feedforward term (KA)
         pAct = new CPropertyAction (this, &CXYStage::OnKFeedforward);
         CreateProperty(g_KFeedforwardPropertyName, "0", MM::Integer, false, pAct);
         UpdateProperty(g_KFeedforwardPropertyName);

         // Align calibration/setting for pot in drive electronics (AA)
         pAct = new CPropertyAction (this, &CXYStage::OnAAlign);
         CreateProperty(g_AAlignPropertyName, "0", MM::Integer, false, pAct);
         UpdateProperty(g_AAlignPropertyName);

         // Autozero drive electronics (AZ)
         pAct = new CPropertyAction (this, &CXYStage::OnAZeroX);
         CreateProperty(g_AZeroXPropertyName, "0", MM::String, false, pAct);
         pAct = new CPropertyAction (this, &CXYStage::OnAZeroY);
         CreateProperty(g_AZeroYPropertyName, "0", MM::String, false, pAct);
         UpdateProperty(g_AZeroYPropertyName);

         // number of extra move repetitions
         pAct = new CPropertyAction (this, &CXYStage::OnNrExtraMoveReps);
         CreateProperty(g_NrExtraMoveRepsPropertyName, "0", MM::Integer, false, pAct);
         SetPropertyLimits(g_NrExtraMoveRepsPropertyName, 0, 3);  // don't let the user set too high, though there is no actual limit
         UpdateProperty(g_NrExtraMoveRepsPropertyName);

         initialized_ = true;
      }
   }
   return DEVICE_OK;
}

int CXYStage::OnWaitTime(MM::PropertyBase* pProp, MM::ActionType eAct)
{
   std::ostringstream command;
   std::ostringstream response;
   long tmp = 0;
   if (eAct == MM::BeforeGet)
   {
      if (!refreshProps_ && initialized_)
         return DEVICE_OK;
      command << "WT " << axisLetterX_ << "?";
      response << ":" << axisLetterX_ << "=";
      RETURN_ON_MM_ERROR( hub_->QueryCommandVerify(command.str(), response.str()));
      RETURN_ON_MM_ERROR ( hub_->ParseAnswerAfterEquals(tmp) );
      pProp->Set(tmp);
   }
   else if (eAct == MM::AfterSet) {
      pProp->Get(tmp);
      command << "WT " << axisLetterX_ << "=" << tmp << " " << axisLetterY_ << "=" << tmp;
      RETURN_ON_MM_ERROR ( hub_->QueryCommandVerify(command.str(), ":A") );
   }
   return DEVICE_OK;
}

int CXYStage::OnSpeedXMicronsPerSec(MM::PropertyBase* pProp, MM::ActionType eAct)
{
   if (eAct == MM::BeforeGet || eAct == MM::AfterSet)
   {
      if (!pProp->Set(lastSpeedX_*1000))
         return DEVICE_INVALID_PROPERTY_VALUE;
   }
   return DEVICE_OK;
}

int CXYStage::OnSpeedYMicronsPerSec(MM::PropertyBase* pProp, MM::ActionType eAct)
{
   if (eAct == MM::BeforeGet || eAct == MM::AfterSet)
   {
      if (!pProp->Set(lastSpeedY_*1000))
         return DEVICE_INVALID_PROPERTY_VALUE;
   }
   return DEVICE_OK;
}

int CXYStage::OnSpeedGeneric(MM::PropertyBase* pProp, MM::ActionType eAct, const std::string& axisLetter)
{
   std::ostringstream command;
   std::ostringstream response;
   double tmp = 0;
   if (eAct == MM::BeforeGet)
   {
      if (!refreshProps_ && initialized_ && !refreshOverride_)
         return DEVICE_OK;
      refreshOverride_ = false;
      command << "S " << axisLetter << "?";
      response << ":A " << axisLetter << "=";
      RETURN_ON_MM_ERROR( hub_->QueryCommandVerify(command.str(), response.str()) );
      RETURN_ON_MM_ERROR( hub_->ParseAnswerAfterEquals(tmp) );
      if (!pProp->Set(tmp))
         return DEVICE_INVALID_PROPERTY_VALUE;
      if (axisLetter == axisLetterX_) {
         lastSpeedX_ = tmp;
         RETURN_ON_MM_ERROR( SetProperty(g_MotorSpeedXMicronsPerSecPropertyName, "1") );  // set to a dummy value, will read from lastSpeedX_ variable
      }
      else
      {
         lastSpeedY_ = tmp;
         RETURN_ON_MM_ERROR( SetProperty(g_MotorSpeedYMicronsPerSecPropertyName, "1") );  // set to a dummy value, will read from lastSpeedX_ variable
      }
   }
   else if (eAct == MM::AfterSet) {
      pProp->Get(tmp);
      command << "S " << axisLetter << "=" << tmp;
      RETURN_ON_MM_ERROR ( hub_->QueryCommandVerify(command.str(), ":A") );
      if (speedTruth_) {
         refreshOverride_ = true;
         return OnSpeedGeneric(pProp, MM::BeforeGet, axisLetter);
      }
      else
      {
         if (axisLetter == axisLetterX_)
         {
            lastSpeedX_ = tmp;
            RETURN_ON_MM_ERROR( SetProperty(g_MotorSpeedXMicronsPerSecPropertyName, "1") );  // set to a dummy value, will read from lastSpeedX_ variable
         }
         else
         {
            lastSpeedY_ = tmp;
            RETURN_ON_MM_ERROR( SetProperty(g_MotorSpeedYMicronsPerSecPropertyName, "1") );  // set to a dummy value, will read from lastSpeedX_ variable
         }
      }
   }
   return DEVICE_OK;
}

// Note: ASI units are in millimeters but MM units are in micrometers
int CXYStage::OnBacklashGeneric(MM::PropertyBase* pProp, MM::ActionType eAct, const std::string& axisLetter)
{
   std::ostringstream command;
   std::ostringstream response;
   double tmp = 0;
   if (eAct == MM::BeforeGet)
   {
      if (!refreshProps_ && initialized_)
         return DEVICE_OK;
      command << "B " << axisLetter << "?";
      response << ":" << axisLetter << "=";
      RETURN_ON_MM_ERROR( hub_->QueryCommandVerify(command.str(), response.str()));
      RETURN_ON_MM_ERROR( hub_->ParseAnswerAfterEquals(tmp) );
      tmp = 1000*tmp;
      if (!pProp->Set(tmp))
         return DEVICE_INVALID_PROPERTY_VALUE;
   }
   else if (eAct == MM::AfterSet) {
      pProp->Get(tmp);
      command << "B " << axisLetter << "=" << tmp/1000;;
      RETURN_ON_MM_ERROR ( hub_->QueryCommandVerify(command.str(), ":A") );
   }
   return DEVICE_OK;
}

// Note: ASI units are in millimeters but MM units are in micrometers
int CXYStage::OnDriftErrorGeneric(MM::PropertyBase* pProp, MM::ActionType eAct, const std::string& axisLetter)
{
   std::ostringstream command;
   std::ostringstream response;
   double tmp = 0;
   if (eAct == MM::BeforeGet)
   {
      if (!refreshProps_ && initialized_)
         return DEVICE_OK;
      command << "E " << axisLetter << "?";
      response << ":" << axisLetter << "=";
      RETURN_ON_MM_ERROR( hub_->QueryCommandVerify(command.str(), response.str()));
      RETURN_ON_MM_ERROR( hub_->ParseAnswerAfterEquals(tmp) );
      tmp = 1000*tmp;
      if (!pProp->Set(tmp))
         return DEVICE_INVALID_PROPERTY_VALUE;
   }
   else if (eAct == MM::AfterSet) {
      pProp->Get(tmp);
      command << "E " << axisLetter << "=" << tmp/1000;
      RETURN_ON_MM_ERROR ( hub_->QueryCommandVerify(command.str(), ":A") );
   }
   return DEVICE_OK;
}

// Note: ASI units are in millimeters but MM units are in micrometers
int CXYStage::OnFinishErrorGeneric(MM::PropertyBase* pProp, MM::ActionType eAct, const std::string& axisLetter)
{
   std::ostringstream command;
   std::ostringstream response;
   double tmp = 0;
   if (eAct == MM::BeforeGet)
   {
      if (!refreshProps_ && initialized_)
         return DEVICE_OK;
      command << "PC " << axisLetter << "?";
      response << ":A " << axisLetter << "=";
      RETURN_ON_MM_ERROR( hub_->QueryCommandVerify(command.str(), response.str()));
      RETURN_ON_MM_ERROR( hub_->ParseAnswerAfterEquals(tmp) );
      tmp = 1000*tmp;
      if (!pProp->Set(tmp))
         return DEVICE_INVALID_PROPERTY_VALUE;
   }
   else if (eAct == MM::AfterSet) {
      pProp->Get(tmp);
      command << "PC " << axisLetter << "=" << tmp/1000;
      RETURN_ON_MM_ERROR ( hub_->QueryCommandVerify(command.str(), ":A") );
   }
   return DEVICE_OK;
}

int CXYStage::OnLowerLimGeneric(MM::PropertyBase* pProp, MM::ActionType eAct, const std::string& axisLetter)
{
   std::ostringstream command;
   std::ostringstream response;
   double tmp = 0;
   if (eAct == MM::BeforeGet)
   {
      if (!refreshProps_ && initialized_)
         return DEVICE_OK;
      command << "SL " << axisLetter << "?";
      response << ":A " << axisLetter << "=";
      RETURN_ON_MM_ERROR( hub_->QueryCommandVerify(command.str(), response.str()));
      RETURN_ON_MM_ERROR( hub_->ParseAnswerAfterEquals(tmp) );
      if (!pProp->Set(tmp))
         return DEVICE_INVALID_PROPERTY_VALUE;
   }
   else if (eAct == MM::AfterSet) {
      pProp->Get(tmp);
      command << "SL " << axisLetter << "=" << tmp;
      RETURN_ON_MM_ERROR ( hub_->QueryCommandVerify(command.str(), ":A") );
   }
   return DEVICE_OK;
}

int CXYStage::OnUpperLimGeneric(MM::PropertyBase* pProp, MM::ActionType eAct, const std::string& axisLetter)
{
   std::ostringstream command;
   std::ostringstream response;
   double tmp = 0;
   if (eAct == MM::BeforeGet)
   {
      if (!refreshProps_ && initialized_)
         return DEVICE_OK;
      command << "SU " << axisLetter << "?";
      response << ":A " << axisLetter << "=";
      RETURN_ON_MM_ERROR( hub_->QueryCommandVerify(command.str(), response.str()));
      RETURN_ON_MM_ERROR( hub_->ParseAnswerAfterEquals(tmp) );
      if (!pProp->Set(tmp))
         return DEVICE_INVALID_PROPERTY_VALUE;
   }
   else if (eAct == MM::AfterSet) {
      pProp->Get(tmp);
      command << "SU " << axisLetter << "=" << tmp;
      RETURN_ON_MM_ERROR ( hub_->QueryCommandVerify(command.str(), ":A") );
   }
   return DEVICE_OK;
}

int CXYStage::OnAccelerationGeneric(MM::PropertyBase* pProp, MM::ActionType eAct, const std::string& axisLetter)
{
   std::ostringstream command;
   long tmp = 0;
   if (eAct == MM::BeforeGet)
   {
      if (!refreshProps_ && initialized_)
         return DEVICE_OK;
      command << "AC " << axisLetter << "?";
      std::ostringstream response;
      response << ":" << axisLetter << "=";
      RETURN_ON_MM_ERROR( hub_->QueryCommandVerify(command.str(), response.str()));
      RETURN_ON_MM_ERROR ( hub_->ParseAnswerAfterEquals(tmp) );
      if (!pProp->Set(tmp))
         return DEVICE_INVALID_PROPERTY_VALUE;
   }
   else if (eAct == MM::AfterSet) {
      pProp->Get(tmp);
      command << "AC " << axisLetter << "=" << tmp;
      RETURN_ON_MM_ERROR ( hub_->QueryCommandVerify(command.str(), ":A") );
   }
   return DEVICE_OK;
}

int CXYStage::OnMaintainStateGeneric(MM::PropertyBase* pProp, MM::ActionType eAct, const std::string& axisLetter)
{
   std::ostringstream command;
   long tmp = 0;
   if (eAct == MM::BeforeGet)
   {
      if (!refreshProps_ && initialized_)
         return DEVICE_OK;
      command << "MA " << axisLetter << "?";
      std::ostringstream response;
      response << ":A " << axisLetter << "=";
      RETURN_ON_MM_ERROR( hub_->QueryCommandVerify(command.str(), response.str()));
      RETURN_ON_MM_ERROR ( hub_->ParseAnswerAfterEquals(tmp) );
      bool success = 0;
      switch (tmp)
      {
         case 0: success = pProp->Set(g_StageMaintain_0); break;
         case 1: success = pProp->Set(g_StageMaintain_1); break;
         case 2: success = pProp->Set(g_StageMaintain_2); break;
         case 3: success = pProp->Set(g_StageMaintain_3); break;
         default: success = 0;                            break;
      }
      if (!success)
         return DEVICE_INVALID_PROPERTY_VALUE;
   }
   else if (eAct == MM::AfterSet) {
      std::string tmpstr;
      pProp->Get(tmpstr);
      if (tmpstr == g_StageMaintain_0)
         tmp = 0;
      else if (tmpstr == g_StageMaintain_1)
         tmp = 1;
      else if (tmpstr == g_StageMaintain_2)
         tmp = 2;
      else if (tmpstr == g_StageMaintain_3)
         tmp = 3;
      else
         return DEVICE_INVALID_PROPERTY_VALUE;
      command << "MA " << axisLetter << "=" << tmp;
      RETURN_ON_MM_ERROR ( hub_->QueryCommandVerify(command.str(), ":A") );
   }
   return DEVICE_OK;
}

int CXYStage::OnOvershoot(MM::PropertyBase* pProp, MM::ActionType eAct)
{
   std::ostringstream command;
   std::ostringstream response;
   double tmp = 0;
   if (eAct == MM::BeforeGet)
   {
      if (!refreshProps_ && initialized_)
         return DEVICE_OK;
      command << "OS " << axisLetterX_ << "?";
      response << ":A " << axisLetterX_ << "=";
      RETURN_ON_MM_ERROR( hub_->QueryCommandVerify(command.str(), response.str()));
      RETURN_ON_MM_ERROR( hub_->ParseAnswerAfterEquals(tmp) );
      tmp = 1000*tmp;
      if (!pProp->Set(tmp))
         return DEVICE_INVALID_PROPERTY_VALUE;
   }
   else if (eAct == MM::AfterSet) {
      pProp->Get(tmp);
      command << "OS " << axisLetterX_ << "=" << tmp/1000 << " " << axisLetterY_ << "=" << tmp/1000;
      RETURN_ON_MM_ERROR ( hub_->QueryCommandVerify(command.str(), ":A") );
   }
   return DEVICE_OK;
}

int CXYStage::OnKIntegral(MM::PropertyBase* pProp, MM::ActionType eAct)
{
   std::ostringstream command;
   std::ostringstream response;
   long tmp = 0;
   if (eAct == MM::BeforeGet)
   {
      if (!refreshProps_ && initialized_)
         return DEVICE_OK;
      command << "KI " << axisLetterX_ << "?";
      response << ":A " << axisLetterX_ << "=";
      RETURN_ON_MM_ERROR( hub_->QueryCommandVerify(command.str(), response.str()));
      RETURN_ON_MM_ERROR ( hub_->ParseAnswerAfterEquals(tmp) );
      if (!pProp->Set(tmp))
         return DEVICE_INVALID_PROPERTY_VALUE;
   }
   else if (eAct == MM::AfterSet) {
      pProp->Get(tmp);
      command << "KI " << axisLetterX_ << "=" << tmp << " " << axisLetterY_ << "=" << tmp;
      RETURN_ON_MM_ERROR ( hub_->QueryCommandVerify(command.str(), ":A") );
   }
   return DEVICE_OK;
}

int CXYStage::OnKProportional(MM::PropertyBase* pProp, MM::ActionType eAct)
{
   std::ostringstream command;
   std::ostringstream response;
   long tmp = 0;
   if (eAct == MM::BeforeGet)
   {
      if (!refreshProps_ && initialized_)
         return DEVICE_OK;
      command << "KP " << axisLetterX_ << "?";
      response << ":A " << axisLetterX_ << "=";
      RETURN_ON_MM_ERROR( hub_->QueryCommandVerify(command.str(), response.str()));
      RETURN_ON_MM_ERROR ( hub_->ParseAnswerAfterEquals(tmp) );
      if (!pProp->Set(tmp))
         return DEVICE_INVALID_PROPERTY_VALUE;
   }
   else if (eAct == MM::AfterSet) {
      pProp->Get(tmp);
      command << "KP " << axisLetterX_ << "=" << tmp << " " << axisLetterY_ << "=" << tmp;
      RETURN_ON_MM_ERROR ( hub_->QueryCommandVerify(command.str(), ":A") );
   }
   return DEVICE_OK;
}

int CXYStage::OnKDerivative(MM::PropertyBase* pProp, MM::ActionType eAct)
{
   std::ostringstream command;
   std::ostringstream response;
   long tmp = 0;
   if (eAct == MM::BeforeGet)
   {
      if (!refreshProps_ && initialized_)
         return DEVICE_OK;
      command << "KD " << axisLetterX_ << "?";
      response << ":A " << axisLetterX_ << "=";
      RETURN_ON_MM_ERROR( hub_->QueryCommandVerify(command.str(), response.str()));
      RETURN_ON_MM_ERROR ( hub_->ParseAnswerAfterEquals(tmp) );
      if (!pProp->Set(tmp))
         return DEVICE_INVALID_PROPERTY_VALUE;
   }
   else if (eAct == MM::AfterSet) {
      pProp->Get(tmp);
      command << "KD " << axisLetterX_ << "=" << tmp << " " << axisLetterY_ << "=" << tmp;
      RETURN_ON_MM_ERROR ( hub_->QueryCommandVerify(command.str(), ":A") );
   }
   return DEVICE_OK;
}

int CXYStage::OnKDrive(MM::PropertyBase* pProp, MM::ActionType eAct)
{
   std::ostringstream command;
   std::ostringstream response;
   long tmp = 0;
   if (eAct == MM::BeforeGet)
   {
      if (!refreshProps_ && initialized_)
         return DEVICE_OK;
      command << "KV " << axisLetterX_ << "?";
      response << ":A " << axisLetterX_ << "=";
      RETURN_ON_MM_ERROR( hub_->QueryCommandVerify(command.str(), response.str()));
      RETURN_ON_MM_ERROR ( hub_->ParseAnswerAfterEquals(tmp) );
      if (!pProp->Set(tmp))
         return DEVICE_INVALID_PROPERTY_VALUE;
   }
   else if (eAct == MM::AfterSet) {
      pProp->Get(tmp);
      command << "KV " << axisLetterX_ << "=" << tmp << " " << axisLetterY_ << "=" << tmp;
      RETURN_ON_MM_ERROR ( hub_->QueryCommandVerify(command.str(), ":A") );
   }
   return DEVICE_OK;
}

int CXYStage::OnKFeedforward(MM::PropertyBase* pProp, MM::ActionType eAct)
{
   std::ostringstream command;
   std::ostringstream response;
   long tmp = 0;
   if (eAct == MM::BeforeGet)
   {
      if (!refreshProps_ && initialized_)
         return DEVICE_OK;
      command << "KA " << axisLetterX_ << "?";
      response << ":A " << axisLetterX_ << "=";
      RETURN_ON_MM_ERROR( hub_->QueryCommandVerify(command.str(), response.str()));
      RETURN_ON_MM_ERROR ( hub_->ParseAnswerAfterEquals(tmp) );
      if (!pProp->Set(tmp))
         return DEVICE_INVALID_PROPERTY_VALUE;
   }
   else if (eAct == MM::AfterSet) {
      pProp->Get(tmp);
      command << "KA " << axisLetterX_ << "=" << tmp << " " << axisLetterY_ << "=" << tmp;
      RETURN_ON_MM_ERROR ( hub_->QueryCommandVerify(command.str(), ":A") );
   }
   return DEVICE_OK;
}

int CXYStage::OnAAlign(MM::PropertyBase* pProp, MM::ActionType eAct)
{
   std::ostringstream command;
   std::ostringstream response;
   long tmp = 0;
   if (eAct == MM::BeforeGet)
   {
      if (!refreshProps_ && initialized_)
         return DEVICE_OK;
      command << "AA " << axisLetterX_ << "?";
      response << ":A " << axisLetterX_ << "=";
      RETURN_ON_MM_ERROR( hub_->QueryCommandVerify(command.str(), response.str()));
      RETURN_ON_MM_ERROR ( hub_->ParseAnswerAfterEquals(tmp) );
      if (!pProp->Set(tmp))
         return DEVICE_INVALID_PROPERTY_VALUE;
   }
   else if (eAct == MM::AfterSet) {
      pProp->Get(tmp);
      command << "AA " << axisLetterX_ << "=" << tmp << " " << axisLetterY_ << "=" << tmp;
      RETURN_ON_MM_ERROR ( hub_->QueryCommandVerify(command.str(), ":A") );
   }
   return DEVICE_OK;
}

// On property change the AZ command is issued, and the reported result becomes the property value
int CXYStage::OnAZeroX(MM::PropertyBase* pProp, MM::ActionType eAct)
{
   std::ostringstream command;
   std::ostringstream response;
   if (eAct == MM::BeforeGet)
   {
      return DEVICE_OK; // do nothing
   }
   else if (eAct == MM::AfterSet) {
      command << "AZ " << axisLetterX_;
      RETURN_ON_MM_ERROR ( hub_->QueryCommandVerify(command.str(), ":A") );
      // last line has result, echo result to user as property
      std::vector<std::string> vReply = hub_->SplitAnswerOnCR();
      if (!pProp->Set(vReply.back().c_str()))
         return DEVICE_INVALID_PROPERTY_VALUE;
   }
   return DEVICE_OK;
}

// On property change the AZ command is issued, and the reported result becomes the property value
int CXYStage::OnAZeroY(MM::PropertyBase* pProp, MM::ActionType eAct)
{
   std::ostringstream command;
   std::ostringstream response;
   if (eAct == MM::BeforeGet)
   {
      return DEVICE_OK; // do nothing
   }
   else if (eAct == MM::AfterSet) {
      command << "AZ " << axisLetterY_;
      RETURN_ON_MM_ERROR ( hub_->QueryCommandVerify(command.str(), ":A") );
      // last line has result, echo result to user as property
      std::vector<std::string> vReply = hub_->SplitAnswerOnCR();
      if (!pProp->Set(vReply.back().c_str()))
         return DEVICE_INVALID_PROPERTY_VALUE;
   }
   return DEVICE_OK;
}

int CXYStage::OnMotorControlGeneric(MM::PropertyBase* pProp, MM::ActionType eAct, const std::string& axisLetter)
{
   std::ostringstream command;
   std::ostringstream response;
   long tmp = 0;
   if (eAct == MM::BeforeGet)
   {
      if (!refreshProps_ && initialized_)
         return DEVICE_OK;
      command << "MC " << axisLetter << "?";
      response << ":A ";
      RETURN_ON_MM_ERROR( hub_->QueryCommandVerify(command.str(), response.str()));
      RETURN_ON_MM_ERROR( hub_->ParseAnswerAfterPosition3(tmp) );
      bool success = 0;
      if (tmp)
         success = pProp->Set(g_OnState);
      else
         success = pProp->Set(g_OffState);
      if (!success)
         return DEVICE_INVALID_PROPERTY_VALUE;
   }
   else if (eAct == MM::AfterSet) {
      std::string tmpstr;
      pProp->Get(tmpstr);
      if (tmpstr == g_OffState)
         command << "MC " << axisLetter << "-";
      else
         command << "MC " << axisLetter << "+";
      RETURN_ON_MM_ERROR ( hub_->QueryCommandVerify(command.str(), ":A") );
   }
   return DEVICE_OK;
}

// ASI controller mirrors by having negative speed, but here we have separate property for mirroring
//   and for speed (which is strictly positive)... that makes this code a bit odd
int CXYStage::OnJoystickFastSpeed(MM::PropertyBase* pProp, MM::ActionType eAct)
{
   std::ostringstream command;
   double tmp = 0;
   if (eAct == MM::BeforeGet)
   {
      if (!refreshProps_ && initialized_)
         return DEVICE_OK;
      command << addressChar_ << "JS X?";
      RETURN_ON_MM_ERROR( hub_->QueryCommandVerify(command.str(), ":A X="));
      RETURN_ON_MM_ERROR( hub_->ParseAnswerAfterEquals(tmp) );
      tmp = abs(tmp);
      if (!pProp->Set(tmp))
         return DEVICE_INVALID_PROPERTY_VALUE;
   }
   else if (eAct == MM::AfterSet) {
      pProp->Get(tmp);
      char joystickMirror[MM::MaxStrLength];
      RETURN_ON_MM_ERROR ( GetProperty(g_JoystickMirrorPropertyName, joystickMirror) );
      command.str("");
      if (strcmp(joystickMirror, g_YesState) == 0)
         command << addressChar_ << "JS X=-" << tmp;
      else
         command << addressChar_ << "JS X=" << tmp;
      RETURN_ON_MM_ERROR ( hub_->QueryCommandVerify(command.str(), ":A") );
   }
   return DEVICE_OK;
}

// ASI controller mirrors by having negative speed, but here we have separate property for mirroring
//   and for speed (which is strictly positive)... that makes this code a bit odd
int CXYStage::OnJoystickSlowSpeed(MM::PropertyBase* pProp, MM::ActionType eAct)
{
   std::ostringstream command;
   double tmp = 0;
   if (eAct == MM::BeforeGet)
   {
      if (!refreshProps_ && initialized_)
         return DEVICE_OK;
      command << addressChar_ << "JS Y?";
      RETURN_ON_MM_ERROR( hub_->QueryCommandVerify(command.str(), ":A Y="));
      RETURN_ON_MM_ERROR( hub_->ParseAnswerAfterEquals(tmp) );
      if (!pProp->Set(tmp))
         return DEVICE_INVALID_PROPERTY_VALUE;
   }
   else if (eAct == MM::AfterSet) {
      pProp->Get(tmp);
      char joystickMirror[MM::MaxStrLength];
      RETURN_ON_MM_ERROR ( GetProperty(g_JoystickMirrorPropertyName, joystickMirror) );
      command.str("");
      if (strcmp(joystickMirror, g_YesState) == 0)
         command << addressChar_ << "JS Y=-" << tmp;
      else
         command << addressChar_ << "JS Y=" << tmp;
      RETURN_ON_MM_ERROR ( hub_->QueryCommandVerify(command.str(), ":A") );
   }
   return DEVICE_OK;
}

// ASI controller mirrors by having negative speed, but here we have separate property for mirroring
//   and for speed (which is strictly positive)... that makes this code a bit odd
int CXYStage::OnJoystickMirror(MM::PropertyBase* pProp, MM::ActionType eAct)
{
   std::ostringstream command;
   double tmp = 0;
   if (eAct == MM::BeforeGet)
   {
      if (!refreshProps_ && initialized_)
         return DEVICE_OK;
      command << addressChar_ << "JS X?";  // query only the fast setting to see if already mirrored
      RETURN_ON_MM_ERROR( hub_->QueryCommandVerify(command.str(), ":A X="));
      RETURN_ON_MM_ERROR( hub_->ParseAnswerAfterEquals(tmp) );
      bool success = 0;
      if (tmp < 0) // speed negative <=> mirrored
         success = pProp->Set(g_YesState);
      else
         success = pProp->Set(g_NoState);
      if (!success)
         return DEVICE_INVALID_PROPERTY_VALUE;
   }
   else if (eAct == MM::AfterSet)
   {
      std::string tmpstr;
      pProp->Get(tmpstr);
      double joystickFast = 0.0;
      RETURN_ON_MM_ERROR ( GetProperty(g_JoystickFastSpeedPropertyName, joystickFast) );
      double joystickSlow = 0.0;
      RETURN_ON_MM_ERROR ( GetProperty(g_JoystickSlowSpeedPropertyName, joystickSlow) );
      command.str("");
      if (tmpstr == g_YesState)
         command << addressChar_ << "JS X=-" << joystickFast << " Y=-" << joystickSlow;
      else
         command << addressChar_ << "JS X=" << joystickFast << " Y=" << joystickSlow;
      RETURN_ON_MM_ERROR ( hub_->QueryCommandVerify(command.str(), ":A") );
   }
   return DEVICE_OK;
}

// interchanges axes for X and Y on the joystick
int CXYStage::OnJoystickRotate(MM::PropertyBase* pProp, MM::ActionType eAct)
{
   std::ostringstream command;
   std::ostringstream response;
   double tmp = 0;
   if (eAct == MM::BeforeGet)
   {
      if (!refreshProps_ && initialized_)
         return DEVICE_OK;
      command << "J " << axisLetterX_ << "?";  // only look at X axis for joystick
      response << ":A " << axisLetterX_ << "=";
      RETURN_ON_MM_ERROR( hub_->QueryCommandVerify(command.str(), response.str()));
      RETURN_ON_MM_ERROR ( hub_->ParseAnswerAfterEquals(tmp) );
      bool success = 0;
      if (tmp == 3) // if set to be Y joystick direction then we are rotated, otherwise assume not rotated
         success = pProp->Set(g_YesState);
      else
         success = pProp->Set(g_NoState);
      if (!success)
         return DEVICE_INVALID_PROPERTY_VALUE;
   }
   else if (eAct == MM::AfterSet) {
      // ideally would call OnJoystickEnableDisable but don't know how to get the appropriate pProp
      std::string tmpstr;
      pProp->Get(tmpstr);
      char joystickEnabled[MM::MaxStrLength];
      RETURN_ON_MM_ERROR ( GetProperty(g_JoystickEnabledPropertyName, joystickEnabled) );
      if (strcmp(joystickEnabled, g_YesState) == 0)
      {
         if (tmpstr == g_YesState)
            command << "J " << axisLetterX_ << "=3" << " " << axisLetterY_ << "=2";  // rotated
         else
            command << "J " << axisLetterX_ << "=2" << " " << axisLetterY_ << "=3";
      }
      else  // No = disabled
         command << "J " << axisLetterX_ << "=0" << " " << axisLetterY_ << "=0";
      RETURN_ON_MM_ERROR ( hub_->QueryCommandVerify(command.str(), ":A") );
   }
   return DEVICE_OK;
}

int CXYStage::OnJoystickEnableDisable(MM::PropertyBase* pProp, MM::ActionType eAct)
{
   std::ostringstream command;
   std::ostringstream response;
   long tmp = 0;
   if (eAct == MM::BeforeGet)
   {
      if (!refreshProps_ && initialized_)
         return DEVICE_OK;
      command << "J " << axisLetterX_ << "?";
      response << ":A " << axisLetterX_ << "=";
      RETURN_ON_MM_ERROR( hub_->QueryCommandVerify(command.str(), response.str()));
      RETURN_ON_MM_ERROR ( hub_->ParseAnswerAfterEquals(tmp) );
      bool success = 0;
      if (tmp) // treat anything nozero as enabled when reading
         success = pProp->Set(g_YesState);
      else
         success = pProp->Set(g_NoState);
      if (!success)
         return DEVICE_INVALID_PROPERTY_VALUE;
   }
   else if (eAct == MM::AfterSet)
   {
      std::string tmpstr;
      pProp->Get(tmpstr);
      if (tmpstr == g_YesState)
      {
         char joystickRotate[MM::MaxStrLength];
         RETURN_ON_MM_ERROR ( GetProperty(g_JoystickRotatePropertyName, joystickRotate) );
         if (strcmp(joystickRotate, g_YesState) == 0)
            command << "J " << axisLetterX_ << "=3" << " " << axisLetterY_ << "=2";  // rotated
         else
            command << "J " << axisLetterX_ << "=2" << " " << axisLetterY_ << "=3";
      }
      else  // No = disabled
         command << "J " << axisLetterX_ << "=0" << " " << axisLetterY_ << "=0";
      RETURN_ON_MM_ERROR ( hub_->QueryCommandVerify(command.str(), ":A") );
   }
   return DEVICE_OK;
}

// ASI controller mirrors by having negative speed, but here we have separate property for mirroring
//   and for speed (which is strictly positive)... that makes this code a bit odd
// note that this setting is per-card, not per-axis
int CXYStage::OnWheelFastSpeed(MM::PropertyBase* pProp, MM::ActionType eAct)
{
   std::ostringstream command;
   double tmp = 0;
   if (eAct == MM::BeforeGet)
   {
      if (!refreshProps_ && initialized_)
         return DEVICE_OK;
      command << addressChar_ << "JS F?";  // query only the fast setting to see if already mirrored
      RETURN_ON_MM_ERROR( hub_->QueryCommandVerify(command.str(), ":A F="));
      RETURN_ON_MM_ERROR( hub_->ParseAnswerAfterEquals(tmp) );
      tmp = abs(tmp);
      if (!pProp->Set(tmp))
         return DEVICE_INVALID_PROPERTY_VALUE;
   }
   else if (eAct == MM::AfterSet) {
      pProp->Get(tmp);
      char wheelMirror[MM::MaxStrLength];
      RETURN_ON_MM_ERROR ( GetProperty(g_WheelMirrorPropertyName, wheelMirror) );
      command.str("");
      if (strcmp(wheelMirror, g_YesState) == 0)
         command << addressChar_ << "JS F=-" << tmp;
      else
         command << addressChar_ << "JS F=" << tmp;
      RETURN_ON_MM_ERROR ( hub_->QueryCommandVerify(command.str(), ":A") );
   }
   return DEVICE_OK;
}

// ASI controller mirrors by having negative speed, but here we have separate property for mirroring
//   and for speed (which is strictly positive)... that makes this code a bit odd
// note that this setting is per-card, not per-axis
int CXYStage::OnWheelSlowSpeed(MM::PropertyBase* pProp, MM::ActionType eAct)
{
   std::ostringstream command;
   double tmp = 0;
   if (eAct == MM::BeforeGet)
   {
      if (!refreshProps_ && initialized_)
         return DEVICE_OK;
      command << addressChar_ << "JS T?";
      RETURN_ON_MM_ERROR( hub_->QueryCommandVerify(command.str(), ":A T="));
      RETURN_ON_MM_ERROR( hub_->ParseAnswerAfterEquals(tmp) );
      tmp = abs(tmp);
      if (!pProp->Set(tmp))
         return DEVICE_INVALID_PROPERTY_VALUE;
   }
   else if (eAct == MM::AfterSet) {
      pProp->Get(tmp);
      char wheelMirror[MM::MaxStrLength];
      RETURN_ON_MM_ERROR ( GetProperty(g_JoystickMirrorPropertyName, wheelMirror) );
      command.str("");
      if (strcmp(wheelMirror, g_YesState) == 0)
         command << addressChar_ << "JS T=-" << tmp;
      else
         command << addressChar_ << "JS T=" << tmp;
      RETURN_ON_MM_ERROR ( hub_->QueryCommandVerify(command.str(), ":A") );
   }
   return DEVICE_OK;
}

// ASI controller mirrors by having negative speed, but here we have separate property for mirroring
//   and for speed (which is strictly positive)... that makes this code a bit odd
// note that this setting is per-card, not per-axis
int CXYStage::OnWheelMirror(MM::PropertyBase* pProp, MM::ActionType eAct)
{
   std::ostringstream command;
   double tmp = 0;
   if (eAct == MM::BeforeGet)
   {
      if (!refreshProps_ && initialized_)
         return DEVICE_OK;
      command << addressChar_ << "JS F?";  // query only the fast setting to see if already mirrored
      RETURN_ON_MM_ERROR( hub_->QueryCommandVerify(command.str(), ":A F="));
      RETURN_ON_MM_ERROR( hub_->ParseAnswerAfterEquals(tmp) );
      bool success = 0;
      if (tmp < 0) // speed negative <=> mirrored
         success = pProp->Set(g_YesState);
      else
         success = pProp->Set(g_NoState);
      if (!success)
         return DEVICE_INVALID_PROPERTY_VALUE;
   }
   else if (eAct == MM::AfterSet)
   {
      std::string tmpstr;
      pProp->Get(tmpstr);
      double wheelFast = 0.0;
      RETURN_ON_MM_ERROR ( GetProperty(g_WheelFastSpeedPropertyName, wheelFast) );
      double wheelSlow = 0.0;
      RETURN_ON_MM_ERROR ( GetProperty(g_WheelSlowSpeedPropertyName, wheelSlow) );
      command.str("");
      if (tmpstr == g_YesState)
         command << addressChar_ << "JS F=-" << wheelFast << " T=-" << wheelSlow;
      else
         command << addressChar_ << "JS F=" << wheelFast << " T=" << wheelSlow;
      RETURN_ON_MM_ERROR ( hub_->QueryCommandVerify(command.str(), ":A") );
   }
   return DEVICE_OK;
}

int CXYStage::OnNrExtraMoveReps(MM::PropertyBase* pProp, MM::ActionType eAct)
{
   std::ostringstream command;
   long tmp = 0;
   if (eAct == MM::BeforeGet)
   {
      if (!refreshProps_ && initialized_)
         return DEVICE_OK;
      command << addressChar_ << "CCA Y?";
      RETURN_ON_MM_ERROR( hub_->QueryCommandVerify(command.str(), ":A"));
      RETURN_ON_MM_ERROR ( hub_->ParseAnswerAfterEquals(tmp) );
      // don't complain if value is larger than MM's "artificial" limits, it just won't be set
      pProp->Set(tmp);
   }
   else if (eAct == MM::AfterSet) {
      pProp->Get(tmp);
      command.str("");
      command << addressChar_ << "CCA Y=" << tmp;
      RETURN_ON_MM_ERROR ( hub_->QueryCommandVerify(command.str(), ":A") );
   }
   return DEVICE_OK;
}

int CXYStage::OnAxisPolarityX(MM::PropertyBase* pProp, MM::ActionType eAct)
{
   if (eAct == MM::BeforeGet)
   {
      // do nothing
   }
   else if (eAct == MM::AfterSet)
   {
      std::string tmpstr;
      pProp->Get(tmpstr);
      if (tmpstr == g_AxisPolarityReversed) {
         unitMultX_ = -1*abs(unitMultX_);
      } else {
         unitMultX_ = abs(unitMultX_);
      }
   }
   return DEVICE_OK;
}

int CXYStage::OnAxisPolarityY(MM::PropertyBase* pProp, MM::ActionType eAct)
{
   if (eAct == MM::BeforeGet)
   {
      // do nothing
   }
   else if (eAct == MM::AfterSet) {
      std::string tmpstr;
      pProp->Get(tmpstr);
      if (tmpstr == g_AxisPolarityReversed) {
         unitMultY_ = -1*abs(unitMultY_);
      } else {
         unitMultY_ = abs(unitMultY_);
      }
   }
   return DEVICE_OK;
}

int CXYStage::OnScanState(MM::PropertyBase* pProp, MM::ActionType eAct)
{
   std::ostringstream command;
   if (eAct == MM::BeforeGet)
   {
      if (!refreshProps_ && initialized_)
         return DEVICE_OK;
      command << addressChar_ << "SN X?";
      RETURN_ON_MM_ERROR( hub_->QueryCommandVerify(command.str(), ":A"));
      bool success;
      char c;
      RETURN_ON_MM_ERROR( hub_->GetAnswerCharAtPosition3(c) );
      switch ( c )
      {
         case g_ScanStateCodeIdle:  success = pProp->Set(g_ScanStateIdle); break;
         default:                   success = pProp->Set(g_ScanStateRunning); break;  // a bunch of different codes are possible while running
      }
      if (!success)
         return DEVICE_INVALID_PROPERTY_VALUE;
   }
   else if (eAct == MM::AfterSet) {
      std::string tmpstr;
      char c;
      pProp->Get(tmpstr);
      if (tmpstr == g_ScanStateIdle)
      {
         // TODO cleanup code by calling action handler with MM::BeforeGet?
         // check status and stop if it's not idle already
         command << addressChar_ << "SN X?";
         RETURN_ON_MM_ERROR( hub_->QueryCommandVerify(command.str(), ":A"));
         RETURN_ON_MM_ERROR( hub_->GetAnswerCharAtPosition3(c) );
         if (c!=g_ScanStateCodeIdle)
         {
            // this will stop state machine if it's running, if we do SN without args we run the risk of it stopping itself before we send the next command
            // after we stop it, it will automatically go to idle state
            command.str("");
            command << addressChar_ << "SN X=" << (int)g_ScanStateCodeStop;
            RETURN_ON_MM_ERROR( hub_->QueryCommandVerify(command.str(), ":A"));
         }
      }
      else if (tmpstr == g_ScanStateRunning)
      {
         // check status and start if it's idle
         command << addressChar_ << "SN X?";
         RETURN_ON_MM_ERROR( hub_->QueryCommandVerify(command.str(), ":A"));
         RETURN_ON_MM_ERROR( hub_->GetAnswerCharAtPosition3(c) );
         if (c==g_SPIMStateCode_Idle)
         {
            // if we are idle or armed then start it
            // assume that nothing else could have started it since our query moments ago
            command.str("");
            command << addressChar_ << "SN";
            RETURN_ON_MM_ERROR( hub_->QueryCommandVerify(command.str(), ":A"));
         }
      }
      else
         return DEVICE_INVALID_PROPERTY_VALUE;
   }
   return DEVICE_OK;
}

int CXYStage::OnScanFastAxis(MM::PropertyBase* pProp, MM::ActionType eAct)
{
   std::ostringstream command;
   if (eAct == MM::BeforeGet)
   {
      if (!refreshProps_ && initialized_)
         return DEVICE_OK;
      command << addressChar_ << "SN Y?";
      RETURN_ON_MM_ERROR( hub_->QueryCommandVerify(command.str(), ":A"));
      bool success;
      char c;
      RETURN_ON_MM_ERROR( hub_->GetAnswerCharAtPosition3(c) );
      switch ( c )
      {
         case g_ScanAxisXCode:  success = pProp->Set(g_ScanAxisX); break;
         case g_ScanAxisYCode:  success = pProp->Set(g_ScanAxisY); break;
         default:               success = false; break;
      }
      if (!success)
         return DEVICE_INVALID_PROPERTY_VALUE;
   }
   else if (eAct == MM::AfterSet) {
      std::string tmpstr;
      char c = ' ';
      pProp->Get(tmpstr);
      if (tmpstr == g_ScanAxisX) {
         c = g_ScanAxisXCode;
      } else if (tmpstr == g_ScanAxisY) {
         c = g_ScanAxisYCode;
      }
      if (c == ' ')
      {
         return DEVICE_INVALID_PROPERTY_VALUE;
      }
      command << addressChar_ << "SN Y=" << c;
      RETURN_ON_MM_ERROR( hub_->QueryCommandVerify(command.str(), ":A"));
   }
   return DEVICE_OK;
}

int CXYStage::OnScanSlowAxis(MM::PropertyBase* pProp, MM::ActionType eAct)
{
   std::ostringstream command;
   if (eAct == MM::BeforeGet)
   {
      if (!refreshProps_ && initialized_)
         return DEVICE_OK;
      command << addressChar_ << "SN Z?";
      RETURN_ON_MM_ERROR( hub_->QueryCommandVerify(command.str(), ":A"));
      bool success;
      char c;
      RETURN_ON_MM_ERROR( hub_->GetAnswerCharAtPosition3(c) );
      switch ( c )
      {
         case g_ScanAxisXCode:    success = pProp->Set(g_ScanAxisX); break;
         case g_ScanAxisYCode:    success = pProp->Set(g_ScanAxisY); break;
         case g_ScanAxisNullCode: success = pProp->Set(g_ScanAxisNull); break;
         default:                 success = false; break;
      }
      if (!success)
         return DEVICE_INVALID_PROPERTY_VALUE;
   }
   else if (eAct == MM::AfterSet) {
      std::string tmpstr;
      char c = ' ';
      pProp->Get(tmpstr);
      if (tmpstr == g_ScanAxisX) {
         c = g_ScanAxisXCode;
      } else if (tmpstr == g_ScanAxisY) {
         c = g_ScanAxisYCode;
      } else if (tmpstr == g_ScanAxisNull) {
         c = g_ScanAxisNullCode;
      }
      if (c == ' ')
      {
         return DEVICE_INVALID_PROPERTY_VALUE;
      }
      command << addressChar_ << "SN Z=" << c;
      RETURN_ON_MM_ERROR( hub_->QueryCommandVerify(command.str(), ":A"));
   }
   return DEVICE_OK;
}

int CXYStage::OnScanPattern(MM::PropertyBase* pProp, MM::ActionType eAct)
{
   std::ostringstream command;
   if (eAct == MM::BeforeGet)
   {
      if (!refreshProps_ && initialized_)
         return DEVICE_OK;
      command << addressChar_ << "SN F?";
      RETURN_ON_MM_ERROR( hub_->QueryCommandVerify(command.str(), ":A"));
      bool success;
      char c;
      RETURN_ON_MM_ERROR( hub_->GetAnswerCharAtPosition3(c) );
      switch ( c )
      {
         case g_ScanPatternRasterCode:      success = pProp->Set(g_ScanPatternRaster); break;
         case g_ScanPatternSerpentineCode:  success = pProp->Set(g_ScanPatternSerpentine); break;
         default:               success = false; break;
      }
      if (!success)
         return DEVICE_INVALID_PROPERTY_VALUE;
   }
   else if (eAct == MM::AfterSet) {
      std::string tmpstr;
      char c = ' ';
      pProp->Get(tmpstr);
      if (tmpstr == g_ScanPatternRaster) {
         c = g_ScanPatternRasterCode;
      } else if (tmpstr == g_ScanPatternSerpentine) {
         c = g_ScanPatternSerpentineCode;
      }
      if (c == ' ')
      {
         return DEVICE_INVALID_PROPERTY_VALUE;
      }
      command << addressChar_ << "SN F=" << c;
      RETURN_ON_MM_ERROR( hub_->QueryCommandVerify(command.str(), ":A"));
   }
   return DEVICE_OK;
}

int CXYStage::OnScanFastStartPosition(MM::PropertyBase* pProp, MM::ActionType eAct)
{
   std::ostringstream command;
   double tmp = 0;
   if (eAct == MM::BeforeGet)
   {
      if (!refreshProps_ && initialized_)
         return DEVICE_OK;
      command << addressChar_ << "NR X?";
      RETURN_ON_MM_ERROR( hub_->QueryCommandVerify(command.str(), ":A X="));
      RETURN_ON_MM_ERROR( hub_->ParseAnswerAfterEquals(tmp) );
      if (!pProp->Set(tmp))
         return DEVICE_INVALID_PROPERTY_VALUE;
   }
   else if (eAct == MM::AfterSet) {
      pProp->Get(tmp);
      command << addressChar_ << "NR X=" << tmp;
      RETURN_ON_MM_ERROR ( hub_->QueryCommandVerify(command.str(), ":A") );
   }
   return DEVICE_OK;
}

int CXYStage::OnScanFastStopPosition(MM::PropertyBase* pProp, MM::ActionType eAct)
{
   std::ostringstream command;
   double tmp = 0;
   if (eAct == MM::BeforeGet)
   {
      if (!refreshProps_ && initialized_)
         return DEVICE_OK;
      command << addressChar_ << "NR Y?";
      RETURN_ON_MM_ERROR( hub_->QueryCommandVerify(command.str(), ":A Y="));
      RETURN_ON_MM_ERROR( hub_->ParseAnswerAfterEquals(tmp) );
      if (!pProp->Set(tmp))
         return DEVICE_INVALID_PROPERTY_VALUE;
   }
   else if (eAct == MM::AfterSet) {
      pProp->Get(tmp);
      command << addressChar_ << "NR Y=" << tmp;
      RETURN_ON_MM_ERROR ( hub_->QueryCommandVerify(command.str(), ":A") );
   }
   return DEVICE_OK;
}

int CXYStage::OnScanSlowStartPosition(MM::PropertyBase* pProp, MM::ActionType eAct)
{
   std::ostringstream command;
   double tmp = 0;
   if (eAct == MM::BeforeGet)
   {
      if (!refreshProps_ && initialized_)
         return DEVICE_OK;
      command << addressChar_ << "NV X?";
      RETURN_ON_MM_ERROR( hub_->QueryCommandVerify(command.str(), ":A X="));
      RETURN_ON_MM_ERROR( hub_->ParseAnswerAfterEquals(tmp) );
      if (!pProp->Set(tmp))
         return DEVICE_INVALID_PROPERTY_VALUE;
   }
   else if (eAct == MM::AfterSet) {
      pProp->Get(tmp);
      command << addressChar_ << "NV X=" << tmp;
      RETURN_ON_MM_ERROR ( hub_->QueryCommandVerify(command.str(), ":A") );
   }
   return DEVICE_OK;
}

int CXYStage::OnScanSlowStopPosition(MM::PropertyBase* pProp, MM::ActionType eAct)
{
   std::ostringstream command;
   double tmp = 0;
   if (eAct == MM::BeforeGet)
   {
      if (!refreshProps_ && initialized_)
         return DEVICE_OK;
      command << addressChar_ << "NV Y?";
      RETURN_ON_MM_ERROR( hub_->QueryCommandVerify(command.str(), ":A Y="));
      RETURN_ON_MM_ERROR( hub_->ParseAnswerAfterEquals(tmp) );
      if (!pProp->Set(tmp))
         return DEVICE_INVALID_PROPERTY_VALUE;
   }
   else if (eAct == MM::AfterSet) {
      pProp->Get(tmp);
      command << addressChar_ << "NV Y=" << tmp;
      RETURN_ON_MM_ERROR ( hub_->QueryCommandVerify(command.str(), ":A") );
   }
   return DEVICE_OK;
}

int CXYStage::OnScanNumLines(MM::PropertyBase* pProp, MM::ActionType eAct)
{
   std::ostringstream command;
   std::ostringstream response;
   long tmp = 0;
   if (eAct == MM::BeforeGet)
   {
      if (!refreshProps_ && initialized_)
         return DEVICE_OK;
      command << addressChar_ << "NV Z?";
      RETURN_ON_MM_ERROR( hub_->QueryCommandVerify(command.str(), ":A Z="));
      RETURN_ON_MM_ERROR( hub_->ParseAnswerAfterEquals(tmp) );
      if (!pProp->Set(tmp))
         return DEVICE_INVALID_PROPERTY_VALUE;
   }
   else if (eAct == MM::AfterSet) {
      pProp->Get(tmp);
      command << addressChar_ << "NV Z=" << tmp;
      RETURN_ON_MM_ERROR ( hub_->QueryCommandVerify(command.str(), ":A") );
   }
   return DEVICE_OK;
}

int CXYStage::OnScanSettlingTime(MM::PropertyBase* pProp, MM::ActionType eAct)
{
   std::ostringstream command;
   double tmp = 0;
   if (eAct == MM::BeforeGet)
   {
      if (!refreshProps_ && initialized_)
         return DEVICE_OK;
      command << addressChar_ << "NV F?";
      RETURN_ON_MM_ERROR( hub_->QueryCommandVerify(command.str(), ":A F="));
      RETURN_ON_MM_ERROR( hub_->ParseAnswerAfterEquals(tmp) );
      if (!pProp->Set(tmp))
         return DEVICE_INVALID_PROPERTY_VALUE;
   }
   else if (eAct == MM::AfterSet) {
      pProp->Get(tmp);
      command << addressChar_ << "NV F=" << tmp;
      RETURN_ON_MM_ERROR ( hub_->QueryCommandVerify(command.str(), ":A") );
   }
   return DEVICE_OK;
}

// Note: ASI units are in millimeters but MM units are in micrometers
int CXYStage::OnScanOvershootDistance(MM::PropertyBase* pProp, MM::ActionType eAct)
{
   std::ostringstream command;
   double tmp = 0;  // represent as integer in um, but controller gives as float in mm
   if (eAct == MM::BeforeGet)
   {
      if (!refreshProps_ && initialized_)
         return DEVICE_OK;
      command << addressChar_ << "NV T?";
      RETURN_ON_MM_ERROR( hub_->QueryCommandVerify(command.str(), ":A T="));
      RETURN_ON_MM_ERROR( hub_->ParseAnswerAfterEquals(tmp) );
      if (!pProp->Set(1000*tmp+0.5))  // convert to um, then round to nearest by adding 0.5 before implicit floor operation
         return DEVICE_INVALID_PROPERTY_VALUE;
   }
   else if (eAct == MM::AfterSet) {
      pProp->Get(tmp);
      command << addressChar_ << "NV T=" << tmp/1000;
      RETURN_ON_MM_ERROR ( hub_->QueryCommandVerify(command.str(), ":A") );
   }
   return DEVICE_OK;
}

int CXYStage::OnScanRetraceSpeedPercent(MM::PropertyBase* pProp, MM::ActionType eAct)
{
   std::ostringstream command;
   double tmp = 0;
   if (eAct == MM::BeforeGet)
   {
      if (!refreshProps_ && initialized_)
         return DEVICE_OK;
      command << addressChar_ << "NR R?";
      RETURN_ON_MM_ERROR( hub_->QueryCommandVerify(command.str(), ":A R="));
      RETURN_ON_MM_ERROR( hub_->ParseAnswerAfterEquals(tmp) );
      if (!pProp->Set(tmp))
         return DEVICE_INVALID_PROPERTY_VALUE;
   }
   else if (eAct == MM::AfterSet) {
      pProp->Get(tmp);
      command << addressChar_ << "NR R=" << tmp;
      RETURN_ON_MM_ERROR ( hub_->QueryCommandVerify(command.str(), ":A") );
   }
   return DEVICE_OK;
}

int CXYStage::OnRBMode(MM::PropertyBase* pProp, MM::ActionType eAct)
{
   std::ostringstream command;
   std::ostringstream response;
   std::string pseudoAxisChar = FirmwareVersionAtLeast(2.89) ? "F" : "X";
   long tmp;
   if (eAct == MM::BeforeGet)
   {
      if (!refreshProps_ && initialized_)
         return DEVICE_OK;
      command << addressChar_ << "RM " << pseudoAxisChar << "?";
      response << ":A " << pseudoAxisChar << "=";
      RETURN_ON_MM_ERROR( hub_->QueryCommandVerify(command.str(), response.str()) );
      RETURN_ON_MM_ERROR( hub_->ParseAnswerAfterEquals(tmp) );
      if (tmp >= 128)
      {
         tmp -= 128;  // remove the "running now" code if present
      }
      bool success;
      switch ( tmp )
      {
         case 1: success = pProp->Set(g_RB_OnePoint_1); break;
         case 2: success = pProp->Set(g_RB_PlayOnce_2); break;
         case 3: success = pProp->Set(g_RB_PlayRepeat_3); break;
         default: success = false;
      }
      if (!success)
         return DEVICE_INVALID_PROPERTY_VALUE;
   }
   else if (eAct == MM::AfterSet) {
      if (hub_->UpdatingSharedProperties())
         return DEVICE_OK;
      std::string tmpstr;
      pProp->Get(tmpstr);
      if (tmpstr == g_RB_OnePoint_1)
         tmp = 1;
      else if (tmpstr == g_RB_PlayOnce_2)
         tmp = 2;
      else if (tmpstr == g_RB_PlayRepeat_3)
         tmp = 3;
      else
         return DEVICE_INVALID_PROPERTY_VALUE;
      command << addressChar_ << "RM " << pseudoAxisChar << "=" << tmp;
      RETURN_ON_MM_ERROR( hub_->QueryCommandVerify(command.str(), ":A"));
      RETURN_ON_MM_ERROR ( hub_->UpdateSharedProperties(addressChar_, pProp->GetName(), tmpstr.c_str()) );
   }
   return DEVICE_OK;
}

int CXYStage::OnRBTrigger(MM::PropertyBase* pProp, MM::ActionType eAct)
{
   std::ostringstream command;
   if (eAct == MM::BeforeGet) {
      pProp->Set(g_IdleState);
   }
   else  if (eAct == MM::AfterSet) {
      if (hub_->UpdatingSharedProperties())
         return DEVICE_OK;
      std::string tmpstr;
      pProp->Get(tmpstr);
      if (tmpstr == g_DoItState)
      {
         command << addressChar_ << "RM";
         RETURN_ON_MM_ERROR ( hub_->QueryCommandVerify(command.str(), ":A") );
         pProp->Set(g_DoneState);
         command.str(""); command << g_DoneState;
         RETURN_ON_MM_ERROR ( hub_->UpdateSharedProperties(addressChar_, pProp->GetName(), command.str()) );
      }
   }
   return DEVICE_OK;
}

int CXYStage::OnRBRunning(MM::PropertyBase* pProp, MM::ActionType eAct)
{
   std::ostringstream command;
   std::ostringstream response;
   std::string pseudoAxisChar = FirmwareVersionAtLeast(2.89) ? "F" : "X";
   long tmp = 0;
   static bool justSet;
   if (eAct == MM::BeforeGet)
   {
      if (!refreshProps_ && initialized_ && !justSet)
         return DEVICE_OK;
      command << addressChar_ << "RM " << pseudoAxisChar << "?";
      response << ":A " << pseudoAxisChar << "=";
      RETURN_ON_MM_ERROR( hub_->QueryCommandVerify(command.str(), response.str()) );
      RETURN_ON_MM_ERROR( hub_->ParseAnswerAfterEquals(tmp) );
      bool success;
      if (tmp >= 128)
      {
         success = pProp->Set(g_YesState);
      }
      else
      {
         success = pProp->Set(g_NoState);
      }
      if (!success)
         return DEVICE_INVALID_PROPERTY_VALUE;
      justSet = false;
   }
   else if (eAct == MM::AfterSet)
   {
      justSet = true;
      return OnRBRunning(pProp, MM::BeforeGet);
      // TODO determine how to handle this with shared properties since ring buffer is per-card and not per-axis
      // the reason this property exists (and why it's not a read-only property) are a bit hazy as of mid-2017
   }
   return DEVICE_OK;
}

int CXYStage::OnRBDelayBetweenPoints(MM::PropertyBase* pProp, MM::ActionType eAct)
{
   std::ostringstream command;
   long tmp = 0;
   if (eAct == MM::BeforeGet)
   {
      if (!refreshProps_ && initialized_)
         return DEVICE_OK;
      command << addressChar_ << "RT Z?";
      RETURN_ON_MM_ERROR( hub_->QueryCommandVerify(command.str(), ":A Z="));
      RETURN_ON_MM_ERROR( hub_->ParseAnswerAfterEquals(tmp) );
      if (!pProp->Set(tmp))
         return DEVICE_INVALID_PROPERTY_VALUE;
   }
   else if (eAct == MM::AfterSet) {
      if (hub_->UpdatingSharedProperties())
         return DEVICE_OK;
      pProp->Get(tmp);
      command << addressChar_ << "RT Z=" << tmp;
      RETURN_ON_MM_ERROR ( hub_->QueryCommandVerify(command.str(), ":A") );
      command.str(""); command << tmp;
      RETURN_ON_MM_ERROR ( hub_->UpdateSharedProperties(addressChar_, pProp->GetName(), command.str()) );
   }
   return DEVICE_OK;
}

int CXYStage::OnUseSequence(MM::PropertyBase* pProp, MM::ActionType eAct)
{
   std::ostringstream command;
   if (eAct == MM::BeforeGet)
   {
      if (ttl_trigger_enabled_)
         pProp->Set(g_YesState);
      else
         pProp->Set(g_NoState);
   }
   else if (eAct == MM::AfterSet) {
      std::string tmpstr;
      pProp->Get(tmpstr);
      ttl_trigger_enabled_ = ttl_trigger_supported_ && (tmpstr == g_YesState);
      return OnUseSequence(pProp, MM::BeforeGet);  // refresh value
   }
   return DEVICE_OK;
}


int CXYStage::OnVectorGeneric(MM::PropertyBase* pProp, MM::ActionType eAct, const std::string& axisLetter)
{
   std::ostringstream command;
   std::ostringstream response;
   double tmp = 0;
   if (eAct == MM::BeforeGet)
   {
      if (!refreshProps_ && initialized_ && !refreshOverride_)
         return DEVICE_OK;
      refreshOverride_ = false;
      command << "VE " << axisLetter << "?";
      response << ":A " << axisLetter << "=";
      RETURN_ON_MM_ERROR( hub_->QueryCommandVerify(command.str(), response.str()));
      RETURN_ON_MM_ERROR( hub_->ParseAnswerAfterEquals(tmp) );
      if (!pProp->Set(tmp))
         return DEVICE_INVALID_PROPERTY_VALUE;
   }
   else if (eAct == MM::AfterSet) {
      pProp->Get(tmp);
      command << "VE " << axisLetter << "=" << tmp;
      RETURN_ON_MM_ERROR ( hub_->QueryCommandVerify(command.str(), ":A") );

   }
   return DEVICE_OK;
}

