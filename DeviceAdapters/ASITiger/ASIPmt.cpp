///////////////////////////////////////////////////////////////////////////////
// FILE:          ASIPmt.c
// PROJECT:       Micro-Manager
// SUBSYSTEM:     DeviceAdapters
//-----------------------------------------------------------------------------
// DESCRIPTION:   ASI PMT device adapter
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
// AUTHOR:        Vikram Kopuri (vik@asiimaging.com) 04/2016
//
// BASED ON:      ASILED.c and others
//

#include "ASIPmt.h"
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
#include <vector>


///////////////////////////////////////////////////////////////////////////////
// CPMT
//
CPMT::CPMT(const char* name) :
   ASIPeripheralBase< ::CSignalIOBase, CPMT >(name),
   channel_(1),
   channelAxisChar_('X'), 
   axisLetter_(g_EmptyAxisLetterStr),
   gain_(0),
   avg_length_(0)
{
   //Figure out what channel we are on
   if (IsExtendedName(name))  // only set up these properties if we have the required information in the name
   {
      channel_= GetChannelFromExtName(name);
	   axisLetter_ = GetAxisLetterFromExtName(name);
   }

   //Pick AxisChar to use.
   switch(channel_)
   {
   case 1:
      channelAxisChar_='X';
      break;
   case 2:
      channelAxisChar_='Y';
      break;
   case 3: 
      channelAxisChar_='Z';
      break;
   case 4:
      channelAxisChar_='F';
      break;
   case 5:
      channelAxisChar_='T';
      break;
   case 6:
      channelAxisChar_='R';
      break;
   default:
      channelAxisChar_='X';
      break;
   }
}

int CPMT::Initialize()
{
   // call generic Initialize first, this gets hub
   RETURN_ON_MM_ERROR( PeripheralInitialize() );

   // create MM description; this doesn't work during hardware configuration wizard but will work afterwards
   std::ostringstream command;
   command << g_PMTDeviceDescription << " HexAddr=" << addressString_<<" Axis Char="<<axisLetter_<<" Channel="<<channel_<<":"<<channelAxisChar_;
   CreateProperty(MM::g_Keyword_Description, command.str().c_str(), MM::String, true);
   
   CPropertyAction* pAct;

   //PMT Gain
   pAct = new CPropertyAction (this, &CPMT::OnGain);
   CreateProperty(g_PMTGainPropertyName, "0", MM::Integer, false, pAct);
   SetPropertyLimits(g_PMTGainPropertyName, 0, 1000);
   UpdateProperty(g_PMTGainPropertyName);  

   //ADC Avg
   pAct = new CPropertyAction (this, &CPMT::OnAverage);
   CreateProperty(g_PMTAVGPropertyName, "1", MM::Integer, false, pAct);
   SetPropertyLimits(g_PMTAVGPropertyName, 0, 5);
   UpdateProperty(g_PMTAVGPropertyName);  

   // refresh properties from controller every time; default is false = no refresh (speeds things up by not redoing so much serial comm)
   pAct = new CPropertyAction (this, &CPMT::OnRefreshProperties);
   CreateProperty(g_RefreshPropValsPropertyName, g_NoState, MM::String, false, pAct);
   AddAllowedValue(g_RefreshPropValsPropertyName, g_NoState);
   AddAllowedValue(g_RefreshPropValsPropertyName, g_YesState);

   // save settings to controller if requested
   pAct = new CPropertyAction (this, &CPMT::OnSaveCardSettings);
   CreateProperty(g_SaveSettingsPropertyName, g_SaveSettingsOrig, MM::String, false, pAct);
   AddAllowedValue(g_SaveSettingsPropertyName, g_SaveSettingsX);
   AddAllowedValue(g_SaveSettingsPropertyName, g_SaveSettingsY);
   AddAllowedValue(g_SaveSettingsPropertyName, g_SaveSettingsZ);
   AddAllowedValue(g_SaveSettingsPropertyName, g_SaveSettingsOrig);
   AddAllowedValue(g_SaveSettingsPropertyName, g_SaveSettingsDone);

   //Overload Reset
   pAct = new CPropertyAction (this, &CPMT::OnOverloadReset);
   CreateProperty(g_PMTOverloadReset,  g_OffState, MM::String, false, pAct);
   AddAllowedValue(g_PMTOverloadReset, g_OffState);
   AddAllowedValue(g_PMTOverloadReset, g_OnState);
   AddAllowedValue(g_PMTOverloadReset, g_PMTOverloadDone);

   //PMT signal, ADC readout
   pAct = new CPropertyAction (this, &CPMT::OnPMTSignal);
   CreateProperty(g_PMTSignal, "0", MM::Integer, true, pAct);
   UpdateProperty(g_PMTSignal);

   //PMT Overload
   pAct = new CPropertyAction (this, &CPMT::OnPMTOverload);
   CreateProperty(g_PMTOverload, g_NoState, MM::String, true, pAct);
   AddAllowedValue(g_PMTOverload, g_NoState);
   AddAllowedValue(g_PMTOverload, g_YesState);
   UpdateProperty(g_PMTOverload);

   initialized_ = true;
   return DEVICE_OK;
}

// This is the overload reset 
int CPMT::SetGateOpen(bool open)
{
   std::ostringstream command;
   if (open)
      command << addressChar_ << "LK " << channelAxisChar_ ;
   else
   {
     // can't do opposite the reset
   }
   RETURN_ON_MM_ERROR( hub_->QueryCommandVerify(command.str(), ":A") );
   return DEVICE_OK;
}

// Get overload status
int CPMT::GetGateOpen(bool& open)
{
   unsigned int val;
   std::ostringstream command;
   command << addressChar_ << "LK " << channelAxisChar_ << "?" ;
   // reply is 0 or 1 , 0 is overloaded , 1 is enabled
   RETURN_ON_MM_ERROR ( hub_->QueryCommandVerify(command.str(), ":A") );
   RETURN_ON_MM_ERROR ( hub_->ParseAnswerAfterPosition2(val) );
   open = val ? true : false;  // cast to boolean
   return DEVICE_OK;
}

// Get PMT's ADC reading
int CPMT::GetSignal(double& volts)
{
   unsigned int val;
   std::ostringstream command;
   command << addressChar_ << "RA " << channelAxisChar_ << "?" ;
   // reply is 0 or 1 , 0 is overloaded , 1 is enabled
   RETURN_ON_MM_ERROR ( hub_->QueryCommandVerify(command.str(), ":A") );
   RETURN_ON_MM_ERROR ( hub_->ParseAnswerAfterPosition2(val) );
   volts = val;
   return DEVICE_OK;
}

// updates PMT gain device property via the controller
int CPMT::UpdateGain()
{
   std::ostringstream command;
   std::ostringstream replyprefix;
   long tmp = 0;
   command << addressChar_ << "WRDAC " << channelAxisChar_ << "?";
   replyprefix << channelAxisChar_ << "=";
   RETURN_ON_MM_ERROR( hub_->QueryCommandVerify(command.str(), replyprefix.str()) );
   RETURN_ON_MM_ERROR( hub_->ParseAnswerAfterEquals(tmp) );
   gain_ = tmp;

   return DEVICE_OK;
}

// updates PMT average length property via the controller
int CPMT::UpdateAvg()
{
   std::ostringstream command;
   std::ostringstream replyprefix;
   long tmp = 0;
   command << "E " << axisLetter_ << "?";
   replyprefix << ":" << axisLetter_ << "=";
   RETURN_ON_MM_ERROR( hub_->QueryCommandVerify(command.str(), replyprefix.str()) );
   RETURN_ON_MM_ERROR( hub_->ParseAnswerAfterEquals(tmp) );
   avg_length_ = tmp;

   return DEVICE_OK;
}

// action handlers

int CPMT::OnSaveCardSettings(MM::PropertyBase* pProp, MM::ActionType eAct)
{
   std::string tmpstr;
   std::ostringstream command;
   if (eAct == MM::AfterSet) {
      if (hub_->UpdatingSharedProperties())
         return DEVICE_OK;
      pProp->Get(tmpstr);
      command << addressChar_ << "SS ";
      if (tmpstr == g_SaveSettingsOrig)
         return DEVICE_OK;
      if (tmpstr == g_SaveSettingsDone)
         return DEVICE_OK;
      if (tmpstr == g_SaveSettingsX)
         command << 'X';
      else if (tmpstr == g_SaveSettingsY)
         command << 'Y';
      else if (tmpstr == g_SaveSettingsZ)
         command << 'Z';
      RETURN_ON_MM_ERROR( hub_->QueryCommandVerify(command.str(), ":A", (long)200) );  // note 200ms delay added
      pProp->Set(g_SaveSettingsDone);
      command.str(""); command << g_SaveSettingsDone;
      RETURN_ON_MM_ERROR ( hub_->UpdateSharedProperties(addressChar_, pProp->GetName(), command.str()) );
   }
   return DEVICE_OK;
}

int CPMT::OnOverloadReset(MM::PropertyBase* pProp, MM::ActionType eAct)
{
   std::string tmpstr;
   std::ostringstream command;
   if (eAct == MM::AfterSet) {
      pProp->Get(tmpstr);
      if (tmpstr == g_OffState)
         return DEVICE_OK;
      else if (tmpstr == g_PMTOverloadDone)
         return DEVICE_OK;
	  else if (tmpstr == g_OnState)
         command << addressChar_ << "LK " << channelAxisChar_ ;
      RETURN_ON_MM_ERROR( hub_->QueryCommandVerify(command.str(), ":A", (long)200) );  // note 200ms delay added
      pProp->Set(g_PMTOverloadDone);
   }
   return DEVICE_OK;
}

int CPMT::OnRefreshProperties(MM::PropertyBase* pProp, MM::ActionType eAct)
{
    if (eAct == MM::AfterSet)
    {
        std::string tmpstr;
        pProp->Get(tmpstr);
        refreshProps_ = (tmpstr == g_YesState) ? true : false;
    }
    return DEVICE_OK;
}

//Get and Set PMT Gain
int CPMT::OnGain(MM::PropertyBase* pProp, MM::ActionType eAct)
{
   std::ostringstream command;
   long tmp = 0;
   if (eAct == MM::BeforeGet)
   { //Query the controller for gain
      if (!refreshProps_ && initialized_)
         return DEVICE_OK;
      UpdateGain();  // will set gain_ 
      if (!pProp->Set((long)gain_))
         return DEVICE_INVALID_PROPERTY_VALUE;
   }
   else if (eAct == MM::AfterSet) {
      pProp->Get(tmp);
         command << addressChar_ << "WRDAC " << channelAxisChar_ << "=" << tmp;
         RETURN_ON_MM_ERROR( hub_->QueryCommandVerify(command.str(), ":A") );
      gain_ = tmp;
   }
   return DEVICE_OK;
}

//Get and Set PMT Average length
int CPMT::OnAverage(MM::PropertyBase* pProp, MM::ActionType eAct)
{
   std::ostringstream command;
   long tmp = 0;
   if (eAct == MM::BeforeGet)
   { //Query the controller for gain
      if (!refreshProps_ && initialized_)
         return DEVICE_OK;
      UpdateAvg();  // will set avg_length_ 
      if (!pProp->Set((long)avg_length_ ))
         return DEVICE_INVALID_PROPERTY_VALUE;
   }
   else if (eAct == MM::AfterSet) {
      pProp->Get(tmp);
      command << "E " << axisLetter_ << "=" << tmp;
      RETURN_ON_MM_ERROR( hub_->QueryCommandVerify(command.str(), ":A") );
      avg_length_ = tmp;
   }
   return DEVICE_OK;
}

int CPMT::OnPMTSignal(MM::PropertyBase* pProp, MM::ActionType eAct)
{
   unsigned int val;
   std::ostringstream command;
   if (eAct == MM::BeforeGet || eAct == MM::AfterSet)
   {
      // always read
      command << addressChar_ << "RA " << channelAxisChar_ << "?" ;
      RETURN_ON_MM_ERROR ( hub_->QueryCommandVerify(command.str(), ":A") );
      RETURN_ON_MM_ERROR ( hub_->ParseAnswerAfterPosition2(val) );
      if (!pProp->Set((long)val))
         return DEVICE_INVALID_PROPERTY_VALUE;
   }
   return DEVICE_OK;
}

int CPMT::OnPMTOverload(MM::PropertyBase* pProp, MM::ActionType eAct)
{
   unsigned int val;
   std::ostringstream command;
   if (eAct == MM::BeforeGet || eAct == MM::AfterSet)
   {
      // always read
      command << addressChar_ << "LK " << channelAxisChar_ << "?" ;
      RETURN_ON_MM_ERROR ( hub_->QueryCommandVerify(command.str(), ":A") );
      RETURN_ON_MM_ERROR ( hub_->ParseAnswerAfterPosition2(val) );
      if(val)
	  {
	  if (!pProp->Set(g_NoState))
         return DEVICE_INVALID_PROPERTY_VALUE;
	  }
	  else
	  {
	  	  if (!pProp->Set(g_YesState))
         return DEVICE_INVALID_PROPERTY_VALUE;
	  }
   }
   return DEVICE_OK;
}
