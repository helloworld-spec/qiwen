#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <net/route.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <dirent.h>
#include <stddef.h>

#include "ak_common.h"

#define ifreq_offset(x) offsetof(struct ifreq, x)

/* arguments options */
struct arg1opt {
	const char *name;	//cmd name
	unsigned short selector;	//cmd magic
	unsigned short ifr_offset;	//struct offset
};

static const char net_version[] = "libapp_net V1.0.00";

/* current use network set and get argument */
static struct arg1opt net_opt_arg[] = {
	/* get */
	{ "GIFADDR",    SIOCGIFADDR, 	ifreq_offset(ifr_addr) },	 //get addr
	{ "GIFNETMASK", SIOCGIFNETMASK, ifreq_offset(ifr_netmask) }, //get netmask
	/* set */
	{ "SIFADDR", 	SIOCSIFADDR, 	ifreq_offset(ifr_addr) },	//set addr
	{ "SIFNETMASK", SIOCSIFNETMASK, ifreq_offset(ifr_netmask) },//set netmask
};

const char* ak_net_get_version(void)
{
	return net_version;
}

/**
 * net_get - get addr or netmask
 * @iface[IN]: interface name
 * @opt[IN]: option addr or netnask
 * @result[OUT]: store result
 * return: 0 success; -1 failed
 */
static int net_get(const char *iface, const char *opt, char *result)
{
	if (!iface || !opt || !result) {
		ak_print_error_ex("Invalid argument\n");
		return AK_FAILED;
	}

    int ret = AK_FAILED;
	int sockfd = -1;
	unsigned char match_flag = AK_FALSE;
	struct arg1opt *argp = NULL;

	/* find option */
	for (argp = net_opt_arg; argp->name; argp++) {
		if (!strcmp(opt, argp->name)) {
		    match_flag = AK_TRUE;
		    break;
		}
	}

	if (match_flag) {
	    /* create socket */
    	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    	if (sockfd < 0) {
    		ak_print_error_ex("socket: %s\n", strerror(errno));
    		goto net_get_end;
    	}

    	/* variable define */
    	struct ifreq ifr;
    	struct sockaddr_in s_addr;

    	/* set interface name */
    	strncpy(ifr.ifr_name, iface, IFNAMSIZ);
    	ifr.ifr_name[IFNAMSIZ - 1] = 0;

    	/* do get by ioctl */
    	if (ioctl(sockfd, argp->selector, &ifr)) {
    		ak_print_error_ex("ioctl: SIOC%s, %s, iface: %s\n",
    				argp->name, strerror(errno), iface);
    		goto net_get_end;
    	}

    	/* clean out s_addr */
    	memset(&s_addr, 0, sizeof(s_addr));
    	/* store result */
    	memcpy(&s_addr, ((char *)&ifr) + argp->ifr_offset, sizeof(s_addr));
    	/* format convert */
    	char *addr = inet_ntoa(s_addr.sin_addr);
    	if (result)
    		strcpy(result, addr);

        ret = AK_SUCCESS;
    	ak_print_debug_ex("get addr: %s\n", addr);
	} else {
	    ak_print_error_ex("unsupport opt: %s\n", opt);
	}

net_get_end:
	if (sockfd != -1)
		close(sockfd);

	return ret;
}

/**
 * net_set - set addr or netmask
 * @iface[IN]: interface name
 * @opt[IN]: option addr or netnask
 * @arg[IN]: set value
 * return: 0 on success, -1 failed
 */
static int net_set(const char *iface, const char *opt, const char *arg)
{
	if (!iface || !opt || !arg) {
		ak_print_error_ex("Invalid argument\n");
		return AK_FAILED;
	}

    int ret = AK_FAILED;
	int sockfd = -1;
    unsigned char match_flag = AK_FALSE;
	struct arg1opt *argp = NULL;

	/* find option */
	for (argp = net_opt_arg; argp->name; argp++) {
		if (!strcmp(opt, argp->name)) {
            match_flag = AK_TRUE;
            break;
		}
	}

	if (match_flag) {
	    /* create socket */
    	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    	if (sockfd < 0) {
    		ak_print_error_ex("socket: %s\n", strerror(errno));
    		goto net_set_end;
    	}

    	/* variable define */
    	struct ifreq ifr;
    	struct sockaddr_in s_addr;

    	memset(&s_addr, 0, sizeof(s_addr));
    	/* check whether arg is valid */
    	ret = inet_aton(arg, &s_addr.sin_addr);
    	if (!ret) {
    		ak_print_error_ex("invalid arg: %s\n", arg);
    		goto net_set_end;
    	}

    	/* set socket struct */
    	s_addr.sin_port = 0;
    	s_addr.sin_family = AF_INET;

    	/* set interface name */
    	strncpy(ifr.ifr_name, iface, IFNAMSIZ);
    	ifr.ifr_name[IFNAMSIZ - 1] = 0;

    	memcpy(((char *)&ifr) + argp->ifr_offset, &s_addr,
    	    sizeof(struct sockaddr));

    	/* do set by ioctl */
    	if (ioctl(sockfd, argp->selector, &ifr)) {
    		ak_print_error_ex("ioctl: SIOC%s, %s, iface: %s\n",
    				argp->name, strerror(errno), iface);
    	} else {
            ret = AK_SUCCESS;
    	    ak_print_debug_ex("set addr: %s ok\n", arg);
    	}
	} else {
        ak_print_error_ex("unsupport opt: %s\n", opt);
	}

net_set_end:
	if (sockfd != -1)
		close(sockfd);

	return ret;
}

/**
 * ak_net_get_ip - get ipaddr
 * @iface_name[IN]: interface name
 * @result[OUT]: store result
 * return: 0 success, -1 failed
 */
int ak_net_get_ip(const char *iface_name, char *result)
{
	return net_get(iface_name, "GIFADDR", result);
}

/**
 * ak_net_set_ip - set current system working ip
 * @iface[IN]: interface name
 * @ip[OUT]: ip
 * return: 0 success, -1 failed
 */
int ak_net_set_ip(const char *iface, const char *ip)
{
	return net_set(iface, "SIFADDR", ip);
}

/**
 * ak_net_get_netmask - get netmask
 * @iface_name[IN]: interface name
 * @result[OUT]: store result
 * return: 0 success, -1 failed
 */
int ak_net_get_netmask(const char *iface_name, char *result)
{
	return net_get(iface_name, "GIFNETMASK", result);
}

/*
 * ak_net_set_netmask - set current system working subnet mask
 * @iface[IN]: interface name
 * @netmask[OUT]: subnet mask
 * return: 0 success, -1 failed
 */
int ak_net_set_netmask(const char *iface, const char *netmask)
{
	return net_set(iface, "SIFNETMASK", netmask);
}

/**
 * ak_net_get_dns - get dns
 * @flag[IN]: first/second switch
 * 			 flag = 0, first dns
 * 			 flag = 1, second dns
 * @result: DNS info
 * return: 0 success, -1 failed
 */
int ak_net_get_dns(int flag, char *result)
{
	FILE *f = fopen("/etc/resolv.conf", "r");
	if (!f) {
		ak_print_error_ex("fopen: %s\n", strerror(errno));
		return AK_FAILED;
	}

	int ret = AK_FAILED;
	int match = 0;
	char *pdns = NULL;
	char dns[32] = {0}, line[1024] = {0};

	while (!feof(f)) {
		memset(line, 0x00, sizeof(line));
		pdns = fgets(line, sizeof(line), f);
		if (!pdns)
			goto get_dns_end;

		/* skip invalid characters */
		if (!strstr(pdns, "nameserver")) {
			ak_print_normal_ex("skip invalid line, %s\n", pdns);
			continue;
		} else {
			ak_print_info_ex("get dns info: %s\n", pdns);
			break;
		}
	}

	/* main */
	switch (flag) {
	case 0:
	    match = sscanf(line, "%*s %s", dns);
	    break;
	case 1:
	    memset(line, 0x00, sizeof(line));
		pdns = fgets(line, 1024, f);
		match = 0;
		if (!pdns) /* no new line */
			goto get_dns_end;
		match = sscanf(line, "%*s %s", dns);
	    break;
	default:
	    break;
	}

get_dns_end:
	fclose(f);
	if (1 == match) {
	    ret = AK_SUCCESS;
		pdns = dns;
		if (result)
			strcpy(result, pdns);
	}

	return ret;
}

/**
 * ak_net_get_route - get default route
 * @iface_name[IN]: interface name
 * @result[OUT]: store result
 * return: 0 success, -1 failed
 */
int ak_net_get_route(const char *iface_name, char *result)
{
	if (!iface_name) {
		ak_print_error_ex("Invalid argument\n");
		return AK_FAILED;
	}

	FILE *f = fopen("/proc/net/route", "r");
	if (!f) {
		ak_print_error_ex("fopen: %s\n", strerror(errno));
		return AK_FAILED;
	}

	char line[1024] = {0}, iface[20] = {0}, route[64] = {0};
	/* strip first identification */
	fgets(line, sizeof(line), f);

	/* get second line */
	memset(line, 0x00, sizeof(line));
	fgets(line, sizeof(line), f);

	int ret = sscanf(line, "%s %*s %s", iface, route);
	if (ret != 2)
		ak_print_error_ex("match failed\n");
	ak_print_info("route: %s\n", route);
	fclose(f);

	/* compare iface name */
	if (strncmp(iface, iface_name, strlen(iface_name)) != 0) {
		ak_print_error_ex("iface: %s, iface_name: %s\n", iface, iface_name);
		return AK_FAILED;	/* un match interface name */
	}

	long long int val;
	char *endptr;

	val = strtoll(route, &endptr, 16);
	if ((errno == ERANGE && (val == LONG_MAX || val == LONG_MIN))
			|| (errno != 0 && val == 0)) {
		ak_print_error_ex("strtoll: %s, route: %s\n", strerror(errno), route);
		return AK_FAILED;
	}
	if (endptr == route) {
		ak_print_error_ex("NO digits were found\n");
		return AK_FAILED;
	}
	if (*endptr != '\0')
		ak_print_error_ex("Further characters after number: %s\n", endptr);

	struct in_addr inp = {.s_addr = val};
	char *proute = inet_ntoa(inp);

	if (proute && result) {
		strcpy(result, proute);
	}

	return AK_SUCCESS;
}

/**
 * ak_net_get_cur_iface - get current system working net interface name
 * @iface[OUT]: store interface name
 * return: 0 success, -1 failed
 */
int ak_net_get_cur_iface(char *iface)
{
	const char *sys_net_entry = "/sys/class/net/";
	DIR *dir = opendir(sys_net_entry);

	struct dirent dp, *dp_result;
	int ret = AK_FAILED;
	char ip[16] = {0};

	do {
		ret = readdir_r(dir, &dp, &dp_result);
		/* if read success, and dp is not . or .. , do test */
		if (!ret && strcmp(dp.d_name, ".") && strcmp(dp.d_name, "..") &&
				strcmp(dp.d_name, "lo")) {
			bzero(ip, 16);
			ak_net_get_ip(dp.d_name, ip);
			if (strlen(ip)) {
				if (iface)
					strncpy(iface, dp.d_name, strlen(dp.d_name));
				ak_print_info_ex("current use iface: %s\n", dp.d_name);
				ret = AK_SUCCESS;
			}
		}
	} while ((!ret) && (dp_result != NULL));

	closedir(dir);

	return ret;
}

/**
 * ak_net_get_mac - get MAC address
 * @iface[IN]: interface name
 * @mac[OUT]: store mac on success
 * @len[IN]: lenght of buf mac
 * return: 0 success, -1 failed
 */
int ak_net_get_mac(const char *iface, char *mac, int len)
{
	if (!mac || len < 18) {
		ak_print_error_ex("invalid arg: macaddr: %p, len: %d\n", mac, len);
		return AK_FAILED;
	}

	/* create socket to INET kernel */
	int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sockfd == -1) {
		ak_print_error_ex("socket: %s\n", strerror(errno));
		return AK_FAILED;
	}

	/* variable define */
	int ret = AK_FAILED;
	struct ifreq req;

	/* clean up req and set interface name */
	memset(&req, 0x00, sizeof(req));
	strncpy(req.ifr_name, iface, IFNAMSIZ - 1);

	/* do get mac addr */
	if (ioctl(sockfd, SIOCGIFHWADDR, &req) >= 0) {
	    ret = AK_SUCCESS;
		snprintf(mac, len, "%02x:%02x:%02x:%02x:%02x:%02x",
				(unsigned char)req.ifr_hwaddr.sa_data[0],
				(unsigned char)req.ifr_hwaddr.sa_data[1],
				(unsigned char)req.ifr_hwaddr.sa_data[2],
				(unsigned char)req.ifr_hwaddr.sa_data[3],
				(unsigned char)req.ifr_hwaddr.sa_data[4],
				(unsigned char)req.ifr_hwaddr.sa_data[5]);
		ak_print_info_ex("mac addr: %s\n", mac);
	}

	/* clean up resource */
	if (sockfd != -1)
		close(sockfd);

	return ret;
}

/*
 * ak_net_set_default_route - set current system default route
 * gateway[IN], default gateway
 * return: 0 success, -1 failed
 */
int ak_net_set_default_gateway(const char *gateway)
{
	if (!gateway) {
		ak_print_error_ex("invalid args\n");
		return AK_FAILED;
	}

	/*
	 * It use two step:
	 * 1. delete old default gateway
	 * 2. add new gateway for default
	 */
	int ret = AK_FAILED;
	int sockfd = -1;
	char rt_buf[sizeof(struct rtentry)] = {0};
	struct rtentry *const rt = (void *)rt_buf;

	/* default destination addr */
	struct sockaddr_in *sin = (struct sockaddr_in *)&rt->rt_dst;
	sin->sin_addr.s_addr = INADDR_ANY;
	sin->sin_port = 0;
	sin->sin_family = AF_INET;

	/* set gateway addr */
	sin = (struct sockaddr_in *)&rt->rt_gateway;
	sin->sin_port = 0;
	sin->sin_family = AF_INET;
	/* check whether gateway is valid */
	if (!inet_aton(gateway, &(sin->sin_addr))) {
		ak_print_error_ex("invalid gateway addr: %s\n", gateway);
		goto set_gateway_end;
	}

	/* set route flags */
	rt->rt_flags |= (RTF_UP | RTF_GATEWAY);

	/* create socket to INET kernel */
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sockfd < 0) {
		ak_print_error_ex("socket: %s\n", strerror(errno));
		goto set_gateway_end;
	}

	/* do delete route */
	int r = ioctl(sockfd, SIOCDELRT, rt);
	if (r != 0 && errno != ESRCH) {
		ak_print_error_ex("SIOCDELRT failed: %s\n", strerror(errno));
		goto set_gateway_end;
	}

	/* do add route */
	if (ioctl(sockfd, SIOCADDRT, rt)) {
		ak_print_error_ex("SIOCADDRT failed: %s\n", strerror(errno));
	} else {
	    ret = AK_SUCCESS;
	}

set_gateway_end:
	if (sockfd != -1)
		close(sockfd);

	return ret;
}
