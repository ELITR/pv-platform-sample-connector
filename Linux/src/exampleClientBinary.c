#include <getopt.h>
#include <stdio.h>
#include <string.h>

#include "Platform.h"
#include "MCloud.h"

static void printUsage (char *prgName) {
  printf ("\n");
  printf ("\nNAME\n\t%s - Example Client.\n", prgName);
  printf ("\nSYNOPSIS\n\t%s -F <file1> [-N <name1> -M <type1>] [ -F <file2> [-N <name2> -M <type2>] ...]\n", prgName);
  printf ("\nDESCRIPTION\n"
          "\tThis client connects to Pervoice Mediator running in the cloud.\n"
          );
  printf ("\nOPTION\n"
          "\t-s, --serverHost=HOSTNAME\n\t\tHost name of the server where the Mediator is running.\n\n"
          "\t-p, --serverPort=PORT\n\t\tPort address at which the Mediator accepts worker. (4445)\n\n"
          "\t-f, --fingerPrint=FPRINT\n\t\tLanguage finger print of the audio file given. (en-EU)\n\n"
          "\t-i, --inputStream=FPRINT\n\t\tLanguage finger print for the results requested. (en)\n\n"
          "\t-t, --inputType=FPRINT\n\t\tType of the results requested. (text)\n\n"
          "\t-F\n\t\tPath to a file to send.\n\n"
          "\t-N\n\t\tName of the file to use when sending.\n\n"
          "\t-M\n\t\tMime type to declare for the file.\n\n"
          "\t-h, --help\n\t\tShows this help.\n\n"
          );
  printf ("\n\tVersion v1.1\n");
  printf ("\t(c) PerVoice. All rights reserved.\n");
  printf ("\t(c) PerVoice SpA. All rights reserved.\n");

  return;
}

struct file_details {
  FILE *f;
  char *name;
  char *mime;
};


typedef struct recvUserData_S {
	MCloud *cloudP;
	int     proceed;
} recvUserData;

Async recvMessagesAsyncMain (void *lpParam) {
  recvUserData     *ud  = (recvUserData *)lpParam;
  MCloud       *cloudP  = (MCloud*)ud->cloudP;
  MCloudPacket *p       = NULL;

  ud->proceed = 1;
  while ( ud->proceed && (p = mcloudGetNextPacket (cloudP)) != NULL ) {
    switch (p->packetType) {
    case MCloudData:
      fprintf (stderr, "INFO received DATA message ==> ignoring:\n%s\n\n", p->xmlString);
	  mcloudPacketDeinit (p);
      break;
    case MCloudFlush:
	  mcloudPacketDeinit (p);
      break;
    case MCloudDone:
      mcloudWaitFinish (cloudP, MCloudProcessingQueue, 1);
      fprintf (stderr, "INFO received DONE message ==> end thread.\n");
	  mcloudPacketDeinit (p);
      ud->proceed = 0;
      break;
    case MCloudError:
      mcloudBreak (cloudP, MCloudProcessingQueue);
      fprintf (stderr, "INFO received ERROR message ==> end thread.\n");
	  mcloudPacketDeinit (p);
      ud->proceed = 0;
      break;
    case MCloudReset:
      mcloudBreak (cloudP, MCloudProcessingQueue);
      fprintf (stderr, "INFO received RESET message ==> end thread.\n");
	  mcloudPacketDeinit (p);
      ud->proceed = 0;
      break;
    default:
      fprintf (stderr, "ERROR unknown packet type %d\n", p->packetType);
	  mcloudPacketDeinit (p);
      ud->proceed = 0;
    }
  }

  ReturnAsync (NULL);
}

int recvMessagesAsync (MCloud *cloudP, recvUserData *ud) {

  HANDLE hThread;

  CallAsync (hThread, recvMessagesAsyncMain, (void*)ud);

  return S2S_Success;
}



int main (int argc, char * argv[]) {

  char *serverHost    = NULL;
  int   serverPort    = 4445;
  int   optX, o, i;

  char    *fingerPrint  = NULL;
  char   **inputStreamA = NULL;
  int      inputStreamN = 0;
  char    *inputType    = NULL;

  struct file_details *filedetA = NULL;
  int filedetN = 0;

  MCloud         *cloudP = NULL;

  recvUserData uData;

  static struct option lopt[] = {
    {"serverHost", 1, NULL, 's'},
    {"serverPort", 1, NULL, 'p'},
    {"fingerPrint", 1, NULL, 'f' },
    {"inputStream", 1, NULL, 'i'},
    {"inputType", 1, 0, 't'},
    { "help", 0, 0, 'h' },  
    {NULL, 0, 0, 0}
  };

  /* command line parsing */
  if ( argc < 0 ) {
    printUsage (argv[0]);
    return -1;
  }

  while ( (o = getopt_long (argc, argv, "s:p:f:i:t:F:M:N:h", lopt, &optX)) != -1 ) {
    switch (o) {
    case 's':
      serverHost = strdup (optarg);
      break;
    case 'p':
      serverPort = atoi (optarg);
      break;
    case 'f':
      fingerPrint = strdup (optarg);
      break;
    case 'i':
      i = inputStreamN++;
      inputStreamA = realloc (inputStreamA, inputStreamN * sizeof (char*));
      inputStreamA[i] = strdup (optarg);
      break;
    case 't':
      inputType = strdup (optarg);
      break;
    case 'F':
      filedetN++;
      filedetA = realloc(filedetA, filedetN * sizeof(struct file_details));
      filedetA[filedetN-1].f = fopen(optarg, "rb");
      if (filedetA[filedetN-1].f == NULL) {
        fprintf(stderr, "Error opening file: %s", optarg);
        exit(2);
      }
      filedetA[filedetN-1].name = NULL;
      filedetA[filedetN-1].mime = NULL;

      break;
    case 'N':
      if (filedetN <= 0) {
        fprintf(stderr, "No file specified!");
        exit(2);
      }
      filedetA[filedetN-1].name = strdup(optarg);
      break;
    case 'M':
      if (filedetN <= 0) {
        fprintf(stderr, "No file specified!");
        exit(2);
      }
      filedetA[filedetN-1].mime = strdup(optarg);
      break;
    case 'h':
    case '?':
    default:
      printUsage (argv[0]);
      return -1;
    }
  }


  if (filedetN <= 0) {
    fprintf(stderr, "No file specified!");
    exit(2);
  }

  if ( !fingerPrint )  fingerPrint  = "any-binary";
  if ( !inputType )  inputType  = "binary";  
  
  
  /* connect and process */
  if ( (cloudP = mcloudCreate ("BinaryClient", MCloudModeClient)) == NULL ) {
    fprintf (stderr, "ERROR creating Cloud Object.\n");
    return -1;
  }
    
  mcloudAddFlowDescription2 (cloudP, NULL, NULL, 0, "", "send-binary", "send-binary");

  fprintf (stderr, "INFO trying to connect to %s at port %d.\n", serverHost, serverPort);
  if ( mcloudConnect (cloudP, serverHost, serverPort) != S2S_Success ) {
    fprintf (stderr, "ERROR while trying to connect.\n");
    return -1;
  }
  fprintf (stderr, "INFO connection established ==> waiting for worker accepting the request.\n");

  fprintf(stderr, "INFO outType=binary outFP=%s\n", fingerPrint);

  if ( mcloudAnnounceOutputStream (cloudP, "binary", fingerPrint, "send-binary", NULL) != S2S_Success ) {
    fprintf (stderr, "ERROR while announcing output stream.\n");
    return -1;
  }

  if ( inputStreamA ) {
    if ( !inputType ) inputType = strdup ("void");		
    for (i=0; i<inputStreamN; i++) {
      char info[2048];
      fprintf(stderr, "Requesting inputstream fp=%s type=%s\n", inputStreamA[i], inputType);
      if ( mcloudRequestInputStream (cloudP, inputType, inputStreamA[i], "send-binary", info, 2048) != S2S_Success ) {
        fprintf (stderr, "ERROR unable to request input stream.\n");
        return -1;
      }
    }
    uData.cloudP = cloudP;
    recvMessagesAsync (cloudP, &uData);
  }

  fprintf (stderr, "INFO request accepted ==> sending packages.\n");
  for (i=0; i<filedetN; i++) {
    if (mcloudSendBinaryFileAsync(cloudP, filedetA[i].f, 1024, filedetA[i].name, filedetA[i].mime, fingerPrint, NULL) != S2S_Success) {
      fprintf(stderr, "Error sending file %d\n", i+1);
    }
    fclose(filedetA[i].f);
  }
  fprintf(stderr, "INFO packages enqueued\n");

  mcloudWaitFinish (cloudP, MCloudSendingQueue, 1);

  fprintf (stderr, "INFO Waiting for worker to finish.\n");
  while ( uData.proceed ) {
    Sleep (500);
  }
  mcloudDisconnect(cloudP);
}
        
