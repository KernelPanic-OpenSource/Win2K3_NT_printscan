#ifndef FXSRES_H
#define FXSRES_H
#include <windows.h>
#include <FXSAPIP.H>

#ifdef __cplusplus
extern "C"{
#endif

HINSTANCE WINAPI GetResInstance(HINSTANCE hModule);
HINSTANCE WINAPI GetResInst(LPCTSTR pResFile, HINSTANCE hModule);
void WINAPI FreeResInstance(void);
UINT WINAPI GetErrorStringId(DWORD ec);

#ifdef __cplusplus
}
#endif

#define FAXUI_ERROR_DEVICE_LIMIT        14000L
#define FAXUI_ERROR_INVALID_RING_COUNT  14001L
#define FAXUI_ERROR_SELECT_PRINTER      14002L
#define FAXUI_ERROR_NAME_IS_TOO_LONG    14003L
#define FAXUI_ERROR_INVALID_RETRIES     14004L
#define FAXUI_ERROR_INVALID_RETRY_DELAY 14005L
#define FAXUI_ERROR_INVALID_DIRTY_DAYS  14006L
#define FAXUI_ERROR_INVALID_CSID        14007L
#define FAXUI_ERROR_INVALID_TSID        14008L


#endif

