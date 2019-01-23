/*
 * MCloud.h
 *
 * Created by Dario Franceschini on 11.01.2012.
 * Copyright (c) 2012 PerVoice S.p.A. . All rights reserved.
 */

#ifndef MCloud_h
#define MCloud_h

#ifdef __cplusplus
extern "C" {
#endif

#include <libxml/tree.h>

#ifndef __JIBBIGO_COMMON_INCLUDED__

#if defined(_USRDLL) /* this is compiled as DLL */
#define SHAREDDLL __declspec(dllexport)
#else
#define SHAREDDLL
#endif

typedef enum {
  S2S_Success =  0,          /**< success */
  S2S_Error   =  1           /**< error */
} S2S_Result;

#endif

/* ======================================================================
   Type Definitions
   ====================================================================== */
typedef enum {
  MCloudModeWorker = 1,         /**< worker mode */
  MCloudModeClient,             /**< client mode */
  MCloudData,                   /**< message of type data */
  MCloudDone,                   /**< status message of type done */
  MCloudError,                  /**< status message of type error */
  MCloudReset,                  /**< status message of type reset */
  MCloudFlush,                  /**< status message of type flush */
  MCloudAudio,                  /**< data packet of type audio */
  MCloudText,                   /**< data packet of type text */
  MCloudImage,                  /**< data packet of type image */
  MCloudMixed,                  /**< data packet of type mixed */
  MCloudBinary,                 /**< data packet of type binary */
  MCloudSendingQueue,           /**< sending queue */
  MCloudProcessingQueue,         /**< receiving/ processing queue */
  MCloudCustomization,			/**< message with customizations */
  MCloudKeepAlive				/**< status message of type keep alive */
} MCloudType;

typedef enum {
  MCloudA_sAudioCodec,          /**< Audio codec type [RPCM (raw PCM), SPEEX (Speex), OPUS (Opus), FLAC (Flac)] (get/set) (default: RPCM) */
  MCloudA_iSampleRate,          /**< input/ output sample rate in Hz (get/set) (default: 16000) */
  MCloudA_iSampleSize,          /**< input/ output sample size in bits (get/set) (default: 16) */
  MCloudA_iChannelN,            /**< input/ output number of channels (get/set) (default: 1) */
  MCloudA_iBitRate              /**< input / output bit rate in bits/sec  */
} MCloudAttribute;

typedef enum {
  MCloudPWD_PLAIN = 0,		/**< Password as plaintext */
  MCloudPWD_ENCODED		/**< Password enctypted with sha1 function */
} MCloudPasswordType;

typedef enum {
  MCloudSSL_FullVerify = 0,	/**< required full verification of server certify */
  MCloudSSL_AcceptSelfSigned,	/**< self signed certificate accepted */
  MCloudSSL_AcceptAll		/**< accepted any type of certificate */
} MCloudSSLVerifyMode;

typedef struct MCloudPacket_S {

  MCloudType     packetType;    /**< \brief Type of the packet */
  MCloudType     dataType;      /**< \brief Type of the data included if packet is of type MCloudData */

  char          *sessionID;     /**< \brief The current sessionID which the packet belongs to */
  char          *streamID;      /**< \brief The current streamID which the packet belongs to */
  char          *fingerPrint;   /**< \brief Fingerprint of the packet */
  char          *creator;       /**< \brief Name of the creator of the packet */
  char          *start;         /**< \brief Human readable time stamp identifying the start time of the packet "dd/MM/YY-hh:mm:ss.mss" */
  char          *stop;          /**< \brief Human readable time stamp identifying the end time of the packet "dd/MM/YY-hh:mm:ss.mss" */
  unsigned int   startOffset;   /**< \brief Start time offset in ms relative to the beginning of the stream */
  unsigned int   stopOffset;    /**< \brief Stop time offset in ms relative to the beginning of the stream */
  char          *statusDescription; /**< \brief Optional detailed status description in case of status messages */
  char			*userID;		/** \brief The current userID */
  char			*cmType;		/** \brief The CM-type of the packet */
  char			*revision;		/** \brief The current revision of the packet */
  char          *xmlString;     /**< \brief Raw XML string */
  xmlDoc        *doc;           /**< \brief Reference to the whole XML document (libXML2 xmlDoc) */
} MCloudPacket;

typedef struct MCloudWordToken_S {
  int          index;           /**< \brief  The token index */
  char        *internal;        /**< \brief  The internal form of the token */
  char        *written;         /**< \brief  The written form of the token (can be NULL) */
  char        *spoken;          /**< \brief  The spoken form of the token (optional) */
  float        confidence;      /**< \brief  The confidence value in the interval [0,1] */
  unsigned int startTime;       /**< \brief  The start time [ms] relative to the start of the stream */
  unsigned int stopTime;        /**< \brief  The end time [ms] relative to the start of the stream */
  int          isFiller;        /**< \brief  This value is set to 1, if the token is a filler token and not a regular word */
} MCloudWordToken;

/**
  * Defines the supported encoding
  */
typedef enum {
  MCloudAC_UNK = 0,         /* Used to signal an unknown codec */  
  MCloudAC_PCM,             /* Raw PCM signed 16bit little endian (equivalent to legacy RPCM codec)*/
  MCloudAC_FLAC,
  MCloudAC_SPEEX,
  MCloudAC_OPUS
} MCloudCodec;

/**
 * MCloud object.
 */
typedef struct MCloud_S MCloud;

/**
 * \brief MCloud general callback function type used for finalize, break, and error.
 *
 * @param[in]  cloudP           reference to an MCloud object
 * @param[in]  userData         user data
 * @return                      0 if success
 */
typedef int MCloudCallbackFct (MCloud *cloudP, void *userData);

/**
 * \brief MCloud packet callback function type used for data and init.
 *
 * @param[in]  cloudP           reference to an MCloud object
 * @param[in]  p                packet containing the data to process
 * @param[in]  userData         user data
 * @return                      0 if success
 */
typedef int MCloudPacketCallbackFct (MCloud *cloudP, MCloudPacket *p, void *userData);

/* MACROS for printing log messages */
#define mcloudERROR(...) mcloudMsgHandler(__FILE__, __LINE__, 1, __VA_ARGS__)
#define mcloudWARN(...) mcloudMsgHandler(__FILE__, __LINE__, 2, __VA_ARGS__)
#define mcloudINFO(...) mcloudMsgHandler(__FILE__, __LINE__, 3, __VA_ARGS__)


/* ======================================================================
   Functions
   ====================================================================== */

/**
 * \brief Message handler that should be only used with the macros defined above.
 *
 * @param[in]  file             source code file name __FILE__
 * @param[in]  line             source code line __LINE__
 * @param[in]  type             message type
 * @param[in]  format           printf format
 * @param[in]  ...              variable list of additional arguments
 */
extern SHAREDDLL void mcloudMsgHandler (const char *file, int line, int type, const char *format, ... );

/* ----------------------------------------------------------------------
   Convenience functions for MCloudWordToken arrays
   ---------------------------------------------------------------------- */
/**
 * \brief Creates an array of MCloudWordTokens.
 *
 * @param[in]  n                number of elements
 * @return                      reference to the created token array or NULL if failed
 */
extern SHAREDDLL MCloudWordToken* mcloudWordTokenArrayCreate (int n);

/**
 * \brief Free an array of MCloudWordTokens.
 *
 * @param[in]  tokenA           reference to an MCloudWorkToken array
 * @param[in]  n                number of elements
 *
 */
extern SHAREDDLL void mcloudWordTokenArrayFree (MCloudWordToken* tokenA, int n);

/* ----------------------------------------------------------------------
   Convenience functions that simply the packet handling
   ---------------------------------------------------------------------- */
/**
 * \brief Initialize a new packet from text for sending.
 *
 * startOffset and stopOffset do not necessarily correspond with startTime and
 * stopTime. While startTime and stopTime define the absolute time of the packet,
 * startOffset and stopOffset define that start and stop time stamp of e.g.
 * the speech transcription within the packet (still relative to the beginning
 * of the stream. For example:
 * Packet from startTime=14:01:00.000 to stopTime=14:02:00.000 in a stream that
 * started at startTime=14:00:00.000. However, speech within this packet starts
 * 10 seconds ahead from the beginning of the packet because there is some silence
 * to first 10 seconds and ends 1 second before the end of the packet. This
 * means we need to define a startOffset=70000 (1 min since start of packet +
 * 10 seconds of silence) and a stopOffset=119000.
 *
 * @param[in]  cloudP           reference to an MCloud object
 * @param[in]  startTime        human readable starting time stamp
 * @param[in]  stopTime         human readable ending time stamp
 * @param[in]  startOffset      start time offset in ms relative to the beginning of the stream
 * @param[in]  stopOffset       stop time offset in ms relative to the beginning of the stream
 * @param[in]  fingerPrint      finger print of the text
 * @param[in]  text             the text string
 * @return                      reference to the created package or NULL if failed
 */
extern SHAREDDLL MCloudPacket* mcloudPacketInitFromText (MCloud *cloudP, const char* startTime, const char* stopTime, unsigned int startOffset, unsigned int stopOffset, const char *fingerPrint, const char *text);

/**
 * \brief Initialize a new packet from a MCloudWordToken array for sending.
 *
 * @param[in]  cloudP           reference to an MCloud object
 * @param[in]  startTime        human readable starting time stamp
 * @param[in]  stopTime         human readable ending time stamp
 * @param[in]  startOffset      start time offset in ms relative to the beginning of the stream
 * @param[in]  stopOffset       stop time offset in ms relative to the beginning of the stream
 * @param[in]  fingerPrint      finger print of the text
 * @param[in]  tokenA           reference to an MCloudWordToken array
 * @param[in]  tokenN           number of word tokens in array
 * @return                      reference to the created package or NULL if failed
 */
extern SHAREDDLL MCloudPacket* mcloudPacketInitFromWordTokenA (MCloud *cloudP, const char* startTime, const char* stopTime, unsigned int startOffset, unsigned int stopOffset, const char *fingerPrint, MCloudWordToken *tokenA, int tokenN);

/**
 * \brief Initialize a new packet from audio for sending.
 * \deprecated This function can be used only for PCM s16le 16KHz audio both as
 * input and as packet content!
 *
 * @param[in]  cloudP           reference to an MCloud object
 * @param[in]  startTime        human readable starting time stamp
 * @param[in]  stopTime         human readable ending time stamp
 * @param[in]  fingerPrint      finger print of the audio
 * @param[in]  sampleA          reference to an array of audio samples
 * @param[in]  sampleN          number of samples in array
 * @param[in]  isFinal          indicates whether the sample array given is the final one
 * @return                      reference to the created package or NULL if failed
 */
extern SHAREDDLL MCloudPacket* mcloudPacketInitFromAudio (MCloud *cloudP, const char* startTime, const char* stopTime, const char *fingerPrint, const short *sampleA, int sampleN, int isFinal);

/**
 * \brief Initialize a new packet from binary for sending.
 *
 * @param[in]  cloudP           reference to an MCloud object
 * @param[in]  startTime        human readable starting time stamp (set to NULL if not relevant)
 * @param[in]  stopTime         human readable ending time stamp (set to NULL if not relevant)
 * @param[in]  fingerPrint      finger print of the binary data
 * @param[in]  filename         name of the file being sent (set to NULL if not relevant)
 * @param[in]  mimetype         MIME type of the file being sent (set to NULL if not relevant)
 * @param[in]  bytes            reference to a buffer of bytes
 * @param[in]  bytesN           number of bytes
 * @param[in]  last             whether this packet is the last for the current file 
 *                              (i.e. following packets with the same filename
 *                              should be treated as a different file)
 * @return                      reference to the created package or NULL if failed
 */
extern SHAREDDLL MCloudPacket* mcloudPacketInitFromBinary (MCloud *cP, const char* startTime, const char* stopTime, const char *fingerPrint, const char *filename, const char *mimetype, const uint8_t *bytes, int bytesN, int last);


/**
 * \brief Initialize a new packet from image for sending.
 *
 * @param[in]  cloudP           reference to an MCloud object
 * @param[in]  startTime        human readable starting time stamp
 * @param[in]  stopTime         human readable ending time stamp
 * @param[in]  fingerPrint      language finger print of the image (slide text language)
 * @param[in]  width            image width
 * @param[in]  height           image height
 * @param[in]  format           image format, i.e. PNG
 * @param[in]  buffer           reference to a buffer that keeps the image
 * @param[in]  bufferN          length of buffer in bytes
 * @return                      reference to the created package or NULL if failed
 */
extern MCloudPacket* mcloudPacketInitFromImage (MCloud *cloudP, const char* startTime, const char* stopTime, const char *fingerPrint, int width, int height, const char *format, const char *buffer, int bufferN);

/**
 * \brief Add audio to an existing package for sending. Packet type will be changed to "mixed".
 *
 * \deprecated This function can be used only for PCM s16le 16KHz audio both as
 * input and as packet content!
 *
 * @param[in]  cloudP           reference to an MCloud object
 * @param[in]  p                MCloudPacket to which the audio should be added
 * @param[in]  startTime        human readable starting time stamp
 * @param[in]  stopTime         human readable ending time stamp
 * @param[in]  fingerPrint      finger print of the audio
 * @param[in]  sampleA          reference to an array of audio samples
 * @param[in]  sampleN          number of samples in array
 * @param[in]  isFinal          indicates whether the sample array given is the final one
 * @return                      reference to the created package or NULL if failed
 */
extern SHAREDDLL MCloudPacket* mcloudPacketAddAudio (MCloud *cloudP, MCloudPacket *p, const char* startTime, const char* stopTime, const char *fingerPrint, const short *sampleA, int sampleN, int isFinal);

/**
 * \brief Free a packet.
 *
 * @param[in]  p                reference to an MCloud package
 */
extern SHAREDDLL void mcloudPacketDeinit (MCloudPacket *p);

/**
 * \brief Convenient function for extracting the string embedded in \<text\>\</text\>.
 *
 * Don't forget to free the memory allocated for text.
 *
 * @param[in]  cloudP           reference to an MCloud object
 * @param[in]  p                reference to an MCloud package
 * @param[out] text             returned text
 * @return                      S2S_Success if no error occurred
 */
extern SHAREDDLL S2S_Result mcloudPacketGetText (MCloud *cloudP, MCloudPacket *p, char **text);

/**
 * \brief Convenient function for replacing the string embedded in \<text\>\</text\>.
 *
 * Don't forget to free the memory allocated for text.
 *
 * @param[in]  cloudP           reference to an MCloud object
 * @param[in]  p                reference to an MCloud package
 * @param[out] text             returned text
 * @return                      S2S_Success if no error occurred
 */
extern SHAREDDLL S2S_Result mcloudPacketReplaceText (MCloud *cloudP, MCloudPacket *p, char *text);

/**
 * \brief Convenient function for extracting a word token array embedded in \<wordtokens\>\</wordtokens\>.
 *
 * Don't forget to free the memory allocated for tokenA.
 *
 * @param[in]  cloudP           reference to an MCloud object
 * @param[in]  p                reference to an MCloud package
 * @param[out] tokenA           returned array of word tokens
 * @param[out] tokenN           number of word tokens returned
 * @return                      S2S_Success if no error occurred
 */
extern SHAREDDLL S2S_Result mcloudPacketGetWordTokenA (MCloud *cloudP, MCloudPacket *p, MCloudWordToken **tokenA, int *tokenN);

/**
 * \brief Convenient function for extracting the data embedded in \<audio\>\</audio\>.
 *
 * Don't forget to free the memory allocated for sampleA.
 *
 * @param[in]  cloudP           reference to an MCloud object
 * @param[in]  p                reference to an MCloud package
 * @param[out] sampleA          returned array of samples
 * @param[out] sampleN          returned number of samples in array
 * @return                      S2S_Success if no error occurred
 */
extern SHAREDDLL S2S_Result mcloudPacketGetAudio (MCloud *cloudP, MCloudPacket *p, short **sampleA, int *sampleN);

/**
 * \brief Convenient function for extracting the data embedded in \<audio\>\</audio\>
 *
 * Don't forget to free the memory allocated for sampleA.
 *
 * @param[in]  cP           reference to an MCloud object
 * @param[in]  p            reference to an MCloud package
 * @param[out] bA           returned array of binary samples
 * @param[out] bN           returned number of binary samples in array
 * @param[out] filename     return the declared name for the file or NULL if not specified
 * @param[out] mimetype     return the declared mimetype for the file or NULL if not specified
 * @param[out] lsat         1 if this is the last packet for the file, 0 if more packet are incoming, -1 if not specified
 * @return                      S2S_Success if no error occurred
 */
  extern SHAREDDLL S2S_Result mcloudPacketGetBinary (MCloud *cP, MCloudPacket *p, uint8_t **bA, int *bN, char **filename, char **mimetype, int *last);


/* ----------------------------------------------------------------------
   MCloud
   ---------------------------------------------------------------------- */

/**
 * \brief Create an MCloud object with a given name and mode.
 *
 * @param[in]  name             descriptive name of the worker or client (used as 'creator' in XML messages)
 * @param[in]  mode             working mode, i.e. MCloudModeWorker, or MCloudModeClient
 * @return                      reference to an MCloud object or NULL if failed
 */
extern SHAREDDLL MCloud* mcloudCreate (const char *name, int mode);



/**
 * \brief Create an MCloud object for an ssl connection with a given name, mode and certificate verification mode.
 * 
 * @param[in]  name             descriptive name of the worker or client (used as 'creator' in XML messages)
 * @param[in]  mode             working mode, i.e. MCloudModeWorker, or MCloudModeClient
 * @param[in]  vmode		verification mode of the server certificate
 * @return                      reference to an MCloud object or NULL if failed
 */


extern SHAREDDLL MCloud* mcloudCreateSSL (const char *name, int mode, MCloudSSLVerifyMode vmode);

/**
 * \brief Create an MCloud object with a given name, mode, username, password.
 *
 * @param[in]  name             descriptive name of the worker or client (used as 'creator' in XML messages)
 * @param[in]  mode             working mode, i.e. MCloudModeWorker, or MCloudModeClient
 * @param[in]  username         organization username
 * @param[in]  password         organization password 
 * @return                      reference to an MCloud object or NULL if failed
 */

extern SHAREDDLL MCloud* mcloudCreate2 (const char *name, int mode, const char *username, const char *password);


/**
 * \brief Create an MCloud object for an ssl connection with a given name, mode, username, password and certificate verification mode.
 *
 * @param[in]  name             descriptive name of the worker or client (used as 'creator' in XML messages)
 * @param[in]  mode             working mode, i.e. MCloudModeWorker, or MCloudModeClient
 * @param[in]  username         organization username
 * @param[in]  password         organization password
 * @param[in]  vmode            verification mode of the server certificate
 * @return                      reference to an MCloud object or NULL if failed
 */

extern SHAREDDLL MCloud* mcloudCreate2SSL (const char *name, int mode, const char *username, const char *password, MCloudSSLVerifyMode vmode);

/**
 * \brief Create an MCloud object with a given name, mode, username, password.
 *
 * @param[in]  name             descriptive name of the worker or client (used as 'creator' in XML messages)
 * @param[in]  mode             working mode, i.e. MCloudModeWorker, or MCloudModeClient
 * @param[in]  username         organization username
 * @param[in]  password         organization password encrypted using sha1 hashing function
 * @return                      reference to an MCloud object or NULL if failed
 */

extern SHAREDDLL MCloud* mcloudCreate3 (const char *name, int mode, const char *username, const char *password);

/**
 * \brief Create an MCloud object for an ssl connection with a given name, mode, username, password and certificate verification mode.
 * 
 * @param[in]  name             descriptive name of the worker or client (used as 'creator' in XML messages)
 * @param[in]  mode             working mode, i.e. MCloudModeWorker, or MCloudModeClient
 * @param[in]  username         organization username
 * @param[in]  password         organization password encrypted using sha1 hashing function
 * @param[in]  vmode            verification mode of the server certificate
 * @return                      reference to an MCloud object or NULL if failed
 */



extern SHAREDDLL MCloud* mcloudCreate3SSL (const char *name, int mode, const char *username, const char *password, MCloudSSLVerifyMode vmode);

/**
 * \brief Free an MCloud object.
 *
 * @param[in]  cloudP           reference to an MCloud object
 */
extern SHAREDDLL void mcloudFree (MCloud *cloudP);

/**
 * \brief Get the value of an attribute.
 *
 * @param[in]  cloudP           reference to an MCloud object
 * @param[in]  attr             an MCloud attribute
 * @param[out] value            reference to the returned value
 * @return                      S2S_Success if no error occurs
 */
extern SHAREDDLL S2S_Result mcloudGetAttr (MCloud *cloudP, MCloudAttribute attr, void *value);

/**
 * \brief Set the value of an attribute.
 * \deprecated Set the value of an attribute.
 *
 * @param[in]  cloudP           reference to an MCloud object
 * @param[in]  attr             an MCloud attribute
 * @param[in]  value            reference to the value to be set
 * @return                      S2S_Success if no error occurs
 */
extern SHAREDDLL S2S_Result mcloudSetAttr (MCloud *cloudP, MCloudAttribute attr, const void *value);


/**
 * \brief Set the audio codec.
 *
 * @param[in]  cp           	reference to an MCloud object
 * @param[in]  codec            MCloud codec used to transmit/receive data to/from the Mediator
 * @param[in]  sampleRate      	sample rate used to transmit/receive data to/from the Mediator
 * @param[in]  bitRate         	bit rate used to transmit/receive data to/from the Mediator
 * @param[in]  channels         channels used to transmit/receive data to/from the Mediator
 * @return                      S2S_Success if no error occurs
 */
extern SHAREDDLL S2S_Result mcloudSetAudioEncoder (MCloud *cp, MCloudCodec codec, int sampleRate, int bitRate, int channels);


/**
 * \brief Set the audio codec.
 *
 * @param[in]  cp           	reference to an MCloud object
 * @param[in]  codec            codec (string form) used to transmit/receive data to/from the Mediator
 * @param[in]  sampleRate      	sample rate used to transmit/receive data to/from the Mediator
 * @param[in]  bitRate         	bit rate used to transmit/receive data to/from the Mediator
 * @param[in]  channels         channels used to transmit/receive data to/from the Mediator
 * @return                      S2S_Success if no error occurs
 */
extern SHAREDDLL S2S_Result mcloudSetAudioEncoder2 (MCloud *cp, char *codec, int sampleRate, int bitRate, int channels);

/**
 * \brief Connect to the MCloud server running on the host at port given.
 *
 * This function has to be called after an MCloud object has been created
 * and before waiting for a client or worker.
 *
 * @param[in]  cloudP           reference to an MCloud object
 * @param[in]  host             host name
 * @param[in]  port             port number
 * @return                      S2S_Success if no error occurs
 */
extern SHAREDDLL S2S_Result mcloudConnect (MCloud *cloudP, const char *host, int port);

/**
 * \brief Disconnect from the MCloud server.
 *
 * @param[in]  cloudP           reference to an MCloud object
 * @return                      S2S_Success if no error occurs
 */
extern SHAREDDLL S2S_Result mcloudDisconnect (MCloud *cloudP);

/**
 * \brief Add a service description of a worker to an MCloud object.
 *
 * This function has to be called after an MCloud object has been created and
 * before connecting to the MCloud.
 *
 * @param[in]  cloudP                   reference to an MCloud object
 * @param[in]  name                     name of the worker
 * @param[in]  service                  name of the service (asr, smt, tts, ...)
 * @param[in]  inputFingerPrint         service input finger print
 * @param[in]  inputType                data input type (audio, text)
 * @param[in]  outputFingerPrint        service output finger print
 * @param[in]  outputType               data output type (audio, text)
 * @param[in]  specifier                an additional specifier, i.e. a speaker identifier
 * @return                              S2S_Success if no error occurs
 */
extern SHAREDDLL S2S_Result mcloudAddService (MCloud *cloudP, const char *name, const char *service, const char *inputFingerPrint, const char *inputType, const char *outputFingerPrint, const char *outputType, const char *specifier);

/**
 * \brief Add a flow description of a client to an MCloud object.
 *
 * This function has to be called after an MCloud object has been created and
 * before connecting to the MCloud.
 * A client can add more than one flow being just translations of the same descriptions.
 * Therefore, the password and logging has to be the same over all flows.
 *
 * @param[in]  cloudP                   reference to an MCloud object
 * @param[in]  username                 an optional username which has to be used in order to subscribe to this flow (display server), NULL for no username
 * @param[in]  password                 an optional password which has to be used in order to subscribe to this flow (display server), NULL for no password
 * @param[in]  logging                  if set to 0, the flow will not be logged in the database
 * @param[in]  language                 descriptive language identifier of the flow, e.g. English
 * @param[in]  name                     name of the flow, e.g. title of a talk
 * @param[in]  description              additional description of the flow, e.g. abstract
 * @return                              S2S_Success if no error occurs
 */
extern SHAREDDLL S2S_Result mcloudAddFlowDescription2 (MCloud *cloudP, const char *username, const char *password, int logging, const char *language, const char *name, const char *description);

/**
* \brief Add a flow description of a client to an MCloud object.
*
* \deprecated This function has to be called after an MCloud object has been created and
* before connecting to the MCloud.
* A client can add more than one flow being just translations of the same descriptions.
* Therefore, the password and logging has to be the same over all flows.
* This call is deprecated and will be removed in future versions of the API
*
* @param[in]  cloudP                   reference to an MCloud object
* @param[in]  password                 an optional password which has to be used in order to subscribe to this flow (display server), NULL for no password
* @param[in]  logging                  if set to 0, the flow will not be logged in the database
* @param[in]  language                 descriptive language identifier of the flow, e.g. English
* @param[in]  name                     name of the flow, e.g. title of a talk
* @param[in]  description              additional description of the flow, e.g. abstract
* @return                              S2S_Success if no error occurs
*/
extern SHAREDDLL S2S_Result mcloudAddFlowDescription(MCloud *cloudP, const char *password, int logging, const char *language, const char *name, const char *description);

/**
 * \brief Announce an output stream of a client to an MCloud object.
 *
 * This function has to be called after the client has been
 * connected to the MCloud.
 *
 * @param[in]  cloudP                   reference to an MCloud object
 * @param[in]  type                     data type (audio, text, image)
 * @param[in]  fingerPrint              finger print of the data stream
 * @param[in]  streamID                 unique stream identifier
 * @param[in]  specifier                an additional specifier, i.e. a speaker identifier
 * @return                              S2S_Success if no error occurs
 */
extern SHAREDDLL S2S_Result mcloudAnnounceOutputStream (MCloud *cloudP, const char *type, const char *fingerPrint, const char *streamID, const char *specifier);

/**
 * \brief Request an input stream of a client from an MCloud object.
 *
 * This function has to be called in order to request a specific
 * input stream from the MCloud such as ASR or MT results. Otherwise,
 * the client will not receive any data.
 * This function has to be called after the client has been
 * connected to the MCloud.
 *
 * @param[in]  cloudP                   reference to an MCloud object
 * @param[in]  type                     data type (audio, text, image)
 * @param[in]  fingerPrint              finger print of the data stream
 * @param[in]  streamID                 stream identifier of the (output) stream which the requested data stream belongs to
 * @param[in,out] info                  additional information received from the MCloud
 * @param[in]  infoN					length of info field in bytes
 * @return                              S2S_Success if no error occurs
 */
extern SHAREDDLL S2S_Result mcloudRequestInputStream (MCloud *cloudP, const char *type, const char *fingerPrint, const char *streamID, char *info, int infoN);

/**
 * \brief Request the display of an output stream.
 *
 * By calling this function, the client requests the display of the
 * output stream on the display server.
 * For cancelling the request for display, the client needs to disconnect.
 *
 * @param[in]  cloudP                   reference to an MCloud object
 * @return                              S2S_Success if no error occurs
 */
extern SHAREDDLL S2S_Result mcloudRequestForDisplay (MCloud *cloudP);

/**
 * \brief Wait for a service request to process.
 *
 * This function has to be called after the worker has been successfully
 * connected to the MCloud in order to wait for an incoming service
 * request to process.
 *
 * @param[in]  cloudP                   reference to an MCloud object
 * @param[out] streamID                 ID of input stream
 * @return                              S2S_Success if no error occurs
 */
extern SHAREDDLL S2S_Result mcloudWaitForClient (MCloud *cloudP, char **streamID);

/**
 * \brief Wait for the next data package.
 *
 * This function has to be called in order to wait for the next
 * data packet.
 * The function times out after an internally specified amount of time.
 *
 * @param[in]  cloudP                   reference to an MCloud object
 * @return                              S2S_Success if no error occurs
 */
extern SHAREDDLL MCloudPacket* mcloudGetNextPacket (MCloud *cloudP);

/**
 * \brief Send a packet.
 *
 * This function has to be called to send a data packet to the MCloud.
 * The data packet has to be created in advance by using the packet handling
 * functions and has to be freed after usage.
 *
 * @param[in]  cloudP                   reference to an MCloud object
 * @param[in]  p                        an MCloud packet
 * @return                              S2S_Success if no error occurs
 */
extern SHAREDDLL S2S_Result mcloudSendPacket (MCloud *cloudP, MCloudPacket *p);

/**
 * \brief Send a packet asynchronously.
 *
 * This function can be used for sending data packets asynchronously. The
 * packages will be placed into an internal queue and sent in the background.
 * Callback functions are used to forward status messages such as errors.
 * Use mcloudBreak to stop sending pending packages. Use mcloudWaitFinish to wait
 * until the last package has been sent.
 * Packages are freed automatically after they have been sent.
 * While sending, the error callback function may be called in case of errors and
 * the break callback function if mcloudBreak has been called. As soon as no more
 * packages are pending and mcloudWaitFinish has been called, the finalize callback
 * function is called.
 *
 * @param[in]  cloudP                   reference to an MCloud object
 * @param[in]  p                        an MCloud packet
 * @param[in]  userData                 additional user data, that is passed to the callback functions
 * @return                              S2S_Success if no error occurs
 */
extern SHAREDDLL S2S_Result mcloudSendPacketAsync (MCloud *cloudP, MCloudPacket *p, void *userData);


/**
 * \brief Convenience function for sending the content of a whole file.
 *
 * @param[in]  cloudP           reference to an MCloud object
 * @param[in]  f                the FILE stream to send                        
 * @param[in]  chunkSize        bytes read from the file to be inserted in 1 packet
 * @param[in]  filename         name of the file being sent (set to NULL if not relevant)
 * @param[in]  mimetype         MIME type of the file being sent (set to NULL if not relevant)
 * @param[in]  fingerPrint      finger print of the file
 * @return                      S2S_Success if no error occurs
 */
extern SHAREDDLL S2S_Result mcloudSendBinaryFile(MCloud *cP, FILE *f, int chunkSize, char *filename, char *mimeType, char *fingerPrint);


/**
 * \brief Convenience function for sending the content of a whole file asyncronously.
 *
 * @param[in]  cloudP           reference to an MCloud object
 * @param[in]  f                the FILE stream to send                        
 * @param[in]  chunkSize        bytes read from the file to be inserted in 1 packet
 * @param[in]  filename         name of the file being sent (set to NULL if not relevant)
 * @param[in]  mimetype         MIME type of the file being sent (set to NULL if not relevant)
 * @param[in]  fingerPrint      finger print of the file
 * @param[in]  userData         additional user data, that is passed to the callback functions for each packet
 * @return                      S2S_Success if no error occurs
 */
extern SHAREDDLL S2S_Result mcloudSendBinaryFileAsync(MCloud *cP, FILE *f, int chunkSize, char *filename, char *mimeType, char *fingerPrint, void *userData);

/**
 * \brief Inform a client or a worker that there is no more data to receive.
 *
 * This function should be called to inform a worker that there is no
 * more data to receive from a client, or to inform a client, that
 * the worker finished processing of the data received. As soon as both,
 * worker and client have sent a done, the session is terminated.
 * @param[in]  cloudP                   reference to an MCloud object
 * @return                              S2S_Success if no error occurs
 */
extern SHAREDDLL S2S_Result mcloudSendDone (MCloud *cloudP);

/**
 * \brief Inform a client or a worker that an error occurred during processing.
 *
 * This function can be called if an error occurs during processing
 * either in the worker or client in order to inform all other involved
 * components. Typically this results in a termination of the current
 * session.
 * @param[in]  cloudP                   reference to an MCloud object
 * @param[in]  description              error description
 * @return                              S2S_Success if no error occurs
 */
extern SHAREDDLL S2S_Result mcloudSendError (MCloud *cloudP, const char *description);

/**
 * \brief Inform subsequent worker to flush their output buffers.
 *
 * This function should be called to inform subsequent workers finalize processing
 * data stored in the queue and to flush their buffers.
 * @param[in]  cloudP                   reference to an MCloud object
 * @return                              S2S_Success if no error occurs
 */
extern SHAREDDLL S2S_Result mcloudSendFlush (MCloud *cloudP);

/**
 * \brief Send a keepalive message to following worker.
 *
 *  This function should be called to inform subsequent workers that 
 *  the client\worker is alive and stay waiting for a batch process.
 * 
 *  @param[in]  cloudP                  reference to an MCloud object
 *  @return                             S2S_Success if no error occurs
 */

extern SHAREDDLL S2S_Result mcloudSendKeepAlive (MCloud *cloudP);

/**
 * \brief Process received packages asynchronously.
 *
 * This function can be used to process packets asynchronously. The packages
 * will be placed into an internal queue and processed in the background
 * by calling mcloudDataCallback.
 * Callback functions are used to forward status messages such as errors.
 * Use mcloudBreak to stop processing pending packages. Use mcloudWaitFinish to wait
 * until the last package has been processed.
 * Packages are freed automatically after they have been sent.
 * While processing, the data callback function is called for the next pending
 * package. As soon as no more packages are pending and mcloudWaitFinish has been called,
 * the finalize callback function is called. The error callback function may
 * be called in case of errors, and the break callback function if mcloudBreak has
 * been called.
 *
 * @param[in]  cloudP                   reference to an MCloud object
 * @param[in]  p                        an MCloud packet
 * @param[in]  userData                 additional user data, that is passed to the callback functions
 * @return                              S2S_Success if no error occurs
 */
extern SHAREDDLL S2S_Result mcloudProcessDataAsync (MCloud *cloudP, MCloudPacket *p, void *userData);

/**
 * \brief Return number of pending packages in queue.
 *
 * @param[in]  cloudP           reference to an MCloud object
 * @param[in]  queueType        type of queue MCloudSendingQueue, or MCloudProcessingQueue
 * @return                      number of pending packages
 */
extern SHAREDDLL int mcloudPending (MCloud *cloudP, MCloudType queueType);

/**
 * \brief Wait until all pending packages have been processed/ sent.
 *
 * This function can be used to wait until all pending packages
 * have been processed or sent in the queue specified.
 *
 * @param[in]  cloudP           reference to an MCloud object
 * @param[in]  queueType        type of queue MCloudSendingQueue, or MCloudProcessingQueue
 * @param[in]  done             if set to 1, indicates that processing of the request has been completed
 * @return                      S2S_Success if no error occurs
 */
extern SHAREDDLL S2S_Result mcloudWaitFinish (MCloud *cloudP, MCloudType queueType, int done);

/**
 * \brief Stop processing, sending pending packages immediately, and reset queue.
 *
 * This function can be used to stop further processing or sending packages in the queue specified.
 *
 * @param[in]  cloudP           reference to an MCloud object
 * @param[in]  queueType        type of queue MCloudSendingQueue, or MCloudProcessingQueue
 * @return                      S2S_Success if no error occurs
 */
extern SHAREDDLL S2S_Result mcloudBreak (MCloud *cloudP, MCloudType queueType);

/**
 * \brief Set an init callback function.
 *
 * This function is called as soon as an incoming service request has been
 * accepted by the worker, i.e. in mcloudWaitForClient.
 * The packet containing the service description is passed to the init callback
 * function as argument.
 * This callback is only available for the processing queue.
 *
 * @param[in]  cloudP           reference to an MCloud object
 * @param[in]  callback         reference to the function that has to be called
 * @param[in]  userData         reference to some data that is passed to the callback function
 */
extern SHAREDDLL void mcloudSetInitCallback (MCloud *cloudP, MCloudPacketCallbackFct *callback, void *userData);

/**
 * \brief Set a finalize callback function.
 *
 * This function is called as soon as the processing of packets
 * should be finalized, i.e. no more packets will follow and the worker
 * should output the final results after all pending packets have been
 * processed.
 * This callback is only available for the processing queue.
 *
 * @param[in]  cloudP           reference to an MCloud object
 * @param[in]  callback         reference to the function that has to be called
 * @param[in]  userData         reference to some data that is passed to the callback function
 */
extern SHAREDDLL void mcloudSetFinalizeCallback (MCloud *cloudP, MCloudCallbackFct *callback, void *userData);

/**
 * \brief Set a break callback function.
 *
 * This function is called when the worker should stop the processing
 * as soon as possible.
 * This callback is available for both the sending and the processing queue.
 *
 * @param[in]  cloudP           reference to an MCloud object
 * @param[in]  queueType        type of queue MCloudSendingQueue, or MCloudProcessingQueue
 * @param[in]  callback         reference to the function that has to be called
 * @param[in]  userData         reference to some data that is passed to the callback function
 */
extern SHAREDDLL void mcloudSetBreakCallback (MCloud *cloudP, MCloudType queueType, MCloudCallbackFct *callback, void *userData);

/**
 * \brief Set an error callback function.
 *
 * This function is called as soon as an error occurs in the asynchronous processing.
 * This callback is available for both the sending and the processing queue.
 *
 * @param[in]  cloudP           reference to an MCloud object
 * @param[in]  queueType        type of queue MCloudSendingQueue, or MCloudProcessingQueue
 * @param[in]  callback         reference to the function that has to be called
 * @param[in]  userData         reference to some data that is passed to the callback function
 */
extern SHAREDDLL void mcloudSetErrorCallback (MCloud *cloudP, MCloudType queueType, MCloudCallbackFct *callback, void *userData);

/**
 * \brief Set a data callback function.
 *
 * This function is called for each incoming data package in a serial way, i.e.
 * after a package has been processed it is called again if more packages are pending.
 * Note that for this function no userData is given at the time of the set
 * of the callback function. Instead, the userData is given per packet with
 * mcloudProcessDataAsync or mcloudSendAsync.
 * This callback is available for the processing queue only.
 *
 * @param[in]  cloudP           reference to an MCloud object
 * @param[in]  callback         reference to the function that has to be called
 */
extern SHAREDDLL void mcloudSetDataCallback (MCloud *cloudP, MCloudPacketCallbackFct *callback);

/**
 * \brief Set a customization callback function.
 *
 * @param[in]  cloudP           reference to an MCloud object
 * @param[in]  callback         reference to the function that has to be called
 * @param[in]  userData         reference to some data that is passed to the callback function 
 */
extern SHAREDDLL void mcloudSetCustomizationCallback (MCloud *cP, MCloudPacketCallbackFct *callback, void *userData);

/**
 * \brief Initialize a new packet from a CmGet for sending.
 *
 * @param[in]  cloudP           reference to an MCloud object
 * @param[in]  sessionId        current sessionID which the packet belongs to
 * @param[in]  userId           current userID
 * @param[in]  fingerPrint      current finger print
 * @param[in]  type             current type
 * @param[in]  revision         current revision
 * @return                      reference to the created package or NULL if failed
 */
extern SHAREDDLL MCloudPacket* mcloudPacketInitFromCmGet (MCloud *cP, const char* sessionId, const char* userId, const char *fingerPrint, const char* type, const char* revision);

/**
 * \brief Convenient function for encoding the data in base64.
 *
 * @param[in]  data             data to be encoded
 * @param[in]  input_length     length of input data stream
 * @param[in]  output_length    length of output data stream
 * @return                      encoded data
 */
extern SHAREDDLL char* base64_encode(const char *data, size_t input_length, size_t *output_length);

/**
 * \brief Convenient function for decoding the data from base64.
 *
 * @param[in]  data             data to be decoded
 * @param[in]  input_length     length of input data stream
 * @param[in]  output_length    length of output data stream
 * @return                      decoded data
 */
extern SHAREDDLL char* base64_decode(const char *data, size_t input_length, size_t *output_length);

/**
 * \brief Convenient function that returns a url-encoded version of a string.
 *
 * Don't forget to free() the returned string after use
 *
 * @param[in]  str              string to be encoded
 * @return                      url-encoded version of str
 */
extern SHAREDDLL char *url_encode(const char *str);

/**
 * \brief Convenient function that returns a url-decoded version of a string.
 *
 * Don't forget to free() the returned string after use
 *
 * @param[in]  str              string to be decoded
 * @return                      url-decoded version of str
 */
extern SHAREDDLL char *url_decode(const char *str);

#ifdef __cplusplus
}
#endif

#endif

