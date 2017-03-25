//
// kernel.cpp
//
#include "kernel.h"
#include <circle/timer.h>
#include <circle/string.h>

static const char FromKernel[] = "kernel";

CKernel::CKernel (void)
<<<<<<< HEAD
:   m_Screen (m_Options.GetWidth (), m_Options.GetHeight ()),
    m_Timer (&m_Interrupt),
    m_Logger (m_Options.GetLogLevel (), &m_Timer),
    // TODO: add more member initializers here
    embedded_rom(),
    nes(embedded_rom)
=======
:	m_Screen (m_Options.GetWidth (), m_Options.GetHeight ()),
	m_Timer (&m_Interrupt),
	m_Logger (m_Options.GetLogLevel (), &m_Timer),
	// TODO: add more member initializers here
        rom(),
	nes(rom)
>>>>>>> 72ec4108796e94e0a2c277c4d45982e3a393de20
{
}

CKernel::~CKernel (void)
{
}

boolean CKernel::Initialize (void)
{
	boolean bOK = TRUE;

	if (bOK)
	{
		bOK = m_Screen.Initialize ();
	}

	if (bOK)
	{
		bOK = m_Serial.Initialize (115200);
	}

	if (bOK)
	{
		CDevice *pTarget = m_DeviceNameService.GetDevice (m_Options.GetLogDevice (), FALSE);
		if (pTarget == 0)
		{
			pTarget = &m_Screen;
		}

		bOK = m_Logger.Initialize (pTarget);
	}

	if (bOK)
	{
		bOK = m_Interrupt.Initialize ();
	}

	if (bOK)
	{
		bOK = m_Timer.Initialize ();
	}

	// TODO: call Initialize () of added members here (if required)

	return bOK;
}

TShutdownMode CKernel::Run (void)
{
    CString str;
    str.Format("%x %x %x", nes.ram[0x6001], nes.ram[0x6002], nes.ram[0x6003]);
    m_Logger.Write(FromKernel, TLogSeverity::LogNotice, str);
    
    while(true){
<<<<<<< HEAD
        nes.Step();
        if (nes.cpu.nmiOccurred){
            for (unsigned int i = 0; i < 256; ++i) {
                for (unsigned int j = 0; j < 240; ++j) {
                    m_Screen.SetPixel(i * 4, j * 4, COLOR32(nes.ppu.front[i][j].red, nes.ppu.front[i][j].green, nes.ppu.front[i][j].blue, 0));
                            m_Screen.SetPixel(i * 4 + 1, j * 4, COLOR32(nes.ppu.front[i][j].red, nes.ppu.front[i][j].green, nes.ppu.front[i][j].blue, 0));
                            m_Screen.SetPixel(i * 4 + 2, j * 4, COLOR32(nes.ppu.front[i][j].red, nes.ppu.front[i][j].green, nes.ppu.front[i][j].blue, 0));
                            m_Screen.SetPixel(i * 4 + 3, j * 4, COLOR32(nes.ppu.front[i][j].red, nes.ppu.front[i][j].green, nes.ppu.front[i][j].blue, 0));
                            m_Screen.SetPixel(i * 4, j * 4 + 1, COLOR32(nes.ppu.front[i][j].red, nes.ppu.front[i][j].green, nes.ppu.front[i][j].blue, 0));
                            m_Screen.SetPixel(i * 4 + 1, j * 4 + 1, COLOR32(nes.ppu.front[i][j].red, nes.ppu.front[i][j].green, nes.ppu.front[i][j].blue, 0));
                            m_Screen.SetPixel(i * 4 + 2, j * 4 + 1, COLOR32(nes.ppu.front[i][j].red, nes.ppu.front[i][j].green, nes.ppu.front[i][j].blue, 0));
                            m_Screen.SetPixel(i * 4 + 3, j * 4 + 1, COLOR32(nes.ppu.front[i][j].red, nes.ppu.front[i][j].green, nes.ppu.front[i][j].blue, 0));
                            m_Screen.SetPixel(i * 4, j * 4 + 2, COLOR32(nes.ppu.front[i][j].red, nes.ppu.front[i][j].green, nes.ppu.front[i][j].blue, 0));
                            m_Screen.SetPixel(i * 4 + 1, j * 4 + 2, COLOR32(nes.ppu.front[i][j].red, nes.ppu.front[i][j].green, nes.ppu.front[i][j].blue, 0));
                            m_Screen.SetPixel(i * 4 + 2, j * 4 + 2, COLOR32(nes.ppu.front[i][j].red, nes.ppu.front[i][j].green, nes.ppu.front[i][j].blue, 0));
                            m_Screen.SetPixel(i * 4 + 3, j * 4 + 2, COLOR32(nes.ppu.front[i][j].red, nes.ppu.front[i][j].green, nes.ppu.front[i][j].blue, 0));
                            m_Screen.SetPixel(i * 4, j * 4 + 3, COLOR32(nes.ppu.front[i][j].red, nes.ppu.front[i][j].green, nes.ppu.front[i][j].blue, 0));
                            m_Screen.SetPixel(i * 4 + 1, j * 4 + 3, COLOR32(nes.ppu.front[i][j].red, nes.ppu.front[i][j].green, nes.ppu.front[i][j].blue, 0));
                            m_Screen.SetPixel(i * 4 + 2, j * 4 + 3, COLOR32(nes.ppu.front[i][j].red, nes.ppu.front[i][j].green, nes.ppu.front[i][j].blue, 0));
                            m_Screen.SetPixel(i * 4 + 3, j * 4 + 3, COLOR32(nes.ppu.front[i][j].red, nes.ppu.front[i][j].green, nes.ppu.front[i][j].blue, 0));
                }
            }
        }
=======
	nes.Step();
        if(nes.ram[0x6000] != 0x80) {
        }
        
	if (nes.cpu.nmiOccurred){
            m_Logger.Write(FromKernel, TLogSeverity::LogNotice, "NMI");
	    for (unsigned int i = 0; i < 256; ++i) {
		for (unsigned int j = 0; j < 240; ++j) {
		    m_Screen.SetPixel(i, j, COLOR32(nes.ppu.front[i][j].red, nes.ppu.front[i][j].green, nes.ppu.front[i][j].blue, 0));
		}
	    }
	}
>>>>>>> 72ec4108796e94e0a2c277c4d45982e3a393de20
    }
    return ShutdownHalt;
}
