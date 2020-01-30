
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <ctype.h>

#include "loadconfig.h"
#include "Data.h"
#include "Dir.h"
#include "Parser.h"

extern int errno;

void loadconfig(char *cfgfile)  /* load isoqlog configuration file */
{
	FILE *fd;
	char buf[BUFSIZE];
	char keyword[KEYSIZE];
	char value[VALSIZE];
	char *cp1, *cp2;
	char *variables[] = { "Invalid",
		"outputdir",
		"logtype",
		"logstore",
		"domainsfile",
		"langfile",
		"htmldir",
		"hostname",
		"maxsender",
		"maxreceiver",
		"maxtotal",
		"maxbyte"
	};

	int i, key, line, keyword_nums = sizeof(variables)/sizeof(char *);

	
	if ((fd = fopen(cfgfile, "r")) == NULL) {
		fprintf(stderr, "loadconfig: cannot open isoqlog configuration file %s, exiting...\n", cfgfile);
		exit(-1);
	}
	line = 0;
	while ((fgets(buf, BUFSIZE, fd)) != NULL) {
		line++;
		if (buf[0] == '#') 
			continue;
		if ((strlen(buf)) <= 1)
			continue;
		cp1 = buf;
		cp2 = keyword;
		while (isspace((int)*cp1)) 
			cp1++;
		while (isgraph((int)*cp1) && (*cp1 != '=')) 
			*cp2++ = *cp1++;
		*cp2 = '\0';
		cp2 = value;
		while ((*cp1 != '\0') && (*cp1 !='\n') && (*cp1 !='='))
			cp1++;
		cp1++; 
		while (isspace((int)*cp1))
			cp1++; 
		if (*cp1 == '"') 
			cp1++;
		while ((*cp1 != '\0') && (*cp1 !='\n') && (*cp1 !='"'))
			*cp2++ = *cp1++;
		*cp2-- = '\0';
		if (keyword[0] =='\0' || value[0] =='\0')
			continue;
		key = 0;
		for (i = 0; i < keyword_nums; i++) {
			if ((strcmp(keyword, variables[i])) == 0) {
				key = i;
				break;
			}
		}

		switch(key) {
		case 0:
			fprintf(stderr, "Illegal Keyword: %s\n", keyword);
			break;
		case 1:
			strncpy(outputdir, value, VALSIZE);
			break;
		case 2:
			strncpy(logtype, value, VALSIZE);
			break;
		case 3:
			strncpy(logstore, value, VALSIZE);
			break;
		case 4:
			strncpy(domainsfile, value, VALSIZE);
			break;
		case 5:
			strncpy(langfile, value, VALSIZE);
			break;
		case 6:
			strncpy(htmldir, value, VALSIZE);
			break;
		case 7:
			strncpy(hostname, value, VALSIZE);
			break;
		case 8:
			maxsender = atoi(value);
			break;
		case 9:
			maxreceiver = atoi(value);
			break;
		case 10:
			maxtotal = atoi(value);
			break;
		case 11:
			maxbyte = atoi(value);
			break;
		}
	}
	fclose(fd);
}

void readconfig(char *cfgfile)
{
	FILE *fd;
	char buf[1024];
	int count = 0; /*counter for domains */
	struct stat statbuf;
	char *dir;

	time(&today);
	t = localtime(&today);

	cur_year = t->tm_year + 1900;
	cur_month = t->tm_mon + 1;
	cur_day = t->tm_mday ;
	cur_hour = t->tm_hour;
	cur_min = t->tm_min;
	cur_sec = t->tm_sec;
	printf("Year: %d Month: %d \n",cur_year, cur_month);


	loadconfig(cfgfile);

	printf("outputdir:%s\n", outputdir);
	printf("htmldir:%s\n", htmldir);
	printf("logtype:%s\n", logtype);
	printf("logstore:%s\n", logstore);
	printf("langfile:%s\n", langfile);
	printf("maxsender:%d\n", maxsender);
	printf("maxreceiver:%d\n", maxreceiver);
	printf("maxtotal:%d\n", maxtotal);
	printf("maxbyte:%d\n", maxbyte);
	
	if ((strlen(hostname)) <= 0 )
        	gethostname(hostname, VALSIZE);
	        printf("hostname: %s\n", hostname);

	if ((strcasecmp(logtype, "qmail-multilog") != 0) && (strcasecmp(logtype, "qmail-syslog") != 0)
			 && (strcasecmp(logtype, "sendmail") != 0) && (strcasecmp(logtype, "postfix") != 0)) {
		printf("Invalid logtype: %s\naccepted logytpes are: "
		        "qmail-multilog, qmail-syslog, sendmail or postfix\n", logtype);
                exit(-1);
	}
	if ((strcasecmp(logtype, "qmail-multilog")) == 0) {
		if ((stat(logstore, &statbuf)) == 0) {
			if((S_ISDIR(statbuf.st_mode)) == 0) {
				fprintf(stderr, "You are using: %s log type "
						"logstore: %s  must be a directory\n", logtype, logstore);
				exit(-1);
			}
		}
		else {
			fprintf(stderr, "You are using: %s log type "
					"logstore: %s directory does not exist!\n", logtype, logstore);
			exit(-1);
		}
	}
	else {
		if ((stat(logstore, &statbuf)) == 0) {
			if((S_ISREG(statbuf.st_mode)) == 0) {
				fprintf(stderr, "You are using: %s log type "
						"logstore: %s  must be a regular file\n", logtype, logstore);
				exit(-1);
			}
		}
		else {
			fprintf(stderr, "You are using: %s log type "
					"logstore: %s file does not exist!\n", logtype, logstore);
			exit(-1);
		}
	}
	if ((strlen(outputdir)) <= 0 ) {
		fprintf(stderr, "You must define output directory");
		exit(-1);
	}
	if ((strlen(domainsfile)) <= 0 ) {
		fprintf(stderr, "You must define domainsfile");
		exit(-1);
	}
	createdir(outputdir);
	if ((fd = fopen(domainsfile, "r")) == NULL) {
		fprintf(stderr, "domainsfile: %s could not be opened\n", domainsfile);
		exit(-1);
	}
	while ((fgets(buf, 1024, fd)) != NULL) {
		 printf("Domains %s", buf);
		 removespaces(buf);
		  if (strlen(buf) <= 0)
			  ;
		   else {
			   lowercase(buf);  /* lowercase domains */
			   addDomain(buf);
			   dir = malloc((strlen(outputdir) + strlen(buf) + 10) * sizeof(char));
			   sprintf(dir, "%s/%s", outputdir, buf);
			   createdir(dir); /*  create domain directory */

			   sprintf(dir, "%s/%s/%d", outputdir, buf, cur_year);
			   createdir(dir); /*  create year dir under domain directory */

			   sprintf(dir, "%s/%s/%d/%d", outputdir, buf, cur_year, cur_month);
			   createdir(dir); /*  create month directory */

			   count++;
		   }
	}
	if (count == 0) {
		fprintf(stderr, "Empty domains file. you must add at least one domain\n");
		exit(-1);
	}
   sprintf(dir, "%s/%s", outputdir, "general");
   createdir(dir); /*  create domain directory */

   sprintf(dir, "%s/%s/%d", outputdir, "general" , cur_year);
   createdir(dir); /*  create year dir under domain directory */

   sprintf(dir, "%s/%s/%d/%d", outputdir, "general", cur_year, cur_month);
   createdir(dir); /*  create month directory */

   free(dir);



}

void removespaces(char *p)
{
	char *l;
	char *q;
	char *lp;

	l = strdup(p);
	q = p;
	lp = l;
	while (*l != '\0') {
		if (isspace(*l) || *l == '\n') {
			l++;
			continue;
		}
		*q++ = *l++;
	}
	*q = '\0';
	free(lp);
}

