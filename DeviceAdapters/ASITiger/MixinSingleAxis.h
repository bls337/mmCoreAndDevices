///////////////////////////////////////////////////////////////////////////////
// FILE:          MixinSingleAxis.h
// PROJECT:       Micro-Manager
// SUBSYSTEM:     DeviceAdapters
//-----------------------------------------------------------------------------
// DESCRIPTION:   ASI mixin for single axis properties
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

#ifndef MIXIN_SINGLEAXIS_H
#define MIXIN_SINGLEAXIS_H

#include "ASITiger.h"

#include <iostream>
#include <sstream>
#include <string>

// A mixin that adds single axis properties to an ASI device.
template <typename T>
class MixinSingleAxis {
private:
    T* GetDerived() { return static_cast<T*>(this); }
    const T* GetDerived() const { return static_cast<const T*>(this); }

public:
    // Create single axis properties
    void MixinCreateSingleAxisProperties() {
        T* derived = GetDerived();

        CreateSingleAxisAmplitudeProperty(g_SAAmplitudePropertyName, derived->GetAxisLetter());
        CreateSingleAxisOffsetProperty(g_SAOffsetPropertyName, derived->GetAxisLetter());
        CreateSingleAxisPeriodProperty(g_SAPeriodPropertyName, derived->GetAxisLetter());
        CreateSingleAxisModeProperty(g_SAModePropertyName, derived->GetAxisLetter());
        CreateSingleAxisPatternProperty(g_SAPatternPropertyName, derived->GetAxisLetter());
        CreateSingleAxisAdvancedProperties();
    }

private:
    void CreateSingleAxisAmplitudeProperty(const std::string& propertyName, const std::string& axisLetter) {
        T* derived = GetDerived();

        derived->CreateFloatProperty(
            propertyName.c_str(), 0.0, false,
            new MM::ActionLambda([derived, axisLetter](MM::PropertyBase* pProp, MM::ActionType eAct) {
                std::ostringstream command;
                std::ostringstream response;
                double tmp = 0;
                if (eAct == MM::BeforeGet) {
                    if (!derived->GetRefreshProps() && derived->GetInitialized()) {
                        return DEVICE_OK;
                    }
                    command << "SAA " << axisLetter << "?";
                    response << ":A " << axisLetter << "=";
                    RETURN_ON_MM_ERROR(derived->GetHub()->QueryCommandVerify(command.str(), response.str()));
                    RETURN_ON_MM_ERROR(derived->GetHub()->ParseAnswerAfterEquals(tmp));
                    tmp = tmp / unitMult_;
                    if (!pProp->Set(tmp)) {
                        return DEVICE_INVALID_PROPERTY_VALUE;
                    }
                } else if (eAct == MM::AfterSet) {
                    pProp->Get(tmp);
                    command << "SAA " << axisLetter << "=" << tmp * unitMult_;
                    RETURN_ON_MM_ERROR(derived->GetHub()->QueryCommandVerify(command.str(), ":A"));
                }
                return DEVICE_OK;
            }
        ));
        derived->UpdateProperty(propertyName.c_str());
    }

    void CreateSingleAxisOffsetProperty(const std::string& propertyName, const std::string& axisLetter) {
        T* derived = GetDerived();

        derived->CreateFloatProperty(
            propertyName.c_str(), 0.0, false,
            new MM::ActionLambda([derived, axisLetter](MM::PropertyBase* pProp, MM::ActionType eAct) {
                std::ostringstream command;
                std::ostringstream response;
                double tmp = 0;
                if (eAct == MM::BeforeGet) {
                    if (!derived->GetRefreshProps() && derived->GetInitialized()) {
                        return DEVICE_OK;
                    }
                    command << "SAO " << axisLetter << "?";
                    response << ":A " << axisLetter << "=";
                    RETURN_ON_MM_ERROR(derived->GetHub()->QueryCommandVerify(command.str(), response.str()));
                    RETURN_ON_MM_ERROR(derived->GetHub()->ParseAnswerAfterEquals(tmp));
                    tmp = tmp / unitMult_;
                    if (!pProp->Set(tmp)) {
                        return DEVICE_INVALID_PROPERTY_VALUE;
                    }
                } else if (eAct == MM::AfterSet) {
                    pProp->Get(tmp);
                    command << "SAO " << axisLetter << "=" << tmp * unitMult_;
                    RETURN_ON_MM_ERROR(derived->GetHub()->QueryCommandVerify(command.str(), ":A"));
                }
                return DEVICE_OK;
            }
        ));
        derived->UpdateProperty(propertyName.c_str());
    }

    void CreateSingleAxisPeriodProperty(const std::string& propertyName, const std::string& axisLetter) {
        T* derived = GetDerived();

        derived->CreateIntegerProperty(
            propertyName.c_str(), 0, false,
            new MM::ActionLambda([derived, axisLetter](MM::PropertyBase* pProp, MM::ActionType eAct) {
                std::ostringstream command;
                std::ostringstream response;
                long tmp = 0;
                if (eAct == MM::BeforeGet) {
                    if (!derived->GetRefreshProps() && derived->GetInitialized()) {
                        return DEVICE_OK;
                    }
                    command << "SAF " << axisLetter << "?";
                    response << ":A " << axisLetter << "=";
                    RETURN_ON_MM_ERROR(derived->GetHub()->QueryCommandVerify(command.str(), response.str()));
                    RETURN_ON_MM_ERROR(derived->GetHub()->ParseAnswerAfterEquals(tmp));
                    if (!pProp->Set(tmp)) {
                        return DEVICE_INVALID_PROPERTY_VALUE;
                    }
                } else if (eAct == MM::AfterSet) {
                    pProp->Get(tmp);
                    command << "SAF " << axisLetter << "=" << tmp;
                    RETURN_ON_MM_ERROR(derived->GetHub()->QueryCommandVerify(command.str(), ":A"));
                }
                return DEVICE_OK;
            }
        ));
        derived->UpdateProperty(propertyName.c_str());
    }

    void CreateSingleAxisModeProperty(const std::string& propertyName, const std::string& axisLetter) {
        T* derived = GetDerived();

        derived->CreateStringProperty(
            propertyName.c_str(), g_SAMode_0, false,
            new MM::ActionLambda([derived, axisLetter](MM::PropertyBase* pProp, MM::ActionType eAct) {
                static bool justSet = false;
                std::ostringstream command;
                std::ostringstream response;
                long tmp = 0;
                if (eAct == MM::BeforeGet) {
                    if (!derived->GetRefreshProps() && derived->GetInitialized() && !justSet) {
                        return DEVICE_OK;
                    }
                    command << "SAM " << axisLetter << "?";
                    response << ":A " << axisLetter << "=";
                    RETURN_ON_MM_ERROR(derived->GetHub()->QueryCommandVerify(command.str(), response.str()));
                    RETURN_ON_MM_ERROR(derived->GetHub()->ParseAnswerAfterEquals(tmp));
                    bool success;
                    switch (tmp) {
                    case 0:
                        success = pProp->Set(g_SAMode_0);
                        break;
                    case 1:
                        success = pProp->Set(g_SAMode_1);
                        break;
                    case 2:
                        success = pProp->Set(g_SAMode_2);
                        break;
                    case 3:
                        success = pProp->Set(g_SAMode_3);
                        break;
                    default:
                        success = 0;
                        break;
                    }
                    if (!success) {
                        return DEVICE_INVALID_PROPERTY_VALUE;
                    }
                    justSet = false;
                } else if (eAct == MM::AfterSet) {
                    std::string tmpstr;
                    pProp->Get(tmpstr);
                    if (tmpstr == g_SAMode_0) {
                        tmp = 0;
                    } else if (tmpstr == g_SAMode_1) {
                        tmp = 1;
                    } else if (tmpstr == g_SAMode_2) {
                        tmp = 2;
                    } else if (tmpstr == g_SAMode_3) {
                        tmp = 3;
                    } else {
                        return DEVICE_INVALID_PROPERTY_VALUE;
                    }
                    command << "SAM " << axisLetter << "=" << tmp;
                    RETURN_ON_MM_ERROR(derived->GetHub()->QueryCommandVerify(command.str(), ":A"));
                    // get the updated value right away
                    justSet = true;
                    // FIXME: how to do this in the mixin
                    //return OnSAMode(pProp, MM::BeforeGet);
                    return DEVICE_OK;
                }
                return DEVICE_OK;
            }
         ));
        derived->AddAllowedValue(g_SAModePropertyName, g_SAMode_0);
        derived->AddAllowedValue(g_SAModePropertyName, g_SAMode_1);
        derived->AddAllowedValue(g_SAModePropertyName, g_SAMode_2);
        derived->AddAllowedValue(g_SAModePropertyName, g_SAMode_3);
        derived->UpdateProperty(propertyName.c_str());
    }

    void CreateSingleAxisPatternProperty(const std::string& propertyName, const std::string& axisLetter) {
        T* derived = GetDerived();

        derived->CreateStringProperty(
            propertyName.c_str(), g_SAPattern_0, false,
            new MM::ActionLambda([derived, axisLetter](MM::PropertyBase* pProp, MM::ActionType eAct) {
                std::ostringstream command;
                std::ostringstream response;
                long tmp = 0;
                if (eAct == MM::BeforeGet) {
                    if (!derived->GetRefreshProps() && derived->GetInitialized()) {
                        return DEVICE_OK;
                    }
                    command << "SAP " << axisLetter << "?";
                    response << ":A " << axisLetter << "=";
                    RETURN_ON_MM_ERROR(derived->GetHub()->QueryCommandVerify(command.str(), response.str()));
                    RETURN_ON_MM_ERROR(derived->GetHub()->ParseAnswerAfterEquals(tmp));
                    bool success;
                    tmp = tmp & ((long)(BIT2 | BIT1 | BIT0));  // zero all but the lowest 3 bits
                    switch (tmp) {
                    case 0:
                        success = pProp->Set(g_SAPattern_0);
                        break;
                    case 1:
                        success = pProp->Set(g_SAPattern_1);
                        break;
                    case 2:
                        success = pProp->Set(g_SAPattern_2);
                        break;
                    case 3:
                        success = pProp->Set(g_SAPattern_3);
                        break;
                    default:success = 0;
                        break;
                    }
                    if (!success) {
                        return DEVICE_INVALID_PROPERTY_VALUE;
                    }
                } else if (eAct == MM::AfterSet) {
                    std::string tmpstr;
                    pProp->Get(tmpstr);
                    if (tmpstr == g_SAPattern_0) {
                        tmp = 0;
                    } else if (tmpstr == g_SAPattern_1) {
                        tmp = 1;
                    } else if (tmpstr == g_SAPattern_2) {
                        tmp = 2;
                    } else if (tmpstr == g_SAPattern_3) {
                        tmp = 3;
                    } else {
                        return DEVICE_INVALID_PROPERTY_VALUE;
                    }
                    // have to get current settings and then modify bits 0-2 from there
                    command << "SAP " << axisLetter << "?";
                    response << ":A " << axisLetter << "=";
                    RETURN_ON_MM_ERROR(derived->GetHub()->QueryCommandVerify(command.str(), response.str()));
                    long current;
                    RETURN_ON_MM_ERROR(derived->GetHub()->ParseAnswerAfterEquals(current));
                    current = current & (~(long)(BIT2 | BIT1 | BIT0));  // set lowest 3 bits to zero
                    tmp += current;
                    command.str("");
                    command << "SAP " << axisLetter << "=" << tmp;
                    RETURN_ON_MM_ERROR(derived->GetHub()->QueryCommandVerify(command.str(), ":A"));
                }
                return DEVICE_OK;
            }
        ));
        derived->AddAllowedValue(propertyName.c_str(), g_SAPattern_0);
        derived->AddAllowedValue(propertyName.c_str(), g_SAPattern_1);
        derived->AddAllowedValue(propertyName.c_str(), g_SAPattern_2);
        // FIXME: add firmware version check
        derived->AddAllowedValue(propertyName.c_str(), g_SAPattern_3);
        derived->UpdateProperty(propertyName.c_str());
    }

    void CreateSingleAxisAdvancedProperties() {
        T* derived = GetDerived();

        derived->CreateStringProperty(
            g_AdvancedSAPropertiesPropertyName, "No", false,
            new MM::ActionLambda([derived](MM::PropertyBase* pProp, MM::ActionType eAct) {
                if (eAct == MM::BeforeGet) {
                    return DEVICE_OK; // do nothing
                } else if (eAct == MM::AfterSet) {
                    //CreateSingleAxisClockSourceProperty(g_SAClkSrcPropertyName, derived->GetAxisLetter());
                    //CreateSingleAxisClockPolarityProperty(g_SAClkPolPropertyName, derived->GetAxisLetter());
                    //CreateSingleAxisTTLOutputProperty(g_SATTLOutPropertyName, derived->GetAxisLetter());
                    //CreateSingleAxisTTLPolarityProperty(g_SATTLPolPropertyName, derived->GetAxisLetter());
                    //CreateSingleAxisPatternByteProperty(g_SAPatternModePropertyName, derived->GetAxisLetter());
                }
                return DEVICE_OK;
            }
        ));

        derived->AddAllowedValue(g_AdvancedSAPropertiesPropertyName, "No");
        derived->AddAllowedValue(g_AdvancedSAPropertiesPropertyName, "Yes");
        derived->UpdateProperty(g_AdvancedSAPropertiesPropertyName);
    }

    // Advanced properties

    void CreateSingleAxisClockSourceProperty(const std::string& propertyName, const std::string& axisLetter) {
        T* derived = GetDerived();

        derived->CreateStringProperty(
            propertyName.c_str(), g_SAClkSrc_0, false,
            new MM::ActionLambda([derived, axisLetter](MM::PropertyBase* pProp, MM::ActionType eAct) {
                std::ostringstream command;
                std::ostringstream response;
                long tmp = 0;
                if (eAct == MM::BeforeGet) {
                    if (!derived->GetRefreshProps() && derived->GetInitialized()) {
                        return DEVICE_OK;
                    }
                    command << "SAP " << axisLetter << "?";
                    response << ":A " << axisLetter << "=";
                    RETURN_ON_MM_ERROR(derived->GetHub()->QueryCommandVerify(command.str(), response.str()));
                    RETURN_ON_MM_ERROR(derived->GetHub()->ParseAnswerAfterEquals(tmp));
                    bool success;
                    tmp = tmp & ((long)(BIT7));  // zero all but bit 7
                    switch (tmp) {
                    case 0:
                        success = pProp->Set(g_SAClkSrc_0);
                        break;
                    case BIT7:
                        success = pProp->Set(g_SAClkSrc_1);
                        break;
                    default:
                        success = 0;
                        break;
                    }
                    if (!success) {
                        return DEVICE_INVALID_PROPERTY_VALUE;
                        }
                } else if (eAct == MM::AfterSet) {
                    std::string tmpstr;
                    pProp->Get(tmpstr);
                    if (tmpstr == g_SAClkSrc_0) {
                        tmp = 0;
                    } else if (tmpstr == g_SAClkSrc_1) {
                        tmp = BIT7;
                    } else {
                        return DEVICE_INVALID_PROPERTY_VALUE;
                    }
                    // have to get current settings and then modify bit 7 from there
                    command << "SAP " << axisLetter << "?";
                    response << ":A " << axisLetter << "=";
                    RETURN_ON_MM_ERROR(derived->GetHub()->QueryCommandVerify(command.str(), response.str()));
                    long current;
                    RETURN_ON_MM_ERROR(derived->GetHub()->ParseAnswerAfterEquals(current));
                    current = current & (~(long)(BIT7));  // clear bit 7
                    tmp += current;
                    command.str("");
                    command << "SAP " << axisLetter << "=" << tmp;
                    RETURN_ON_MM_ERROR(derived->GetHub()->QueryCommandVerify(command.str(), ":A"));
                }
                return DEVICE_OK;
            }
        ));
        derived->AddAllowedValues(g_SAClkSrc_0);
        derived->AddAllowedValues(g_SAClkSrc_1);
        derived->UpdateProperty(propertyName.c_str());
    }

    void CreateSingleAxisClockPolarityProperty(const std::string& propertyName, const std::string& axisLetter) {
        T* derived = GetDerived();

        derived->CreateStringProperty(
            propertyName.c_str(), g_SAClkPol_0, false,
            new MM::ActionLambda([derived, axisLetter](MM::PropertyBase* pProp, MM::ActionType eAct) {
                std::ostringstream command;
                std::ostringstream response;
                long tmp = 0;
                if (eAct == MM::BeforeGet) {
                    if (!derived->GetRefreshProps() && derived->GetInitialized()) {
                        return DEVICE_OK;
                    }
                    command << "SAP " << axisLetter << "?";
                    response << ":A " << axisLetter << "=";
                    RETURN_ON_MM_ERROR(derived->GetHub()->QueryCommandVerify(command.str(), response.str()));
                    RETURN_ON_MM_ERROR(derived->GetHub()->ParseAnswerAfterEquals(tmp));
                    bool success;
                    tmp = tmp & ((long)(BIT6));  // zero all but bit 6
                    switch (tmp) {
                    case 0:
                        success = pProp->Set(g_SAClkPol_0);
                        break;
                    case BIT6:
                        success = pProp->Set(g_SAClkPol_1);
                        break;
                    default:
                        success = 0;
                        break;
                    }
                    if (!success) {
                        return DEVICE_INVALID_PROPERTY_VALUE;
                    }
                } else if (eAct == MM::AfterSet) {
                    std::string tmpstr;
                    pProp->Get(tmpstr);
                    if (tmpstr == g_SAClkPol_0) {
                        tmp = 0;
                    } else if (tmpstr == g_SAClkPol_1) {
                        tmp = BIT6;
                    } else {
                        return DEVICE_INVALID_PROPERTY_VALUE;
                    }
                    // have to get current settings and then modify bit 6 from there
                    command << "SAP " << axisLetter << "?";
                    response << ":A " << axisLetter << "=";
                    RETURN_ON_MM_ERROR(derived->GetHub()->QueryCommandVerify(command.str(), response.str()));
                    long current;
                    RETURN_ON_MM_ERROR(derived->GetHub()->ParseAnswerAfterEquals(current));
                    current = current & (~(long)(BIT6));  // clear bit 6
                    tmp += current;
                    command.str("");
                    command << "SAP " << axisLetter << "=" << tmp;
                    RETURN_ON_MM_ERROR(derived->GetHub()->QueryCommandVerify(command.str(), ":A"));
                }
                return DEVICE_OK;
            }
        ));
        derived->AddAllowedValues(g_SAClkPol_0);
        derived->AddAllowedValues(g_SAClkPol_1);
        derived->UpdateProperty(propertyName.c_str());
    }

    void CreateSingleAxisTTLOutputProperty(const std::string& propertyName, const std::string& axisLetter) {
        T* derived = GetDerived();

        derived->CreateStringProperty(
            propertyName.c_str(), g_SATTLOut_0, false,
            new MM::ActionLambda([derived, axisLetter](MM::PropertyBase* pProp, MM::ActionType eAct) {
                std::ostringstream command;
                std::ostringstream response;
                long tmp = 0;
                if (eAct == MM::BeforeGet) {
                    if (!derived->GetRefreshProps() && derived->GetInitialized()) {
                        return DEVICE_OK;
                    }
                    command << "SAP " << axisLetter << "?";
                    response << ":A " << axisLetter << "=";
                    RETURN_ON_MM_ERROR(derived->GetHub()->QueryCommandVerify(command.str(), response.str()));
                    RETURN_ON_MM_ERROR(derived->GetHub()->ParseAnswerAfterEquals(tmp));
                    bool success;
                    tmp = tmp & ((long)(BIT5));  // zero all but bit 5
                    switch (tmp) {
                    case 0:
                        success = pProp->Set(g_SATTLOut_0);
                        break;
                    case BIT5:
                        success = pProp->Set(g_SATTLOut_1);
                        break;
                    default:success = 0;
                        break;
                    }
                    if (!success) {
                        return DEVICE_INVALID_PROPERTY_VALUE;
                    }
                } else if (eAct == MM::AfterSet) {
                    std::string tmpstr;
                    pProp->Get(tmpstr);
                    if (tmpstr == g_SATTLOut_0) {
                        tmp = 0;
                    } else if (tmpstr == g_SATTLOut_1) {
                        tmp = BIT5;
                    } else {
                        return DEVICE_INVALID_PROPERTY_VALUE;
                    }
                    // have to get current settings and then modify bit 5 from there
                    command << "SAP " << axisLetter << "?";
                    response << ":A " << axisLetter << "=";
                    RETURN_ON_MM_ERROR(derived->GetHub()->QueryCommandVerify(command.str(), response.str()));
                    long current;
                    RETURN_ON_MM_ERROR(derived->GetHub()->ParseAnswerAfterEquals(current));
                    current = current & (~(long)(BIT5));  // clear bit 5
                    tmp += current;
                    command.str("");
                    command << "SAP " << axisLetter << "=" << tmp;
                    RETURN_ON_MM_ERROR(derived->GetHub()->QueryCommandVerify(command.str(), ":A"));
                }
                return DEVICE_OK;
            }
        ));
        derived->AddAllowedValues(g_SATTLOut_0);
        derived->AddAllowedValues(g_SATTLOut_1);
        derived->UpdateProperty(propertyName.c_str());
    }

    void CreateSingleAxisTTLPolarityProperty(const std::string& propertyName, const std::string& axisLetter) {
        T* derived = GetDerived();

        derived->CreateStringProperty(
            propertyName.c_str(), g_SATTLPol_0, false,
            new MM::ActionLambda([derived, axisLetter](MM::PropertyBase* pProp, MM::ActionType eAct) {
                std::ostringstream command;
                std::ostringstream response;
                long tmp = 0;
                if (eAct == MM::BeforeGet) {
                    if (!derived->GetRefreshProps() && derived->GetInitialized()) {
                        return DEVICE_OK;
                    }
                    command << "SAP " << axisLetter << "?";
                    response << ":A " << axisLetter << "=";
                    RETURN_ON_MM_ERROR(derived->GetHub()->QueryCommandVerify(command.str(), response.str()));
                    RETURN_ON_MM_ERROR(derived->GetHub()->ParseAnswerAfterEquals(tmp));
                    bool success;
                    tmp = tmp & ((long)(BIT4));  // zero all but bit 4
                    switch (tmp) {
                    case 0:
                        success = pProp->Set(g_SATTLPol_0);
                        break;
                    case BIT4:
                        success = pProp->Set(g_SATTLPol_1);
                        break;
                    default:
                        success = 0;
                        break;
                    }
                    if (!success) {
                        return DEVICE_INVALID_PROPERTY_VALUE;
                    }
                } else if (eAct == MM::AfterSet) {
                    std::string tmpstr;
                    pProp->Get(tmpstr);
                    if (tmpstr == g_SATTLPol_0) {
                        tmp = 0;
                    } else if (tmpstr == g_SATTLPol_1) {
                        tmp = BIT4;
                    } else {
                        return DEVICE_INVALID_PROPERTY_VALUE;
                    }
                    // have to get current settings and then modify bit 4 from there
                    command << "SAP " << axisLetter << "?";
                    response << ":A " << axisLetter << "=";
                    RETURN_ON_MM_ERROR(derived->GetHub()->QueryCommandVerify(command.str(), response.str()));
                    long current;
                    RETURN_ON_MM_ERROR(derived->GetHub()->ParseAnswerAfterEquals(current));
                    current = current & (~(long)(BIT4));  // clear bit 4
                    tmp += current;
                    command.str("");
                    command << "SAP " << axisLetter << "=" << tmp;
                    RETURN_ON_MM_ERROR(derived->GetHub()->QueryCommandVerify(command.str(), ":A"));
                }
                return DEVICE_OK;
            }
        ));
        derived->AddAllowedValues(g_SATTLPol_0);
        derived->AddAllowedValues(g_SATTLPol_1);
        derived->UpdateProperty(propertyName.c_str());
    }

    void CreateSingleAxisPatternByteProperty(const std::string& propertyName, const std::string& axisLetter) {
        T* derived = GetDerived();

        derived->CreateIntegerProperty(
            propertyName.c_str(), "", false,
            new MM::ActionLambda([derived, axisLetter](MM::PropertyBase* pProp, MM::ActionType eAct) {
                // get every single time
                std::ostringstream command;
                std::ostringstream response;
                long tmp = 0;
                if (eAct == MM::BeforeGet) {
                    command << "SAP " << axisLetter << "?";
                    response << ":A " << axisLetter << "=";
                    RETURN_ON_MM_ERROR(derived->GetHub()->QueryCommandVerify(command.str(), response.str()));
                    RETURN_ON_MM_ERROR(derived->GetHub()->ParseAnswerAfterEquals(tmp));
                    if (!pProp->Set(tmp)) {
                        return DEVICE_INVALID_PROPERTY_VALUE;
                    }
                } else if (eAct == MM::AfterSet) {
                    pProp->Get(tmp);
                    command << "SAP " << axisLetter << "=" << tmp;
                    RETURN_ON_MM_ERROR(derived->GetHub()->QueryCommandVerify(command.str(), ":A"));
                }
                return DEVICE_OK;
            }
        ));
        derived->SetPropertyLimits(propertyName.c_str(), 0, 255);
        derived->UpdateProperty(propertyName.c_str());
    }

};

#endif // MIXIN_SINGLEAXIS_H
