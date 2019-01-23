#ifndef __S2STIME_INCLUDED__
#define __S2STIME_INCLUDED__

#include "Platform.h"

#ifdef __cplusplus
#ifndef NN_HARDWARE_CTR
extern "C" {
#endif
#endif

#if defined(_USRDLL) /* this is compiled as DLL */
#define SHAREDDLL __declspec(dllexport)
#else
#define SHAREDDLL
#endif


/**
* Platform independent time structure.
*/
typedef struct S2S_Time_S {

#ifdef WIN32
  __int64 utime;
#else
  unsigned long long int utime;
#endif

  int year;
  int month;
  int day;
  int hour;
  int minute;
  int second;
  int milliseconds;
} S2S_Time;


/**
* Initialize time.
*
* @param[int] t  Time.
*/
extern SHAREDDLL void s2s_TimeInit (S2S_Time *t);

/**
* Get the current system time.
*
* @param[in,out] time  The current system time.
* @return              Pointer to the time structure.
*/
extern SHAREDDLL S2S_Time *s2s_GetSystemTime(S2S_Time *time);

/**
* Compare two times.
*
* @return  -1 if @a t1 is smaller than @a t2, 1 if @a t1 is greater than @a t2, 0 if both are equal.
*/
extern SHAREDDLL int s2s_CmpTime(S2S_Time *t1, S2S_Time *t2);

/**
* Add number of milliseconds and days to a given time.
*
* @param[in,out] t      The time to which @a msecs and @a days should be added.
* @param[in]     msecs  Number of milliseconds to add.
* @param[in]     days   Number of days to add.
*
* @return               A pointer to the result time.
*/
extern SHAREDDLL S2S_Time *s2s_AddToTime (S2S_Time *t, unsigned int msecs, unsigned int days);


/**
* Return difference between two S2S_Time in milliseconds.
*
* @param[in] from  Start time.
* @param[in] to    Stop time.
*
* @return          difference in milliseconds.
*/
extern SHAREDDLL unsigned int s2s_TimeDuration (S2S_Time *from, S2S_Time *to);

/**
* Converts time into a human readable format.
*
* @param[in] t    Time.
* @param[in] str  Buffer to which the time should be printed.
*/
extern SHAREDDLL void s2s_TimePrint (S2S_Time *t, char *str);

#ifdef __cplusplus
#ifndef NN_HARDWARE_CTR
}
#endif
#endif

#endif
