//-----------------------------------------------------------------------------//
// Name        | SC2_defs.h                  | Type: ( ) source                //
//-------------------------------------------|       (*) header                //
// Project     | PCO                         |       ( ) others                //
//-----------------------------------------------------------------------------//
// Purpose     | Defines, constants for use with SDK commands for              //
//             | pco.camera (SC2)                                              //
//-----------------------------------------------------------------------------//
// Author      | MBl/FRe/LWa/AGr and others, Excelitas PCO GmbH                //
//-----------------------------------------------------------------------------//
// Notes       |                                                               //
//-----------------------------------------------------------------------------//
// (c) 2003-2021 Excelitas PCO GmbH * Donaupark 11 * D-93309 Kelheim / Germany //
// *  Phone: +49 (0)9441 / 2005-0  *                                           //
// *  Fax:   +49 (0)9441 / 2005-20 * Email: pco@excelitas.com                  //
//-----------------------------------------------------------------------------//


// Do not change any values after release! Only additions are allowed!

#ifndef SC2_DEFS_H
#define SC2_DEFS_H


// ------------------------------------------------------------------------ //
// -- Defines for Get Camera Type Command: -------------------------------- //
// ------------------------------------------------------------------------ //

// pco.camera types
#define CAMERATYPE_PCO1200HS     0x0100
#define CAMERATYPE_PCO1300       0x0200
#define CAMERATYPE_PCO1600       0x0220
#define CAMERATYPE_PCO2000       0x0240
#define CAMERATYPE_PCO4000       0x0260

// pco.1300 types
#define CAMERATYPE_ROCHEHTC      0x0800 // Roche OEM
#define CAMERATYPE_284XS         0x0800
#define CAMERATYPE_KODAK1300OEM  0x0820 // Kodak OEM

// pco.1400 types
#define CAMERATYPE_PCO1400       0x0830
#define CAMERATYPE_NEWGEN        0x0840 // Roche OEM
#define CAMERATYPE_PROVEHR       0x0850 // Zeiss OEM

// pco.usb.pixelfly
#define CAMERATYPE_PCO_USBPIXELFLY        0x0900


// pco.dimax types
#define CAMERATYPE_PCO_DIMAX_STD           0x1000
#define CAMERATYPE_PCO_DIMAX_TV            0x1010

#define CAMERATYPE_PCO_DIMAX_AUTOMOTIVE    0x1020   // obsolete and not used for the pco.dimax, please remove from your sources!
#define CAMERATYPE_PCO_DIMAX_CS            0x1020   // code is now used for pco.dimax CS

#define CAMERASUBTYPE_PCO_DIMAX_Weisscam   0x0064   // 100 = Weisscam, all features
#define CAMERASUBTYPE_PCO_DIMAX_HD         0x80FF   // pco.dimax HD
#define CAMERASUBTYPE_PCO_DIMAX_HD_plus    0xC0FF   // pco.dimax HD+
#define CAMERASUBTYPE_PCO_DIMAX_X35        0x00C8   // 200 = Weisscam/P+S HD35

#define CAMERASUBTYPE_PCO_DIMAX_HS1        0x207F
#define CAMERASUBTYPE_PCO_DIMAX_HS2        0x217F
#define CAMERASUBTYPE_PCO_DIMAX_HS4        0x237F

#define CAMERASUBTYPE_PCO_DIMAX_CS_AM_DEPRECATED      0x407F
#define CAMERASUBTYPE_PCO_DIMAX_CS_1       0x417F
#define CAMERASUBTYPE_PCO_DIMAX_CS_2       0x427F
#define CAMERASUBTYPE_PCO_DIMAX_CS_3       0x437F
#define CAMERASUBTYPE_PCO_DIMAX_CS_4       0x447F

// #define CAMERASUBTYPE_PCO_DIMAX_CS_1_PLUS  0x517F  // RFU
// #define CAMERASUBTYPE_PCO_DIMAX_CS_2_PLUS  0x527F  // RFU
// #define CAMERASUBTYPE_PCO_DIMAX_CS_3_PLUS  0x537F  // RFU
#define CAMERASUBTYPE_PCO_DIMAX_CS_4_PLUS  0x547F


// pco.sensicam types                   // tbd., all names are internal ids
#define CAMERATYPE_SC3_SONYQE    0x1200 // SC3 based - Sony 285
#define CAMERATYPE_SC3_EMTI      0x1210 // SC3 based - TI 285SPD
#define CAMERATYPE_SC3_KODAK4800 0x1220 // SC3 based - Kodak KAI-16000



// pco.edge types
#define CAMERATYPE_PCO_EDGE                  0x1300 // pco.edge 5.5 (Sensor CIS2521) Interface: CameraLink , rolling shutter
#define CAMERATYPE_PCO_EDGE_42               0x1302 // pco.edge 4.2 (Sensor CIS2020) Interface: CameraLink , rolling shutter
#define CAMERATYPE_PCO_EDGE_GL               0x1310 // pco.edge 5.5 (Sensor CIS2521) Interface: CameraLink , global  shutter
#define CAMERATYPE_PCO_EDGE_USB3             0x1320 // pco.edge     (all sensors   ) Interface: USB 3.0    ,(all shutter modes)
#define CAMERATYPE_PCO_EDGE_HS               0x1340 // pco.edge     (all sensors   ) Interface: high speed ,(all shutter modes)
#define CAMERATYPE_PCO_EDGE_MT               0x1304 // pco.edge MT2 (all sensors   ) Interface: CameraLink Base, rolling shutter


#define CAMERASUBTYPE_PCO_EDGE_SPRINGFIELD   0x0006
#define CAMERASUBTYPE_PCO_EDGE_31            0x0031
#define CAMERASUBTYPE_PCO_EDGE_42            0x0042
#define CAMERASUBTYPE_PCO_EDGE_55            0x0055
#define CAMERASUBTYPE_PCO_EDGE_DEVELOPMENT   0x0100
#define CAMERASUBTYPE_PCO_EDGE_X2            0x0200
#define CAMERASUBTYPE_PCO_EDGE_RESOLFT       0x0300
#define CAMERASUBTYPE_PCO_EDGE_GOLD          0x0FF0
#define CAMERASUBTYPE_PCO_EDGE_DUAL_CLOCK    0x000D
#define CAMERASUBTYPE_PCO_EDGE_DICAM         0xDC00
#define CAMERASUBTYPE_PCO_EDGE_42_LT         0x8042


// pco.flim types
#define CAMERATYPE_PCO_FLIM      0x1400 // pco.flim

// pco.flow types
#define CAMERATYPE_PCO_FLOW      0x1500 // pco.flow

// pco.panda types
#define CAMERATYPE_PCO_PANDA     0x1600 // pco.panda (deprecated, use CAMERATYPE_PCO_FAMILY_PANDA for the future)



// camera types for pco camera families
#define CAMERATYPE_PCO_FAMILY_PANDA          0x1600 // pco.panda
#define CAMERATYPE_PCO_FAMILY_EDGE           0x1800 // pco.edge
#define CAMERATYPE_PCO_FAMILY_DICAM          0x1700 // pco.dicam
#define CAMERATYPE_PCO_FAMILY_DIMAX          0x1900 // pco.dimax
#define CAMERATYPE_PCO_FAMILY_PIXELFLY       0x1A00 // pco.pixelfly
#define CAMERATYPE_PCO_FAMILY_OEM            0x1B00


// PANDA Family
#define CAMERASUBTYPE_PCO_PANDA_42                0x0000 // pco.panda 4.2
#define CAMERASUBTYPE_PCO_PANDA_42_BI             0x0001 // pco.panda 4.2 bi
#define CAMERASUBTYPE_PCO_PANDA_150               0x0002 // pco.panda 15
#define CAMERASUBTYPE_PCO_PANDA_OEM1              0x0003
#define CAMERASUBTYPE_PCO_PANDA_260               0x0004 // pco.panda 26
#define CAMERASUBTYPE_PCO_PANDA_260_DS            0x0005 // pco.panda 26 DS
#define CAMERASUBTYPE_PCO_PANDA_OEM2              0x0006
#define CAMERASUBTYPE_PCO_PANDA_260_CLHS          0x0007 // pco.panda 26 CLHS
#define CAMERASUBTYPE_PCO_PANDA_260_DS_CLHS       0x0008 // pco.panda 26 DS CLHS

// EDGE Family
#define CAMERASUBTYPE_PCO_EDGE_42_BI              0x0001 // pco.edge 4.2 bi
#define CAMERASUBTYPE_PCO_EDGE_260                0x0002 // pco.edge 26
#define CAMERASUBTYPE_PCO_EDGE_260_DS             0x0003 // pco.edge 26 DS
#define CAMERASUBTYPE_PCO_EDGE_62_LE              0x0004 // pco.edge 6.2 LE
#define CAMERASUBTYPE_PCO_EDGE_100_BI_CLHS        0x0005 // pco.edge 10 bi CLHS
#define CAMERASUBTYPE_PCO_EDGE_100_BI_LT_CLHS     0x0006 // pco.edge 10 bi LT CLHS
#define CAMERASUBTYPE_PCO_EDGE_OEM3               0x0007
#define CAMERASUBTYPE_PCO_EDGE_260_CLHS           0x0008 // pco.edge 26 CLHS
#define CAMERASUBTYPE_PCO_EDGE_260_DS_CLHS        0x0009 // pco.edge 26 DS CLHS
#define CAMERASUBTYPE_PCO_EDGE_260_NIR_CLHS       0x000A // pco.edge 26 NIR CLHS
#define CAMERASUBTYPE_PCO_EDGE_94_BI_CLHS         0x000B // pco.edge 9.4 bi CLHS

// DICAM Family
#define CAMERASUBTYPE_PCO_DICAM_C1                0x0001 // pco.dicam C1
#define CAMERASUBTYPE_PCO_DICAM_C2                0x0002 // pco.dicam C2
#define CAMERASUBTYPE_PCO_DICAM_C3                0x0003 // pco.dicam C3
#define CAMERASUBTYPE_PCO_DICAM_C4                0x0004 // pco.dicam C4
#define CAMERASUBTYPE_PCO_DICAM_C5                0x0005 // pco.dicam C5
#define CAMERASUBTYPE_PCO_DICAM_C6                0x0006 // pco.dicam C6
#define CAMERASUBTYPE_PCO_DICAM_C7                0x0007 // pco.dicam C7
#define CAMERASUBTYPE_PCO_DICAM_C8                0x0008 // pco.dicam C8

// DIMAX Family
#define CAMERASUBTYPE_PCO_DIMAX_OEM4              0x0001
#define CAMERASUBTYPE_PCO_DIMAX_36_ST_CLHS        0x0002 // pco.dimax 3.6 ST CLHS (air/water)

// OEMs
#define CAMERASUBTYPE_PCO_OEM_OEM5                0x0001
#define CAMERASUBTYPE_PCO_OEM_OEM6                0x0002


// PIXELFLY Family
#define CAMERASUBTYPE_PCO_PIXELFLY_13_SWIR   0x0001 // pco.pixelfly 1.3 SWIR



//#define CAMERATYPE_PCOUPDATE     0xFFFF   // indicates Camera in update mode!

// ------------------------------------------------------------------------ //
// -- Defines for Interfaces ---------------------------------------------- //
// ------------------------------------------------------------------------ //
// These defines are camera internal defines and are not SDK related!
#define INTERFACE_FIREWIRE       0x0001
#define INTERFACE_CAMERALINK     0x0002
#define INTERFACE_USB            0x0003
#define INTERFACE_ETHERNET       0x0004
#define INTERFACE_SERIAL         0x0005
#define INTERFACE_USB3           0x0006
#define INTERFACE_CAMERALINKHS   0x0007
#define INTERFACE_COAXPRESS      0x0008
#define INTERFACE_USB31_GEN1     0x0009


// ------------------------------------------------------------------------ //
// -- Defines for USB Interface ------------------------------------------- //
// ------------------------------------------------------------------------ //
// USB device codes are for internal use only
// these defines are therefore in sc2_defs_intern.h


// ------------------------------------------------------------------------ //
// -- Defines for CameraLink DataFormat  ---------------------------------- //
// ------------------------------------------------------------------------ //

// Obsolete. Use defines in SC2_SdkAddendum.h
#define CL_DATAFORMAT     0x0F
#define CL_FORMAT_1x16    0x01
#define CL_FORMAT_2x12    0x02
#define CL_FORMAT_3x8     0x03
#define CL_FORMAT_4x16    0x04
#define CL_FORMAT_5x16    0x05
#define CL_FORMAT_5x12    0x07

#define CL_TESTPATTERN    0xF0
#define CL_TESTPATTERN_1  0x10
#define CL_TESTPATTERN_2  0x20
#define CL_TESTPATTERN_3  0x30

// ------------------------------------------------------------------------ //
// -- Bitmask Defines for CameraLink Transmit------------------------------ //
// ------------------------------------------------------------------------ //

#define CL_TRANSMIT_ENABLE          0x01
#define CL_TRANSMIT_LONGGAP         0x02

// ------------------------------------------------------------------------ //
// -- Defines for CameraLink CCLines     ---------------------------------- //
// ------------------------------------------------------------------------ //

#define CL_CCLINE_LINE1_TRIGGER           0x01
#define CL_CCLINE_LINE2_ACQUIRE           0x02
#define CL_CCLINE_LINE3_HANDSHAKE         0x04
#define CL_CCLINE_LINE4_TRANSMIT_ENABLE   0x08



// ------------------------------------------------------------------------ //
// -- Defines for Get Camera Health Status Command: ----------------------- //
// ------------------------------------------------------------------------ //

// mask bits: evaluate as follows: if (stat & ErrorSensorTemperature) ... //

#define WARNING_POWERSUPPLYVOLTAGERANGE 0x00000001
#define WARNING_POWERSUPPLYTEMPERATURE  0x00000002
#define WARNING_CAMERATEMPERATURE       0x00000004
#define WARNING_SENSORTEMPERATURE       0x00000008
#define WARNING_EXTERNAL_BATTERY_LOW    0x00000010
#define WARNING_OFFSET_REGULATION_RANGE 0x00000020

#define WARNING_CAMERARAM               0x00020000


#define ERROR_POWERSUPPLYVOLTAGERANGE   0x00000001
#define ERROR_POWERSUPPLYTEMPERATURE    0x00000002
#define ERROR_CAMERATEMPERATURE         0x00000004
#define ERROR_SENSORTEMPERATURE         0x00000008

#define ERROR_EXTERNAL_BATTERY_LOW      0x00000010
#define ERROR_FIRMWARE_CORRUPTED        0x00000020
#define ERROR_CAMERA_INIT               0x00000040

#define ERROR_CAMERAINTERFACE           0x00010000
#define ERROR_CAMERARAM                 0x00020000
#define ERROR_CAMERAMAINBOARD           0x00040000
#define ERROR_CAMERAHEADBOARD           0x00080000


#define STATUS_DEFAULT_STATE                 0x00000001
#define STATUS_SETTINGS_VALID                0x00000002
#define STATUS_RECORDING_ON                  0x00000004
#define STATUS_READ_IMAGE_ON                 0x00000008
#define STATUS_FRAMERATE_VALID               0x00000010
#define STATUS_SEQ_STOP_TRIGGERED            0x00000020
#define STATUS_LOCKED_TO_EXTSYNC             0x00000040
#define STATUS_EXT_BATTERY_AVAILABLE         0x00000080
#define STATUS_IS_IN_POWERSAVE               0x00000100
#define STATUS_POWERSAVE_LEFT                0x00000200
#define STATUS_LOCKED_TO_IRIG                0x00000400

#define STATUS_NON_PCO_STANDARD_SETTINGS     0x20000000
#define STATUS_DESCRIPTOR_MUST_BE_RELOADED   0x40000000
#define STATUS_IS_IN_BOOTLOADER              0x80000000


// ------------------------------------------------------------------------ //
// -- Defines for Get Camera Description Command: ------------------------- //
// ------------------------------------------------------------------------ //

  // Description type

#define DESCRIPTION_STANDARD     0x0000       // Standard Descripton
#define DESCRIPTION_2            0x0001       // Descripton nr. 2
#define DESCRIPTION_INTENSIFIED  0x0002       // Descripton nr. 3 (Intensified Camera Description)
#define DESCRIPTION_3            0x0003       // Descripton nr. 4 (Extended Camera Description)

// ------------------------------------------------------------------------ //
// -- Sensor type definitions --------------------------------------------- //
// ------------------------------------------------------------------------ //
  // Sensor Type
  // ATTENTION: Lowest bit is reserved for COLOR CCDs
  // In case a new color CCD is added the lowest bit MUST be set!!!
#define SENSOR_ICX285AL           0x0010      // Sony
#define SENSOR_ICX285AK           0x0011      // Sony
#define SENSOR_ICX263AL           0x0020      // Sony
#define SENSOR_ICX263AK           0x0021      // Sony
#define SENSOR_ICX274AL           0x0030      // Sony
#define SENSOR_ICX274AK           0x0031      // Sony
#define SENSOR_ICX407AL           0x0040      // Sony
#define SENSOR_ICX407AK           0x0041      // Sony
#define SENSOR_ICX414AL           0x0050      // Sony
#define SENSOR_ICX414AK           0x0051      // Sony
#define SENSOR_ICX407BLA          0x0060      // Sony UV type
#define SENSOR_IMX990             0x0070      // Sony SWIR

#define SENSOR_KAI2000M           0x0110      // Kodak
#define SENSOR_KAI2000CM          0x0111      // Kodak
#define SENSOR_KAI2001M           0x0120      // Kodak
#define SENSOR_KAI2001CM          0x0121      // Kodak
#define SENSOR_KAI2002M           0x0122      // Kodak slow roi
#define SENSOR_KAI2002CM          0x0123      // Kodak slow roi

#define SENSOR_KAI4010M           0x0130      // Kodak
#define SENSOR_KAI4010CM          0x0131      // Kodak
#define SENSOR_KAI4011M           0x0132      // Kodak slow roi
#define SENSOR_KAI4011CM          0x0133      // Kodak slow roi

#define SENSOR_KAI4020M           0x0140      // Kodak
#define SENSOR_KAI4020CM          0x0141      // Kodak
#define SENSOR_KAI4021M           0x0142      // Kodak slow roi
#define SENSOR_KAI4021CM          0x0143      // Kodak slow roi
#define SENSOR_KAI4022M           0x0144      // Kodak 4022 monochrom
#define SENSOR_KAI4022CM          0x0145      // Kodak 4022 color

#define SENSOR_KAI11000M          0x0150      // Kodak
#define SENSOR_KAI11000CM         0x0151      // Kodak
#define SENSOR_KAI11002M          0x0152      // Kodak slow roi
#define SENSOR_KAI11002CM         0x0153      // Kodak slow roi

#define SENSOR_KAI16000AXA        0x0160      // Kodak t:4960x3324, e:4904x3280, a:4872x3248
#define SENSOR_KAI16000CXA        0x0161      // Kodak

#define SENSOR_MV13BW             0x1010      // Micron
#define SENSOR_MV13COL            0x1011      // Micron

#define SENSOR_CIS2051_V1_FI_BW   0x2000      //Fairchild front illuminated
#define SENSOR_CIS2051_V1_FI_COL  0x2001
#define SENSOR_CIS1042_V1_FI_BW   0x2002
#define SENSOR_CIS2051_V1_BI_BW   0x2010      //Fairchild back illuminated

#define SENSOR_LTN4323_BI_BW      0x2020      // BAE LTN4323 B/W
#define SENSOR_LTN4323_BI_COL     0x2021      // BAE LTN4323 Color
#define SENSOR_HWK4123_BI_BW      0x2030      // BAE HWK4123 B/W


//obsolete #define SENSOR_CCD87           0x2010         // E2V
//obsolete #define SENSOR_TC253           0x2110         // TI
#define SENSOR_TC285SPD           0x2120      // TI 285SPD

#define SENSOR_CYPRESS_RR_V1_BW   0x3000      // CYPRESS RoadRunner V1 B/W
#define SENSOR_CYPRESS_RR_V1_COL  0x3001      // CYPRESS RoadRunner V1 Color

#define SENSOR_CMOSIS_CMV12000_BW    0x3100   // CMOSIS CMV12000 4096x3072 b/w
#define SENSOR_CMOSIS_CMV12000_COL   0x3101   // CMOSIS CMV12000 4096x3072 color

#define SENSOR_QMFLIM_V2B_BW      0x4000      // CSEM QMFLIM V2B B/W

//#define SENSOR_GPIXEL_X2_BW       0x5000  // Gpixel 2k
//#define SENSOR_GPIXEL_X2_COL      0x5001  // Gpixel 2k

//#define SENSOR_GPIXEL_X5_BW       0x5100  // Gpixel 5k

#define SENSOR_GPIXEL_GSENSE2020_BW       0x5000  // Gpixel GSENSE2020 4.2M
#define SENSOR_GPIXEL_GSENSE2020_COL      0x5001  // Gpixel GSENSE2020 4.2M

#define SENSOR_GPIXEL_GSENSE2020BI_BW     0x5002  // Gpixel GSENSE2020BSI 4.2M-BI

#define SENSOR_GPIXEL_GSENSE5130_BW       0x5004  // Gpixel GSENSE5130 15M
#define SENSOR_GPIXEL_GSENSE5130_COL      0x5005  // Gpixel GSENSE5130 15M

#define SENSOR_GPIXEL_GMAX0505_BW         0x5006  // Gpixel GMAX0505 26M
#define SENSOR_GPIXEL_GMAX0505_COL        0x5007  // Gpixel GMAX0505 26M

#define SENSOR_GPIXEL_GSPRINT1105_BW      0x5008  // Gpixel GSPRINT1105 5M 
// ------------------------------------------------------------------------ //
// -- Defines for Get Info String Command: -------------------------------- //
// ------------------------------------------------------------------------ //
typedef struct
{
 WORD  wTypdef;
 char  szName[40];
}PCO_SENSOR_TYPE_DEF;

#if defined PCO_SENSOR_CREATE_OBJECT
const PCO_SENSOR_TYPE_DEF far pco_sensor[] =
{
               // Sony sensor types
              { SENSOR_ICX285AL,               "Sony ICX285AL"                      },
              { SENSOR_ICX285AK,               "Sony ICX285AK"                      },
              { SENSOR_ICX263AL,               "Sony ICX263AL"                      },
              { SENSOR_ICX263AK,               "Sony ICX263AK"                      },
              { SENSOR_ICX274AL,               "Sony ICX274AL"                      },
              { SENSOR_ICX274AK,               "Sony ICX274AK"                      },
              { SENSOR_ICX407AL,               "Sony ICX407AL"                      },
              { SENSOR_ICX407AK,               "Sony ICX407AK"                      },
              { SENSOR_ICX414AL,               "Sony ICX414AL"                      },
              { SENSOR_ICX414AK,               "Sony ICX414AK"                      },
              { SENSOR_ICX407BLA,              "Sony ICX407BLA"                     },
              { SENSOR_IMX990,                 "Sony IMX990"                        },

               // Kodak sensor types
              { SENSOR_KAI2000M,               "Kodak KAI2000M"                     },
              { SENSOR_KAI2000CM,              "Kodak KAI2000CM"                    },
              { SENSOR_KAI2001M,               "Kodak KAI2001M"                     },
              { SENSOR_KAI2001CM,              "Kodak KAI2001CM"                    },
              { SENSOR_KAI2002M,               "Kodak KAI2002M"                     },
              { SENSOR_KAI2002CM,              "Kodak KAI2002CM"                    },
              { SENSOR_KAI4010M,               "Kodak KAI4010M"                     },
              { SENSOR_KAI4010CM,              "Kodak KAI4010CM"                    },
              { SENSOR_KAI4011M,               "Kodak KAI4011M"                     },
              { SENSOR_KAI4011CM,              "Kodak KAI4011CM"                    },
              { SENSOR_KAI4020M,               "Kodak KAI4020M"                     },
              { SENSOR_KAI4020CM,              "Kodak KAI4020CM"                    },
              { SENSOR_KAI4021M,               "Kodak KAI4021M"                     },
              { SENSOR_KAI4021CM,              "Kodak KAI4021CM"                    },
              { SENSOR_KAI4022M,               "Kodak KAI4022M"                     },
              { SENSOR_KAI4022CM,              "Kodak KAI4022CM"                    },
              { SENSOR_KAI11000M,              "Kodak KAI11000M"                    },
              { SENSOR_KAI11000CM,             "Kodak KAI11000CM"                   },
              { SENSOR_KAI11002M,              "Kodak KAI11002M"                    },
              { SENSOR_KAI11002CM,             "Kodak KAI11002CM"                   },
              { SENSOR_KAI16000AXA,            "Kodak KAI16000AXA"                  },
              { SENSOR_KAI16000CXA,            "Kodak KAI16000CXA"                  },

               // Mircon sensor types
              { SENSOR_MV13BW,                 "Micron MV13BW"                      },
              { SENSOR_MV13COL,                "Micron MV13COL"                     },

               // Other sensor types
              { SENSOR_TC285SPD,               "TI TC285SPD"                        },
              { SENSOR_QMFLIM_V2B_BW,          "QMFLIM V2B BW"                      },

              { SENSOR_CYPRESS_RR_V1_BW,       "Cypress Roadrunner V1 BW"           },
              { SENSOR_CYPRESS_RR_V1_COL,      "Cypress Roadrunner V1 Color"        },

              { SENSOR_CIS2051_V1_FI_BW,       "Fairchild CIS2521 V1 I-Front BW"    },
              { SENSOR_CIS2051_V1_FI_COL,      "Fairchild CIS2521 V1 I-Front Color" },
              { SENSOR_CIS1042_V1_FI_BW,       "Fairchild CIS2020 V1 I-Front BW"    },
              { SENSOR_CIS2051_V1_BI_BW,       "Fairchild CIS2521 V1 I-Back BW"     },

              { SENSOR_LTN4323_BI_BW,          "BAE LTN4323 BW"                     },
              { SENSOR_LTN4323_BI_COL,         "BAE LTN4323 Color"                  },
              { SENSOR_HWK4123_BI_BW,          "BAE HWK4123 BW"                     },

              { SENSOR_CMOSIS_CMV12000_BW,     "CMOSIS CMV12000 BW"                 },
              { SENSOR_CMOSIS_CMV12000_COL,    "CMOSIS CMV12000 Color"              },


              { SENSOR_GPIXEL_GSENSE2020_BW,   "Gpixel GSENSE2020 BW"               },
              { SENSOR_GPIXEL_GSENSE2020_COL,  "Gpixel GSENSE2020 Color"            },
              { SENSOR_GPIXEL_GSENSE2020BI_BW, "Gpixel GSENSE2020BSI BW"            },
              { SENSOR_GPIXEL_GSENSE5130_BW,   "Gpixel GSENSE5130 BW"               },
              { SENSOR_GPIXEL_GSENSE5130_COL,  "Gpixel GSENSE5130 Color"            },
              { SENSOR_GPIXEL_GMAX0505_BW,     "Gpixel GMAX0505 BW"                 },
              { SENSOR_GPIXEL_GMAX0505_COL,    "Gpixel GMAX0505 Color"              },
              { SENSOR_GPIXEL_GSPRINT1105_BW,  "Gpixel GSPRINT1105 BW"              }
};

const int far PCO_SENSOR_TYPE_DEF_NUM = sizeof(pco_sensor) / sizeof(pco_sensor[0]);

#else
extern const PCO_SENSOR_TYPE_DEF far pco_sensor[];
extern const int far PCO_SENSOR_TYPE_DEF_NUM;
#endif

#define INFO_STRING_CAMERA              1   // Camera name
#define INFO_STRING_SENSOR              2   // Sensor name
#define INFO_STRING_PCO_MATERIALNUMBER  3   // get PCO material number

#define INFO_STRING_BUILD               4   // Build number and date
#define INFO_STRING_PCO_INCLUDE         5   // PCO_Include rev used for building



  // these are defines for interpreting the dwGeneralCaps1 member of the
  // Camera Description structure.
  //
  // How to use the member:
  //
  // if (CameraDescription.dwGeneralCaps1 & GENERALCAPS1_NOISE_FILTER)
  // {
  //   noise filter can be used! ...
  //   ...

#define GENERALCAPS1_NOISE_FILTER                      0x00000001
#define GENERALCAPS1_HOTPIX_FILTER                     0x00000002
#define GENERALCAPS1_HOTPIX_ONLY_WITH_NOISE_FILTER     0x00000004
#define GENERALCAPS1_TIMESTAMP_ASCII_ONLY              0x00000008

#define GENERALCAPS1_DATAFORMAT2X12                    0x00000010
#define GENERALCAPS1_RECORD_STOP                       0x00000020 // Record stop event mode
#define GENERALCAPS1_HOT_PIXEL_CORRECTION              0x00000040
#define GENERALCAPS1_NO_EXTEXPCTRL                     0x00000080 // Ext. Exp. ctrl not possible

#define GENERALCAPS1_NO_TIMESTAMP                      0x00000100
#define GENERALCAPS1_NO_ACQUIREMODE                    0x00000200
#define GENERALCAPS1_DATAFORMAT4X16                    0x00000400
#define GENERALCAPS1_DATAFORMAT5X16                    0x00000800

#define GENERALCAPS1_NO_RECORDER                       0x00001000 // Camera has no internal memory
#define GENERALCAPS1_FAST_TIMING                       0x00002000 // Camera can be set to fast timing mode (PIV)
#define GENERALCAPS1_METADATA                          0x00004000 // Camera can produce metadata
#define GENERALCAPS1_SETFRAMERATE_ENABLED              0x00008000 // Camera allows Set/GetFrameRate cmd

#define GENERALCAPS1_CDI_MODE                          0x00010000 // Camera has Correlated Double Image Mode
#define GENERALCAPS1_CCM                               0x00020000 // Camera has CCM
#define GENERALCAPS1_EXTERNAL_SYNC                     0x00040000 // Camera can be synced externally
#define GENERALCAPS1_NO_GLOBAL_SHUTTER                 0x00080000 // Camera does not support global shutter
#define GENERALCAPS1_GLOBAL_RESET_MODE                 0x00100000 // Camera supports global reset rolling readout
#define GENERALCAPS1_EXT_ACQUIRE                       0x00200000 // Camera supports extended acquire command
#define GENERALCAPS1_FAN_LED_CONTROL                   0x00400000 // Camera supports fan and LED control command

#define GENERALCAPS1_ROI_VERT_SYMM_TO_HORZ_AXIS        0x00800000 // Camera vert.ROI must be symmetrical to horizontal axis
#define GENERALCAPS1_ROI_HORZ_SYMM_TO_VERT_AXIS        0x01000000 // Camera horz.ROI must be symmetrical to vertical axis

#define GENERALCAPS1_COOLING_SETPOINTS                 0x02000000 // Camera has cooling setpoints instead of cooling range
#define GENERALCAPS1_USER_INTERFACE                    0x04000000 // Camera has user interface commands

//define GENERALCAPS1_LAST_ONE!!!                      0x08000000 // This is the last flag.

#define GENERALCAPS1_ENHANCED_DESCRIPTOR_3             0x10000000 // Used for DESCRIPTOR_3
#define GENERALCAPS1_ENHANCED_DESCRIPTOR_INTENSIFIED   0x20000000
#define GENERALCAPS1_HW_IO_SIGNAL_DESCRIPTOR           0x40000000
#define GENERALCAPS1_ENHANCED_DESCRIPTOR_2             0x80000000


// dwGeneralCaps2 is for internal use only
// defines for interpreting the dwGeneralCaps2 member are therefore in sc2_defs_intern.h

// dwGeneralCaps3:
#define GENERALCAPS3_HDSDI_1G5                         0x00000001 // with HD/SDI interface, 1.5 GBit data rate
#define GENERALCAPS3_HDSDI_3G                          0x00000002 // with HD/SDI interface, 3.0 GBit data rate
#define GENERALCAPS3_IRIG_B_UNMODULATED                0x00000004 // can evaluate an IRIG B unmodulated signal
#define GENERALCAPS3_IRIG_B_MODULATED                  0x00000008 // can evaluate an IRIG B modulated signal

#define GENERALCAPS3_CAMERA_SYNC                       0x00000010 // has camera sync mode implemented
#define GENERALCAPS3_RESERVED0                         0x00000020 // reserved
#define GENERALCAPS3_HS_READOUT_MODE                   0x00000040 // special fast sensor readout mode
#define GENERALCAPS3_EXT_SYNC_1HZ_MODE                 0x00000080 // in trigger mode external synchronized, multiples of
                                                                  //   1 F/s can be set (until now: 100 Hz)

#define GENERALCAPS3_TRANSFER_MODE_ALIGN_8BIT          0x00000100 // possibility to align 8 bit transfer modes into 12 bit
#define GENERALCAPS3_MEMORY_COMPRESSION_GENERAL        0x00000200 // possibility to set general memory compression generally
#define GENERALCAPS3_MEMORY_COMPRESSION_PER_SEGMENT    0x00000400 // possibility to set general memory compression per segment
#define GENERALCAPS3_BINNING_MODE_EXTENDED             0x00000800 // supports extended binning modes e.g. average

#define GENERALCAPS3_CAMERA_SETUP                      0x00001000 // supports camera setup
#define GENERALCAPS3_LOW_LATENCY_EXTERNALTRIGGER       0x00002000 // supports low latency trigger mode (TRIGGER_MODE_LOW_LATENCY_EXTERNALTRIGGER)
#define GENERALCAPS3_BINNING_MODE_AVERAGE              0x00004000 // supports additional binning mode average
#define GENERALCAPS3_ENHANCED_READOUT_MODES            0x00008000 // supports enhanced readout modes; See read out capabilities and mode telegrams

#define GENERALCAPS3_OFFSET_BASED_TIMING_CALC          0x00010000 // Camera needs timing calculation exp = exp0 + x * expstep
#define GENERALCAPS3_CAMERA_SETUP_PIXELFORMAT          0x00020000 // Camera can be switched to different pixelformat output
#define GENERALCAPS3_CAMERA_SETUP_SENSOR_DYNRES        0x00040000 // Camera can be switched to different sensor dynamic resolution

// ------------------------------------------------------------------------ //
// -- Defines for Get/Set Camera Temperature Command: --------------------- //
// ------------------------------------------------------------------------ //

#define TEMPERATURE_NOT_AVAILABLE 0x8000

// ------------------------------------------------------------------------ //
// -- Defines for Get / Set Camera Setup: --------------------------------- //
// ------------------------------------------------------------------------ //
// 
// Common camera setup type
#define CAMERA_SETUP_TYPE_SHUTTERMODE              0x0000          // Shuttermode setup type
#define CAMERA_SETUP_TYPE_PIXELFORMAT              0x0001          // Pixelformat of camera image data output
#define CAMERA_SETUP_TYPE_SENSOR_DYNRES            0x0002          // Dynamic resolution of camera

// Dimax reserved types
#define PCO_DIMAX_CS_CAMERA_SETUP_TYPE_RSRVD_0     0x1001          // pco.dimax CS CameraSetup
#define PCO_DIMAX_CS_CAMERA_SETUP_TYPE_RSRVD_1     0x1002          // definitions for type parameter
#define PCO_DIMAX_CS_CAMERA_SETUP_TYPE_RSRVD_2     0x1004          // used for calibration purposes
#define PCO_DIMAX_CS_CAMERA_SETUP_TYPE_RSRVD_3     0x1008
#define PCO_DIMAX_CS_CAMERA_SETUP_TYPE_RSRVD_4     0x1010
#define PCO_DIMAX_CS_CAMERA_SETUP_TYPE_RSRVD_5     0x1020
#define PCO_DIMAX_CS_CAMERA_SETUP_TYPE_RSRVD_6     0x1040
#define PCO_DIMAX_CS_CAMERA_SETUP_TYPE_RSRVD_7     0x1080


// Camera dwSetupFlags[0] parameter for type SHUTTERMODE
#define PCO_EDGE_SETUP_ROLLING_SHUTTER             0x00000001      // rolling shutter
#define PCO_EDGE_SETUP_GLOBAL_SHUTTER              0x00000002      // global shutter
#define PCO_EDGE_SETUP_GLOBAL_RESET                0x00000004      // global reset rolling readout

// Camera requirement dwSetupFlags[2] parameter for type GENERAL:
#define SHUTTER_SETUP_REBOOT_NOT_REQUIRED          0x80000000      // A reboot of the camera is not required


// Camera dwSetupFlags[0] parameter for type PIXELFORMAT:
#define PIXELFORMAT_SETUP_16                       0x00000001      // 16 Bit Pixel Data
#define PIXELFORMAT_SETUP_14                       0x00000002      // 14 Bit Pixel Data in 16 Bit, MSB/LSB alignment selectable
#define PIXELFORMAT_SETUP_12                       0x00000004      // 12 Bit Pixel Data in 16 Bit, MSB/LSB alignment selectable
#define PIXELFORMAT_SETUP_10                       0x00000008      // 10 Bit Pixel Data in 16 Bit, MSB/LSB alignment selectable
#define PIXELFORMAT_SETUP_8                        0x00000010      //  8 Bit Pixel Data
#define PIXELFORMAT_SETUP_14P                      0x00000020      // 14 Bit Pixel Data packed in line
#define PIXELFORMAT_SETUP_12P                      0x00000040      // 12 Bit Pixel Data packed in line
#define PIXELFORMAT_SETUP_10P                      0x00000080      // 10 Bit Pixel Data packed in line

// add more format flags here if requested
//#define PIXELFORMAT_SETUP_RGB_888                0x00000100      //  8 cBit RGB Pixel Data 3 Byte per Pixel

// Camera capability dwSetupFlags[1] parameter for type PIXELFORMAT:
// use same defines as in dwSetupFlags[0]
// e.g. if PIXELFORMAT_SETUP_8 is set in capability dwSetupFlags[1], it can be selected from SC2_Set_Camera_Setup call

// Camera requirement dwSetupFlags[2] parameter for type PIXELFORMAT:
#define PIXELFORMAT_REBOOT_REQUIRED                0x80000000

// Camera dwSetupFlags[0] parameter for type DYNRES:
#define DYNRES_SETUP_16                            0x00000001
#define DYNRES_SETUP_15                            0x00000002
#define DYNRES_SETUP_14                            0x00000004
#define DYNRES_SETUP_13                            0x00000008
#define DYNRES_SETUP_12                            0x00000010
#define DYNRES_SETUP_11                            0x00000020
#define DYNRES_SETUP_10                            0x00000040
#define DYNRES_SETUP_9                             0x00000080
#define DYNRES_SETUP_8                             0x00000100

// Camera capability dwSetupFlags[1] parameter for type DYNRES:
// use same defines as in dwSetupFlags[0]
// e.g. if DYNRES_SETUP_10 is set in capability dwSetupFlags[1], it can be selected from SC2_Set_Camera_Setup call

// Camera requirement dwSetupFlags[2] parameter for type DYNRES:
#define DYNRES_SETUP_REBOOT_REQUIRED               0x80000000


// ------------------------------------------------------------------------ //
// -- Defines for User Interface Commands: -------------------------------- //
// ------------------------------------------------------------------------ //
#define USER_INTERFACE_DISABLE                         0x00
#define USER_INTERFACE_ENABLE                          0x01

#define USER_INTERFACE_TYPE_UART                       0x0001
#define USER_INTERFACE_TYPE_UART_UNIDIRECTIONAL        0x0002
#define USER_INTERFACE_TYPE_USART                      0x0003
#define USER_INTERFACE_TYPE_SPI                        0x0004
#define USER_INTERFACE_TYPE_I2C                        0x0005

#define USER_INTERFACE_OPTIONS_UART_PARITY_NONE        0x00000001
#define USER_INTERFACE_OPTIONS_UART_PARITY_EVEN        0x00000002
#define USER_INTERFACE_OPTIONS_UART_PARITY_ODD         0x00000004

#define USER_INTERFACE_EQUIPMENT_LENS_CONTROL_BIRGER   0x00000001

#define USER_INTERFACE_HANDSHAKE_TYPE_NONE             0x0001
#define USER_INTERFACE_HANDSHAKE_TYPE_RTS_CTS          0x0002
#define USER_INTERFACE_HANDSHAKE_TYPE_XON_XOFF         0x0004


#define USER_INTERFACE_DO_NOT_CLEAR_BUFFERS            0x00
#define USER_INTERFACE_CLEAR_RX_BUFFER                 0x01
#define USER_INTERFACE_CLEAR_TX_BUFFER                 0x02
#define USER_INTERFACE_CLEAR_RX_AND_TX_BUFFER          0x03





// ------------------------------------------------------------------------ //
// -- Defines for Read/Write Mailbox & Get Mailbox Status Commands: ------- //
// ------------------------------------------------------------------------ //

#define MAILBOX_READ_STATUS_NO_VALID_MESSAGE                0x0000
#define MAILBOX_READ_STATUS_MESSAGE_VALID                   0x0001
#define MAILBOX_READ_STATUS_MESSAGE_HAS_BEEN_READ           0x0003

#define MAILBOX_STATUS_NO_VALID_MESSAGE                     0x0000
#define MAILBOX_STATUS_MESSAGE_VALID                        0x0001
#define MAILBOX_STATUS_MESSAGE_HAS_BEEN_READ                0x0003



// ------------------------------------------------------------------------ //
// -- Defines for Get/Set Battery Status: --------------------------------- //
// ------------------------------------------------------------------------ //

  // the following are bit flags which can be combined:

#define BATTERY_STATUS_MAINS_AVAILABLE                      0x0001
#define BATTERY_STATUS_CONNECTED                            0x0002
#define BATTERY_STATUS_CHARGING                             0x0004



// ------------------------------------------------------------------------ //
// -- Defines for Get/Set Powersave Mode: --------------------------------- //
// ------------------------------------------------------------------------ //

#define POWERSAVE_MODE_OFF                                  0x0000
#define POWERSAVE_MODE_ON                                   0x0001
#define POWERSAVE_MODE_DO_NOT_USE_BATTERY                   0x0002


// ------------------------------------------------------------------------ //
// -- Defines for Get/Set Binning Command: -------------------------------- //
// ------------------------------------------------------------------------ //

#define BINNING_STEPPING_BINARY 0
#define BINNING_STEPPING_LINEAR 1

#define BINNING_MASK                                        0x0FFF

#define BINNING_MODE_MASK                                   0xF000
#define BINNING_MODE_SUM                                    0x0000
#define BINNING_MODE_AVERAGE                                0x1000


// ------------------------------------------------------------------------ //
// -- Defines for Get/Set Sensor Format Command: -------------------------- //
// ------------------------------------------------------------------------ //

#define SENSORFORMAT_STANDARD 0
#define SENSORFORMAT_EXTENDED 1


// ------------------------------------------------------------------------ //
// -- Defines for Get/Set ADC Operation: ---------------------------------- //
// ------------------------------------------------------------------------ //

#define ADC_MODE_SINGLE 1
#define ADC_MODE_DUAL   2

// ------------------------------------------------------------------------ //
// -- Defines for Get/Set Pixelrate Operation: ---------------------------- //
// ------------------------------------------------------------------------ //

#define PIXELRATE_10MHZ 10000000
#define PIXELRATE_20MHZ 20000000
#define PIXELRATE_40MHZ 40000000
#define PIXELRATE_5MHZ   5000000


// ------------------------------------------------------------------------ //
// -- Defines for Get/Set OffsetMode: ------------------------------------- //
// ------------------------------------------------------------------------ //

#define OFFSET_MODE_AUTO 0
#define OFFSET_MODE_OFF  1


// ------------------------------------------------------------------------ //
// -- Defines for Get/Set Double Image Mode Command: ---------------------- //
// ------------------------------------------------------------------------ //

#define DOUBLE_IMAGE_MODE_OFF            0x0000
#define DOUBLE_IMAGE_MODE_ON             0x0001


// ------------------------------------------------------------------------ //
// -- Defines for Get/Set Noise Filter Mode: ------------------------------ //
// ------------------------------------------------------------------------ //

#define NOISE_FILTER_MODE_OFF              0x0000
#define NOISE_FILTER_MODE_ON               0x0001
#define NOISE_FILTER_MODE_REMOVE_HOT_DARK  0x0100


// ------------------------------------------------------------------------ //
// -- Defines for Get/Set Hot Pixel Correction: --------------------------- //
// ------------------------------------------------------------------------ //

#define HOT_PIXEL_CORRECTION_OFF           0x0000
#define HOT_PIXEL_CORRECTION_ON            0x0001
#define HOT_PIXEL_CORRECTION_TEST          0x0100  // for test purposes only!


// ------------------------------------------------------------------------ //
// -- Defines for Get/Set DSNU Adjust Mode: ------------------------------- //
// ------------------------------------------------------------------------ //

#define DSNU_ADJUST_MODE_OFF               0x0000
#define DSNU_ADJUST_MODE_AUTO              0x0001
#define DSNU_ADJUST_MODE_USER              0x0002
  //only for internal use!
#define DSNU_ADJUST_MODE_CONT              0x4000
#define DSNU_ADJUST_MODE_STOP              0x8000


// ------------------------------------------------------------------------ //
// -- Defines for Get/Set CDI Mode: --------------------------------------- //
// ------------------------------------------------------------------------ //

#define CDI_MODE_OFF                       0x0000
#define CDI_MODE_ON                        0x0001


// ------------------------------------------------------------------------ //
// -- Defines for Init DSNU Adjustment: ----------------------------------- //
// ------------------------------------------------------------------------ //

#define INIT_DSNU_ADJUSTMENT_OFF           0x0000
#define INIT_DSNU_ADJUSTMENT_ON            0x0001
#define INIT_DSNU_ADJUSTMENT_DARK_MODE     0x0002
#define INIT_DSNU_ADJUSTMENT_AUTO_MODE     0x0003


// ------------------------------------------------------------------------ //
// -- Defines for Get/Set Timebase Command: ------------------------------- //
// ------------------------------------------------------------------------ //

#define TIMEBASE_NS 0x0000
#define TIMEBASE_US 0x0001
#define TIMEBASE_MS 0x0002



// ------------------------------------------------------------------------ //
// -- Defines for Get/Set FPS Exposure Mode: ------------------------------ //
// ------------------------------------------------------------------------ //

#define FPS_EXPOSURE_MODE_OFF 0x0000
#define FPS_EXPOSURE_MODE_ON  0x0001


// ------------------------------------------------------------------------ //
// -- Defines for Get/Set Framerate: -------------------------------------- //
// ------------------------------------------------------------------------ //

#define SET_FRAMERATE_MODE_AUTO                            0x0000
#define SET_FRAMERATE_MODE_FRAMERATE_HAS_PRIORITY          0x0001
#define SET_FRAMERATE_MODE_EXPTIME_HAS_PRIORITY            0x0002
#define SET_FRAMERATE_MODE_STRICT                          0x0003
#define SET_FRAMERATE_MODE_ALLOW_ROI_REDUCTION             0x0004 // used in soft fps

#define SET_FRAMERATE_STATUS_OK                            0x0000
#define SET_FRAMERATE_STATUS_FPS_LIMITED_BY_READOUT        0x0001
#define SET_FRAMERATE_STATUS_FPS_LIMITED_BY_EXPTIME        0x0002
#define SET_FRAMERATE_STATUS_EXPTIME_CUT_TO_FRAMETIME      0x0004
#define SET_FRAMERATE_STATUS_ROI_REDUCED                   0x0008 // used in soft fps
#define SET_FRAMERATE_STATUS_NOT_YET_VALIDATED             0x8000
#define SET_FRAMERATE_STATUS_ERROR_SETTINGS_INCONSISTENT   0x8001


// ------------------------------------------------------------------------ //
// -- Defines for Get/Set Delay Exposure Timetable Command: --------------- //
// ------------------------------------------------------------------------ //

#define MAX_TIMEPAIRS   16    // max size of time table for



// ------------------------------------------------------------------------ //
// -- Defines for Get/Set Trigger Mode Command: --------------------------- //
// ------------------------------------------------------------------------ //

#define TRIGGER_MODE_AUTOTRIGGER                      0x0000
#define TRIGGER_MODE_SOFTWARETRIGGER                  0x0001
#define TRIGGER_MODE_EXTERNALTRIGGER                  0x0002
#define TRIGGER_MODE_EXTERNALEXPOSURECONTROL          0x0003
#define TRIGGER_MODE_SOURCE_HDSDI                     0x0102
#define TRIGGER_MODE_EXTERNAL_SYNCHRONIZED            0x0004
#define TRIGGER_MODE_FAST_EXTERNALEXPOSURECONTROL     0x0005
#define TRIGGER_MODE_EXTERNAL_CDS                     0x0006
#define TRIGGER_MODE_SLOW_EXTERNALEXPOSURECONTROL     0x0007
#define TRIGGER_MODE_AUTO_MULTI_CHANNEL               0x0008
#define TRIGGER_MODE_LOW_LATENCY_EXTERNALTRIGGER      0x0009

// ------------------------------------------------------------------------ //
// -- Defines for Force Trigger Command: ---------------------------------- //
// ------------------------------------------------------------------------ //

#define FORCE_TRIGGER_NOT_POSSIBLE                    0x0000
#define FORCE_TRIGGER_SUCCESSFUL                      0x0001


// ------------------------------------------------------------------------ //
// -- Defines for Get/Set Camera Sync Mode Command: ----------------------- //
// ------------------------------------------------------------------------ //

#define CAMERA_SYNC_MODE_STANDALONE               0x0000
#define CAMERA_SYNC_MODE_MASTER                   0x0001
#define CAMERA_SYNC_MODE_SLAVE                    0x0002

// ------------------------------------------------------------------------ //
// -- Defines for Get/Set Fan Control Command: ---------------------------- //
// ------------------------------------------------------------------------ //

#define FAN_CONTROL_MODE_AUTO                     0x0000
#define FAN_CONTROL_MODE_USER                     0x0001


// ------------------------------------------------------------------------ //
// -- Defines for Get/Set Power Down Mode Command: ------------------------ //
// ------------------------------------------------------------------------ //

#define POWERDOWN_MODE_AUTO   0
#define POWERDOWN_MODE_USER   1


// ------------------------------------------------------------------------ //
// -- Defines for Get/Set Storage Mode Command: --------------------------- //
// ------------------------------------------------------------------------ //

#define STORAGE_MODE_RECORDER      0
#define STORAGE_MODE_FIFO_BUFFER   1

// ------------------------------------------------------------------------ //
// -- Defines for Get/Set Memory Commpression Mode Command: --------------- //
// ------------------------------------------------------------------------ //

#define MEMORY_COMPRESSION_MODE_OFF                0x0000
#define MEMORY_COMPRESSION_MODE_GENERAL            0x0001
#define MEMORY_COMPRESSION_MODE_PER_SEGMENT        0x0002


// ------------------------------------------------------------------------ //
// -- Defines for Get/Set Recorder Submode Command: ----------------------- //
// ------------------------------------------------------------------------ //

#define RECORDER_SUBMODE_SEQUENCE     0
#define RECORDER_SUBMODE_RINGBUFFER   1


// ------------------------------------------------------------------------ //
// -- Defines for Set Record Stop Mode: ----------------------------------- //
// ------------------------------------------------------------------------ //

#define RECORD_STOP_EVENT_OFF            0x0000    // no delayed stop poss.
#define RECORD_STOP_EVENT_STOP_BY_SW     0x0001    // stop only by sw command
#define RECORD_STOP_EVENT_STOP_EXTERNAL  0x0002    // stop by signat at Acq.

// the following filter modes can be added (just ored to the mode parameter)
// when using external record stop:

#define RECORD_STOP_FILTER_OFF           0x0000    // no additional filter
#define RECORD_STOP_FILTER_1us           0x1000    // pulse length filter   1 us
#define RECORD_STOP_FILTER_10us          0x2000    // pulse length filter  10 us
#define RECORD_STOP_FILTER_100us         0x3000    // pulse length filter 100 us
#define RECORD_STOP_FILTER_1000us        0x4000    // pulse length filter   1 ms

// ------------------------------------------------------------------------ //
// -- Defines for Set Event Monitor Configuration: ------------------------ //
// ------------------------------------------------------------------------ //

#define EVENT_CONFIG_EXPTRIG_RISING            0x0001
#define EVENT_CONFIG_EXPTRIG_FALLING           0x0002
#define EVENT_CONFIG_ACQENBL_RISING            0x0004
#define EVENT_CONFIG_ACQENBL_FALLING           0x0008


// ------------------------------------------------------------------------ //
// -- Defines for Get/Set Acquire Mode Command: --------------------------- //
// ------------------------------------------------------------------------ //

#define ACQUIRE_MODE_AUTO                    0x0000   // normal auto mode
#define ACQUIRE_MODE_EXTERNAL                0x0001   // ext. as enable signal
#define ACQUIRE_MODE_EXTERNAL_FRAME_TRIGGER  0x0002   // ext. as frame trigger
#define ACQUIRE_MODE_USE_FOR_LIVEVIEW        0x0003   // use acq. for live view
#define ACQUIRE_MODE_IMAGE_SEQUENCE          0x0004   // use acq. for image sequence

// ------------------------------------------------------------------------ //
// -- Defines for Get/Set Acquire Mode Command: --------------------------- //
// ------------------------------------------------------------------------ //

#define ACQUIRE_CONTROL_OFF                  0x0000     // use external signal
#define ACQUIRE_CONTROL_FORCE_LOW            0x0001     // force aquire  low
#define ACQUIRE_CONTROL_FORCE_HIGH           0x0002     // force acquire high



// ------------------------------------------------------------------------ //
// -- Defines for Get/Set Timestamp Mode Command: ------------------------- //
// ------------------------------------------------------------------------ //

#define TIMESTAMP_MODE_OFF              0
#define TIMESTAMP_MODE_BINARY           1
#define TIMESTAMP_MODE_BINARYANDASCII   2
#define TIMESTAMP_MODE_ASCII            3


// ------------------------------------------------------------------------ //
// -- Defines for Get/Set Metadata Mode: ---------------------------------- //
// ------------------------------------------------------------------------ //

#define METADATA_MODE_OFF               0x0000
#define METADATA_MODE_ON                0x0001
#define METADATA_MODE_TEST              0x8000


// ------------------------------------------------------------------------ //
// -- Defines for Get/Set PIV Mode Command: ------------------------------- //
// ------------------------------------------------------------------------ //

#define FAST_TIMING_MODE_OFF            0x0000
#define FAST_TIMING_MODE_ON             0x0001



// ------------------------------------------------------------------------ //
// -- Defines for Get/Set Bit Alignment: ---------------------------------- //
// ------------------------------------------------------------------------ //

#define BIT_ALIGNMENT_MSB               0
#define BIT_ALIGNMENT_LSB               1
#define BIT_ALIGNMENT_MID               0x1000  // for 3x8 bit CL (Hamamatsu)


// ------------------------------------------------------------------------ //
// -- Defines for GetSensorSignalStatus: ---------------------------------- //
// ------------------------------------------------------------------------ //

#define SIGNAL_STATE_BUSY               0x00000001
#define SIGNAL_STATE_IDLE               0x00000002
#define SIGNAL_STATE_EXP                0x00000004
#define SIGNAL_STATE_READ               0x00000008
#define SIGNAL_STATE_FIFO_FULL          0x00000010


// ------------------------------------------------------------------------ //
// -- Defines for Play Images from Segment Modes: ------------------------- //
// ------------------------------------------------------------------------ //

#define PLAY_IMAGES_MODE_OFF                                  0x0000
#define PLAY_IMAGES_MODE_FAST_FORWARD                         0x0001
#define PLAY_IMAGES_MODE_FAST_REWIND                          0x0002
#define PLAY_IMAGES_MODE_SLOW_FORWARD                         0x0003
#define PLAY_IMAGES_MODE_SLOW_REWIND                          0x0004
#define PLAY_IMAGES_MODE_REPLAY_AT_END                        0x0100
#define PLAY_IMAGES_MODE_EXT_CONTROL                          0x4000

#define PLAY_IMAGES_MODE_IS_FORWARD                           0x0001

#define PLAY_POSITION_STATUS_NO_PLAY_ACTIVE                   0x0000
#define PLAY_POSITION_STATUS_VALID                            0x0001


// ------------------------------------------------------------------------ //
// -- Defines for Color Chips    ------------------------------------------ //
// ------------------------------------------------------------------------ //

#define COLOR_RED     0x01
#define COLOR_GREENA  0x02
#define COLOR_GREENBA 0x03
#define COLOR_BLUE    0x04

#define BAYER_BGGR      ((COLOR_BLUE    << 0) | (COLOR_GREENBA << 4) | (COLOR_GREENA  << 8)  | (COLOR_RED     << 12))
#define BAYER_RGGB      ((COLOR_RED     << 0) | (COLOR_GREENA  << 4) | (COLOR_GREENBA << 8)  | (COLOR_BLUE    << 12))
#define BAYER_GBRG      ((COLOR_GREENBA << 0) | (COLOR_BLUE    << 4) | (COLOR_RED     << 8)  | (COLOR_GREENA  << 12))
#define BAYER_GRBG      ((COLOR_GREENA  << 0) | (COLOR_RED     << 4) | (COLOR_BLUE    << 8)  | (COLOR_GREENBA << 12))


#define COLOR_CYAN    0x05
#define COLOR_MAGENTA 0x06
#define COLOR_YELLOWA 0x07

#define PATTERN_BAYER 0x01

// ------------------------------------------------------------------------ //
// -- Defines for Modulate mode  ------------------------------------------ //
// ------------------------------------------------------------------------ //

#define MODULATECAPS_MODULATE                 0x00000001
#define MODULATECAPS_MODULATE_EXT_TRIG        0x00000002
#define MODULATECAPS_MODULATE_EXT_EXP         0x00000004
#define MODULATECAPS_MODULATE_ACQ_EXT_FRAME   0x00000008


// ------------------------------------------------------------------------ //
// -- Defines for Get/Set Sensor Readout Mode: ---------------------------- //
// ------------------------------------------------------------------------ //
#define SENSOR_READOUTMODE_TOP_BOTTOM                      0x01
#define SENSOR_READOUTMODE_TOP_CENTER_BOTTOM_CENTER        0x02

#define SENSOR_READOUTCTRL_STANDARD                        0x0001
#define SENSOR_READOUTCTRL_REVERSE                         0x0002
#define SENSOR_READOUTCTRL_EXTERNALCONTROL                 0x0004
#define SENSOR_READOUTCTRL_ALTERNATING                     0x0008


// ------------------------------------------------------------------------ //
// -- Defines for Get/Set Interface Output Format: ------------------------ //
// ------------------------------------------------------------------------ //

//can use wInterface definitions below also
#define INTERFACE_HDSDI                                    0x0001
#define INTERFACE_CL_SCCMOS                                0x0002
#define INTERFACE_USB_PIXELFLY                             0x0003

//wInterface
#define SET_INTERFACE_HDSDI                                0x0001 // dimax
#define SET_INTERFACE_CAMERALINK                           0x0002 // edge sccmos cameralink,CLHS,USB3
#define SET_INTERFACE_USB                                  0x0003 // usb pixelfly
#define SET_INTERFACE_DVI                                  0x0004 // dimax
#define SET_INTERFACE_CLHS                                 0x0005 // 


#define SCCMOS_FORMAT_TOP_BOTTOM                                  0x0000  //Mode E
#define SCCMOS_FORMAT_TOP_CENTER_BOTTOM_CENTER                    0x0100  //Mode A
#define SCCMOS_FORMAT_CENTER_TOP_CENTER_BOTTOM                    0x0200  //Mode B
#define SCCMOS_FORMAT_CENTER_TOP_BOTTOM_CENTER                    0x0300  //Mode C
#define SCCMOS_FORMAT_TOP_CENTER_CENTER_BOTTOM                    0x0400  //Mode D
#define SCCMOS_FORMAT_TOP_CENTER_BOTTOM_CENTER_INVERSE            0x0500  //Mode F


//#define USB_FORMAT_14BIT                                          0x0000  //USB Pixelfly
//#define USB_FORMAT_12BIT                                          0x0001  //USB Pixelfly

//wFormat
#define HDSDI_FORMAT_OUTPUT_OFF                                   0x0000
#define HDSDI_FORMAT_1080P25_SINGLE_LINK_RGB                      0x0001
#define HDSDI_FORMAT_1080P25_SINGLE_LINK_RAW10BIT_2_IMAGES        0x0002
#define HDSDI_FORMAT_1080P50_DUAL_LINK_RGB                        0x0003
#define HDSDI_FORMAT_1080P50_DUAL_LINK_RAW10BIT_2_IMAGES          0x0004
#define HDSDI_FORMAT_2048x1536_SINGLE_LINK_RAW12BIT               0x0005
#define HDSDI_FORMAT_2048x1536_DUAL_LINK_RAW12BIT                 0x0006
#define HDSDI_FORMAT_720P50_SINGLE_LINK_RGB                       0x0007

#define HDSDI_FORMAT_720P50_SINGLE_LINK_RAW10BIT_2_IMAGES         0x0008
#define HDSDI_FORMAT_1080P25_SINGLE_LINK_RAW10BIT_1_IMAGE         0x0009
#define HDSDI_FORMAT_720P50_SINGLE_LINK_RAW10BIT_1_IMAGE          0x000A
#define HDSDI_FORMAT_1080P30_SINGLE_LINK_RGB                      0x000B
#define HDSDI_FORMAT_1080P2997_SINGLE_LINK_RGB                    0x000C
#define HDSDI_FORMAT_1080P24_SINGLE_LINK_RGB                      0x000D
#define HDSDI_FORMAT_1080P2398_SINGLE_LINK_RGB                    0x000E
#define HDSDI_FORMAT_1080P60_SINGLE_LINK_RAW10BIT_2_IMAGES        0x000F

#define HDSDI_FORMAT_1080P5994_SINGLE_LINK_RAW10BIT_2_IMAGES      0x0010
#define HDSDI_FORMAT_1080P48_SINGLE_LINK_RAW10BIT_2_IMAGES        0x0011
#define HDSDI_FORMAT_1080P4795_SINGLE_LINK_RAW10BIT_2_IMAGES      0x0012
#define HDSDI_FORMAT_1080P48_DUAL_LINK_RGB                        0x0013
#define HDSDI_FORMAT_1080P4795_DUAL_LINK_RGB                      0x0014
#define HDSDI_FORMAT_1080P96_DUAL_LINK_RAW10BIT_2_IMAGES          0x0015
#define HDSDI_FORMAT_1080P9550_DUAL_LINK_RAW10BIT_2_IMAGES        0x0016
#define HDSDI_FORMAT_720P24_SINGLE_LINK_RGB                       0x0017

#define HDSDI_FORMAT_720P2398_SINGLE_LINK_RGB                     0x0018
#define HDSDI_FORMAT_720P48_SINGLE_LINK_RAW10BIT_2_IMAGES         0x0019
#define HDSDI_FORMAT_720P4795_SINGLE_LINK_RAW10BIT_2_IMAGES       0x001A
#define HDSDI_FORMAT_1080P30_SINGLE_LINK_RAW10BIT_1_IMAGE         0x001B
#define HDSDI_FORMAT_1080P2997_SINGLE_LINK_RAW10BIT_1_IMAGE       0x001C
#define HDSDI_FORMAT_1080P24_SINGLE_LINK_RAW10BIT_1_IMAGE         0x001D
#define HDSDI_FORMAT_1080P2398_SINGLE_LINK_RAW10BIT_1_IMAGE       0x001E
#define HDSDI_FORMAT_1080P60_DUAL_LINK_RGB                        0x001F

#define HDSDI_FORMAT_1080P120_DUAL_LINK_RAW10BIT_2_IMAGES         0x0020
#define HDSDI_FORMAT_720P2398_SINGLE_LINK_RAW10BIT_1_IMAGE        0x0021
#define HDSDI_FORMAT_720P24_SINGLE_LINK_RAW10BIT_1_IMAGE          0x0022
#define HDSDI_FORMAT_720P25_SINGLE_LINK_RAW10BIT_1_IMAGE          0x0023
#define HDSDI_FORMAT_720P25_SINGLE_LINK_RAW10BIT_2_IMAGES         0x0024
#define HDSDI_FORMAT_720P2997_SINGLE_LINK_RAW10BIT_1_IMAGE        0x0025
#define HDSDI_FORMAT_720P2997_SINGLE_LINK_RAW10BIT_2_IMAGES       0x0026
#define HDSDI_FORMAT_720P5994_SINGLE_LINK_RAW10BIT_1_IMAGE        0x0027
#define HDSDI_FORMAT_720P5994_SINGLE_LINK_RAW10BIT_2_IMAGES       0x0028
#define HDSDI_FORMAT_1080P2498_SINGLE_LINK_RGB                    0x0029

#define HDSDI_FORMAT_OPTIONS_TIMECODE_OUT                         0x0001
#define HDSDI_FORMAT_OPTIONS_RECORD_ENABLE_FLAG                   0x0002
#define HDSDI_FORMAT_OPTIONS_VIEWER_3G_OUT                        0x0040
#define HDSDI_FORMAT_OPTIONS_LINKAD_3G_OUT                        0x0080


// ------------------------------------------------------------------------ //
// -- Defines for Get Interface Status for HD/SDI: ------------------------ //
// ------------------------------------------------------------------------ //

#define HDSDI_STATUS_OUTPUT_ACTIVE                             0x00000001
#define HDSDI_STATUS_GENLOCK_AVAIL                             0x00000002
#define HDSDI_STATUS_GENLOCK_LOCKED                            0x00000004
#define HDSDI_STATUS_RECORD_IDLE                               0x00000008
#define HDSDI_STATUS_RECORD_ON                                 0x00000010
#define HDSDI_STATUS_PLAY_ACTIVE                               0x00000020

#define HDSDI_ERROR_INIT_FAILED                                0x00000001
#define HDSDI_ERROR_NO_RESPONSE                                0x00000002
#define HDSDI_ERROR_GENLOCK_PLL_UNLOCKED                       0x00000004
#define HDSDI_ERROR_GENLOCK_WRONG_FORMAT                       0x00000008


// ------------------------------------------------------------------------ //
// -- Defines for Get White Balance Status: ------------------------------- //
// ------------------------------------------------------------------------ //

#define WHITE_BALANCE_STATUS_DEFAULT                              0x0000
#define WHITE_BALANCE_STATUS_IN_PROGRESS                          0x0100
#define WHITE_BALANCE_STATUS_SUCCESS                              0x0001
#define WHITE_BALANCE_STATUS_TIMEOUT                              0x8001
#define WHITE_BALANCE_STATUS_FAILED                               0x8002


// ------------------------------------------------------------------------ //
// -- Defines for Get / Set Color Settings: ------------------------------- //
// ------------------------------------------------------------------------ //

#define COLOR_PROC_OPTIONS_COLOR_REFINE                           0x0001
//#define COLOR_PROC_OPTIONS_USE_REC709                             0x0002
//#define COLOR_PROC_OPTIONS_USE_LOG90                              0x0004

#define COLOR_SETTINGS_LUT_NOT_USED                               0x0000
#define COLOR_SETTINGS_LUT_REC709                                 0x1001
#define COLOR_SETTINGS_LUT_LOG90                                  0x1002


// ------------------------------------------------------------------------ //
// -- Defines for Get / Set Image Transfer Mode: -------------------------- //
// ------------------------------------------------------------------------ //

#define IMAGE_TRANSFER_MODE_STANDARD                              0x0000
#define IMAGE_TRANSFER_MODE_SCALED_XY_8BIT                        0x0001
#define IMAGE_TRANSFER_MODE_CUTOUT_XY_8BIT                        0x0002
#define IMAGE_TRANSFER_MODE_FULL_RGB_24BIT                        0x0003
#define IMAGE_TRANSFER_MODE_BIN_SCALED_8BIT_BW                    0x0004
#define IMAGE_TRANSFER_MODE_BIN_SCALED_8BIT_COLOR                 0x0005
#define IMAGE_TRANSFER_MODE_TEST_ONLY                             0x8000


// ------------------------------------------------------------------------ //
// -- Defines for Get Image Timing: --------------------------------------- //
// ------------------------------------------------------------------------ //

#define IMAGE_TIMING_NOT_APPLICABLE                           0xFFFFFFFF



// ------------------------------------------------------------------------ //
// -- Defines for Lookup Table: ------------------------------------------- //
// ------------------------------------------------------------------------ //

#define LOOKUPTABLE_FORMAT_8BIT   0x0001
#define LOOKUPTABLE_FORMAT_12BIT  0x0002
#define LOOKUPTABLE_FORMAT_16BIT  0x0004
#define LOOKUPTABLE_FORMAT_24BIT  0x0008
#define LOOKUPTABLE_FORMAT_32BIT  0x0010
#define LOOKUPTABLE_FORMAT_AUTO   0x8000


// ------------------------------------------------------------------------ //
// -- Defines for Cooling Setpoints---------------------------------------- //
// ------------------------------------------------------------------------ //

#define COOLING_SETPOINTS_BLOCKSIZE 10


// ------------------------------------------------------------------------ //
// -- Defines for Linetiming   -------------------------------------------- //
// ------------------------------------------------------------------------ //

// wParameter:
#define CMOS_LINETIMING_PARAM_OFF  0x0000
#define CMOS_LINETIMING_PARAM_ON   0x0001

// dwLineCaps:
#define CMOS_LINE_TIMING_CAPS_PIXCLK_SLOW_ONLY  0x0001  // Camera supports line timing with slowest pixel clk frequency
#define CMOS_LINE_TIMING_CAPS_PIXCLK_ALL        0x0002  // Camera supports line timing with all pixel clk frequencies




// ------------------------------------------------------------------------ //
// -- Defines for HWIO Signal Timing: ------------------------------------- //
// ------------------------------------------------------------------------ //

// Hardware IO Signals definition
// SIGNAL options definitions (up to 8 different defines)
#define SIGNAL_DEF_ENABLE         0x0001 // Signal can be enabled/disabled
#define SIGNAL_DEF_OUTPUT         0x0002 // Signal is a status signal (output)
#define SIGNAL_DEF_MASK           0x00FF // Signal options mask

// SIGNAL Type definitions (up to 16 different types)
#define SIGNAL_TYPE_TTL           0x0001 // Signal can be switched to TTL level
// (0V to 0.8V, 2V to VCC, VCC is 4.75V to 5.25V)
#define SIGNAL_TYPE_HL_SIG        0x0002 // Signal can be switched to high level signal
// (0V to 5V, 10V to VCC, VCC is 56V)
#define SIGNAL_TYPE_CONTACT       0x0004 // Signal can be switched to contact level
#define SIGNAL_TYPE_RS485         0x0008 // Signal can be switched to RS485 level
#define SIGNAL_TYPE_TTL_A_GND_B   0x0080 // Two pin diff. output, A = TTL, B = GND
#define SIGNAL_TYPE_MASK          0xFFFF // Signal type mask

// SIGNAL Polarity definitions (up to 16 different types)
#define SIGNAL_POL_HIGH           0x0001 // Signal can be switched to sense low level
#define SIGNAL_POL_LOW            0x0002 // Signal can be switched to sense high level
#define SIGNAL_POL_RISE           0x0004 // Signal can be switched to sense rising edge
#define SIGNAL_POL_FALL           0x0008 // Signal can be switched to sense falling edge
#define SIGNAL_POL_MASK           0xFFFF // Signal polarity mask

// SIGNAL Filter settings definitions (up to 16 different filter)
#define SIGNAL_FILTER_OFF         0x0001 // Filter can be switched off (t > ~65ns)
#define SIGNAL_FILTER_MED         0x0002 // Filter can be switched to medium (t > 1us)
#define SIGNAL_FILTER_HIGH        0x0004 // Signal can be switched to high (t > 100ms)
#define SIGNAL_FILTER_MASK        0xFFFF // Signal polarity mask

//--HW IO TYPE (HW-IO descriptor flags, see HW-IO descriptor
#define HW_IO_SIGNAL_TIMING_0_AVAILABLE   0x10
#define HW_IO_SIGNAL_TIMING_1_AVAILABLE   0x20
#define HW_IO_SIGNAL_TIMING_2_AVAILABLE   0x40
#define HW_IO_SIGNAL_TIMING_3_AVAILABLE   0x80

// Signal type for parameter selection
#define HW_IO_SIGNAL_TIMING_TYPE_TRIGGER        0x00000001
#define HW_IO_SIGNAL_TIMING_TYPE_ACQUIRE        0x00000002
#define HW_IO_SIGNAL_TIMING_TYPE_BUSY           0x00000003
#define HW_IO_SIGNAL_TIMING_TYPE_EXPOSURE       0x00000004
#define HW_IO_SIGNAL_TIMING_TYPE_READ           0x00000005
#define HW_IO_SIGNAL_TIMING_TYPE_SYNC           0x00000006
#define HW_IO_SIGNAL_TIMING_TYPE_EXPOSURE_RS    0x00000007

// Parameter definitions (defines timing behaviour)
//--Rolling Shutter Exposure Configurations (HW_IO_SIGNAL_TIMING_TYPE_EXPOSURE_RS)
#define HW_IO_SIGNAL_TIMING_EXPOSURE_RS_FIRSTLINE   0x00000001
#define HW_IO_SIGNAL_TIMING_EXPOSURE_RS_GLOBAL      0x00000002
#define HW_IO_SIGNAL_TIMING_EXPOSURE_RS_LASTLINE    0x00000003
#define HW_IO_SIGNAL_TIMING_EXPOSURE_RS_ALLLINES    0x00000004
// ATTENTION: Set timing max value at least to the biggest signal timing
// value in the parameters list
#define HW_IO_SIGNAL_TIMING_MAX_VALUE               0x00000010

// ------------------------------------------------------------------------ //
// -- Defines for HW LED Signal ------------------------------------------- //
// ------------------------------------------------------------------------ //

#define HW_LED_SIGNAL_OFF                               0x00000000
#define HW_LED_SIGNAL_ON                                0xFFFFFFFF

// ------------------------------------------------------------------------ //
// -- Defines for pco.flim Commands --------------------------------------- //
// ------------------------------------------------------------------------ //

// command: SET_FLIM_MODULATION_PARAMS

#define FLIM_MODULATION_SOURCE_INTERN        0x0000 // camera creates the modulation (master)
#define FLIM_MODULATION_SOURCE_EXTERN        0x0001 // camera tries to synchronize to an external source

#define FLIM_MODULATION_OUTPUT_WAVEFORM_NONE 0x0000 // modulation output disabled
#define FLIM_MODULATION_OUTPUT_WAVEFORM_SINE 0x0001 // sinusoidal
#define FLIM_MODULATION_OUTPUT_WAVEFORM_RECT 0x0002 // rectangular

// command: SET_FLIM_PHASE_SEQUENCE_PARAMS

#define FLIM_PHASE_MANUAL_SHIFTING           0x0000
#define FLIM_PHASE_NUMBER_2                  0x0001
#define FLIM_PHASE_NUMBER_4                  0x0002
#define FLIM_PHASE_NUMBER_8                  0x0003
#define FLIM_PHASE_NUMBER_16                 0x0004

#define FLIM_PHASE_SYMMETRY_SINGULAR         0x0000
#define FLIM_PHASE_SYMMETRY_TWICE            0x0001

#define FLIM_PHASE_ORDER_ASCENDING           0x0000
#define FLIM_PHASE_ORDER_OPPOSITE            0x0001

#define FLIM_TAP_SELECT_BOTH                 0x0000
#define FLIM_TAP_SELECT_0                    0x0001
#define FLIM_TAP_SELECT_180                  0x0002

// command: SET_FLIM_IMAGE_PROCESSING_FLOW

#define FLIM_ASYMMETRY_CORRECTION_OFF        0x0000
#define FLIM_ASYMMETRY_CORRECTION_AVERAGE    0x0001

#define FLIM_OUTPUT_MODE_MULT_X2_FLAG        0x0001 // pixel raw values are multiplied by two


// ------------------------------------------------------------------------ //
// -- Defines for intensified camera description and commands ------------- //
// ------------------------------------------------------------------------ //

// descriptor:
#define INTENSIFIED_DESC_FLAG_HAS_GATING_MODE      0x00000001
#define INTENSIFIED_DESC_FLAG_HAS_GATING_MODE2     0x00000002

// command: SET_GATING_MODE

#define INTENSIFIED_GATING_MODE_OFF                0
#define INTENSIFIED_GATING_MODE_1                  1
#define INTENSIFIED_GATING_MODE_2                  2

#endif

// ------------------------------< end of file >--------------------------- //


