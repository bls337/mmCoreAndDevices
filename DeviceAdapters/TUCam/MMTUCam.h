///////////////////////////////////////////////////////////////////////////////
// FILE:          MMTUCam.h
// PROJECT:       Micro-Manager
// SUBSYSTEM:     DeviceAdapters
//-----------------------------------------------------------------------------
// DESCRIPTION:   The example implementation of the demo camera.
//                Simulates generic digital camera and associated automated
//                microscope devices and enables testing of the rest of the
//                system without the need to connect to the actual hardware. 
//                
// AUTHOR:        fandayu, fandayu@tucsen.com 2024
//                
//                Karl Hoover (stuff such as programmable CCD size  & the various image processors)
//                Arther Edelstein ( equipment error simulation)
//
// COPYRIGHT:     Tucsen Photonics Co., Ltd., 2024
//               
//
// LICENSE:       This file is distributed under the BSD license.
//                License text is included with the source distribution.
//
//                This file is distributed in the hope that it will be useful,
//                but WITHOUT ANY WARRANTY; without even the implied warranty
//                of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
//
//                IN NO EVENT SHALL THE COPYRIGHT OWNER OR
//                CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
//                INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES.

#ifndef _MMTUCAM_H_
#define _MMTUCAM_H_

#include "TUCamApi.h"
#include "DeviceBase.h"
#include "ImgBuffer.h"
#include "DeviceThreads.h"
#include <string>
#include <map>
#include <algorithm>

//////////////////////////////////////////////////////////////////////////////
// Error codes
//
#define ERR_UNKNOWN_MODE         102
#define ERR_UNKNOWN_POSITION     103
#define ERR_IN_SEQUENCE          104
#define ERR_SEQUENCE_INACTIVE    105
#define ERR_STAGE_MOVING         106
#define HUB_NOT_AVAILABLE        107

const char* NoHubError = "Parent Hub not defined.";

// Defines which segments in a seven-segment display are lit up for each of
// the numbers 0-9. Segments are:
//
//  0       1
// 1 2     2 4
//  3       8
// 4 5    16 32
//  6      64
const int SEVEN_SEGMENT_RULES[] = {1+2+4+16+32+64, 4+32, 1+4+8+16+64,
      1+4+8+32+64, 2+4+8+32, 1+2+8+32+64, 2+8+16+32+64, 1+4+32,
      1+2+4+8+16+32+64, 1+2+4+8+32+64};
// Indicates if the segment is horizontal or vertical.
const int SEVEN_SEGMENT_HORIZONTALITY[] = {1, 0, 0, 1, 0, 0, 1};
// X offset for this segment.
const int SEVEN_SEGMENT_X_OFFSET[] = {0, 0, 1, 0, 0, 1, 0};
// Y offset for this segment.
const int SEVEN_SEGMENT_Y_OFFSET[] = {0, 0, 0, 1, 1, 1, 2};

////////////////////////
// DemoHub
//////////////////////
#define TUDBG_PRINTF(format,...)	{char dbgMsg[1024] = {0}; sprintf_s(dbgMsg, "" format"", ##__VA_ARGS__); OutputDebugString(dbgMsg);}
///////////////////////
// CameraPID
///////////////////////
#define DHYANA_400D_X45     0x6404
#define DHYANA_D95_X45      0x6405
#define DHYANA_400DC_X45    0x6804
#define DHYANA_400DC_X100   0xEC03
#define DHYANA_400D_X100    0xE404
#define DHYANA_400BSIV1     0xE405
#define DHYANA_D95_X100     0xE406
#define DHYANA_400BSIV2     0xE408
#define PID_FL_20BW         0xE40D
#define DHYANA_D95_V2       0xE40F
#define DHYANA_401D         0xE005
#define DHYANA_201D         0xE007
#define DHYANA_4040V2       0xE412    
#define DHYANA_4040BSI      0xE413    
#define DHYANA_400BSIV3     0xE419
#define DHYANA_XF4040BSI    0xE41B  
#define PID_FL_20           0xEC0D
#define PID_FL_9BW          0xE422
#define PID_FL_9BW_LT       0xE426
#define PID_FL_26BW         0xE423

#define PID_ARIES16LT       0xE424
#define PID_ARIES16         0xE425
///////////////////////
// ImgMode
///////////////////////
#define MODE_12BIT    0x00
#define MODE_CMS      0x01
#define MODE_11BIT    0x02
#define MODE_GLRESET  0x03

typedef enum
{
	TU_USB2_DRIVER       = 0x00,     // USB2.0 driver
	TU_USB3_DRIVER       = 0x01,     // USB3.0 driver
	TU_PHXCAMERALINK     = 0x02,     // Fire Bird CameraLink
	TU_EURESYSCAMERALINK = 0x03,     // Euresys CameraLink
} TUDRVER_TYPE;

class DemoHub : public HubBase<DemoHub>
{
public:
   DemoHub() :
      initialized_(false),
      busy_(false)
   {}
   ~DemoHub() {}

   // Device API
   // ---------
   int Initialize();
   int Shutdown() {return DEVICE_OK;};
   void GetName(char* pName) const; 
   bool Busy() { return busy_;} ;

   // HUB api
   int DetectInstalledDevices();

private:
   void GetPeripheralInventory();

   std::vector<std::string> peripherals_;
   bool initialized_;
   bool busy_;
};


//////////////////////////////////////////////////////////////////////////////
// CMMTUCam class
// Simulation of the Camera device
//////////////////////////////////////////////////////////////////////////////

class CTUCamThread;

// outputtrigger mode
// typedef enum the output trigger port mode
/*typedef enum 
{
	TUPORT_ONE                  = 0x00,            // use port1
	TUPORT_TWO                  = 0x01,            // use port2
	TUPORT_THREE                = 0x02,            // use port3
}TUCAM_OUTPUTTRG_PORT;*/

typedef enum
{
	TRITYPE_SMA = 0x00,            
	TRITYPE_HR  = 0x01,
	TRITYPE_END = 0x02,
}TRIGGER_TYPE;

// the camera triggerout attribute
typedef struct _tagTUCAM_PATAM_TRGOUTPUT
{                      
	INT32   nTgrOutMode;                           
	INT32   nEdgeMode;                            
	INT32   nDelayTm;                              
	INT32   nWidth;                                
}TUCAM_PATAM_TRGOUTPUT;

typedef struct _tagTUCAM_TRGOUTPUT
{
	INT32   nTgrOutPort;                           // [in/out] The port of triggerout 
	TUCAM_PATAM_TRGOUTPUT TgrPort1;
    TUCAM_PATAM_TRGOUTPUT TgrPort2;
	TUCAM_PATAM_TRGOUTPUT TgrPort3;
}TUCAM_TRGOUTPUT;

// the camera rolling scan para
typedef struct _tagTUCAM_RSPARA
{
	INT32  nMode;         // Mode
	INT32  nLTDelay;      // Line time delay
	INT32  nLTDelayMax;   // Line time delay Max
	INT32  nLTDelayMin;   // Line time delay Min
	INT32  nLTDelayStep;  // Line time delay Step
	INT32  nSlitHeight;   // Slit height
	INT32  nSlitHeightMax;// Slit height Max
	INT32  nSlitHeightMin;// Slit height Min
	INT32  nSlitHeightStep;// Slit height Step
	DOUBLE dbLineInvalTm; // Line interval time
}TUCAM_RSPARA;

class CMMTUCam : public CCameraBase<CMMTUCam>  
{
public:
    CMMTUCam();
    ~CMMTUCam();
  
    // MMDevice API
    // ------------
    int Initialize();
    int Shutdown();

    void GetName(char* name) const;      

    // MMCamera API
    // ------------
    int SnapImage();
    const unsigned char* GetImageBuffer();
    unsigned GetImageWidth() const;
    unsigned GetImageHeight() const;
    unsigned GetImageBytesPerPixel() const;
    unsigned GetBitDepth() const;
    long GetImageBufferSize() const;
    double GetExposure() const;
    void SetExposure(double exp);
    int SetROI(unsigned x, unsigned y, unsigned xSize, unsigned ySize); 
    int GetROI(unsigned& x, unsigned& y, unsigned& xSize, unsigned& ySize); 
    int ClearROI();

    int PrepareSequenceAcqusition() { return DEVICE_OK; }
    int StartSequenceAcquisition(double interval);
    int StartSequenceAcquisition(long numImages, double interval_ms, bool stopOnOverflow);
    int StopSequenceAcquisition();
    int InsertImage();
    int RunSequenceOnThread(MM::MMTime startTime);
    bool IsCapturing();
    void OnThreadExiting() throw(); 
    double GetNominalPixelSizeUm() const {return nominalPixelSizeUm_;}
    double GetPixelSizeUm() const {return nominalPixelSizeUm_ * GetBinning();}
    int GetBinning() const;
    int SetBinning(int bS);

    int IsExposureSequenceable(bool& isSequenceable) const;
    int GetExposureSequenceMaxLength(long& nrEvents) const;
    int StartExposureSequence();
    int StopExposureSequence();
    int ClearExposureSequence();
    int AddToExposureSequence(double exposureTime_ms);
    int SendExposureSequence() const;

	double LineIntervalTime(int nLineDelayTm);
	int LineIntervalCal(int nVal, bool bExpChange = true);

    unsigned  GetNumberOfComponents() const { return nComponents_;};

    // action interface
    // ----------------
    // floating point read-only properties for testing
    int OnTestProperty(MM::PropertyBase* pProp, MM::ActionType eAct, long);

    int OnBinning(MM::PropertyBase* pProp, MM::ActionType eAct);
    int OnBinningSum(MM::PropertyBase* pProp, MM::ActionType eAct);
    int OnPixelClock(MM::PropertyBase* pProp, MM::ActionType eAct);
    int OnExposure(MM::PropertyBase* pProp, MM::ActionType eAct);
	int OnBrightness(MM::PropertyBase* pProp, MM::ActionType eAct);
	int OnPixelRatio(MM::PropertyBase* pProp, MM::ActionType eAct);
    int OnGlobalGain(MM::PropertyBase* pProp, MM::ActionType eAct);
	int OnFrameRate(MM::PropertyBase* pProp, MM::ActionType eAct);
	int OnSensorReset(MM::PropertyBase* pProp, MM::ActionType eAct);
    int OnCMSMode(MM::PropertyBase* pProp, MM::ActionType eAct);
	int OnLEDMode(MM::PropertyBase* pProp, MM::ActionType eAct);
	int OnPIMode(MM::PropertyBase* pProp, MM::ActionType eAct);
	int OnTECMode(MM::PropertyBase* pProp, MM::ActionType eAct);
	int OnTriOutEnable(MM::PropertyBase* pProp, MM::ActionType eAct);

	int OnRollingScanMode(MM::PropertyBase* pProp, MM::ActionType eAct);
	int OnRollingScanLtd(MM::PropertyBase* pProp, MM::ActionType eAct);
	int OnRollinScanSlit(MM::PropertyBase* pProp, MM::ActionType eAct);
	int OnRollinScanLITm(MM::PropertyBase* pProp, MM::ActionType eAct);
	int OnRollingScanDir(MM::PropertyBase* pProp, MM::ActionType eAct);
	int OnRollingScanReset(MM::PropertyBase* pProp, MM::ActionType eAct);
	int OnTestImageMode(MM::PropertyBase* pProp, MM::ActionType eAct);

    int OnGlobalGainMode(MM::PropertyBase* pProp, MM::ActionType eAct);
	int OnGAINMode(MM::PropertyBase* pProp, MM::ActionType eAct);
    int OnShutterMode(MM::PropertyBase* pProp, MM::ActionType eAct);
	int OnModeSelect(MM::PropertyBase* pProp, MM::ActionType eAct);
    int OnImageMode(MM::PropertyBase* pProp, MM::ActionType eAct);
    int OnPixelType(MM::PropertyBase* pProp, MM::ActionType eAct);
    int OnBitDepth(MM::PropertyBase* pProp, MM::ActionType eAct);
	int OnBitDepthEum(MM::PropertyBase* pProp, MM::ActionType eAct);
    int OnFlipH(MM::PropertyBase* pProp, MM::ActionType eAct);
    int OnFlipV(MM::PropertyBase* pProp, MM::ActionType eAct);
    int OnGamma(MM::PropertyBase* pProp, MM::ActionType eAct);
    int OnContrast(MM::PropertyBase* pProp, MM::ActionType eAct);
    int OnSaturation(MM::PropertyBase* pProp, MM::ActionType eAct);
    int OnWhiteBalance(MM::PropertyBase* pProp, MM::ActionType eAct);
	int OnClrTemp(MM::PropertyBase* pProp, MM::ActionType eAct);
    int OnRedGain(MM::PropertyBase* pProp, MM::ActionType eAct);
    int OnGreenGain(MM::PropertyBase* pProp, MM::ActionType eAct);
    int OnBlueGain(MM::PropertyBase* pProp, MM::ActionType eAct);
	int OnATExpMode(MM::PropertyBase* pProp, MM::ActionType eAct);
    int OnATExposure(MM::PropertyBase* pProp, MM::ActionType eAct);
	int OnTimeStamp(MM::PropertyBase* pProp, MM::ActionType eAct);
    int OnTemperature(MM::PropertyBase* pProp, MM::ActionType eAct);
    int OnFan(MM::PropertyBase* pProp, MM::ActionType eAct);
	int OnFanState(MM::PropertyBase* pProp, MM::ActionType eAct);
    int OnLeftLevels(MM::PropertyBase* pProp, MM::ActionType eAct);
    int OnRightLevels(MM::PropertyBase* pProp, MM::ActionType eAct);
    int OnImageFormat(MM::PropertyBase* pProp, MM::ActionType eAct);
	int OnTriggerMode(MM::PropertyBase* pProp, MM::ActionType eAct);
	int OnTriggerExpMode(MM::PropertyBase* pProp, MM::ActionType eAct);
	int OnTriggerEdgeMode(MM::PropertyBase* pProp, MM::ActionType eAct);
	int OnTriggerDelay(MM::PropertyBase* pProp, MM::ActionType eAct);
	int OnTriggerFilter(MM::PropertyBase* pProp, MM::ActionType eAct);
	int OnTriggerFrames(MM::PropertyBase* pProp, MM::ActionType eAct);
    int OnTriggerTotalFrames(MM::PropertyBase* pProp, MM::ActionType eAct);
	int OnTriggerDoSoftware(MM::PropertyBase* pProp, MM::ActionType eAct);
	int OnSharpness(MM::PropertyBase* pProp, MM::ActionType eAct);
	int OnDPCLevel(MM::PropertyBase* pProp, MM::ActionType eAct);
    int OnDPCAdjust(MM::PropertyBase* pProp, MM::ActionType eAct);
	int OnBlackLevel(MM::PropertyBase* pProp, MM::ActionType eAct);

	int OnTrgOutPortMode(MM::PropertyBase* pProp, MM::ActionType eAct);
	int OnTrgOutKindMode(MM::PropertyBase* pProp, MM::ActionType eAct);
	int OnTrgOutEdgeMode(MM::PropertyBase* pProp, MM::ActionType eAct);
	int OnTrgOutDelay(MM::PropertyBase* pProp, MM::ActionType eAct);
	int OnTrgOutWidth(MM::PropertyBase* pProp, MM::ActionType eAct);
	int OnWaitForTimeOut(MM::PropertyBase* pProp, MM::ActionType eAct);
/*
   int OnMaxExposure(MM::PropertyBase* pProp, MM::ActionType eAct);             // 设置曝光最大值上限
 
   int OnMono(MM::PropertyBase* pProp, MM::ActionType eAct);					// 彩色模式

   int OnTemperatureState(MM::PropertyBase* pProp, MM::ActionType eAct);        // 温控开关
   int OnTemperatureCurrent(MM::PropertyBase* pProp, MM::ActionType eAct);      // 当前温度
   int OnTemperatureCooling(MM::PropertyBase* pProp, MM::ActionType eAct);      // 目标温度
*/ 

    int OnReadoutTime(MM::PropertyBase* pProp, MM::ActionType eAct);
    int OnScanMode(MM::PropertyBase* pProp, MM::ActionType eAct);
    int OnErrorSimulation(MM::PropertyBase* , MM::ActionType );
    int OnCameraCCDXSize(MM::PropertyBase* , MM::ActionType );
    int OnCameraCCDYSize(MM::PropertyBase* , MM::ActionType );
    int OnTriggerDevice(MM::PropertyBase* pProp, MM::ActionType eAct);
    int OnDropPixels(MM::PropertyBase* pProp, MM::ActionType eAct);
    int OnFastImage(MM::PropertyBase* pProp, MM::ActionType eAct);
    int OnSaturatePixels(MM::PropertyBase* pProp, MM::ActionType eAct);
    int OnFractionOfPixelsToDropOrSaturate(MM::PropertyBase* pProp, MM::ActionType eAct);
    int OnShouldRotateImages(MM::PropertyBase* pProp, MM::ActionType eAct);
    int OnShouldDisplayImageNumber(MM::PropertyBase* pProp, MM::ActionType eAct);
    int OnStripeWidth(MM::PropertyBase* pProp, MM::ActionType eAct);
    int OnCCDTemp(MM::PropertyBase* pProp, MM::ActionType eAct);
    int OnIsSequenceable(MM::PropertyBase* pProp, MM::ActionType eAct);


private:
	int SetAllowedDepth();
    int SetAllowedBinning();
    int SetAllowedBinningSum();
    int SetAllowedPixelClock();
    int SetAllowedFanGear();

	int SetAllowedGainMode();
    int SetAllowedImageMode();
	int SetAllowedRSMode();
	int SetAllowedRSDir();
	int SetAllowedRSReset();
	int SetAllowedTestImg();
	int SetAllowedClrTemp();
    int SetAllowedShutterMode();

    void TestResourceLocking(const bool);
    void GenerateEmptyImage(ImgBuffer& img);
    void GenerateSyntheticImage(ImgBuffer& img, double exp);
    int ResizeImageBuffer();

    void ResizeBinImageBufferFL9BW(int &width, int &height);
    void ResizeBinImageBufferFL26BW(int &width, int &height);

	bool IsSupport95V2New()     { return DHYANA_D95_V2   == m_nPID && m_nBCD >= 0x2000; }
	bool IsSupport401DNew()     { return DHYANA_401D     == m_nPID && m_nBCD >= 0x2000; }
	bool IsSupport201DNew()     { return DHYANA_201D     == m_nPID && m_nBCD >= 0x2000; }
	bool IsSupport400BSIV3New() { return DHYANA_400BSIV3 == m_nPID && m_nBCD >= 0x2000; }
	bool IsSupportAries16()     { return 0xE424 == m_nPID || 0xE425 == m_nPID; }

    static const double nominalPixelSizeUm_;

    double exposureMaximum_;
    double exposureMinimum_;
    double dPhase_;
    ImgBuffer img_;
    bool busy_;
    bool stopOnOverFlow_;
    bool initialized_;
    double readoutUs_;
    MM::MMTime readoutStartTime_;
    long scanMode_;
    int bitDepth_;
    unsigned roiX_;
    unsigned roiY_;
    MM::MMTime sequenceStartTime_;
    bool isSequenceable_;
    long sequenceMaxLength_;
    bool sequenceRunning_;
    unsigned long sequenceIndex_;
    double GetSequenceExposure();
    std::vector<double> exposureSequence_;
    long imageCounter_;
    long binSize_;
    long cameraCCDXSize_;
    long cameraCCDYSize_;
    double ccdT_;
    std::string triggerDevice_;

    bool stopOnOverflow_;

    bool dropPixels_;
    bool fastImage_;
    bool saturatePixels_;
    double fractionOfPixelsToDropOrSaturate_;
    bool shouldRotateImages_;
    bool shouldDisplayImageNumber_;
    double stripeWidth_;

    double testProperty_[10];
    MMThreadLock imgPixelsLock_;
    int nComponents_;
    bool returnToSoftwareTriggers_;

    friend class CTUCamThread;
    CTUCamThread * thd_;

private:
	void TestImage(ImgBuffer& img, double exp);

    static void __cdecl GetTemperatureThread(LPVOID lParam);    // The thread get the value of temperature
    
	void RunTemperature();

    int InitTUCamApi();
    int UninitTUCamApi();

    int AllocBuffer();
    int ResizeBuffer();
    int ReleaseBuffer();

    int StopCapture();
    int StartCapture();
    int WaitForFrame(ImgBuffer& img, int timeOut = 10000);

    bool SaveRaw(char *pfileName, unsigned char *pData, unsigned long ulSize);
	bool isSupportFanCool();
	bool isSupportFanWaterCool();
	bool isSupportSoftProtect();

	void UpdateSlitHeightRange();
	void UpdateExpRange();
    void UpdateLevelsRange();
/*
    void LoadProfile();
    void SaveProfile();
*/
	static int   	s_nNumCam;				// The number of cameras
	static int		s_nCntCam;				// The count of camera

	int             m_nWaitForFrameTimeOut; // The WaitForFrameTimeOut
	int             m_nDriverType;          // The Driver Type
	int             m_nPID;                 // The PID 
	int             m_nBCD;                 // The BCD
    int             m_nIdxGain;             // The gain mode
    int             m_nMaxHeight;           // The max height size
	int             m_nTriType;             // The trigger type   
	int             m_nZeroTemp;            // The zero temperature reference value

    char            m_szImgPath[MAX_PATH];  // The save image path
    float           m_fCurTemp;             // The current temperature
    float           m_fValTemp;             // The current temperature
    float           m_fScaTemp;             // The scale of temperature
    int             m_nMidTemp;             // The middle value of temperature
    bool            m_bROI;                 // The ROI state
    bool            m_bSaving;              // The tag of save image            
    bool            m_bTemping;             // The get temperature state
    bool            m_bLiving;              // The capturing state
	bool            m_bAcquisition;         // The Acquisition
	bool            m_bCC1Support;          // The support cc1
	bool            m_bTriEn;
	bool            m_bOffsetEn;
	bool            m_bTempEn;              // The support temperature

	HANDLE          m_hThdWaitEvt;          // The waiting frame thread event handle
    HANDLE          m_hThdTempEvt;          // To get the value of temperature event handle

    TUCAM_INIT       m_itApi;               // The initialize SDK environment
    TUCAM_OPEN       m_opCam;               // The camera open parameters
    TUCAM_FRAME      m_frame;               // The frame object
	TUCAM_TRIGGER_ATTR m_tgrAttr;			// The trigger parameters
	TUCAM_TRGOUT_ATTR  m_tgrOutAttr;        // The output trigger parameters
	TUCAM_TRGOUTPUT    m_tgrOutPara;        // The output trigger parameter port
	TUCAM_RSPARA       m_rsPara;            // The rolling scan parameter
};

class CTUCamThread : public MMDeviceThreadBase
{
   friend class CMMTUCam;
   enum { default_numImages=1, default_intervalMS = 100 };
   public:
      CTUCamThread(CMMTUCam* pCam);
      ~CTUCamThread();
      void Stop();
      void Start(long numImages, double intervalMs);
      bool IsStopped();
      void Suspend();
      bool IsSuspended();
      void Resume();
      double GetIntervalMs(){return intervalMs_;}                               
      void SetLength(long images) {numImages_ = images;}                        
      long GetLength() const {return numImages_;}
      long GetImageCounter(){return imageCounter_;}                             
      MM::MMTime GetStartTime(){return startTime_;}                             
      MM::MMTime GetActualDuration(){return actualDuration_;}
   private:                                                                     
      int svc(void) throw();
      double intervalMs_;                                                       
      long numImages_;                                                          
      long imageCounter_;                                                       
      bool stop_;                                                               
      bool suspend_;                                                            
      CMMTUCam* camera_;                                                     
      MM::MMTime startTime_;                                                    
      MM::MMTime actualDuration_;                                               
      MM::MMTime lastFrameTime_;                                                
      MMThreadLock stopLock_;                                                   
      MMThreadLock suspendLock_;                                                
};  


#endif //_MMTUCAM_H_
