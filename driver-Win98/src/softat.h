#include <vtoolsc.h>		// primary include for VtoolsD

// Device Parameters

#define SOFTAT_Major		1
#define SOFTAT_Minor		0
#define SOFTAT_DeviceID		UNDEFINED_DEVICE_ID
#define SOFTAT_Init_Order	UNDEFINED_INIT_ORDER


#define LOG_FILE_PATH "C:\\softat.log"
#define CONFIG_FILE_PATH "C:\\softat.cfg"

#define SYSTEM_VM_ID 1

// Forward declarations for functions
BOOL __stdcall shutdownInt_Handler(VMHANDLE hVM, IRQHANDLE hIRQ);

VOID __stdcall sendDebugLog(VMHANDLE hVM, PVOID Refdata, PCLIENT_STRUCT pRegs);

VOID __stdcall doShutdown(VMHANDLE hVM, PVOID Refdata, PCLIENT_STRUCT pRegs);

VOID __stdcall fileLoggerInit();

VOID __stdcall fileLoggerClose();

VOID __stdcall fileLoggerWriteLine(PCHAR logStr);

VOID __stdcall loadConfigFile();
