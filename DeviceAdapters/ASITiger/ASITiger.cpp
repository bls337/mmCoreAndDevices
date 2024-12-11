///////////////////////////////////////////////////////////////////////////////
// FILE:          ASITiger.cpp
// PROJECT:       Micro-Manager
// SUBSYSTEM:     DeviceAdapters
//-----------------------------------------------------------------------------
// DESCRIPTION:   ASI Tiger MODULE_API items and ASIUtility class
//                Note this is for the "Tiger" MM set of adapters, which should
//                  work for more than just the TG-1000 "Tiger" controller
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
// BASED ON:      ASIStage.cpp, ASIFW1000.cpp, Arduino.cpp, and DemoCamera.cpp
//
//

#include "ASITiger.h"
#include "ASITigerComm.h"
#include "ASIXYStage.h"
#include "ASIZStage.h"
#include "ASIClocked.h"
#include "ASIFWheel.h"
#include "ASIScanner.h"
#include "ASIPiezo.h"
#include "ASICRISP.h"
#include "ASILED.h"
#include "ASIPLogic.h"
#include "ASIPmt.h"
#include "ASILens.h"
#include "ASIDac.h"
#include "ASIDacXYStage.h"
#include "MMDevice.h"
#include "DeviceBase.h"
#include <iostream>
#include <sstream>
#include <string>
#include <vector>


// TODO add in support for other devices, each time modifying these places
//    name constant declarations in the corresponding .h file
//    MODULE_API MM::Device* CreateDevice(const char* deviceName) in this file
//    DetectInstalledDevices in TigerComm (or other hub)


///////////////////////////////////////////////////////////////////////////////
// Exported MMDevice API
///////////////////////////////////////////////////////////////////////////////

MODULE_API void InitializeModuleData()
{
    RegisterDevice(g_TigerCommHubName, MM::HubDevice, g_TigerCommHubDescription);
}

MODULE_API MM::Device* CreateDevice(const char* deviceName)
{
    std::string device = deviceName;
    if (deviceName == 0)
    {
        return 0;
    }
    else if (device == g_TigerCommHubName)
    {
        return new CTigerCommHub();
    }
    else if (device == g_XYStageDeviceName)
    {
        return new CXYStage(deviceName);
    }
    else if (device == g_ZStageDeviceName)
    {
        return new CZStage(deviceName);
    }
    else if (device == g_FSliderDeviceName)
    {
        return new CFSlider(deviceName);
    }
    else if (device == g_TurretDeviceName)
    {
        return new CTurret(deviceName);
    }
    else if (device == g_PortSwitchDeviceName)
    {
        return new CPortSwitch(deviceName);
    }
    else if (device == g_FWheelDeviceName)
    {
        return new CFWheel(deviceName);
    }
    else if (device == g_ScannerDeviceName)
    {
        return new CScanner(deviceName);
    }
    else if (device == g_PiezoDeviceName)
    {
        return new CPiezo(deviceName);
    }
    else if (device == g_CRISPDeviceName)
    {
        return new CCRISP(deviceName);
    }
    else if (device == g_LEDDeviceName)
    {
        return new CLED(deviceName);
    }
    else if (device == g_PLogicDeviceName)
    {
        return new CPLogic(deviceName);
    }
    else if (device == g_PMTDeviceName)
    {
        return new CPMT(deviceName);
    }
    else if (device == g_LensDeviceName)
    {
        return new CLens(deviceName);
    }
    else if (device == g_DacXYStageDeviceName)
    {
        return new CDACXYStage(deviceName);
    }
    else if (device == g_DacDeviceName)
    {
        return new CDAC(deviceName);
    }
    else
    {
        return 0;
    }
}

MODULE_API void DeleteDevice(MM::Device* pDevice)
{
    delete pDevice;
}
