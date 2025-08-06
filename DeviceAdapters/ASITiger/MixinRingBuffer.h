///////////////////////////////////////////////////////////////////////////////
// FILE:          MixinRingBuffer.h
// PROJECT:       Micro-Manager
// SUBSYSTEM:     DeviceAdapters
//-----------------------------------------------------------------------------
// DESCRIPTION:   ASI mixin for ring buffer properties
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
// AUTHOR:        Brandon Simpson (brandon@asiimaging.com) 07/2025
//

#ifndef MIXIN_RINGBUFFER_H
#define MIXIN_RINGBUFFER_H
 
#include "ASITiger.h"

#include <string>

// A mixin that adds ring buffer properties to an ASI device.
template <typename T>
class MixinRingBuffer {
private:
    T* GetDerived() { return static_cast<T*>(this); }
    const T* GetDerived() const { return static_cast<const T*>(this); }

public:
    // Create ring buffer properties, call this in type T's Initialize() function.
    void MixinCreateRingBufferProperties() {
        CreateRingBufferModeProperty();
        CreateRingBufferDelayProperty();
        CreateRingBufferTriggerProperty();
        CreateRingBufferAutoplayRunningProperty();
    }

private:
    void CreateRingBufferModeProperty() {
        T* derived = GetDerived();

        const std::string axisLetter = derived->FirmwareVersionAtLeast(2.89) ? "F" : "X";

        const std::string query = derived->GetAddressChar() + "RM " + axisLetter + "?";
        const std::string command = derived->GetAddressChar() + "RM " + axisLetter + "=";
        const std::string response = ":A " + axisLetter + "=";

        derived->CreateStringProperty(
            g_RB_ModePropertyName, g_RB_OnePoint_1, false,
            new MM::ActionLambda([derived, query, command, response](MM::PropertyBase* pProp, MM::ActionType eAct) {
                long tmp;
                if (eAct == MM::BeforeGet) {
                    if (!derived->GetRefreshProps() && derived->GetInitialized()) {
                        return DEVICE_OK;
                    }
                    RETURN_ON_MM_ERROR(derived->GetHub()->QueryCommandVerify(query, response));
                    RETURN_ON_MM_ERROR(derived->GetHub()->ParseAnswerAfterEquals(tmp));
                    if (tmp >= 128) {
                        tmp -= 128;  // remove the "running now" code if present
                    }
                    bool success;
                    switch (tmp) {
                    case 0:
                        success = pProp->Set(g_RB_PlayConsume_0);
                        break;
                    case 1:
                        success = pProp->Set(g_RB_OnePoint_1);
                        break;
                    case 2:
                        success = pProp->Set(g_RB_PlayOnce_2);
                        break;
                    case 3:
                        success = pProp->Set(g_RB_PlayRepeat_3);
                        break;
                    case 4:
                        success = pProp->Set(g_RB_PlayOnce_NoReturn_4);
                        break;
                    default:
                        success = false;
                        break;
                    }
                    if (!success) {
                        return DEVICE_INVALID_PROPERTY_VALUE;
                    }
                } else if (eAct == MM::AfterSet) {
                    if (derived->GetHub()->UpdatingSharedProperties()) {
                        return DEVICE_OK;
                    }
                    std::string tmpstr;
                    pProp->Get(tmpstr);
                    if (tmpstr == g_RB_OnePoint_1) {
                        tmp = 1;
                    } else if (tmpstr == g_RB_PlayOnce_2) {
                        tmp = 2;
                    } else if (tmpstr == g_RB_PlayRepeat_3) {
                        tmp = 3;
                    } else {
                        return DEVICE_INVALID_PROPERTY_VALUE;
                    }
                    RETURN_ON_MM_ERROR(derived->GetHub()->QueryCommandVerify(command + std::to_string(tmp), ":A"));
                    RETURN_ON_MM_ERROR(derived->GetHub()->UpdateSharedProperties(derived->GetAddressChar(), pProp->GetName(), tmpstr.c_str()));
                }
                return DEVICE_OK;
            }
        ));

        if (derived->FirmwareVersionAtLeast(3.41)) {
            derived->AddAllowedValue(g_RB_ModePropertyName, g_RB_PlayConsume_0);
        }
        derived->AddAllowedValue(g_RB_ModePropertyName, g_RB_OnePoint_1);
        derived->AddAllowedValue(g_RB_ModePropertyName, g_RB_PlayOnce_2);
        derived->AddAllowedValue(g_RB_ModePropertyName, g_RB_PlayRepeat_3);
        if (derived->FirmwareVersionAtLeast(3.45)) {
            derived->AddAllowedValue(g_RB_ModePropertyName, g_RB_PlayOnce_NoReturn_4);
        }
        derived->UpdateProperty(g_RB_ModePropertyName);
    }

    void CreateRingBufferDelayProperty() {
        T* derived = GetDerived();

        const std::string query = derived->GetAddressChar() + "RT Z?";
        const std::string command = derived->GetAddressChar() + "RT Z=";

        derived->CreateIntegerProperty(
            g_RB_DelayPropertyName, 0, false,
            new MM::ActionLambda([derived, query, command](MM::PropertyBase* pProp, MM::ActionType eAct) {
                long tmp = 0;
                if (eAct == MM::BeforeGet) {
                    if (!derived->GetRefreshProps() && derived->GetInitialized()) {
                        return DEVICE_OK;
                    }
                    RETURN_ON_MM_ERROR(derived->GetHub()->QueryCommandVerify(query, ":A Z="));
                    RETURN_ON_MM_ERROR(derived->GetHub()->ParseAnswerAfterEquals(tmp));
                    if (!pProp->Set(tmp)) {
                        return DEVICE_INVALID_PROPERTY_VALUE;
                    }
                } else if (eAct == MM::AfterSet) {
                    if (derived->GetHub()->UpdatingSharedProperties()) {
                        return DEVICE_OK;
                    }
                    pProp->Get(tmp);
                    const std::string tmpstr = std::to_string(tmp);
                    RETURN_ON_MM_ERROR(derived->GetHub()->QueryCommandVerify(command + tmpstr, ":A"));
                    RETURN_ON_MM_ERROR(derived->GetHub()->UpdateSharedProperties(derived->GetAddressChar(), pProp->GetName(), tmpstr));
                }
                return DEVICE_OK;
            }
        ));

        derived->UpdateProperty(g_RB_DelayPropertyName);
    }

    void CreateRingBufferTriggerProperty() {
        T* derived = GetDerived();

        const std::string command = derived->GetAddressChar() + "RM";

        derived->CreateStringProperty(
            g_RB_TriggerPropertyName, g_IdleState, false,
            new MM::ActionLambda([derived, command](MM::PropertyBase* pProp, MM::ActionType eAct) {
                if (eAct == MM::BeforeGet) {
                    pProp->Set(g_IdleState);
                } else if (eAct == MM::AfterSet) {
                    if (derived->GetHub()->UpdatingSharedProperties()) {
                        return DEVICE_OK;
                    }
                    std::string tmpstr;
                    pProp->Get(tmpstr);
                    if (tmpstr == g_DoItState) {
                        RETURN_ON_MM_ERROR(derived->GetHub()->QueryCommandVerify(command, ":A"));
                        pProp->Set(g_DoneState);
                    }
                }
                return DEVICE_OK;
            }
        ));

        derived->AddAllowedValue(g_RB_TriggerPropertyName, g_IdleState, 0);
        derived->AddAllowedValue(g_RB_TriggerPropertyName, g_DoItState, 1);
        derived->AddAllowedValue(g_RB_TriggerPropertyName, g_DoneState, 2);
        derived->UpdateProperty(g_RB_TriggerPropertyName);
    }

    void CreateRingBufferAutoplayRunningProperty() {
        T* derived = GetDerived();

        const std::string axisLetter = derived->FirmwareVersionAtLeast(2.89) ? "F" : "X";

        const std::string query = derived->GetAddressChar() + "RM " + axisLetter + "?";
        const std::string response = ":A " + axisLetter + "=";

        derived->CreateStringProperty(
            g_RB_AutoplayRunningPropertyName, "No", false,
            new MM::ActionLambda([derived, query, response](MM::PropertyBase* pProp, MM::ActionType eAct) {
                long tmp = 0;
                static bool justSet;
                if (eAct == MM::BeforeGet) {
                    if (!derived->GetRefreshProps() && derived->GetInitialized() && !justSet) {
                        return DEVICE_OK;
                    }
                    RETURN_ON_MM_ERROR(derived->GetHub()->QueryCommandVerify(query, response));
                    RETURN_ON_MM_ERROR(derived->GetHub()->ParseAnswerAfterEquals(tmp));
                    bool success;
                    if (tmp >= 128) {
                        success = pProp->Set("Yes");
                    } else {
                        success = pProp->Set("No");
                    }
                    if (!success) {
                        return DEVICE_INVALID_PROPERTY_VALUE;
                    }
                    justSet = false;
                } else if (eAct == MM::AfterSet) {
                    justSet = true;
                    // FIXME: how to do this in the mixin
                    //return OnRBRunning(pProp, MM::BeforeGet);
                    // TODO determine how to handle this with shared properties since ring buffer is per-card and not per-axis
                    // the reason this property exists (and why it's not a read-only property) are a bit hazy as of mid-2017
                }
                return DEVICE_OK;
            }
        ));

        derived->AddAllowedValue(g_RB_AutoplayRunningPropertyName, "No");
        derived->AddAllowedValue(g_RB_AutoplayRunningPropertyName, "Yes");
        derived->UpdateProperty(g_RB_AutoplayRunningPropertyName);
    }

};

#endif // MIXIN_RINGBUFFER_H
