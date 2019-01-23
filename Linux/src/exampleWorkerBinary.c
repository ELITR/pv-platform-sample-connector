/*
 * backendASR2.c
 *
 *  Created on: Jan 23, 2014
 *      Author: PerVoice
 *      (c) PerVoice
 *
 * This is an example implementation for an ASR backend process, which connects
 * to the MCloud and waits for audio input to process.
 * Note that this example doesn't contain any ASR component, instead it
 * returns always some dummy results.
 */

#include <getopt.h>

#include "Platform.h"
#include "S2STime.h"
#include "MCloud.h"

static int windex = 0;           /* dummy index for this example */

/* =========================================================================
 * Type Definitions
 * ========================================================================= */

typedef struct OpenFile_S {
  char *name;
  FILE *file;
  struct OpenFile_S *next;
} Open_File;

/* user data struct for passing information from the main function into the
   callback functions */
typedef struct UserData_S {
  MCloud       *cloudP;
  char         *outDir;
  Open_File     *openFiles;
} UserData;


/* =========================================================================
 * Helper functions
 * ========================================================================= */
static void printUsage (char *prgName) {
  printf ("\n");
  printf ("\nNAME\n\t%s - Binary Worker Example.\n", prgName);
  printf ("\nSYNOPSIS\n\t%s [OPTION]\n", prgName);
  printf ("\nDESCRIPTION\n"
      "\tThis is a example implementation of worker accepting arbitrary binary files"
      );
  printf ("\nOPTION\n"
      "\t-s, --serverHost=HOSTNAME\n\t\tHost name of the server where the Mediator is running.\n\n"
      "\t-p, --serverPort=PORT\n\t\tPort address at which the Mediator accepts worker.\n\n"
      "\t-f, --outputFP=FPRINT\n\t\tLanguage finger print of the audio file given.\n\n"
      "\t-i, --inputFP=FPRINT\n\t\tLanguage finger print for the results requested.\n\n"
      "\t-t, --outputType=FPRINT\n\t\tType of the results requested. (binary)\n\n"	  
      "\t-d, --outDir=OUTPUT_FOLDER\n\t\tOutput Folder to put recordings to\n\n"	  
      "\t-h, --help\n\t\tShows this help.\n\n"
      );

  return;
}


static FILE* getOutFile(UserData *ud, char *filename, int *new) {
  Open_File *of = ud->openFiles;
  Open_File **n = &ud->openFiles;

  if (new) *new = 0;
  while (of != NULL) {
    if (strcmp(of->name, filename) == 0) break;
    else {
      of = of->next;
      n = &of->next;
    }
  }

  if (of == NULL) {
    char path[1024];

    int t = (int)time(NULL);
    snprintf(path, sizeof(path), "%s/%s_%d", ud->outDir, filename, t);
    of = malloc(sizeof(Open_File));
    of->name = strdup(filename);
    of->file = fopen(path, "wb");
    of->next = NULL;
    if (of->file == NULL) {
      fprintf(stderr, "Error opening file for dumping: %s\n", path);
      free(of);
    } else *n = of;
    if (new) *new = 1;
  }

  return of->file;
}

static void closeOutFile(UserData *ud, char *filename) {
  Open_File *prev = NULL;
  Open_File *of = ud->openFiles;
  
  while (of != NULL) {
    if (strcmp(of->name, filename) == 0) break;
    else {
      of = of->next;
      prev = of;
    }
  }

  if (of) {
    if (prev) {
      prev->next = of->next;
    } else {
      ud->openFiles = of->next;
    }
    fclose(of->file);
    free(of);
  }
}

/* =========================================================================
 * Callback Functions
 * ========================================================================= */

/**
 * This function is called as soon as an incoming service request has been
 * accepted by the worker, i.e. in mcloudWaitForClient.
 * The packet containing the service description is passed to the init callback
 * function as argument.
 */
int initCallback (MCloud *cP, MCloudPacket *p, void *userData) {
  UserData *ud = (UserData*)userData;

  return 0;
}

/**
 * This function is called for each incoming data package in a serial way, i.e.
 * after a package has been processed it is called again if more packages are pending.
 * Note that for this function no userData is given at the time of the set
 * of the callback function. Instead, the userData is given per packet with
 * mcloudProcessDataAsync or mcloudSendAsync.
 */ 
int dataCallback (MCloud *cP, MCloudPacket *p, void *userData) {

  UserData *ud = (UserData*)userData;

  switch (p->dataType) {
  case MCloudBinary:
  {
    uint8_t *bytesA = NULL;
    int    bytesN;
    char *filename;
    char *mimetype;
    int last;
    int new = 1;
	FILE *outFile;
	
    /* extract audio from packet */
    mcloudPacketGetBinary (cP, p, &bytesA, &bytesN, &filename, &mimetype, &last);

    if (filename == NULL) filename="unnamed";
    if (mimetype == NULL) mimetype="unknown";
    outFile = getOutFile(ud, filename, &new);
    if (new) fprintf(stderr, "INFO Starting dump of file=%s mime=%s\n", filename, mimetype);

    fwrite(bytesA, sizeof(uint8_t), bytesN, outFile);

    free (bytesA); bytesA = NULL;

    if (last > 0) {
      fprintf(stderr, "INFO Fiinished dump of %s\n", filename);
      closeOutFile(ud, filename);
    }
  }
  break;
  default:
    fprintf (stderr, "ERROR Unsupported data type %d.\n", p->dataType);
  }

  return 0;
}

/**
 * This function is called as soon as the processing of packets
 * should be finalized, i.e. no more packets will follow and the worker
 * should output the final results after all pending packets have been
 * processed.
 */
int finalizeCallback (MCloud *cP, void *userData) {


  return 0;
}

/**
 * This function is called when the worker should stop the processing
 * as soon as possible.
 */
int breakCallback (MCloud *cP, void *userData) {

  UserData *ud = (UserData*)userData;

  /* break processing of pending packets in ASR thread */
  //asr_Break (asrH);

  return 0;
}

/**
 * This function is called as soon as an error occurs in the asynchronous processing.
 */
int errorCallback (MCloud *cP, void *userData) {

  UserData *ud = (UserData*)userData;

  /* break processing of pending packets in ASR thread */
  //asr_Break (asrH);

  return 0;
}


/* =========================================================================
 * Main Function
 * ========================================================================= */
int main (int argc, char * argv[]) {

  char *serverHost    = NULL;
  int   serverPort    = 60021;
  
  /* Fingerprints are used to specify the exact language and genre of a media
     stream. They consist of a two-letter language code (ISO369-1) followed by
     an optional two-letter country code (ISO3166) and an optional additional
     string specifying other properties such as domain, type, version, or
     dialect. Different parts have to be separated by a minus sign
     ({\tt ll[-LL[-dddd]]}). For example, {\tt en-EU} means European accented
     English, {\tt de-AT} denotes German that is spoken in Austria, and
     {\tt en-US-weather} is the fingerprint of a media stream for the US
     American accented English in the weather-domain.
     See also the documentation for more details.
  */
  const char *inputFingerPrint  = "any-binary";      /* special finger print for testing */
  const char *inputType         = "binary";
  const char *outputFingerPrint = "null-binary";      /* special finger print for testing */
  const char *outputType        = "binary";
  const char *outputDir         = ".";
  
  int   optX, o;

  MCloud    *cloudP = NULL;
  UserData   userData;

  static struct option lopt[] = {
      {"serverHost", 1, 0, 's'},
      {"serverPort", 1, 0, 'p'},
      {"outDir", 1, 0, 'd'},
      {"inputFP", 1, 0, 'i'},
      {"outputFP", 1, 0, 'f'}, 
      {"outputType", 1, 0, 't'},
      {"help", 0, 0, 'h'},
      {NULL, 0, 0, 0}
  };

  /* command line parsing */
  if ( argc < 2 ) {
    printUsage (argv[0]);
    return -1;
  }

  while ( (o = getopt_long (argc, argv, "s:p:d:i:f:t:h", lopt, &optX)) != -1 ) {
    switch (o) {
    case 's':
      serverHost = strdup (optarg);
      break;
    case 'p':
      serverPort = atoi (optarg);
      break;
    case 'i':
      inputFingerPrint = strdup(optarg);
      break;
    case 'f':
      outputFingerPrint = strdup(optarg);
      break;
    case 't':
      outputType = strdup (optarg);
      break;      
    case 'd':
      outputDir = strdup (optarg);
      break;      
    case 'h':
    case '?':
    default:
      printUsage (argv[0]);
      return -1;
    }
  }

  if ( optind != argc ) {
    printUsage (argv[0]);
    return -1;
  }
  

  /******************************************************
    initialize ASR
    to fill with functions for starting the ASR
    *****************************************************/

  /* connect and process */
  if ( (cloudP = mcloudCreate ("asr", MCloudModeWorker)) == NULL ) {
    fprintf (stderr, "ERROR creating Cloud Object.\n");
    return -1;
  }

  userData.cloudP      = cloudP;
  userData.outDir      = outputDir;
  userData.openFiles   = NULL;

  /* add service description for ASR */
  mcloudAddService (cloudP, "PerVoice bin-worker", "binary-dump", inputFingerPrint, inputType, outputFingerPrint, outputType, NULL);
  fprintf(stderr, "Added service inputFP=%s inputType=%s outputFP=%s outputType=%s\n", inputFingerPrint, inputType, outputFingerPrint, outputType);
  /* set callback functions */
  mcloudSetInitCallback (cloudP, initCallback, &userData);
  mcloudSetDataCallback (cloudP, dataCallback);
  mcloudSetFinalizeCallback (cloudP, finalizeCallback, &userData);
  mcloudSetErrorCallback (cloudP, MCloudProcessingQueue, errorCallback, &userData);
  mcloudSetBreakCallback (cloudP, MCloudProcessingQueue, breakCallback, &userData);
  
  fprintf (stderr, "INFO trying to connect to %s at port %d.\n", serverHost, serverPort);
  while ( 1 ) {
    int err = 0; 

    if ( mcloudConnect (cloudP, serverHost, serverPort) != S2S_Success ) {
      Sleep (2000);
      continue;
    }
    fprintf (stderr, "INFO connection established ==> waiting for clients.\n");

    while ( 1 ) {
      MCloudPacket *p        = NULL;
      char         *streamID = NULL;
      int           proceed  = 1;

      if ( mcloudWaitForClient (cloudP, &streamID) != S2S_Success ) {
        fprintf (stderr, "ERROR while waiting for client.\n");
        break;
      }
      fprintf (stderr, "INFO received client request ==> waiting for packages.\n");
      while ( proceed && (p = mcloudGetNextPacket (cloudP)) != NULL ) {
        switch (p->packetType) {
        case MCloudData:
          /* a data message has been received -> append it to the processing queue */
          mcloudProcessDataAsync (cloudP, p, &userData);
          fflush(stderr);
          break;
        case MCloudFlush:
          /* a flush message has been received -> wait (block) until all pending packages from
             the processing queue has been processed -> finalizeCallback will be called
             -> flush message will be passed to subsequent components */
          mcloudWaitFinish (cloudP, MCloudProcessingQueue, 0);
          mcloudSendFlush (cloudP);
          fprintf (stderr, "INFO received DONE message ==> waiting for packages.\n");
		  mcloudPacketDeinit (p);
          break;
        case MCloudDone:
          /* a done message has been received -> wait (block) until all pending packages from
             the processing queue has been processed -> finalizeCallback will be called
             -> (send done) */
          mcloudWaitFinish (cloudP, MCloudProcessingQueue, 1);
          fprintf (stderr, "INFO received DONE message ==> waiting for clients.\n");
		  mcloudPacketDeinit (p);
          proceed = 0;
          break;
        case MCloudError:
          /* an error message has been received -> break processing */
          mcloudBreak (cloudP, MCloudProcessingQueue);
          fprintf (stderr, "INFO received ERROR message ==> waiting for clients.\n");
		  mcloudPacketDeinit (p);
          proceed = 0;
          break;
        case MCloudReset:
          /* a reset message has been received -> break processing */
          mcloudBreak (cloudP, MCloudProcessingQueue);
          fprintf (stderr, "INFO received RESET message ==> waiting for clients.\n");
		  mcloudPacketDeinit (p);
          proceed = 0;
          break;
        default:
          fprintf (stderr, "ERROR unknown packet type %d\n", p->packetType);
		  mcloudPacketDeinit (p);
          proceed = 0;
          err = 1;
        }
      }

      if ( p == NULL ) {
        fprintf (stderr, "ERROR while waiting for messages.\n");
        err = 1;
      }

      if ( err ) break;
    }

    fprintf (stderr, "WARN connection terminated ==> trying to reconnect.\n");
  }
    
  /* free */

  return 0;
}


  
