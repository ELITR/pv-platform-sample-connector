/*
 * exampleClientBidir.c
 *
 *  Created on: Jan 23, 2014
 *      Author: PerVoice
 *      (c) 2011 PerVoice
 */

#include <getopt.h>
#include <stdio.h>
#include <string.h>

#include "Platform.h"
#include "S2STime.h"
#include "MCloud.h"


// https://stackoverflow.com/questions/2347770/how-do-you-clear-the-console-screen-in-c
#include <unistd.h>
void clearScreen()
{
  const char *CLEAR_SCREEN_ANSI = "\e[1;1H\e[2J";
  write(STDOUT_FILENO, CLEAR_SCREEN_ANSI, 12);
}


// my extension
int interactive = 0;


typedef struct MCloudQueue_S    MCloudQueue;

/* =========================================================================
 * Type Definitions
 * ========================================================================= */


/* =========================================================================
 * Helper functions
 * ========================================================================= */
static void printUsage (char *prgName) {
	printf ("\n");
	printf ("\nNAME\n\t%s - Example Client.\n", prgName);
	printf ("\nSYNOPSIS\n\t%s [OPTION]... AUDIOFILE\n", prgName);
	printf ("\nDESCRIPTION\n"
		"\tThis client connects to Pervoice Mediator running in the cloud.\n"
		);
	printf ("\nOPTION\n"
		"\t-s, --serverHost=HOSTNAME\n\t\tHost name of the server where the Mediator is running. (demo.audioma.it)\n\n"
		"\t-p, --serverPort=PORT\n\t\tPort address at which the Mediator accepts worker. (8080)\n\n"
		"\t-f, --fingerPrint=FPRINT\n\t\tLanguage finger print of the audio file given. (en-EU)\n\n"
		"\t-i, --inputStream=FPRINT\n\t\tLanguage finger print for the results requested. (en)\n\n"
		"\t-t, --inputType=FPRINT\n\t\tType of the results requested. (text)\n\n"
		"\t-l, --logging\n\t\tTurn on/off logging of submitted data.\n\n"
		"\t-w, --writeCTM=FILE\n\t\tWrite results into a NIST CTM file.\n\n"
		"\t-n, --conv=ID\n\t\tConversation ID for NIST CTM file.\n\n"
		"\t-C, --codec=CODEC\n\t\tAudio codec used to transmit and receive data to the Mediator.\n\t\tCurrently supported codecs are OPUS, SPEEX and FLAC.\n\n"
		"\t-S, --sampleRate=SRATE\n\t\tSample rate used to transmit and receive data to the Mediator.\n\n"
		"\t-B, --bitRate=BRATE\n\t\tSample rate used to transmit and receive data to the Mediator.\n\n"		
		//      "\t-v, --verbose\n\t\tVerbose output.\n\n"
		"\t-x, --plaintxt\n\t\tProvides output in plain text.\n\n"
		"\t-T, --ssl\n\t\tProvides ssl connection to Mediator.\n\n"
		"\t-W, --selfSigned\n\t\tAccept self signed certify from Server.\n\n"
		"\t-I, --interactive\n\t\tInteractive mode. Shows only the subtitles.\n\n"
		"\t-h, --help\n\t\tShows this help.\n\n"
		);
	printf ("\n\tVersion v1.1\n");
	printf ("\t(c) PerVoice. All rights reserved.\n");
	printf ("\t(c) PerVoice SpA. All rights reserved.\n");

	return;
}

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
typedef struct recvUserData_S {
	MCloud *cloudP;
	void   *userData;
	int     proceed;
	int     uttX;
	FILE   *fpCTM;
	char   *conv;
	int     verbose;
	char   *startTime;
        int    isNum;
} recvUserData;

static void writeTEXT(MCloudWordToken *a, int n, FILE *fp, float from, char *utt, char *conv, int channelX) {

	float score = 0;
	int   i;

	if (n) {
		for (i = 0; i<n; i++) score += a[i].confidence;
		score /= n;
	}

	for (i = 0; i<n; i++) {
		float beg = from + a[i].startTime / 1000.0;
		float len = (from + a[i].stopTime / 1000.0) - beg;
		fprintf(fp, "%s\n", a[i].written);
	}
	fflush(fp);

	return;
}

static void writeCTM (MCloudWordToken *a, int n, FILE *fp, float from, char *utt, char *conv, int channelX) {

	float score = 0;
	int   i;

	if ( n ) {
		for (i=0; i<n; i++) score += a[i].confidence;
		score /= n;
	}

	fprintf (fp, "# %s %f %f\n", utt, from, score);
	for (i=0; i<n; i++) {
		float beg = from + a[i].startTime/1000.0;
		float len = (from + a[i].stopTime/1000.0) - beg;
		fprintf (fp, "%s %d %7.2f %7.2f %-20s %7.2f\n",
			conv, channelX, beg, len, a[i].written, a[i].confidence);
	}
	fflush (fp);

	return;
}

#ifdef USE_TCL
extern char *Tcl_lindex(char *item, const char *list, int index);
extern int Tcl_llength(const char *list);

static void writeCTM2 (const char *text, unsigned int offset, unsigned int duration, FILE *fp, char *utt, char *conv, int channelX) {

	int   n   = Tcl_llength (text);
	float beg = offset;
	float len;
	char  item[1024];
	int   i;

	fprintf (stderr, "%s %d %d: %s\n", utt, offset, duration, text);

	fprintf (fp, "# %s %f %f\n", utt, offset/1000.0, 0.0);

	if ( !n ) return;
	len = (float)duration / (float)n;

	for (i=0; i<n; i++) {
		Tcl_lindex (item, text, i);
		fprintf (fp, "%s %d %7.2f %7.2f %-20s %7.2f\n",
			conv, channelX, beg/1000.0, len/1000.0, item, 1.0);
		beg += len;
	}
	fflush (fp);

	return;
}
#endif

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
		if (!interactive)
			fprintf (stderr, "received text from stream %s: %s\n", p->streamID, text);
		else {
//			clearScreen();
			fprintf (stderr, "%s\n", text);
		}

		if ( ud->fpCTM && text && text[0] != '\0' ) {
			S2S_Time startT;
			S2S_Time stopT;
			S2S_Time refT;
			unsigned int offset, duration;

			parseTime (p->start, &startT);
			parseTime (p->stop, &stopT);
			parseTime (ud->startTime, &refT);

			offset   = s2s_TimeDuration (&refT, &startT);
			duration = s2s_TimeDuration (&startT, &stopT);

			sprintf (utt, "%s_utt-%05d", ud->conv, ud->uttX++);
			/*
			static void writeCTM (MCloudWordToken *a, int n, FILE *fp, float from, char *utt, char *conv, int channelX) {
			*/
			if (!interactive)
#ifdef USE_TCL
				writeCTM2(text, offset, duration, ud->fpCTM, utt, ud->conv, 1);
#else
				writeCTM(tokenA, tokenN, ud->fpCTM, 0, utt, ud->conv, 1);
#endif
		}
		free (text); text = NULL;
	} else {
		fprintf (stderr, "received tokenA: ");
		for (i=0; i<tokenN; i++) {
			fprintf (stderr, "{%s %d %d} ", tokenA[i].written, tokenA[i].startTime, tokenA[i].stopTime);
		}
		fprintf (stderr, "\n");

		if ( ud->fpCTM && tokenN ) {
			sprintf (utt, "%s_utt-%05d", ud->conv, ud->uttX++);
			writeCTM (tokenA, tokenN, ud->fpCTM, 0, utt, ud->conv, 1);
		}

		mcloudWordTokenArrayFree (tokenA, tokenN);
		tokenA = NULL; tokenN = 0;
	}

	return 0;
}

int dataCallbackPlainText(MCloud *cP, MCloudPacket *p, void *userData) {

	recvUserData    *ud = (recvUserData*)userData;
	MCloud          *cloudP = ud->cloudP;
	char            *text = NULL;
	MCloudWordToken *tokenA = NULL;
	int              tokenN = 0;
	char             utt[125];
	int              i;

	if (mcloudPacketGetWordTokenA(cloudP, p, &tokenA, &tokenN) != S2S_Success) {
		mcloudPacketGetText(cloudP, p, &text);

		if (ud->fpCTM && text && text[0] != '\0') {
			S2S_Time startT;
			S2S_Time stopT;
			S2S_Time refT;
			unsigned int offset, duration;

			parseTime(p->start, &startT);
			parseTime(p->stop, &stopT);
			parseTime(ud->startTime, &refT);

			offset = s2s_TimeDuration(&refT, &startT);
			duration = s2s_TimeDuration(&startT, &stopT);

			sprintf(utt, "%s_utt-%05d", ud->conv, ud->uttX++);
			writeTEXT(tokenA, tokenN, ud->fpCTM, 0, utt, ud->conv, 1);
		}

		free(text); text = NULL;
	}
	else {
		if (ud->fpCTM && tokenN) {
			sprintf(utt, "%s_utt-%05d", ud->conv, ud->uttX++);
			writeTEXT(tokenA, tokenN, ud->fpCTM, 0, utt, ud->conv, 1);
		}

		mcloudWordTokenArrayFree(tokenA, tokenN);
		tokenA = NULL; tokenN = 0;
	}

	return 0;
}

int finalizeCallback(MCloud *cP, void *userData) {

	fprintf(stderr, "In finalizeCallback");
	//recvUserData *ud     = (recvUserData*)userData;
	//MCloud       *cloudP = ud->cloudP;
	return 0;
}

int breakCallback (MCloud *cP, void *userData) {
	fprintf(stderr, "In finalizeCallback");
	//recvUserData *ud     = (recvUserData*)userData;
	//MCloud       *cloudP = ud->cloudP;
	return 0;
}

int errorCallback (MCloud *cP, void *userData) {
	fprintf(stderr, "In finalizeCallback");
	//recvUserData *ud     = (recvUserData*)userData;
	//MCloud       *cloudP = ud->cloudP;
	return 0;
}


/* =========================================================================
 * Asyncronous parallel polling for new data
 * ========================================================================= */
Async recvMessagesAsyncMain (void *lpParam) {

	recvUserData *ud      = (recvUserData*)lpParam;
	MCloud       *cloudP  = (MCloud*)ud->cloudP;
	MCloudPacket *p       = NULL;
	int           err     = 0;

	ud->proceed = 1;
	while ( ud->proceed && (p = mcloudGetNextPacket (cloudP)) != NULL ) {
		switch (p->packetType) {
		case MCloudData:
			mcloudProcessDataAsync (cloudP, p, ud);
			break;
		case MCloudFlush:
			mcloudPacketDeinit (p);
			break;
		case MCloudDone:
                        fprintf (stderr, "INFO received DONE message.\n");
                        if (--ud->isNum <= 0) {
                                fprintf (stderr, "INFO ALL DONE messages received ==> end thread.\n");
         			mcloudWaitFinish (cloudP, MCloudProcessingQueue, 1);
		        	ud->proceed = 0;
                        }
						mcloudPacketDeinit (p);
			break;

		case MCloudError:
			mcloudBreak (cloudP, MCloudProcessingQueue);
			fprintf (stderr, "INFO received ERROR message: %s ==> end thread.\n", p->statusDescription);
			mcloudPacketDeinit (p);
			ud->proceed = 0;
			break;
		case MCloudReset:
			mcloudBreak (cloudP, MCloudProcessingQueue);
			fprintf (stderr, "INFO received RESET message: %s ==> end thread.\n", p->statusDescription);
			mcloudPacketDeinit (p);
			ud->proceed = 0;
			break;
		default:
			fprintf (stderr, "ERROR unknown packet type %d\n", p->packetType);
			mcloudPacketDeinit (p);
			ud->proceed = 0;
			err = 1;
		}
	}

	ReturnAsync (NULL);
}

int recvMessagesAsync (MCloud *cloudP, recvUserData *userData) {

	HANDLE hThread;

	CallAsync (hThread, recvMessagesAsyncMain, (void*)userData);

	return S2S_Success;
}



/* =========================================================================
 * Main Function
 * ========================================================================= */
int main (int argc, char * argv[]) {

	char *serverHost    = NULL;
	int   serverPort    = 0;
	char *username = NULL;
	char *password = NULL;
	int   optX, o, i;

	char    *audioFile    = NULL;
	char    *fingerPrint  = NULL;
	char    *type         = NULL;
	char   **inputStreamA = NULL;
	int      inputStreamN = 0;
	char    *inputType    = NULL;
	char    *ctmFile      = NULL;
	char    *conv         = NULL;
	char     startTime[128];
	char     stopTime[128];

	FILE       *afp        = NULL;
	short      *buffer     = NULL;
	int         bufferN    = 0;
	int         pos        = 0;
	char        riff[]     = "RIFF";
	int         sampleRate = 16000;
	int         chunkSize  = 0.256 * sampleRate; /* 0.5 seconds */
	int         sampleN    = 0;
	int         logging    = 0;
	int		realtimemode = 0;
	int		streamSimulation = 0;

	short		*mybuffer;
	size_t readBytes;

	char 		 *o_codec = "RPCM";
	int 		o_channels = 1;
	int 		o_sampleRate = 16000;
	int 		o_bitRate = 32000;
        int		ssl = 0; 
        MCloudSSLVerifyMode  verifyMode = MCloudSSL_FullVerify;


	MCloud         *cloudP = NULL;
	MCloudPacket   *p      = NULL;

	MCloudPacketCallbackFct *_dataCallback = dataCallback;

	recvUserData recvUserData;

	S2S_Time t;
	S2S_Time startT;
	S2S_Time stopT;


	static struct option lopt[] = {
		{"serverHost", 1, NULL, 's'},
		{"serverPort", 1, NULL, 'p'},
		{"username", 1, NULL, 'U' },
		{"password", 1, NULL, 'P' },
		{"fingerPrint", 1, NULL, 'f' },
		{"inputStream", 1, NULL, 'i'},
		{"inputType", 1, 0, 't'},
		{"logging", 1, 0, 'l'},
		{"writeCTM", 1, 0, 'w'},
		{"conv", 1, 0, 'n'},
		{"plaintxt", 0, 0, 'x' },
		{"realtime", 0, 0, 'r' },
		{"codec", 1, NULL, 'C'},	
		{"bitRate", 1, NULL, 'B'},
		{"sampleRate", 1, NULL, 'S'},
		{"ssl", 0, 0, 'T'},
		{"selfSigned", 0, 0, 'W'},		
		{ "help", 0, 0, 'h' },
		// my extension
		{"interactive", 0, 0, 'I'},
		{NULL, 0, 0, 0}
	};

	/* command line parsing */
	if ( argc < 0 ) {
		printUsage (argv[0]);
		return -1;
	}



	while ( (o = getopt_long (argc, argv, "s:p:U:P:f:i:t:l:w:n:xrhIC:B:S:TW", lopt, &optX)) != -1 ) {
		switch (o) {
		case 's':
			serverHost = strdup (optarg);
			break;
		case 'p':
			serverPort = atoi (optarg);
			break;
		case 'U':
			username = strdup(optarg);
			break;
		case 'P':
			password = strdup(optarg);
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
		case 'l':
			logging = atoi (optarg);
			break;
		case 'w':
			ctmFile = strdup (optarg);
			break;
		case 'n':
			conv = strdup (optarg);
			break;
		case 'x':
			_dataCallback = dataCallbackPlainText;
			break;
		case 'r':
			streamSimulation = 1;
			break;
		case 'C':
			o_codec = strdup (optarg);
			break;
		case 'S':
			o_sampleRate = atoi (optarg);
			break;
		case 'B':
			o_bitRate = atoi (optarg);
			break;
		case 'T':
			ssl = 1;
			break;
		case 'W':
			verifyMode = MCloudSSL_AcceptSelfSigned;
			break;			
		case 'I':
			interactive = 1;
			break;
		case 'h':
		case '?':
		default:
			printUsage (argv[0]);
			return -1;
		}
	}

	if ( !serverHost )  serverHost = strdup ("demo.audioma.it");
	if ( !serverPort )  serverPort  = 4445;
	if (!username)  username = strdup("SA");
	if (!password)  password = strdup("SA");
	if (!fingerPrint) fingerPrint = strdup("en-EU-lecture");
	if ( !inputStreamA ) {
		i = inputStreamN++;
		inputStreamA = realloc (inputStreamA, inputStreamN * sizeof (char*));
		inputStreamA[i] = strdup ("en");
	}
	if ( !inputType )   inputType   = strdup ("text");

	if ( !fingerPrint ) {
		printUsage (argv[0]);
		return -1;
	}

	/*read audio file and evaluate if it's a streaming input*/
	if(argv[optind]==NULL){audioFile="-";}
	else audioFile = strdup (argv[optind]);
	type      = strdup ("audio");
	if((strcmp("stdin",audioFile)==0)||(strcmp("-",audioFile)==0)||(audioFile==NULL)){
		realtimemode=1;
	}

	s2s_TimeInit (&t);



	/* process audio file */
	if(realtimemode==0){

		if ( !(afp = fopen (audioFile, "rb")) ) {
			perror ("ERROR fopen:");
			return -1;
		}

		pos = 0;
		while ( !feof(afp) ) {
			if ( pos + chunkSize > bufferN ) {
				if ( !(buffer = realloc (buffer, (bufferN+chunkSize) * sizeof(short))) ) {
					fprintf (stderr, "ERROR memory allocation failure.\n");
					return -1;
				}
				bufferN += chunkSize;
			}
			pos += fread (buffer+pos, sizeof(short), chunkSize, afp);
		}
		fclose (afp);

		/* skip WAV header */
		sampleN = pos;
		pos     = 0;
		if ( ((int*)buffer)[0] == ((int*)riff)[0] ) pos += 22;
	} else {
		fprintf( stderr, "INFO Reading audio from stdin\n");
#ifdef _WIN32 
		if( _setmode ( _fileno ( stdin ), O_BINARY ) == -1 ) {
			perror ( "Cannot set stdin to binary mode" );
		} else {
			fprintf( stderr, "stdin mode successfully set to binary\n" );
		}
#endif
	}


	/*if(!ssl){
		cloudP = mcloudCreate2 ("ExampleClient", MCloudModeClient, username, password)			
	}else{
		cloudP = mcloudCreate2SSL ("ExampleClient", MCloudModeClient, username, password, verifyMode)
	}*/

	cloudP = ssl ? mcloudCreate2SSL ("ExampleClient", MCloudModeClient, username, password, verifyMode) : mcloudCreate2 ("ExampleClient", MCloudModeClient, username, password);


	/* connect and process */
	if ( cloudP == NULL ) {
		fprintf (stderr, "ERROR creating Cloud Object.\n");
		return -1;
	}
	
	
	if(mcloudSetAudioEncoder2 (cloudP, o_codec, o_sampleRate, o_bitRate, o_channels) != S2S_Success )
	{
		fprintf (stderr, "ERROR setting encoder.\n");
		return -1;
	}

	mcloudAddFlowDescription2 (cloudP, username, password, logging, "Language", "Lecture-Name", "Lecture-Description");

	fprintf (stderr, "INFO trying to connect to %s at port %d.\n", serverHost, serverPort);

	if ( mcloudConnect (cloudP, serverHost, serverPort) != S2S_Success ) {
		fprintf (stderr, "ERROR while trying to connect.\n");
		return -1;
	}
	fprintf (stderr, "INFO connection established ==> waiting for worker accepting the request.\n");

	if ( mcloudAnnounceOutputStream (cloudP, type, fingerPrint, "speech", NULL) != S2S_Success ) {
		fprintf (stderr, "ERROR while announcing output stream.\n");
		return -1;
	}

	if ( mcloudRequestForDisplay (cloudP) != S2S_Success ) {
		fprintf (stderr, "ERROR while requesting for display.\n");
		return -1;
	}

	/* install thread for receiving data in parallel */
	recvUserData.cloudP    = cloudP;
	recvUserData.userData  = NULL;
	recvUserData.proceed   = 1;
	recvUserData.fpCTM     = NULL;
	recvUserData.conv      = (conv) ? strdup(conv) : strdup("conv");
	recvUserData.uttX      = 0;
	recvUserData.verbose   = 1;
	recvUserData.startTime = NULL;
        recvUserData.isNum = inputStreamN;

	if ( ctmFile ) {
		if ( (recvUserData.fpCTM = fopen (ctmFile, "w")) == NULL ) {
			fprintf (stderr, "ERROR opening file for writing: %s.\n", ctmFile);
			return -1;
		}
	}
	else {
		recvUserData.fpCTM = stdout;
	}

	mcloudSetDataCallback(cloudP, _dataCallback);
	mcloudSetFinalizeCallback (cloudP, finalizeCallback, &recvUserData);
	mcloudSetErrorCallback (cloudP, MCloudSendingQueue, errorCallback, &recvUserData);
	mcloudSetBreakCallback (cloudP, MCloudSendingQueue, breakCallback, &recvUserData);

	if ( inputStreamA ) {
		if ( !inputType ) inputType = strdup ("speech");
		for (i=0; i<inputStreamN; i++) {
			char info[2048];
			if ( mcloudRequestInputStream (cloudP, inputType, inputStreamA[i], "speech", info, 2048) != S2S_Success ) {
				fprintf (stderr, "ERROR unable to request input stream.\n");
				return -1;
			}
			fprintf(stderr, "INFO: %s\n", info);
		}
		mcloudSetErrorCallback (cloudP, MCloudProcessingQueue, errorCallback, &recvUserData);
		mcloudSetBreakCallback (cloudP, MCloudProcessingQueue, breakCallback, &recvUserData);
		recvMessagesAsync (cloudP, &recvUserData);
	}

	/* In order to be able to select an output stream in the display server, we wait for a key press
	* until the data will be sent */
	fprintf (stderr, "INFO request accepted ==> sending packages.\n");
	//  fprintf (stderr, "INFO Press return to continue: ...\n");
	//  getchar();

	/* simulate correct time stamps */
	s2s_GetSystemTime (&t);
	timePrint (&t, startTime);
	recvUserData.startTime = strdup (startTime);

	s2s_GetSystemTime (&startT);
	fprintf (stderr, "INFO Sending data.\n");
	if(realtimemode==0) {
		while ( pos + chunkSize < sampleN ) {
			s2s_AddToTime (&t, (float)chunkSize/(float)sampleRate*1000.0, 0);
			timePrint (&t, stopTime);
			p = mcloudPacketInitFromAudio (cloudP, startTime, stopTime, fingerPrint, buffer+pos, chunkSize, (pos+chunkSize == sampleN));
			if (mcloudSendPacketAsync(cloudP, p, NULL) != S2S_Success) {
				fprintf(stderr, "ERROR Error sending data.\n");
			}
			pos += chunkSize;
			timePrint (&t, startTime);
			if(streamSimulation) {
				Sleep((float)chunkSize / (float)sampleRate*1000.0);
			}
		}

		if ( sampleN - pos != 0 ) {
			s2s_AddToTime (&t, (float)(sampleN-pos)/(float)sampleRate*1000.0, 0);
			timePrint (&t, stopTime);
			p = mcloudPacketInitFromAudio (cloudP, startTime, stopTime, fingerPrint, buffer+pos, sampleN - pos, 1);
			if (mcloudSendPacketAsync(cloudP, p, NULL) != S2S_Success) {
				fprintf(stderr, "ERROR Error sending data.\n");
			}
		}
	} else{
		mybuffer = (void*) malloc((chunkSize) * sizeof(short));

		while((readBytes = fread (mybuffer, sizeof(short), chunkSize, stdin))>0){
			s2s_AddToTime (&t, (float)chunkSize/(float)sampleRate*1000.0, 0);
			timePrint (&t, stopTime);
			p = mcloudPacketInitFromAudio (cloudP, startTime, stopTime, fingerPrint, (short*)mybuffer, readBytes, (readBytes == chunkSize));

			if (mcloudSendPacketAsync(cloudP, p, NULL) != S2S_Success) {
				fprintf(stderr, "ERROR Error sending data.\n");
			}

			timePrint (&t, startTime);
			if (streamSimulation) {
				Sleep((float)chunkSize / (float)sampleRate*1000.0);
			}
		}
	}


	mcloudWaitFinish (cloudP, MCloudSendingQueue, 1);

	fprintf (stderr, "INFO Waiting for worker to finish.\n");
	while ( recvUserData.proceed ) {
		Sleep (500);
	}
	s2s_GetSystemTime (&stopT);

	timePrint (&startT, startTime);
	timePrint (&stopT, stopTime);
	fprintf (stderr, "INFO DONE: %s to %s -> duration %d\n", startTime, stopTime, s2s_TimeDuration (&startT, &stopT));

	mcloudDisconnect (cloudP);

	if ( ctmFile ) {
		fclose (recvUserData.fpCTM);
	}

	/* free */
	mcloudFree (cloudP);

	free(buffer); buffer = NULL;

	for (i=0; i<inputStreamN; i++) free(inputStreamA[i]);
	free (inputStreamA);
	free (inputType);

	return 0;
}

