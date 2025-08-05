///////////////////////////////////////////////////////////////////////////////
// FILE:          MixinInput.h
// PROJECT:       Micro-Manager
// SUBSYSTEM:     DeviceAdapters
//-----------------------------------------------------------------------------
// DESCRIPTION:   ASI mixin for joystick and wheel properties
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

#ifndef MIXIN_INPUT_H
#define MIXIN_INPUT_H

#include "ASITiger.h"

#include <array>
#include <cmath> // for std::abs
#include <iostream>
#include <sstream>
#include <string>
#include <type_traits>

// This mixin implements the following device properties:
// 01) JoystickEnabled
// 02) JoystickInput
// 03) JoystickInputX
// 04) JoystickInputY
// 05) JoystickFastSpeed
// 06) JoystickSlowSpeed
// 07) JoystickReverse
// 08) JoystickRotate
// 09) WheelFastSpeed
// 10) WheelSlowSpeed
// 11) WheelReverse

// Compile-time checks to determine if the derived class T provides specific axis letter functions.
// This uses type traits to adapt the mixin's behavior based on T's interface.

// Helper to detect if T has GetAxisLetter()
template <typename T, typename = void>
struct HasGetAxisLetter : std::false_type {};

template <typename T>
struct HasGetAxisLetter<T, std::void_t<decltype(std::declval<T>().GetAxisLetter())>> : std::true_type {};

// Helper to detect if T has GetAxisLetterX()
template <typename T, typename = void>
struct HasGetAxisLetterX : std::false_type {};

template <typename T>
struct HasGetAxisLetterX<T, std::void_t<decltype(std::declval<T>().GetAxisLetterX())>> : std::true_type {};

// Helper to detect if T has GetAxisLetterY()
template <typename T, typename = void>
struct HasGetAxisLetterY : std::false_type {};

template <typename T>
struct HasGetAxisLetterY<T, std::void_t<decltype(std::declval<T>().GetAxisLetterY())>> : std::true_type {};

/* When C++20 is available for device adapters, switch to using concepts:
Remove "#include <type_traits>" and replace with "#include <concepts>"

template<typename T>
concept HasAxisLetter = requires(const T& t) {
    { t.GetAxisLetter() } -> std::same_as<const std::string&>;
};

template<typename T>
concept HasAxisLettersXY = requires(const T& t) {
    { t.GetAxisLetterX() } -> std::same_as<const std::string&>;
    { t.GetAxisLetterY() } -> std::same_as<const std::string&>;
};

How to use concepts:
if constexpr (HasAxisLettersXY<T>) { }
*/

// A mixin that adds manual input properties to an ASI device.
// These properties are for the joystick and wheel.
// 
// Requirements (the derived class should implement these depending on number of axes):
// const std::string& GetAxisLetter() const { return axisLetter_; }   // For One Axis
// const std::string& GetAxisLetterX() const { return axisLetterX_; } // For Two Axes
// const std::string& GetAxisLetterY() const { return axisLetterY_; } // For Two Axes
// 
// Example:
// 1) Inherit MixinInput<T>, where T is the derived class that uses the mixin (CRTP).
// Example: class CXYStage : public ASIPeripheralBase<CXYStageBase, CXYStage>, public MixinInput<CXYStage>
// 2) Call the function MixinCreateInputProperties() from T's Initialize() function.
template <typename T>
class MixinInput {
private:
    T* GetDerived() { return static_cast<T*>(this); }
    const T* GetDerived() const { return static_cast<const T*>(this); }

public:
    // Creates manual input properties, call this in type T's Initialize() function.
    // The isXYStage parameter is for two axis stages such as ASIXYStage and ASIDacXYStage.
    void MixinCreateInputProperties(const bool isXYStage = false) {
        T* derived = GetDerived();

        // Joystick properties - always added
        CreateJoystickFastSpeedProperty();
        CreateJoystickSlowSpeedProperty();
        CreateJoystickReverseProperty();

        // Joystick properties that require a specific interface for the derived class T
        if constexpr (HasGetAxisLetterX<T>::value && HasGetAxisLetterY<T>::value) {
            // Two Axes
            if (isXYStage) {
                CreateJoystickEnabledProperty();
                CreateJoystickRotateProperty();
            } else {
                CreateJoystickInputProperty(
                    std::string(g_JoystickSelectPropertyName) + "X", derived->GetAxisLetterX());
                CreateJoystickInputProperty(
                    std::string(g_JoystickSelectPropertyName) + "Y", derived->GetAxisLetterY());
            }
        } else if constexpr (HasGetAxisLetter<T>::value) {
            // Single Axis
            CreateJoystickInputProperty(g_JoystickSelectPropertyName, derived->GetAxisLetter());
        } else {
            static_assert(false,
                "The derived class T must provide GetAxisLetter() or both "
                "GetAxisLetterX() and GetAxisLetterY() for MixinInput.");
        }

        // Wheel properties
        if (derived->FirmwareVersionAtLeast(3.14)) {
            // Changed behavior of JS F and JS T as of v2.87
            CreateWheelFastSpeedProperty();
            CreateWheelSlowSpeedProperty();
            CreateWheelReverseProperty();
        }
    }

private:
    // Joystick enable and disable, requires 2 axes.
    void CreateJoystickEnabledProperty() {
        if constexpr (HasGetAxisLetterX<T>::value && HasGetAxisLetterY<T>::value) {
            T* derived = GetDerived();

            const std::string query = "J " + derived->GetAxisLetterX() + "?";
            const std::string response = ":A " + derived->GetAxisLetterX() + "=";

            derived->CreateStringProperty(
                g_JoystickEnabledPropertyName, "No", false,
                new MM::ActionLambda([derived, query, response](MM::PropertyBase* pProp, MM::ActionType eAct) {
                    std::ostringstream command;
                    long tmp = 0;
                    if (eAct == MM::BeforeGet) {
                        if (!derived->GetRefreshProps() && derived->GetInitialized()) {
                            return DEVICE_OK;
                        }
                        RETURN_ON_MM_ERROR(derived->GetHub()->QueryCommandVerify(query, response));
                        RETURN_ON_MM_ERROR(derived->GetHub()->ParseAnswerAfterEquals(tmp));
                        // treat anything nozero as enabled when reading
                        const bool success = pProp->Set(tmp ? "Yes" : "No");
                        if (!success) {
                            return DEVICE_INVALID_PROPERTY_VALUE;
                        }
                    } else if (eAct == MM::AfterSet) {
                        std::string isEnabled;
                        pProp->Get(isEnabled);
                        if (isEnabled == "Yes") {
                            std::array<char, MM::MaxStrLength + 1> buffer;
                            RETURN_ON_MM_ERROR(derived->GetProperty(g_JoystickRotatePropertyName, buffer.data()));
                            const std::string joystickRotate(buffer.data());
                            if (joystickRotate == "Yes") {
                                command << "J " << derived->GetAxisLetterX() << "=3" << " " << derived->GetAxisLetterY() << "=2";  // rotated
                            } else {
                                command << "J " << derived->GetAxisLetterX() << "=2" << " " << derived->GetAxisLetterY() << "=3";
                            }
                        } else { // "No" == disabled
                            command << "J " << derived->GetAxisLetterX() << "=0" << " " << derived->GetAxisLetterY() << "=0";
                        }
                        RETURN_ON_MM_ERROR(derived->GetHub()->QueryCommandVerify(command.str(), ":A"));
                    }
                    return DEVICE_OK;
                }
             ));
            derived->AddAllowedValue(g_JoystickEnabledPropertyName, "No");
            derived->AddAllowedValue(g_JoystickEnabledPropertyName, "Yes");
            derived->UpdateProperty(g_JoystickEnabledPropertyName);
        }
    }

    // Creates "JoystickInput", "JoystickInputX", and "JoystickInputY".
    void CreateJoystickInputProperty(const std::string& propertyName, const std::string& axisLetter) {
        T* derived = GetDerived();

        const std::string query = "J " + axisLetter + "?";
        const std::string command = "J " + axisLetter + "=";
        const std::string response = ":A " + axisLetter + "=";

        derived->CreateStringProperty(
            propertyName.c_str(), g_JSCode_0, false,
            new MM::ActionLambda([derived, query, command, response](MM::PropertyBase* pProp, MM::ActionType eAct) {
                long tmp = 0;
                if (eAct == MM::BeforeGet) {
                    if (!derived->GetRefreshProps() && derived->GetInitialized()) {
                        return DEVICE_OK;
                    }
                    RETURN_ON_MM_ERROR(derived->GetHub()->QueryCommandVerify(query, response));
                    RETURN_ON_MM_ERROR(derived->GetHub()->ParseAnswerAfterEquals(tmp));
                    bool success = 0;
                    switch (tmp) {
                    case 0:
                        success = pProp->Set(g_JSCode_0);
                        break;
                    case 1:
                        success = pProp->Set(g_JSCode_1);
                        break;
                    case 2:
                        success = pProp->Set(g_JSCode_2);
                        break;
                    case 3:
                        success = pProp->Set(g_JSCode_3);
                        break;
                    case 22:
                        success = pProp->Set(g_JSCode_22);
                        break;
                    case 23:
                        success = pProp->Set(g_JSCode_23);
                        break;
                    default:
                        success = 0;
                        break;
                    }
                    // don't complain if value is unsupported, just leave as-is
                } else if (eAct == MM::AfterSet) {
                    std::string tmpstr;
                    pProp->Get(tmpstr);
                    if (tmpstr == g_JSCode_0) {
                        tmp = 0;
                    } else if (tmpstr == g_JSCode_1) {
                        tmp = 1;
                    } else if (tmpstr == g_JSCode_2) {
                        tmp = 2;
                    } else if (tmpstr == g_JSCode_3) {
                        tmp = 3;
                    } else if (tmpstr == g_JSCode_22) {
                        tmp = 22;
                    } else if (tmpstr == g_JSCode_23) {
                        tmp = 23;
                    } else {
                        return DEVICE_INVALID_PROPERTY_VALUE;
                    }
                    RETURN_ON_MM_ERROR(derived->GetHub()->QueryCommandVerify(command + std::to_string(tmp), ":A"));
                }
                return DEVICE_OK;
            }
        ));
        derived->AddAllowedValue(propertyName.c_str(), g_JSCode_0);
        derived->AddAllowedValue(propertyName.c_str(), g_JSCode_2);
        derived->AddAllowedValue(propertyName.c_str(), g_JSCode_3);
        derived->AddAllowedValue(propertyName.c_str(), g_JSCode_22);
        derived->AddAllowedValue(propertyName.c_str(), g_JSCode_23);
        derived->UpdateProperty(propertyName.c_str());
    }

    // Joystick fast speed (JS X)
    void CreateJoystickFastSpeedProperty() {
        T* derived = GetDerived();

        const std::string query = derived->GetAddressChar() + "JS X?";

        derived->CreateFloatProperty(
            g_JoystickFastSpeedPropertyName, 100.0, false,
            new MM::ActionLambda([derived, query](MM::PropertyBase* pProp, MM::ActionType eAct) {
                std::ostringstream command;
                double tmp = 0;
                if (eAct == MM::BeforeGet) {
                    if (!derived->GetRefreshProps() && derived->GetInitialized()) {
                        return DEVICE_OK;
                    }
                    RETURN_ON_MM_ERROR(derived->GetHub()->QueryCommandVerify(query, ":A X="));
                    RETURN_ON_MM_ERROR(derived->GetHub()->ParseAnswerAfterEquals(tmp));
                    tmp = std::abs(tmp);
                    if (!pProp->Set(tmp)) {
                        return DEVICE_INVALID_PROPERTY_VALUE;
                    }
                } else if (eAct == MM::AfterSet) {
                    pProp->Get(tmp);
                    std::array<char, MM::MaxStrLength + 1> buffer;
                    RETURN_ON_MM_ERROR(derived->GetProperty(g_JoystickMirrorPropertyName, buffer.data()));
                    const std::string joystickReverse(buffer.data());
                    if (joystickReverse == "Yes") {
                        command << derived->GetAddressChar() << "JS X=-" << tmp;
                    } else {
                        command << derived->GetAddressChar() << "JS X=" << tmp;
                    }
                    RETURN_ON_MM_ERROR(derived->GetHub()->QueryCommandVerify(command.str(), ":A"));
                }
                return DEVICE_OK;
            }
        ));
        derived->SetPropertyLimits(g_JoystickFastSpeedPropertyName, 0.0, 100.0);
        derived->UpdateProperty(g_JoystickFastSpeedPropertyName);
    }

    // Joystick slow speed (JS Y)
    void CreateJoystickSlowSpeedProperty() {
        T* derived = GetDerived();

        const std::string query = derived->GetAddressChar() + "JS Y?";

        derived->CreateFloatProperty(
            g_JoystickSlowSpeedPropertyName, 10.0, false,
            new MM::ActionLambda([derived, query](MM::PropertyBase* pProp, MM::ActionType eAct) {
                std::ostringstream command;
                double tmp = 0;
                if (eAct == MM::BeforeGet) {
                    if (!derived->GetRefreshProps() && derived->GetInitialized()) {
                        return DEVICE_OK;
                    }
                    RETURN_ON_MM_ERROR(derived->GetHub()->QueryCommandVerify(query, ":A Y="));
                    RETURN_ON_MM_ERROR(derived->GetHub()->ParseAnswerAfterEquals(tmp));
                    if (!pProp->Set(tmp)) {
                        return DEVICE_INVALID_PROPERTY_VALUE;
                    }
                } else if (eAct == MM::AfterSet) {
                    pProp->Get(tmp);
                    std::array<char, MM::MaxStrLength + 1> buffer;
                    RETURN_ON_MM_ERROR(derived->GetProperty(g_JoystickMirrorPropertyName, buffer.data()));
                    const std::string joystickReverse(buffer.data());
                    if (joystickReverse == "Yes") {
                        command << derived->GetAddressChar() << "JS Y=-" << tmp;
                    } else {
                        command << derived->GetAddressChar() << "JS Y=" << tmp;
                    }
                    RETURN_ON_MM_ERROR(derived->GetHub()->QueryCommandVerify(command.str(), ":A"));
                }
                return DEVICE_OK;
            }
        ));
        derived->SetPropertyLimits(g_JoystickSlowSpeedPropertyName, 0.0, 100.0);
        derived->UpdateProperty(g_JoystickSlowSpeedPropertyName);
    }

    // Joystick reverse (changes joystick fast/slow speeds to negative)
    void CreateJoystickReverseProperty() {
        T* derived = GetDerived();

        const std::string query = derived->GetAddressChar() + "JS X?";

        derived->CreateStringProperty(
            g_JoystickMirrorPropertyName, "No", false,
            new MM::ActionLambda([derived, query](MM::PropertyBase* pProp, MM::ActionType eAct) {
                std::ostringstream command;
                double tmp = 0;
                if (eAct == MM::BeforeGet) {
                    if (!derived->GetRefreshProps() && derived->GetInitialized()) {
                        return DEVICE_OK;
                    }
                    // query only the fast setting to see if already mirrored
                    RETURN_ON_MM_ERROR(derived->GetHub()->QueryCommandVerify(query, ":A X="));
                    RETURN_ON_MM_ERROR(derived->GetHub()->ParseAnswerAfterEquals(tmp));
                    // speed negative <=> mirrored
                    const bool success = pProp->Set((tmp < 0) ? "Yes" : "No");
                    if (!success) {
                        return DEVICE_INVALID_PROPERTY_VALUE;
                    }
                } else if (eAct == MM::AfterSet) {
                    std::string isReversed;
                    pProp->Get(isReversed);
                    double joystickFast = 0.0;
                    RETURN_ON_MM_ERROR(derived->GetProperty(g_JoystickFastSpeedPropertyName, joystickFast));
                    double joystickSlow = 0.0;
                    RETURN_ON_MM_ERROR(derived->GetProperty(g_JoystickSlowSpeedPropertyName, joystickSlow));
                    if (isReversed == "Yes") {
                        command << derived->GetAddressChar() << "JS X=-" << joystickFast << " Y=-" << joystickSlow;
                    } else {
                        command << derived->GetAddressChar() << "JS X=" << joystickFast << " Y=" << joystickSlow;
                    }
                    RETURN_ON_MM_ERROR(derived->GetHub()->QueryCommandVerify(command.str(), ":A"));
                }
                return DEVICE_OK;
            }
        ));
        derived->AddAllowedValue(g_JoystickMirrorPropertyName, "No");
        derived->AddAllowedValue(g_JoystickMirrorPropertyName, "Yes");
        derived->UpdateProperty(g_JoystickMirrorPropertyName);
    }

    // Joystick rotate, requires 2 axes. (interchanges X and Y axes, useful if camera is rotated)
    void CreateJoystickRotateProperty() {
        if constexpr (HasGetAxisLetterX<T>::value && HasGetAxisLetterY<T>::value) {
            T* derived = GetDerived();

            const std::string query = "J " + derived->GetAxisLetterX() + "?";
            const std::string response = ":A " + derived->GetAxisLetterX() + "=";

            derived->CreateStringProperty(
                g_JoystickRotatePropertyName, "No", false,
                new MM::ActionLambda([derived, query, response](MM::PropertyBase* pProp, MM::ActionType eAct) {
                    std::ostringstream command;
                    double tmp = 0;
                    if (eAct == MM::BeforeGet) {
                        if (!derived->GetRefreshProps() && derived->GetInitialized()) {
                            return DEVICE_OK;
                        }
                        // only look at X axis for joystick
                        RETURN_ON_MM_ERROR(derived->GetHub()->QueryCommandVerify(query, response));
                        RETURN_ON_MM_ERROR(derived->GetHub()->ParseAnswerAfterEquals(tmp));
                        // if set to be Y joystick direction then we are rotated, otherwise assume not rotated
                        const bool success = pProp->Set((tmp == 3) ? "Yes" : "No");
                        if (!success) {
                            return DEVICE_INVALID_PROPERTY_VALUE;
                        }
                    } else if (eAct == MM::AfterSet) {
                        std::string isRotated;
                        pProp->Get(isRotated);
                        std::array<char, MM::MaxStrLength + 1> buffer;
                        RETURN_ON_MM_ERROR(derived->GetProperty(g_JoystickEnabledPropertyName, buffer.data()));
                        const std::string joystickEnabled(buffer.data());
                        if (joystickEnabled == "Yes") {
                            if (isRotated == "Yes") {
                                command << "J " << derived->GetAxisLetterX() << "=3" << " " << derived->GetAxisLetterY() << "=2";
                            } else {
                                command << "J " << derived->GetAxisLetterX() << "=2" << " " << derived->GetAxisLetterY() << "=3";
                            }
                        } else { // "No" == disabled
                            command << "J " << derived->GetAxisLetterX() << "=0" << " " << derived->GetAxisLetterY() << "=0";
                        }
                        RETURN_ON_MM_ERROR(derived->GetHub()->QueryCommandVerify(command.str(), ":A"));
                    }
                    return DEVICE_OK;
                }
            ));
            derived->AddAllowedValue(g_JoystickRotatePropertyName, "No");
            derived->AddAllowedValue(g_JoystickRotatePropertyName, "Yes");
            derived->UpdateProperty(g_JoystickRotatePropertyName);
        }
    }
 
    // ASI controller mirrors by having negative speed, but here we have separate property for mirroring
    // and for speed (which is strictly positive)... that makes this code a bit odd
 
    // Wheel fast speed (JS F) (per-card, not per-axis)
    void CreateWheelFastSpeedProperty() { 
        T* derived = GetDerived();

        const std::string query = derived->GetAddressChar() + "JS F?";

        derived->CreateFloatProperty(
            g_WheelFastSpeedPropertyName, 10.0, false,
            new MM::ActionLambda([derived, query](MM::PropertyBase* pProp, MM::ActionType eAct) {
                std::ostringstream command;
                double tmp = 0;
                if (eAct == MM::BeforeGet) {
                    if (!derived->GetRefreshProps() && derived->GetInitialized()) {
                        return DEVICE_OK;
                    }
                    // query only the fast setting to see if already mirrored
                    RETURN_ON_MM_ERROR(derived->GetHub()->QueryCommandVerify(query, ":A F="));
                    RETURN_ON_MM_ERROR(derived->GetHub()->ParseAnswerAfterEquals(tmp));
                    tmp = std::abs(tmp);
                    if (!pProp->Set(tmp)) {
                        return DEVICE_INVALID_PROPERTY_VALUE;
                    }
                } else if (eAct == MM::AfterSet) {
                    pProp->Get(tmp);
                    std::array<char, MM::MaxStrLength + 1> buffer;
                    RETURN_ON_MM_ERROR(derived->GetProperty(g_WheelMirrorPropertyName, buffer.data()));
                    const std::string wheelReverse(buffer.data());
                    if (wheelReverse == "Yes") {
                        command << derived->GetAddressChar() << "JS F=-" << tmp;
                    } else {
                        command << derived->GetAddressChar() << "JS F=" << tmp;
                    }
                    RETURN_ON_MM_ERROR(derived->GetHub()->QueryCommandVerify(command.str(), ":A"));
                }
                return DEVICE_OK;
            }
        ));
        derived->SetPropertyLimits(g_WheelFastSpeedPropertyName, 0.0, 100.0);
        derived->UpdateProperty(g_WheelFastSpeedPropertyName);
    }

    // Wheel slow speed (JS T) (per-card, not per-axis)
    void CreateWheelSlowSpeedProperty() {
        T* derived = GetDerived();

        const std::string query = derived->GetAddressChar() + "JS T?";

        derived->CreateFloatProperty(
            g_WheelSlowSpeedPropertyName, 5.0, false,
            new MM::ActionLambda([derived, query](MM::PropertyBase* pProp, MM::ActionType eAct) {
                std::ostringstream command;
                double tmp = 0;
                if (eAct == MM::BeforeGet) {
                    if (!derived->GetRefreshProps() && derived->GetInitialized()) {
                        return DEVICE_OK;
                    }
                    RETURN_ON_MM_ERROR(derived->GetHub()->QueryCommandVerify(query, ":A T="));
                    RETURN_ON_MM_ERROR(derived->GetHub()->ParseAnswerAfterEquals(tmp));
                    tmp = std::abs(tmp);
                    if (!pProp->Set(tmp)) {
                        return DEVICE_INVALID_PROPERTY_VALUE;
                    }
                } else if (eAct == MM::AfterSet) {
                    pProp->Get(tmp);
                    std::array<char, MM::MaxStrLength + 1> buffer;
                    RETURN_ON_MM_ERROR(derived->GetProperty(g_WheelMirrorPropertyName, buffer.data()));
                    const std::string wheelReverse(buffer.data());
                    if (wheelReverse == "Yes") {
                        command << derived->GetAddressChar() << "JS T=-" << tmp;
                    }  else {
                        command << derived->GetAddressChar() << "JS T=" << tmp;
                    }
                    RETURN_ON_MM_ERROR(derived->GetHub()->QueryCommandVerify(command.str(), ":A"));
                }
                return DEVICE_OK;
            }
        ));
        derived->SetPropertyLimits(g_WheelSlowSpeedPropertyName, 0.0, 100.0);
        derived->UpdateProperty(g_WheelSlowSpeedPropertyName);
    }

    // Wheel reverse (changes wheel fast/slow speeds to negative, per-card, not per-axis)
    void CreateWheelReverseProperty() {
        T* derived = GetDerived();

        const std::string query = derived->GetAddressChar() + "JS F?";

        derived->CreateStringProperty(
            g_WheelMirrorPropertyName, "No", false,
            new MM::ActionLambda([derived, query](MM::PropertyBase* pProp, MM::ActionType eAct) {
                std::ostringstream command;
                double tmp = 0;
                if (eAct == MM::BeforeGet) {
                    if (!derived->GetRefreshProps() && derived->GetInitialized()) {
                        return DEVICE_OK;
                    }
                    // query only the fast setting to see if already mirrored
                    RETURN_ON_MM_ERROR(derived->GetHub()->QueryCommandVerify(query, ":A F="));
                    RETURN_ON_MM_ERROR(derived->GetHub()->ParseAnswerAfterEquals(tmp));
                    // speed negative <=> mirrored
                    const bool success = pProp->Set((tmp < 0) ? "Yes" : "No");
                    if (!success) {
                        return DEVICE_INVALID_PROPERTY_VALUE;
                    }
                } else if (eAct == MM::AfterSet) {
                    std::string tmpstr;
                    pProp->Get(tmpstr);
                    double wheelFast = 0.0;
                    RETURN_ON_MM_ERROR(derived->GetProperty(g_WheelFastSpeedPropertyName, wheelFast));
                    double wheelSlow = 0.0;
                    RETURN_ON_MM_ERROR(derived->GetProperty(g_WheelSlowSpeedPropertyName, wheelSlow));
                    if (tmpstr == "Yes") {
                        command << derived->GetAddressChar() << "JS F=-" << wheelFast << " T=-" << wheelSlow;
                    } else {
                        command << derived->GetAddressChar() << "JS F=" << wheelFast << " T=" << wheelSlow;
                    }
                    RETURN_ON_MM_ERROR(derived->GetHub()->QueryCommandVerify(command.str(), ":A"));
                }
                return DEVICE_OK;
            }
        ));
        derived->AddAllowedValue(g_WheelMirrorPropertyName, "No");
        derived->AddAllowedValue(g_WheelMirrorPropertyName, "Yes");
        derived->UpdateProperty(g_WheelMirrorPropertyName);
    }

};

#endif // MIXIN_INPUT_H
