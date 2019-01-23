/*
 * FingerPrints.h
 *
 *  Created on: Jun 29, 2010
 *      Author: fuegen
 */

#ifndef _FINGERPRINTS_H_
#define _FINGERPRINTS_H_

#ifdef __cplusplus
extern "C" {
#endif

#if defined(_USRDLL) /* this is compiled as DLL */
#define SHAREDDLL __declspec(dllexport)
#else
#define SHAREDDLL
#endif

    /**
   * FingerPrintMatchResult
   */
  typedef enum FingerPrintMatchResult_E {
    FPMR_NoMatch        = 0,
    FPMR_StemOnly       = 1,
    FPMR_StemAndDomain  = 2, 
    FPMR_StemAndCountry = 3, 
    FPMR_Exact          = 4
  } FingerPrintMatchResult;


extern SHAREDDLL FingerPrintMatchResult matchFingerPrints (const char *request, const char *model);
extern SHAREDDLL int splitFingerPrint (const char *fingerPrint, char *source, char *target);
extern SHAREDDLL const char* getFingerPrintLanguageName_deprecated(const char *fingerPrint);
extern SHAREDDLL char* setFingerPrintDomain(char *fingerPrint, const char *domain);
extern SHAREDDLL char* copyFingerPrintStem(char *stem, const char *fingerPrint);
extern SHAREDDLL char* composeFingerPrintFromStrings(char* fingerPrint, const char* language, const char *country, const char *domain);
extern SHAREDDLL char *getFingerPrintLanguage(char *language, const char *fingerPrint);
extern SHAREDDLL char *getFingerPrintCountry(char *country, const char *fingerPrint);
extern SHAREDDLL char *getFingerPrintDomain(char *domain, const char *fingerPrint);
extern SHAREDDLL char *getSourceFingerPrint(char *source, const char* fingerPrints);
extern SHAREDDLL char *getTargetFingerPrint(char *target, const char* fingerPrints);
extern SHAREDDLL char *combineSourceTargetFingerPrint(char *fingerPrints, const char* source, const char* target);

#ifdef __cplusplus
}
#endif


#endif /* _FINGERPRINTS_H_ */
