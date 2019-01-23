#include <getopt.h>
#include <stdio.h>
#include <string.h>

#include "Platform.h"
#include "S2STime.h"
#include "MCloud.h"

static void printUsage (char *prgName) {
  printf ("\n");
  printf ("\nNAME\n\t%s - Example Client SMT.\n", prgName);
  printf ("\nSYNOPSIS\n\t%s [OPTION]... FILE[srt|ctm|txt]\n", prgName);
  printf ("\nDESCRIPTION\n"
          "\tThis client connects to Pervoice Mediator running in the cloud.\n"
          );
  printf ("\nOPTION\n"
          "\t-s, --serverHost=HOSTNAME\n\t\tHost name of the server where the Mediator is running. (demo.audioma.it)\n\n"
          "\t-p, --serverPort=PORT\n\t\tPort address at which the Mediator accepts worker. (4445)\n\n"
          "\t-f, --fingerPrint=FPRINT\n\t\tLanguage finger print of the ctm file given. (en-EU)\n\n"
          "\t-i, --inputStream=FPRINT\n\t\tLanguage finger print for the results requested. (it)\n\n"
          "\t-t, --inputType=INTYPE\n\t\tType of the results requested. (text)\n\n"
		  "\t-o, --outputType=OUTTYPE\n\t\tType of the elements sended. (text)\n\n"
          "\t-g  --gap=GAP\n\t\tMaximum silence time, in milliseconds, between two consecutive ctm records in input file. (5000)\n\n"
          "\t-w  --write=FILE\n\t\tWrite results into a file. Currently supported file formats are NIST CTM , SRT and TEXT(txt). \n\n"
		  "\t-n, --conv=ID\n\t\tConversation ID for NIST CTM file.  (conv)\n\n"	
          "\t-h, --help\n\t\tShows this help.\n\n"
          );
  printf ("\n\tVersion v1.1\n");
  printf ("\t(c) PerVoice. All rights reserved.\n");
  printf ("\t(c) PerVoice SpA. All rights reserved.\n");

  return;
}

static int windex = 0; 
static int outIndex = 1;

/** detecting whether base is starts with str
 */
int startsWith (char* base, char* str) {
    return (strstr(base, str) - base) == 0;
}

/** detecting whether base is ends with str
 */
int endsWith (char* base, char* str) {
    int blen = strlen(base);
    int slen = strlen(str);
    return (blen >= slen) && (0 == strcmp(base + blen - slen, str));
}

char* millisToSRTTime(int millis) {
  int hh = millis/3600000;
  int mm = (millis%3600000)/60000;
  int ss = (millis%60000)/1000;
  int mmm = millis%1000;

  char *str = malloc(sizeof(char) * 13);
  snprintf(str, 13, "%02d:%02d:%02d,%03d", hh, mm, ss, mmm);
  return str;
}

int srtToMills(char * str){
	int mills = 0;
	mills+=atoi(strtok(str,":")) * 3600000;
	mills+=atoi(strtok(NULL,":")) * 60000;
	char* app = strtok(NULL,":");
	mills+=atoi(strtok(app,",")) * 1000 + atoi(strtok(NULL,","));
	return mills;
} 

typedef struct recvUserData_S {
	MCloud *cloudP;
	int     proceed;
    int     uttX;
	FILE   *fp;
	char   *outFile; 	
	char   *conv;
} recvUserData;

static void writeCTM (MCloudWordToken *a, int n, FILE *fp, float from, char *utt, char *conv, int channelX) {

	float score = 0;
	int   i;

	if ( n ) {
		for (i=0; i<n; i++) score += a[i].confidence;
		score /= n;
	}

	fprintf (fp, ";; %s %f %f\n", utt, from, score);
	for (i=0; i<n; i++) {
		float beg = from + a[i].startTime/1000.0;
		float len = (from + a[i].stopTime/1000.0) - beg;
		fprintf (fp, "%s %d %7.2f %7.2f %-20s %7.2f\n",
			conv, channelX, beg, len, a[i].written, a[i].confidence);
	}
	fflush (fp);

	return;
}

static void writeSRT (char *sentence, FILE *fp, int startOffset, int stopOffset) {
	
	char  * start = millisToSRTTime(startOffset);	
	char  * stop = millisToSRTTime(stopOffset);
	fprintf (fp, "%d\n%s --> %s\n%s\n", outIndex, start, stop, strdup(sentence));//strtok(sentence,"\n"));
	outIndex++;
	fflush (fp);
	free(start);
	free(stop);
	
	return;
}

static void writeTXT (char *sentence, FILE *fp){

	fprintf (fp, "%s\n", sentence);
	fflush(fp);
	
	return;
}

int dataCallback (MCloud *cP, MCloudPacket *p, void *userData) {

	recvUserData    *ud     = (recvUserData*)userData;
	MCloud          *cloudP = ud->cloudP;
	char            *text   = NULL;
	MCloudWordToken *tokenA = NULL;
	int              tokenN = 0;
	char             utt[125];
	int              i;
  
	if ( mcloudPacketGetWordTokenA (cloudP, p, &tokenA, &tokenN) != S2S_Success ) {
		mcloudPacketGetText (cloudP, p, &text);
		
		
		if (text && text[0] != '\0' ) {
			if(ud->fp){
				if(endsWith(ud->outFile,".ctm")){
					fprintf(stderr, "INFO received text unable to save in a ctm file\n");
					fprintf (stderr, "received text from stream %s: \n", p->streamID);
					fprintf (stdout, "%s %s %s\n", text, p->start, p->stop);
				}else if(endsWith(ud->outFile,".srt")){
					writeSRT(text, ud->fp, p->startOffset, p->stopOffset);
				}else{
					writeTXT(text, ud->fp);
				}
			}else{
				fprintf (stderr, "received text from stream %s: \n", p->streamID);
				fprintf (stdout, "%s %s %s\n", text, p->start, p->stop);
			}	
		}
		free (text); text = NULL;
	} else {
		fprintf (stderr, "received tokenA: ");
		for (i=0; i<tokenN; i++) {
			fprintf (stdout, "{%s %d %d} ", tokenA[i].written, tokenA[i].startTime, tokenA[i].stopTime);
		}
		fprintf (stderr, "\n");
		
			fprintf (stderr, "\n");

		if ( ud->fp && tokenN ) {
			if(endsWith(ud->outFile,".ctm")){
				sprintf (utt, "%s_utt-%05d", ud->conv, ud->uttX++);
				writeCTM (tokenA, tokenN, ud->fp, 0, utt, ud->conv, 1);
			}			
			else{
				mcloudPacketGetText (cloudP, p, &text);
				free(text); text=NULL;
				if(endsWith(ud->outFile,".srt")){
					writeSRT(text, ud->fp, p->startOffset, p->stopOffset);
				}else {
					writeTXT(text, ud->fp);	
				}	
			}	
		}

		
		mcloudWordTokenArrayFree (tokenA, tokenN);
		tokenA = NULL; tokenN = 0;
	}

	return 0;
}

Async recvMessagesAsyncMain (void *lpParam) {
  recvUserData     *ud  = (recvUserData *)lpParam;
  MCloud       *cloudP  = (MCloud*)ud->cloudP;
  MCloudPacket *p       = NULL;

  ud->proceed = 1;
  while ( ud->proceed && (p = mcloudGetNextPacket (cloudP)) != NULL ) {
    switch (p->packetType) {
    case MCloudData:
	  mcloudProcessDataAsync (cloudP, p, ud);
      //fprintf (stderr, "INFO received DATA message ==> ignoring:\n%s\n\n", p->xmlString);
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

int sendWordTokenArray(MCloud *cP, MCloudWordToken** wordTokens, int size, const char* fingerprint, S2S_Time t){
  MCloudPacket    *np     = NULL;
  MCloudWordToken *tokenA = NULL; 	
  
  if ( (tokenA = mcloudWordTokenArrayCreate (size)) == NULL ) {
        fprintf (stderr, "ERROR Memory allocation failure.\n");
        return S2S_Error;
  }
	
  int startTime = wordTokens[0]->startTime;
  int stopTime = wordTokens[size-1]->stopTime;	
  
  S2S_Time start = t;
  S2S_Time end = t;

  s2s_AddToTime(&start, startTime, 0);
  s2s_AddToTime(&end, stopTime, 0);

  char *strStart = malloc(sizeof(char)*22);    
  char *strStop = malloc(sizeof(char)*22);    
    
  s2s_TimePrint(&start, strStart);
  s2s_TimePrint(&end, strStop);
		
  int i = 0;
 
  for (i=0; i<size; i++) {
	tokenA[i] = *wordTokens[i];
	free(wordTokens[i]);  
  }

 /* initialize packet from word token array
    Each data packet of type text, does contain at least a text string as
    content, which is in the case when a word token array is used a
    concatenation of the written forms of the tokens.
  */
  
  np = mcloudPacketInitFromWordTokenA (cP, strStart, strStop, startTime, stopTime, fingerprint, tokenA, size);
	
   
  /* send packet */
  fprintf(stderr, "INFO Sending packet.\n"); 	
  if(mcloudSendPacketAsync (cP, np, NULL) != S2S_Success) {
	fprintf(stderr, "ERROR Error sending data.\n");
  }
  mcloudWordTokenArrayFree (tokenA, size);	
  free(strStop);
  free(strStart);	
  
  return S2S_Success;
} 

void sendCTMFile(MCloud *cloudP, char* ctmFileName, MCloudPacketCallbackFct* _dataCallback, char* fingerPrint, int gap) {
  FILE    *ctmFile = fopen(ctmFileName, "rb"); 	
  char * line = NULL;
  size_t len = 0;
  ssize_t read;
  if(ctmFile==NULL){
   	fprintf(stderr, "ERROR unable to open file %s\n", ctmFileName);
	exit(1);
  }
  
  mcloudSetDataCallback(cloudP, _dataCallback);
  MCloudWordToken** wordTokens = (MCloudWordToken**) malloc (sizeof(MCloudWordToken**) * 200);
  int n = 0;
  int stopMS = +10000000;	
  
  S2S_Time t;
  s2s_TimeInit(&t);
  s2s_GetSystemTime (&t);   
  
  while((read = getline(&line, &len, ctmFile)) != -1) {
		if(!startsWith(line, ";;") && line[0] != '\n' && line[0] != '\0'){
				if(strtok(line," ")==NULL) continue;  //waveform filename	
				if(strtok(NULL," ")==NULL) continue; //waveform channel
				char* startTime = strtok(NULL," ");
				if(startTime==NULL) continue;
				char* duration = strtok(NULL," ");
				if(duration==NULL) continue;
				char* word = strtok(NULL," ");
				if(word==NULL) continue;
				char* confidence = strtok(NULL," ");
				int startMS = atof(startTime) * 1000;
				if((startMS - stopMS) > gap){
					if(sendWordTokenArray(cloudP, wordTokens, n, fingerPrint, t)!=S2S_Success) exit(1); 
					n =  0;	
				} 
				stopMS = startMS + ( atof(duration) * 1000);
				MCloudWordToken* wordToken = (MCloudWordToken*) malloc (sizeof(MCloudWordToken));
				wordToken->index      = windex++;       /* continuously increasing index */
       			wordToken->internal   = strdup(word);
                wordToken->written    = strdup(word);
       			wordToken->spoken     = strdup("optional");
				if(confidence!=NULL) wordToken->confidence = atof(confidence);
				else wordToken->confidence = 1.0;
        			wordToken->startTime  = startMS; /* start time of word in stream in ms */
        			wordToken->stopTime   = stopMS;  /* end time of word in stream in ms */
        			wordToken->isFiller   = 0;       /* flag to indicate whether word is a filler word, such as noises */
				wordTokens[n]=wordToken;
				n++;
		}
  }

  if(n!=0){
	if(sendWordTokenArray(cloudP, wordTokens, n, fingerPrint, t)!=S2S_Success) exit(1); 
  }	
  
  fclose(ctmFile);
}

void sendSRTFile(MCloud *cloudP, char* srtFileName, MCloudPacketCallbackFct* _dataCallback, char* fingerPrint) {
	FILE    *srtFile = fopen(srtFileName, "rb"); 	
	char  line[256];
	
	if(srtFile==NULL){
		fprintf(stderr, "ERROR unable to open file %s\n", srtFileName);
		exit(1);
	}

	mcloudSetDataCallback(cloudP, _dataCallback);
	
	MCloudPacket *p;
	
	S2S_Time t;
	s2s_TimeInit(&t);
	s2s_GetSystemTime (&t);
	
	while(!feof(srtFile)) {
		if (fgets(line,256,srtFile)) {
			if(line[0] != '\n' && line[0] != '\0'){
				//int index = atoi(line);
				//fprintf(stderr, "INFO index %d ", index);
				fgets(line,256,srtFile);
				if(feof(srtFile)){
					fprintf(stderr, "\nERROR\n"); 
					exit(1);
				}
				
				char* appStartTime = strtok(line," ");
				char* startTime = strdup(appStartTime);
				strtok(NULL," ");
				char* appStopTime = strtok(NULL," ");
				char* stopTime = strdup(strtok(appStopTime, "\n"));
						
				int startMS = srtToMills(strdup(startTime));
				int stopMS = srtToMills(strdup(stopTime)); 
				
				S2S_Time start = t;
				S2S_Time stop = t;

				s2s_AddToTime(&start, startMS, 0);
				s2s_AddToTime(&stop, stopMS, 0);

				char *strStart = malloc(sizeof(char)*22);    
				char *strStop = malloc(sizeof(char)*22);    
    
				s2s_TimePrint(&start, strStart);
				s2s_TimePrint(&stop, strStop);
				
				//fprintf(stderr, "startTime: %s stopTime: %s startoffset %d stopoffset %d ", strStart, strStop, startMS, stopMS);
				char sentence[200];
				strcpy (sentence,"");
				fgets(line,256,srtFile);
				while(!feof(srtFile)  && line[0] != '\n' && line[0] != '\0') {
					strcat(sentence, line);
					fgets(line,256,srtFile);
				}
				//fprintf(stderr, "sentence : %s\n", sentence);
				p = mcloudPacketInitFromText (cloudP, strStart, strStop, startMS, stopMS, fingerPrint, sentence);
				mcloudSendPacketAsync (cloudP, p, NULL);
				free(strStart);
				free(strStop);
			}
		}
	}
	
	fclose(srtFile);
}  

void sendTXTFile(MCloud *cloudP, char* txtFileName, MCloudPacketCallbackFct* _dataCallback, char* fingerPrint) {
	FILE    *txtFile = fopen(txtFileName, "rb"); 	
	char  line[256];
	
	if(txtFile==NULL){
		fprintf(stderr, "ERROR unable to open file %s\n", txtFileName);
		exit(1);
	}

	mcloudSetDataCallback(cloudP, _dataCallback);
	
	MCloudPacket *p;
	
	S2S_Time t;
	s2s_TimeInit(&t);
	s2s_GetSystemTime (&t);
	
	int startMS = 0;
	
	while(!feof(txtFile)) {
		if (fgets(line,256,txtFile)) {
			//fprintf(stderr, "INFO line %s \n", line);
			S2S_Time start = t;
			S2S_Time stop = t;

			s2s_AddToTime(&start, startMS, 0);
			s2s_AddToTime(&stop, startMS+1000, 0);
			
			char *strStart = malloc(sizeof(char)*22);    
			char *strStop = malloc(sizeof(char)*22);    
    
			s2s_TimePrint(&start, strStart);
			s2s_TimePrint(&stop, strStop);
			
			p = mcloudPacketInitFromText (cloudP, strStart, strStop, startMS, startMS+1000, fingerPrint, strtok(line,"\n"));
			mcloudSendPacketAsync (cloudP, p, NULL);
			free(strStart);
			free(strStop);
			startMS = startMS+1001;
		}
	}
	fclose(txtFile);
}

int main (int argc, char * argv[]) {

  char *serverHost    = NULL;
  int   serverPort    = 4445;
  int   optX, o, i;

  char    *fingerPrint  = NULL;
  char   **inputStreamA = NULL;
  int      inputStreamN = 0;
  char    *inputType    = NULL;
  char	  *outputType	= NULL;		
  char	  *fileName = NULL;
 

  char    *conv         = NULL;
  char    *outFile   = NULL;
  
  int gap = -1;	

  MCloud         *cloudP = NULL;
  
  MCloudPacketCallbackFct *_dataCallback = dataCallback;
  recvUserData uData;

  static struct option lopt[] = {
    {"serverHost", 1, NULL, 's'},
    {"serverPort", 1, NULL, 'p'},
    {"fingerPrint", 1, NULL, 'f' },
    {"inputStream", 1, NULL, 'i'},
    {"inputType", 1, 0, 't'},
    {"outputType", 1, 0, 'o'},
    {"gap", 1, 0, 'g'},
    {"conv", 1, 0, 'n'},
    {"write", 1, 0, 'w'},	
    { "help", 0, 0, 'h' },  
    {NULL, 0, 0, 0}
  };

  /* command line parsing */
  if ( argc < 0 ) {
    printUsage (argv[0]);
    return -1;
  }

  while ( (o = getopt_long (argc, argv, "s:p:f:i:t:o:g:w:n:h", lopt, &optX)) != -1 ) {
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
	case 'o':
	  outputType = strdup (optarg);
	  break;	
    case 'g':
	  gap = atoi(optarg);
	  break;
    case 'w':
	  outFile = strdup (optarg);
	  break;
    case 'n':
	  conv = strdup (optarg);
	  break;		
    case 'h':
    case '?':
    default:
      printUsage (argv[0]);
      return -1;
    }
  }


  fileName = strdup (argv[optind]);

  if(fileName==NULL){ 
	fprintf(stderr, "ERROR INPUT FILE NOT SPECIFIED\n");	
  	exit(1);
  }

  if ( !serverHost )  serverHost = strdup ("demo.audioma.it");
  if ( !serverPort )  serverPort  = 4445;
  if ( !fingerPrint )  fingerPrint  = "en-EU";
  if ( !inputType )  inputType  = "text";  
  if ( !outputType ) outputType = "text";
  if ( !inputStreamA ) {
		i = inputStreamN++;
		inputStreamA = realloc (inputStreamA, inputStreamN * sizeof (char*));
		inputStreamA[i] = strdup ("it");
  }
  if (gap==-1) gap = 5000;
 
  
  /* connect and process */
  if ( (cloudP = mcloudCreate ("CMT-SRTClient", MCloudModeClient)) == NULL ) {
    fprintf (stderr, "ERROR creating Cloud Object.\n");
    return -1;
  }
    
  mcloudAddFlowDescription2 (cloudP, NULL, NULL, 0, "", "send-ctm-srt", "send-ctm-srt");

  fprintf (stderr, "INFO trying to connect to %s at port %d.\n", serverHost, serverPort);
  if ( mcloudConnect (cloudP, serverHost, serverPort) != S2S_Success ) {
    fprintf (stderr, "ERROR while trying to connect.\n");
    return -1;
  }
  fprintf (stderr, "INFO connection established ==> waiting for worker accepting the request.\n");

  fprintf(stderr, "INFO outType=%s outFP=%s\n", outputType, fingerPrint);

  if ( mcloudAnnounceOutputStream (cloudP, outputType, fingerPrint, "send-ctm-srt", NULL) != S2S_Success ) {
    fprintf (stderr, "ERROR while announcing output stream.\n");
    return -1;
  }
  
  uData.cloudP = cloudP;
  uData.fp     = NULL;
  uData.conv      = (conv) ? strdup(conv) : strdup("conv");
  uData.uttX      = 0;

  if ( outFile ) {
      uData.outFile = outFile;	  
      if ( (uData.fp = fopen (outFile, "w")) == NULL ) {
		fprintf (stderr, "ERROR opening file for writing: %s.\n", outFile);
		return -1;
	}
  }

  if ( inputStreamA ) {
    if ( !inputType ) inputType = strdup ("text");		
    for (i=0; i<inputStreamN; i++) {
      char info[2048];
      fprintf(stderr, "INFO Requesting inputstream fp=%s type=%s\n", inputStreamA[i], inputType);
      if ( mcloudRequestInputStream (cloudP, inputType, inputStreamA[i], "send-ctm-srt", info, 2048) != S2S_Success ) {
        fprintf (stderr, "ERROR unable to request input stream.\n");
        return -1;
      }
    }	
    
    recvMessagesAsync (cloudP, &uData);//receive message async
  }

  fprintf (stderr, "INFO request accepted ==> sending packages.\n");
  
  //send packet 
  if(endsWith(fileName,".ctm"))
	sendCTMFile(cloudP, fileName, _dataCallback, fingerPrint, gap);
  else if(endsWith(fileName,".srt")){
		//fprintf(stderr, "INFO srt file in input\n");
		sendSRTFile(cloudP, fileName, _dataCallback, fingerPrint); 	
	}
  else if(endsWith(fileName,".txt")){
		//fprintf(stderr, "INFO txt file in input\n");
		sendTXTFile(cloudP, fileName, _dataCallback, fingerPrint);
  }
  else{
	fprintf(stderr, "ERROR unsupported input file type\n");
	exit(1);
  }  
  
  fprintf(stderr, "INFO packages enqueued\n");

  mcloudWaitFinish (cloudP, MCloudSendingQueue, 1);

  fprintf (stderr, "INFO Waiting for worker to finish.\n");
  while ( uData.proceed ) {
    Sleep (500);
  }
  mcloudDisconnect(cloudP);
  if(uData.fp!=NULL) fclose(uData.fp);
  return 0;
}
        
