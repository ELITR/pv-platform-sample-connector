#ifndef __PLATFORM_H_INCLUDED__
#define __PLATFORM_H_INCLUDED__

#include <stdlib.h>
#include "stdio.h"
#include <string.h>
#include <ctype.h>
//#include "math.h"
#include <assert.h>

#if defined(WIN32)   ||   defined(WIN64)       /* =============================== Windows */

  #include <io.h>
  #include <winsock2.h>
  #include <Ws2tcpip.h>
  #include "windows.h"

  #include <sys/types.h>
  #include <sys/stat.h>
  #include <wchar.h>
  #include <errno.h>
  #include <direct.h>
  #include <time.h>
  #include <fcntl.h>
  #include <string.h>
  #include <mbstring.h>
  #include "Process.h"

/* fix primitive data types */
  typedef size_t            ssize_t;
//#ifndef _UINTPTR_T_DEFINED
  typedef signed __int64           int64_t;    /* integer 64 bit signed */
  typedef signed __int32           int32_t;    /* integer 32 bit signed */
  typedef signed __int16           int16_t;    /* integer 16 bit signed */
  typedef signed __int8            int8_t;     /* integer  8 bit signed */    
  typedef unsigned __int64  uint64_t;   /* integer 64 bit unsigned */
  typedef unsigned __int32  uint32_t;   /* integer 32 bit unsigned */
  typedef unsigned __int16  uint16_t;   /* integer 16 bit unsigned */
  typedef unsigned __int8   uint8_t;    /* integer  8 bit unsigned */ 
  typedef unsigned __int8   bool_t;     /* type for boolean values */
//#endif
//#define stricmp(x,y) ((x) == NULL ? 1 : _stricmp(x,y))
//  #define strdup(s) _strdup(s)
  #define strdup(A) ((A))?_strdup((A)):NULL  
//  #define strcmp(a,b) _strcmp(a,b)
  #define snprintf _snprintf
  #define stricmp(a,b) _stricmp(a,b)
  #define strnicmp(a,b,n) _strnicmp(a,b,n)
  #define strcasecmp(a,b) stricmp(a,b) 
  #define strncasecmp(a,b,n) strnicmp(a,b,n) 
  #define open(fn,m) _open(fn,m) 
  #define close(fp) _close(fp) 
//  #define close(s) closesocket(s) // Thilo: this would close a socket, in contrast to close which closes only files

//  #define ntohl(A) A
//  #define htonl(A) A
  #define mkdir(A,B) _mkdir(A)
  #define O_NONBLOCK 1
  #define F_GETFL 1
  #define F_SETFL 2

  #define CallAsync(hThread,Function,Parameter) hThread = (HANDLE)_beginthread(Function, 0,(void*) Parameter)

  #define Async void
  #define ReturnAsync(result) return
  #define MS_VC_EXCEPTION 0x406D1388

  #pragma pack(push,8)
  typedef struct tagTHREADNAME_INFO {
     DWORD dwType; // Must be 0x1000.
     LPCSTR szName; // Pointer to name (in user addr space).
     DWORD dwThreadID; // Thread ID (-1=caller thread).
     DWORD dwFlags; // Reserved for future use, must be zero.
  } THREADNAME_INFO;
  #pragma pack(pop)
//  void SetThreadName( DWORD dwThreadID, char* threadName);
  #define SetThreadName(a,b)

#ifndef strncasecmp
  #define strncasecmp(A,B,C) strnicmp(A,B,C)
#endif

#elif defined(NN_HARDWARE_CTR) /* =============================== Nintendo */

  #include <unistd.h>
  #include <arpa/inet.h>
  #include "CTRPlatform.h"
  #include "_String.h"
  #include <string.h>
  #include <nn/init.h>
  #include <nn/os.h>
  #include <nn/socket.h>
  #include <nn/ac.h>
  #include <nn/fnd.h>
  #include <time.h>

  typedef long ssize_t;
  #define _stricmp strcasecmp
  #define SetThreadName(a,b)
  #define CallAsync(hThread,Function,Parameter) pthread_create (&hThread, NULL, (Function), Parameter)

  #define ai_family family
  #define ai_socktype sockType
  #define ai_protocol protocol
  #define ai_next next
  #define ai_addr addr
  #define ai_addrlen addrLen

  #define getaddrinfo nn::socket::GetAddrInfo
  #define fcntl nn::socket::Fcntl
  #define freeaddrinfo nn::socket::FreeAddrInfo

  #define ClientServerASR
  #define XCaliburX
  #define NinjaX
  #define ClientServerSMT
  #define PandoraX
  #define TextProcessorX
  #define ClientServerTTS

#elif defined(ANDROID)         /* =============================== Android */

  #include <stdlib.h>
  #include <unistd.h>
  #include <stdarg.h>

  #include <sys/stat.h>
  #include <sys/time.h>
  #include <sys/select.h>
  #include <sys/errno.h>

  #include <arpa/inet.h>
	#include <dirent.h>
  #include <fcntl.h>
  #include <netdb.h>
  #include <netinet/in.h>
  #include <pthread.h>

  #include "glob.h"
  #include "XLog.h"

  #define strdup(A) ((A))?strdup((A)):NULL   
  #define strnicmp(A,B,C) strncasecmp(A,B,C)
  #define stricmp(A,B) strcasecmp(A,B)
  #define HWND void*
  #define Async void*
  #define HANDLE pthread_t
  #define SetThreadName(a,b)
  #define ReturnAsync(result) return(NULL)
  #define CallAsync(hThread,Function,Parameter) pthread_create (&hThread, NULL, (Function), Parameter)
  #define Sleep(x) usleep(x*1000)

  typedef uint8_t   bool_t;     /* type for boolean values */

  enum {
		FILE_ATTRIBUTE_ARCHIVE,
		FILE_ATTRIBUTE_DIRECTORY
	};

#elif defined(__linux__) /* =============================== Linux */

  #include <stdlib.h>
  #include <unistd.h>
  #include <stdarg.h>

  #include <sys/stat.h>
  #include <sys/time.h>
  #include <sys/select.h>
  #include <sys/errno.h>

  #include <arpa/inet.h>
	#include <dirent.h>
  #include <fcntl.h>
  #include <netdb.h>
  #include <netinet/in.h>
  #include <netinet/tcp.h>

  #include <sys/types.h>
  #include <sys/socket.h>
  #include <string.h>
  #include <wchar.h>
  #include <time.h>
  #include <glob.h>
  #include <pthread.h>

#if defined(strdup)
#undef strdup
#endif
  #define strdup(A) ((A))?strdup((A)):NULL   
  #define strnicmp(A,B,C) strncasecmp(A,B,C)
  #define stricmp(A,B) strcasecmp(A,B)
  #define HWND void*
  #define Async void*
  #define HANDLE pthread_t
  #define SetThreadName(a,b)
  #define ReturnAsync(result) return(NULL)
  #define CallAsync(hThread,Function,Parameter) pthread_create (&hThread, NULL, (Function), Parameter)

  #define Sleep(x) usleep(x*1000)

  typedef uint8_t   bool_t;     /* type for boolean values */

  enum {
        FILE_ATTRIBUTE_ARCHIVE,
        FILE_ATTRIBUTE_DIRECTORY
  };

#elif defined(__APPLE__)  /* =============================== MacOS/iPhoneOS */

  #include <unistd.h>
  #include <stdlib.h>
  #include <stdarg.h>
  #include <arpa/inet.h>
  #include <sys/socket.h>
  #include <netdb.h>
  #include <sys/types.h>
  #include <string.h>
  #include <dirent.h>
  #include <fcntl.h>
  #include <sys/stat.h>
  #include <sys/errno.h>
  #include <glob.h>
  #include <stdint.h>
  #include <time.h>
  #include <pthread.h>

// Giga TTS expects those to be set...
#include <machine/endian.h>
#ifdef LITTLE_ENDIAN
    #define _LITTLE_ENDIAN_
#elif BIG_ENDIAN
    #define _BIG_ENDIAN_
#endif

// XXX why do we rely on those monstrosities for portable code?

#define strnicmp(A,B,C) strncasecmp(A,B,C)
#define stricmp(A,B) strcasecmp(A,B)
// this is beyond dangerous.
#define strdup(A) ((A))?strdup((A)):NULL
#define Sleep(x) usleep(x*1000)

typedef void *Async;
typedef pthread_t HANDLE;

#define ReturnAsync(result) return NULL

#ifdef __cplusplus
extern "C" {
#endif

    void _CallAsync(pthread_t *h, void *(*start_routine)(void *), void *arg);
#define CallAsync(thread, ...) _CallAsync(&thread, __VA_ARGS__)
//    void SetThreadName(pthread_t *thread, const char *name);
#define SetThreadName(...)
    pthread_t GetCurrentThreadId(void);

#ifdef __cplusplus
}
#endif
        

typedef unsigned short TCHAR;
typedef char *LPCSTR;
typedef int DWORD;
typedef void *HWND;
typedef uint8_t   bool_t;     /* type for boolean values */

enum {
    FILE_ATTRIBUTE_ARCHIVE,
    FILE_ATTRIBUTE_DIRECTORY
};
	
#elif AMIGA            /* ================================= AMIGA */

typedef unsigned long long    uint64_t;   /* integer 64 bit unsigned */
typedef unsigned long         uint32_t;   /* integer 32 bit unsigned */
typedef unsigned short        uint16_t;   /* integer 16 bit unsigned */
typedef unsigned char         uint8_t;    /* integer  8 bit unsigned */
typedef unsigned char         bool_t;  /* type for boolean values */
typedef long long             int64_t;   /* integer 64 bit unsigned */
typedef int                   int32_t;   /* integer 32 bit unsigned */
typedef short                 int16_t;   /* integer 16 bit unsigned */
typedef signed char           int8_t;    /* integer  8 bit unsigned */

#define expf exp
#define floorf floor
#define fmodf fmod
#define powf pow
#define logf log
#define atan2f atan2
#define cosf cos
#define sinf sin
#define sqrtf sqrt

//#define msg(msg,...) fprintf(stdout,msg,...)
//#define error(msg,...) fprintf(stderr,msg,...)
//#define debug(msg,...) fprintf(stdout,msg,...)

#else /* ================================= other */
...

#endif


/* define endianes */
#ifdef LITTLE_ENDIAN
  /* we are in little endian mode */
#elif BIG_ENDIAN
  /* we are in big endian mode */
#elif defined(AMIGA) || defined(PPC)
  #define BIG_ENDIAN
#else /* must be x86, ARM or something */
  #define LITTLE_ENDIAN
#endif 


/* boolean constants */
#if !defined(__cplusplus)
#if !defined(True)
#define True  1
#define False 0
#endif
#endif

/* other constants */
#if !defined(M_PI)
//#define M_PI 3.1415926535897932384626433832795
#endif
#if !defined(M_PIf)
#define M_PIf 3.1415926535897932384626433832795f
#endif

/* helper functions */
#ifndef max
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif

/* error handling */
#ifdef ANDROID
#define msg xlogf
#define error xlogf
#define dbug xlogf
#else
extern void error(const char *text, ...);
extern void msg  (const char *msg, ...);
extern void dbug (const char *debug, ...);
#endif

/* helper macros for memory access */
#define poke_l(addr,val) *((int32*)(addr)) = (int32)(val)
#define poke_w(addr,val) *((int16*)(addr)) = (int16)(val)
#define poke_b(addr,val) *((int8 *)(addr)) = (int8)(val)
#define poke_f(addr,val) *((float*)(addr)) = (float)(val)
#define poke_s(addr,val) str_Poke((char*)(addr), (str)(val),-1)

#define peek_l(addr) *((int32*)(addr))
#define peek_w(addr) *((int16*)(addr))
#define peek_b(addr) *((int8 *)(addr))
#define peek_f(addr) *((float*)(addr))
#define peek_s(addr) cstr((char*)(addr))

/* helper macros for endianes conversion */
#define XEndian16(A) (((A)<<8)&0xFF00) | (((A)>>8)&0x00FF)
#define XEndian32(A) (((A)<<24)&0xFF000000) | (((A)<<8)&0x00FF0000) | (((A)>>8)&0x0000FF00) | (((A)>>24)&0x000000FF)

#ifdef LITTLE_ENDIAN
  #define TagID(a,b,c,d) ((uint32) (d)<<24 | (uint32) (c)<<16 | (uint32) (b)<<8 | (uint32) (a))
  #define IntTag(A) (((((int32)A[3])<<24)&0xFF000000) | ((((int32)A[2])<<16)&0x00FF0000) | ((((int32)A[1])<<8)&0x0000FF00) | ((((int32)A[0]))&0x000000FF))
  #define BigEndian32(A) XEndian32(A)
  #define BigEndian16(A) XEndian16(A)
  #define LittleEndian32(A) (A)
  #define LittleEndian16(A) (A)
#else
  #define TagID(a,b,c,d) ((uint32) (a)<<24 | (uint32) (b)<<16 | (uint32) (c)<<8 | (uint32) (d))
  #define IntTag(A) (((((int32)A[0])<<24)&0xFF000000) | ((((int32)A[1])<<16)&0x00FF0000) | ((((int32)A[2])<<8)&0x0000FF00) | ((((int32)A[3]))&0x000000FF))
  #define BigEndian32(A) (A)
  #define BigEndian16(A) (A)
  #define LittleEndian32(A) XEndian32(A)
  #define LittleEndian16(A) XEndian16(A)
#endif

#define peek_lbe(addr) BigEndian32(*((int32*)(addr)))
#define poke_lbe(addr,val) *((int32*)(addr)) = BigEndian32((int32)(val))
#define peek_lle(addr) LittleEndian32(*((int32*)(addr)))
#define poke_lle(addr,val) *((int32*)(addr)) = LittleEndian32((int32)(val))
#endif
