/*
 * backendSMT.c
 *
 *  Created on: Jan 23, 2014
 *      Author: PerVoice
 *      (c) PerVoice
 *
 * This is an example implementation for an SMT backend process, which connects
 * to the MCloud and waits for text input to process.
 * Note that this example doesn't contain any SMT component, instead it
 * returns always some dummy results.
 */

#include <getopt.h>

#include "Platform.h"
#include "FingerPrints.h"
#include "MCloud.h"

/* define dummy handle for this examle */
typedef void* SMT_Handle;

/* =========================================================================
 * Type Definitions
 * ========================================================================= */

/* user data struct for passing information from the main function into the
   callback functions */
typedef struct UserData_S {
  MCloud    *cP;
  SMT_Handle smtH;
  char      *inputFingerPrint;
  char      *outputFingerPrint;
} UserData;


/* =========================================================================
 * Helper functions
 * ========================================================================= */
static void printUsage (char *prgName) {
  printf ("\n");
  printf ("\nNAME\n\t%s - Machine Translation Backend.\n", prgName);
  printf ("\nSYNOPSIS\n\t%s [OPTION]... TP-CONFIGFILE SMT-CONFIGFILE\n", prgName);
  printf ("\nDESCRIPTION\n"
      "\tThis is a example implementation of an SMT backend which connects to"
      "\tMTEC's Mediator in the cloud.\n"
      );
  printf ("\nOPTION\n"
      "\t-s, --server=HOSTNAME\n\t\tHost name of the server where the Mediator is running.\n\n"
      "\t-p, --serverPort=PORT\n\t\tPort address at which the Mediator accepts worker.\n\n"
      "\t-h, --help\n\t\tShows this help.\n\n"
      );

  return;
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

  UserData  *data = (UserData*)userData;
  SMT_Handle smtH = data->smtH;

  /* initialize the SMT in order to be able to start translating a
     new utterance */

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

  UserData  *data = (UserData*)userData;
  SMT_Handle smtH = data->smtH;

  switch (p->dataType) {
  case MCloudText:
  {
    char *text = NULL;
    char *targetFingerPrint = NULL;
    char  outputText[4096];

    FingerPrintMatchResult fpRes = 0;
    
    /* extract text from packet
       We simply use the text string stored in the packet and currently do not
       care about stored token arrays.
       Note that the MT must cope with probably differently formatted text
       strings due to different ASR engines. Therefore, the backend should
       contain some functions for re-formatting the text as required by the MT.
    */
    mcloudPacketGetText (cP, p, &text);

    /* determine translation direction */
    fpRes = matchFingerPrints ((char*)p->fingerPrint, data->inputFingerPrint);
    targetFingerPrint = data->outputFingerPrint;
    if ( matchFingerPrints ((char*)p->fingerPrint, data->outputFingerPrint) > fpRes ) {
      targetFingerPrint = data->inputFingerPrint;
    }

    /* re-format text as required by MT engine */

    /* translate text into outputText */
    //smt_Translate (smtH, text, outputText, targetFingerPrint);

    /* since this example does not contain any SMT component, we simluate sending
       a translation result */
    {
      outputText[0] = '\0';
      sprintf (outputText, "translated %s", text);
    }
    
    /* currently, and in this example, the time stamps from the ASR word token
       array are not somehow transferred to the translation. Instead, the packet
       content is always translated at whole, i.e. the time stamps must correspond
       exactly with the time stamps of the packet received */
    p = mcloudPacketInitFromText (cP, p->start, p->stop, p->startOffset, p->stopOffset, targetFingerPrint, outputText);

    /* send packet */
    mcloudSendPacketAsync (cP, p, NULL);
    free (text); text = NULL;
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

  UserData  *data = (UserData*)userData;
  SMT_Handle smtH = data->smtH;

  return 0;
}

/**
 * This function is called when the worker should stop the processing
 * as soon as possible.
 */
int breakCallback (MCloud *cP, void *userData) {

  UserData  *data = (UserData*)userData;
  SMT_Handle smtH = data->smtH;

  /* break processing of pending packets */
  //smt_Break (smtH);

  return 0;
}

/**
 * This function is called as soon as an error occurs in the asynchronous processing.
 */
int errorCallback (MCloud *cP, void *userData) {

  UserData  *data = (UserData*)userData;
  SMT_Handle smtH = data->smtH;

  /* break processing of pending packets */
  //smt_Break (smtH);

  return 0;
}


/* =========================================================================
 * Main Function
 * ========================================================================= */
int main (int argc, char * argv[]) {

  char *serverHost        = NULL;
  int   serverPort        = 80;
  char *username      = "";
  char *password      = "";

  
  /* see documentation for correct finger print specification */
  char *inputFingerPrint  = "en-DB";    /* special finger print for testing */
  char *inputType         = "text";
  char *outputFingerPrint = "es-DB";    /* special finger print for testing */
  char *outputType        = "text";
  int   optX, o;

  SMT_Handle smtH   = NULL;
  MCloud    *cloudP = NULL;
  UserData   userData;

  static struct option lopt[] = {
      {"serverHost", 1, 0, 's'},
      {"serverPort", 1, 0, 'p'},
      {"username", 1, NULL, 'U'},
      {"password", 1, NULL, 'P'},
      {"help", 0, 0, 'h'},
      {NULL, 0, 0, 0}
  };

  /* command line parsing */
  if ( argc < 2 ) {
    printUsage (argv[0]);
    return -1;
  }

  while ( (o = getopt_long (argc, argv, "s:p:U:P:h", lopt, &optX)) != -1 ) {
    switch (o) {
    case 's':
      serverHost = strdup (optarg);
      break;
    case 'p':
      serverPort = atoi (optarg);
      break;
    case 'U':
      username = strdup (optarg);
      break;	
    case 'P':
      password = strdup (optarg);
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
    initialize SMT
    to fill with functions for starting the SMT
    *****************************************************/
  smtH = NULL; /* dummy, for this example */

  /* connect and process */
  if ( (cloudP = mcloudCreate2 ("smt", MCloudModeWorker, username, password)) == NULL ) {
    fprintf (stderr, "ERROR creating Cloud Object.\n");
    return -1;
  }

  userData.cP        = cloudP;
  userData.smtH      = smtH;
  userData.inputFingerPrint  = inputFingerPrint;
  userData.outputFingerPrint = outputFingerPrint;

  /* add service decriptions for a bi-directional MT system
     for an uni-directional MT system, only one service description has to be added */
  mcloudAddService (cloudP, "MTEC SMT", "smt", inputFingerPrint, inputType, outputFingerPrint, outputType, NULL);
  mcloudAddService (cloudP, "MTEC SMT", "smt", outputFingerPrint, outputType, inputFingerPrint, inputType, NULL);

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


  
