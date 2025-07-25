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

// TODO: implement this mixin

// A mixin that adds single axis properties to an ASI device.
template <typename T>
class MixinSingleAxis {
private:
    T* GetDerived() { return static_cast<T*>(this); }
    const T* GetDerived() const { return static_cast<const T*>(this); }

public:
    void MixinCreateSingleAxisProperties() {
        CreateSingleAxisAmplitudeProperty();
        CreateSingleAxisOffsetProperty();
        CreateSingleAxisPeriodProperty();
        CreateSingleAxisModeProperty();
        CreateSingleAxisPatternProperty();
        CreateSingleAxisAdvancedProperties();
    }

private:
    void CreateSingleAxisAmplitudeProperty() {

    }

    void CreateSingleAxisOffsetProperty() {

    }

    void CreateSingleAxisPeriodProperty() {

    }

    void CreateSingleAxisModeProperty() {

    }

    void CreateSingleAxisPatternProperty() {

    }

    void CreateSingleAxisAdvancedProperties() {

    }

    // Advanced properties

    void CreateSingleAxisClockSourceProperty() {

    }

    void CreateSingleAxisClockPolarityProperty() {

    }

    void CreateSingleAxisTTLOutputProperty() {

    }

    void CreateSingleAxisTTLPolarityProperty() {

    }

    void CreateSingleAxisPatternByteProperty() {

    }

};

#endif // MIXIN_SINGLEAXIS_H
