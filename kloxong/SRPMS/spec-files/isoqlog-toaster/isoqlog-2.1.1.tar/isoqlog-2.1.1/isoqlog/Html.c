#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/times.h>
#include <errno.h>

#include "Html.h"
#include "Data.h"
#include "Store.h"
#include "LangCfg.h"
#include "Dir.h"
#include "Global.h"

extern char outputdir [VALSIZE];
extern char htmldir [VALSIZE];
extern char language [VALSIZE];
extern int maxsender;
extern int maxreceiver;
extern int maxtotal;
extern int maxbyte;
extern char *months[12];

/* language file variables */

extern char L_Average[VALSIZE];
extern char L_Creationtime[VALSIZE];
extern char L_Daily[VALSIZE];
extern char L_Day[VALSIZE];
extern char L_Domain[VALSIZE];
extern char L_Encoding[VALSIZE];
extern char L_Mail[VALSIZE];
extern char L_Main_Page[VALSIZE];
extern char L_Month[VALSIZE];
extern char L_Monthly[VALSIZE];
extern char L_Number[VALSIZE];
extern char L_Receiver[VALSIZE];
extern char L_Received[VALSIZE];
extern char L_Sender[VALSIZE];
extern char L_Sent[VALSIZE];
extern char L_Size[VALSIZE];
extern char L_Stats[VALSIZE];
extern char L_Top[VALSIZE];
extern char L_Total[VALSIZE];
extern char L_Yearly[VALSIZE];
extern char L_Year[VALSIZE];

void createHtml()
{
	char *fname;
	domain *d;
	int i;

	fname = (char *)calloc(1024, sizeof(char));

	for (i = 0; i < DomainsTab.nval; i++) {
		d = &DomainsTab.alldomains[i];
		sprintf(fname, "%s/%s/%d/%d/%d.html",
				outputdir,
				d->domain,
				cur_year,
				cur_month,
				cur_day);
		createDailyHtml(fname, d);
		sprintf(fname, "%s/%s/%d/%d/",
				outputdir,
				d->domain,
				cur_year,
				cur_month);
		createMonthlyHtml(fname, d->domain);
		sprintf(fname, "%s/%s/%d/",
				outputdir,
				d->domain,
				cur_year);
		createYearlyHtml(fname, d->domain);
		sprintf(fname, "%s/%s/",
				outputdir,
				d->domain);
		createDomainHtml(fname, d->domain);
	}
	/* Generals	*/
	sprintf(fname, "%s/general/%d/%d/%d.html",
			outputdir,
			cur_year,
			cur_month,
			cur_day);
	createGeneralDailyHtml(fname);
	sprintf(fname, "%s/general/%d/%d/",
			outputdir,
			cur_year,
			cur_month);
	createGeneralMonthlyHtml(fname);
	sprintf(fname, "%s/%s/%d/",
			outputdir,
			"general",
			cur_year);
	createGeneralYearlyHtml(fname);
	sprintf(fname, "%s/general/",
			outputdir);
	createGeneralHtml(fname);

	sprintf(fname, "%s/index.html",outputdir);
	createIndexHtml(fname);
	
	free(fname);
}


void createDailyHtml(char *f, domain *d)
{
	FILE *fp;
	FILE *tp;
	int i;
	user *u;
	int max;
	double byte = 0.0;
	char *tmpstr, *substr, *newstr;
	char *tfmt[2] = { "%s: %s %d, %d / %d:0%d",  
			  "%s: %s %d, %d / %d:%d" };


	tmpstr = (char *)calloc(1024, sizeof(char));
	substr = (char *)calloc(512, sizeof(char));
	newstr = (char *)calloc(512, sizeof(char));
	if ((fp = fopen(f, "w")) == NULL) {
		fprintf(stderr, "createDailyHtml: cannot create daily html output %s: %s\n", f, strerror(errno));
		exit(-1);
	}

	sprintf(tmpstr, "%s/daily.html", htmldir);

	if ((tp = fopen(tmpstr, "r")) == NULL) {
		fprintf(stderr, "createDailyHtml: cannot open daily template file %s: %s\n", tmpstr, strerror(errno));
		exit(-1);
	}

	while(fgets(tmpstr, 1024, tp) != NULL) {
		if ((substr = strstr(tmpstr, "<!--%d-->")) != NULL) {
			fwrite(tmpstr, sizeof(char), (strlen(tmpstr) - strlen(substr)), fp);
                        sprintf(newstr, "%s", d->domain);
                        fwrite(newstr, sizeof(char), strlen(newstr), fp);
                        fwrite(substr + 9, sizeof(char), strlen(substr) - 9, fp);
                        memset(substr, 0, 512);
                        memset(newstr, 0, 512);
		}
		else if ((substr = strstr(tmpstr, "<!--%s-->")) != NULL) {
			fwrite(tmpstr, sizeof(char), (strlen(tmpstr) - strlen(substr)), fp);
			sprintf(newstr, "%s %d, %d %s", months[cur_month - 1], cur_day, cur_year, L_Stats);
                        fwrite(newstr, sizeof(char), strlen(newstr), fp);
                        fwrite(substr + 9, sizeof(char), strlen(substr) - 9, fp);
                        memset(substr, 0, 512);
                        memset(newstr, 0, 512);
		}
		else if ((substr = strstr(tmpstr, "<!--%c-->")) != NULL) {
			fwrite(tmpstr, sizeof(char), (strlen(tmpstr) - strlen(substr)), fp);
			sprintf(newstr, tfmt[(cur_min > 9 ? 1 : 0)],
						L_Creationtime,
						months[cur_month - 1], 
						cur_day,
						cur_year,
						cur_hour, 
						cur_min);
                        fwrite(newstr, sizeof(char), strlen(newstr), fp);
                        fwrite(substr + 9, sizeof(char), strlen(substr) - 9, fp);
                        memset(substr, 0, 512);
                        memset(newstr, 0, 512);
		}
		else if ((substr = strstr(tmpstr, "<!--%1-->")) != NULL) {
			fprintf(fp, "<table width=\"80%%\" border='1' cellspacing='0' cellpadding='2'" 
				     "bordercolor='#F4F4F4'>"
				     "<tr align='left' valign='middle'>"
				     "<td class='table_top_header' colspan='3'>%s %d %s</td>\n"
				     "</tr>"
				     "<tr align='left' valign='middle' class='table_header'>\n"
				     "<td class='table_header' align='center' width=\"7%%\"></td>\n"
				     "<td class='table_header' align='center' valign='middle' width=\"80%%\">%s</td>\n"
				"<td class='table_header' align='center' valign='middle' width=\"13%%\">%s</td>\n"
 				"</tr>", L_Top, maxsender,  L_Sender, L_Mail, L_Number);
			sortDomainUsersFrom(d->domain);
			max = (SortDomain.nval < maxsender ? SortDomain.nval : maxsender);
			for (i = 0; i < max; i++) {
				u = (user *)SortDomain.allusers[i];
				if (u->from_cnt == 0)
					break;
				fprintf(fp, "<tr align='center' valign='middle'>");
				fprintf(fp, "<td width=\"7%%\" class='table_data'>%d</td>\n"
					"<td class='table_data' width=\"80%%\">%s</td>\n"
				        "<td class='table_data' width=\"13%%\">%d</td>\n",
							i + 1,
							u->user,
							u->from_cnt);
				fprintf(fp, "</tr>\n");
			}
			fprintf(fp, "</TABLE>\n");
			freeSortDomainTab();
		}
		else if ((substr = strstr(tmpstr, "<!--%2-->")) != NULL) {
			fprintf(fp, "<table width=\"80%%\" border='1' cellspacing='0' cellpadding='2'" 
				     "bordercolor='#F4F4F4'>"
				     "<tr align='left' valign='middle'>"
				     "<td class='table_top_header' colspan='3'>%s %d %s</td>\n"
				     "</tr>"
				     "<tr align='left' valign='middle' class='table_header'>\n"
				     "<td class='table_header' align='center' width=\"7%%\"></td>\n"
				     "<td class='table_header' align='center' valign='middle' width=\"80%%\">%s</td>\n"
				"<td class='table_header' align='center' valign='middle' width=\"13%%\">%s</td>\n"
 				"</tr>", L_Top, maxsender, L_Receiver, L_Mail, L_Number);
			sortDomainUsersTo(d->domain);
			max = (SortDomain.nval < maxreceiver ? SortDomain.nval : maxreceiver);
			for (i = 0; i < max; i++) {
				u = (user *)SortDomain.allusers[i];
				if (u->to_cnt == 0)
					break;
				fprintf(fp, "<tr align='center' valign='middle'>");
				fprintf(fp, "<td width=\"7%%\" class='table_data'>%d</td>\n"
					"<td class='table_data' width=\"80%%\">%s</td>\n"
				        "<td class='table_data' width=\"13%%\">%d</td>\n",
							i + 1,
							u->user,
							u->to_cnt);
				fprintf(fp, "</tr>\n");
			}
			fprintf(fp, "</TABLE>");
			freeSortDomainTab();
		}
		else if ((substr = strstr(tmpstr, "<!--%3-->")) != NULL) {
			fprintf(fp, "<table width=\"80%%\" border='1' cellspacing='0' cellpadding='2'" 
				     "bordercolor='#F4F4F4'>"
				     "<tr align='left' valign='middle'>"
				     "<td class='table_top_header' colspan='3'>%s %d %s</td>\n"
				     "</tr>"
				     "<tr align='left' valign='middle' class='table_header'>\n"
				     "<td class='table_header' align='center' width=\"7%%\"></td>\n"
				     "<td class='table_header' align='center' valign='middle' width=\"80%%\">%s</td>\n"
				"<td class='table_header' align='center' valign='middle' width=\"13%%\">%s</td>\n"
 				"</tr>", L_Top, maxsender, L_Size, L_Mail, L_Size);
			sortDomainUsersByte(d->domain);
			max = (SortDomain.nval < maxbyte ? SortDomain.nval : maxbyte);
			for (i = 0; i < max; i++) {
				u = (user *)SortDomain.allusers[i];
				if (u->from_byte == 0)
					break;
				getSizeStr(u->from_byte, tmpstr, &byte);
				fprintf(fp, "<tr align='center' valign='middle'>");
				fprintf(fp, "<td width=\"7%%\" class='table_data'>%d</td>\n"
					"<td class='table_data' width=\"80%%\">%s</td>\n"
				        "<td class='table_data' width=\"13%%\">%.2f %s</td>\n",
							i + 1,
							u->user,
							byte,
							tmpstr);
				fprintf(fp, "</tr>\n");
			}
			fprintf(fp, "</TABLE>");
			freeSortDomainTab();
		}
		else if ((substr = strstr(tmpstr, "<!--%4-->")) != NULL) {
			fprintf(fp, "<table width=\"80%%\" border='1' cellspacing='0' cellpadding='2'" 
				     "bordercolor='#F4F4F4'>"
				     "<tr align='left' valign='middle'>"
				     "<td class='table_top_header' colspan='3'>%s %d %s</td>\n"
				     "</tr>"
				     "<tr align='left' valign='middle' class='table_header'>\n"
				     "<td class='table_header' align='center' width=\"7%%\"></td>\n"
				     "<td class='table_header' align='center' valign='middle' width=\"80%%\">%s</td>\n"
				"<td class='table_header' align='center' valign='middle' width=\"13%%\">%s</td>\n"
 				"</tr>", L_Top, maxtotal, L_Total, L_Mail, L_Number);
			sortDomainUsersTotal(d->domain);
			max = (SortDomain.nval < maxtotal ? SortDomain.nval : maxtotal);
			for (i = 0; i < max; i++) {
				u = (user *)SortDomain.allusers[i];
				if ((u->to_cnt + u->from_cnt) == 0)
					break;
				fprintf(fp, "<tr align='center' valign='middle'>");
				fprintf(fp, "<td width=\"7%%\" class='table_data'>%d</td>\n"
					"<td class='table_data' width=\"80%%\">%s</td>\n"
				        "<td class='table_data' width=\"13%%\">%d</td>\n",
							i + 1,
							u->user,
							(u->from_cnt + u->to_cnt));
				fprintf(fp, "</tr>\n");
	
			}
			fprintf(fp, "</TABLE>");
			freeSortDomainTab();
		} else
			fwrite(tmpstr, sizeof(char), strlen(tmpstr), fp);
		memset(tmpstr, 0, 1024);
	}

	fprintf(fp, "<p><font face='Verdana, Arial, Helvetica, sans-serif' size='2' color='#666600'>"
			"Generated by <a href='http://www.enderunix.org/isoqlog/' target='_blank'>isoqlog</a>"
			" version 2.1.1 <a href='http://www.enderunix.org/' target='_blank'>"
			"EnderUNIX Software Development Team<a/></font></p>");

	sprintf(tmpstr, "%s/%s/%d/%d/AllDays", outputdir, d->domain, cur_year, cur_month);
	storeCurrentDayHistory(tmpstr, d);

	free(tmpstr);
	free(substr);
	free(newstr);

	fclose(fp);
	fclose(tp);
}

void createMonthlyHtml(char *d, char *dmn)
{
	FILE *fp, *tp;
	char *tmp, *tmpstr, *newstr, *substr;
	int i;
	double byte = 0.0, t_b = 0.0;
	int t_s, t_r;
	int c = 0; /* counter for days  to calculate average*/
	hist h[MAXDAYS];
	char *tfmt[2] = { "%s: %s %d, %d / %d:0%d",  
			  "%s: %s %d, %d / %d:%d" };

	tmp = (char *)calloc(1024, sizeof(char));
	tmpstr = (char *)calloc(1024, sizeof(char));
	newstr = (char *)calloc(1024, sizeof(char));
	substr = (char *)calloc(1024, sizeof(char));

	t_s = t_r = 0;

	sprintf(tmp, "%s/index.html", d);
	if ((fp = fopen(tmp, "w")) == NULL) {
		fprintf(stderr, "createMonthlyHtml: cannot create months file %s: %s\n", tmp, strerror(errno));
		exit(-1);
	}

	sprintf(tmp, "%s/days.html", htmldir);
	if ((tp = fopen(tmp, "r")) == NULL) {
		fprintf(stderr, "createDailyHtml: cannot open days template file %s: %s\n", tmpstr, strerror(errno));
		exit(-1);
	}

	sprintf(tmp, "%s/AllDays", d);
	getHistData(tmp, h, sizeof(h));

	while(fgets(tmpstr, 1024, tp) != NULL) {
		if ((substr = strstr(tmpstr, "<!--%d-->")) != NULL) {
			fwrite(tmpstr, sizeof(char), (strlen(tmpstr) - strlen(substr)), fp);
                        sprintf(newstr, "%s", dmn);
                        fwrite(newstr, sizeof(char), strlen(newstr), fp);
                        fwrite(substr + 9, sizeof(char), strlen(substr) - 9, fp);
                        memset(substr, 0, 512);
                        memset(newstr, 0, 512);
		}
		else if ((substr = strstr(tmpstr, "<!--%s-->")) != NULL) {
			fwrite(tmpstr, sizeof(char), (strlen(tmpstr) - strlen(substr)), fp);
			sprintf(newstr, "%s, %d %s", months[cur_month - 1], cur_year, L_Stats);
                        fwrite(newstr, sizeof(char), strlen(newstr), fp);
                        fwrite(substr + 9, sizeof(char), strlen(substr) - 9, fp);
                        memset(substr, 0, 512);
                        memset(newstr, 0, 512);
		}
		else if ((substr = strstr(tmpstr, "<!--%c-->")) != NULL) {
			fwrite(tmpstr, sizeof(char), (strlen(tmpstr) - strlen(substr)), fp);
			sprintf(newstr, tfmt[(cur_min > 9 ? 1 : 0)],
						L_Creationtime,
						months[cur_month - 1], 
						cur_day,
						cur_year,
						cur_hour, 
						cur_min);
                        fwrite(newstr, sizeof(char), strlen(newstr), fp);
                        fwrite(substr + 9, sizeof(char), strlen(substr) - 9, fp);
                        memset(substr, 0, 512);
                        memset(newstr, 0, 512);
		}
		else if ((substr = strstr(tmpstr, "<!--%1-->")) != NULL) {
			fprintf(fp, "<table width='80%%' border='1' cellspacing='0' cellpadding='2' bordercolor='#F4F4F4'>");
			fprintf(fp, "<tr align='left' valign='middle' class='table_header'>"
				"<td width='20%%' class='table_header' align='center' valign='middle'>%s</td>"
				"<td width='20%%' class='table_header' align='center' valign='middle'>%s</td>"
				"<td width='20%%' class='table_header' align='center' valign='middle'>%s</td>"
				"<td width='20%%' class='table_header' align='center' valign='middle'>%s</td>"
				"<td width='20%%' class='table_header' align='center' valign='middle'>%s</td>"
				"</tr>\n", L_Day, L_Sent, L_Received, L_Total, L_Size );
			for (i = 0; i < MAXDAYS; i++) {
				if ((h[i].sent + h[i].received) == 0)
					continue;
				getSizeStr(h[i].bytes_sent, tmpstr, &byte);
				t_s += h[i].sent;
				t_r += h[i].received;
				t_b += h[i].bytes_sent;
				c++;

				fprintf(fp, "<tr align='left' valign='middle'>"
				"<td width='20%%' class='table_data_day' align='center' valign='middle'>"
				"<a href='%d.html'>%d</a></td>"
				"<td width='20%%' class='table_data' align='center' valign='middle'>%d</td>"
				"<td width='20%%' class='table_data' align='center' valign='middle'>%d</td>"
				"<td width='20%%' class='table_data' align='center' valign='middle'>%d</td>"
				"<td width='20%%' class='table_data' align='center' valign='middle'>%.2f %s</td>"
				"</tr>\n",
						i + 1,
						i + 1,
						h[i].sent,
						h[i].received,
						(h[i].sent + h[i].received),
						byte,
						tmpstr);
			}
			memset(tmpstr, 0, 1024);
			getSizeStr(t_b, tmpstr, &byte);
			fprintf(fp, "<tr align='left' valign='middle'>"
				"<td width='20%%' class='table_foother' align='center' valign='middle'><b>%s</b></a>"
				"<td width='20%%' class='table_data_total' align='center' valign='middle'>%d</td>"
				"<td width='20%%' class='table_data_total' align='center' valign='middle'>%d</td>"
				"<td width='20%%' class='table_data_total' align='center' valign='middle'>%d</td>"
				"<td width='20%%' class='table_data_total' align='center' valign='middle'>%.2f %s</td>"
				"</tr>\n",
					L_Total,
					t_s,
					t_r,
					t_s + t_r,
					byte,
					tmpstr);
			if (c == 0)
				c = 1;
			getSizeStr(t_b/c, tmpstr, &byte);
		fprintf(fp, "<tr align='left' valign='middle'>"
				"<td width='20%%' class='table_foother' align='center' valign='middle'><b>%s</b></a>"
				"<td width='20%%' class='table_data_total' align='center' valign='middle'>%d</td>"
				"<td width='20%%' class='table_data_total' align='center' valign='middle'>%d</td>"
				"<td width='20%%' class='table_data_total' align='center' valign='middle'>%d</td>"
				"<td width='20%%' class='table_data_total' align='center' valign='middle'>%.2f %s</td>"
				"</tr>\n",
					L_Average,
					t_s/c,
					t_r/c,
					(t_s + t_r)/c,
					byte,
					tmpstr);
		
			fprintf(fp, "</TABLE>");
		}
		else 
			fwrite(tmpstr, sizeof(char), strlen(tmpstr), fp);
		memset(tmpstr, 0, 1024);
	}

	fprintf(fp, "<p><font face='Verdana, Arial, Helvetica, sans-serif' size='2' color='#666600'>"
			"Generated by <a href='http://www.enderunix.org/isoqlog/' target='_blank'>isoqlog</a>"
			" version 2.1.1 <a href='http://www.enderunix.org/' target='_blank'>"
			"EnderUNIX Software Development Team<a/></font></p>");

	sprintf(tmpstr, "%s/%s/%d/AllMonths", outputdir, dmn, cur_year);
	/* storeCurrentMonthHistory(tmpstr, t_s, t_r, byte); */
	storeCurrentMonthHistory(tmpstr, t_s, t_r, t_b);
	free(tmp);
	free(tmpstr);
	free(substr);
	free(newstr);
	fclose(fp);
	fclose(tp);

}

void createYearlyHtml(char *d, char *dmn)
{
	FILE *fp, *tp;
	char *tmp, *tmpstr, *newstr, *substr;
	int i;
	int t_s, t_r;
	int c = 0; /* average counter */
	double byte = 0.0, t_b = 0.0;
	hist h[MAXMONTHS];
	char *tfmt[2] = { "%s: %s %d, %d / %d:0%d",  
			  "%s: %s %d, %d / %d:%d" };

	tmp = (char *)calloc(1024, sizeof(char));
	tmpstr = (char *)calloc(1024, sizeof(char));
	newstr = (char *)calloc(1024, sizeof(char));
	substr = (char *)calloc(1024, sizeof(char));

	t_s = t_r = 0;

	sprintf(tmp, "%s/index.html", d);
	if ((fp = fopen(tmp, "w")) == NULL) {
		fprintf(stderr, "createYearlyHtml: cannot create months file %s: %s\n", tmp, strerror(errno));
		exit(-1);
	}

	sprintf(tmp, "%s/monthly.html", htmldir);
	if ((tp = fopen(tmp, "r")) == NULL) {
		fprintf(stderr, "createYearHtml: cannot open daily template file %s: %s\n", tmpstr, strerror(errno));
		exit(-1);
	}

	sprintf(tmp, "%s/AllMonths", d);
	getHistData(tmp, h, sizeof(h));

	while(fgets(tmpstr, 1024, tp) != NULL) {
		if ((substr = strstr(tmpstr, "<!--%d-->")) != NULL) {
			fwrite(tmpstr, sizeof(char), (strlen(tmpstr) - strlen(substr)), fp);
                        sprintf(newstr, "%s", dmn);
                        fwrite(newstr, sizeof(char), strlen(newstr), fp);
                        fwrite(substr + 9, sizeof(char), strlen(substr) - 9, fp);
                        memset(substr, 0, 512);
                        memset(newstr, 0, 512);
		}
		else if ((substr = strstr(tmpstr, "<!--%s-->")) != NULL) {
			fwrite(tmpstr, sizeof(char), (strlen(tmpstr) - strlen(substr)), fp);
			sprintf(newstr, "%d %s", cur_year, L_Stats);
                        fwrite(newstr, sizeof(char), strlen(newstr), fp);
                        fwrite(substr + 9, sizeof(char), strlen(substr) - 9, fp);
                        memset(substr, 0, 512);
                        memset(newstr, 0, 512);
		}
		else if ((substr = strstr(tmpstr, "<!--%c-->")) != NULL) {
			fwrite(tmpstr, sizeof(char), (strlen(tmpstr) - strlen(substr)), fp);
			sprintf(newstr, tfmt[(cur_min > 9 ? 1 : 0)],
						L_Creationtime,
						months[cur_month - 1], 
						cur_day,
						cur_year,
						cur_hour, 
						cur_min);
                        fwrite(newstr, sizeof(char), strlen(newstr), fp);
                        fwrite(substr + 9, sizeof(char), strlen(substr) - 9, fp);
                        memset(substr, 0, 512);
                        memset(newstr, 0, 512);
		}
		else if ((substr = strstr(tmpstr, "<!--%1-->")) != NULL) {
			fprintf(fp, "<table width='80%%' border='1' cellspacing='0' cellpadding='2' bordercolor='#F4F4F4'>");
			fprintf(fp, "<tr align='left' valign='middle' class='table_header'>"
				"<td width='20%%' class='table_header' align='center' valign='middle'>%s</td>"
				"<td width='20%%' class='table_header' align='center' valign='middle'>%s</td>"
				"<td width='20%%' class='table_header' align='center' valign='middle'>%s</td>"
				"<td width='20%%' class='table_header' align='center' valign='middle'>%s</td>"
				"<td width='20%%' class='table_header' align='center' valign='middle'>%s</td>"
				"</tr>\n", L_Month, L_Sent, L_Received, L_Total, L_Size );
			for (i = 0; i < MAXMONTHS; i++) {
				if ((h[i].sent + h[i].received) == 0)
					continue;
				getSizeStr(h[i].bytes_sent, tmpstr, &byte);
				t_s += h[i].sent;
				t_r += h[i].received;
				t_b += h[i].bytes_sent;
				c++;

				fprintf(fp, "<tr align='left' valign='middle'>"
				"<td width='20%%' class='table_data_day' align='center' valign='middle'>"
				"<a href='%d/'>%d</a></td>"
				"<td width='20%%' class='table_data' align='center' valign='middle'>%d</td>"
				"<td width='20%%' class='table_data' align='center' valign='middle'>%d</td>"
				"<td width='20%%' class='table_data' align='center' valign='middle'>%d</td>"
				"<td width='20%%' class='table_data' align='center' valign='middle'>%.2f %s</td>"
				"</tr>\n",
						i + 1,
						i + 1,
						h[i].sent,
						h[i].received,
						(h[i].sent + h[i].received),
						byte,
						tmpstr);
			}
			memset(tmpstr, 0, 1024);
			getSizeStr(t_b, tmpstr, &byte);
			fprintf(fp, "<tr align='left' valign='middle'>"
				"<td width='20%%' class='table_foother' align='center' valign='middle'><b>%s</b></a>"
				"<td width='20%%' class='table_data_total' align='center' valign='middle'>%d</td>"
				"<td width='20%%' class='table_data_total' align='center' valign='middle'>%d</td>"
				"<td width='20%%' class='table_data_total' align='center' valign='middle'>%d</td>"
				"<td width='20%%' class='table_data_total' align='center' valign='middle'>%.2f %s</td>"
				"</tr>\n",
					L_Total,
					t_s,
					t_r,
					t_s + t_r,
					byte,
					tmpstr);

			if (c == 0) /* do not divide zero */
				c = 1;
			getSizeStr(t_b/c, tmpstr, &byte);
			fprintf(fp, "<tr align='left' valign='middle'>"
				"<td width='20%%' class='table_foother' align='center' valign='middle'><b>%s</b></a>"
				"<td width='20%%' class='table_data_total' align='center' valign='middle'>%d</td>"
				"<td width='20%%' class='table_data_total' align='center' valign='middle'>%d</td>"
				"<td width='20%%' class='table_data_total' align='center' valign='middle'>%d</td>"
				"<td width='20%%' class='table_data_total' align='center' valign='middle'>%.2f %s</td>"
				"</tr>\n",
					L_Average,
					t_s/c,
					t_r/c,
					(t_s + t_r)/c,
					byte,
					tmpstr);
			
			fprintf(fp, "</TABLE>");
		}
		else 
			fwrite(tmpstr, sizeof(char), strlen(tmpstr), fp);
		memset(tmpstr, 0, 1024);
	}

	fprintf(fp, "<p><font face='Verdana, Arial, Helvetica, sans-serif' size='2' color='#666600'>"
			"Generated by <a href='http://www.enderunix.org/isoqlog/' target='_blank'>isoqlog</a>"
			" version 2.1.1 <a href='http://www.enderunix.org/' target='_blank'>"
			"EnderUNIX Software Development Team<a/></font></p>");

	sprintf(tmpstr, "%s/%s/AllYears", outputdir, dmn);
	storeCurrentYearHistory(tmpstr, t_s, t_r, t_b);
	free(tmp);
	free(tmpstr);
	free(substr);
	free(newstr);
	fclose(fp);
	fclose(tp);

}

void createDomainHtml(char *d, char *dmn)
{
	FILE *fp, *tp;
	char *tmp, *tmpstr, *newstr, *substr;
	int i;
	int t_s, t_r;
	double byte = 0.0, t_b = 0.0;
	int c = 0; /* average counter */
	hist h[MAXYEARS];
	char *tfmt[2] = { "%s: %s %d, %d / %d:0%d",  
			  "%s: %s %d, %d / %d:%d" };

	tmp = (char *)calloc(1024, sizeof(char));
	tmpstr = (char *)calloc(1024, sizeof(char));
	newstr = (char *)calloc(1024, sizeof(char));
	substr = (char *)calloc(1024, sizeof(char));

	t_s = t_r = 0;

	sprintf(tmp, "%s/index.html", d);
	if ((fp = fopen(tmp, "w")) == NULL) {
		fprintf(stderr, "createDomainHtml: cannot create months file %s: %s\n", tmp, strerror(errno));
		exit(-1);
	}

	sprintf(tmp, "%s/domain.html", htmldir);
	if ((tp = fopen(tmp, "r")) == NULL) {
		fprintf(stderr, "createDomainHtml: cannot open daily template file %s: %s\n", tmpstr, strerror(errno));
		exit(-1);
	}

	sprintf(tmp, "%s/AllYears", d);
	getHistData(tmp, h, sizeof(h));

	while(fgets(tmpstr, 1024, tp) != NULL) {
		if ((substr = strstr(tmpstr, "<!--%d-->")) != NULL) {
			fwrite(tmpstr, sizeof(char), (strlen(tmpstr) - strlen(substr)), fp);
                        sprintf(newstr, "%s", dmn);
                        fwrite(newstr, sizeof(char), strlen(newstr), fp);
                        fwrite(substr + 9, sizeof(char), strlen(substr) - 9, fp);
                        memset(substr, 0, 512);
                        memset(newstr, 0, 512);
		}
		else if ((substr = strstr(tmpstr, "<!--%s-->")) != NULL) {
			fwrite(tmpstr, sizeof(char), (strlen(tmpstr) - strlen(substr)), fp);
			sprintf(newstr, "%s %s", L_Yearly, L_Stats);
                        fwrite(newstr, sizeof(char), strlen(newstr), fp);
                        fwrite(substr + 9, sizeof(char), strlen(substr) - 9, fp);
                        memset(substr, 0, 512);
                        memset(newstr, 0, 512);
		}
		else if ((substr = strstr(tmpstr, "<!--%c-->")) != NULL) {
			fwrite(tmpstr, sizeof(char), (strlen(tmpstr) - strlen(substr)), fp);
			sprintf(newstr, tfmt[(cur_min > 9 ? 1 : 0)],
						L_Creationtime,
						months[cur_month - 1], 
						cur_day,
						cur_year,
						cur_hour, 
						cur_min);
                        fwrite(newstr, sizeof(char), strlen(newstr), fp);
                        fwrite(substr + 9, sizeof(char), strlen(substr) - 9, fp);
                        memset(substr, 0, 512);
                        memset(newstr, 0, 512);
		}
		else if ((substr = strstr(tmpstr, "<!--%1-->")) != NULL) {
			fprintf(fp, "<table width='80%%' border='1' cellspacing='0' cellpadding='2' bordercolor='#F4F4F4'>");
			fprintf(fp, "<tr align='left' valign='middle' class='table_header'>"
				"<td width='20%%' class='table_header' align='center' valign='middle'>%s</td>"
				"<td width='20%%' class='table_header' align='center' valign='middle'>%s</td>"
				"<td width='20%%' class='table_header' align='center' valign='middle'>%s</td>"
				"<td width='20%%' class='table_header' align='center' valign='middle'>%s</td>"
				"<td width='20%%' class='table_header' align='center' valign='middle'>%s</td>"
				"</tr>\n", L_Year, L_Sent, L_Received, L_Total, L_Size);
			for (i = 0; i < MAXYEARS; i++) {
				if ((h[i].sent + h[i].received) == 0)
					continue;
				getSizeStr(h[i].bytes_sent, tmpstr, &byte);
				t_s += h[i].sent;
				t_r += h[i].received;
				t_b += h[i].bytes_sent;
				c++;

				fprintf(fp, "<tr align='left' valign='middle'>"
				"<td width='20%%' class='table_data_day' align='center' valign='middle'>"
				"<a href='%d/'>%d</a></td>"
				"<td width='20%%' class='table_data' align='center' valign='middle'>%d</td>"
				"<td width='20%%' class='table_data' align='center' valign='middle'>%d</td>"
				"<td width='20%%' class='table_data' align='center' valign='middle'>%d</td>"
				"<td width='20%%' class='table_data' align='center' valign='middle'>%.2f %s</td>"
				"</tr>\n",
						i + 2002,
						i + 2002,
						h[i].sent,
						h[i].received,
						(h[i].sent + h[i].received),
						byte,
						tmpstr);
			}
			memset(tmpstr, 0, 1024);
			getSizeStr(t_b, tmpstr, &byte);
			fprintf(fp, "<tr align='left' valign='middle'>"
				"<td width='20%%' class='table_foother' align='center' valign='middle'><b>%s</b></a>"
				"<td width='20%%' class='table_data_total' align='center' valign='middle'>%d</td>"
				"<td width='20%%' class='table_data_total' align='center' valign='middle'>%d</td>"
				"<td width='20%%' class='table_data_total' align='center' valign='middle'>%d</td>"
				"<td width='20%%' class='table_data_total' align='center' valign='middle'>%.2f %s</td>"
				"</tr>\n",
					L_Total,
					t_s,
					t_r,
					t_s + t_r,
					byte,
					tmpstr);

			if (c == 0)
				c = 1;
			getSizeStr(t_b/c, tmpstr, &byte);
			fprintf(fp, "<tr align='left' valign='middle'>"
				"<td width='20%%' class='table_foother' align='center' valign='middle'><b>%s</b></a>"
				"<td width='20%%' class='table_data_total' align='center' valign='middle'>%d</td>"
				"<td width='20%%' class='table_data_total' align='center' valign='middle'>%d</td>"
				"<td width='20%%' class='table_data_total' align='center' valign='middle'>%d</td>"
				"<td width='20%%' class='table_data_total' align='center' valign='middle'>%.2f %s</td>"
				"</tr>\n",
					L_Average,
					t_s/c,
					t_r/c,
					(t_s + t_r)/c,
					byte,
					tmpstr);
			
			fprintf(fp, "</TABLE>");
		}
		else 
			fwrite(tmpstr, sizeof(char), strlen(tmpstr), fp);
		memset(tmpstr, 0, 1024);
	}

	fprintf(fp, "<p><font face='Verdana, Arial, Helvetica, sans-serif' size='2' color='#666600'>"
			"Generated by <a href='http://www.enderunix.org/isoqlog/' target='_blank'>isoqlog</a>"
			" version 2.1.1 <a href='http://www.enderunix.org/' target='_blank'>"
			"EnderUNIX Software Development Team<a/></font></p>");

	free(tmp);
	free(tmpstr);
	free(substr);
	free(newstr);
	fclose(fp);
	fclose(tp);

}

/* creates daily statics generaly,  /isolog/general/year/month/day.html */
void createGeneralDailyHtml(char *f)
{
	FILE *fp;
	FILE *tp;
	 double byte = 0.0 ;
	int i;
	user *u;
	domain *d;
	int max;
	char *tmpstr, *substr, *newstr;
	char *tfmt[2] = { "%s: %s %d, %d / %d:0%d",  
			  "%s: %s %d, %d / %d:%d" };


	tmpstr = (char *)calloc(1024, sizeof(char));
	substr = (char *)calloc(512, sizeof(char));
	newstr = (char *)calloc(512, sizeof(char));
	if ((fp = fopen(f, "w")) == NULL) {
		fprintf(stderr, "createGeneralDailyHtml: cannot create daily html output %s: %s\n", f, strerror(errno));
		exit(-1);
	}

	sprintf(tmpstr, "%s/generaldaily.html", htmldir);

	if ((tp = fopen(tmpstr, "r")) == NULL) {
		fprintf(stderr, "createGeneralDailyHtml: cannot open daily template file %s: %s\n", tmpstr, strerror(errno));
		exit(-1);
	}

	while(fgets(tmpstr, 1024, tp) != NULL) {
		if ((substr = strstr(tmpstr, "<!--%d-->")) != NULL) {
			fwrite(tmpstr, sizeof(char), (strlen(tmpstr) - strlen(substr)), fp);
                        sprintf(newstr, "%s", "General");
                        fwrite(newstr, sizeof(char), strlen(newstr), fp);
                        fwrite(substr + 9, sizeof(char), strlen(substr) - 9, fp);
                        memset(substr, 0, 512);
                        memset(newstr, 0, 512);
		}
		else if ((substr = strstr(tmpstr, "<!--%s-->")) != NULL) {
			fwrite(tmpstr, sizeof(char), (strlen(tmpstr) - strlen(substr)), fp);
			sprintf(newstr, "%s %d, %d %s", months[cur_month - 1], cur_day, cur_year, L_Stats);
                        fwrite(newstr, sizeof(char), strlen(newstr), fp);
                        fwrite(substr + 9, sizeof(char), strlen(substr) - 9, fp);
                        memset(substr, 0, 512);
                        memset(newstr, 0, 512);
		}
		else if ((substr = strstr(tmpstr, "<!--%c-->")) != NULL) {
			fwrite(tmpstr, sizeof(char), (strlen(tmpstr) - strlen(substr)), fp);
			sprintf(newstr, tfmt[(cur_min > 9 ? 1 : 0)],
						L_Creationtime,
						months[cur_month - 1], 
						cur_day,
						cur_year,
						cur_hour, 
						cur_min);
                        fwrite(newstr, sizeof(char), strlen(newstr), fp);
                        fwrite(substr + 9, sizeof(char), strlen(substr) - 9, fp);
                        memset(substr, 0, 512);
                        memset(newstr, 0, 512);
		}
		else if ((substr = strstr(tmpstr, "<!--%1-->")) != NULL) {
			fprintf(fp, "<table width=\"80%%\" border='1' cellspacing='0' cellpadding='2'" 
				     "bordercolor='#F4F4F4'>"
				     "<tr align='left' valign='middle'>"
				     "<td class='table_top_header' colspan='3'>%s %d %s %s</td>\n"
				     "</tr>"
				     "<tr align='left' valign='middle' class='table_header'>\n"
				     "<td class='table_header' align='center' width=\"7%%\"></td>\n"
				     "<td class='table_header' align='center' valign='middle' width=\"80%%\">%s</td>\n"
				"<td class='table_header' align='center' valign='middle' width=\"13%%\">%s</td>\n"
 				"</tr>", L_Top, maxsender,  L_Sender, L_Domain, L_Mail, L_Number);
			sortDomainsFrom();
			max = (DomainsTab.nval < maxsender ? DomainsTab.nval : maxsender);
			for (i = 0; i < max; i++) {
				d = &(DomainsTab.alldomains[i]);
				if (d->from_cnt == 0)
					break;
				fprintf(fp, "<tr align='center' valign='middle'>");
				fprintf(fp, "<td width=\"7%%\" class='table_data'>%d</td>\n"
					"<td class='table_data' width=\"80%%\">%s</td>\n"
				        "<td class='table_data' width=\"13%%\">%d</td>\n",
							i + 1,
							d->domain,
							d->from_cnt);
				fprintf(fp, "</tr>\n");
			}
			fprintf(fp, "</TABLE>\n");
		}
		else if ((substr = strstr(tmpstr, "<!--%2-->")) != NULL) {
			fprintf(fp, "<table width=\"80%%\" border='1' cellspacing='0' cellpadding='2'" 
				     "bordercolor='#F4F4F4'>"
				     "<tr align='left' valign='middle'>"
				     "<td class='table_top_header' colspan='3'>%s %d %s %s</td>\n"
				     "</tr>"
				     "<tr align='left' valign='middle' class='table_header'>\n"
				     "<td class='table_header' align='center' width=\"7%%\"></td>\n"
				     "<td class='table_header' align='center' valign='middle' width=\"80%%\">%s</td>\n"
				"<td class='table_header' align='center' valign='middle' width=\"13%%\">%s</td>\n"
 				"</tr>",L_Top, maxsender, L_Receiver, L_Domain, L_Mail, L_Number);
			sortDomainsTo();
			max = (DomainsTab.nval < maxreceiver ? DomainsTab.nval : maxreceiver);
			for (i = 0; i < max; i++) {
				d = &(DomainsTab.alldomains[i]);
				if (d->to_cnt == 0)
					break;
				fprintf(fp, "<tr align='center' valign='middle'>");
				fprintf(fp, "<td width=\"6%%\" class='table_data'>%d</td>\n"
					"<td class='table_data' width=\"80%%\">%s</td>\n"
				        "<td class='table_data' width=\"13%%\">%d</td>\n",
							i + 1,
							d->domain,
							d->to_cnt);
				fprintf(fp, "</tr>\n");
			}
			fprintf(fp, "</TABLE>");
		}
		else if ((substr = strstr(tmpstr, "<!--%3-->")) != NULL) {
			fprintf(fp, "<table width=\"80%%\" border='1' cellspacing='0' cellpadding='2'" 
				     "bordercolor='#F4F4F4'>"
				     "<tr align='left' valign='middle'>"
				     "<td class='table_top_header' colspan='3'>%s %d %s (%s)</td>\n"
				     "</tr>"
				     "<tr align='left' valign='middle' class='table_header'>\n"
				     "<td class='table_header' align='center' width=\"7%%\"></td>\n"
				     "<td class='table_header' align='center' valign='middle' width=\"80%%\">%s</td>\n"
				"<td class='table_header' align='center' valign='middle' width=\"13%%\">%s</td>\n"
 				"</tr>", L_Top, maxsender, L_Domain, L_Size, L_Mail, L_Size);
			sortDomainsByte();
			max = (DomainsTab.nval < maxbyte ? DomainsTab.nval : maxbyte);
			for (i = 0; i < max; i++) {
				d = &(DomainsTab.alldomains[i]);
				if (d->from_byte == 0)
					break;
				getSizeStr(d->from_byte, tmpstr, &byte);
				fprintf(fp, "<tr align='center' valign='middle'>");
				fprintf(fp, "<td width=\"7%%\" class='table_data'>%d</td>\n"
					"<td class='table_data' width=\"80%%\">%s</td>\n"
				        "<td class='table_data' width=\"13%%\">%.2f %s</td>\n",
							i + 1,
							d->domain,
							byte,
							tmpstr);
				fprintf(fp, "</tr>\n");
			}
			fprintf(fp, "</TABLE>");
		}
		else if ((substr = strstr(tmpstr, "<!--%4-->")) != NULL) {
			fprintf(fp, "<table width=\"80%%\" border='1' cellspacing='0' cellpadding='2'" 
				     "bordercolor='#F4F4F4'>"
				     "<tr align='left' valign='middle'>"
				     "<td class='table_top_header' colspan='3'>%s %d %s (%s)</td>\n"
				     "</tr>"
				     "<tr align='left' valign='middle' class='table_header'>\n"
				     "<td class='table_header' align='center' width=\"7%%\"></td>\n"
				     "<td class='table_header' align='center' valign='middle' width=\"80%%\">%s</td>\n"
				"<td class='table_header' align='center' valign='middle' width=\"13%%\">%s</td>\n"
 				"</tr>", L_Top, maxtotal, L_Domain, L_Total, L_Mail, L_Number);
			sortDomainsTotal();
			max = (DomainsTab.nval < maxtotal ? DomainsTab.nval : maxtotal);
			for (i = 0; i < max; i++) {
				d = &(DomainsTab.alldomains[i]);
				if ((d->to_cnt + d->from_cnt) == 0)
					break;
				fprintf(fp, "<tr align='center' valign='middle'>");
				fprintf(fp, "<td width=\"7%%\" class='table_data'>%d</td>\n"
					"<td class='table_data' width=\"80%%\">%s</td>\n"
				        "<td class='table_data' width=\"13%%\">%d</td>\n",
							i + 1,
							d->domain,
							(d->from_cnt + d->to_cnt));
				fprintf(fp, "</tr>\n");
	
			}
			fprintf(fp, "</TABLE>");
		}
		else if ((substr = strstr(tmpstr, "<!--%5-->")) != NULL) {
			fprintf(fp, "<table width=\"80%%\" border='1' cellspacing='0' cellpadding='2'" 
				     "bordercolor='#F4F4F4'>"
				     "<tr align='left' valign='middle'>"
				     "<td class='table_top_header' colspan='3'>%s %d %s</td>\n"
				     "</tr>"
				     "<tr align='left' valign='middle' class='table_header'>\n"
				     "<td class='table_header' align='center' width=\"7%%\"></td>\n"
				     "<td class='table_header' align='center' valign='middle' width=\"80%%\">%s</td>\n"
				"<td class='table_header' align='center' valign='middle' width=\"13%%\">%s</td>\n"
 				"</tr>", L_Top, maxsender, L_Sender, L_Mail, L_Number);
			sortUsersFrom();
			max = (SortDomain.nval < maxsender ? SortDomain.nval : maxsender);
			for (i = 0; i < max; i++) {
				u = (user *)SortDomain.allusers[i];
				if (u->from_cnt == 0)
					break;
				fprintf(fp, "<tr align='center' valign='middle'>");
				fprintf(fp, "<td width=\"7%%\" class='table_data'>%d</td>\n"
					"<td class='table_data' width=\"80%%\">%s</td>\n"
				        "<td class='table_data' width=\"13%%\">%d</td>\n",
							i + 1,
							u->user,
							u->from_cnt);
				fprintf(fp, "</tr>\n");
			}
			fprintf(fp, "</TABLE>\n");
			freeSortDomainTab();
		}
		else if ((substr = strstr(tmpstr, "<!--%6-->")) != NULL) {
			fprintf(fp, "<table width=\"80%%\" border='1' cellspacing='0' cellpadding='2'" 
				     "bordercolor='#F4F4F4'>"
				     "<tr align='left' valign='middle'>"
				     "<td class='table_top_header' colspan='3'>%s %d %s</td>\n"
				     "</tr>"
				     "<tr align='left' valign='middle' class='table_header'>\n"
				     "<td class='table_header' align='center' width=\"7%%\"></td>\n"
				     "<td class='table_header' align='center' valign='middle' width=\"80%%\">%s</td>\n"
				"<td class='table_header' align='center' valign='middle' width=\"13%%\">%s</td>\n"
 				"</tr>", L_Top, maxsender, L_Receiver, L_Mail, L_Number);
			sortUsersTo();
			max = (SortDomain.nval < maxreceiver ? SortDomain.nval : maxreceiver);
			for (i = 0; i < max; i++) {
				u = (user *)SortDomain.allusers[i];
				if (u->to_cnt == 0)
					break;
				fprintf(fp, "<tr align='center' valign='middle'>");
				fprintf(fp, "<td width=\"6%%\" class='table_data'>%d</td>\n"
					"<td class='table_data' width=\"80%%\">%s</td>\n"
				        "<td class='table_data' width=\"13%%\">%d</td>\n",
							i + 1,
							u->user,
							u->to_cnt);
				fprintf(fp, "</tr>\n");
			}
			fprintf(fp, "</TABLE>");
			freeSortDomainTab();
		}
		else if ((substr = strstr(tmpstr, "<!--%7-->")) != NULL) {
			fprintf(fp, "<table width=\"80%%\" border='1' cellspacing='0' cellpadding='2'" 
				     "bordercolor='#F4F4F4'>"
				     "<tr align='left' valign='middle'>"
				     "<td class='table_top_header' colspan='3'>%s %d %s</td>\n"
				     "</tr>"
				     "<tr align='left' valign='middle' class='table_header'>\n"
				     "<td class='table_header' align='center' width=\"7%%\"></td>\n"
				     "<td class='table_header' align='center' valign='middle' width=\"80%%\">%s</td>\n"
				"<td class='table_header' align='center' valign='middle' width=\"13%%\">%s</td>\n"
 				"</tr>", L_Top, maxsender, L_Size, L_Mail, L_Size);
			sortUsersByte();
			max = (SortDomain.nval < maxbyte ? SortDomain.nval : maxbyte);
			for (i = 0; i < max; i++) {
				u = (user *)SortDomain.allusers[i];
				if (u->from_byte == 0)
					break;
				getSizeStr(u->from_byte, tmpstr, &byte);
				fprintf(fp, "<tr align='center' valign='middle'>");
				fprintf(fp, "<td width=\"7%%\" class='table_data'>%d</td>\n"
					"<td class='table_data' width=\"80%%\">%s</td>\n"
				        "<td class='table_data' width=\"13%%\">%.2f %s</td>\n",
							i + 1,
							u->user,
							byte,
							tmpstr);
				fprintf(fp, "</tr>\n");
			}
			fprintf(fp, "</TABLE>");
			freeSortDomainTab();
		}
		else if ((substr = strstr(tmpstr, "<!--%8-->")) != NULL) {
			fprintf(fp, "<table width=\"80%%\" border='1' cellspacing='0' cellpadding='2'" 
				     "bordercolor='#F4F4F4'>"
				     "<tr align='left' valign='middle'>"
				     "<td class='table_top_header' colspan='3'>%s %d %s</td>\n"
				     "</tr>"
				     "<tr align='left' valign='middle' class='table_header'>\n"
				     "<td class='table_header' align='center' width=\"7%%\"></td>\n"
				     "<td class='table_header' align='center' valign='middle' width=\"80%%\">%s</td>\n"
				"<td class='table_header' align='center' valign='middle' width=\"13%%\">%s</td>\n"
 				"</tr>", L_Top, maxtotal, L_Total, L_Mail, L_Number);
			sortUsersTotal();
			max = (SortDomain.nval < maxtotal ? SortDomain.nval : maxtotal);
			for (i = 0; i < max; i++) {
				u = (user *)SortDomain.allusers[i];
				if ((u->to_cnt + u->from_cnt) == 0)
					break;
				fprintf(fp, "<tr align='center' valign='middle'>");
				fprintf(fp, "<td width=\"7%%\" class='table_data'>%d</td>\n"
					"<td class='table_data' width=\"80%%\">%s</td>\n"
				        "<td class='table_data' width=\"13%%\">%d</td>\n",
							i + 1,
							u->user,
							(u->from_cnt + u->to_cnt));
				fprintf(fp, "</tr>\n");
	
			}
			fprintf(fp, "</TABLE>");
			freeSortDomainTab();
		} else
			fwrite(tmpstr, sizeof(char), strlen(tmpstr), fp);
		memset(tmpstr, 0, 1024);
	}

	fprintf(fp, "<p><font face='Verdana, Arial, Helvetica, sans-serif' size='2' color='#666600'>"
			"Generated by <a href='http://www.enderunix.org/isoqlog/' target='_blank'>isoqlog</a>"
			" version 2.1.1 <a href='http://www.enderunix.org/' target='_blank'>"
			"EnderUNIX Software Development Team<a/></font></p>");

	sprintf(tmpstr, "%s/general/%d/%d/AllDays", outputdir, cur_year, cur_month);
	/*sumTotalStats();	*/
	storeCurrentDayHistory(tmpstr, &general);

	free(tmpstr);
	free(substr);
	free(newstr);

	fclose(fp);
	fclose(tp);
}



void createGeneralMonthlyHtml(char *d)
{
	FILE *fp, *tp;
	char *tmp, *tmpstr, *newstr, *substr;
	int i;
	int t_s = 0, t_r = 0;
	int c = 0; /* average counter */
	double byte = 0.0, t_b = 0.0;
	hist h[MAXDAYS];
	char *tfmt[2] = { "%s: %s %d, %d / %d:0%d",  
			  "%s: %s %d, %d / %d:%d" };

	tmp = (char *)calloc(1024, sizeof(char));
	tmpstr = (char *)calloc(1024, sizeof(char));
	newstr = (char *)calloc(1024, sizeof(char));
	substr = (char *)calloc(1024, sizeof(char));


	sprintf(tmp, "%s/index.html", d);
	if ((fp = fopen(tmp, "w")) == NULL) {
		fprintf(stderr, "createGeneralMonthlyHtml: cannot create months file %s: %s\n", tmp, strerror(errno));
		exit(-1);
	}

	sprintf(tmp, "%s/generalmonthly.html", htmldir);
	if ((tp = fopen(tmp, "r")) == NULL) {
		fprintf(stderr, "createGeneralMonthlyHtml: cannot open monthly template file %s: %s\n", tmpstr, strerror(errno));
		exit(-1);
	}

	sprintf(tmp, "%s/AllDays", d);
	getHistData(tmp, h, sizeof(h));

	while(fgets(tmpstr, 1024, tp) != NULL) {
		if ((substr = strstr(tmpstr, "<!--%d-->")) != NULL) {
			fwrite(tmpstr, sizeof(char), (strlen(tmpstr) - strlen(substr)), fp);
                        sprintf(newstr, "%s", "general");
                        fwrite(newstr, sizeof(char), strlen(newstr), fp);
                        fwrite(substr + 9, sizeof(char), strlen(substr) - 9, fp);
                        memset(substr, 0, 512);
                        memset(newstr, 0, 512);
		}
		else if ((substr = strstr(tmpstr, "<!--%s-->")) != NULL) {
			fwrite(tmpstr, sizeof(char), (strlen(tmpstr) - strlen(substr)), fp);
			sprintf(newstr, "%s, %d %s", months[cur_month - 1], cur_year, L_Stats);
                        fwrite(newstr, sizeof(char), strlen(newstr), fp);
                        fwrite(substr + 9, sizeof(char), strlen(substr) - 9, fp);
                        memset(substr, 0, 512);
                        memset(newstr, 0, 512);
		}
		else if ((substr = strstr(tmpstr, "<!--%c-->")) != NULL) {
			fwrite(tmpstr, sizeof(char), (strlen(tmpstr) - strlen(substr)), fp);
			sprintf(newstr, tfmt[(cur_min > 9 ? 1 : 0)],
						L_Creationtime,
						months[cur_month - 1], 
						cur_day,
						cur_year,
						cur_hour, 
						cur_min);
                        fwrite(newstr, sizeof(char), strlen(newstr), fp);
                        fwrite(substr + 9, sizeof(char), strlen(substr) - 9, fp);
                        memset(substr, 0, 512);
                        memset(newstr, 0, 512);
		}
		else if ((substr = strstr(tmpstr, "<!--%1-->")) != NULL) {
			fprintf(fp, "<table width='80%%' border='1' cellspacing='0' cellpadding='2' bordercolor='#F4F4F4'>");
			fprintf(fp, "<tr align='left' valign='middle' class='table_header'>"
				"<td width='20%%' class='table_header' align='center' valign='middle'>%s</td>"
				"<td width='20%%' class='table_header' align='center' valign='middle'>%s</td>"
				"<td width='20%%' class='table_header' align='center' valign='middle'>%s</td>"
				"<td width='20%%' class='table_header' align='center' valign='middle'>%s</td>"
				"<td width='20%%' class='table_header' align='center' valign='middle'>%s</td>"
				"</tr>\n", L_Day,  L_Sent, L_Received, L_Total, L_Size);
			for (i = 0; i < MAXDAYS; i++) {
				if ((h[i].sent + h[i].received) == 0)
					continue;
				getSizeStr(h[i].bytes_sent, tmpstr, &byte);
				t_s += h[i].sent;
				t_r += h[i].received;
				t_b += h[i].bytes_sent;
				c++;
				fprintf(fp, "<tr align='left' valign='middle'>"
				"<td width='20%%' class='table_data_day' align='center' valign='middle'>"
				"<a href='%d.html'>%d</a></td>"
				"<td width='20%%' class='table_data' align='center' valign='middle'>%d</td>"
				"<td width='20%%' class='table_data' align='center' valign='middle'>%d</td>"
				"<td width='20%%' class='table_data' align='center' valign='middle'>%d</td>"
				"<td width='20%%' class='table_data' align='center' valign='middle'>%.2f %s</td>"
				"</tr>\n",
						i + 1,
						i + 1,
						h[i].sent,
						h[i].received,
						(h[i].sent + h[i].received),
						byte,
						tmpstr);
			}
			memset(tmpstr, 0, 1024);
			getSizeStr(t_b, tmpstr, &byte);
			fprintf(fp, "<tr align='left' valign='middle'>"
				"<td width='20%%' class='table_foother' align='center' valign='middle'><b>%s</b></a>"
				"<td width='20%%' class='table_data_total' align='center' valign='middle'>%d</td>"
				"<td width='20%%' class='table_data_total' align='center' valign='middle'>%d</td>"
				"<td width='20%%' class='table_data_total' align='center' valign='middle'>%d</td>"
				"<td width='20%%' class='table_data_total' align='center' valign='middle'>%.2f %s</td>"
				"</tr>\n",L_Total,
					t_s,
					t_r,
					t_s + t_r,
					byte,
					tmpstr);
			if (c == 0) /* Do not divide by zero! */
				c = 1;
			getSizeStr(t_b/c, tmpstr, &byte);
			fprintf(fp, "<tr align='left' valign='middle'>"
				"<td width='20%%' class='table_foother' align='center' valign='middle'><b>%s</b></a>"
				"<td width='20%%' class='table_data_total' align='center' valign='middle'>%d</td>"
				"<td width='20%%' class='table_data_total' align='center' valign='middle'>%d</td>"
				"<td width='20%%' class='table_data_total' align='center' valign='middle'>%d</td>"
				"<td width='20%%' class='table_data_total' align='center' valign='middle'>%.2f %s</td>"
				"</tr>\n",L_Average,
					t_s/c,
					t_r/c,
					(t_s + t_r)/c,
					byte,
					tmpstr);
			
			fprintf(fp, "</TABLE>");
		}
		else 
			fwrite(tmpstr, sizeof(char), strlen(tmpstr), fp);
		memset(tmpstr, 0, 1024);
	}

	fprintf(fp, "<p><font face='Verdana, Arial, Helvetica, sans-serif' size='2' color='#666600'>"
			"Generated by <a href='http://www.enderunix.org/isoqlog/' target='_blank'>isoqlog</a>"
			" version 2.1.1 <a href='http://www.enderunix.org/' target='_blank'>"
			"EnderUNIX Software Development Team<a/></font></p>");

	sprintf(tmpstr, "%s/general/%d/AllMonths", outputdir, cur_year);
	/* storeCurrentMonthHistory(tmpstr, t_s, t_r, t_b); */
	storeCurrentMonthHistory(tmpstr, t_s, t_r, t_b);
	free(tmp);
	free(tmpstr);
	free(substr);
	free(newstr);
	fclose(fp);
	fclose(tp);

}


void createGeneralYearlyHtml(char *d)
{
	FILE *fp, *tp;
	char *tmp, *tmpstr, *newstr, *substr;
	int i, c = 0;
	int t_s, t_r;
	double byte = 0.0, t_b = 0.0;
	hist h[MAXMONTHS];
	char *tfmt[2] = { "%s: %s %d, %d / %d:0%d",  
			  "%s: %s %d, %d / %d:%d" };

	tmp = (char *)calloc(1024, sizeof(char));
	tmpstr = (char *)calloc(1024, sizeof(char));
	newstr = (char *)calloc(1024, sizeof(char));
	substr = (char *)calloc(1024, sizeof(char));

	t_s = t_r = 0;

	sprintf(tmp, "%s/index.html", d);
	if ((fp = fopen(tmp, "w")) == NULL) {
		fprintf(stderr, "createGeneralYearlyHtml: cannot create months file %s: %s\n", tmp, strerror(errno));
		exit(-1);
	}

	sprintf(tmp, "%s/generalyearly.html", htmldir);
	if ((tp = fopen(tmp, "r")) == NULL) {
		fprintf(stderr, "createGeneralYearlyHtml: cannot open monthly template file %s: %s\n", tmpstr, strerror(errno));
		exit(-1);
	}

	sprintf(tmp, "%s/AllMonths", d);
	getHistData(tmp, h, sizeof(h));

	while(fgets(tmpstr, 1024, tp) != NULL) {
		if ((substr = strstr(tmpstr, "<!--%d-->")) != NULL) {
			fwrite(tmpstr, sizeof(char), (strlen(tmpstr) - strlen(substr)), fp);
                        sprintf(newstr, "%s", "general");
                        fwrite(newstr, sizeof(char), strlen(newstr), fp);
                        fwrite(substr + 9, sizeof(char), strlen(substr) - 9, fp);
                        memset(substr, 0, 512);
                        memset(newstr, 0, 512);
		}
		else if ((substr = strstr(tmpstr, "<!--%s-->")) != NULL) {
			fwrite(tmpstr, sizeof(char), (strlen(tmpstr) - strlen(substr)), fp);
			sprintf(newstr, "%s, %d %s", months[cur_month - 1], cur_year, L_Stats);
                        fwrite(newstr, sizeof(char), strlen(newstr), fp);
                        fwrite(substr + 9, sizeof(char), strlen(substr) - 9, fp);
                        memset(substr, 0, 512);
                        memset(newstr, 0, 512);
		}
		else if ((substr = strstr(tmpstr, "<!--%c-->")) != NULL) {
			fwrite(tmpstr, sizeof(char), (strlen(tmpstr) - strlen(substr)), fp);
			sprintf(newstr, tfmt[(cur_min > 9 ? 1 : 0)],
						L_Creationtime,
						months[cur_month - 1], 
						cur_day,
						cur_year,
						cur_hour, 
						cur_min);
                        fwrite(newstr, sizeof(char), strlen(newstr), fp);
                        fwrite(substr + 9, sizeof(char), strlen(substr) - 9, fp);
                        memset(substr, 0, 512);
                        memset(newstr, 0, 512);
		}
		else if ((substr = strstr(tmpstr, "<!--%1-->")) != NULL) {
			fprintf(fp, "<table width='80%%' border='1' cellspacing='0' cellpadding='2' bordercolor='#F4F4F4'>");
			fprintf(fp, "<tr align='left' valign='middle' class='table_header'>"
				"<td width='20%%' class='table_header' align='center' valign='middle'>%s</td>"
				"<td width='20%%' class='table_header' align='center' valign='middle'>%s</td>"
				"<td width='20%%' class='table_header' align='center' valign='middle'>%s</td>"
				"<td width='20%%' class='table_header' align='center' valign='middle'> %s</td>"
				"<td width='20%%' class='table_header' align='center' valign='middle'>%s</td>"
				"</tr>\n", L_Month,  L_Sent, L_Received,  L_Total, L_Size);
			for (i = 0; i < MAXMONTHS; i++) {
				if ((h[i].sent + h[i].received) == 0)
					continue;
				getSizeStr(h[i].bytes_sent, tmpstr, &byte);
				t_s += h[i].sent;
				t_r += h[i].received;
				t_b += h[i].bytes_sent;
				c++;

				fprintf(fp, "<tr align='left' valign='middle'>"
				"<td width='20%%' class='table_data_day' align='center' valign='middle'>"
				"<a href='%d/'>%d</a></td>"
				"<td width='20%%' class='table_data' align='center' valign='middle'>%d</td>"
				"<td width='20%%' class='table_data' align='center' valign='middle'>%d</td>"
				"<td width='20%%' class='table_data' align='center' valign='middle'>%d</td>"
				"<td width='20%%' class='table_data' align='center' valign='middle'>%.2f %s</td>"
				"</tr>\n",
						i + 1,
						i + 1,
						h[i].sent,
						h[i].received,
						(h[i].sent + h[i].received),
						byte,
						tmpstr);
			}
			memset(tmpstr, 0, 1024);
			getSizeStr(t_b, tmpstr, &byte);
			fprintf(fp, "<tr align='left' valign='middle'>"
				"<td width='20%%' class='table_foother' align='center' valign='middle'><b>%s</b></a>"
				"<td width='20%%' class='table_data_total' align='center' valign='middle'>%d</td>"
				"<td width='20%%' class='table_data_total' align='center' valign='middle'>%d</td>"
				"<td width='20%%' class='table_data_total' align='center' valign='middle'>%d</td>"
				"<td width='20%%' class='table_data_total' align='center' valign='middle'>%.2f %s</td>"
				"</tr>\n",L_Total,
					t_s,
					t_r,
					t_s + t_r,
					byte,
					tmpstr);
			if (c == 0) /* Do not divide to zero! */
				c = 1;
			getSizeStr(t_b/c, tmpstr, &byte);
			fprintf(fp, "<tr align='left' valign='middle'>"
				"<td width='20%%' class='table_foother' align='center' valign='middle'><b>%s</b></a>"
				"<td width='20%%' class='table_data_total' align='center' valign='middle'>%d</td>"
				"<td width='20%%' class='table_data_total' align='center' valign='middle'>%d</td>"
				"<td width='20%%' class='table_data_total' align='center' valign='middle'>%d</td>"
				"<td width='20%%' class='table_data_total' align='center' valign='middle'>%.2f %s</td>"
				"</tr>\n", L_Average,
					t_s/c,
					t_r/c,
					(t_s + t_r)/c,
					byte,
					tmpstr);
			
			fprintf(fp, "</TABLE>");
		}
		else 
			fwrite(tmpstr, sizeof(char), strlen(tmpstr), fp);
		memset(tmpstr, 0, 1024);
	}

	fprintf(fp, "<p><font face='Verdana, Arial, Helvetica, sans-serif' size='2' color='#666600'>"
			"Generated by <a href='http://www.enderunix.org/isoqlog/' target='_blank'>isoqlog</a>"
			" version 2.1.1 <a href='http://www.enderunix.org/' target='_blank'>"
			"EnderUNIX Software Development Team<a/></font></p>");

	/* sprintf(tmpstr, "%s/general/%d/AllYears", outputdir, cur_year); */
	sprintf(tmpstr, "%s/general/AllYears", outputdir);
	/* storeCurrentYearHistory(tmpstr, t_s, t_r, byte); */
	storeCurrentYearHistory(tmpstr, t_s, t_r, t_b);

	free(tmp);
	free(tmpstr);
	free(substr);
	free(newstr);

	fclose(fp);
	fclose(tp);

}

void createGeneralHtml(char *d)
{
	FILE *fp, *tp;
	char *tmp, *tmpstr, *newstr, *substr;
	int i, c = 0;
	int t_s = 0, t_r = 0;
	 double byte = 0.0, t_b = 0.0;
	hist h[MAXYEARS];
	char *tfmt[2] = { "%s: %s %d, %d / %d:0%d",  
			  "%s: %s %d, %d / %d:%d" };

	tmp = (char *)calloc(1024, sizeof(char));
	tmpstr = (char *)calloc(1024, sizeof(char));
	newstr = (char *)calloc(1024, sizeof(char));
	substr = (char *)calloc(1024, sizeof(char));


	sprintf(tmp, "%s/index.html", d);
	if ((fp = fopen(tmp, "w")) == NULL) {
		fprintf(stderr, "createGeneralHtml: cannot create general output file %s: %s\n", tmp, strerror(errno));
		exit(-1);
	}

	sprintf(tmp, "%s/generaldomain.html", htmldir);
	if ((tp = fopen(tmp, "r")) == NULL) {
		fprintf(stderr, "createGeneralHtml: cannot open general template file %s: %s\n", tmpstr, strerror(errno));
		exit(-1);
	}

	sprintf(tmp, "%s/AllYears", d);
	getHistData(tmp, h, sizeof(h));

	while(fgets(tmpstr, 1024, tp) != NULL) {
		if ((substr = strstr(tmpstr, "<!--%d-->")) != NULL) {
			fwrite(tmpstr, sizeof(char), (strlen(tmpstr) - strlen(substr)), fp);
                        sprintf(newstr, "%s", "general");
                        fwrite(newstr, sizeof(char), strlen(newstr), fp);
                        fwrite(substr + 9, sizeof(char), strlen(substr) - 9, fp);
                        memset(substr, 0, 512);
                        memset(newstr, 0, 512);
		}
		else if ((substr = strstr(tmpstr, "<!--%s-->")) != NULL) {
			fwrite(tmpstr, sizeof(char), (strlen(tmpstr) - strlen(substr)), fp);
			sprintf(newstr, "%s %s", L_Yearly, L_Stats);
                        fwrite(newstr, sizeof(char), strlen(newstr), fp);
                        fwrite(substr + 9, sizeof(char), strlen(substr) - 9, fp);
                        memset(substr, 0, 512);
                        memset(newstr, 0, 512);
		}
		else if ((substr = strstr(tmpstr, "<!--%c-->")) != NULL) {
			fwrite(tmpstr, sizeof(char), (strlen(tmpstr) - strlen(substr)), fp);
			sprintf(newstr, tfmt[(cur_min > 9 ? 1 : 0)],
						L_Creationtime,
						months[cur_month - 1], 
						cur_day,
						cur_year,
						cur_hour, 
						cur_min);
                        fwrite(newstr, sizeof(char), strlen(newstr), fp);
                        fwrite(substr + 9, sizeof(char), strlen(substr) - 9, fp);
                        memset(substr, 0, 512);
                        memset(newstr, 0, 512);
		}
		else if ((substr = strstr(tmpstr, "<!--%1-->")) != NULL) {
			fprintf(fp, "<table width='80%%' border='1' cellspacing='0' cellpadding='2' bordercolor='#F4F4F4'>");
			fprintf(fp, "<tr align='left' valign='middle' class='table_header'>"
				"<td width='20%%' class='table_header' align='center' valign='middle'>%s</td>"
				"<td width='20%%' class='table_header' align='center' valign='middle'>%s</td>"
				"<td width='20%%' class='table_header' align='center' valign='middle'>%s</td>"
				"<td width='20%%' class='table_header' align='center' valign='middle'>%s</td>"
				"<td width='20%%' class='table_header' align='center' valign='middle'>%s</td>"
				"</tr>\n",L_Year, L_Sent, L_Received,  L_Total, L_Size);
			for (i = 0; i < MAXYEARS; i++) {
				if ((h[i].sent + h[i].received) == 0)
					continue;
				getSizeStr(h[i].bytes_sent, tmpstr, &byte);
				t_s += h[i].sent;
				t_r += h[i].received;
				t_b += h[i].bytes_sent;
				c++;
				fprintf(fp, "<tr align='left' valign='middle'>"
				"<td width='20%%' class='table_data_day' align='center' valign='middle'>"
				"<a href='%d/'>%d</a></td>"
				"<td width='20%%' class='table_data' align='center' valign='middle'>%d</td>"
				"<td width='20%%' class='table_data' align='center' valign='middle'>%d</td>"
				"<td width='20%%' class='table_data' align='center' valign='middle'>%d</td>"
				"<td width='20%%' class='table_data' align='center' valign='middle'>%.2f %s</td>"
				"</tr>\n",
						i + 2002,
						i + 2002,
						h[i].sent,
						h[i].received,
						(h[i].sent + h[i].received),
						byte,
						tmpstr);
			}
			memset(tmpstr, 0, 1024);
			getSizeStr(t_b, tmpstr, &byte);
			fprintf(fp, "<tr align='left' valign='middle'>"
				"<td width='20%%' class='table_foother' align='center' valign='middle'><b>%s</b></a>"
				"<td width='20%%' class='table_data_total' align='center' valign='middle'>%d</td>"
				"<td width='20%%' class='table_data_total' align='center' valign='middle'>%d</td>"
				"<td width='20%%' class='table_data_total' align='center' valign='middle'>%d</td>"
				"<td width='20%%' class='table_data_total' align='center' valign='middle'>%.2f %s</td>"
				"</tr>\n",L_Total,
					t_s,
					t_r,
					t_s + t_r,
					byte,
					tmpstr);
			if (c == 0)
				c = 1;
			getSizeStr(t_b/c, tmpstr, &byte);
			fprintf(fp, "<tr align='left' valign='middle'>"
				"<td width='20%%' class='table_foother' align='center' valign='middle'><b>%s</b></a>"
				"<td width='20%%' class='table_data_total' align='center' valign='middle'>%d</td>"
				"<td width='20%%' class='table_data_total' align='center' valign='middle'>%d</td>"
				"<td width='20%%' class='table_data_total' align='center' valign='middle'>%d</td>"
				"<td width='20%%' class='table_data_total' align='center' valign='middle'>%.2f %s</td>"
				"</tr>\n",L_Average,
					t_s/c,
					t_r/c,
					(t_s + t_r)/c,
					byte,
					tmpstr);


			fprintf(fp, "</TABLE>");
		}
		else 
			fwrite(tmpstr, sizeof(char), strlen(tmpstr), fp);
		memset(tmpstr, 0, 1024);
	}

	fprintf(fp, "<p><font face='Verdana, Arial, Helvetica, sans-serif' size='2' color='#666600'>"
			"Generated by <a href='http://www.enderunix.org/isoqlog/' target='_blank'>isoqlog</a>"
			" version 2.1.1 <a href='http://www.enderunix.org/' target='_blank'>"
			"EnderUNIX Software Development Team<a/></font></p>");

	free(tmp);
	free(tmpstr);
	free(substr);
	free(newstr);
	fclose(fp);
	fclose(tp);

}


void getSizeStr(double b, char *s, double *v) 
{
	char *bytestrs[] = {
		"bytes", 
		"KBytes", 
		"MBytes", 
		"GBytes"};

	if (b > 1073741824) {
		strcpy(s, bytestrs[3]);
		*v = b / 1073741824;
	}
	else if (b > 1048576) {
		strcpy(s, bytestrs[2]);
		*v = b / 1048576;
	} 
	else if (b > 1024) {
		strcpy(s, bytestrs[1]);
		*v = b / 1024;
	}
	else {
		strcpy(s, bytestrs[0]);
		*v = b;
	}
}
void createIndexHtml(char *f)
{
	FILE *tp; 
	FILE *fp;
	int i;
	char *tmpstr, *substr, *newstr;
	char *tfmt[2] = { "%s: %s %d, %d / %d:0%d",  
			  "%s: %s %d, %d / %d:%d" };

      tmpstr = (char *)calloc(1024, sizeof(char));
      substr = (char *)calloc(512, sizeof(char));
      newstr = (char *)calloc(512, sizeof(char));

       if ((fp = fopen(f, "w")) == NULL) {
 	      fprintf(stderr, "createIndexHtml: cannot create index html output %s: %s\n", f, strerror(errno));
              exit(-1);
       }
       sprintf(tmpstr, "%s/index.html", htmldir);

       if ((tp = fopen(tmpstr, "r")) == NULL) {
	       fprintf(stderr, "createIndexHtml: cannot open index template file %s: %s\n", tmpstr, strerror(errno));
               exit(-1);
        }

	while (fgets(tmpstr, 1024, tp) != NULL) {
		if ((substr = strstr(tmpstr, "<!--%1-->")) != NULL) {
	        /* First write the first part of the string */
	        fwrite(tmpstr, sizeof(char), (strlen(tmpstr) - strlen(substr)), fp);
	        /* Then write what we want */
	        sprintf(newstr, "<!-- %s -->\n", "isoqlog  Team");
	        fwrite(newstr, sizeof(char), strlen(newstr), fp);
	        /* Skip the comment and continue from the "tmpstr"        */
	        fwrite(substr + 9, sizeof(char), strlen(substr) - 9, fp);
	        memset(substr, 0, 512);
	        memset(newstr, 0, 512);
	}

		else if ((substr = strstr(tmpstr, "<!--%2-->")) != NULL) {
	        fwrite(tmpstr, sizeof(char), (strlen(tmpstr) - strlen(substr)), fp);
	        sprintf(newstr,
		    "<img src=\"images/isoqlog.gif\" width=\"288\" height=\"66\"><br><br>\n");
	        fwrite(newstr, sizeof(char), strlen(newstr), fp);

	       sprintf(newstr,
		    "<table width=\"100%%\" border=\"0\" cellspacing=\"0\" cellpadding=\"1\">\n");
	       fwrite(newstr, sizeof(char), strlen(newstr), fp);
	       sprintf(newstr, "	<tr>\n");
	       fwrite(newstr, sizeof(char), strlen(newstr), fp);
	       sprintf(newstr, "		<td width=\"11%%\"></td>\n");
	       fwrite(newstr, sizeof(char), strlen(newstr), fp);
	       sprintf(newstr,
		    "		<td width=\"89%%\" class=\"header\">%s</td>\n", L_Domain);
  	       fwrite(newstr, sizeof(char), strlen(newstr), fp);
	       sprintf(newstr, "	</tr>\n");
	       fwrite(newstr, sizeof(char), strlen(newstr), fp);
	       sprintf(newstr, "	<tr> \n");
	       fwrite(newstr, sizeof(char), strlen(newstr), fp);
	       sprintf(newstr,
		    "		<td width=\"11%%\" height=\"1\"></td>\n");
	      fwrite(newstr, sizeof(char), strlen(newstr), fp);
	      sprintf(newstr,
		    "		<td width=\"89%%\" height=\"1\"><img src=\"images/dot.gif\" width=\"500\" height=\"1\"></td>\n");
	    fwrite(newstr, sizeof(char), strlen(newstr), fp);
	    sprintf(newstr, "	</tr>\n");
	    fwrite(newstr, sizeof(char), strlen(newstr), fp);
	    sprintf(newstr, "</table>\n");

	    sprintf(newstr,
		    "<table width=\"100%%\" border=\"0\" cellspacing=\"0\" cellpadding=\"1\">\n");
	    fwrite(newstr, sizeof(char), strlen(newstr), fp);
	    for (i = 0; i < DomainsTab.nval; i++) {
		sprintf(newstr, "	<tr>\n");
		fwrite(newstr, sizeof(char), strlen(newstr), fp);
		sprintf(newstr,
			"		<td width=\"23%%\" align=\"right\" valign=\"middle\"><img src=\"images/pk.gif\" width=\"20\" height=\"12\"></td>\n");
		fwrite(newstr, sizeof(char), strlen(newstr), fp);
		sprintf(newstr,
			"		<td width=\"1%%\" align=\"right\" valign=\"middle\">&nbsp;</td>\n");
		fwrite(newstr, sizeof(char), strlen(newstr), fp);
		sprintf(newstr,
			"		<td width=\"76%%\" align=\"left\" valign=\"middle\"> <a href=\"%s/\" class=\"domains\" >%s</a></td>\n",
			DomainsTab.alldomains[i].domain,
			DomainsTab.alldomains[i].domain);
		fwrite(newstr, sizeof(char), strlen(newstr), fp);
		sprintf(newstr, "	</tr>\n");
		fwrite(newstr, sizeof(char), strlen(newstr), fp);
	    }
	/* for general */
	sprintf(newstr, "	<tr>\n");
		fwrite(newstr, sizeof(char), strlen(newstr), fp);
		sprintf(newstr,
			"		<td width=\"23%%\" align=\"right\" valign=\"middle\"><img src=\"images/pk.gif\" width=\"20\" height=\"12\"></td>\n");
		fwrite(newstr, sizeof(char), strlen(newstr), fp);
		sprintf(newstr,
			"		<td width=\"1%%\" align=\"right\" valign=\"middle\">&nbsp;</td>\n");
		fwrite(newstr, sizeof(char), strlen(newstr), fp);
		sprintf(newstr,
			"<td width=\"76%%\" align=\"left\" valign=\"middle\"> <a href=\"general/\" class=\"domains\" >General</a></td>\n");
		fwrite(newstr, sizeof(char), strlen(newstr), fp);
		sprintf(newstr, "	</tr>\n");
		fwrite(newstr, sizeof(char), strlen(newstr), fp);

	    sprintf(newstr, "	</table>\n");
	    fwrite(newstr, sizeof(char), strlen(newstr), fp);

	    fwrite(substr + 9, sizeof(char), strlen(substr) - 9, fp);
	    memset(substr, 0, 512);
	    memset(newstr, 0, 512);
	} else if ((substr = strstr(tmpstr, "<!--%3-->")) != NULL) {
	    fwrite(tmpstr, sizeof(char), (strlen(tmpstr) - strlen(substr)),
		   fp);
	    sprintf(newstr,
		    "<font face=\"Verdana, Arial, Helvetica, sans-serif\" size=\"2\" color=\"#666600\">Log Analysis for All MTA. (For the present Qmail, Postfix, Sendmail.)</font>");
	    fwrite(newstr, sizeof(char), strlen(newstr), fp);
	    fwrite(substr + 9, sizeof(char), strlen(substr) - 9, fp);
	    memset(substr, 0, 512);
	    memset(newstr, 0, 512);
	} 
	else if ((substr = strstr(tmpstr, "<!--%c-->")) != NULL) {
		fwrite(tmpstr, sizeof(char), (strlen(tmpstr) - strlen(substr)), fp);
		sprintf(newstr, tfmt[(cur_min > 9 ? 1 : 0)],
						L_Creationtime,
						months[cur_month - 1], 
						cur_day,
						cur_year,
						cur_hour, 
						cur_min);
                fwrite(newstr, sizeof(char), strlen(newstr), fp);
                fwrite(substr + 9, sizeof(char), strlen(substr) - 9, fp);
                memset(substr, 0, 512);
                memset(newstr, 0, 512);
	}
		else
	 	   fwrite(tmpstr, sizeof(char), strlen(tmpstr), fp);
	}

		fprintf(fp, "<p><font face='Verdana, Arial, Helvetica, sans-serif' size='2' color='#666600'>"
			"Generated by <a href='http://www.enderunix.org/isoqlog/' target='_blank'>isoqlog</a>"
			" version 2.1.1 <a href='http://www.enderunix.org/' target='_blank'>"
			"EnderUNIX Software Development Team<a/></font></p>");
	memset(tmpstr, 0, 1024);

    free(tmpstr);
    free(substr);
    free(newstr);

	fclose(fp);
	fclose(tp);
}

