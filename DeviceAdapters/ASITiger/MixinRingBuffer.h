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

// TODO: implement this mixin
 
// A mixin that adds ring buffer properties to an ASI device.
template <typename T>
class MixinRingBuffer {
private:
    T* GetDerived() { return static_cast<T*>(this); }
    const T* GetDerived() const { return static_cast<const T*>(this); }

public:
    void MixinCreateRingBufferProperties() {
        CreateRingBufferModeProperty();
        CreateRingBufferDelayProperty();
        CreateRingBufferTriggerProperty();
        CreateRingBufferRunningProperty();
    }

private:
    void CreateRingBufferModeProperty() {

    }

    void CreateRingBufferDelayProperty() {

    }

    void CreateRingBufferTriggerProperty() {

    }

    void CreateRingBufferRunningProperty() {

    }

};

#endif // MIXIN_RINGBUFFER_H
