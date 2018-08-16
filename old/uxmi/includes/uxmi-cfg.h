/* uxmi config structures */

/* config is broken down in a hierarchy :

	system
	network	
	security
	auth
	asl

 */

#define UXS_HOSTNAME_MAXLEN	 128;

#define UXN_MAX_IFACE		  64;
#define UXN_MAX_GW		  16;

#define UXSEC_MAX_SECTTY	  16;

#define UXA_MAX_USERS		  64;

#define UXASL_MAX_ROLES		   4;

typedef struct {
	char hostname[UXS_HOSTNAME_MAXLEN];
	cfg_dns_t dns;
	cfg_mgmt_t mgmt;
	cfg_sched_t scheduler;
} cfg_sys_t;

typedef struct {
	cfg_iface_t iface[UXN_MAX_IFACE];
	cfg_gw_t gw[UXN_MAX_GW];
} cfg_net_t;

typedef struct {
	char sectty[UXSEC_MAX_SECTTY];
} cfg_sec_t;

typedef struct {
	cfg_user_t users[UXA_MAX_USERS];
} cfg_auth_t;

typedef struct {
	cfg_asl_load[UXASL_MAX_ROLES];
} cfg_asl_t;


typedef struct {
	char domain[UXS_HOSTNAME_MAXLEN];
	char nameserver[UXS_HOSTNAME_MAXLEN][6];
	char search[UXS_HOSTNAME_MAXLEN][6];
} cfg_dns_t;

typedef struct {
	u8 sshd
} cfg_mgmt_t;
