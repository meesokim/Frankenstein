//
// kernel.h
//
#ifndef _kernel_h
#define _kernel_h

#include <circle/memory.h>
#include <circle/actled.h>
#include <circle/koptions.h>
#include <circle/devicenameservice.h>
#include <circle/screen.h>
#include <circle/serial.h>
#include <circle/exceptionhandler.h>
#include <circle/interrupt.h>
#include <circle/timer.h>
#include <circle/logger.h>
#include <circle/usb/dwhcidevice.h>
#include <circle/usb/usbgamepad.h>
#include <circle/types.h>
#include <circle/util.h>

#include "../emulator/include/nes.h"
#include "../emulator/include/rom_static.h"

using namespace Frankenstein;

enum TShutdownMode
{
    ShutdownNone,
    ShutdownHalt,
    ShutdownReboot
};

class CKernel
{
public:
    CKernel (void);
    ~CKernel (void);

    boolean Initialize (void);

    TShutdownMode Run (void);
    
private:
    static void GamePadStatusHandler (unsigned nDeviceIndex, const TGamePadState *pState);
    static CLogger* s_logger;
    static Nes* s_nes;
    static TGamePadState s_input_player1;
    static TGamePadState s_input_player2;
    static CInterruptSystem* s_interrupt;

private:
    // do not change this order
    CMemorySystem	m_Memory;
    CActLED		m_ActLED;
    CKernelOptions	m_Options;
    CDeviceNameService  m_DeviceNameService;
    CScreenDevice       m_Screen;
    CSerialDevice       m_Serial;
    CExceptionHandler   m_ExceptionHandler;
    CInterruptSystem    m_Interrupt;
    CTimer		m_Timer;
    CLogger		m_Logger;
    CDWHCIDevice	m_DWHCI;

    // TODO: add more members here
    Rom embedded_rom;
    Nes nes;
    
};

#endif
