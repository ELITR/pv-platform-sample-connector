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
static time_t touchfile_last_change = 0;



static void datetostr(char *date) {
	/*
		*/
	time_t tnow;
	tnow = time(NULL);


	// https://stackoverflow.com/questions/10192903/time-in-milliseconds
//	struct timeval now;
//	gettimeofday(&now, NULL);
//	printf("%d\n",now.tv_sec);

	if(tnow != -1) {
		strftime(date, MAX_DATE_LEN, "%Y%m%d_%H%M%S", localtime(&tnow));
	}
//	sprintf(date, "%s.%f", date, now.tv_usec);
	return &date;
}

static void close_segmenting() {
	fclose(segmentFile);
	segmentFile = NULL;
	fprintf(stderr, "STOP closing segmentfile %s\n",segmentFileName);
	return;
}


// start or stop recording, depending on the touchfile
static void check_segmenting() {
//	struct timeval now;
//	gettimeofday(&now, NULL);
//	printf("tady %d\n",((now.tv_sec==last.tv_sec) && (now.tv_usec==last.tv_usec))?1:0);
//	last = now;


	FILE *f;
	if ((f = fopen(TOUCHFILE, "r"))) {
		struct stat touch_stat;
		stat(TOUCHFILE, &touch_stat);
		time_t mod_time = touch_stat.st_ctime;
		//fprintf(stderr, "mod_time %d %d\n",mod_time, touchfile_last_change);

		if ((mod_time != touchfile_last_change) && (segmentFile != NULL)) {
			close_segmenting();
		}

		if (segmentFile == NULL) { // recording is off, start it

			touchfile_last_change = mod_time;


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
			fprintf(stderr, "START opening new segmentfile %s\n",segmentFileName);
			segmentFile = fopen(segmentFileName, "wb");

		}/* else { // recording is running, keep it 
		}
		*/

		fclose(f);


	} else { // touchfile doesn't exist
		if (segmentFile != NULL) { // recording is on, close it
			close_segmenting();
		}
	}

}
