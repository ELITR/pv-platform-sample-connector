#ifndef MCloudPThread_h
#define MCloudPThread_h

#if defined(_USRDLL) /* this is compiled as DLL */
#define SHAREDDLL __declspec(dllexport)
#else
#define SHAREDDLL
#endif

#include "Platform.h"

typedef struct MCloudPThreadHandle_S *MCloudPThread;

extern SHAREDDLL MCloudPThread* MCloudPThreadInit();

extern SHAREDDLL void MCloudPThreadCallAsync(MCloudPThread* mCloudPThread, Async (*routine)(void *), void *arg);

extern SHAREDDLL void MCloudPThreadFree(MCloudPThread* mCloudPThread);

#endif