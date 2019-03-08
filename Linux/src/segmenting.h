// because of creating a new directory:
// https://stackoverflow.com/questions/7430248/creating-a-new-directory-in-c
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>
#define MAX_DATE_LEN 30

// end of includes for a new directory

struct stat st = {0};
static void make_directory(char *dirname) {
	if (stat(dirname, &st) == -1) {
		mkdir(dirname, 0700);
	}
	return;
}

static FILE* segmentFile = NULL;
static char segmentFileName[BUFSIZ];
static char segmentDirName[BUFSIZ];
static char* outputDir = NULL;


#define CLOSED 0; // když je segmentFile NULL
#define OPENED 1; // když je segmentFile not NULL



#define TOUCHFILE "recording"


static void datetostr(char *date) {
	/*
		*/
	time_t tnow;
	tnow = time(NULL);


	// https://stackoverflow.com/questions/10192903/time-in-milliseconds
	struct timeval now;
	gettimeofday(&now, NULL);

	if(tnow != -1) {
		strftime(date, MAX_DATE_LEN, "%Y%m%d_%H%M%S", gmtime(&tnow));
	}
	sprintf(date, "%s.%f", date, now.tv_usec);
	return &date;
}


// start or stop recording, depending on the touchfile
static void check_segmenting() {

	FILE *f;
	if ((f = fopen(TOUCHFILE, "r"))) {

		if (segmentFile == NULL) { // recording is off, start it

			char buf[BUFSIZ];
			char content[BUFSIZ];
			content[0] = '\0';
			size_t nread;

			while ((nread = fread(buf, 1, sizeof buf, f)) > 0) {
				for (int i=0; i<nread; i++) {
					if (buf[i] == '\n')
						break;
					content[i] = buf[i];
				}
			}
			fclose(f);

			if ((content[0] == '\0')) {
				char date[MAX_DATE_LEN];
				datetostr(&date);

				sprintf(segmentFileName, "%s/segment-%s.pcm", segmentDirName, date);
			}
			else {
				sprintf(segmentFileName, "%s/%s", segmentDirName, content);
			}


			int i = 0;
			char sfn[BUFSIZ];
			sprintf(sfn, "%s", segmentFileName);
			while ( access( segmentFileName, F_OK) != -1) {
				sprintf(segmentFileName, "%s_%d.pcm", sfn, i);
				i++;
			}
			// open a new segmentFile => recording is started
			segmentFile = fopen(segmentFileName, "wb");

		}/* else { // recording is running, let it be
		}
		*/


	} else { // touchfile doesn't exist
		if (segmentFile != NULL) { // recording is on, close it
			fclose(segmentFile);
			segmentFile = NULL;
		}
	}

}