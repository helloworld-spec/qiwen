/*************************************************

httpd.c
used by onvif

**************************************************/

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <getopt.h>		/* getopt_long() */

#include <fcntl.h>		/* low-level i/o */
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>

#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <pthread.h>

#include <sys/ipc.h>
#include <sys/shm.h>
#include <signal.h>

#include "dvs.h"
#include "http_service.h"
//#include "anyka_types.h"
//#include "camera.h"
// #include "inisetting.h"
//#include "cgi_anyka.h"
#include "ak_onvif.h"
/*
config_save_videoprofile (videoprofile, 0);
hal_adjust_enc_profile (dvs, 0);
config_save_videoprofile_small (videoprofile, 0);
hal_adjust_enc_profile_small (dvs, 0);
config_save_videocolor (videocolor, 0);
int config_save_network (config_network_t * network)
*/

extern dvs_t g_dvs;

static char *http_head_str = "HTTP/1.1 200 OK\r\nServer: gSOAP/2.8\r\nContent-Type: application/soap+xml; charset=utf-8\r\nContent-Length: %d\r\nConnection: close\r\n\r\n";

static char *xml_version = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";

static char *soap_env_head = "<env:Envelope xmlns:env=\"http://www.w3.org/2003/05/soap-envelope\" xmlns:soapenc=\"http://www.w3.org/2003/05/soap-encoding\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns:xs=\"http://www.w3.org/2001/XMLSchema\" xmlns:tt=\"http://www.onvif.org/ver10/schema\" xmlns:tds=\"http://www.onvif.org/ver10/device/wsdl\" xmlns:trt=\"http://www.onvif.org/ver10/media/wsdl\" xmlns:timg=\"http://www.onvif.org/ver20/imaging/wsdl\" xmlns:tev=\"http://www.onvif.org/ver10/events/wsdl\" xmlns:tptz=\"http://www.onvif.org/ver20/ptz/wsdl\" xmlns:tan=\"http://www.onvif.org/ver20/analytics/wsdl\" xmlns:tst=\"http://www.onvif.org/ver10/storage/wsdl\" xmlns:ter=\"http://www.onvif.org/ver10/error\" xmlns:dn=\"http://www.onvif.org/ver10/network/wsdl\" xmlns:tns1=\"http://www.onvif.org/ver10/topics\" xmlns:tmd=\"http://www.onvif.org/ver10/deviceIO/wsdl\" xmlns:wsdl=\"http://schemas.xmlsoap.org/wsdl\" xmlns:wsoap12=\"http://schemas.xmlsoap.org/wsdl/soap12\" xmlns:http=\"http://schemas.xmlsoap.org/wsdl/http\" xmlns:d=\"http://schemas.xmlsoap.org/ws/2005/04/discovery\" xmlns:wsadis=\"http://schemas.xmlsoap.org/ws/2004/08/addressing\" xmlns:xop=\"http://www.w3.org/2004/08/xop/include\" xmlns:wsnt=\"http://docs.oasis-open.org/wsn/b-2\" xmlns:wsa=\"http://www.w3.org/2005/08/addressing\" xmlns:wstop=\"http://docs.oasis-open.org/wsn/t-1\" xmlns:wsrf-bf=\"http://docs.oasis-open.org/wsrf/bf-2\" xmlns:wsntw=\"http://docs.oasis-open.org/wsn/bw-2\" xmlns:wsrf-rw=\"http://docs.oasis-open.org/wsrf/rw-2\" xmlns:wsaw=\"http://www.w3.org/2006/05/addressing/wsdl\" xmlns:wsrf-r=\"http://docs.oasis-open.org/wsrf/r-2\" xmlns:tnshik=\"http://www.hikvision.com/2011/event/topics\">";

#if 1
// static struct video_info *s_video_info;
// static struct ethernet_info* s_ethernet_info;
#endif

static int get_prefixlen_by_addr(const char *addr)
{
	//printf("addr: %s\n", addr);

	struct in_addr net;
	if (inet_aton(addr, &net)) {
		int i = 0, j = 32;
		unsigned net_addr = htonl(net.s_addr);
		//printf("start 0x%x\n", net_addr);
		for (;j > 0; j--) {
			if (net_addr & 0x1)
				i++;
			net_addr >>= 1;
			//printf("tmp: 0x%x, %d\n", net_addr, i);
		}
		//printf("bit number: %d\n", i);
		return i;
	} else
		return 24;	//by default
}

static int set_rtc_time_from_sys_time (dvs_t * dvs)
{
	char cmdstr[TEMP_STR_LEN];
	memset (cmdstr, 0, TEMP_STR_LEN);
	sprintf (cmdstr, "hwclock -w");
	printf ("%s\r\n", cmdstr);
	send_cmd (dvs, cmdstr);
	return 0;
}

#if 0
static int hal_set_brightness (dvs_t *dvs, int index, int brightness)
{
	// SetBrightness(brightness / 15);//SetBrightness(0);
	SetBrightness(brightness);
	return 0;
}

static int hal_set_contrast (dvs_t *dvs, int index, int contrast)
{
	// SetGAMMA(contrast / 15);//SetGAMMA(0);
	SetGAMMA(contrast);
	return 0;
}

static int hal_set_saturation (dvs_t *dvs, int index, int saturation)
{
	// SetSATURATION(saturation / 15);//SetSATURATION(0);
	SetSATURATION(saturation);
	return 0;
}
#endif

#if 0
static int IniWriteString (const char *section, const char *indent, const char *value, char *inifilename)
{
	FILE *fp;
	struct stringlist *head, *node, *p, *t;
	char tempstr[MAXSTRLEN];
	char sectionstr[MAXSTRLEN];
	int i, n, len;
	int sectionfinded;
	bzero (sectionstr, MAXSTRLEN);
	sprintf (sectionstr, "[%s]", section);
	fp = fopen (inifilename, "rt");
	if (fp == NULL)
	{
		fp = fopen (inifilename, "wt");
		fputs (sectionstr, fp);
		fputs ("\r\n", fp);
		bzero (tempstr, MAXSTRLEN);
		sprintf (tempstr, "%s=%s", indent, value);
		fputs (tempstr, fp);
		fputs ("\r\n", fp);
		fclose (fp);
		return 0;
	}
	head = (struct stringlist *) malloc (sizeof (struct stringlist));
	p = head;
	while (!feof (fp))
	{
		node = (struct stringlist *) malloc (sizeof (struct stringlist));
		node->next = NULL;
		bzero (node->string, MAXSTRLEN);
		fgets (node->string, MAXSTRLEN, fp);
		p->next = node;
		p = p->next;
	}
	fclose (fp);
	p = head;
	while (p->next != NULL)
	{
		t = p->next;
		len = strlen (t->string);
		bzero (tempstr, MAXSTRLEN);
		n = 0;
		for (i = 0; i < len; i++)
		{
			if ((t->string[i] != '\r') && (t->string[i] != '\n')
					&& (t->string[i] != ' '))
			{
				tempstr[n] = t->string[i];
				n++;
			}
		}
		if (strlen (tempstr) == 0)
		{
			p->next = t->next;
			free (t);
		}
		else
		{
			bzero (t->string, MAXSTRLEN);
			strcpy (t->string, tempstr);
			p = p->next;
		}
	}
	p = head;
	sectionfinded = 0;
	while (p->next != NULL)
	{
		t = p;
		p = p->next;
		if (sectionfinded == 0)
		{
			if (strcmp (p->string, sectionstr) == 0)
			{
				sectionfinded = 1;
				node =
					(struct stringlist *) malloc (sizeof (struct stringlist));
				bzero (node->string, MAXSTRLEN);
				sprintf (node->string, "%s=%s", indent, value);
				node->next = p->next;
				p->next = node;
				p = p->next;
			}
		}
		else
		{
			if (p->string[0] == '[')
			{
				goto end;
			}
			else
			{
				if (strncmp (p->string, indent, strlen (indent)) == 0)
				{
					if (p->string[strlen (indent)] == '=')
					{
						node = p;
						t->next = p->next;
						free (node);
						goto end;
					}
				}
			}
		}
	}
end:
	fp = fopen (inifilename, "wt");
	p = head;
	while (p->next != NULL)
	{
		p = p->next;
		fputs (p->string, fp);
		fputs ("\r\n", fp);
	}
	if (sectionfinded == 0)
	{
		fputs (sectionstr, fp);
		fputs ("\r\n", fp);
		bzero (tempstr, MAXSTRLEN);
		sprintf (tempstr, "%s=%s", indent, value);
		fputs (tempstr, fp);
		fputs ("\r\n", fp);
	}
	fclose (fp);
	while (head->next != NULL)
	{
		t = head->next;
		head->next = t->next;
		free (t);
	}
	free (head);
	return 0;
}

static int
IniWriteInteger (const char *section, const char *indent, int value,
		 char *inifilename)
{
  char str[20];
  memset (str, 0, 20);
  sprintf (str, "%d", value);
  IniWriteString (section, indent, str, inifilename);
  return 0;
}

static int
IniReadString (const char *section, const char *indent,
	       const char *defaultresult, char *result,
	       int resultlen, char *inifilename)
{
  FILE *fp;
  struct stringlist *head, *node, *p, *t;
  char tempstr[MAXSTRLEN];
  char sectionstr[MAXSTRLEN];
  int i, n, len;
  int sectionfinded;
  bzero (sectionstr, MAXSTRLEN);
  sprintf (sectionstr, "[%s]", section);
  fp = fopen (inifilename, "rt");
  if (fp == NULL)
    {
      strcpy (result, defaultresult);
      return 0;
    }
  head = (struct stringlist *) malloc (sizeof (struct stringlist));
  p = head;
  bzero (tempstr, MAXSTRLEN);
  while (fgets (tempstr, MAXSTRLEN, fp) != NULL)
    {
      node = (struct stringlist *) malloc (sizeof (struct stringlist));
      node->next = NULL;
      bzero (node->string, MAXSTRLEN);
      strcpy (node->string, tempstr);
      p->next = node;
      p = p->next;
      bzero (tempstr, MAXSTRLEN);
    }
  fclose (fp);
  p = head;
  while (p->next != NULL)
    {
      t = p->next;
      len = strlen (t->string);
      bzero (tempstr, MAXSTRLEN);
      n = 0;
      for (i = 0; i < len; i++)
	{
	  if ((t->string[i] != '\r') && (t->string[i] != '\n')
	      && (t->string[i] != ' '))
	    {
	      tempstr[n] = t->string[i];
	      n++;
	    }
	}
      if (strlen (tempstr) == 0)
	{
	  p->next = t->next;
	  free (t);
	}
      else
	{
	  bzero (t->string, MAXSTRLEN);
	  strcpy (t->string, tempstr);
	  p = p->next;
	}
    }
  p = head;
  sectionfinded = 0;
  while (p->next != NULL)
    {
      p = p->next;
      if (sectionfinded == 0)
	{
	  if (strcmp (p->string, sectionstr) == 0)
	    sectionfinded = 1;
	}
      else
	{
	  if (p->string[0] == '[')
	    {
	      strcpy (result, defaultresult);
	      goto end;
	    }
	  else
	    {
	      if (strncmp (p->string, indent, strlen (indent)) == 0)
		{
		  if (p->string[strlen (indent)] == '=')
		    {
		      strncpy (result, p->string + strlen (indent) + 1,
			       resultlen);
		      goto end;
		    }
		}
	    }
	}
    }
  strcpy (result, defaultresult);
end:while (head->next != NULL)
    {
      t = head->next;
      head->next = t->next;
      free (t);
    }
  free (head);
  return 0;
}

static int
IniReadInteger (const char *section, const char *indent, int defaultresult,
		char *inifilename)
{
  int i, len;
  char str[1024];
  IniReadString (section, indent, "NULL", str, 1024, inifilename);
  if (strcmp (str, "NULL") == 0)
    {
      return defaultresult;
    }
  len = strlen (str);
  if (len == 0)
    {
      return defaultresult;
    }
  for (i = 0; i < len; i++)
    {
      if ((*(str + i) < '0') || (*(str + i) > '9'))
	{
	  return defaultresult;
	}
    }
  return atoi (str);
}
#endif
// fix me
static int config_save_network (config_network_t * network)
{
	IniWriteInteger ("sys", "type", network->type, "/etc/jffs2/network.ini");
	IniWriteInteger ("sys", "auto_get_dns", network->auto_get_dns, "/etc/jffs2/network.ini");

	IniWriteString ("static", "ip", network->ip, "/etc/jffs2/network.ini");
	IniWriteString ("static", "netmask", network->netmask, "/etc/jffs2/network.ini");
	IniWriteString ("static", "gateway", network->gateway, "/etc/jffs2/network.ini");
	IniWriteString ("static", "dns1", network->dns1, "/etc/jffs2/network.ini");
	IniWriteString ("static", "dns2", network->dns2, "/etc/jffs2/network.ini");
	return 0;
}

static int config_save_videocolor (config_videocolor_t * videocolor, int index)
{
	char tmpstr[MAX_NAME_LEN];
	sprintf (tmpstr, "%d", index + 1);
	IniWriteInteger (tmpstr, "brightness", videocolor->brightness,"/etc/jffs2/videocolor.ini");
	IniWriteInteger (tmpstr, "contrast", videocolor->contrast,"/etc/jffs2/videocolor.ini");
	IniWriteInteger (tmpstr, "saturation", videocolor->saturation,"/etc/jffs2/videocolor.ini");
	IniWriteInteger (tmpstr, "hue", videocolor->hue, "/etc/jffs2/videocolor.ini");
	return 0;
}
#if 0
static int config_save_videoprofile (config_videoprofile_t * videoprofile, int index)
{
	char tmpstr[MAX_NAME_LEN];
	sprintf (tmpstr, "%d", index + 1);
	IniWriteInteger (tmpstr, "resolution", videoprofile->resolution, "/etc/jffs2/videoprofile.ini");
	// printf ("config_save_videoprofile:videoprofile->resolution=%d\r\n",videoprofile->resolution);
	IniWriteInteger (tmpstr, "quality", videoprofile->quality, "/etc/jffs2/videoprofile.ini");
	IniWriteInteger (tmpstr, "bitrate", videoprofile->bitrate, "/etc/jffs2/videoprofile.ini");
	IniWriteInteger (tmpstr, "framerate", videoprofile->framerate, "/etc/jffs2/videoprofile.ini");
	IniWriteInteger (tmpstr, "recordaudio", videoprofile->recordaudio, "/etc/jffs2/videoprofile.ini");

	return 0;
}
#endif

static void set_default_config(dvs_t * dvs)
{
	// send_cmd (dvs, "cp /mn");
	IniWriteInteger ("1", "resolution", 0, "/etc/jffs2/videoprofile.ini");
	switch (dvs->sensor)
	{
		case HM1375:
			IniWriteInteger ("1", "bitrate", 2048, "/etc/jffs2/videoprofile.ini");
			IniWriteInteger ("1", "framerate", 25, "/etc/jffs2/videoprofile.ini");
			IniWriteInteger ("1", "brightness", 25, "/etc/jffs2/videocolor.ini");
			IniWriteInteger ("1", "contrast", 29, "/etc/jffs2/videocolor.ini");
			IniWriteInteger ("1", "saturation", 25, "/etc/jffs2/videocolor.ini");
			IniWriteInteger ("1", "hue", 0, "/etc/jffs2/videocolor.ini");
			break;
		default:
			IniWriteInteger ("1", "bitrate", 2048, "/etc/jffs2/videoprofile.ini");
			IniWriteInteger ("1", "framerate", 25, "/etc/jffs2/videoprofile.ini");
			IniWriteInteger ("1", "brightness", 25, "/etc/jffs2/videocolor.ini");
			IniWriteInteger ("1", "contrast", 29, "/etc/jffs2/videocolor.ini");
			IniWriteInteger ("1", "saturation", 25, "/etc/jffs2/videocolor.ini");
			IniWriteInteger ("1", "hue", 0, "/etc/jffs2/videocolor.ini");
			break;
	}
}

static int get_ip_value (char *ipstr)
{
	char tmpstr[128];
	unsigned int i, n, l, value[4], res;
	int len = strlen (ipstr);
	for (i = 0; i < 4; i++)
	{
		value[i] = 0;
	}
	n = 0;
	l = 0;
	memset (tmpstr, 0, 128);
	for (i = 0; i < len; i++)
	{
		if (ipstr[i] != '.')
		{
			tmpstr[l] = ipstr[i];
			l++;
		}
		else
		{
			value[n] = atoi (tmpstr);
			memset (tmpstr, 0, 128);
			l = 0;
			n++;
			if (n > 3)
			{
				break;
			}
		}
	}
	value[3] = atoi (tmpstr);
	for (i = 0; i < 4; i++)
	{
		if (value[i] > 255)
		{
			return -1;
		}
	}
	res =
		((value[0] & 0x000000ff) << 24) + ((value[1] & 0x000000ff) << 16) +
		((value[2] & 0x000000ff) << 8) + (value[3] & 0x000000ff);
	//   printf("res=%x\r\n",res);
	return res;
}

static void ajust_ipgateway (char *ipstr, char *mask, char *gateway)
{
	int ip_value = get_ip_value (ipstr);
	int mask_value = get_ip_value (mask);
	int gateway_value = get_ip_value (gateway);
	int value = ip_value & mask_value;
	int value1 = gateway_value & (~mask_value);
	gateway_value = value | value1;

	sprintf (gateway, "%d.%d.%d.%d",
			(gateway_value >> 24) & 0x000000ff,
			(gateway_value >> 16) & 0x000000ff,
			(gateway_value >> 8) & 0x000000ff,
			gateway_value & 0x000000ff);
}

static void get_time_zone_str(int time_zone, char *str)
{
	if(time_zone<0)
	{
		sprintf(str,"UTC+%.2d:%.2d:%.2d", abs(time_zone)/3600, (abs(time_zone)%3600)/60, abs(time_zone)%60);
	}
	else
	{
		sprintf(str,"UTC-%.2d:%.2d:%.2d", abs(time_zone)/3600, (abs(time_zone)%3600)/60, abs(time_zone)%60);
	}
	printf("time_zone=%d,[%s]\r\n",  time_zone, str);
}


static int client_send (client_t * client, char *buf, int buflen)
{
	int res;
	fd_set writefds;
	struct timeval tv;
	int ret;
	pthread_mutex_lock (&(client->mutex_send));
	if ((client->used == 1) && (client->sock_fd != -1))
	{
		//  printf ("client_send_0\r\n");
		//printf("%d,%d,%d\r\n",client->sock_fd,buf,buflen);
		FD_ZERO (&writefds);
		FD_SET (client->sock_fd, &writefds);
		tv.tv_sec = 0;
		tv.tv_usec = 1000;
		ret = select (client->sock_fd + 1, NULL, &writefds, NULL, &tv);
		if (ret == 1)
		{
			//  printf ("client_send_1\r\n");
			res = send (client->sock_fd, buf, buflen, 0);
			if (res <= 0)
			{
				pthread_mutex_unlock (&(client->mutex_send));
				return -1;
			}
		}
		else if (ret < 0)
		{
			//   printf ("client_send_2\r\n");
			client->used = -1;
			close (client->sock_fd);
			client->sock_fd = -1;
			pthread_mutex_unlock (&(client->mutex_send));
			return -1;
		}
		//  printf ("client_send_3\r\n");
	}
	else if (client->sock_fd != -1)
	{
		close (client->sock_fd);
		client->sock_fd = -1;
		//  printf ("client_send_4\r\n");
	}
	pthread_mutex_unlock (&(client->mutex_send));
	return 0;
}

static int findStrFromBuf (const char *buf, int buflen, const char *findingstr, int findinglen)
{
	int i, j;

	for (i = 0; i < buflen - findinglen + 1; i++)
	{
		for (j = 0; j < findinglen; j++)
		{
			if (buf[i + j] != findingstr[j])
				break;
			if (j == findinglen - 1)
				return i;
		}
	}

	return -1;
}


static int get_message (char *buf, char *result, char *left_str, char *right_str)
{
	int pos, pos1;
	int left_len = strlen (left_str);
	int right_len = strlen (right_str);
	int buflen = strlen (buf);
	pos = findStrFromBuf (buf, buflen, left_str, left_len);
	if (pos < 0)
	{
		return -1;
	}
	pos1 = findStrFromBuf (buf + pos, buflen - pos, right_str, right_len);
	if (pos1 < 0)
	{
		return -1;
	}
	memcpy (result, buf + pos + left_len, pos1 - left_len);
	return 0;
}

#if 0
static int get_message2 (char *buf, char *result, char *left_str, char *right_str)
{
	int pos, pos1, pos2;
	int left_len = strlen (left_str);
	int right_len = strlen (right_str);
	int buflen = strlen (buf);
	char tmpstr[HTTP_TEMP_STR_LEN];
	pos = findStrFromBuf (buf, buflen, left_str, left_len);
	if (pos < 0)
	{
		return -1;
	}
	pos1 = findStrFromBuf (buf + pos, buflen - pos, right_str, right_len);
	if (pos1 < 0)
	{
		return -1;
	}
	memset (tmpstr, 0, HTTP_TEMP_STR_LEN);
	memcpy (tmpstr, buf + pos + left_len, pos1 - left_len);
	pos2 = findStrFromBuf (tmpstr, strlen (tmpstr), ">", 1);
	if (pos2 < 0)
	{
		memcpy (result, tmpstr, strlen (tmpstr));
	}
	else
	{
		memcpy (result, tmpstr + pos2 + 1, strlen (tmpstr) - pos2 - 1);
	}
	return 0;
}
#endif

static int soap_get_value (char *buf, char *result, char *key_str)
{
	int i, n, pos, pos1, pos2;
	int key_len = strlen (key_str);
	int buflen = strlen (buf);
	char tmpstr[HTTP_TEMP_STR_LEN];
	char tmpstr1[HTTP_TEMP_STR_LEN];
	int len = 0;

	//<key />
	memset (tmpstr, 0, HTTP_TEMP_STR_LEN);
	sprintf (tmpstr, "<%s />", key_str);
	pos = findStrFromBuf (buf, buflen, tmpstr, strlen (tmpstr));
	if (pos >= 0)
	{
		//sprintf (result, "");
		bzero(result, strlen(result));
		len = pos + strlen (tmpstr);
		return len;
	}

	//<tds:key/>
	memset (tmpstr, 0, HTTP_TEMP_STR_LEN);
	sprintf (tmpstr, ":%s/>", key_str);
	pos = findStrFromBuf (buf, buflen, tmpstr, strlen (tmpstr));
	if (pos >= 0)
	{
		//sprintf (result, "");
		bzero(result, strlen(result));
		len = pos + strlen (tmpstr);
		return len;
	}

	//<tds:key />
	memset (tmpstr, 0, HTTP_TEMP_STR_LEN);
	sprintf (tmpstr, ":%s />", key_str);
	pos = findStrFromBuf (buf, buflen, tmpstr, strlen (tmpstr));
	if (pos >= 0)
	{
		//sprintf (result, "");
		bzero(result, strlen(result));
		len = pos + strlen (tmpstr);
		return len;
	}

	//<key>xxx</key>
	memset (tmpstr, 0, HTTP_TEMP_STR_LEN);
	sprintf (tmpstr, "<%s>", key_str);
	pos = findStrFromBuf (buf, buflen, tmpstr, strlen (tmpstr));
	if (pos >= 0)
	{
		memset (tmpstr, 0, HTTP_TEMP_STR_LEN);
		sprintf (tmpstr, "</%s>", key_str);
		pos1 = findStrFromBuf (buf + pos, buflen - pos, tmpstr, strlen (tmpstr));
		if (pos1 >= key_len + 2)
		{
			memcpy (result, buf + pos + key_len + 2, pos1 - key_len - 2);
			len = pos + pos1 + strlen (tmpstr);
			// printf ("pos = %d, pos1 = %d, strlen=%d\r\n", pos, pos1,
			// strlen (tmpstr));
			return len;
		}
		else
		{
			return -1;
		}
	}

	//<tds:key>xxx</tds:key>
	memset (tmpstr, 0, HTTP_TEMP_STR_LEN);
	sprintf (tmpstr, ":%s>", key_str);
	pos = findStrFromBuf (buf, buflen, tmpstr, strlen (tmpstr));
	if (pos > 0)
	{
		n = 0;
		for (i = pos - 1; i > 0; i--)
		{
			if (buf[i] == '/')
			{
				goto next;
			}
			if (buf[i] == '<')
			{
				break;
			}
			n++;
		}

		if (n > 0)
		{
			memset (tmpstr1, 0, HTTP_TEMP_STR_LEN);
			memcpy (tmpstr1, buf + pos - n, n);
			memset (tmpstr, 0, HTTP_TEMP_STR_LEN);
			sprintf (tmpstr, "</%s:%s>", tmpstr1, key_str);
			pos1 = findStrFromBuf (buf + pos, buflen - pos, tmpstr, strlen (tmpstr));
			if (pos1 >= key_len + 2)
			{
				memcpy (result, buf + pos + key_len + 2, pos1 - key_len - 2);
				len = pos + pos1 + strlen (tmpstr);
				return len;
			}
			else
			{
				return -1;
			}
		}
	}

next:
	//<tds:key a=xxxx>xxx</tds:key>
	memset (tmpstr, 0, HTTP_TEMP_STR_LEN);
	sprintf (tmpstr, ":%s ", key_str);
	pos = findStrFromBuf (buf, buflen, tmpstr, strlen (tmpstr));
	if (pos > 0)
	{
		n = 0;
		for (i = pos - 1; i >= 0; i--)
		{
			if (buf[i] == '<')
			{
				break;
			}
			n++;
		}

		if (n > 0)
		{
			memset (tmpstr1, 0, HTTP_TEMP_STR_LEN);
			memcpy (tmpstr1, buf + pos - n, n);
			pos1 = findStrFromBuf (buf + pos, buflen - pos, ">", 1);
			if (pos1 >= key_len + 2)
			{
				if (buf[pos + pos1 - 1] == '/')
				{
					//sprintf (result, "");
					bzero(result, strlen(result));
					len = pos + pos1;
					return len;
				}
				else
				{
					memset (tmpstr, 0, HTTP_TEMP_STR_LEN);
					sprintf (tmpstr, "</%s:%s>", tmpstr1, key_str);
					pos2 = findStrFromBuf (buf + pos, buflen - pos, tmpstr, strlen (tmpstr));
					if (pos2 > pos1)
					{
						memcpy (result, buf + pos + pos1 + 1, pos2 - pos1 - 1);
						len = pos + pos2 + strlen (tmpstr);
						return len;
					}
					else
					{
						return -1;
					}
				}
			}
		}
		else
		{
			return -1;
		}

	}


	//<key tds=xxxxxx>xxx</key>
	memset (tmpstr, 0, HTTP_TEMP_STR_LEN);
	sprintf (tmpstr, "<%s ", key_str);
	pos = findStrFromBuf (buf, buflen, tmpstr, strlen (tmpstr));
	if (pos >= 0)
	{
		pos1 = findStrFromBuf (buf + pos, buflen - pos, ">", 1);
		if (pos1 >= key_len + 2)
		{
			if (buf[pos + pos1 - 1] == '/')
			{
				//sprintf (result, "");
				bzero(result, strlen(result));
				len = pos + strlen (tmpstr);
				return len;
			}
			else
			{
				memset (tmpstr, 0, HTTP_TEMP_STR_LEN);
				sprintf (tmpstr, "</%s>", key_str);
				pos2 = findStrFromBuf (buf + pos, buflen - pos, tmpstr, strlen (tmpstr));
				if (pos2 > 0)
				{
					memcpy (result, buf + pos + pos1 + 1, pos2 - pos1 - 1);
					len = pos + pos2 + strlen (tmpstr);
					return len;
				}
				else
				{
					return -1;
				}
			}
		}
		else
		{
			return -1;
		}
	}
	return -1;
}


int http_init_client (client_t * client)
{
	client->used = 0;
	client->timeout = 0;
	client->sock_fd = -1;
	client->send_pos = 0;
	client->send_len = 0;
	client->recv_pos = 0;
	client->recv_len = 0;
	client->logined = 0;

	if (pthread_mutex_init (&(client->mutex_buf), NULL) != 0)
	{
		return -1;
	}
	if (pthread_mutex_init (&(client->mutex_send), NULL) != 0)
	{
		return -1;
	}

	client->send_buf = (unsigned char *) malloc (HTTP_SEND_BUF_SIZE);
	if (NULL == client->send_buf)
		return -1;
	client->sending_buf = (unsigned char *) malloc (HTTP_SEND_BUF_SIZE);
	if (NULL == client->sending_buf)
		return -1;
	client->recv_buf = (unsigned char *) malloc (HTTP_RECV_BUF_SIZE);
	if (NULL == client->recv_buf)
		return -1;

	return 0;
}


static void DoSystemReboot (char *cmdstr, char *soapstr, dvs_t * dvs, client_t * client)
{
	//char tmpstr[HTTP_TEMP_STR_LEN];
	char http_str[HTTP_TEMP_STR_LEN];
	char soap_str[MAX_SOAP_SIZE];

	memset (soap_str, 0, MAX_SOAP_SIZE);
	sprintf (soap_str, "%s%s%s",
			xml_version,
			soap_env_head,
			"<env:Body><tds:SystemRebootResponse><tds:Message>Rebooting in 90 seconds</tds:Message>");

	strcat (soap_str,
			"</tds:SystemRebootResponse></env:Body></env:Envelope>\r\n");


	memset (http_str, 0, HTTP_TEMP_STR_LEN);
	sprintf (http_str, http_head_str, strlen (soap_str));

	client_send (client, http_str, strlen (http_str));
	client_send (client, soap_str, strlen (soap_str));

	// fix me Bug 1. reboot; 2. open WDT
#if 0
	//send_cmd (dvs, "reboot");
	usleep(200);
	//send_cmd (dvs, "reboot");
	usleep(200);
	//system("reboot");
	usleep(200);
	//system("reboot");
#else
	printf("testting !!!!!!!! not reboot\n");
#endif
}

static void DoSetSystemFactoryDefault (char *cmdstr, char *soapstr, dvs_t * dvs, client_t * client)
{
	//char tmpstr[HTTP_TEMP_STR_LEN];
	char http_str[HTTP_TEMP_STR_LEN];
	char soap_str[MAX_SOAP_SIZE];

	memset (soap_str, 0, MAX_SOAP_SIZE);
	sprintf (soap_str, "%s%s%s",
			xml_version,
			soap_env_head, "<env:Body><tds:SetSystemFactoryDefaultResponse>");

	strcat (soap_str,
			"</tds:SetSystemFactoryDefaultResponse></env:Body></env:Envelope>\r\n");

	memset (http_str, 0, HTTP_TEMP_STR_LEN);
	sprintf (http_str, http_head_str, strlen (soap_str));

	client_send (client, http_str, strlen (http_str));
	client_send (client, soap_str, strlen (soap_str));
	set_default_config(dvs);
	//send_cmd (dvs, "reboot");
}

static void DoGetSystemDateAndTime (char *cmdstr, char *soapstr, dvs_t * dvs, client_t * client)
{
	char tmpstr[HTTP_TEMP_STR_LEN];
	char tmpstr1[TEMP_STR_LEN];
	char http_str[HTTP_TEMP_STR_LEN];
	char soap_str[MAX_SOAP_SIZE];
	struct tm timeval;
	struct tm timeval1;
	time_t timecal;
	time (&timecal);

	localtime_r (&timecal, &timeval);
	timecal -= dvs->time_zone;
	localtime_r (&timecal, &timeval1);

	memset (soap_str, 0, MAX_SOAP_SIZE);
	sprintf (soap_str, "%s%s%s",
			xml_version,
			soap_env_head,
			"<env:Body><tds:GetSystemDateAndTimeResponse><tds:SystemDateAndTime>");

	memset (tmpstr1, 0, TEMP_STR_LEN);
	get_time_zone_str(dvs->time_zone, tmpstr1);

	memset (tmpstr, 0, HTTP_TEMP_STR_LEN);
	sprintf (tmpstr,
			"<tt:DateTimeType>Manual</tt:DateTimeType><tt:DaylightSavings>%s</tt:DaylightSavings><tt:TimeZone><tt:TZ>%s</tt:TZ></tt:TimeZone>", "false", tmpstr1);
	strcat (soap_str, tmpstr);

	memset (tmpstr, 0, HTTP_TEMP_STR_LEN);
	sprintf (tmpstr,
			"<tt:UTCDateTime><tt:Time><tt:Hour>%d</tt:Hour><tt:Minute>%d</tt:Minute><tt:Second>%d</tt:Second></tt:Time><tt:Date><tt:Year>%d</tt:Year><tt:Month>%d</tt:Month><tt:Day>%d</tt:Day></tt:Date></tt:UTCDateTime>",
			timeval1.tm_hour, timeval1.tm_min, timeval1.tm_sec,
			timeval1.tm_year + 1900, timeval1.tm_mon + 1, timeval1.tm_mday);
	strcat (soap_str, tmpstr);

	memset (tmpstr, 0, HTTP_TEMP_STR_LEN);
	sprintf (tmpstr,
			"<tt:LocalDateTime><tt:Time><tt:Hour>%d</tt:Hour><tt:Minute>%d</tt:Minute><tt:Second>%d</tt:Second></tt:Time><tt:Date><tt:Year>%d</tt:Year><tt:Month>%d</tt:Month><tt:Day>%d</tt:Day></tt:Date></tt:LocalDateTime>",
			timeval.tm_hour, timeval.tm_min, timeval.tm_sec,
			timeval.tm_year + 1900, timeval.tm_mon + 1, timeval.tm_mday);

	strcat (soap_str, tmpstr);
	strcat (soap_str,
			"</tds:SystemDateAndTime></tds:GetSystemDateAndTimeResponse></env:Body></env:Envelope>\r\n");

	memset (http_str, 0, HTTP_TEMP_STR_LEN);
	sprintf (http_str, http_head_str, strlen (soap_str));

	client_send (client, http_str, strlen (http_str));
	//printf("http_str=[%s]\r\n", http_str);
	client_send (client, soap_str, strlen (soap_str));
	//printf("soap_str=[%s]\r\n", soap_str);
}

static void DoGetDeviceInformation (char *cmdstr, char *soapstr, dvs_t * dvs, client_t * client)
{
	char tmpstr[HTTP_TEMP_STR_LEN];
	char http_str[HTTP_TEMP_STR_LEN];
	char soap_str[MAX_SOAP_SIZE];
	char sn_str[TEMP_STR_LEN];
	int i, len, n;
	memset (sn_str, 0, TEMP_STR_LEN);
	len = strlen (g_dvs.macaddr);
	n = 0;
	for (i = 0; i < len; i++)
	{
		if (g_dvs.macaddr[i] != ':')
		{
			sn_str[n] = g_dvs.macaddr[i];
			n++;
		}
	}
	memset (soap_str, 0, MAX_SOAP_SIZE);
	sprintf (soap_str, "%s%s%s",
			xml_version,
			soap_env_head, "<env:Body><tds:GetDeviceInformationResponse>");

	memset (tmpstr, 0, HTTP_TEMP_STR_LEN);
	sprintf (tmpstr,
			"<tds:Manufacturer>H.264_IPCamera</tds:Manufacturer><tds:Model>AK3918_01</tds:Model><tds:FirmwareVersion>V1.0.1</tds:FirmwareVersion>");
	strcat (soap_str, tmpstr);

	memset (tmpstr, 0, HTTP_TEMP_STR_LEN);
	sprintf (tmpstr,
			"<tds:SerialNumber>%s</tds:SerialNumber><tds:HardwareId>%s</tds:HardwareId>",
			sn_str, "AK3918");
	strcat (soap_str, tmpstr);

	strcat (soap_str,
			"</tds:GetDeviceInformationResponse></env:Body></env:Envelope>\r\n");

	memset (http_str, 0, HTTP_TEMP_STR_LEN);
	sprintf (http_str, http_head_str, strlen (soap_str));

	client_send (client, http_str, strlen (http_str));
	client_send (client, soap_str, strlen (soap_str));

}

static void DoSetScopes (char *cmdstr, char *soapstr, dvs_t * dvs, client_t * client)
{
	int pos, i;
	char *dst = NULL;
	//char tmpstr[HTTP_TEMP_STR_LEN];
	char http_str[HTTP_TEMP_STR_LEN];
	char soap_str[MAX_SOAP_SIZE];
	char city[512];
	char name[512];

	get_message(soapstr, soap_str, "<SetScopes", "</SetScopes>");

	// get xxx  cityxxx<
	memset(city, 0, sizeof(city)/sizeof(char));
	pos = findStrFromBuf(soap_str, strlen(soap_str), "location", strlen("location"));
	if (-1 != pos)
	{
		dst = soap_str + pos + strlen("location") + 1;
		for (i = 0;*dst != '<';dst++, i++)
		{
			city[i] = *dst;
		}
		city[i] = '\0';
		printf("%s", city);
		sprintf(dvs->onvif_data[0].identification_location, "%s", city);
	}
	// get xxx  namexxx<
	memset(name, 0, sizeof(name)/sizeof(char));
	pos = findStrFromBuf(soap_str, strlen(soap_str), "name:", strlen("name:"));
	if (-1 != pos)
	{
		dst = soap_str + pos + strlen("name:");
		for (i = 0;*dst != '<';dst++, i++)
		{
			name[i] = *dst;
		}
		name[i] = '\0';
		printf("%s", name);
		sprintf(dvs->onvif_data[0].identification_name, "%s", name);
	}
	SetOnvifData(dvs->onvif_data);

	memset (soap_str, 0, MAX_SOAP_SIZE);
	sprintf (soap_str, "%s%s%s",
			xml_version, soap_env_head, "<env:Body><tds:SetScopesResponse />");

	strcat (soap_str, "</env:Body></env:Envelope>\r\n");

	memset (http_str, 0, HTTP_TEMP_STR_LEN);
	sprintf (http_str, http_head_str, strlen (soap_str));

	client_send (client, http_str, strlen (http_str));
	client_send (client, soap_str, strlen (soap_str));

}

static void DoGetScopes (char *cmdstr, char *soapstr, dvs_t * dvs, client_t * client)
{
	char tmpstr[HTTP_TEMP_STR_LEN];
	char http_str[HTTP_TEMP_STR_LEN];
	char soap_str[MAX_SOAP_SIZE];

	memset (soap_str, 0, MAX_SOAP_SIZE);
	sprintf (soap_str, "%s%s%s",
			xml_version, soap_env_head, "<env:Body><tds:GetScopesResponse>");

	memset (tmpstr, 0, HTTP_TEMP_STR_LEN);
	sprintf (tmpstr,
			"<tds:Scopes><tt:ScopeDef>%s</tt:ScopeDef><tt:ScopeItem>onvif://www.onvif.org/type/video_encoder</tt:ScopeItem></tds:Scopes>",
			"Fixed");
	strcat (soap_str, tmpstr);

	memset (tmpstr, 0, HTTP_TEMP_STR_LEN);
	sprintf (tmpstr,
			"<tds:Scopes><tt:ScopeDef>%s</tt:ScopeDef><tt:ScopeItem>onvif://www.onvif.org/type/ptz</tt:ScopeItem></tds:Scopes>",
			"Fixed");
	strcat (soap_str, tmpstr);

	memset (tmpstr, 0, HTTP_TEMP_STR_LEN);
	sprintf (tmpstr,
			"<tds:Scopes><tt:ScopeDef>%s</tt:ScopeDef><tt:ScopeItem>onvif://www.onvif.org/type/audio_encoder</tt:ScopeItem></tds:Scopes>",
			"Fixed");
	strcat (soap_str, tmpstr);

	memset (tmpstr, 0, HTTP_TEMP_STR_LEN);
	/*sprintf (tmpstr,
	  "<tds:Scopes><tt:ScopeDef>%s</tt:ScopeDef><tt:ScopeItem>onvif://www.onvif.org/location/city/guangzhou</tt:ScopeItem></tds:Scopes>",
	  "Configurable");*/
	sprintf (tmpstr,
			"<tds:Scopes><tt:ScopeDef>%s</tt:ScopeDef><tt:ScopeItem>onvif://www.onvif.org/location/%s</tt:ScopeItem></tds:Scopes>",
			"Configurable", dvs->onvif_data[0].identification_location);
	strcat (soap_str, tmpstr);

	memset (tmpstr, 0, HTTP_TEMP_STR_LEN);
	sprintf (tmpstr,
			"<tds:Scopes><tt:ScopeDef>%s</tt:ScopeDef><tt:ScopeItem>onvif://www.onvif.org/hardware/AK3918_01</tt:ScopeItem></tds:Scopes>",
			"Fixed");
	strcat (soap_str, tmpstr);

	memset (tmpstr, 0, HTTP_TEMP_STR_LEN);
	/*sprintf (tmpstr,
	  "<tds:Scopes><tt:ScopeDef>%s</tt:ScopeDef><tt:ScopeItem>onvif://www.onvif.org/name/H.264_IPCamera</tt:ScopeItem></tds:Scopes>",
	  "Fixed");*/
	sprintf (tmpstr,
			"<tds:Scopes><tt:ScopeDef>%s</tt:ScopeDef><tt:ScopeItem>onvif://www.onvif.org/name/%s</tt:ScopeItem></tds:Scopes>",
			"Fixed", dvs->onvif_data[0].identification_name);
	strcat (soap_str, tmpstr);

	memset (tmpstr, 0, HTTP_TEMP_STR_LEN);
	sprintf (tmpstr,
			"<tds:Scopes><tt:ScopeDef>%s</tt:ScopeDef><tt:ScopeItem>onvif://www.onvif.org/Profile/Streaming</tt:ScopeItem></tds:Scopes>",
			"Fixed");
	strcat (soap_str, tmpstr);

	strcat (soap_str, "</tds:GetScopesResponse></env:Body></env:Envelope>\r\n");

	memset (http_str, 0, HTTP_TEMP_STR_LEN);
	sprintf (http_str, http_head_str, strlen (soap_str));

	client_send (client, http_str, strlen (http_str));
	client_send (client, soap_str, strlen (soap_str));

}

static void DoGetCapabilities (char *cmdstr, char *soapstr, dvs_t * dvs, client_t * client)
{
	char tmpstr[HTTP_TEMP_STR_LEN];
	char http_str[HTTP_TEMP_STR_LEN];
	char soap_str[MAX_SOAP_SIZE];
	char category[HTTP_TEMP_STR_LEN];

	memset (category, 0, HTTP_TEMP_STR_LEN);

	int category_all = 0;
	int category_analytics = 0;
	int category_device = 0;
	int category_events = 0;
	int category_imaging = 0;
	int category_media = 0;
	int category_ptz = 0;

	memset (soap_str, 0, MAX_SOAP_SIZE);
	sprintf (soap_str, "%s%s%s",
			xml_version,
			soap_env_head,
			"<env:Body><tds:GetCapabilitiesResponse><tds:Capabilities>");

	int res = soap_get_value (soapstr, category, "Category");
	if (res < 0)
	{
		category_all = 1;
	}
	else
		if (findStrFromBuf (category, strlen (category), "All", strlen ("All")) >= 0)
		{
			category_all = 1;
		}
		else
		{
			if (findStrFromBuf (category, strlen (category), "Analytics", strlen ("Analytics")) >= 0)
			{
				category_analytics = 1;
			}
			if (findStrFromBuf (category, strlen (category), "Device", strlen ("Device")) >= 0)
			{
				category_device = 1;
			}
			if (findStrFromBuf (category, strlen (category), "Events", strlen ("Events")) >= 0)
			{
				category_events = 1;
			}
			if (findStrFromBuf (category, strlen (category), "Imaging", strlen ("Imaging")) >= 0)
			{
				category_imaging = 1;
			}
			if (findStrFromBuf (category, strlen (category), "Media", strlen ("Media")) >= 0)
			{
				category_media = 1;
			}
			if (findStrFromBuf (category, strlen (category), "PTZ", strlen ("PTZ")) >= 0)
			{
				category_ptz = 1;
			}
		}

	if ((category_analytics == 1) || (category_all == 1))
	{
		memset (tmpstr, 0, HTTP_TEMP_STR_LEN);
		sprintf (tmpstr,
				"<tt:Analytics><tt:XAddr>http://%s/onvif/Analytics</tt:XAddr><tt:RuleSupport>true</tt:RuleSupport><tt:AnalyticsModuleSupport>true</tt:AnalyticsModuleSupport></tt:Analytics>", g_dvs.config.network.ip);
		strcat (soap_str, tmpstr);
	}

	if ((category_device == 1) || (category_all == 1))
	{
		memset (tmpstr, 0, HTTP_TEMP_STR_LEN);
		sprintf (tmpstr,
				"<tt:Device><tt:XAddr>http://%s/onvif/device_service</tt:XAddr><tt:Network><tt:IPFilter>true</tt:IPFilter><tt:ZeroConfiguration>true</tt:ZeroConfiguration><tt:IPVersion6>true</tt:IPVersion6><tt:DynDNS>true</tt:DynDNS><tt:Extension><tt:Dot11Configuration>false</tt:Dot11Configuration></tt:Extension></tt:Network><tt:System><tt:DiscoveryResolve>false</tt:DiscoveryResolve><tt:DiscoveryBye>true</tt:DiscoveryBye><tt:RemoteDiscovery>true</tt:RemoteDiscovery><tt:SystemBackup>false</tt:SystemBackup><tt:SystemLogging>false</tt:SystemLogging><tt:FirmwareUpgrade>false</tt:FirmwareUpgrade><tt:SupportedVersions><tt:Major>2</tt:Major><tt:Minor>1</tt:Minor></tt:SupportedVersions><tt:SupportedVersions><tt:Major>2</tt:Major><tt:Minor>0</tt:Minor></tt:SupportedVersions><tt:Extension><tt:HttpFirmwareUpgrade>true</tt:HttpFirmwareUpgrade><tt:HttpSystemBackup>false</tt:HttpSystemBackup><tt:HttpSystemLogging>false</tt:HttpSystemLogging><tt:HttpSupportInformation>false</tt:HttpSupportInformation></tt:Extension></tt:System><tt:IO><tt:InputConnectors>0</tt:InputConnectors><tt:RelayOutputs>0</tt:RelayOutputs><tt:Extension><tt:Auxiliary>false</tt:Auxiliary><tt:AuxiliaryCommands>nothing</tt:AuxiliaryCommands><tt:Extension/></tt:Extension></tt:IO><tt:Security><tt:TLS1.1>false</tt:TLS1.1><tt:TLS1.2>false</tt:TLS1.2><tt:OnboardKeyGeneration>false</tt:OnboardKeyGeneration><tt:AccessPolicyConfig>false</tt:AccessPolicyConfig><tt:X.509Token>false</tt:X.509Token><tt:SAMLToken>false</tt:SAMLToken><tt:KerberosToken>false</tt:KerberosToken><tt:RELToken>false</tt:RELToken><tt:Extension><tt:TLS1.0>false</tt:TLS1.0><tt:Extension><tt:Dot1X>false</tt:Dot1X><tt:SupportedEAPMethod>0</tt:SupportedEAPMethod><tt:RemoteUserHandling>false</tt:RemoteUserHandling></tt:Extension></tt:Extension></tt:Security></tt:Device>",
				g_dvs.config.network.ip);
		strcat (soap_str, tmpstr);
	}

	if ((category_events == 1) || (category_all == 1))
	{
		memset (tmpstr, 0, HTTP_TEMP_STR_LEN);
		sprintf (tmpstr,
				"<tt:Events><tt:XAddr>http://%s/onvif/Events</tt:XAddr><tt:WSSubscriptionPolicySupport>true</tt:WSSubscriptionPolicySupport><tt:WSPullPointSupport>true</tt:WSPullPointSupport><tt:WSPausableSubscriptionManagerInterfaceSupport>false</tt:WSPausableSubscriptionManagerInterfaceSupport></tt:Events>",
				g_dvs.config.network.ip);
		strcat (soap_str, tmpstr);
	}

	if ((category_imaging == 1) || (category_all == 1))
	{
		memset (tmpstr, 0, HTTP_TEMP_STR_LEN);
		sprintf (tmpstr,
				"<tt:Imaging><tt:XAddr>http://%s/onvif/Imaging</tt:XAddr></tt:Imaging>", g_dvs.config.network.ip);
		strcat (soap_str, tmpstr);
	}

	if ((category_media == 1) || (category_all == 1))
	{
		memset (tmpstr, 0, HTTP_TEMP_STR_LEN);
		sprintf (tmpstr,
				"<tt:Media><tt:XAddr>http://%s/onvif/Media</tt:XAddr><tt:StreamingCapabilities><tt:RTPMulticast>false</tt:RTPMulticast><tt:RTP_TCP>true</tt:RTP_TCP><tt:RTP_RTSP_TCP>true</tt:RTP_RTSP_TCP></tt:StreamingCapabilities><tt:Extension><tt:ProfileCapabilities><tt:MaximumNumberOfProfiles>3</tt:MaximumNumberOfProfiles></tt:ProfileCapabilities></tt:Extension></tt:Media>",
				g_dvs.config.network.ip);
		strcat (soap_str, tmpstr);
	}

	if ((category_ptz == 1) || (category_all == 1))
	{
		memset (tmpstr, 0, HTTP_TEMP_STR_LEN);
		sprintf (tmpstr,
				"<tt:PTZ><tt:XAddr>http://%s/onvif/ptz_service</tt:XAddr></tt:PTZ>", g_dvs.config.network.ip);
		strcat (soap_str, tmpstr);
	}

	if ((category_device == 1) || (category_all == 1))
	{
		memset (tmpstr, 0, HTTP_TEMP_STR_LEN);
		sprintf (tmpstr,
				"<tt:Extension><tt:DeviceIO><tt:XAddr>http://%s/onvif/DeviceIO</tt:XAddr><tt:VideoSources>0</tt:VideoSources><tt:VideoOutputs>0</tt:VideoOutputs><tt:AudioSources>0</tt:AudioSources><tt:AudioOutputs>0</tt:AudioOutputs><tt:RelayOutputs>0</tt:RelayOutputs></tt:DeviceIO></tt:Extension>", g_dvs.config.network.ip);
		strcat (soap_str, tmpstr);
	}

	strcat (soap_str,
			"</tds:Capabilities></tds:GetCapabilitiesResponse></env:Body></env:Envelope>\r\n");

	memset (http_str, 0, HTTP_TEMP_STR_LEN);
	sprintf (http_str, http_head_str, strlen (soap_str));

	client_send (client, http_str, strlen (http_str));
	client_send (client, soap_str, strlen (soap_str));

}

static void DoGetNetworkInterfaces (char *cmdstr, char *soapstr, dvs_t * dvs, client_t * client)
{
	char tmpstr[HTTP_TEMP_STR_LEN];
	char http_str[HTTP_TEMP_STR_LEN];
	char soap_str[MAX_SOAP_SIZE];
	struct netinfo_t netinfo;
	GetNet(&netinfo);
	g_dvs.config.network.type = netinfo.IpType;
	// printf("%d\r\n", g_dvs.config.network.type);

	memset (soap_str, 0, MAX_SOAP_SIZE);
	sprintf (soap_str, "%s%s%s",
			xml_version,
			soap_env_head, "<env:Body><tds:GetNetworkInterfacesResponse>");

	memset (tmpstr, 0, HTTP_TEMP_STR_LEN);
	if (g_dvs.config.network.type == 1)
	{
		int prefixlen = get_prefixlen_by_addr(g_dvs.config.network.netmask);
		sprintf (tmpstr, "<tds:NetworkInterfaces token=\"eth0\"><tt:Enabled>true</tt:Enabled><tt:Info><tt:Name >eth0</tt:Name ><tt:HwAddress >%s</tt:HwAddress ><tt:MTU>1500</tt:MTU></tt:Info><tt:Link><tt:AdminSettings><tt:AutoNegotiation>true</tt:AutoNegotiation><tt:Speed>100</tt:Speed><tt:Duplex>Full</tt:Duplex></tt:AdminSettings><tt:OperSettings><tt:AutoNegotiation>true</tt:AutoNegotiation><tt:Speed>100</tt:Speed><tt:Duplex>Full</tt:Duplex></tt:OperSettings><tt:InterfaceType>0</tt:InterfaceType></tt:Link><tt:IPv4><tt:Enabled>true</tt:Enabled><tt:Config><tt:FromDHCP><tt:Address>%s</tt:Address><tt:PrefixLength>%d</tt:PrefixLength></tt:FromDHCP><tt:DHCP>%s</tt:DHCP></tt:Config></tt:IPv4><tt:IPv6><tt:Enabled>false</tt:Enabled></tt:IPv6></tds:NetworkInterfaces>", g_dvs.macaddr, g_dvs.config.network.ip, prefixlen, "true");
		//sprintf (tmpstr, "<tds:NetworkInterfaces token=\"eth0\"><tt:Enabled>true</tt:Enabled><tt:Info><tt:Name>eth0</tt:Name><tt:HwAddress>%s</tt:HwAddress><tt:MTU>1500</tt:MTU></tt:Info><tt:Link><tt:AdminSettings><tt:AutoNegotiation>true</tt:AutoNegotiation><tt:Speed>100</tt:Speed><tt:Duplex>Full</tt:Duplex></tt:AdminSettings><tt:OperSettings><tt:AutoNegotiation>true</tt:AutoNegotiation><tt:Speed>100</tt:Speed><tt:Duplex>Full</tt:Duplex></tt:OperSettings><tt:InterfaceType>0</tt:InterfaceType></tt:Link><tt:IPv4><tt:Enabled>true</tt:Enabled><tt:Config><tt:FromDHCP><tt:Address>%s</tt:Address><tt:PrefixLength>24</tt:PrefixLength></tt:FromDHCP><tt:DHCP>%s</tt:DHCP></tt:Config></tt:IPv4><tt:IPv6><tt:Enabled>false</tt:Enabled></tt:IPv6></tds:NetworkInterfaces>", g_dvs.macaddr, g_dvs.config.network.ip, "true");
		// sprintf (tmpstr, "<tds:NetworkInterfaces token=\"eth0\"><tt:Enabled>true</tt:Enabled><tt:Info><tt:Name>eth0</tt:Name><tt:HwAddress>%s</tt:HwAddress><tt:MTU>1500</tt:MTU></tt:Info><tt:Link><tt:AdminSettings><tt:AutoNegotiation>false</tt:AutoNegotiation><tt:Speed>10</tt:Speed><tt:Duplex>Full</tt:Duplex></tt:AdminSettings><tt:OperSettings><tt:AutoNegotiation>true</tt:AutoNegotiation><tt:Speed>10</tt:Speed><tt:Duplex>Full</tt:Duplex></tt:OperSettings><tt:InterfaceType>0</tt:InterfaceType></tt:Link><tt:IPv4><tt:Enabled>true</tt:Enabled><tt:Config><tt:Manual><tt:Address>%s</tt:Address><tt:PrefixLength>24</tt:PrefixLength></tt:Manual><tt:DHCP>%s</tt:DHCP></tt:Config></tt:IPv4><tt:IPv6><tt:Enabled>false</tt:Enabled></tt:IPv6></tds:NetworkInterfaces>", g_dvs.macaddr, g_dvs.config.network.ip, "true");
	}
	else
	{
		// sprintf (tmpstr, "<tds:NetworkInterfaces token=\"eth0\"><tt:Enabled>true</tt:Enabled><tt:Info><tt:Name >eth0</tt:Name ><tt:HwAddress >%s</tt:HwAddress ><tt:MTU>1500</tt:MTU></tt:Info><tt:Link><tt:AdminSettings><tt:AutoNegotiation>true</tt:AutoNegotiation><tt:Speed>100</tt:Speed><tt:Duplex>Full</tt:Duplex></tt:AdminSettings><tt:OperSettings><tt:AutoNegotiation>true</tt:AutoNegotiation><tt:Speed>100</tt:Speed><tt:Duplex>Full</tt:Duplex></tt:OperSettings><tt:InterfaceType>0</tt:InterfaceType></tt:Link><tt:IPv4><tt:Enabled>true</tt:Enabled><tt:Config><tt:FromDHCP><tt:Address>%s</tt:Address><tt:PrefixLength>24</tt:PrefixLength></tt:FromDHCP><tt:DHCP>%s</tt:DHCP></tt:Config></tt:IPv4><tt:IPv6><tt:Enabled>false</tt:Enabled></tt:IPv6></tds:NetworkInterfaces>", g_dvs.macaddr, g_dvs.config.network.ip, "true");
		// sprintf (tmpstr, "<tds:NetworkInterfaces token=\"eth0\"><tt:Enabled>true</tt:Enabled><tt:Info><tt:Name >eth0</tt:Name><tt:HwAddress>%s</tt:HwAddress><tt:MTU>1500</tt:MTU></tt:Info><tt:Link><tt:AdminSettings><tt:AutoNegotiation>true</tt:AutoNegotiation><tt:Speed>100</tt:Speed><tt:Duplex>Full</tt:Duplex></tt:AdminSettings><tt:OperSettings><tt:AutoNegotiation>true</tt:AutoNegotiation><tt:Speed>100</tt:Speed><tt:Duplex>Full</tt:Duplex></tt:OperSettings><tt:InterfaceType>0</tt:InterfaceType></tt:Link><tt:IPv4><tt:Enabled>true</tt:Enabled><tt:Config><tt:FromDHCP><tt:Address>%s</tt:Address><tt:PrefixLength>24</tt:PrefixLength></tt:FromDHCP><tt:DHCP>%s</tt:DHCP></tt:Config></tt:IPv4><tt:IPv6><tt:Enabled>false</tt:Enabled></tt:IPv6></tds:NetworkInterfaces>", g_dvs.macaddr, g_dvs.config.network.ip, "false");
		int prefixlen = get_prefixlen_by_addr(g_dvs.config.network.netmask);
		sprintf (tmpstr, "<tds:NetworkInterfaces token=\"eth0\"><tt:Enabled>true</tt:Enabled><tt:Info><tt:Name>eth0</tt:Name><tt:HwAddress>%s</tt:HwAddress><tt:MTU>1500</tt:MTU></tt:Info><tt:Link><tt:AdminSettings><tt:AutoNegotiation>false</tt:AutoNegotiation><tt:Speed>10</tt:Speed><tt:Duplex>Full</tt:Duplex></tt:AdminSettings><tt:OperSettings><tt:AutoNegotiation>false</tt:AutoNegotiation><tt:Speed>10</tt:Speed><tt:Duplex>Full</tt:Duplex></tt:OperSettings><tt:InterfaceType>0</tt:InterfaceType></tt:Link><tt:IPv4><tt:Enabled>true</tt:Enabled><tt:Config><tt:Manual><tt:Address>%s</tt:Address><tt:PrefixLength>%d</tt:PrefixLength></tt:Manual><tt:DHCP>%s</tt:DHCP></tt:Config></tt:IPv4><tt:IPv6><tt:Enabled>false</tt:Enabled></tt:IPv6></tds:NetworkInterfaces>", g_dvs.macaddr, g_dvs.config.network.ip,
			   prefixlen, "false");
	}
	// printf("DoGetNetworkInterfaces[%s]", tmpstr);
	strcat (soap_str, tmpstr);

	strcat (soap_str,
			"</tds:GetNetworkInterfacesResponse></env:Body></env:Envelope>\r\n");

	memset (http_str, 0, HTTP_TEMP_STR_LEN);
	sprintf (http_str, http_head_str, strlen (soap_str));

	client_send (client, http_str, strlen (http_str));
	client_send (client, soap_str, strlen (soap_str));

}

static void DoGetDNS (char *cmdstr, char *soapstr, dvs_t * dvs, client_t * client)
{
	char tmpstr[HTTP_TEMP_STR_LEN];
	char http_str[HTTP_TEMP_STR_LEN];
	char soap_str[MAX_SOAP_SIZE];

	memset (soap_str, 0, MAX_SOAP_SIZE);
	sprintf (soap_str, "%s%s%s",
			xml_version, soap_env_head, "<env:Body><tds:GetDNSResponse>");

	memset (tmpstr, 0, HTTP_TEMP_STR_LEN);
	// DHCP //0:Fixed IP  1:dynamic ip
	// DNS  //0:no  1:yes
	if (g_dvs.config.network.auto_get_dns == 1)
	{
		sprintf (tmpstr,
				"<tds:DNSInformation><tt:FromDHCP>true</tt:FromDHCP><tt:DNSManual><tt:Type>IPv4</tt:Type><tt:IPv4Address>%s</tt:IPv4Address></tt:DNSManual></tds:DNSInformation>", g_dvs.config.network.dns1);
	}
	else
	{
		sprintf (tmpstr,
				"<tds:DNSInformation><tt:FromDHCP>false</tt:FromDHCP><tt:DNSManual><tt:Type>IPv4</tt:Type><tt:IPv4Address>%s</tt:IPv4Address></tt:DNSManual></tds:DNSInformation>", g_dvs.config.network.dns1);
	}

	strcat (soap_str, tmpstr);
	strcat (soap_str, "</tds:GetDNSResponse></env:Body></env:Envelope>\r\n");

	memset (http_str, 0, HTTP_TEMP_STR_LEN);
	sprintf (http_str, http_head_str, strlen (soap_str));

	client_send (client, http_str, strlen (http_str));
	client_send (client, soap_str, strlen (soap_str));
}

static void DoSetDNS (char *cmdstr, char *soapstr, dvs_t * dvs, client_t * client)
{
	char http_str[HTTP_TEMP_STR_LEN];
	char soap_str[MAX_SOAP_SIZE];
	//config_network_t *network = &(dvs->config.network);

	memset (soap_str, 0, MAX_SOAP_SIZE);
	sprintf (soap_str, "%s%s%s", xml_version, soap_env_head, "<env:Body><tds:SetDNSResponse/>");

	strcat (soap_str, "</env:Body></env:Envelope>\r\n");
	memset (http_str, 0, HTTP_TEMP_STR_LEN);
	sprintf (http_str, http_head_str, strlen (soap_str));

	client_send (client, http_str, strlen (http_str));
	client_send (client, soap_str, strlen (soap_str));
}

static void DoGetServices (char *cmdstr, char *soapstr, dvs_t * dvs, client_t * client)
{
	char tmpstr[HTTP_TEMP_STR_LEN];
	char http_str[HTTP_TEMP_STR_LEN];
	char soap_str[MAX_SOAP_SIZE];
	char IncludeCapability[HTTP_TEMP_STR_LEN];
	int include_capbility = 0;

	memset (IncludeCapability, 0, HTTP_TEMP_STR_LEN);
	//int res = soap_get_value (soapstr, IncludeCapability, "IncludeCapability");

	if (findStrFromBuf (IncludeCapability, strlen (IncludeCapability), "true", strlen ("true")) >= 0)
	{
		include_capbility = 1;
	}

	memset (soap_str, 0, MAX_SOAP_SIZE);
	sprintf (soap_str, "%s%s%s", xml_version, soap_env_head, "<env:Body><tds:GetServicesResponse>");
	//----------------------------------
	memset (tmpstr, 0, HTTP_TEMP_STR_LEN);
	sprintf (tmpstr,
			"<tds:Service><tds:Namespace>http://www.onvif.org/ver10/device/wsdl</tds:Namespace><tds:XAddr>http://%s/onvif/device_service</tds:XAddr>", g_dvs.config.network.ip);
	strcat (soap_str, tmpstr);

	if (include_capbility == 1)
	{
		memset (tmpstr, 0, HTTP_TEMP_STR_LEN);
		sprintf (tmpstr,
				"<tds:Capabilities><tds:Capabilities><tds:Network IPFilter=\"true\" ZeroConfiguration=\"true\" IPVersion6=\"false\" DynDNS=\"true\" Dot11Configuration=\"false\" HostnameFromDHCP=\"true\" NTP=\"1\"></tds:Network><tds:Security TLS1.0=\"false\" TLS1.1=\"false\" TLS1.2=\"false\" OnboardKeyGeneration=\"false\" AccessPolicyConfig=\"false\" DefaultAccessPolicy=\"true\" Dot1X=\"false\" RemoteUserHandling=\"false\" X.509Token=\"false\" SAMLToken=\"false\" KerberosToken=\"false\" UsernameToken=\"true\" HttpDigest=\"false\" RELToken=\"false\" SupportedEAPMethods=\"0\"></tds:Security><tds:System DiscoveryResolve=\"false\" DiscoveryBye=\"true\" RemoteDiscovery=\"true\" SystemBackup=\"false\" SystemLogging=\"false\" FirmwareUpgrade=\"false\" HttpFirmwareUpgrade=\"true\" HttpSystemBackup=\"false\" HttpSystemLogging=\"false\" HttpSupportInformation=\"false\"></tds:System></tds:Capabilities></tds:Capabilities>");
		strcat (soap_str, tmpstr);
	}
	memset (tmpstr, 0, HTTP_TEMP_STR_LEN);
	sprintf (tmpstr,
			"<tds:Version><tt:Major>2</tt:Major><tt:Minor>1</tt:Minor></tds:Version></tds:Service>");
	strcat (soap_str, tmpstr);


	//-------------------------------
	memset (tmpstr, 0, HTTP_TEMP_STR_LEN);
	sprintf (tmpstr,
			"<tds:Service><tds:Namespace>http://www.onvif.org/ver10/media/wsdl</tds:Namespace><tds:XAddr>http://%s/onvif/Media</tds:XAddr>", g_dvs.config.network.ip);
	strcat (soap_str, tmpstr);

	if (include_capbility == 1)
	{
		memset (tmpstr, 0, HTTP_TEMP_STR_LEN);
		sprintf (tmpstr,
				"<tds:Capabilities><trt:Capabilities><trt:ProfileCapabilities MaximumNumberOfProfiles=\"3\"></trt:ProfileCapabilities><trt:StreamingCapabilities RTPMulticast=\"true\" RTP_TCP=\"true\" RTP_RTSP_TCP=\"true\" NonAggregateControl=\"false\"></trt:StreamingCapabilities></trt:Capabilities></tds:Capabilities>");
		strcat (soap_str, tmpstr);
	}
	memset (tmpstr, 0, HTTP_TEMP_STR_LEN);
	sprintf (tmpstr,
			"<tds:Version><tt:Major>2</tt:Major><tt:Minor>1</tt:Minor></tds:Version></tds:Service>");
	strcat (soap_str, tmpstr);

	//-------------------------------
	memset (tmpstr, 0, HTTP_TEMP_STR_LEN);
	sprintf (tmpstr,
			"<tds:Service><tds:Namespace>http://www.onvif.org/ver10/events/wsdl</tds:Namespace><tds:XAddr>http://%s/onvif/Events</tds:XAddr>", g_dvs.config.network.ip);
	strcat (soap_str, tmpstr);

	if (include_capbility == 1)
	{
		memset (tmpstr, 0, HTTP_TEMP_STR_LEN);
		sprintf (tmpstr,
				"<tds:Capabilities><tev:Capabilities WSSubscriptionPolicySupport=\"true\" WSPullPointSupport=\"true\" WSPausableSubscriptionManagerInterfaceSupport=\"false\"></tev:Capabilities></tds:Capabilities>");
		strcat (soap_str, tmpstr);
	}
	memset (tmpstr, 0, HTTP_TEMP_STR_LEN);
	sprintf (tmpstr,
			"<tds:Version><tt:Major>2</tt:Major><tt:Minor>1</tt:Minor></tds:Version></tds:Service>");
	strcat (soap_str, tmpstr);

	//---------------------------
	memset (tmpstr, 0, HTTP_TEMP_STR_LEN);
	sprintf (tmpstr,
			"<tds:Service><tds:Namespace>http://www.onvif.org/ver20/imaging/wsdl</tds:Namespace><tds:XAddr>http://%s/onvif/Imaging</tds:XAddr>", g_dvs.config.network.ip);
	strcat (soap_str, tmpstr);

	if (include_capbility == 1)
	{
		memset (tmpstr, 0, HTTP_TEMP_STR_LEN);
		sprintf (tmpstr,
				"<tds:Capabilities><timg:Capabilities/></tds:Capabilities>");
		strcat (soap_str, tmpstr);
	}
	memset (tmpstr, 0, HTTP_TEMP_STR_LEN);
	sprintf (tmpstr,
			"<tds:Version><tt:Major>2</tt:Major><tt:Minor>1</tt:Minor></tds:Version></tds:Service>");
	strcat (soap_str, tmpstr);

	//---------------------------
	memset (tmpstr, 0, HTTP_TEMP_STR_LEN);
	sprintf (tmpstr,
			"<tds:Service><tds:Namespace>http://www.onvif.org/ver10/deviceIO/wsdl</tds:Namespace><tds:XAddr>http://%s/onvif/DeviceIO</tds:XAddr>", g_dvs.config.network.ip);
	strcat (soap_str, tmpstr);

	if (include_capbility == 1)
	{
		memset (tmpstr, 0, HTTP_TEMP_STR_LEN);
		sprintf (tmpstr,
				"<tds:Capabilities><tmd:Capabilities VideoSources=\"1\" VideoOutputs=\"0\" AudioSources=\"0\" AudioOutputs=\"0\" RelayOutputs=\"0\"></tmd:Capabilities></tds:Capabilities>");
		strcat (soap_str, tmpstr);
	}
	memset (tmpstr, 0, HTTP_TEMP_STR_LEN);
	sprintf (tmpstr,
			"<tds:Version><tt:Major>2</tt:Major><tt:Minor>1</tt:Minor></tds:Version></tds:Service>");
	strcat (soap_str, tmpstr);

	//------------------------
	memset (tmpstr, 0, HTTP_TEMP_STR_LEN);
	sprintf (tmpstr,
			"<tds:Service><tds:Namespace>http://www.onvif.org/ver20/analytics/wsdl</tds:Namespace><tds:XAddr>http://%s/onvif/Analytics</tds:XAddr>", g_dvs.config.network.ip);
	strcat (soap_str, tmpstr);

	if (include_capbility == 1)
	{
		memset (tmpstr, 0, HTTP_TEMP_STR_LEN);
		sprintf (tmpstr,
				"<tds:Capabilities><tan:Capabilities/></tds:Capabilities>");
		strcat (soap_str, tmpstr);
	}
	memset (tmpstr, 0, HTTP_TEMP_STR_LEN);
	sprintf (tmpstr,
			"<tds:Version><tt:Major>2</tt:Major><tt:Minor>1</tt:Minor></tds:Version></tds:Service>");
	strcat (soap_str, tmpstr);

	strcat (soap_str,
			"</tds:GetServicesResponse></env:Body></env:Envelope>\r\n");

	memset (http_str, 0, HTTP_TEMP_STR_LEN);
	sprintf (http_str, http_head_str, strlen (soap_str));

	client_send (client, http_str, strlen (http_str));
	client_send (client, soap_str, strlen (soap_str));

}

static void DoGetVideoSources (char *cmdstr, char *soapstr, dvs_t * dvs, client_t * client)
{
	char tmpstr[HTTP_TEMP_STR_LEN];
	char http_str[HTTP_TEMP_STR_LEN];
	char soap_str[MAX_SOAP_SIZE];
	printf("DoGetVideoSources_1\r\n");
	memset (soap_str, 0, MAX_SOAP_SIZE);
	sprintf (soap_str, "%s%s%s",
			xml_version,
			soap_env_head, "<env:Body><trt:GetVideoSourcesResponse>");

	memset (tmpstr, 0, HTTP_TEMP_STR_LEN);
#if 0
	sprintf (tmpstr,
			"<trt:VideoSources token=\"VideoSource_1\"><tt:Framerate>25.000000</tt:Framerate><tt:Resolution><tt:Width>%d</tt:Width><tt:Height>%d</tt:Height></tt:Resolution><tt:Imaging><tt:BacklightCompensation><tt:Mode>OFF</tt:Mode><tt:Level>0</tt:Level></tt:BacklightCompensation><tt:Brightness>50</tt:Brightness><tt:ColorSaturation>50</tt:ColorSaturation><tt:Contrast>50</tt:Contrast><tt:Exposure><tt:Mode>AUTO</tt:Mode><tt:Priority>LowNoise</tt:Priority><tt:Window bottom=\"0\" top=\"0\" right=\"0\" left=\"0\"/><tt:MinExposureTime>10</tt:MinExposureTime><tt:MaxExposureTime>320000</tt:MaxExposureTime><tt:MinGain>0</tt:MinGain><tt:MaxGain>100</tt:MaxGain><tt:MinIris>0</tt:MinIris><tt:MaxIris>0</tt:MaxIris><tt:ExposureTime>40000</tt:ExposureTime><tt:Gain>100</tt:Gain><tt:Iris>1</tt:Iris></tt:Exposure><tt:IrCutFilter>AUTO</tt:IrCutFilter><tt:Sharpness>50</tt:Sharpness><tt:WideDynamicRange><tt:Mode>OFF</tt:Mode><tt:Level>3</tt:Level></tt:WideDynamicRange><tt:WhiteBalance><tt:Mode>AUTO</tt:Mode><tt:CrGain>0</tt:CrGain><tt:CbGain>0</tt:CbGain></tt:WhiteBalance></tt:Imaging></trt:VideoSources>",
			dvs->video_width, dvs->video_height);
#endif
	sprintf (tmpstr,
			"<trt:VideoSources token=\"VideoSource_1\"><tt:Framerate>25.000000</tt:Framerate><tt:Resolution><tt:Width>%d</tt:Width><tt:Height>%d</tt:Height></tt:Resolution><tt:Imaging><tt:BacklightCompensation><tt:Mode>OFF</tt:Mode><tt:Level>0</tt:Level></tt:BacklightCompensation><tt:Brightness>50</tt:Brightness><tt:ColorSaturation>50</tt:ColorSaturation><tt:Contrast>50</tt:Contrast></tt:Imaging></trt:VideoSources>", dvs->video_width, dvs->video_height);
	strcat (soap_str, tmpstr);

	strcat (soap_str,
			"</trt:GetVideoSourcesResponse></env:Body></env:Envelope>\r\n");

	memset (http_str, 0, HTTP_TEMP_STR_LEN);
	sprintf (http_str, http_head_str, strlen (soap_str));

	client_send (client, http_str, strlen (http_str));
	client_send (client, soap_str, strlen (soap_str));
}

static void DowsdlGetVideoSources (char *cmdstr, char *soapstr, dvs_t * dvs, client_t * client)
{
	DoGetVideoSources (cmdstr, soapstr, dvs, client);
}

static void DoGetProfiles (char *cmdstr, char *soapstr, dvs_t * dvs, client_t * client)
{
	char tmpstr[HTTP_TEMP_STR_LEN];
	char http_str[HTTP_TEMP_STR_LEN];
	char soap_str[MAX_SOAP_SIZE];
	config_videoprofile_t *videoprofile = &(dvs->chunnel[0].camera.videoprofile);
	config_videoprofile_t *videoprofile_small = &(dvs->chunnel[0].camera.videoprofile_small);

	memset (soap_str, 0, MAX_SOAP_SIZE);
	sprintf (soap_str, "%s%s%s", xml_version, soap_env_head, "<env:Body><trt:GetProfilesResponse>");

	memset (tmpstr, 0, HTTP_TEMP_STR_LEN);
	sprintf (tmpstr, "<trt:Profiles token=\"Profile_1\" fixed=\"true\"><tt:Name>mainStream</tt:Name>");
	strcat (soap_str, tmpstr);

	memset (tmpstr, 0, HTTP_TEMP_STR_LEN);
	sprintf (tmpstr,
			"<tt:VideoSourceConfiguration token=\"VideoSourceToken\"><tt:Name>VideoSourceConfig</tt:Name><tt:UseCount>2</tt:UseCount><tt:SourceToken>VideoSource_1</tt:SourceToken><tt:Bounds x=\"0\" y=\"0\" width=\"%d\" height=\"%d\"></tt:Bounds></tt:VideoSourceConfiguration>", dvs->video_width, dvs->video_height);
	strcat (soap_str, tmpstr);

	memset (tmpstr, 0, HTTP_TEMP_STR_LEN);
	sprintf (tmpstr,
			"<tt:VideoEncoderConfiguration token=\"VideoEncoderToken_1\"><tt:Name>VideoEncoder_1</tt:Name><tt:UseCount>1</tt:UseCount><tt:Encoding>H264</tt:Encoding><tt:Resolution><tt:Width>%d</tt:Width><tt:Height>%d</tt:Height></tt:Resolution><tt:Quality>3.000000</tt:Quality><tt:RateControl><tt:FrameRateLimit>%d</tt:FrameRateLimit><tt:EncodingInterval>1</tt:EncodingInterval><tt:BitrateLimit>%d</tt:BitrateLimit></tt:RateControl><tt:H264><tt:GovLength>30</tt:GovLength><tt:H264Profile>Baseline</tt:H264Profile></tt:H264><tt:Multicast><tt:Address><tt:Type>IPv4</tt:Type><tt:IPv4Address >239.168.40.236</tt:IPv4Address ></tt:Address><tt:Port>0</tt:Port><tt:TTL>1</tt:TTL><tt:AutoStart>false</tt:AutoStart></tt:Multicast><tt:SessionTimeout>PT5S</tt:SessionTimeout></tt:VideoEncoderConfiguration>",
			dvs->video_width, dvs->video_height, videoprofile->framerate, videoprofile->bitrate);
	strcat (soap_str, tmpstr);

	memset (tmpstr, 0, HTTP_TEMP_STR_LEN);
	sprintf (tmpstr,
			"<tt:VideoAnalyticsConfiguration token=\"VideoAnalyticsToken\"><tt:Name>VideoAnalyticsName</tt:Name><tt:UseCount>2</tt:UseCount><tt:AnalyticsEngineConfiguration><tt:AnalyticsModule Name=\"MyCellMotionModule\" Type=\"tt:CellMotionEngine\"><tt:Parameters><tt:SimpleItem Name=\"Sensitivity\" Value=\"0\"/><tt:ElementItem Name=\"Layout\"><tt:CellLayout Columns=\"22\" Rows=\"15\"><tt:Transformation><tt:Translate x=\"-1.000000\" y=\"-1.000000\"/><tt:Scale x=\"0.001563\" y=\"0.002083\"/></tt:Transformation></tt:CellLayout></tt:ElementItem></tt:Parameters></tt:AnalyticsModule></tt:AnalyticsEngineConfiguration><tt:RuleEngineConfiguration><tt:Rule Name=\"MyMotionDetectorRule\" Type=\"tt:CellMotionDetector\"><tt:Parameters><tt:SimpleItem Name=\"MinCount\" Value=\"5\"/><tt:SimpleItem Name=\"AlarmOnDelay\" Value=\"100\"/><tt:SimpleItem Name=\"AlarmOffDelay\" Value=\"100\"/><tt:SimpleItem Name=\"ActiveCells\" Value=\"1wA=\"/></tt:Parameters></tt:Rule></tt:RuleEngineConfiguration></tt:VideoAnalyticsConfiguration>");
	strcat (soap_str, tmpstr);

	memset (tmpstr, 0, HTTP_TEMP_STR_LEN);
	sprintf (tmpstr,
			"<tt:MetadataConfiguration token=\"MetaDataToken\"><tt:Name>metaData</tt:Name><tt:UseCount>2</tt:UseCount><tt:Analytics>false</tt:Analytics><tt:Multicast><tt:Address><tt:Type>IPv4</tt:Type><tt:IPv4Address>239.168.40.236</tt:IPv4Address></tt:Address><tt:Port>0</tt:Port><tt:TTL>1</tt:TTL><tt:AutoStart>false</tt:AutoStart></tt:Multicast><tt:SessionTimeout>PT0S</tt:SessionTimeout></tt:MetadataConfiguration>");
	strcat (soap_str, tmpstr);

	memset (tmpstr, 0, HTTP_TEMP_STR_LEN);
	sprintf (tmpstr, "</trt:Profiles>");
	strcat (soap_str, tmpstr);

	memset (tmpstr, 0, HTTP_TEMP_STR_LEN);
	sprintf (tmpstr,
			"<trt:Profiles token=\"Profile_2\" fixed=\"true\"><tt:Name>subStream</tt:Name>");
	strcat (soap_str, tmpstr);

	memset (tmpstr, 0, HTTP_TEMP_STR_LEN);
	sprintf (tmpstr,
			"<tt:VideoSourceConfiguration token=\"VideoSourceToken\"><tt:Name>VideoSourceConfig</tt:Name><tt:UseCount>2</tt:UseCount><tt:SourceToken>VideoSource_1</tt:SourceToken><tt:Bounds x=\"0\" y=\"0\" width=\"%d\" height=\"%d\"></tt:Bounds></tt:VideoSourceConfiguration>", dvs->video_width, dvs->video_height);
	strcat (soap_str, tmpstr);

	memset (tmpstr, 0, HTTP_TEMP_STR_LEN);
	sprintf (tmpstr,
			"<tt:VideoEncoderConfiguration token=\"VideoEncoderToken_2\"><tt:Name>VideoEncoder_2</tt:Name><tt:UseCount>1</tt:UseCount><tt:Encoding>H264</tt:Encoding><tt:Resolution><tt:Width>%d</tt:Width><tt:Height>%d</tt:Height></tt:Resolution><tt:Quality>3.000000</tt:Quality><tt:RateControl><tt:FrameRateLimit>%d</tt:FrameRateLimit><tt:EncodingInterval>1</tt:EncodingInterval><tt:BitrateLimit>%d</tt:BitrateLimit></tt:RateControl><tt:H264><tt:GovLength>30</tt:GovLength><tt:H264Profile>Baseline</tt:H264Profile></tt:H264><tt:Multicast><tt:Address><tt:Type>IPv4</tt:Type><tt:IPv4Address >239.168.40.236</tt:IPv4Address ></tt:Address><tt:Port>0</tt:Port><tt:TTL>1</tt:TTL><tt:AutoStart>false</tt:AutoStart></tt:Multicast><tt:SessionTimeout>PT5S</tt:SessionTimeout></tt:VideoEncoderConfiguration>",
			dvs->video_width_small, dvs->video_height_small,
			videoprofile_small->framerate, videoprofile_small->bitrate);
	strcat (soap_str, tmpstr);

	memset (tmpstr, 0, HTTP_TEMP_STR_LEN);
	sprintf (tmpstr,
			"<tt:VideoAnalyticsConfiguration token=\"VideoAnalyticsToken\"><tt:Name>VideoAnalyticsName</tt:Name><tt:UseCount>2</tt:UseCount><tt:AnalyticsEngineConfiguration><tt:AnalyticsModule Name=\"MyCellMotionModule\" Type=\"tt:CellMotionEngine\"><tt:Parameters><tt:SimpleItem Name=\"Sensitivity\" Value=\"0\"/><tt:ElementItem Name=\"Layout\"><tt:CellLayout Columns=\"22\" Rows=\"15\"><tt:Transformation><tt:Translate x=\"-1.000000\" y=\"-1.000000\"/><tt:Scale x=\"0.001563\" y=\"0.002083\"/></tt:Transformation></tt:CellLayout></tt:ElementItem></tt:Parameters></tt:AnalyticsModule></tt:AnalyticsEngineConfiguration><tt:RuleEngineConfiguration><tt:Rule Name=\"MyMotionDetectorRule\" Type=\"tt:CellMotionDetector\"><tt:Parameters><tt:SimpleItem Name=\"MinCount\" Value=\"5\"/><tt:SimpleItem Name=\"AlarmOnDelay\" Value=\"100\"/><tt:SimpleItem Name=\"AlarmOffDelay\" Value=\"100\"/><tt:SimpleItem Name=\"ActiveCells\" Value=\"1wA=\"/></tt:Parameters></tt:Rule></tt:RuleEngineConfiguration></tt:VideoAnalyticsConfiguration>");
	strcat (soap_str, tmpstr);

	memset (tmpstr, 0, HTTP_TEMP_STR_LEN);
	sprintf (tmpstr,
			"<tt:MetadataConfiguration token=\"MetaDataToken\"><tt:Name>metaData</tt:Name><tt:UseCount>2</tt:UseCount><tt:Analytics>false</tt:Analytics><tt:Multicast><tt:Address><tt:Type>IPv4</tt:Type><tt:IPv4Address>239.168.40.236</tt:IPv4Address></tt:Address><tt:Port>0</tt:Port><tt:TTL>1</tt:TTL><tt:AutoStart>false</tt:AutoStart></tt:Multicast><tt:SessionTimeout>PT0S</tt:SessionTimeout></tt:MetadataConfiguration>");
	strcat (soap_str, tmpstr);

	memset (tmpstr, 0, HTTP_TEMP_STR_LEN);
	sprintf (tmpstr, "</trt:Profiles>");
	strcat (soap_str, tmpstr);
	strcat (soap_str, "</trt:GetProfilesResponse></env:Body></env:Envelope>\r\n");

	memset (http_str, 0, HTTP_TEMP_STR_LEN);
	sprintf (http_str, http_head_str, strlen (soap_str));

	client_send (client, http_str, strlen (http_str));
	client_send (client, soap_str, strlen (soap_str));
}

static void DoGetSnapshotUri (char *cmdstr, char *soapstr, dvs_t * dvs, client_t * client)
{
	//char tmpstr[HTTP_TEMP_STR_LEN];
	char http_str[HTTP_TEMP_STR_LEN];
	char soap_str[MAX_SOAP_SIZE];

	memset (soap_str, 0, MAX_SOAP_SIZE);
	sprintf (soap_str, "%s%s%s",
			xml_version,
			soap_env_head,
			"<env:Body><trt:GetSnapshotUriResponse></trt:GetSnapshotUriResponse></env:Body></env:Envelope>\r\n");

	memset (http_str, 0, HTTP_TEMP_STR_LEN);
	sprintf (http_str, http_head_str, strlen (soap_str));

	client_send (client, http_str, strlen (http_str));
	client_send (client, soap_str, strlen (soap_str));
}

static void DoGetProfile (char *cmdstr, char *soapstr, dvs_t * dvs, client_t * client)
{
	char tmpstr[HTTP_TEMP_STR_LEN];
	char http_str[HTTP_TEMP_STR_LEN];
	char soap_str[MAX_SOAP_SIZE];
	char ProfileToken[HTTP_TEMP_STR_LEN];
	int profile_token = 0;
	config_videoprofile_t *videoprofile = &(dvs->chunnel[0].camera.videoprofile);
	config_videoprofile_t *videoprofile_small = &(dvs->chunnel[0].camera.videoprofile_small);

	memset (ProfileToken, 0, HTTP_TEMP_STR_LEN);
	int res = soap_get_value (soapstr, ProfileToken, "ProfileToken");
	if (findStrFromBuf (ProfileToken, strlen (ProfileToken), "Profile_1", strlen ("Profile_1")) >= 0)
	{
		profile_token = 1;
	}
	else if (findStrFromBuf (ProfileToken, strlen (ProfileToken), "Profile_2", strlen ("Profile_2")) >= 0)
	{
		profile_token = 2;
	}

	memset (soap_str, 0, MAX_SOAP_SIZE);
	sprintf (soap_str, "%s%s%s", xml_version, soap_env_head, "<env:Body><trt:GetProfileResponse>");
	if (profile_token == 1)
	{
		memset (tmpstr, 0, HTTP_TEMP_STR_LEN);
		sprintf (tmpstr, "<trt:Profile token=\"Profile_1\" fixed=\"true\"><tt:Name>mainStream</tt:Name>");
		strcat (soap_str, tmpstr);

		memset (tmpstr, 0, HTTP_TEMP_STR_LEN);
		sprintf (tmpstr,
				"<tt:VideoSourceConfiguration token=\"VideoSourceToken\"><tt:Name>VideoSourceConfig</tt:Name><tt:UseCount>2</tt:UseCount><tt:SourceToken>VideoSource_1</tt:SourceToken><tt:Bounds x=\"0\" y=\"0\" width=\"%d\" height=\"%d\"></tt:Bounds></tt:VideoSourceConfiguration>", dvs->video_width, dvs->video_height);
		strcat (soap_str, tmpstr);

		memset (tmpstr, 0, HTTP_TEMP_STR_LEN);
		sprintf (tmpstr,
				"<tt:VideoEncoderConfiguration token=\"VideoEncoderToken_1\"><tt:Name>VideoEncoder_1</tt:Name><tt:UseCount>1</tt:UseCount><tt:Encoding>H264</tt:Encoding><tt:Resolution><tt:Width>%d</tt:Width><tt:Height>%d</tt:Height></tt:Resolution><tt:Quality>3.000000</tt:Quality><tt:RateControl><tt:FrameRateLimit>%d</tt:FrameRateLimit><tt:EncodingInterval>1</tt:EncodingInterval><tt:BitrateLimit>%d</tt:BitrateLimit></tt:RateControl><tt:H264><tt:GovLength>30</tt:GovLength><tt:H264Profile>Baseline</tt:H264Profile></tt:H264><tt:Multicast><tt:Address><tt:Type>IPv4</tt:Type><tt:IPv4Address >239.168.40.236</tt:IPv4Address ></tt:Address><tt:Port>0</tt:Port><tt:TTL>1</tt:TTL><tt:AutoStart>false</tt:AutoStart></tt:Multicast><tt:SessionTimeout>PT5S</tt:SessionTimeout></tt:VideoEncoderConfiguration>",
				dvs->video_width, dvs->video_height, videoprofile->framerate, videoprofile->bitrate);
		strcat (soap_str, tmpstr);

		memset (tmpstr, 0, HTTP_TEMP_STR_LEN);
		sprintf (tmpstr,
				"<tt:VideoAnalyticsConfiguration token=\"VideoAnalyticsToken\"><tt:Name>VideoAnalyticsName</tt:Name><tt:UseCount>2</tt:UseCount><tt:AnalyticsEngineConfiguration><tt:AnalyticsModule Name=\"MyCellMotionModule\" Type=\"tt:CellMotionEngine\"><tt:Parameters><tt:SimpleItem Name=\"Sensitivity\" Value=\"0\"/><tt:ElementItem Name=\"Layout\"><tt:CellLayout Columns=\"22\" Rows=\"15\"><tt:Transformation><tt:Translate x=\"-1.000000\" y=\"-1.000000\"/><tt:Scale x=\"0.001563\" y=\"0.002083\"/></tt:Transformation></tt:CellLayout></tt:ElementItem></tt:Parameters></tt:AnalyticsModule></tt:AnalyticsEngineConfiguration><tt:RuleEngineConfiguration><tt:Rule Name=\"MyMotionDetectorRule\" Type=\"tt:CellMotionDetector\"><tt:Parameters><tt:SimpleItem Name=\"MinCount\" Value=\"5\"/><tt:SimpleItem Name=\"AlarmOnDelay\" Value=\"100\"/><tt:SimpleItem Name=\"AlarmOffDelay\" Value=\"100\"/><tt:SimpleItem Name=\"ActiveCells\" Value=\"1wA=\"/></tt:Parameters></tt:Rule></tt:RuleEngineConfiguration></tt:VideoAnalyticsConfiguration>");
		strcat (soap_str, tmpstr);

		memset (tmpstr, 0, HTTP_TEMP_STR_LEN);
		sprintf (tmpstr,
				"<tt:MetadataConfiguration token=\"MetaDataToken\"><tt:Name>metaData</tt:Name><tt:UseCount>2</tt:UseCount><tt:Analytics>false</tt:Analytics><tt:Multicast><tt:Address><tt:Type>IPv4</tt:Type><tt:IPv4Address>239.168.40.236</tt:IPv4Address></tt:Address><tt:Port>0</tt:Port><tt:TTL>1</tt:TTL><tt:AutoStart>false</tt:AutoStart></tt:Multicast><tt:SessionTimeout>PT0S</tt:SessionTimeout></tt:MetadataConfiguration>");
		strcat (soap_str, tmpstr);

		memset (tmpstr, 0, HTTP_TEMP_STR_LEN);
		sprintf (tmpstr, "</trt:Profile>");
		strcat (soap_str, tmpstr);
	}
	else
	{
		memset (tmpstr, 0, HTTP_TEMP_STR_LEN);
		sprintf (tmpstr, "<trt:Profile token=\"Profile_2\" fixed=\"true\"><tt:Name>subStream</tt:Name>");
		strcat (soap_str, tmpstr);

		memset (tmpstr, 0, HTTP_TEMP_STR_LEN);
		sprintf (tmpstr,
				"<tt:VideoSourceConfiguration token=\"VideoSourceToken\"><tt:Name>VideoSourceConfig</tt:Name><tt:UseCount>2</tt:UseCount><tt:SourceToken>VideoSource_1</tt:SourceToken><tt:Bounds x=\"0\" y=\"0\" width=\"%d\" height=\"%d\"></tt:Bounds></tt:VideoSourceConfiguration>",
				dvs->video_width, dvs->video_height);
		strcat (soap_str, tmpstr);

		memset (tmpstr, 0, HTTP_TEMP_STR_LEN);
		sprintf (tmpstr,
				"<tt:VideoEncoderConfiguration token=\"VideoEncoderToken_2\"><tt:Name>VideoEncoder_2</tt:Name><tt:UseCount>1</tt:UseCount><tt:Encoding>H264</tt:Encoding><tt:Resolution><tt:Width>%d</tt:Width><tt:Height>%d</tt:Height></tt:Resolution><tt:Quality>3.000000</tt:Quality><tt:RateControl><tt:FrameRateLimit>%d</tt:FrameRateLimit><tt:EncodingInterval>1</tt:EncodingInterval><tt:BitrateLimit>%d</tt:BitrateLimit></tt:RateControl><tt:H264><tt:GovLength>30</tt:GovLength><tt:H264Profile>Baseline</tt:H264Profile></tt:H264><tt:Multicast><tt:Address><tt:Type>IPv4</tt:Type><tt:IPv4Address >239.168.40.236</tt:IPv4Address ></tt:Address><tt:Port>0</tt:Port><tt:TTL>1</tt:TTL><tt:AutoStart>false</tt:AutoStart></tt:Multicast><tt:SessionTimeout>PT5S</tt:SessionTimeout></tt:VideoEncoderConfiguration>",
				dvs->video_width_small, dvs->video_height_small,
				videoprofile_small->framerate, videoprofile_small->bitrate);
		strcat (soap_str, tmpstr);

		memset (tmpstr, 0, HTTP_TEMP_STR_LEN);
		sprintf (tmpstr,
				"<tt:VideoAnalyticsConfiguration token=\"VideoAnalyticsToken\"><tt:Name>VideoAnalyticsName</tt:Name><tt:UseCount>2</tt:UseCount><tt:AnalyticsEngineConfiguration><tt:AnalyticsModule Name=\"MyCellMotionModule\" Type=\"tt:CellMotionEngine\"><tt:Parameters><tt:SimpleItem Name=\"Sensitivity\" Value=\"0\"/><tt:ElementItem Name=\"Layout\"><tt:CellLayout Columns=\"22\" Rows=\"15\"><tt:Transformation><tt:Translate x=\"-1.000000\" y=\"-1.000000\"/><tt:Scale x=\"0.001563\" y=\"0.002083\"/></tt:Transformation></tt:CellLayout></tt:ElementItem></tt:Parameters></tt:AnalyticsModule></tt:AnalyticsEngineConfiguration><tt:RuleEngineConfiguration><tt:Rule Name=\"MyMotionDetectorRule\" Type=\"tt:CellMotionDetector\"><tt:Parameters><tt:SimpleItem Name=\"MinCount\" Value=\"5\"/><tt:SimpleItem Name=\"AlarmOnDelay\" Value=\"100\"/><tt:SimpleItem Name=\"AlarmOffDelay\" Value=\"100\"/><tt:SimpleItem Name=\"ActiveCells\" Value=\"1wA=\"/></tt:Parameters></tt:Rule></tt:RuleEngineConfiguration></tt:VideoAnalyticsConfiguration>");
		strcat (soap_str, tmpstr);

		memset (tmpstr, 0, HTTP_TEMP_STR_LEN);
		sprintf (tmpstr,
				"<tt:MetadataConfiguration token=\"MetaDataToken\"><tt:Name>metaData</tt:Name><tt:UseCount>2</tt:UseCount><tt:Analytics>false</tt:Analytics><tt:Multicast><tt:Address><tt:Type>IPv4</tt:Type><tt:IPv4Address>239.168.40.236</tt:IPv4Address></tt:Address><tt:Port>0</tt:Port><tt:TTL>1</tt:TTL><tt:AutoStart>false</tt:AutoStart></tt:Multicast><tt:SessionTimeout>PT0S</tt:SessionTimeout></tt:MetadataConfiguration>");
		strcat (soap_str, tmpstr);

		memset (tmpstr, 0, HTTP_TEMP_STR_LEN);
		sprintf (tmpstr, "</trt:Profile>");
		strcat (soap_str, tmpstr);
	}
	strcat (soap_str,
			"</trt:GetProfileResponse></env:Body></env:Envelope>\r\n");
	memset (http_str, 0, HTTP_TEMP_STR_LEN);
	sprintf (http_str, http_head_str, strlen (soap_str));

	client_send (client, http_str, strlen (http_str));
	client_send (client, soap_str, strlen (soap_str));
}

static void DoGetStreamUri (char *cmdstr, char *soapstr, dvs_t * dvs, client_t * client)
{
	char tmpstr[HTTP_TEMP_STR_LEN];
	char http_str[HTTP_TEMP_STR_LEN];
	char soap_str[MAX_SOAP_SIZE];
	char ProfileToken[HTTP_TEMP_STR_LEN];
	int profile_token = 0;

	memset (ProfileToken, 0, HTTP_TEMP_STR_LEN);
	int res = soap_get_value (soapstr, ProfileToken, "ProfileToken");

	if (findStrFromBuf (ProfileToken, strlen (ProfileToken), "Profile_1", strlen ("Profile_1")) >= 0)
	{
		profile_token = 1;
	}
	else if (findStrFromBuf (ProfileToken, strlen (ProfileToken), "Profile_2", strlen ("Profile_2")) >= 0)
	{
		profile_token = 2;
	}
	memset (soap_str, 0, MAX_SOAP_SIZE);
	sprintf (soap_str, "%s%s%s", xml_version, soap_env_head, "<env:Body><trt:GetStreamUriResponse>");
	memset (tmpstr, 0, HTTP_TEMP_STR_LEN);

	if (profile_token == 1)
	{
		sprintf (tmpstr,
				"<trt:MediaUri><tt:Uri>rtsp://%s/video0.sdp</tt:Uri><tt:InvalidAfterConnect>false</tt:InvalidAfterConnect><tt:InvalidAfterReboot>false</tt:InvalidAfterReboot><tt:Timeout>PT0S</tt:Timeout></trt:MediaUri>",
				g_dvs.config.network.ip);
	}
	else
	{
		sprintf (tmpstr,
				"<trt:MediaUri><tt:Uri>rtsp://%s/video1.sdp</tt:Uri><tt:InvalidAfterConnect>false</tt:InvalidAfterConnect><tt:InvalidAfterReboot>false</tt:InvalidAfterReboot><tt:Timeout>PT0S</tt:Timeout></trt:MediaUri>",
				g_dvs.config.network.ip);
	}
	strcat (soap_str, tmpstr);

	strcat (soap_str,
			"</trt:GetStreamUriResponse></env:Body></env:Envelope>\r\n");
	memset (http_str, 0, HTTP_TEMP_STR_LEN);
	sprintf (http_str, http_head_str, strlen (soap_str));

	client_send (client, http_str, strlen (http_str));
	client_send (client, soap_str, strlen (soap_str));
}

static void DoGetVideoSourceConfiguration (char *cmdstr, char *soapstr, dvs_t * dvs, client_t * client)
{
	char tmpstr[HTTP_TEMP_STR_LEN];
	char http_str[HTTP_TEMP_STR_LEN];
	char soap_str[MAX_SOAP_SIZE];

	memset (soap_str, 0, MAX_SOAP_SIZE);
	sprintf (soap_str, "%s%s%s",
			xml_version,
			soap_env_head,
			"<env:Body><trt:GetVideoSourceConfigurationResponse>");

	memset (tmpstr, 0, HTTP_TEMP_STR_LEN);
	sprintf (tmpstr,
			"<trt:Configuration token=\"VideoSourceToken\"><tt:Name>VideoSourceConfig</tt:Name><tt:UseCount>2</tt:UseCount><tt:SourceToken>VideoSource_1</tt:SourceToken><tt:Bounds x=\"0\" y=\"0\" width=\"%d\" height=\"%d\"></tt:Bounds></trt:Configuration>",
			dvs->video_width, dvs->video_height);
	strcat (soap_str, tmpstr);

	strcat (soap_str,
			"</trt:GetVideoSourceConfigurationResponse></env:Body></env:Envelope>\r\n");
	memset (http_str, 0, HTTP_TEMP_STR_LEN);
	sprintf (http_str, http_head_str, strlen (soap_str));

	client_send (client, http_str, strlen (http_str));
	client_send (client, soap_str, strlen (soap_str));
}

static void DoGetVideoSourceConfigurations (char *cmdstr, char *soapstr, dvs_t * dvs, client_t * client)
{
	char tmpstr[HTTP_TEMP_STR_LEN];
	char http_str[HTTP_TEMP_STR_LEN];
	char soap_str[MAX_SOAP_SIZE];

	memset (soap_str, 0, MAX_SOAP_SIZE);
	sprintf (soap_str, "%s%s%s", xml_version, soap_env_head,
			"<env:Body><trt:GetVideoSourceConfigurationsResponse>");

	memset (tmpstr, 0, HTTP_TEMP_STR_LEN);
	sprintf (tmpstr,
			"<trt:Configurations token=\"VideoSourceToken\"><tt:Name>VideoSourceConfig</tt:Name><tt:UseCount>2</tt:UseCount><tt:SourceToken>VideoSource_1</tt:SourceToken><tt:Bounds x=\"0\" y=\"0\" width=\"%d\" height=\"%d\"></tt:Bounds></trt:Configurations>",
			dvs->video_width, dvs->video_height);
	strcat (soap_str, tmpstr);

	strcat (soap_str,
			"</trt:GetVideoSourceConfigurationsResponse></env:Body></env:Envelope>\r\n");
	memset (http_str, 0, HTTP_TEMP_STR_LEN);
	sprintf (http_str, http_head_str, strlen (soap_str));

	client_send (client, http_str, strlen (http_str));
	client_send (client, soap_str, strlen (soap_str));
}

static void DoGetVideoSourceConfigurationOptions (char *cmdstr, char *soapstr, dvs_t * dvs, client_t * client)
{
	char tmpstr[HTTP_TEMP_STR_LEN];
	char http_str[HTTP_TEMP_STR_LEN];
	char soap_str[MAX_SOAP_SIZE];

	memset (soap_str, 0, MAX_SOAP_SIZE);
	sprintf (soap_str, "%s%s%s", xml_version, soap_env_head,
			"<env:Body><trt:GetVideoSourceConfigurationOptionsResponse>");

	memset (tmpstr, 0, HTTP_TEMP_STR_LEN);
	sprintf (tmpstr,
			"<trt:Options><tt:BoundsRange><tt:XRange><tt:Min>0</tt:Min><tt:Max>0</tt:Max></tt:XRange><tt:YRange><tt:Min>0</tt:Min><tt:Max>0</tt:Max></tt:YRange><tt:WidthRange><tt:Min>%d</tt:Min><tt:Max>%d</tt:Max></tt:WidthRange><tt:HeightRange><tt:Min>%d</tt:Min><tt:Max>%d</tt:Max></tt:HeightRange></tt:BoundsRange><tt:VideoSourceTokensAvailable >VideoSource_1</tt:VideoSourceTokensAvailable ></trt:Options>", dvs->video_width, dvs->video_width, dvs->video_height, dvs->video_height);
	strcat (soap_str, tmpstr);

	strcat (soap_str, "</trt:GetVideoSourceConfigurationOptionsResponse></env:Body></env:Envelope>\r\n");
	memset (http_str, 0, HTTP_TEMP_STR_LEN);
	sprintf (http_str, http_head_str, strlen (soap_str));

	client_send (client, http_str, strlen (http_str));
	client_send (client, soap_str, strlen (soap_str));
}

static void DoGetVideoEncoderConfigurations (char *cmdstr, char *soapstr, dvs_t * dvs, client_t * client)
{
	char tmpstr[HTTP_TEMP_STR_LEN];
	char http_str[HTTP_TEMP_STR_LEN];
	char soap_str[MAX_SOAP_SIZE];
	config_videoprofile_t *videoprofile = &(dvs->chunnel[0].camera.videoprofile);
	config_videoprofile_t *videoprofile_small = &(dvs->chunnel[0].camera.videoprofile_small);

	memset (soap_str, 0, MAX_SOAP_SIZE);
	sprintf (soap_str, "%s%s%s", xml_version, soap_env_head, "<env:Body><trt:GetVideoEncoderConfigurationsResponse>");

	memset (tmpstr, 0, HTTP_TEMP_STR_LEN);
	sprintf (tmpstr,
			"<trt:Configurations token=\"VideoEncoderToken_1\"><tt:Name>VideoEncoder_1</tt:Name><tt:UseCount>1</tt:UseCount><tt:Encoding>H264</tt:Encoding><tt:Resolution><tt:Width>%d</tt:Width><tt:Height>%d</tt:Height></tt:Resolution><tt:Quality>3.000000</tt:Quality><tt:RateControl><tt:FrameRateLimit>%d</tt:FrameRateLimit><tt:EncodingInterval>1</tt:EncodingInterval><tt:BitrateLimit>%d</tt:BitrateLimit></tt:RateControl><tt:H264><tt:GovLength>30</tt:GovLength><tt:H264Profile>Baseline</tt:H264Profile></tt:H264><tt:Multicast><tt:Address><tt:Type>IPv4</tt:Type><tt:IPv4Address >239.168.40.236</tt:IPv4Address ></tt:Address><tt:Port>0</tt:Port><tt:TTL>1</tt:TTL><tt:AutoStart>false</tt:AutoStart></tt:Multicast><tt:SessionTimeout>PT5S</tt:SessionTimeout></trt:Configurations>",
			dvs->video_width, dvs->video_height, videoprofile->framerate,
			videoprofile->bitrate);
	strcat (soap_str, tmpstr);

	memset (tmpstr, 0, HTTP_TEMP_STR_LEN);
	sprintf (tmpstr,
			"<trt:Configurations token=\"VideoEncoderToken_2\"><tt:Name>VideoEncoder_2</tt:Name><tt:UseCount>1</tt:UseCount><tt:Encoding>H264</tt:Encoding><tt:Resolution><tt:Width>%d</tt:Width><tt:Height>%d</tt:Height></tt:Resolution><tt:Quality>3.000000</tt:Quality><tt:RateControl><tt:FrameRateLimit>%d</tt:FrameRateLimit><tt:EncodingInterval>1</tt:EncodingInterval><tt:BitrateLimit>%d</tt:BitrateLimit></tt:RateControl><tt:H264><tt:GovLength>30</tt:GovLength><tt:H264Profile>Baseline</tt:H264Profile></tt:H264><tt:Multicast><tt:Address><tt:Type>IPv4</tt:Type><tt:IPv4Address >239.168.40.236</tt:IPv4Address ></tt:Address><tt:Port>0</tt:Port><tt:TTL>1</tt:TTL><tt:AutoStart>false</tt:AutoStart></tt:Multicast><tt:SessionTimeout>PT5S</tt:SessionTimeout></trt:Configurations>",
			dvs->video_width_small, dvs->video_height_small,
			videoprofile_small->framerate, videoprofile_small->bitrate);
	strcat (soap_str, tmpstr);

	strcat (soap_str,
			"</trt:GetVideoEncoderConfigurationsResponse></env:Body></env:Envelope>\r\n");
	memset (http_str, 0, HTTP_TEMP_STR_LEN);
	sprintf (http_str, http_head_str, strlen (soap_str));

	client_send (client, http_str, strlen (http_str));
	client_send (client, soap_str, strlen (soap_str));
}

static void DoGetVideoEncoderConfiguration (char *cmdstr, char *soapstr, dvs_t * dvs, client_t * client)
{
	char tmpstr[HTTP_TEMP_STR_LEN];
	char http_str[HTTP_TEMP_STR_LEN];
	char soap_str[MAX_SOAP_SIZE];
	//char ConfigToken[HTTP_TEMP_STR_LEN];
	int config_token = 0;
	config_videoprofile_t *videoprofile;

	if (findStrFromBuf (soapstr, strlen (soapstr), "VideoEncoderToken_1", strlen ("VideoEncoderToken_1")) >= 0)
	{
		config_token = 1;
	}
	else if (findStrFromBuf (soapstr, strlen (soapstr), "VideoEncoderToken_2", strlen ("VideoEncoderToken_2")) >= 0)
	{
		config_token = 2;
	}

	memset (soap_str, 0, MAX_SOAP_SIZE);
	sprintf (soap_str, "%s%s%s", xml_version, soap_env_head,
			"<env:Body><trt:GetVideoEncoderConfigurationResponse>");
	if (config_token == 1)
	{
		videoprofile = &(dvs->chunnel[0].camera.videoprofile);
		memset (tmpstr, 0, HTTP_TEMP_STR_LEN);
		sprintf (tmpstr,
				"<trt:Configuration token=\"VideoEncoderToken_1\"><tt:Name>VideoEncoder_1</tt:Name><tt:UseCount>1</tt:UseCount><tt:Encoding>H264</tt:Encoding><tt:Resolution><tt:Width>%d</tt:Width><tt:Height>%d</tt:Height></tt:Resolution><tt:Quality>3.000000</tt:Quality><tt:RateControl><tt:FrameRateLimit>%d</tt:FrameRateLimit><tt:EncodingInterval>1</tt:EncodingInterval><tt:BitrateLimit>%d</tt:BitrateLimit></tt:RateControl><tt:H264><tt:GovLength>30</tt:GovLength><tt:H264Profile>Baseline</tt:H264Profile></tt:H264><tt:Multicast><tt:Address><tt:Type>IPv4</tt:Type><tt:IPv4Address >239.168.40.236</tt:IPv4Address ></tt:Address><tt:Port>0</tt:Port><tt:TTL>1</tt:TTL><tt:AutoStart>false</tt:AutoStart></tt:Multicast><tt:SessionTimeout>PT5S</tt:SessionTimeout></trt:Configuration>",
				dvs->video_width, dvs->video_height, videoprofile->framerate, videoprofile->bitrate);
		strcat (soap_str, tmpstr);
	}
	else
	{
		videoprofile = &(dvs->chunnel[0].camera.videoprofile_small);
		memset (tmpstr, 0, HTTP_TEMP_STR_LEN);
		sprintf (tmpstr,
				"<trt:Configuration token=\"VideoEncoderToken_2\"><tt:Name>VideoEncoder_2</tt:Name><tt:UseCount>1</tt:UseCount><tt:Encoding>H264</tt:Encoding><tt:Resolution><tt:Width>%d</tt:Width><tt:Height>%d</tt:Height></tt:Resolution><tt:Quality>3.000000</tt:Quality><tt:RateControl><tt:FrameRateLimit>%d</tt:FrameRateLimit><tt:EncodingInterval>1</tt:EncodingInterval><tt:BitrateLimit>%d</tt:BitrateLimit></tt:RateControl><tt:H264><tt:GovLength>30</tt:GovLength><tt:H264Profile>Baseline</tt:H264Profile></tt:H264><tt:Multicast><tt:Address><tt:Type>IPv4</tt:Type><tt:IPv4Address>239.168.40.236</tt:IPv4Address></tt:Address><tt:Port>0</tt:Port><tt:TTL>1</tt:TTL><tt:AutoStart>false</tt:AutoStart></tt:Multicast><tt:SessionTimeout>PT5S</tt:SessionTimeout></trt:Configuration>",
				dvs->video_width_small, dvs->video_height_small,
				videoprofile->framerate, videoprofile->bitrate);
		strcat (soap_str, tmpstr);
	}

	strcat (soap_str,
			"</trt:GetVideoEncoderConfigurationResponse></env:Body></env:Envelope>\r\n");
	memset (http_str, 0, HTTP_TEMP_STR_LEN);
	sprintf (http_str, http_head_str, strlen (soap_str));

	client_send (client, http_str, strlen (http_str));
	client_send (client, soap_str, strlen (soap_str));
}


static void DoGetVideoEncoderConfigurationOptions (char *cmdstr, char *soapstr, dvs_t * dvs, client_t * client)
{
	char tmpstr[HTTP_TEMP_STR_LEN];
	char http_str[HTTP_TEMP_STR_LEN];
	char soap_str[MAX_SOAP_SIZE];
	//char ConfigToken[HTTP_TEMP_STR_LEN];
	int config_token = 0;

	if (findStrFromBuf (soapstr, strlen (soapstr), "VideoEncoderToken_1", strlen ("VideoEncoderToken_1")) >= 0)
	{
		config_token = 1;
	}
	else if (findStrFromBuf (soapstr, strlen (soapstr), "VideoEncoderToken_2", strlen ("VideoEncoderToken_2")) >= 0)
	{
		config_token = 2;
	}

	memset (soap_str, 0, MAX_SOAP_SIZE);
	sprintf (soap_str, "%s%s%s",
			xml_version,
			soap_env_head,
			"<env:Body><trt:GetVideoEncoderConfigurationOptionsResponse>");

	if (config_token == 1)
	{
		memset (tmpstr, 0, HTTP_TEMP_STR_LEN);
		sprintf (tmpstr,
				"<trt:Options><tt:QualityRange><tt:Min>0</tt:Min><tt:Max>5</tt:Max></tt:QualityRange>");
		strcat (soap_str, tmpstr);

		memset (tmpstr, 0, HTTP_TEMP_STR_LEN);
		switch (dvs->sensor)
		{
			case OV2715:
			case MT9P031:
				sprintf (tmpstr,
						"<tt:H264><tt:ResolutionsAvailable><tt:Width>1920</tt:Width><tt:Height>1080</tt:Height></tt:ResolutionsAvailable><tt:ResolutionsAvailable><tt:Width>1280</tt:Width><tt:Height>720</tt:Height></tt:ResolutionsAvailable><tt:GovLengthRange><tt:Min>30</tt:Min><tt:Max>30</tt:Max></tt:GovLengthRange><tt:FrameRateRange><tt:Min>1</tt:Min><tt:Max>30</tt:Max></tt:FrameRateRange><tt:EncodingIntervalRange><tt:Min>1</tt:Min><tt:Max>400</tt:Max></tt:EncodingIntervalRange><tt:H264ProfilesSupported>Baseline</tt:H264ProfilesSupported></tt:H264>");
				break;
			case MT9M034:
				sprintf (tmpstr,
						"<tt:H264><tt:ResolutionsAvailable><tt:Width>1280</tt:Width><tt:Height>960</tt:Height></tt:ResolutionsAvailable><tt:ResolutionsAvailable><tt:Width>1280</tt:Width><tt:Height>720</tt:Height></tt:ResolutionsAvailable><tt:GovLengthRange><tt:Min>30</tt:Min><tt:Max>30</tt:Max></tt:GovLengthRange><tt:FrameRateRange><tt:Min>1</tt:Min><tt:Max>30</tt:Max></tt:FrameRateRange><tt:EncodingIntervalRange><tt:Min>1</tt:Min><tt:Max>400</tt:Max></tt:EncodingIntervalRange><tt:H264ProfilesSupported>Baseline</tt:H264ProfilesSupported></tt:H264>");
				break;
			case TW9900:
				sprintf (tmpstr,
						"<tt:H264><tt:ResolutionsAvailable><tt:Width>704</tt:Width><tt:Height>576</tt:Height></tt:ResolutionsAvailable><tt:ResolutionsAvailable><tt:Width>352</tt:Width><tt:Height>288</tt:Height></tt:ResolutionsAvailable><tt:GovLengthRange><tt:Min>30</tt:Min><tt:Max>30</tt:Max></tt:GovLengthRange><tt:FrameRateRange><tt:Min>1</tt:Min><tt:Max>30</tt:Max></tt:FrameRateRange><tt:EncodingIntervalRange><tt:Min>1</tt:Min><tt:Max>400</tt:Max></tt:EncodingIntervalRange><tt:H264ProfilesSupported>Baseline</tt:H264ProfilesSupported></tt:H264>");
				break;
			default:
				sprintf (tmpstr,
						"<tt:H264><tt:ResolutionsAvailable><tt:Width>1280</tt:Width><tt:Height>960</tt:Height></tt:ResolutionsAvailable><tt:ResolutionsAvailable><tt:Width>1280</tt:Width><tt:Height>720</tt:Height></tt:ResolutionsAvailable><tt:GovLengthRange><tt:Min>30</tt:Min><tt:Max>30</tt:Max></tt:GovLengthRange><tt:FrameRateRange><tt:Min>1</tt:Min><tt:Max>30</tt:Max></tt:FrameRateRange><tt:EncodingIntervalRange><tt:Min>1</tt:Min><tt:Max>400</tt:Max></tt:EncodingIntervalRange><tt:H264ProfilesSupported>Baseline</tt:H264ProfilesSupported></tt:H264>");
				break;
		}

		strcat (soap_str, tmpstr);

		memset (tmpstr, 0, HTTP_TEMP_STR_LEN);
		switch (dvs->sensor)
		{
			case OV2715:
			case MT9P031:
				sprintf (tmpstr,
						"<tt:Extension><tt:H264><tt:ResolutionsAvailable><tt:Width>1920</tt:Width><tt:Height>1080</tt:Height></tt:ResolutionsAvailable><tt:ResolutionsAvailable><tt:Width>1280</tt:Width><tt:Height>720</tt:Height></tt:ResolutionsAvailable><tt:GovLengthRange><tt:Min>30</tt:Min><tt:Max>30</tt:Max></tt:GovLengthRange><tt:FrameRateRange><tt:Min>1</tt:Min><tt:Max>30</tt:Max></tt:FrameRateRange><tt:EncodingIntervalRange><tt:Min>1</tt:Min><tt:Max>400</tt:Max></tt:EncodingIntervalRange><tt:H264ProfilesSupported>Baseline</tt:H264ProfilesSupported><tt:BitrateRange><tt:Min>32</tt:Min><tt:Max>16384</tt:Max></tt:BitrateRange></tt:H264></tt:Extension></trt:Options>");
				break;
			case MT9M034:
				sprintf (tmpstr,
						"<tt:Extension><tt:H264><tt:ResolutionsAvailable><tt:Width>1280</tt:Width><tt:Height>960</tt:Height></tt:ResolutionsAvailable><tt:ResolutionsAvailable><tt:Width>1280</tt:Width><tt:Height>720</tt:Height></tt:ResolutionsAvailable><tt:GovLengthRange><tt:Min>30</tt:Min><tt:Max>30</tt:Max></tt:GovLengthRange><tt:FrameRateRange><tt:Min>1</tt:Min><tt:Max>30</tt:Max></tt:FrameRateRange><tt:EncodingIntervalRange><tt:Min>1</tt:Min><tt:Max>400</tt:Max></tt:EncodingIntervalRange><tt:H264ProfilesSupported>Baseline</tt:H264ProfilesSupported><tt:BitrateRange><tt:Min>32</tt:Min><tt:Max>16384</tt:Max></tt:BitrateRange></tt:H264></tt:Extension></trt:Options>");
				break;
			case TW9900:
				sprintf (tmpstr,
						"<tt:Extension><tt:H264><tt:ResolutionsAvailable><tt:Width>704</tt:Width><tt:Height>576</tt:Height></tt:ResolutionsAvailable><tt:ResolutionsAvailable><tt:Width>352</tt:Width><tt:Height>288</tt:Height></tt:ResolutionsAvailable><tt:GovLengthRange><tt:Min>30</tt:Min><tt:Max>30</tt:Max></tt:GovLengthRange><tt:FrameRateRange><tt:Min>1</tt:Min><tt:Max>30</tt:Max></tt:FrameRateRange><tt:EncodingIntervalRange><tt:Min>1</tt:Min><tt:Max>400</tt:Max></tt:EncodingIntervalRange><tt:H264ProfilesSupported>Baseline</tt:H264ProfilesSupported><tt:BitrateRange><tt:Min>32</tt:Min><tt:Max>16384</tt:Max></tt:BitrateRange></tt:H264></tt:Extension></trt:Options>");
				break;
			default:
				sprintf (tmpstr,
						"<tt:Extension><tt:H264><tt:ResolutionsAvailable><tt:Width>1280</tt:Width><tt:Height>960</tt:Height></tt:ResolutionsAvailable><tt:ResolutionsAvailable><tt:Width>1280</tt:Width><tt:Height>720</tt:Height></tt:ResolutionsAvailable><tt:GovLengthRange><tt:Min>30</tt:Min><tt:Max>30</tt:Max></tt:GovLengthRange><tt:FrameRateRange><tt:Min>1</tt:Min><tt:Max>30</tt:Max></tt:FrameRateRange><tt:EncodingIntervalRange><tt:Min>1</tt:Min><tt:Max>400</tt:Max></tt:EncodingIntervalRange><tt:H264ProfilesSupported>Baseline</tt:H264ProfilesSupported><tt:BitrateRange><tt:Min>32</tt:Min><tt:Max>16384</tt:Max></tt:BitrateRange></tt:H264></tt:Extension></trt:Options>");
				break;
		}

		strcat (soap_str, tmpstr);
	}
	else
	{
		memset (tmpstr, 0, HTTP_TEMP_STR_LEN);
		sprintf (tmpstr,
				"<trt:Options><tt:QualityRange><tt:Min>0</tt:Min><tt:Max>5</tt:Max></tt:QualityRange>");
		strcat (soap_str, tmpstr);

		memset (tmpstr, 0, HTTP_TEMP_STR_LEN);
		sprintf (tmpstr,
				"<tt:H264><tt:ResolutionsAvailable><tt:Width>320</tt:Width><tt:Height>240</tt:Height></tt:ResolutionsAvailable><tt:ResolutionsAvailable><tt:Width>640</tt:Width><tt:Height>%d</tt:Height></tt:ResolutionsAvailable><tt:GovLengthRange><tt:Min>30</tt:Min><tt:Max>30</tt:Max></tt:GovLengthRange><tt:FrameRateRange><tt:Min>1</tt:Min><tt:Max>25</tt:Max></tt:FrameRateRange><tt:EncodingIntervalRange><tt:Min>1</tt:Min><tt:Max>400</tt:Max></tt:EncodingIntervalRange><tt:H264ProfilesSupported>Baseline</tt:H264ProfilesSupported></tt:H264>", dvs->video_height/2);
		strcat (soap_str, tmpstr);

		memset (tmpstr, 0, HTTP_TEMP_STR_LEN);
		sprintf (tmpstr,
				"<tt:Extension><tt:H264><tt:ResolutionsAvailable><tt:Width>320</tt:Width><tt:Height>240</tt:Height></tt:ResolutionsAvailable><tt:ResolutionsAvailable><tt:Width>640</tt:Width><tt:Height>%d</tt:Height></tt:ResolutionsAvailable><tt:FrameRateRange><tt:Min>1</tt:Min><tt:Max>25</tt:Max></tt:FrameRateRange><tt:GovLengthRange><tt:Min>30</tt:Min><tt:Max>30</tt:Max></tt:GovLengthRange><tt:EncodingIntervalRange><tt:Min>1</tt:Min><tt:Max>400</tt:Max></tt:EncodingIntervalRange><tt:H264ProfilesSupported>Baseline</tt:H264ProfilesSupported><tt:BitrateRange><tt:Min>32</tt:Min><tt:Max>8192</tt:Max></tt:BitrateRange></tt:H264></tt:Extension></trt:Options>", dvs->video_height/2);
		strcat (soap_str, tmpstr);
	}
	strcat (soap_str,
			"</trt:GetVideoEncoderConfigurationOptionsResponse></env:Body></env:Envelope>\r\n");
	memset (http_str, 0, HTTP_TEMP_STR_LEN);
	sprintf (http_str, http_head_str, strlen (soap_str));

	client_send (client, http_str, strlen (http_str));
	client_send (client, soap_str, strlen (soap_str));
}

static void DoGetCompatibleVideoEncoderConfigurations (char *cmdstr, char *soapstr, dvs_t * dvs, client_t * client)
{
	char tmpstr[HTTP_TEMP_STR_LEN];
	char http_str[HTTP_TEMP_STR_LEN];
	char soap_str[MAX_SOAP_SIZE];

	config_videoprofile_t *videoprofile = &(dvs->chunnel[0].camera.videoprofile);
	config_videoprofile_t *videoprofile_small = &(dvs->chunnel[0].camera.videoprofile_small);
	memset (soap_str, 0, MAX_SOAP_SIZE);
	sprintf (soap_str, "%s%s%s",
			xml_version,
			soap_env_head,
			"<env:Body><trt:GetCompatibleVideoEncoderConfigurationsResponse>");

	memset (tmpstr, 0, HTTP_TEMP_STR_LEN);
	sprintf (tmpstr,
			"<trt:Configurations token=\"VideoEncoderToken_1\"><tt:Name>VideoEncoder_1</tt:Name><tt:UseCount>1</tt:UseCount><tt:Encoding>H264</tt:Encoding><tt:Resolution><tt:Width>%d</tt:Width><tt:Height>%d</tt:Height></tt:Resolution><tt:Quality>3.000000</tt:Quality><tt:RateControl><tt:FrameRateLimit>%d</tt:FrameRateLimit><tt:EncodingInterval>1</tt:EncodingInterval><tt:BitrateLimit>%d</tt:BitrateLimit></tt:RateControl><tt:H264><tt:GovLength>30</tt:GovLength><tt:H264Profile>Baseline</tt:H264Profile></tt:H264><tt:Multicast><tt:Address><tt:Type>IPv4</tt:Type><tt:IPv4Address>239.168.40.236</tt:IPv4Address></tt:Address><tt:Port>0</tt:Port><tt:TTL>1</tt:TTL><tt:AutoStart>false</tt:AutoStart></tt:Multicast><tt:SessionTimeout>PT5S</tt:SessionTimeout></trt:Configurations>",
			dvs->video_width, dvs->video_height, videoprofile->framerate,
			videoprofile->bitrate);
	strcat (soap_str, tmpstr);

	memset (tmpstr, 0, HTTP_TEMP_STR_LEN);
	sprintf (tmpstr,
			"<trt:Configurations token=\"VideoEncoderToken_2\"><tt:Name>VideoEncoder_2</tt:Name><tt:UseCount>1</tt:UseCount><tt:Encoding>H264</tt:Encoding><tt:Resolution><tt:Width>%d</tt:Width><tt:Height>%d</tt:Height></tt:Resolution><tt:Quality>3.000000</tt:Quality><tt:RateControl><tt:FrameRateLimit>%d</tt:FrameRateLimit><tt:EncodingInterval>1</tt:EncodingInterval><tt:BitrateLimit>%d</tt:BitrateLimit></tt:RateControl><tt:H264><tt:GovLength>30</tt:GovLength><tt:H264Profile>Baseline</tt:H264Profile></tt:H264><tt:Multicast><tt:Address><tt:Type>IPv4</tt:Type><tt:IPv4Address>239.168.40.236</tt:IPv4Address></tt:Address><tt:Port>0</tt:Port><tt:TTL>1</tt:TTL><tt:AutoStart>false</tt:AutoStart></tt:Multicast><tt:SessionTimeout>PT5S</tt:SessionTimeout></trt:Configurations>",
			dvs->video_width_small, dvs->video_height_small,
			videoprofile_small->framerate, videoprofile_small->bitrate);
	strcat (soap_str, tmpstr);

	strcat (soap_str,
			"</trt:GetCompatibleVideoEncoderConfigurationsResponse></env:Body></env:Envelope>\r\n");
	memset (http_str, 0, HTTP_TEMP_STR_LEN);
	sprintf (http_str, http_head_str, strlen (soap_str));

	client_send (client, http_str, strlen (http_str));
	client_send (client, soap_str, strlen (soap_str));
}

static void DoGetCompatibleVideoSourceConfigurations (char *cmdstr, char *soapstr, dvs_t * dvs, client_t * client)
{
	char tmpstr[HTTP_TEMP_STR_LEN];
	char http_str[HTTP_TEMP_STR_LEN];
	char soap_str[MAX_SOAP_SIZE];
	char ProfileToken[HTTP_TEMP_STR_LEN];
	int profile_token = 0;

	memset (ProfileToken, 0, HTTP_TEMP_STR_LEN);
	get_message (soapstr, ProfileToken, "<ProfileToken>", "</ProfileToken>");
	if (findStrFromBuf (ProfileToken, strlen (ProfileToken), "Profile_1", strlen ("Profile_1")) >= 0)
	{
		profile_token = 1;
	}
	else if (findStrFromBuf (ProfileToken, strlen (ProfileToken), "Profile_2", strlen ("Profile_2")) >= 0)
	{
		profile_token = 2;
	}

	memset (soap_str, 0, MAX_SOAP_SIZE);
	sprintf (soap_str, "%s%s%s",
			xml_version,
			soap_env_head,
			"<env:Body><trt:GetCompatibleVideoSourceConfigurationsResponse>");

	if (profile_token == 1)
	{
		memset (tmpstr, 0, HTTP_TEMP_STR_LEN);
		sprintf (tmpstr,
				"<trt:Configurations token=\"VideoSourceToken\"><tt:Name>VideoSourceConfig</tt:Name><tt:UseCount>2</tt:UseCount><tt:SourceToken>VideoSource_1</tt:SourceToken><tt:Bounds x=\"0\" y=\"0\" width=\"%d\" height=\"%d\"></tt:Bounds></trt:Configurations>",
				dvs->video_width, dvs->video_height);
		strcat (soap_str, tmpstr);
	}
	else
	{
		memset (tmpstr, 0, HTTP_TEMP_STR_LEN);
		sprintf (tmpstr,
				"<trt:Configurations token=\"VideoSourceToken\"><tt:Name>VideoSourceConfig</tt:Name><tt:UseCount>2</tt:UseCount><tt:SourceToken>VideoSource_2</tt:SourceToken><tt:Bounds x=\"0\" y=\"0\" width=\"%d\" height=\"%d\"></tt:Bounds></trt:Configurations>",
				dvs->video_width / 2, dvs->video_height / 2);
		strcat (soap_str, tmpstr);
	}

	strcat (soap_str,
			"</trt:GetCompatibleVideoSourceConfigurationsResponse></env:Body></env:Envelope>\r\n");
	memset (http_str, 0, HTTP_TEMP_STR_LEN);
	sprintf (http_str, http_head_str, strlen (soap_str));

	client_send (client, http_str, strlen (http_str));
	client_send (client, soap_str, strlen (soap_str));
}

static void DoGetZeroConfiguration (char *cmdstr, char *soapstr, dvs_t * dvs, client_t * client)
{
	char tmpstr[HTTP_TEMP_STR_LEN];
	char http_str[HTTP_TEMP_STR_LEN];
	char soap_str[MAX_SOAP_SIZE];

	memset (soap_str, 0, MAX_SOAP_SIZE);
	sprintf (soap_str, "%s%s%s",
			xml_version,
			soap_env_head, "<env:Body><tds:GetZeroConfigurationResponse>");
	memset (tmpstr, 0, HTTP_TEMP_STR_LEN);
	sprintf (tmpstr,
			"<tds:ZeroConfiguration><tt:InterfaceToken>eth0</tt:InterfaceToken><tt:Enabled>true</tt:Enabled></tds:ZeroConfiguration>");
	strcat (soap_str, tmpstr);

	strcat (soap_str,
			"</tds:GetZeroConfigurationResponse></env:Body></env:Envelope>\r\n");
	memset (http_str, 0, HTTP_TEMP_STR_LEN);
	sprintf (http_str, http_head_str, strlen (soap_str));

	client_send (client, http_str, strlen (http_str));
	client_send (client, soap_str, strlen (soap_str));
}

static void DoGetServiceCapabilities (char *cmdstr, char *soapstr, dvs_t * dvs, client_t * client)
{
	char tmpstr[HTTP_TEMP_STR_LEN];
	char http_str[HTTP_TEMP_STR_LEN];
	char soap_str[MAX_SOAP_SIZE];
	int service_type = 0;
	char service_str[HTTP_TEMP_STR_LEN];

	memset (service_str, 0, HTTP_TEMP_STR_LEN);
	get_message (soapstr, service_str, "<GetServiceCapabilities", ">");
	if (findStrFromBuf (service_str, strlen (service_str), "/media/wsdl", strlen ("/media/wsdl")) >= 0)
	{
		service_type = 1;
	}
	else if (findStrFromBuf (service_str, strlen (service_str), "/events/wsdl", strlen ("/events/wsdl")) >= 0)
	{
		service_type = 2;
	}
	else if (findStrFromBuf (service_str, strlen (service_str), "/imaging/wsdl", strlen ("/imaging/wsdl")) >= 0)
	{
		service_type = 3;
	}
	else if (findStrFromBuf (service_str, strlen (service_str), "/device/wsdl", strlen ("/device/wsdl")) >= 0)
	{
		service_type = 4;
	}

	memset (soap_str, 0, MAX_SOAP_SIZE);
	sprintf (soap_str, "%s%s%s",
			xml_version,
			soap_env_head,
			"<env:Header><wsa:Action>http://www.onvif.org/ver10/events/wsdl/EventPortType/GetServiceCapabilitiesResponse</wsa:Action></env:Header><env:Body>");
	memset (tmpstr, 0, HTTP_TEMP_STR_LEN);
	switch (service_type)
	{
		case 1:
			sprintf (tmpstr,
					"<trt:GetServiceCapabilitiesResponse><trt:Capabilities><trt:ProfileCapabilities MaximumNumberOfProfiles=\"3\"></trt:ProfileCapabilities><trt:StreamingCapabilities RTPMulticast=\"true\" RTP_TCP=\"true\" RTP_RTSP_TCP=\"true\" NonAggregateControl=\"false\"></trt:StreamingCapabilities></trt:Capabilities></trt:GetServiceCapabilitiesResponse>");
			break;
		case 2:
			sprintf (tmpstr,
					"<tev:GetServiceCapabilitiesResponse><tev:Capabilities WSSubscriptionPolicySupport=\"true\" WSPullPointSupport=\"true\" WSPausableSubscriptionManagerInterfaceSupport=\"false\"></tev:Capabilities></tev:GetServiceCapabilitiesResponse>");
			break;
		case 3:
			sprintf (tmpstr,
					"<timg:GetServiceCapabilitiesResponse><timg:Capabilities /></timg:GetServiceCapabilitiesResponse>");
			break;
		case 4:
			sprintf (tmpstr,
					"<tds:GetServiceCapabilitiesResponse><tds:Capabilities><tds:Network IPFilter=\"true\" ZeroConfiguration=\"true\" IPVersion6=\"false\" DynDNS=\"true\" Dot11Configuration=\"false\" HostnameFromDHCP=\"true\" NTP=\"1\"></tds:Network><tds:Security TLS1.0=\"false\" TLS1.1=\"false\" TLS1.2=\"false\" OnboardKeyGeneration=\"false\" AccessPolicyConfig=\"false\" DefaultAccessPolicy=\"true\" Dot1X=\"false\" RemoteUserHandling=\"false\" X.509Token=\"false\" SAMLToken=\"false\" KerberosToken=\"false\" UsernameToken=\"true\" HttpDigest=\"false\" RELToken=\"false\" SupportedEAPMethods=\"0\"></tds:Security><tds:System DiscoveryResolve=\"false\" DiscoveryBye=\"true\" RemoteDiscovery=\"true\" SystemBackup=\"false\" SystemLogging=\"false\" FirmwareUpgrade=\"false\" HttpFirmwareUpgrade=\"true\" HttpSystemBackup=\"false\" HttpSystemLogging=\"false\" HttpSupportInformation=\"false\"></tds:System></tds:Capabilities></tds:GetServiceCapabilitiesResponse>");
			break;
		default:
			sprintf (tmpstr,
					"<tds:GetServiceCapabilitiesResponse><tds:Capabilities><tds:Network IPFilter=\"true\" ZeroConfiguration=\"true\" IPVersion6=\"false\" DynDNS=\"true\" Dot11Configuration=\"false\" HostnameFromDHCP=\"true\" NTP=\"1\"></tds:Network><tds:Security TLS1.0=\"false\" TLS1.1=\"false\" TLS1.2=\"false\" OnboardKeyGeneration=\"false\" AccessPolicyConfig=\"false\" DefaultAccessPolicy=\"true\" Dot1X=\"false\" RemoteUserHandling=\"false\" X.509Token=\"false\" SAMLToken=\"false\" KerberosToken=\"false\" UsernameToken=\"true\" HttpDigest=\"false\" RELToken=\"false\" SupportedEAPMethods=\"0\"></tds:Security><tds:System DiscoveryResolve=\"false\" DiscoveryBye=\"true\" RemoteDiscovery=\"true\" SystemBackup=\"false\" SystemLogging=\"false\" FirmwareUpgrade=\"false\" HttpFirmwareUpgrade=\"true\" HttpSystemBackup=\"false\" HttpSystemLogging=\"false\" HttpSupportInformation=\"false\"></tds:System></tds:Capabilities></tds:GetServiceCapabilitiesResponse>");
			break;
	}
	strcat (soap_str, tmpstr);
	strcat (soap_str, "</env:Body></env:Envelope>\r\n");

	memset (http_str, 0, HTTP_TEMP_STR_LEN);
	sprintf (http_str, http_head_str, strlen (soap_str));

	client_send (client, http_str, strlen (http_str));
	client_send (client, soap_str, strlen (soap_str));
}

static void DoGetNTP (char *cmdstr, char *soapstr, dvs_t * dvs, client_t * client)
{
	char tmpstr[HTTP_TEMP_STR_LEN];
	char http_str[HTTP_TEMP_STR_LEN];
	char soap_str[MAX_SOAP_SIZE];

	memset (soap_str, 0, MAX_SOAP_SIZE);
	sprintf (soap_str, "%s%s%s",
			xml_version, soap_env_head, "<env:Body><tds:GetNTPResponse>");
	memset (tmpstr, 0, HTTP_TEMP_STR_LEN);
	sprintf (tmpstr,
			"<tds:NTPInformation><tt:FromDHCP>false</tt:FromDHCP><tt:NTPManual><tt:Type>IPv4</tt:Type><tt:IPv4Address>0.0.0.0</tt:IPv4Address></tt:NTPManual></tds:NTPInformation>");
	strcat (soap_str, tmpstr);

	strcat (soap_str, "</tds:GetNTPResponse></env:Body></env:Envelope>\r\n");
	memset (http_str, 0, HTTP_TEMP_STR_LEN);
	sprintf (http_str, http_head_str, strlen (soap_str));

	client_send (client, http_str, strlen (http_str));
	client_send (client, soap_str, strlen (soap_str));
}

static void DoSetNTP (char *cmdstr, char *soapstr, dvs_t * dvs, client_t * client)
{
	char http_str[HTTP_TEMP_STR_LEN];
	char soap_str[MAX_SOAP_SIZE];
	//config_network_t *network = &(dvs->config.network);

	memset (soap_str, 0, MAX_SOAP_SIZE);
	sprintf (soap_str, "%s%s%s",
			xml_version, soap_env_head, "<env:Body><tds:SetNTPResponse/>");

	strcat (soap_str,
			"</env:Body></env:Envelope>\r\n");
	memset (http_str, 0, HTTP_TEMP_STR_LEN);
	sprintf (http_str, http_head_str, strlen (soap_str));

	client_send (client, http_str, strlen (http_str));
	client_send (client, soap_str, strlen (soap_str));
}

static void DoGetNetworkDefaultGateway (char *cmdstr, char *soapstr, dvs_t * dvs, client_t * client)
{
	char tmpstr[HTTP_TEMP_STR_LEN];
	char http_str[HTTP_TEMP_STR_LEN];
	char soap_str[MAX_SOAP_SIZE];

	memset (soap_str, 0, MAX_SOAP_SIZE);
	sprintf (soap_str, "%s%s%s",
			xml_version,
			soap_env_head, "<env:Body><tds:GetNetworkDefaultGatewayResponse>");
	memset (tmpstr, 0, HTTP_TEMP_STR_LEN);
	sprintf (tmpstr,
			"<tds:NetworkGateway><tt:IPv4Address>%s</tt:IPv4Address></tds:NetworkGateway>",
			dvs->config.network.gateway);
	strcat (soap_str, tmpstr);

	strcat (soap_str,
			"</tds:GetNetworkDefaultGatewayResponse></env:Body></env:Envelope>\r\n");
	memset (http_str, 0, HTTP_TEMP_STR_LEN);
	sprintf (http_str, http_head_str, strlen (soap_str));

	client_send (client, http_str, strlen (http_str));
	client_send (client, soap_str, strlen (soap_str));
}

static void DoGetHostname (char *cmdstr, char *soapstr, dvs_t * dvs, client_t * client)
{
	char tmpstr[HTTP_TEMP_STR_LEN];
	char http_str[HTTP_TEMP_STR_LEN];
	char soap_str[MAX_SOAP_SIZE];

	memset (soap_str, 0, MAX_SOAP_SIZE);
	sprintf (soap_str, "%s%s%s",
			xml_version, soap_env_head, "<env:Body><tds:GetHostnameResponse>");
	memset (tmpstr, 0, HTTP_TEMP_STR_LEN);
	sprintf (tmpstr,
			"<tds:HostnameInformation><tt:FromDHCP>true</tt:FromDHCP><tt:Name>H.264_IPCamera</tt:Name></tds:HostnameInformation>");
	strcat (soap_str, tmpstr);

	strcat (soap_str,
			"</tds:GetHostnameResponse></env:Body></env:Envelope>\r\n");
	memset (http_str, 0, HTTP_TEMP_STR_LEN);
	sprintf (http_str, http_head_str, strlen (soap_str));

	client_send (client, http_str, strlen (http_str));
	client_send (client, soap_str, strlen (soap_str));
}

static void DoGetNetworkProtocols (char *cmdstr, char *soapstr, dvs_t * dvs, client_t * client)
{
	char tmpstr[HTTP_TEMP_STR_LEN];
	char http_str[HTTP_TEMP_STR_LEN];
	char soap_str[MAX_SOAP_SIZE];

	memset (soap_str, 0, MAX_SOAP_SIZE);
	sprintf (soap_str, "%s%s%s",
			xml_version,
			soap_env_head, "<env:Body><tds:GetNetworkProtocolsResponse>");
	memset (tmpstr, 0, HTTP_TEMP_STR_LEN);
	sprintf (tmpstr,
			"<tds:NetworkProtocols><tt:Name>HTTP</tt:Name><tt:Enabled>true</tt:Enabled><tt:Port>80</tt:Port></tds:NetworkProtocols>");
	strcat (soap_str, tmpstr);

	memset (tmpstr, 0, HTTP_TEMP_STR_LEN);
	sprintf (tmpstr,
			"<tds:NetworkProtocols><tt:Name>RTSP</tt:Name><tt:Enabled>true</tt:Enabled><tt:Port>554</tt:Port></tds:NetworkProtocols>");
	strcat (soap_str, tmpstr);

	strcat (soap_str,
			"</tds:GetNetworkProtocolsResponse></env:Body></env:Envelope>\r\n");
	memset (http_str, 0, HTTP_TEMP_STR_LEN);
	sprintf (http_str, http_head_str, strlen (soap_str));

	client_send (client, http_str, strlen (http_str));
	client_send (client, soap_str, strlen (soap_str));
}

static void DoGetUsers (char *cmdstr, char *soapstr, dvs_t * dvs, client_t * client)
{
	char tmpstr[HTTP_TEMP_STR_LEN];
	char http_str[HTTP_TEMP_STR_LEN];
	char soap_str[MAX_SOAP_SIZE];

	memset (soap_str, 0, MAX_SOAP_SIZE);
	sprintf (soap_str, "%s%s%s",
			xml_version, soap_env_head, "<env:Body><tds:GetUsersResponse>");
	memset (tmpstr, 0, HTTP_TEMP_STR_LEN);
	sprintf (tmpstr,
			"<tds:User><tt:Username>admin</tt:Username><tt:UserLevel >Administrator</tt:UserLevel ></tds:User>");
	strcat (soap_str, tmpstr);

	strcat (soap_str, "</tds:GetUsersResponse></env:Body></env:Envelope>\r\n");
	memset (http_str, 0, HTTP_TEMP_STR_LEN);
	sprintf (http_str, http_head_str, strlen (soap_str));

	client_send (client, http_str, strlen (http_str));
	client_send (client, soap_str, strlen (soap_str));
}

static void DoGetWsdlUrl (char *cmdstr, char *soapstr, dvs_t * dvs, client_t * client)
{
	char tmpstr[HTTP_TEMP_STR_LEN];
	char http_str[HTTP_TEMP_STR_LEN];
	char soap_str[MAX_SOAP_SIZE];

	memset (soap_str, 0, MAX_SOAP_SIZE);
	sprintf (soap_str, "%s%s%s",
			xml_version, soap_env_head, "<env:Body><tds:GetWsdlUrlResponse>");
	memset (tmpstr, 0, HTTP_TEMP_STR_LEN);
	sprintf (tmpstr, "<tds:WsdlUrl>http://www.onvif.org/</tds:WsdlUrl>");
	strcat (soap_str, tmpstr);

	strcat (soap_str,
			"</tds:GetWsdlUrlResponse></env:Body></env:Envelope>\r\n");
	memset (http_str, 0, HTTP_TEMP_STR_LEN);
	sprintf (http_str, http_head_str, strlen (soap_str));

	client_send (client, http_str, strlen (http_str));
	client_send (client, soap_str, strlen (soap_str));
}

static void DoGetDiscoveryMode (char *cmdstr, char *soapstr, dvs_t * dvs, client_t * client)
{
	char tmpstr[HTTP_TEMP_STR_LEN];
	char http_str[HTTP_TEMP_STR_LEN];
	char soap_str[MAX_SOAP_SIZE];

	memset (soap_str, 0, MAX_SOAP_SIZE);
	sprintf (soap_str, "%s%s%s",
			xml_version, soap_env_head,
			"<env:Body><tds:GetDiscoveryModeResponse>");

	memset (tmpstr, 0, HTTP_TEMP_STR_LEN);
	sprintf (tmpstr, "<tds:DiscoveryMode>Discoverable</tds:DiscoveryMode>");
	strcat (soap_str, tmpstr);

	strcat (soap_str,
			"</tds:GetDiscoveryModeResponse></env:Body></env:Envelope>\r\n");
	memset (http_str, 0, HTTP_TEMP_STR_LEN);
	sprintf (http_str, http_head_str, strlen (soap_str));

	client_send (client, http_str, strlen (http_str));
	client_send (client, soap_str, strlen (soap_str));
}

static void DoGetImagingSettings (char *cmdstr, char *soapstr, dvs_t * dvs, client_t * client)
{
	char tmpstr[HTTP_TEMP_STR_LEN];
	char http_str[HTTP_TEMP_STR_LEN];
	char soap_str[MAX_SOAP_SIZE];
	config_videocolor_t *videocolor = &(dvs->chunnel[0].camera.videocolor);

	memset (soap_str, 0, MAX_SOAP_SIZE);
	sprintf (soap_str, "%s%s%s",
			xml_version, soap_env_head,
			"<env:Body><timg:GetImagingSettingsResponse>");

	memset (tmpstr, 0, HTTP_TEMP_STR_LEN);

	sprintf (tmpstr,
			"<timg:ImagingSettings><tt:Brightness>%d</tt:Brightness><tt:ColorSaturation>%d</tt:ColorSaturation><tt:Contrast>%d</tt:Contrast></timg:ImagingSettings>",
			videocolor->brightness, videocolor->saturation,
			videocolor->contrast);
	strcat (soap_str, tmpstr);

	strcat (soap_str,
			"</timg:GetImagingSettingsResponse></env:Body></env:Envelope>\r\n");
	memset (http_str, 0, HTTP_TEMP_STR_LEN);
	sprintf (http_str, http_head_str, strlen (soap_str));

	client_send (client, http_str, strlen (http_str));
	client_send (client, soap_str, strlen (soap_str));
}

static void DoGetOptions (char *cmdstr, char *soapstr, dvs_t * dvs, client_t * client)
{
	char tmpstr[HTTP_TEMP_STR_LEN];
	char http_str[HTTP_TEMP_STR_LEN];
	char soap_str[MAX_SOAP_SIZE];
	int service_type = 0;
	char service_str[HTTP_TEMP_STR_LEN];

	memset (service_str, 0, HTTP_TEMP_STR_LEN);
	get_message (soapstr, service_str, "<GetOptions", ">");
	if (findStrFromBuf (service_str, strlen (service_str), "/media/wsdl", strlen ("/media/wsdl")) >= 0)
	{
		service_type = 1;
	}
	else if (findStrFromBuf (service_str, strlen (service_str), "/events/wsdl", strlen ("/events/wsdl")) >= 0)
	{
		service_type = 2;
	}
	else if (findStrFromBuf (service_str, strlen (service_str), "/imaging/wsdl", strlen ("/imaging/wsdl")) >= 0)
	{
		service_type = 3;
	}
	else if (findStrFromBuf (service_str, strlen (service_str), "/device/wsdl", strlen ("/device/wsdl")) >= 0)
	{
		service_type = 4;
	}

	memset (soap_str, 0, MAX_SOAP_SIZE);
	sprintf (soap_str, "%s%s%s", xml_version, soap_env_head, "<env:Body>");
	memset (tmpstr, 0, HTTP_TEMP_STR_LEN);
	switch (service_type)
	{
		case 3:
			sprintf (tmpstr,
					"<timg:GetOptionsResponse><timg:ImagingOptions><tt:Brightness><tt:Min>0</tt:Min><tt:Max>255</tt:Max></tt:Brightness><tt:ColorSaturation><tt:Min>0</tt:Min><tt:Max>255</tt:Max></tt:ColorSaturation><tt:Contrast><tt:Min>0</tt:Min><tt:Max>255</tt:Max></tt:Contrast></timg:ImagingOptions></timg:GetOptionsResponse> ");
			strcat (soap_str, tmpstr);
			break;
		default:
			break;
	}

	strcat (soap_str, "</env:Body></env:Envelope>\r\n");
	memset (http_str, 0, HTTP_TEMP_STR_LEN);
	sprintf (http_str, http_head_str, strlen (soap_str));

	client_send (client, http_str, strlen (http_str));
	client_send (client, soap_str, strlen (soap_str));
}

static void DoGetMoveOptions (char *cmdstr, char *soapstr, dvs_t * dvs, client_t * client)
{
	char tmpstr[HTTP_TEMP_STR_LEN];
	char http_str[HTTP_TEMP_STR_LEN];
	char soap_str[MAX_SOAP_SIZE];
	int service_type = 0;
	char service_str[HTTP_TEMP_STR_LEN];

	memset (service_str, 0, HTTP_TEMP_STR_LEN);
	get_message (soapstr, service_str, "<GetMoveOptions", ">");
	if (findStrFromBuf (service_str, strlen (service_str), "/media/wsdl", strlen ("/media/wsdl")) >= 0)
	{
		service_type = 1;
	}
	else if (findStrFromBuf (service_str, strlen (service_str), "/events/wsdl", strlen ("/events/wsdl")) >= 0)
	{
		service_type = 2;
	}
	else if (findStrFromBuf (service_str, strlen (service_str), "/imaging/wsdl", strlen ("/imaging/wsdl")) >= 0)
	{
		service_type = 3;
	}
	else if (findStrFromBuf (service_str, strlen (service_str), "/device/wsdl", strlen ("/device/wsdl")) >= 0)
	{
		service_type = 4;
	}

	memset (soap_str, 0, MAX_SOAP_SIZE);
	sprintf (soap_str, "%s%s%s", xml_version, soap_env_head, "<env:Body>");
	memset (tmpstr, 0, HTTP_TEMP_STR_LEN);
	switch (service_type)
	{
		case 3:
			sprintf (tmpstr,
					"<timg:GetMoveOptionsResponse><timg:MoveOptions/></timg:GetMoveOptionsResponse>");
			break;
		default:
			break;
	}

	strcat (soap_str, tmpstr);

	strcat (soap_str, "</env:Body></env:Envelope>\r\n");
	memset (http_str, 0, HTTP_TEMP_STR_LEN);
	sprintf (http_str, http_head_str, strlen (soap_str));

	client_send (client, http_str, strlen (http_str));
	client_send (client, soap_str, strlen (soap_str));
}

static void DoCommandNotFind (dvs_t * dvs, client_t * client)
{
	char http_str[HTTP_TEMP_STR_LEN];
	char soap_str[MAX_SOAP_SIZE];

	memset (soap_str, 0, MAX_SOAP_SIZE);
	sprintf (soap_str, "%s%s%s",
			xml_version,
			soap_env_head,
			"<env:Code><env:Value>env:Sender</env:Value><env:Subcode><env:Value>ter:ActionNotSupported</env:Value><env:Subcode><env:Value>ter:AudioNotSupported</env:Value></env:Subcode></env:Subcode></env:Code><env:Reason><env:Text xml:lang=\"en\">NVT does not support audio.</env:Text></env:Reason></env:Fault></env:Body></env:Envelope>");

	memset (http_str, 0, HTTP_TEMP_STR_LEN);
	sprintf (http_str,
			"HTTP/1.1 500 Internal Server Error\r\nServer: App-webs/\r\nConnection: close\r\nContent-Length: %d\r\nContent-Type: application/soap+xml; charset=utf-8\r\n\r\n",
			strlen (soap_str));
	client_send (client, http_str, strlen (http_str));
	client_send (client, soap_str, strlen (soap_str));
}

static void make_netmask_str (char *str, int prefixlen)
{
	int i;
	int netmask = 0xffffffff;

	for (i = 0; i < 32 - prefixlen; i++)
	{
		netmask = netmask & (~(0x01 << i));
	}
	sprintf (str, "%d.%d.%d.%d",
			(netmask >> 24) & 0x000000ff,
			(netmask >> 16) & 0x000000ff,
			(netmask >> 8) & 0x000000ff, netmask & 0x000000ff);
}

static void DoSetNetworkInterfaces (char *cmdstr, char *soapstr, dvs_t * dvs, client_t * client)
{
	int res, res1;
	char tmpstr[HTTP_TEMP_STR_LEN];
	char tmpstr1[HTTP_TEMP_STR_LEN];
	char http_str[HTTP_TEMP_STR_LEN];
	char soap_str[MAX_SOAP_SIZE];
	config_network_t *network = &(dvs->config.network);

	memset (tmpstr, 0, HTTP_TEMP_STR_LEN);
	res = soap_get_value (soapstr, tmpstr, "Manual");
	if (res >= 0)
	{
		memset (tmpstr1, 0, HTTP_TEMP_STR_LEN);
		res1 = soap_get_value (tmpstr, tmpstr1, "Address");
		if (res1 >= 0)
		{
			sprintf (network->ip, "%s", tmpstr1);
		}
		memset (tmpstr1, 0, HTTP_TEMP_STR_LEN);
		res1 = soap_get_value (tmpstr, tmpstr1, "PrefixLength");
		if (res1 >= 0)
		{
			//todo:make netmask str
			// printf ("tmpstr1=%s\r\n", tmpstr1);
			// printf ("PrefixLength=%d\r\n", atoi (tmpstr1));
			make_netmask_str (network->netmask, atoi (tmpstr1));

			//sprintf(network->netmask, "%s", tmpstr1);
		}
	}
	memset (tmpstr1, 0, HTTP_TEMP_STR_LEN);

	struct netinfo_t netinfo;
	res1 = soap_get_value (soapstr, tmpstr1, "DHCP");
	if (res1 >= 0)
	{
		if (0 == strcmp("true", tmpstr1))
		{
			g_dvs.config.network.type = 1;
			g_dvs.config.network.auto_get_dns = 1;
			GetNet(&netinfo);
			netinfo.IpType = g_dvs.config.network.type;//0:Fixed IP  1:dynamic ip
			netinfo.DnsType = g_dvs.config.network.auto_get_dns;//0:no	1:yes
			SetNet(&netinfo);
		}
		else
		{
			g_dvs.config.network.type = 0;
			g_dvs.config.network.auto_get_dns = 0;
			ajust_ipgateway (network->ip, network->netmask, network->gateway);
			GetNet(&netinfo);
			netinfo.IpType = g_dvs.config.network.type;//0:Fixed IP  1:dynamic ip
			netinfo.DnsType = g_dvs.config.network.auto_get_dns;//0:no	1:yes
			sprintf(netinfo.cIp, "%s", network->ip);
			sprintf(netinfo.cNetMask, "%s", network->netmask);
			sprintf(netinfo.cGateway, "%s", network->gateway);
			sprintf(netinfo.cMDns, "%s", g_dvs.config.network.dns1);
			sprintf(netinfo.cSDns, "%s", g_dvs.config.network.dns2);
			SetNet(&netinfo);
		}
	}
	/*GetNet(&netinfo);
	  netinfo.IpType = g_dvs.config.network.type;//0:Fixed IP  1:dynamic ip
	  netinfo.DnsType = g_dvs.config.network.auto_get_dns;//0:no	1:yes
	  sprintf(netinfo.cIp, "%s", network->ip);
	  sprintf(netinfo.cNetMask, "%s", network->netmask);
	  sprintf(netinfo.cGateway, "%s", network->gateway);
	  sprintf(netinfo.cMDns, "%s", g_dvs.config.network.dns1);
	  sprintf(netinfo.cSDns, "%s", g_dvs.config.network.dns2);
	  SetNet(&netinfo);*/
	// printf("GetNet save value %d %d %s %s %s %s\r\n", netinfo.IpType, netinfo.DnsType, netinfo.cIp, netinfo.cNetMask, netinfo.cGateway, netinfo.cMDns, netinfo.cSDns);fflush(NULL);

	printf ("ip:%s, netmask:%s, gateway:%s\r\n", network->ip, network->netmask, network->gateway);

	memset (soap_str, 0, MAX_SOAP_SIZE);
	sprintf (soap_str, "%s%s%s",
			xml_version, soap_env_head,
			"<env:Body><tds:SetNetworkInterfacesResponse>");

	memset (tmpstr, 0, HTTP_TEMP_STR_LEN);
	sprintf (tmpstr, "<tds:RebootNeeded>true</tds:RebootNeeded>");
	strcat (soap_str, tmpstr);

	strcat (soap_str,
			"</tds:SetNetworkInterfacesResponse></env:Body></env:Envelope>\r\n");
	memset (http_str, 0, HTTP_TEMP_STR_LEN);
	sprintf (http_str, http_head_str, strlen (soap_str));
	client_send (client, http_str, strlen (http_str));
	client_send (client, soap_str, strlen (soap_str));
}

static void DoSetNetworkDefaultGateway (char *cmdstr, char *soapstr, dvs_t * dvs, client_t * client)
{
	int res, res1;
	char tmpstr[HTTP_TEMP_STR_LEN];
	char tmpstr1[HTTP_TEMP_STR_LEN];
	char http_str[HTTP_TEMP_STR_LEN];
	char soap_str[MAX_SOAP_SIZE];
	config_network_t *network = &(dvs->config.network);

	memset (tmpstr, 0, HTTP_TEMP_STR_LEN);
	res = soap_get_value (soapstr, tmpstr, "SetNetworkDefaultGateway");
	if (res >= 0)
	{
		memset (tmpstr1, 0, HTTP_TEMP_STR_LEN);
		res1 = soap_get_value (tmpstr, tmpstr1, "IPv4Address");
		if (res1 >= 0)
		{
			sprintf (network->gateway, "%s", tmpstr1);
		}

		ajust_ipgateway (network->ip, network->netmask, network->gateway);
		config_save_network (network);
	}

	//printf ("ip:%s, netmask:%s, gateway:%s\r\n", network->ip, network->netmask, network->gateway);
	memset (soap_str, 0, MAX_SOAP_SIZE);
	sprintf (soap_str, "%s%s%s",
			xml_version, soap_env_head,
			"<env:Body><tds:SetNetworkDefaultGatewayResponse>");

	strcat (soap_str,
			"</tds:SetNetworkDefaultGatewayResponse></env:Body></env:Envelope>\r\n");
	memset (http_str, 0, HTTP_TEMP_STR_LEN);
	sprintf (http_str, http_head_str, strlen (soap_str));

	client_send (client, http_str, strlen (http_str));
	client_send (client, soap_str, strlen (soap_str));
}

static void DoSetHostname (char *cmdstr, char *soapstr, dvs_t * dvs, client_t * client)
{
	int res, res1;
	char tmpstr[HTTP_TEMP_STR_LEN];
	char tmpstr1[HTTP_TEMP_STR_LEN];
	char http_str[HTTP_TEMP_STR_LEN];
	char soap_str[MAX_SOAP_SIZE];
	//config_network_t *network = &(dvs->config.network);

	memset (tmpstr, 0, HTTP_TEMP_STR_LEN);
	res = soap_get_value (soapstr, tmpstr, "SetHostname");
	if (res >= 0)
	{
		memset (tmpstr1, 0, HTTP_TEMP_STR_LEN);
		res1 = soap_get_value (tmpstr, tmpstr1, "Name");
		if (res1 >= 0)
		{
			printf ("SetHostname:%s\r\n", tmpstr1);
		}

	}

	memset (soap_str, 0, MAX_SOAP_SIZE);
	sprintf (soap_str, "%s%s%s",
			xml_version, soap_env_head, "<env:Body><tds:SetHostnameResponse>");

	strcat (soap_str,
			"</tds:SetHostnameResponse></env:Body></env:Envelope>\r\n");
	memset (http_str, 0, HTTP_TEMP_STR_LEN);
	sprintf (http_str, http_head_str, strlen (soap_str));

	client_send (client, http_str, strlen (http_str));
	client_send (client, soap_str, strlen (soap_str));
}

static int find_first_digital(char *str, int len)
{
	int i;
	for(i=0;i<len;i++)
	{
		if(str[i] >='0' && str[i] <='9')
		{
			return i;
		}
	}
	return -1;
}

static int get_time_zone_from_str(char *str)
{
	char tmpstr[ TEMP_STR_LEN];
	int m = -1;
	int hour, min, sec;
	hour = 0;
	min = 0;
	sec = 0;
	int len = strlen(str);
	int pos = findStrFromBuf (str, len, "+", 1);
	if (pos < 0)
	{
		pos = findStrFromBuf (str, len, "-", 1);
		if(pos < 0)
		{
			pos = find_first_digital(str, len);
			if(pos < 0)
			{
				return -1;
			}
			else
			{
				m = -1;
				pos = pos - 1;
			}
			printf("find_first_digital, pos=%d\r\n", pos);
		}
		else
		{
			m = 1;
		}
	}
	int pos1 = findStrFromBuf (str + pos + 1, len-pos-1, ":", 1);
	if(pos1 < 0)
	{
		hour = atoi(str + pos + 1);
		printf("hour=%d\r\n", hour);
		return  m*hour*3600;
	}
	else
	{
		memset (tmpstr, 0, TEMP_STR_LEN);
		memcpy(tmpstr, str + pos + 1, pos1);
		hour = atoi(tmpstr);
		printf("hour=%d\r\n", hour);
	}
	pos = pos + 1 + pos1;
	pos1 = findStrFromBuf (str + pos + 1, len-pos-1, ":", 1);
	if(pos1 < 0)
	{
		min = atoi(str + pos + 1);
		printf("hour=%d,min=%d,sec=%d\r\n", hour, min, sec);
		return  m*(hour*3600+ min*60);
	}
	else
	{
		memset (tmpstr, 0, TEMP_STR_LEN);
		memcpy(tmpstr, str + pos + 1, pos1);
		min = atoi(tmpstr);
		sec = atoi(str + pos + pos1 + 1);
		printf("hour=%d,min=%d,sec=%d\r\n", hour, min, sec);
		return  m*(hour*3600 + min*60 +sec) ;
	}
	return m*hour*3600;
}

static void DoSetSystemDateAndTime (char *cmdstr, char *soapstr, dvs_t * dvs, client_t * client)
{
	int res, res1;
	char tmpstr[HTTP_TEMP_STR_LEN];
	char tmpstr1[HTTP_TEMP_STR_LEN];
	char http_str[HTTP_TEMP_STR_LEN];
	char soap_str[MAX_SOAP_SIZE];
	int DaylightSavings = 0;
	int DateTimeType = 0;
	int hour = 1;
	int minute = 1;
	int second = 1;
	int year = 2012;
	int month = 1;
	int day = 1;
	int timezone;

	//  printf("[%s]\r\n", soapstr);
	memset (tmpstr, 0, HTTP_TEMP_STR_LEN);
	res = soap_get_value (soapstr, tmpstr, "SetSystemDateAndTime");
	if (res >= 0)
	{

		memset (tmpstr1, 0, HTTP_TEMP_STR_LEN);
		res1 = soap_get_value (tmpstr, tmpstr1, "DateTimeType");
		if (res1 >= 0)
		{
			if (strcmp (tmpstr1, "Manual") == 0)
			{
				DateTimeType = 0;
			}
		}
		memset (tmpstr1, 0, HTTP_TEMP_STR_LEN);
		res1 = soap_get_value (tmpstr, tmpstr1, "DaylightSavings");
		if (res1 >= 0)
		{
			if (strcmp (tmpstr1, "false") == 0)
			{
				DaylightSavings = 0;
			}
			else
			{
				DaylightSavings = 1;
			}
		}

		memset (tmpstr1, 0, HTTP_TEMP_STR_LEN);
		res1 = soap_get_value (tmpstr, tmpstr1, "TZ");
		if (res1 >= 0)
		{
			timezone = get_time_zone_from_str(tmpstr1);
			if(timezone != -1)
			{
				dvs->time_zone = timezone;
				IniWriteInteger ("sys", "timezone", dvs->time_zone + 12*3600, "/etc/jffs2/system.ini");
			}
			else
			{
				goto InvalidTimeZone;
			}
		}

		memset (tmpstr1, 0, HTTP_TEMP_STR_LEN);
		res1 = soap_get_value (tmpstr, tmpstr1, "Hour");
		if (res1 >= 0)
		{
			hour = atoi (tmpstr1);
		}
		memset (tmpstr1, 0, HTTP_TEMP_STR_LEN);
		res1 = soap_get_value (tmpstr, tmpstr1, "Minute");
		if (res1 >= 0)
		{
			minute = atoi (tmpstr1);
		}

		memset (tmpstr1, 0, HTTP_TEMP_STR_LEN);
		res1 = soap_get_value (tmpstr, tmpstr1, "Second");
		if (res1 >= 0)
		{
			second = atoi (tmpstr1);
		}

		memset (tmpstr1, 0, HTTP_TEMP_STR_LEN);
		res1 = soap_get_value (tmpstr, tmpstr1, "Year");
		if (res1 >= 0)
		{
			year = atoi (tmpstr1);
		}

		memset (tmpstr1, 0, HTTP_TEMP_STR_LEN);
		res1 = soap_get_value (tmpstr, tmpstr1, "Month");
		if (res1 >= 0)
		{
			month = atoi (tmpstr1);
		}

		memset (tmpstr1, 0, HTTP_TEMP_STR_LEN);
		res1 = soap_get_value (tmpstr, tmpstr1, "Day");
		if (res1 >= 0)
		{
			day = atoi (tmpstr1);
		}
		printf ("%.4d-%.2d-%.2d %.2d:%.2d:%.2d\r\n", year, month, day, hour,
				minute, second);
		struct timeval sTimeVal;
		struct tm sTm;
		sTm.tm_year = year - 1900;
		sTm.tm_mon = month - 1;
		sTm.tm_mday = day;
		sTm.tm_hour = hour;
		sTm.tm_min = minute;
		sTm.tm_sec = second;

		time_t tTime;
		tTime = mktime (&sTm)  + dvs->time_zone;
		sTimeVal.tv_sec = tTime;
		sTimeVal.tv_usec = 0;
		settimeofday (&sTimeVal, NULL);
		set_rtc_time_from_sys_time (dvs);

	}

	memset (soap_str, 0, MAX_SOAP_SIZE);
	sprintf (soap_str, "%s%s%s",
			xml_version, soap_env_head,
			"<env:Body><tds:SetSystemDateAndTimeResponse>");

	strcat (soap_str,
			"</tds:SetSystemDateAndTimeResponse></env:Body></env:Envelope>\r\n");

	memset (http_str, 0, HTTP_TEMP_STR_LEN);
	sprintf (http_str, http_head_str, strlen (soap_str));

	client_send (client, http_str, strlen (http_str));
	client_send (client, soap_str, strlen (soap_str));
	return;

InvalidTimeZone:
	memset (soap_str, 0, MAX_SOAP_SIZE);
	sprintf (soap_str, "%s%s%s",
			xml_version, soap_env_head,
			"<env:Body><env:Fault>");

	strcat (soap_str,
			"<env:Code><env:Value>env:Sender</env:Value>"
			"<env:Subcode><env:Value>ter:InvalidArgVal</env:Value>"
			"<env:Subcode><env:Value>ter:InvalidTimeZone</env:Value>"
			"</env:Subcode>"
			"</env:Subcode>"
			"</env:Code>"
			"<env:Reason><env:Text xml:lang=\"en\">Invalid data</env:Text>"
			"</env:Reason>"
			"</env:Fault>"
			"</env:Body>"
			"</env:Envelope>\r\n");

	memset (http_str, 0, HTTP_TEMP_STR_LEN);
	sprintf (http_str, http_head_str, strlen (soap_str));

	client_send (client, http_str, strlen (http_str));
	client_send (client, soap_str, strlen (soap_str));
}

static void DoSetImagingSettings (char *cmdstr, char *soapstr, dvs_t * dvs, client_t * client)
{
	int res, res1;
	char tmpstr[HTTP_TEMP_STR_LEN];
	char tmpstr1[HTTP_TEMP_STR_LEN];
	char http_str[HTTP_TEMP_STR_LEN];
	char soap_str[MAX_SOAP_SIZE];
	int Brightness = 30;
	int Saturation = 30;
	int Contrast = 30;
	config_videocolor_t *videocolor = &(dvs->chunnel[0].camera.videocolor);

	memset (tmpstr, 0, HTTP_TEMP_STR_LEN);
	res = soap_get_value (soapstr, tmpstr, "ImagingSettings");
	if (res >= 0)
	{
		memset (tmpstr1, 0, HTTP_TEMP_STR_LEN);
		res1 = soap_get_value (tmpstr, tmpstr1, "Brightness");
		if (res1 >= 0)
		{
			Brightness = atoi (tmpstr1);
			videocolor->brightness = Brightness;
		}

		memset (tmpstr1, 0, HTTP_TEMP_STR_LEN);
		res1 = soap_get_value (tmpstr, tmpstr1, "ColorSaturation");
		if (res1 >= 0)
		{
			Saturation = atoi (tmpstr1);
			videocolor->saturation = Saturation;
		}

		memset (tmpstr1, 0, HTTP_TEMP_STR_LEN);
		res1 = soap_get_value (tmpstr, tmpstr1, "Contrast");
		if (res1 >= 0)
		{
			Contrast = atoi (tmpstr1);
			videocolor->contrast = Contrast;
		}

	}

	printf ("%d,%d,%d\r\n", videocolor->brightness, videocolor->saturation, videocolor->contrast);

	hal_set_brightness (dvs, 0, videocolor->brightness);
	hal_set_contrast (dvs, 0, videocolor->contrast);
	hal_set_saturation (dvs, 0, videocolor->saturation);

	config_save_videocolor (videocolor, 0);

	memset (soap_str, 0, MAX_SOAP_SIZE);
	sprintf (soap_str, "%s%s%s",
			xml_version, soap_env_head,
			"<env:Body><timg:SetImagingSettingsResponse>");

	strcat (soap_str,
			"</timg:SetImagingSettingsResponse></env:Body></env:Envelope>\r\n");

	memset (http_str, 0, HTTP_TEMP_STR_LEN);
	sprintf (http_str, http_head_str, strlen (soap_str));

	client_send (client, http_str, strlen (http_str));
	client_send (client, soap_str, strlen (soap_str));
}

static void DoSetVideoEncoderConfiguration (char *cmdstr, char *soapstr, dvs_t * dvs, client_t * client)
{
	int res, res1;
	char tmpstr[HTTP_TEMP_STR_LEN];
	char tmpstr1[HTTP_TEMP_STR_LEN];
	char http_str[HTTP_TEMP_STR_LEN];
	char soap_str[MAX_SOAP_SIZE];
	int width = 1280;
	int height = 720;
	int quality = 3;
	int framerate = 25;
	int EncodingInterval = 1;
	int bitrate = 1024;
	int govlength = 30;
	int resolution = 0;
	int resolution_changed = 0;
	//char ConfigToken[HTTP_TEMP_STR_LEN];
	int config_token = 0;
	config_videoprofile_t *videoprofile;

	// printf ("[%s]\r\n", soapstr);

	if (findStrFromBuf (soapstr, strlen (soapstr), "VideoEncoderToken_1", strlen ("VideoEncoderToken_1")) >= 0)
	{
		config_token = 1;
	}
	else if (findStrFromBuf (soapstr, strlen (soapstr), "VideoEncoderToken_2", strlen ("VideoEncoderToken_2")) >= 0)
	{
		config_token = 2;
	}

	printf ("config_token=%d\r\n", config_token);
	if (config_token == 1)
	{
		videoprofile = &(dvs->chunnel[0].camera.videoprofile);
	}
	else
	{
		videoprofile = &(dvs->chunnel[0].camera.videoprofile_small);
	}
	//  printf("[%s]\r\n", soapstr);
	memset (tmpstr, 0, HTTP_TEMP_STR_LEN);
	res = soap_get_value (soapstr, tmpstr, "SetVideoEncoderConfiguration");
	if (res >= 0)
	{
		memset (tmpstr1, 0, HTTP_TEMP_STR_LEN);
		res1 = soap_get_value (tmpstr, tmpstr1, "Width");
		if (res1 >= 0)
		{
			width = atoi (tmpstr1);
		}

		memset (tmpstr1, 0, HTTP_TEMP_STR_LEN);
		res1 = soap_get_value (tmpstr, tmpstr1, "Height");
		if (res1 >= 0)
		{
			height = atoi (tmpstr1);
			if (config_token == 1)
			{
				switch (dvs->sensor)
				{
					case OV2715:
					case MT9P031:
						if (height == 1080)
						{
							resolution = 0;
						}
						else
						{
							resolution = 1;
						}
						break;
					case HM1375:
						if (height == 960)
						{
							resolution = 0;
						}
						else
						{
							resolution = 1;
						}
						break;
					default:
						resolution = 0;
						break;
				}
			}
			else
			{
				if (height == 480)
				{
					resolution = 0;
				}
				else
				{
					resolution = 1;
				}
			}

			if (videoprofile->resolution != resolution)
			{
				resolution_changed = 1;
				videoprofile->resolution = resolution;
			}
		}
		printf("resolution=%d\r\n", resolution);
		printf("videoprofile->resolution=%d\r\n", videoprofile->resolution);

		memset (tmpstr1, 0, HTTP_TEMP_STR_LEN);
		res1 = soap_get_value (tmpstr, tmpstr1, "Quality");
		if (res1 >= 0)
		{
			quality = atoi (tmpstr1);
		}

		memset (tmpstr1, 0, HTTP_TEMP_STR_LEN);
		res1 = soap_get_value (tmpstr, tmpstr1, "FrameRateLimit");
		if (res1 >= 0)
		{
			framerate = atoi (tmpstr1);
			videoprofile->framerate = framerate;
		}

		memset (tmpstr1, 0, HTTP_TEMP_STR_LEN);
		res1 = soap_get_value (tmpstr, tmpstr1, "EncodingInterval");
		if (res1 >= 0)
		{
			EncodingInterval = atoi (tmpstr1);
		}

		memset (tmpstr1, 0, HTTP_TEMP_STR_LEN);
		res1 = soap_get_value (tmpstr, tmpstr1, "BitrateLimit");
		if (res1 >= 0)
		{
			bitrate = atoi (tmpstr1);
			videoprofile->bitrate = bitrate;
		}

		memset (tmpstr1, 0, HTTP_TEMP_STR_LEN);
		res1 = soap_get_value (tmpstr, tmpstr1, "GovLength");
		if (res1 >= 0)
		{
			govlength = atoi (tmpstr1);
		}

		if (config_token == 1)
		{
			res = SetVideoPara(videoprofile->framerate, videoprofile->bitrate,
					quality, width, height, 0);
			printf("Main ChangeChangeChangeChangeChangeChangeChangeChange. \n");
			// config_save_videoprofile (videoprofile, 0);
			// hal_adjust_enc_profile (dvs, 0);

			/** cyh add for save config */
			if (!res) {
				dvs->video_width = width;
				dvs->video_height = height;
			}
		}
		else
		{
			res = SetVideoPara(videoprofile->framerate, videoprofile->bitrate, quality,
					width, height, 1);
			printf("Small ChangeChangeChangeChangeChangeChangeChangeChange. \n");
			// config_save_videoprofile_small (videoprofile, 0);
			// hal_adjust_enc_profile_small (dvs, 0);
		}

	}

	printf ("%d,%d,%d,%d,%d,%d,%d\r\n", width, height, quality, framerate,
			EncodingInterval, bitrate, govlength);

	memset (soap_str, 0, MAX_SOAP_SIZE);
	sprintf (soap_str, "%s%s%s", xml_version, soap_env_head,
			"<env:Body><trt:SetVideoEncoderConfigurationResponse>");

	strcat (soap_str, "</trt:SetVideoEncoderConfigurationResponse></env:Body></env:Envelope>\r\n");

	memset (http_str, 0, HTTP_TEMP_STR_LEN);
	sprintf (http_str, http_head_str, strlen (soap_str));

	client_send (client, http_str, strlen (http_str));
	client_send (client, soap_str, strlen (soap_str));
	if (resolution_changed == 1)
	{
		//send_cmd (dvs, "reboot");
		//send_cmd (dvs, "reboot");
		//system("reboot");
		printf("\n\t### we do not reboot the system ###\n\n");
	}
}


#if 1
static int do_command (char *cmdstr, int cmdlen, dvs_t * dvs, client_t * client)
{
	int content_len = 0;
	int boundarylen = strlen ("\r\n\r\n");
	char tmpstr[HTTP_TEMP_STR_LEN];
	memset (tmpstr, 0, HTTP_TEMP_STR_LEN);
	char soapstr[MAX_SOAP_SIZE];
	memset (soapstr, 0, MAX_SOAP_SIZE);
	int tmplen;
	int res = get_message (cmdstr, tmpstr, "Content-Length: ", "\r\n");
	if (res == 0)
	{
		content_len = atoi (tmpstr);
		if (content_len > client->recv_len - cmdlen)
		{
			return -1;
		}
		memset (tmpstr, 0, HTTP_TEMP_STR_LEN);
		memcpy (tmpstr, client->recv_buf + cmdlen + boundarylen, content_len);
	}

	// printf("[%s]\r\n", tmpstr);

	if (findStrFromBuf
			(cmdstr, cmdlen, "jpgcap.cgi", strlen ("jpgcap.cgi")) >= 0)
	{
		DoCommandNotFind (dvs, client);
	}

	else
	{
		tmplen = strlen (tmpstr);

		if (soap_get_value (tmpstr, soapstr, "SystemReboot") >= 0)
		{
			printf ("SystemReboot\r\n");
			DoSystemReboot (cmdstr, tmpstr, dvs, client);
		}
		else if (soap_get_value (tmpstr, soapstr, "SetSystemFactoryDefault") >= 0)
		{
			printf ("SetSystemFactoryDefault\r\n");
			DoSetSystemFactoryDefault (cmdstr, tmpstr, dvs, client);
		}
		else if (soap_get_value (tmpstr, soapstr, "GetSystemDateAndTime") >= 0)
		{
			printf ("DoGetSystemDateAndTime\r\n");
			DoGetSystemDateAndTime (cmdstr, tmpstr, dvs, client);
		}
		else if (soap_get_value (tmpstr, soapstr, "GetDeviceInformation") >= 0)
		{
			printf ("DoGetDeviceInformation\r\n");
			DoGetDeviceInformation (cmdstr, tmpstr, dvs, client);
		}
		else if (soap_get_value (tmpstr, soapstr, "SetScopes") >= 0)
		{
			printf ("DoSetScopes\r\n");
			DoSetScopes (cmdstr, tmpstr, dvs, client);
		}
		else if (soap_get_value (tmpstr, soapstr, "GetScopes") >= 0)
		{
			printf ("DoGetScopes\r\n");
			DoGetScopes (cmdstr, tmpstr, dvs, client);
		}
		else if (soap_get_value (tmpstr, soapstr, "GetCapabilities") >= 0)
		{
			printf ("DoGetCapabilities\r\n");
			DoGetCapabilities (cmdstr, tmpstr, dvs, client);
		}
		else if (soap_get_value (tmpstr, soapstr, "GetNetworkInterfaces") >= 0)
		{
			printf ("DoGetNetworkInterfaces\r\n");
			DoGetNetworkInterfaces (cmdstr, tmpstr, dvs, client);
		}
		else if (soap_get_value (tmpstr, soapstr, "GetDNS") >= 0)
		{
			printf ("DoGetDNS\r\n");
			DoGetDNS (cmdstr, tmpstr, dvs, client);
		}
		else if (soap_get_value (tmpstr, soapstr, "SetDNS") >= 0)
		{
			printf ("DoSetDNS\r\n");
			DoSetDNS (cmdstr, tmpstr, dvs, client);
		}
		else if (soap_get_value (tmpstr, soapstr, "GetServices") >= 0)
		{
			printf ("DoGetServices\r\n");
			DoGetServices (tmpstr, tmpstr, dvs, client);
		}
		else if (soap_get_value (tmpstr, soapstr, "wsdlGetVideoSources") >= 0)
		{
			printf ("DowsdlGetVideoSources\r\n");
			DowsdlGetVideoSources (cmdstr, tmpstr, dvs, client);
		}
		else if (soap_get_value (tmpstr, soapstr, "GetVideoSources") >= 0)
		{
			printf ("DoGetVideoSources\r\n");
			DoGetVideoSources (cmdstr, tmpstr, dvs, client);
		}
		else if (soap_get_value (tmpstr, soapstr, "GetProfiles") >= 0)
		{
			printf ("DoGetProfiles\r\n");
			DoGetProfiles (cmdstr, tmpstr, dvs, client);
		}
		else if (soap_get_value (tmpstr, soapstr, "GetProfile") >= 0)
		{
			printf ("DoGetProfile\r\n");
			DoGetProfile (cmdstr, tmpstr, dvs, client);
		}
		else if (soap_get_value (tmpstr, soapstr, "GetSnapshotUri") >= 0)
		{
			printf ("DoGetSnapshotUri\r\n");
			DoGetSnapshotUri (cmdstr, tmpstr, dvs, client);
		}
		else if (soap_get_value (tmpstr, soapstr, "GetStreamUri") >= 0)
		{
			printf ("DoGetStreamUri\r\n");
			DoGetStreamUri (cmdstr, tmpstr, dvs, client);
		}
		else if (soap_get_value (tmpstr, soapstr, "GetVideoSourceConfigurationOptions") >= 0)
		{
			printf ("DoGetVideoSourceConfigurationOptions\r\n");
			DoGetVideoSourceConfigurationOptions (cmdstr, tmpstr, dvs, client);
		}
		else if (soap_get_value (tmpstr, soapstr, "GetVideoSourceConfigurations") >= 0)
		{
			printf ("DoGetVideoSourceConfigurations\r\n");
			DoGetVideoSourceConfigurations (cmdstr, tmpstr, dvs, client);
		}
		else if (soap_get_value (tmpstr, soapstr, "GetVideoSourceConfiguration") >= 0)
		{
			printf ("DoGetVideoSourceConfiguration\r\n");
			DoGetVideoSourceConfiguration (cmdstr, tmpstr, dvs, client);
		}
		else if (soap_get_value (tmpstr, soapstr, "GetVideoEncoderConfigurations") >= 0)
		{
			printf ("DoGetVideoEncoderConfigurations\r\n");
			DoGetVideoEncoderConfigurations (cmdstr, tmpstr, dvs, client);
		}
		else if (soap_get_value (tmpstr, soapstr, "GetVideoEncoderConfigurationOptions") >= 0)
		{
			printf ("DoGetVideoEncoderConfigurationOptions\r\n");
			DoGetVideoEncoderConfigurationOptions (cmdstr, tmpstr, dvs, client);
		}
		else if (soap_get_value (tmpstr, soapstr, "GetVideoEncoderConfiguration") >= 0)
		{
			printf ("DoGetVideoEncoderConfiguration\r\n");
			DoGetVideoEncoderConfiguration (cmdstr, tmpstr, dvs, client);
		}
		else if (soap_get_value (tmpstr, soapstr, "GetCompatibleVideoEncoderConfigurations") >= 0)
		{
			printf ("DoGetCompatibleVideoEncoderConfigurations\r\n");
			DoGetCompatibleVideoEncoderConfigurations (cmdstr, tmpstr, dvs,
					client);
		}
		else if (soap_get_value (tmpstr, soapstr, "GetZeroConfiguration") >= 0)
		{
			printf ("DoGetZeroConfiguration\r\n");
			DoGetZeroConfiguration (cmdstr, tmpstr, dvs, client);
		}
		else if (soap_get_value (tmpstr, soapstr, "GetMoveOptions") >= 0)
		{
			printf ("DoGetMoveOptions\r\n");
			DoGetMoveOptions (cmdstr, tmpstr, dvs, client);
		}
		else if (soap_get_value (tmpstr, soapstr, "GetServiceCapabilities") >= 0)
		{
			printf ("DoGetServiceCapabilities\r\n");
			DoGetServiceCapabilities (cmdstr, tmpstr, dvs, client);
		}
		else if (soap_get_value (tmpstr, soapstr, "GetNTP") >= 0)
		{
			printf ("DoGetNTP\r\n");
			DoGetNTP (cmdstr, tmpstr, dvs, client);
		}
		else if (soap_get_value (tmpstr, soapstr, "SetNTP") >= 0)
		{
			printf ("DoSetNTP\r\n");
			DoSetNTP (cmdstr, tmpstr, dvs, client);
		}
		else if (soap_get_value (tmpstr, soapstr, "GetNetworkDefaultGateway") >= 0)
		{
			printf ("DoGetNetworkDefaultGateway\r\n");
			DoGetNetworkDefaultGateway (cmdstr, tmpstr, dvs, client);
		}
		else if (soap_get_value (tmpstr, soapstr, "GetHostname") >= 0)
		{
			printf ("DoGetHostname\r\n");
			DoGetHostname (cmdstr, tmpstr, dvs, client);
		}
		/*else if (soap_get_value (tmpstr, soapstr, "SetHostname") >= 0)
		  {
		  printf ("DoSetHostname\r\n");
		  DoSetHostname (cmdstr, tmpstr, dvs, client);
		  }*/
		else if (soap_get_value (tmpstr, soapstr, "GetNetworkProtocols") >= 0)
		{
			printf ("DoGetNetworkProtocols\r\n");
			DoGetNetworkProtocols (cmdstr, tmpstr, dvs, client);
		}
		else if (soap_get_value (tmpstr, soapstr, "GetUsers") >= 0)
		{
			printf ("DoGetUsers\r\n");
			DoGetUsers (cmdstr, tmpstr, dvs, client);
		}
		else if (soap_get_value (tmpstr, soapstr, "GetWsdlUrl") >= 0)
		{
			printf ("DoGetWsdlUrl\r\n");
			DoGetWsdlUrl (cmdstr, tmpstr, dvs, client);
		}
		else if (soap_get_value (tmpstr, soapstr, "GetCompatibleVideoSourceConfigurations") >= 0)
		{
			printf ("GetCompatibleVideoSourceConfigurations\r\n");
			DoGetCompatibleVideoSourceConfigurations (cmdstr, tmpstr, dvs, client);
		}
		else if (soap_get_value (tmpstr, soapstr, "GetDiscoveryMode") >= 0)
		{
			printf ("GetDiscoveryMode\r\n");
			DoGetDiscoveryMode (cmdstr, tmpstr, dvs, client);
		}
		else if (soap_get_value (tmpstr, soapstr, "GetImagingSettings") >= 0)
		{
			printf ("GetImagingSettings\r\n");
			DoGetImagingSettings (cmdstr, tmpstr, dvs, client);
		}
		else if (soap_get_value (tmpstr, soapstr, "GetOptions") >= 0)
		{
			printf ("GetOptions\r\n");
			DoGetOptions (cmdstr, tmpstr, dvs, client);
		}
		else if (soap_get_value (tmpstr, soapstr, "SetNetworkInterfaces") >= 0)
		{
			printf ("SetNetworkInterfaces\r\n");
			DoSetNetworkInterfaces (cmdstr, tmpstr, dvs, client);
		}
		else if (soap_get_value (tmpstr, soapstr, "SetNetworkDefaultGateway") >= 0)
		{
			printf ("SetNetworkDefaultGateway\r\n");
			DoSetNetworkDefaultGateway (cmdstr, tmpstr, dvs, client);
		}
		else if (soap_get_value (tmpstr, soapstr, "SetHostname") >= 0)
		{
			printf ("SetHostname\r\n");
			DoSetHostname (cmdstr, tmpstr, dvs, client);
		}
		else if (soap_get_value (tmpstr, soapstr, "SetSystemDateAndTime") >= 0)
		{
			printf ("SetSystemDateAndTime\r\n");
			DoSetSystemDateAndTime (cmdstr, tmpstr, dvs, client);
		}
		else if (soap_get_value (tmpstr, soapstr, "SetImagingSettings") >= 0)
		{
			printf ("SetImagingSettings\r\n");
			DoSetImagingSettings (cmdstr, tmpstr, dvs, client);
		}
		else if (soap_get_value (tmpstr, soapstr, "SetVideoEncoderConfiguration") >= 0)
		{
			printf ("SetVideoEncoderConfiguration\r\n");
			DoSetVideoEncoderConfiguration (cmdstr, tmpstr, dvs, client);
		}
		else
		{
			printf ("[%s]\r\n", cmdstr);
			printf ("{%s}\r\n", tmpstr);
			DoCommandNotFind (dvs, client);
		}

		usleep(100000);
		pthread_mutex_lock (&(client->mutex_send));
		client->used = -1;
		close(client->sock_fd);
		client->sock_fd = -1;
		printf ("client disconnected\r\n");
		pthread_mutex_unlock (&(client->mutex_send));

	}
	return 0;
}
#endif

static void client_recv (dvs_t * dvs, client_t * client)
{
	int pos;
	int length;
	char tmpstr[HTTP_CMDSIZE];
	int boundarylen = strlen ("\r\n\r\n");
	fd_set rdfds;
	struct timeval tv;
	int ret, res;
	//FILE *fp ;

	if ((client->used == 1) && (client->sock_fd != -1))
	{
		FD_ZERO (&rdfds);
		FD_SET (client->sock_fd, &rdfds);
		tv.tv_sec = 2;
		tv.tv_usec = 0;

		ret = select (client->sock_fd + 1, &rdfds, NULL, NULL, &tv);
		if (ret == 1)
		{
			length = recv (client->sock_fd, client->recv_buf + client->recv_len, HTTP_RECV_BUF_SIZE - client->recv_len, 0);
			if (length <= 0)
			{
				pthread_mutex_lock (&(client->mutex_send));
				client->used = -1;
				close (client->sock_fd);
				client->sock_fd = -1;
				printf ("client disconnected\r\n");
				pthread_mutex_unlock (&(client->mutex_send));
			}
			else
			{
				client->recv_len += length;
				while ((pos = findStrFromBuf ((char *)client->recv_buf, client->recv_len, "\r\n\r\n", boundarylen)) > 0)
				{
					memset (tmpstr, 0, HTTP_CMDSIZE);
					memcpy (tmpstr, client->recv_buf, pos + 4);
					res = do_command (tmpstr, pos, dvs, client);
					if (res == 0)
					{
						client->recv_len = 0;
					}
					else
					{
						break;
					}
				}
			}
		}
		else if (ret < 0)
		{
			pthread_mutex_lock (&(client->mutex_send));
			client->used = -1;
			pthread_mutex_unlock (&(client->mutex_send));
		}
	}
}

void * http_client_recv (void *data)
{
	dvs_params_t *params;
	dvs_t *dvs;
	int index;
	client_t *client;

	params = (dvs_params_t *) data;
	dvs = params->dvs;
	index = params->index;
	pthread_mutex_unlock (&(params->mutex));

	client = (client_t *) & (dvs->http_client[index]);
	client->recv_len = 0;
	while (1)
	{
		client_recv (dvs, client);
		usleep (100000);
	}
	return (void *) NULL;
}

void * http_client_listen (void *data)
{
	int i;
	int fd;
	int addrlen;
	int opt = 1;
	//int length;
	int clientfd;
	struct sockaddr_in addr;
	struct sockaddr_in client_addr;

	dvs_t *dvs = (dvs_t *) data;

	fd = socket (AF_INET, SOCK_STREAM, IPPROTO_IP);
	if (fd < 0)
	{
		printf ("http_client_listen:socket error.\r\n");
		return (void *) NULL;
	}
	setsockopt (fd, SOL_SOCKET, SO_REUSEADDR, (char *) &opt, sizeof (opt));

	addr.sin_family = AF_INET;
	addr.sin_port = htons (HTTP_LISTEN_PORT);
	addr.sin_addr.s_addr = htonl (INADDR_ANY);
	memset (&(addr.sin_zero), 0, sizeof (addr.sin_zero));
	if (bind (fd, (struct sockaddr *) &addr, sizeof (addr)) != 0)
	{
		printf ("http_client_listen:Binding error.\r\n");
		close (fd);
		return (void *) NULL;
	}
	if (listen (fd, 100) != 0)
	{
		printf ("http_client_listen:Listen Error.\r\n");
	}
	printf ("http_client_listen:Listen to %d\r\n", HTTP_LISTEN_PORT);

	while (1)
	{
		addrlen = sizeof (client_addr);
		clientfd = accept (fd, (struct sockaddr *) &client_addr, (socklen_t *) &addrlen);
		if (clientfd != -1)
		{
			printf ("Client %s is connected.\r\n", inet_ntoa (client_addr.sin_addr));
			for (i = 0; i < MAX_HTTP_CLIENT_NUM; i++)
			{
				if (dvs->http_client[i].used < 0)
				{
					dvs->http_client[i].used++;
				}
			}
			for (i = 0; i < MAX_HTTP_CLIENT_NUM; i++)
			{
				if (dvs->http_client[i].used == 0)
				{
					printf ("dvs->client[%d].used=1,clientfd=%d\r\n", i, clientfd);

					dvs->http_client[i].used = 1;
					dvs->http_client[i].timeout = 0;
					dvs->http_client[i].sock_fd = clientfd;
					dvs->client[i].recv_pos = 0;
					dvs->http_client[i].recv_len = 0;
					dvs->http_client[i].send_pos = 0;
					dvs->http_client[i].send_len = 0;
					dvs->http_client[i].logined = 0;
					break;
				}
			}
		}
	}
	return (void *) NULL;
}

