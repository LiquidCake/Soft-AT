#define   DEVICE_MAIN
#include  "SOFTAT.h"
#undef    DEVICE_MAIN

Declare_Virtual_Device(SOFTAT)		// Declare Device Data Block

// ***********************************************************************
//
// Static data

VPICD_HWInt_THUNK shutdownInt_Thunk;		// Thunk for interrupt handler
Event_THUNK eventThunk;		// Thunk for hour event handler
EVENTHANDLE hEvent;			// Handle of timer event

IRQHANDLE shutdownIRQHandle;			// Handle for virtual IRQ


// ***********************************************************************
//
// Handlers for Control Messages
//
// This VxD handles DEVICE_INIT (to hook the interrupt and configure things)
// and SYSTEM_EXIT

DefineControlHandler(DEVICE_INIT, OnDeviceInit);
DefineControlHandler(SYS_VM_INIT, OnSysVmInit);
DefineControlHandler(SYSTEM_EXIT, OnSystemExit);


#ifdef WIN40
DefineControlHandler(SYS_DYNAMIC_DEVICE_INIT, OnSysDynamicDeviceInit);
DefineControlHandler(SYS_DYNAMIC_DEVICE_EXIT, OnSysDynamicDeviceExit);

BOOL OnSysDynamicDeviceInit() {
	return OnDeviceInit(0, 0);
}

BOOL OnSysDynamicDeviceExit() {
	OnSystemExit(0);

	return TRUE;
}
#endif


// ******** VARIABLES
BOOL isShuttingDown = FALSE;

//loaded from config
int parsedIOPort;
int parsedIRQNum;
int parsedDelaySec;


//file config
HANDLE fhConfig;

//file logging
HANDLE fh;
DWORD filepos = 0;


// Function
//	OnSysVmInit - Handler for SYS_VM_INIT control message
//
// Input
//	hSysVM		Handle of system virtual machine
//
// Remarks
//	This routine is responsible for hooking the RTC
//	interrupt and configuring it.
//
// Returns
//	Returns TRUE if successfully initialized
//
BOOL OnSysVmInit(VMHANDLE hVM)
{
	// Hooking a hardware interrupt requires calling the Virtual Programmable
	// Interrupt Controlloer Device (VPICD), to inform that this VxD will
	// be responsible for the IRQ. VPICD provides five different notifications
	// of events related to the IRQ, but this VxD uses only one of them, namely,
	// that of the actual hardware interrupt event. The VPICD_Virtualize_IRQ
	// call takes one parameter which points to a structure containing the
	// address of this VxD's hardware interrupt handler, along with additional
	// information related to the IRQ.

	// struct to pass to VPICD_Virtualize_IRQ
	struct VPICD_IRQ_Descriptor IRQdesc;

	//log startup
	fileLoggerWriteLine("Event: SYS_VM_INIT\r\n");
	
	//load config
	loadConfigFile();

	// Set up the structure to pass to VPICD_Virtualize_IRQ
	IRQdesc.VID_IRQ_Number = parsedIRQNum;	// IRQ to virtualize
	IRQdesc.VID_Options = 0;		// reserved

	// To set the address of the our handler in the structure that will be
	// passed to VPICD_Virtualize_IRQ, we pass the address of the handler's
	// thunk to VPICD_Thunk_HWInt, which initializes the thunk and returns
	// its address.

	IRQdesc.VID_Hw_Int_Proc = (DWORD) VPICD_Thunk_HWInt(shutdownInt_Handler, &shutdownInt_Thunk);

	// The other callbacks are not used.

	IRQdesc.VID_EOI_Proc = 0;
	IRQdesc.VID_Virt_Int_Proc = 0;
	IRQdesc.VID_Mask_Change_Proc = 0;
	IRQdesc.VID_IRET_Proc = 0;

	IRQdesc.VID_IRET_Time_Out = 500;

	// Now pass the structure to VPICD. VPICD returns the IRQ handle.

	shutdownIRQHandle = VPICD_Virtualize_IRQ(&IRQdesc);

	if (shutdownIRQHandle == 0) {
		// failed to virtualize IRQ
		fileLoggerWriteLine("Event: failed to virtualize IRQ\r\n");

		return FALSE;
	}

	// Make sure the IRQ is unmasked on the PIC. Otherwise, the interrupt will never occur.
	VPICD_Physically_Unmask(shutdownIRQHandle);

	return TRUE;				// initialized successfully
}


// Function
// 	OnSystemExit - Handler for SYSTEM_EXIT control message
//
// Input
//	hSysVM		Handle of system virtual machine
//
VOID OnSystemExit(VMHANDLE hVM)
{
	//write delay value to IO port
	Voutportb(parsedIOPort, parsedDelaySec);

	//log shutdown
	fileLoggerWriteLine("Event: SYSTEM_EXIT. Write to IO port\r\n");

	//cleanup
	Cancel_Global_Event(hEvent);

	VPICD_Physically_Mask(shutdownIRQHandle);
	VPICD_Force_Default_Behavior(shutdownIRQHandle);
}


// Function
// 	OnDeviceInit - Handler for DEVICE_INIT control message
//
// Input
//	hVM		    Handle of system virtual machine
//	CommandTail	Pointer to Windows command line
BOOL OnDeviceInit(VMHANDLE hVM, PCHAR CommandTail)
{
	return TRUE;
}


// Function
// 	ControlDispatcher - dispatch control messages
//
// Input
//	dwControlMessage	specifies control message sent
//	registers
//
// Remarks
//	This routine is responsible for dispatching to control message handlers
//
BOOL __cdecl ControlDispatcher(
	DWORD dwControlMessage,
	DWORD EBX,
	DWORD EDX,
	DWORD ESI,
	DWORD EDI,
	DWORD ECX)
{
	START_CONTROL_DISPATCH

		ON_DEVICE_INIT(OnDeviceInit);
		ON_SYS_VM_INIT(OnSysVmInit);
		ON_SYSTEM_EXIT(OnSystemExit);

#ifdef WIN40
		ON_SYS_DYNAMIC_DEVICE_INIT(OnSysDynamicDeviceInit);
		ON_SYS_DYNAMIC_DEVICE_EXIT(OnSysDynamicDeviceExit);
#endif
	END_CONTROL_DISPATCH

	return TRUE;
}


// Function
// 	shutdownInt_Handler - handler for IRQdesc.VID_Hw_Int_Proc callback
//
// Input
//	hVM		handle of current virtual machine
//	hIRQ	handle of this virtualized IRQ
//
// Remarks
//	VPICD invokes this routine (via the thunk) when interrupt (IRQ X) occurrs.
//
BOOL __stdcall shutdownInt_Handler(VMHANDLE hVM, IRQHANDLE hIRQ)
{
	if (!isShuttingDown && hVM->CB_VMID == SYSTEM_VM_ID) {
		//shutdown
		isShuttingDown = TRUE;
		
		fileLoggerWriteLine("Got IRQ. Shutting down\r\n");
		Call_Global_Event(sendDebugLog, (PVOID) "Got IRQ. Shutting down", &eventThunk);
		Call_Global_Event(doShutdown, NULL, &eventThunk);

	}

	VPICD_Phys_EOI(hIRQ);		// tell VPICD to clear the interrupt

	return TRUE;	    		// thunk will clear carry
}


VOID __stdcall doShutdown(VMHANDLE hVM, PVOID Refdata, PCLIENT_STRUCT pRegs)
{
	ExecuteApplication("rundll32.exe", "c:\\", " user.exe,exitwindows");
}



//Function reads driver config file
//driver reads first 11 bytes of this file (Three 3-digit hex numbers split by semi-colon). Format is strict
//numbers: first - IO port, second - IRQ number, third - shutdown delay in seconds
//000;000;000
VOID __stdcall loadConfigFile() {
	BYTE action;
	WORD err;

	char buf[11];
	int fileOffset = 0;
	int bytesToRead = 11;

	int res;
	char *resStr;
	
	char numParseBuf[3];

	fhConfig = R0_OpenCreateFile(FALSE, CONFIG_FILE_PATH, OPEN_ACCESS_READONLY, ATTR_NORMAL, ACTION_IFEXISTS_OPEN, 0x00, &err, &action);
	

	if (!fhConfig 
			|| (res = R0_ReadFile(FALSE, fhConfig, buf, bytesToRead, fileOffset, &err)) <= 0) {

		Call_Global_Event(sendDebugLog, (PVOID) "Failed to read config file", &eventThunk);
		fileLoggerWriteLine("Failed to read config file\r\n");

		return;
	}


	R0_CloseFile(fhConfig, &err);

	//parse config params

	numParseBuf[0] = buf[0];
	numParseBuf[1] = buf[1];
	numParseBuf[2] = buf[2];
	parsedIOPort = strtol(numParseBuf, NULL, 16);

	numParseBuf[0] = buf[4];
	numParseBuf[1] = buf[5];
	numParseBuf[2] = buf[6];
	parsedIRQNum = strtol(numParseBuf, NULL, 16);

	numParseBuf[0] = buf[8];
	numParseBuf[1] = buf[9];
	numParseBuf[2] = buf[10];
	parsedDelaySec = strtol(numParseBuf, NULL, 16);

	resStr = (char*) malloc(256 * sizeof(char));

	sprintf(resStr, "Config values: IO=%d, IRQ=%d, DELAY=%d\r\n", parsedIOPort, parsedIRQNum, parsedDelaySec);

	fileLoggerWriteLine(resStr);

	Call_Global_Event(sendDebugLog, (PVOID) resStr, &eventThunk);

	free(resStr);
}




// Logging

// send driver debug log
VOID __stdcall sendDebugLog(VMHANDLE hVM, PVOID Refdata, PCLIENT_STRUCT pRegs)
{
	dprintf("%s", (char*) Refdata);
}



VOID __stdcall fileLoggerInit() {
	BYTE action;
	WORD err;
	int existingFileSize;

	fh = R0_OpenCreateFile(FALSE, LOG_FILE_PATH, OPEN_ACCESS_READWRITE, ATTR_NORMAL, 
		ACTION_IFEXISTS_OPEN | ACTION_IFNOTEXISTS_CREATE, 
		0x00, &err, &action);
	
	if (fh) {
		existingFileSize = R0_GetFileSize(fh, &err);

		if (err == 0) {
			filepos = existingFileSize;
		}
	} else {
		Call_Global_Event(sendDebugLog, (PVOID) "Failed to start file logger", &eventThunk);
	}
}

VOID __stdcall fileLoggerClose() {
	WORD err;
	
	R0_CloseFile(fh, &err);
}

VOID __stdcall fileLoggerWriteLine(PCHAR logStr) {
	WORD err;
	int count;
	char buf[512];
	buf[0] = 0;

	fileLoggerInit();
	
	Call_Global_Event(sendDebugLog, (PVOID) logStr, &eventThunk);
	
	count = sprintf(buf, logStr);
	
	if (count) {
		count = R0_WriteFile(FALSE, fh, buf, count, filepos, &err);
		filepos += count; 
	}

	fileLoggerClose();
}




//////////////////////////////////////////////////////////////////////
// utils to execute windows application


struct ExecInfo
{
	SHEXPACKET shex;
	char data[1];
};


VOID _cdecl Appy_Handler(PVOID RefData, DWORD flags)
{
	struct ExecInfo* pInfo = (struct ExecInfo*)RefData;

	SHELL_ShellExecute((SHEXPACKET*)pInfo);

	free(pInfo);
}

// / / / / / / / / / / / / / / / / / / / / / / / / / / / / / / 
// ExecuteApplication
//
// Input
//		file		name of application file to execute
//		startdir	path in which to start execution
//		params		parameter string for app
//
BOOL ExecuteApplication(PCHAR file, PCHAR startdir, PCHAR params)
{
	DWORD flen = strlen(file)+1;
	DWORD dlen = strlen(startdir)+1;
	DWORD plen = strlen(params)+1;

	DWORD size = sizeof(struct ExecInfo) + flen + dlen + plen;

	struct ExecInfo* pInfo = (struct ExecInfo*)malloc(size);

	if (pInfo == NULL) {
		Call_Global_Event(sendDebugLog, (PVOID) "Failed to allocate memory for shell command exec", &eventThunk);

		return FALSE;
	}

	// intialize packet to zero

	memset(pInfo, 0, sizeof(struct ExecInfo));

	// set the size fields

	pInfo->shex.dwTotalSize = size;
	pInfo->shex.dwSize = sizeof(SHEXPACKET);

	// copy data to packet

	pInfo->shex.ibFile = sizeof(SHEXPACKET);
	strcpy(pInfo->data, file);

	pInfo->shex.ibDir = sizeof(SHEXPACKET) + flen;
	strcpy(&pInfo->data[flen], startdir);

	pInfo->shex.ibParams = sizeof(SHEXPACKET) + flen + dlen;
	strcpy(&pInfo->data[flen+dlen], params);

	pInfo->shex.nCmdShow = 1;

	if (_SHELL_CallAtAppyTime(Appy_Handler, pInfo, 0, 0) == 0)
	{
		free(pInfo);
		Call_Global_Event(sendDebugLog, (PVOID) "Failed to execute shell command", &eventThunk);
		return FALSE;
	} else {
		Call_Global_Event(sendDebugLog, (PVOID) "Executed shell command", &eventThunk);
		return TRUE;
	}
}

