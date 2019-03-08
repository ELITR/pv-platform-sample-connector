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
#include <time.h>

#include "Platform.h"
#include "S2STime.h"
#include "MCloud.h"

#define MAX_DATE_LEN 16

typedef void* ASR_Handle;       /* define dummy handle for this examle */
static int windex = 0;           /* dummy index for this example */
static FILE* outputFile = NULL;
static char* outputDir = NULL;
static int output_stdout = 0;

/* =========================================================================
 * Type Definitions
 * ========================================================================= */

/* user data struct for passing information from the main function into the
   callback functions */
typedef struct UserData_S {
  MCloud       *cloudP;
  ASR_Handle    asrH;
  char         *startTime;      /* startTime of the first packet, used as offset */  
} UserData;


/* =========================================================================
 * Helper functions
 * ========================================================================= */
static void printUsage (char *prgName) {
  printf ("\n");
  printf ("\nNAME\n\t%s - Speech Recognition Backend.\n", prgName);
  printf ("\nSYNOPSIS\n\t%s [OPTION]... ASRCONFIGFILE\n", prgName);
  printf ("\nDESCRIPTION\n"
      "\tThis is a example implementation of an audio recorder backend which connects to"
      "\tPEV's Mediator in the cloud.\n"
      );
  printf ("\nOPTION\n"
      "\t-s, --server=HOSTNAME\n\t\tHost name of the server where the Mediator is running.\n\n"
      "\t-p, --serverPort=PORT\n\t\tPort address at which the Mediator accepts worker.\n\n"
      "\t-f, --fingerPrint=FPRINT\n\t\tLanguage finger print of the audio file given. (en-EU)\n\n"
      "\t-i, --inputStream=FPRINT\n\t\tLanguage finger print for the results requested. (en)\n\n"
      "\t-d, --outpuFolder=OUTPUT_FOLDER\n\t\tOutput Folder to put recordings to\n\n"
      "\t-o, --output\n\t\tOutput the recording also to stdout\n\n"
      "\t-T, --ssl\n\t\tProvides ssl connection to Mediator.\n\n"
      "\t-W, --selfSigned\n\t\tAccept self signed certify from Server.\n\n"	
      "\t-h, --help\n\t\tShows this help.\n\n"
      );

  return;
}

/* all packets are tagged with time stamps. Use these functions for working
 * with them more easily.
 * time stamps are currently in the format DD/MM/YY-HH:mm:ss.mss
 */
static void timePrint (S2S_Time *t, char *str) {
  sprintf (str, "%02d/%02d/%02d-%02d:%02d:%02d.%d",
      t->day, t->month, t->year-2000,
      t->hour, t->minute, t->second, t->milliseconds);
}

static int parseTime (char *str, S2S_Time *t) {
  int res = (str) ? sscanf (str, "%02d/%02d/%02d-%02d:%02d:%02d.%d",
      &(t->day), &(t->month), &(t->year),
      &(t->hour), &(t->minute), &(t->second), &(t->milliseconds)) : -1;
  t->year += 2000;

  if ( res == 7 ) return 0;
  return res;
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
  time_t now;
  char date[MAX_DATE_LEN];
  char filename[BUFSIZ];

  UserData *ud = (UserData*)userData;
  ASR_Handle     asrH = ud->asrH;
  
  /* initialize the ASR in order to be able to start decoding a
     new utterance */
  //asr_InitDecode (asrH);

	  
  now = time(NULL);
  date[0] = '\0';
  filename[0] = '\0';

  if(now != -1) {
	strftime(date, MAX_DATE_LEN, "%Y%m%d_%H%M", gmtime(&now));
  }
  
  sprintf(filename, "%s/recording-%s.pcm", outputDir, date);
  outputFile = fopen(filename, "wb");

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
  ASR_Handle     asrH = ud->asrH;

  switch (p->dataType) {
  case MCloudAudio:
  {
    short *sampleA = NULL;
    int    sampleN;

    /* extract audio from packet */
    mcloudPacketGetAudio (cP, p, &sampleA, &sampleN);
    
    fwrite(sampleA, sizeof(short), sampleN, outputFile);
	if (output_stdout)
		fwrite(sampleA, sizeof(short), sampleN, stdout);

    if ( !ud->startTime ) ud->startTime = strdup (p->start);
    
    /* decode the audio data contained in the packet */
    //asr_Decode (asrH, sampleA, sampleN);    

    /* especially for real-time processing, a partial hypothesis should be
       retrieved, otherwise simply do nothing.
       since this example does not contain any ASR component, we simulate
       sending a hypothesis */
    // {
      // MCloudPacket    *np     = NULL;
      // MCloudWordToken *tokenA = NULL;
      
      // S2S_Time startT;
      // S2S_Time stopT;
      // S2S_Time tmpT;
      // char     startTime[128];
      // char     stopTime[128];

      // int n = 1;
      // int i, startMS, stopMS;
      
      // if ( (tokenA = mcloudWordTokenArrayCreate (n)) == NULL ) {
        // fprintf (stderr, "ERROR Memory allocation failure.\n");
        // return S2S_Error;
      // }
      
      // parseTime (p->start, &startT);
      // parseTime (p->stop, &stopT);
      // parseTime (ud->startTime, &tmpT);
      // startMS = s2s_TimeDuration (&tmpT, &startT);
      // stopMS  = s2s_TimeDuration (&tmpT, &stopT);
      
      // /* prepare word token array */
      // for (i=0; i<n; i++) {
        // tokenA[i].index      = windex++;       /* continuously increasing index */
        // tokenA[i].internal   = strdup("internalForm");
        // tokenA[i].written    = strdup("writtenForm");
        // tokenA[i].spoken     = strdup("optional");
        // tokenA[i].confidence = 1.0;
        // tokenA[i].startTime  = startMS; /* start time of word in stream in ms */
        // tokenA[i].stopTime   = stopMS;  /* end time of word in stream in ms */
        // tokenA[i].isFiller   = 0;       /* flag to indicate whether word is a filler word, such as noises */
      // }
      
      // /* all outgoing packets have to be tagged with correct time stamps
         // the current implementation is as follows, but might change in the future
         // time stamps specified in the packet header (or by using the packet init
         // functions) have to be absolute time stamps
         // time stamps within the packet corresponding to words are relative time stamps
         // to the beginning of the audio stream
       // */         
      // if ( parseTime (ud->startTime, &startT) != 0 ) {
        // parseTime ("00/00/00-00:00:00.000", &startT);
      // }
      // if ( parseTime (ud->startTime, &stopT) != 0 ) {
        // parseTime ("00/00/00-00:00:00.000", &stopT);
      // }
      // s2s_AddToTime (&startT, tokenA[0].startTime, 0);      /* start time is the time of the beginning of the stream + the offset wihtin the stream */
      // s2s_AddToTime (&stopT,  tokenA[n-1].stopTime, 0);
      // timePrint (&startT, startTime);
      // timePrint (&stopT,  stopTime);
      
      // /* initialize packet from word token array
         // Each data packet of type text, does contain at least a text string as
         // content, which is in the case when a word token array is used a
         // concatenation of the written forms of the tokens.
      // */
      // np = mcloudPacketInitFromWordTokenA (cP, startTime, stopTime, startMS, stopMS, NULL, tokenA, n);
      
      // /* send packet */
      // mcloudSendPacketAsync (cP, np, NULL);
      // mcloudWordTokenArrayFree (tokenA, n);
    // }
    
    free (sampleA); sampleA = NULL;
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

  UserData *ud = (UserData*)userData;
  ASR_Handle     asrH = ud->asrH;

  /* finalize decoding, e.g. apply lattice rescoring */
  //asr_FinalizeDecode (asrH);

  /* all packets have been processed with dataCallback
     e.g. retrieve last partial result in case of real-time processing,
     or retrieve finalized hypothesis in case of batch processing (see above dataCallback) */
  fclose(outputFile);
  
  return 0;
}

/**
 * This function is called when the worker should stop the processing
 * as soon as possible.
 */
int breakCallback (MCloud *cP, void *userData) {

  UserData *ud = (UserData*)userData;
  ASR_Handle     asrH = ud->asrH;

  /* break processing of pending packets in ASR thread */
  //asr_Break (asrH);

  return 0;
}

/**
 * This function is called as soon as an error occurs in the asynchronous processing.
 */
int errorCallback (MCloud *cP, void *userData) {

  UserData *ud = (UserData*)userData;
  ASR_Handle     asrH = ud->asrH;

  /* break processing of pending packets in ASR thread */
  //asr_Break (asrH);
  fclose(outputFile);

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
  char *inputFingerPrint  = "any-recorder";      /* special finger print for testing */
  const char *inputType         = "audio";
  char *outputFingerPrint = "any-recorder";      /* special finger print for testing */
  const char *outputType        = "text";
  int	ssl = 0; 
  MCloudSSLVerifyMode  verifyMode = MCloudSSL_FullVerify;	
  
  int   optX, o;

  ASR_Handle asrH   = NULL;
  MCloud    *cloudP = NULL;
  UserData   userData;

  static struct option lopt[] = {
      {"serverHost", 1, 0, 's'},
      {"serverPort", 1, 0, 'p'},
      {"fingerPrint", 1, NULL, 'f' },
      {"inputStream", 1, NULL, 'i'},
      {"outputDir", 1, 0, 'd'},
	  {"output",0, 0, 'o'},
      {"ssl", 0, 0, 'T'},
      {"selfSigned", 0, 0, 'W'},	
      {"help", 0, 0, 'h'},
      {NULL, 0, 0, 0}
  };

  /* command line parsing */
  if ( argc < 2 ) {
    printUsage (argv[0]);
    return -1;
  }

  while ( (o = getopt_long (argc, argv, "s:p:d:f:i:ToWh", lopt, &optX)) != -1 ) {
    switch (o) {
    case 's':
      serverHost = strdup (optarg);
      break;
    case 'p':
      serverPort = atoi (optarg);
      break;
    case 'd':
      outputDir = strdup (optarg);
      break;
    case 'f':
      inputFingerPrint = strdup (optarg);
      break;
    case 'i':
      outputFingerPrint = strdup (optarg);
      break;
    case 'T':
      ssl = 1;
      break;
    case 'W':
      verifyMode = MCloudSSL_AcceptSelfSigned;
      break;		
	case 'o':
		output_stdout = 1;
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

  if(outputDir==NULL){
     fprintf(stderr, "INFO The folder where to save the recording was not set. It will be set to the current one.\n");
     outputDir=".";
  }

  /******************************************************
    initialize ASR
    to fill with functions for starting the ASR
    *****************************************************/
  asrH = NULL; /* dummy, for this example */

  cloudP = ssl ? mcloudCreateSSL ("asr", MCloudModeWorker, verifyMode) : mcloudCreate ("asr", MCloudModeWorker);

  /* connect and process */
  if ( cloudP == NULL ) {
    fprintf (stderr, "ERROR creating Cloud Object.\n");
    return -1;
  }

  userData.cloudP      = cloudP;
  userData.asrH        = asrH;
  userData.startTime   = NULL;

  /* add service description for ASR */
  mcloudAddService (cloudP, "PEV recorder", "asr", inputFingerPrint, inputType, outputFingerPrint, outputType, NULL);
  
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

      if ( userData.startTime ) free (userData.startTime); userData.startTime = NULL;

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
          break;
        case MCloudFlush:
          /* a flush message has been received -> wait (block) until all pending packages from
             the processing queue has been processed -> finalizeCallback will be called
             -> flush message will be passed to subsequent components */
          mcloudWaitFinish (cloudP, MCloudProcessingQueue, 0);
          mcloudSendFlush (cloudP);
		  mcloudPacketDeinit (p);
          fprintf (stderr, "INFO received DONE message ==> waiting for packages.\n");
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


  
