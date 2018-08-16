/*
 * kattach (kernel attach)
 * Copyright (c) 2009 - 2010 Carbon Mountain LLC.
 * All Rights Reserved.
 *
 * John Buswell <buswellj@carbonmountain.com>
 * version 0.6.0
 *
 * LICENSE: GPL v2
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 *
 * To contact Carbon Mountain LLC please visit http://www.carbonmountain.com
 *
 * Source File :		thedawn.c
 * Purpose     :		early stage system initialization
 * Callers     :		main() in kattach.c
 *
 */

#include <stdio.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/mount.h>
#include <linux/fs.h>
#include <sys/time.h>
#include <time.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include "kattach_types.h"
#include "kattach_shm.h"
#include "kattach.h"

void
kattach_thedawn(void)
{
	FILE *stream;
	int ret = 0;
	kattach_cfg_t *cPtr = &kattach_cfg;
	kattach_vns_t *vPtr = &kattach_vns;
	kattach_cfggrp_t *gPtr = &kattach_cfggrp;
	kattach_fw_t *fPtr = &kattach_fw;

	/* initialize some global variables */
	kattach_reboot_me = 0;
	kattach_setup = 0;
	kattach_macfound = 0;
	kattach_dbactive = 0;
	kattach_change_detected = 0;
	kattach_recovery = 0;
        kattach_gettypid[0] = 0;
        kattach_gettypid[1] = 0;
        kattach_gettypid[2] = 0;
        kattach_gettypid[3] = 0;
	kattach_sshdpid = 0;
	kattach_hasdhcpif = 0;
	kattach_ntpnext = 0;
	kattach_ntptime = 0;
	kattach_ntppid = 0;
	kattach_dbtmpindex = 0;

	/* initialize the config memory area */
	memset(cPtr,0,sizeof(kattach_cfg));
	memset(vPtr,0,sizeof(kattach_vns));
	memset(gPtr,0,sizeof(kattach_cfggrp));
	memset(fPtr,0,sizeof(kattach_fw));

	/* Very early userspace -- do just enough */
	ret = mkdir(KATTACH_PROCPATH, KATTACH_PERM_PROC);
	ret = mount(KATTACH_PROC, KATTACH_PROCPATH, KATTACH_PROC, MS_RELATIME, KATTACH_FS_DEFAULT);

	/* open the console so we can see what we are doing */
	kattach_thedawn_console();

	/* generate the dev tree */
	kattach_thedawn_generate_dev();

	/* create some other directories */
	ret = mkdir(KATTACH_SSHKEYPATH, KATTACH_PERM_SECURE);
	ret = mkdir(KATTACH_VARRUNPATH, KATTACH_PERM);
	ret = mkdir(KATTACH_VARDBPATH, KATTACH_PERM);
	ret = mkdir(KATTACH_VARLOGPATH, KATTACH_PERM);
	ret = mkdir(KATTACH_SYSPATH, KATTACH_PERM);
	ret = mkdir(KATTACH_ROOTHOME, KATTACH_PERM_SECURE);

	/* get the kernel boot_id */
	kattach_getbuuid();

	/* write out config files */
	kattach_thedawn_cfg_write();

	/* compress */
	kattach_thedawn_cmfs();

#if !defined(KATTACH_BLD_VKAOS)
	/* initialize shared memory */
	kattach_thedawn_shm();

	/* create dhcpd.leases file */
	stream = fopen(KATTACH_CONF_DHCPD_LEASES,"w");
	if (stream != (FILE *)0) {
		fclose(stream);
	}
#endif /* !defined(KATTACH_BLD_VKAOS) */

	/* create lastlog file */
	stream = fopen(KATTACH_VARLASTLOGPATH,"w");
	if (stream != (FILE *)0) {
		fclose(stream);
	}	

	return;
}

void
kattach_thedawn_console(void)
{
	char lcmd[255];
#if !defined(KATTACH_DEBUG)
	fd_con = open(KATTACH_CONSOLE, O_RDONLY | O_NONBLOCK);
#endif /* !defined(KATTACH_DEBUG) */

	/* set the panic timer, just in case something goes south */
	sprintf(lcmd,"%s%s %u > %s",KATTACH_BINPATH,KCMD_ECHO,KATTACH_PANICTIMER,KATTACH_PANICPATH);
	kattach_sysexec(lcmd);
	memset(lcmd,0,sizeof(lcmd));

	sprintf(lcmd,"%s%s",KATTACH_BINPATH,KCMD_CLEAR);
	kattach_sysexec(lcmd);

	printf("\n\n");
	printf("kattach %s-%s (%s)\n",KATTACH_VERSION, KATTACH_RELEASE, KATTACH_ARCH);
	printf("%s\n\n",KATTACH_COPYRIGHT);
	return;
}

void
kattach_thedawn_generate_dev()
{
	int ret = 0;

	ret = mkdir(KATTACH_DEVPATH, KATTACH_PERM_DEV);
	ret = mkdir(KATTACH_DEVSHMPATH, KATTACH_PERM_DEV);
	ret = mkdir(KATTACH_DEVPTSPATH, KATTACH_PERM_DEV);
	ret = mkdir(KATTACH_DEVNETPATH, KATTACH_PERM_DEV);
	ret = mkdir(KATTACH_DEVMISCPATH, KATTACH_PERM_DEV);


	kattach_thedawn_init_devlist();
	kattach_thedawn_devlist_add();

	ret = mount(KATTACH_DEVPTS, KATTACH_DEVPTSPATH, KATTACH_DEVPTS, MS_RELATIME, KATTACH_FS_DEVPTS);
	ret = mount(KATTACH_TMPFS, KATTACH_DEVSHMPATH, KATTACH_TMPFS, MS_RELATIME, "");

	ret = symlink(KATTACH_DEVLINK_FD_TGT,KATTACH_DEVLINK_FD);
	ret = symlink(KATTACH_DEVLINK_STDIN_TGT,KATTACH_DEVLINK_STDIN);
	ret = symlink(KATTACH_DEVLINK_STDOUT_TGT,KATTACH_DEVLINK_STDOUT);
	ret = symlink(KATTACH_DEVLINK_STDERR_TGT,KATTACH_DEVLINK_STDERR);
	return;
}

void
kattach_thedawn_init_devlist()
{
	kattach_dev_t *dPtr = &kattach_devices;
	memset(dPtr,0,sizeof(kattach_devices));
	return;
}

void
kattach_thedawn_devlist_build(char *devname, int major, int minor, u16 perm)
{
	dev_t new_kadev = makedev(major,minor);
	int ret = mknod(devname,perm,new_kadev);
        u32 index = kattach_devices.index;

	kattach_devices.device[index].res = (u16) ret;
	kattach_devices.device[index].major = major;
	kattach_devices.device[index].minor = minor;
	kattach_devices.device[index].devtype = perm;
	sprintf(kattach_devices.device[index].devname,"%s",devname);
	kattach_devices.index++;

	return;
}

void
kattach_thedawn_devlist_add()
{
	int x = 0;
	char ctmp[64];

	/* misc character devices */
	kattach_thedawn_devlist_build("/dev/tty",5,0,KATTACH_PERM_W_CHR);
	kattach_thedawn_devlist_build("/dev/tty1",4,1,KATTACH_PERM_CHR);
	kattach_thedawn_devlist_build("/dev/tty2",4,2,KATTACH_PERM_CHR);
	kattach_thedawn_devlist_build("/dev/tty3",4,3,KATTACH_PERM_CHR);
	kattach_thedawn_devlist_build("/dev/tty4",4,4,KATTACH_PERM_CHR);
	kattach_thedawn_devlist_build("/dev/ttyS0",4,64,KATTACH_PERM_CHR);
	kattach_thedawn_devlist_build("/dev/random",1,8,KATTACH_PERM_CHR);
	kattach_thedawn_devlist_build("/dev/urandom",1,9,KATTACH_PERM_CHR);
	kattach_thedawn_devlist_build("/dev/ptmx",5,2,KATTACH_PERM_W_CHR);
	kattach_thedawn_devlist_build("/dev/zero",1,5,KATTACH_PERM_W_CHR);
	kattach_thedawn_devlist_build("/dev/mptctl",10,240,KATTACH_PERM_W_CHR);
	kattach_thedawn_devlist_build("/dev/misc/rtc",254,0,KATTACH_PERM_CHR);
	kattach_thedawn_devlist_build("/dev/rtc0",254,0,KATTACH_PERM_CHR);


	/* loopback devices */
	for (x = 0; x <= 7; x++) {
		sprintf(ctmp,"/dev/loop%d",x);
		kattach_thedawn_devlist_build(ctmp,7,x,KATTACH_PERM_BLK);
	}

	/* sda */
	kattach_thedawn_devlist_build("/dev/sda",8,0,KATTACH_PERM_BLK);
	for (x = 1; x <= 15; x++) {
		sprintf(ctmp,"/dev/sda%d",x);
		kattach_thedawn_devlist_build(ctmp,8,x,KATTACH_PERM_BLK);
	}

	/* sdb */
	kattach_thedawn_devlist_build("/dev/sdb",8,16,KATTACH_PERM_BLK);
	for (x = 17; x <= 31; x++) {
		sprintf(ctmp,"/dev/sdb%d",(x - 16));
		kattach_thedawn_devlist_build(ctmp,8,x,KATTACH_PERM_BLK);
	}

	/* sdc */
	kattach_thedawn_devlist_build("/dev/sdc",8,32,KATTACH_PERM_BLK);
	for (x = 33; x <= 47; x++) {
		sprintf(ctmp,"/dev/sdc%d",(x - 32));
		kattach_thedawn_devlist_build(ctmp,8,x,KATTACH_PERM_BLK);
	}

	/* sdd */
	kattach_thedawn_devlist_build("/dev/sdd",8,48,KATTACH_PERM_BLK);
	for (x = 49; x <= 63; x++) {
		sprintf(ctmp,"/dev/sdd%d",(x - 48));
		kattach_thedawn_devlist_build(ctmp,8,x,KATTACH_PERM_BLK);
	}

	/* sde */
	kattach_thedawn_devlist_build("/dev/sde",8,64,KATTACH_PERM_BLK);
	for (x = 65; x <= 79; x++) {
		sprintf(ctmp,"/dev/sde%d",(x - 64));
		kattach_thedawn_devlist_build(ctmp,8,x,KATTACH_PERM_BLK);
	}

	/* hda */
	kattach_thedawn_devlist_build("/dev/hda",3,0,KATTACH_PERM_BLK);
	for (x = 1; x <= 8; x++) {
		sprintf(ctmp,"/dev/hda%d",x);
		kattach_thedawn_devlist_build(ctmp,3,x,KATTACH_PERM_BLK);
	}

	/* hdb */
	kattach_thedawn_devlist_build("/dev/hdb",3,64,KATTACH_PERM_BLK);
	for (x = 65; x <= 72; x++) {
		sprintf(ctmp,"/dev/hdb%d",(x - 64));
		kattach_thedawn_devlist_build(ctmp,3,x,KATTACH_PERM_BLK);
	}

	/* hdc */
	kattach_thedawn_devlist_build("/dev/hdc",22,0,KATTACH_PERM_BLK);
	for (x = 1; x <= 8; x++) {
		sprintf(ctmp,"/dev/hdc%d",x);
		kattach_thedawn_devlist_build(ctmp,22,x,KATTACH_PERM_BLK);
	}

	/* hdd */
	kattach_thedawn_devlist_build("/dev/hdd",22,64,KATTACH_PERM_BLK);
	for (x = 65; x <= 72; x++) {
		sprintf(ctmp,"/dev/hdd%d",(x - 64));
		kattach_thedawn_devlist_build(ctmp,22,x,KATTACH_PERM_BLK);
	}

	/* hde */
	kattach_thedawn_devlist_build("/dev/hde",33,0,KATTACH_PERM_BLK);
	for (x = 1; x <= 8; x++) {
		sprintf(ctmp,"/dev/hde%d",x);
		kattach_thedawn_devlist_build(ctmp,33,x,KATTACH_PERM_BLK);
	}

	/* hdf */
	kattach_thedawn_devlist_build("/dev/hdf",33,64,KATTACH_PERM_BLK);
	for (x = 65; x <= 72; x++) {
		sprintf(ctmp,"/dev/hdf%d",(x - 64));
		kattach_thedawn_devlist_build(ctmp,33,x,KATTACH_PERM_BLK);
	}

	for (x = 0; x <= 16; x++) {
		sprintf(ctmp,"/dev/md%d",x);
		kattach_thedawn_devlist_build(ctmp,9,x,KATTACH_PERM_BLK);
	}

	kattach_thedawn_devlist_build("/dev/net/tun",10,200,KATTACH_PERM_CHR);
	kattach_thedawn_devlist_build("/dev/kvm",10,232,KATTACH_PERM_CHR);

	return;
}

#if !defined(KATTACH_BLD_VKAOS)
void
kattach_thedawn_shm(void)
{
	kattach_fd_shm_q = kattach_thedawn_shm_open(KATTACH_SHM_FILE_Q);
	kattach_fd_shm_dev = kattach_thedawn_shm_open(KATTACH_SHM_FILE_DEV);
	kattach_fd_shm_cfg = kattach_thedawn_shm_open(KATTACH_SHM_FILE_CFG);
	kattach_fd_shm_install = kattach_thedawn_shm_open(KATTACH_SHM_FILE_INSTALL);
	kattach_fd_shm_vmst = kattach_thedawn_shm_open(KATTACH_SHM_FILE_VMST);
	kattach_fd_shm_vmports = kattach_thedawn_shm_open(KATTACH_SHM_FILE_VMPORTS);
	kattach_fd_shm_vbridge = kattach_thedawn_shm_open(KATTACH_SHM_FILE_VBRIDGE);
	kattach_fd_shm_vmimages = kattach_thedawn_shm_open(KATTACH_SHM_FILE_VMIMAGES);
	kattach_fd_shm_appmods = kattach_thedawn_shm_open(KATTACH_SHM_FILE_APPMODS);
	kattach_fd_shm_netdev = kattach_thedawn_shm_open(KATTACH_SHM_FILE_NETDEV);
	kattach_fd_shm_vns = kattach_thedawn_shm_open(KATTACH_SHM_FILE_VNS);
	kattach_fd_shm_cfggrp = kattach_thedawn_shm_open(KATTACH_SHM_FILE_CFGGRP);
	kattach_fd_shm_fw = kattach_thedawn_shm_open(KATTACH_SHM_FILE_FW);
	return;
}

int
kattach_thedawn_shm_open(char *kattach_shm_path)
{
	int kattach_shm_fd = 0;

	int new = 0, res = 0;

	if ((kattach_shm_fd = shm_open(kattach_shm_path, (O_CREAT | O_EXCL | O_RDWR), (S_IREAD | S_IWRITE))) > 0) {
                new = 1;
        } else if ((kattach_shm_fd = shm_open(kattach_shm_path, (O_CREAT | O_RDWR), (S_IREAD | S_IWRITE))) < 0) {
                printf("\n [!] Unexpected SHM Error - %s\n",kattach_shm_path);
                return 0;
        }

	if (!new) {
                printf("\n [X] Unexpected SHM State - %s\n",kattach_shm_path);
	}

	res = fchown(kattach_shm_fd, KATTACH_UID_ROOT, KATTACH_GID_APPQ);
	res = fchmod(kattach_shm_fd, KATTACH_PERM_SHM);

	return (kattach_shm_fd);
}

void
kattach_thedawn_shm_close(void)
{
	/* move this to shutdown file */
	fsync(kattach_fd_shm_q);
	fsync(kattach_fd_shm_dev);
	fsync(kattach_fd_shm_cfg);
	fsync(kattach_fd_shm_install);
	fsync(kattach_fd_shm_vmst);
	fsync(kattach_fd_shm_vmports);
	fsync(kattach_fd_shm_vbridge);
	fsync(kattach_fd_shm_vmimages);
	fsync(kattach_fd_shm_appmods);
	fsync(kattach_fd_shm_netdev);
	fsync(kattach_fd_shm_vns);
	fsync(kattach_fd_shm_cfggrp);
	fsync(kattach_fd_shm_fw);

	close(kattach_fd_shm_q);
	close(kattach_fd_shm_dev);
	close(kattach_fd_shm_cfg);
	close(kattach_fd_shm_install);
	close(kattach_fd_shm_vmst);
	close(kattach_fd_shm_vmports);
	close(kattach_fd_shm_vbridge);
	close(kattach_fd_shm_vmimages);
	close(kattach_fd_shm_appmods);
	close(kattach_fd_shm_netdev);
	close(kattach_fd_shm_vns);
	close(kattach_fd_shm_cfggrp);
	close(kattach_fd_shm_fw);

	shm_unlink(KATTACH_SHM_FILE_Q);
	shm_unlink(KATTACH_SHM_FILE_DEV);
	shm_unlink(KATTACH_SHM_FILE_CFG);
	shm_unlink(KATTACH_SHM_FILE_INSTALL);
	shm_unlink(KATTACH_SHM_FILE_VMST);
	shm_unlink(KATTACH_SHM_FILE_VMPORTS);
	shm_unlink(KATTACH_SHM_FILE_VBRIDGE);
	shm_unlink(KATTACH_SHM_FILE_VMIMAGES);
	shm_unlink(KATTACH_SHM_FILE_APPMODS);
	shm_unlink(KATTACH_SHM_FILE_NETDEV);
	shm_unlink(KATTACH_SHM_FILE_VNS);
	shm_unlink(KATTACH_SHM_FILE_CFGGRP);
	shm_unlink(KATTACH_SHM_FILE_FW);
	return;
}
#endif /* !defined(KATTACH_BLD_VKAOS) */

void
kattach_thedawn_cfg_write(void)
{
	FILE *stream;
	char cfgfile[255];
	char dfpass[32];
	struct timeval pwdtime;
	time_t pwdsecs;
	int ret = 0;

	/* kaos.release */
	memset(cfgfile,0,sizeof(cfgfile));
	sprintf(cfgfile,"%s/%s",KATTACH_CFGPATH,KATTACH_CONF_RELEASE);
	stream = fopen(cfgfile,"w");

	if (stream == (FILE *)0) {
		printf("\n [!] FATAL -- Unable to write %s\n",cfgfile);
	} else {
		fprintf(stream,"KaOS release %s",KATTACH_VERSION);
		fclose(stream);
	}

	/* issue */
	memset(cfgfile,0,sizeof(cfgfile));
	sprintf(cfgfile,"%s/%s",KATTACH_CFGPATH,KATTACH_CONF_ISSUE);
	stream = fopen(cfgfile,"w");

	if (stream == (FILE *)0) {
		printf("\n [!] FATAL -- Unable to write %s\n",cfgfile);
	} else {
		fprintf(stream,"\n\n");
#if defined(KATTACH_BLD_VKAOS)
        	fprintf(stream,"Virtual Kernel Attached Operating System (vKaOS)\n");
#else /* defined(KATTACH_BLD_VKAOS) */
        	fprintf(stream,"Kernel Attached Operating System (KaOS)\n");
#endif /* defined(KATTACH_BLD_VKAOS) */
        	fprintf(stream,"version %s\n",KATTACH_VERSION);
        	fprintf(stream,"%s\n\n",KATTACH_COPYRIGHT);
        	fprintf(stream,"%s\n\n",KATTACH_URL);
		fclose(stream);
	}

	/* hosts */
	memset(cfgfile,0,sizeof(cfgfile));
	sprintf(cfgfile,"%s/%s",KATTACH_CFGPATH,KATTACH_CONF_HOSTS);
	stream = fopen(cfgfile,"w");

	if (stream == (FILE *)0) {
		printf("\n [!] FATAL -- Unable to write %s\n",cfgfile);
	} else {
		fprintf(stream,"127.0.0.1 localhost");
		fprintf(stream,"255.255.255.255 broadcasthost");
		fclose(stream);
	}

	/* passwd file */
	memset(cfgfile,0,sizeof(cfgfile));
	sprintf(cfgfile,"%s/%s",KATTACH_CFGPATH,KATTACH_CONF_PASSWD);
	stream = fopen(cfgfile,"w");

	if (stream == (FILE *)0) {
		printf("\n [!] FATAL -- Unable to write %s\n",cfgfile);
	} else {
		fprintf(stream,"root:x:0:0:root:/root:/bin/ash\n");
		fprintf(stream,"bin:x:1:1:bin:/dev/null:/bin/flase\n");
		fprintf(stream,"nobody:x:99:99:unpriviledged user:/dev/null:/bin/false\n");
#if defined(KATTACH_BLD_VKAOS)
		fprintf(stream,"vkuser:x:100:77:AppQueue CLI user:/appq:/bin/ash\n");
#else /* defined(KATTACH_BLD_VKAOS) */
		fprintf(stream,"aqcli:x:100:77:AppQueue CLI user:/appq:/kaos/core/aq/appqueue\n");
#endif /* defined(KATTACH_BLD_VKAOS) */
		fclose(stream);
	}

	/* group file */
	memset(cfgfile,0,sizeof(cfgfile));
	sprintf(cfgfile,"%s/%s",KATTACH_CFGPATH,KATTACH_CONF_GROUP);
	stream = fopen(cfgfile,"w");

	if (stream == (FILE *)0) {
		printf("\n [!] FATAL -- Unable to write %s\n",cfgfile);
	} else {
		fprintf(stream,"root:x:0:\n");
		fprintf(stream,"bin:x:1:\n");
		fprintf(stream,"sys:x:2:\n");
		fprintf(stream,"kmem:x:3:\n");
		fprintf(stream,"tty:x:4:\n");
		fprintf(stream,"utmp:x:13:\n");
		fprintf(stream,"usb:x:14:\n");
		fprintf(stream,"aqcli:x:77:root,aqcli\n");
		fprintf(stream,"nogroup:x:99:\n");
		fclose(stream);
	}

	/* nsswitch file */
	memset(cfgfile,0,sizeof(cfgfile));
	sprintf(cfgfile,"%s/%s",KATTACH_CFGPATH,KATTACH_CONF_NSSWITCH);
	stream = fopen(cfgfile,"w");

	if (stream == (FILE *)0) {
		printf("\n [!] FATAL -- Unable to write %s\n",cfgfile);
	} else {
		fprintf(stream,"# auto-generated nsswitch.conf\n\n");
		fprintf(stream,"passwd: files\n");
		fprintf(stream,"group: files\n");
		fprintf(stream,"shadow: files\n\n");
		fprintf(stream,"hosts: files dns\n");
		fprintf(stream,"networks: files\n\n");
		fprintf(stream,"protocols: files\n");
		fprintf(stream,"services: files\n");
		fprintf(stream,"ethers: files\n");
		fprintf(stream,"rpc: files\n\n");
		fprintf(stream,"# End nsswitch.conf");
		fclose(stream);
	}

	/* shadow file */
	memset(cfgfile,0,sizeof(cfgfile));
	sprintf(cfgfile,"%s/%s",KATTACH_CFGPATH,KATTACH_CONF_SHADOW);
	stream = fopen(cfgfile,"w");

	if (stream == (FILE *)0) {
		printf("\n [!] FATAL -- Unable to write %s\n",cfgfile);
	} else {
		gettimeofday(&pwdtime,NULL);
		pwdsecs = pwdtime.tv_sec;
		sprintf(dfpass,"kaos!%s",KATTACH_VERSION);		
		fprintf(stream,"root:%s:%u:0:99999:7:::\n",kattach_thedawn_genpass(dfpass),(unsigned int) (pwdsecs / 86400));
		fprintf(stream,"bin:x:%u:0:99999:7:::\n",(unsigned int) (pwdsecs / 86400));
		fprintf(stream,"nobody:x:%u:0:99999:7:::\n",(unsigned int) (pwdsecs / 86400));
#if !defined(KATTACH_BLD_VKAOS)
		fprintf(stream,"aqcli:%s:%u:0:99999:7:::\n",kattach_thedawn_genpass(dfpass),(unsigned int) (pwdsecs / 86400));
#else /* !defined(KATTACH_BLD_VKAOS) */
		fprintf(stream,"vkuser:%s:%u:0:99999:7:::\n",kattach_thedawn_genpass(dfpass),(unsigned int) (pwdsecs / 86400));
#endif /* !defined(KATTACH_BLD_VKAOS) */
		fclose(stream);
		ret = chmod(cfgfile, KATTACH_PERM_SECURE_RO);
	}


	/* shells file */
	memset(cfgfile,0,sizeof(cfgfile));
	sprintf(cfgfile,"%s/%s",KATTACH_CFGPATH,KATTACH_CONF_SHELLS);
	stream= fopen(cfgfile,"w");

        if (stream == (FILE *)0) {
                printf("\n [!] FATAL -- Unable to write %s\n",cfgfile);
        } else {
		fprintf(stream,"%s\n",KATTACH_APPQUEUE_CLI);
		fprintf(stream,"%s\n",KATTACH_APPQUEUE_CLI_II);
		fclose(stream);
	}

	/* protocols file */
	memset(cfgfile,0,sizeof(cfgfile));
	sprintf(cfgfile,"%s/%s",KATTACH_CFGPATH,KATTACH_CONF_PROTOCOLS);
	stream= fopen(cfgfile,"w");

        if (stream == (FILE *)0) {
                printf("\n [!] FATAL -- Unable to write %s\n",cfgfile);
        } else {
                fprintf(stream,"ip      0       IP              # internet protocol, pseudo protocol number\n");
                fprintf(stream,"icmp    1       ICMP            # internet control message protocol\n");
                fprintf(stream,"igmp    2       IGMP            # Internet Group Management\n");
                fprintf(stream,"ggp     3       GGP             # gateway-gateway protocol\n");
                fprintf(stream,"ipencap 4       IP-ENCAP        # IP encapsulated in IP (officially ``IP'')\n");
                fprintf(stream,"st      5       ST              # ST datagram mode\n");
                fprintf(stream,"tcp     6       TCP             # transmission control protocol\n");
                fprintf(stream,"egp     8       EGP             # exterior gateway protocol\n");
                fprintf(stream,"igp     9       IGP             # any private interior gateway (Cisco)\n");
                fprintf(stream,"pup     12      PUP             # PARC universal packet protocol\n");
                fprintf(stream,"udp     17      UDP             # user datagram protocol\n");
                fprintf(stream,"hmp     20      HMP             # host monitoring protocol\n");
                fprintf(stream,"xns-idp 22      XNS-IDP         # Xerox NS IDP\n");
                fprintf(stream,"rdp     27      RDP             # reliable datagram protocol\n");
                fprintf(stream,"iso-tp4 29      ISO-TP4         # ISO Transport Protocol class 4 [RFC905]\n");
                fprintf(stream,"xtp     36      XTP             # Xpress Transfer Protocol\n");
                fprintf(stream,"ddp     37      DDP             # Datagram Delivery Protocol\n");
                fprintf(stream,"idpr-cmtp 38    IDPR-CMTP       # IDPR Control Message Transport\n");
                fprintf(stream,"ipv6    41      IPv6            # Internet Protocol, version 6\n");
                fprintf(stream,"ipv6-route 43   IPv6-Route      # Routing Header for IPv6\n");
                fprintf(stream,"ipv6-frag 44    IPv6-Frag       # Fragment Header for IPv6\n");
                fprintf(stream,"idrp    45      IDRP            # Inter-Domain Routing Protocol\n");
                fprintf(stream,"rsvp    46      RSVP            # Reservation Protocol\n");
                fprintf(stream,"gre     47      GRE             # General Routing Encapsulation\n");
                fprintf(stream,"esp     50      IPSEC-ESP       # Encap Security Payload [RFC2406]\n");
                fprintf(stream,"ah      51      IPSEC-AH        # Authentication Header [RFC2402]\n");
                fprintf(stream,"skip    57      SKIP            # SKIP\n");
                fprintf(stream,"ipv6-icmp 58    IPv6-ICMP       # ICMP for IPv6\n");
                fprintf(stream,"ipv6-nonxt 59   IPv6-NoNxt      # No Next Header for IPv6\n");
                fprintf(stream,"ipv6-opts 60    IPv6-Opts       # Destination Options for IPv6\n");
                fprintf(stream,"rspf    73      RSPF CPHB       # Radio Shortest Path First (officially CPHB)\n");
                fprintf(stream,"vmtp    81      VMTP            # Versatile Message Transport\n");
                fprintf(stream,"eigrp   88      EIGRP           # Enhanced Interior Routing Protocol (Cisco)\n");
                fprintf(stream,"ospf    89      OSPFIGP         # Open Shortest Path First IGP\n");
                fprintf(stream,"ax.25   93      AX.25           # AX.25 frames\n");
                fprintf(stream,"ipip    94      IPIP            # IP-within-IP Encapsulation Protocol\n");
                fprintf(stream,"etherip 97      ETHERIP         # Ethernet-within-IP Encapsulation [RFC3378]\n");
                fprintf(stream,"encap   98      ENCAP           # Yet Another IP encapsulation [RFC1241]\n");
                fprintf(stream,"pim     103     PIM             # Protocol Independent Multicast\n");
                fprintf(stream,"ipcomp  108     IPCOMP          # IP Payload Compression Protocol\n");
                fprintf(stream,"vrrp    112     VRRP            # Virtual Router Redundancy Protocol\n");
                fprintf(stream,"l2tp    115     L2TP            # Layer Two Tunneling Protocol [RFC2661]\n");
                fprintf(stream,"isis    124     ISIS            # IS-IS over IPv4\n");
                fprintf(stream,"sctp    132     SCTP            # Stream Control Transmission Protocol\n");
                fprintf(stream,"fc      133     FC              # Fibre Channel\n");
                fprintf(stream,"udplite 136     UDPLite         # UDP-Lite\n");
		fclose(stream);
	}

#if defined(KATTACH_IANA)
	/* generate services */
	kattach_iana_ports();
#else /* defined(KATTACH_IANA) */
	/* protocols file */
	memset(cfgfile,0,sizeof(cfgfile));
	sprintf(cfgfile,"%s/%s",KATTACH_CFGPATH,KATTACH_CONF_SERVICES);
	stream= fopen(cfgfile,"w");

        if (stream == (FILE *)0) {
                printf("\n [!] FATAL -- Unable to write %s\n",cfgfile);
        } else {
		fprintf(stream,"ftp-data        20/tcp\n");
		fprintf(stream,"ftp-data        20/udp\n");
		fprintf(stream,"ftp             21/tcp\n");
		fprintf(stream,"ftp             21/udp\n");
		fprintf(stream,"ssh             22/tcp\n");
		fprintf(stream,"ssh             22/udp\n");
		fprintf(stream,"telnet          23/tcp\n");
		fprintf(stream,"telnet          23/udp\n");
		fprintf(stream,"smtp            25/tcp\n");
		fprintf(stream,"smtp            25/udp\n");
		fprintf(stream,"tacacs          49/tcp\n");
		fprintf(stream,"tacacs          49/udp\n");
		fprintf(stream,"domain          53/tcp\n");
		fprintf(stream,"domain          53/udp\n");
		fprintf(stream,"bootps          67/tcp\n");
		fprintf(stream,"bootps          67/udp\n");
		fprintf(stream,"dhcpc           68/tcp\n");
		fprintf(stream,"dhcpc           68/udp\n");
		fprintf(stream,"tftp            69/tcp\n");
		fprintf(stream,"tftp            69/udp\n");
		fprintf(stream,"http            80/tcp\n");
		fprintf(stream,"http            80/udp\n");
		fprintf(stream,"kerberos        88/tcp\n");
		fprintf(stream,"kerberos        88/udp\n");
		fprintf(stream,"pop3            110/tcp\n");
		fprintf(stream,"pop3            110/udp\n");
		fprintf(stream,"portmapper      111/tcp\n");
		fprintf(stream,"portmapper      111/udp\n");
		fprintf(stream,"ntp             123/tcp\n");
		fprintf(stream,"ntp             123/udp\n");
		fprintf(stream,"imap            143/tcp\n");
		fprintf(stream,"imap            143/udp\n");
		fprintf(stream,"snmp            161/tcp\n");
		fprintf(stream,"snmp            161/udp\n");
		fprintf(stream,"snmptrap        162/udp\n");
		fprintf(stream,"bgp             179/tcp\n");
		fprintf(stream,"bgp             179/udp\n");
		fprintf(stream,"ldap            389/tcp\n");
		fprintf(stream,"ldap            389/udp\n");
		fprintf(stream,"https           443/tcp\n");
		fprintf(stream,"https           443/udp\n");
		fprintf(stream,"rtsp            554/tcp\n");
		fprintf(stream,"rtsp            554/udp\n");
		fprintf(stream,"rsync           873/tcp\n");
		fprintf(stream,"rsync           873/udp\n");
		fprintf(stream,"imaps           993/tcp\n");
		fprintf(stream,"imaps           993/udp\n");
		fprintf(stream,"pop3s           995/tcp\n");
		fprintf(stream,"pop3s           995/udp\n");
		fprintf(stream,"traceroute      33434/tcp\n");
		fprintf(stream,"traceroute      33434/udp\n");
		fclose(stream);
	}
#endif /* defined(KATTACH_IANA) */

	return;	
}

char *
kattach_thedawn_genpass(char *dfpass)
{
	unsigned long pwdhash = 0;
	unsigned int pwdtmp = 0;	
	char *passwd;
	char salt[16];
	char salty[19];
	int i = 0, y = 0, z = 0, j = 0;

	if (strlen(dfpass) == 0) return(NULL);

	while (!z) {
		kattach_getruuid();
		pwdhash = kattach_hash(kattach_ruuid);
		pwdhash ^= time(NULL);

		for (j = 0; j <= 36; j++) {
			if (i == 15) {
				z = 1;
				salt[i] = '\0';
				break;
			}
			pwdtmp = ((pwdhash >> j) & 0xff);
			y = pwdtmp;
			if (((y >= 0x2e) && (y <= 0x39)) ||
				((y >= 0x41) && (y <= 0x5a)) ||
				((y >= 0x61) && (y <= 0x7a))) {
				salt[i] = (char) y;
				i++;
			}
		}
	}
	sprintf(salty,"$6$%s",salt);
	passwd = crypt(dfpass,salty);

	return(passwd);
}

char *
kattach_thedawn_chkpass(char *dfpass, char *encpass)
{
        char *passwd;

        if (strlen(dfpass) == 0) return(NULL);

        passwd = crypt(dfpass,encpass);
        return(passwd);
}


void
kattach_thedawn_cmfs(void)
{
        char lcmd[255];

        sprintf(lcmd,"%s%s %s %s%s",KATTACH_SQFSPATH,KCMD_MKSQUASHFS,KATTACH_CMFSPATH,KATTACH_CMFIPATH,KATTACH_NET_MAGIC);
        kattach_sysexec(lcmd);

        sprintf(lcmd,"%s%s -rf %s*",KATTACH_BINPATH,KCMD_RM,KATTACH_CMFSPATH);
        kattach_sysexec(lcmd);

	sync();

        sprintf(lcmd,"%s%s %s%s %s -t %s -o loop=%s",KATTACH_BINPATH,KCMD_MOUNT,KATTACH_CMFIPATH,KATTACH_NET_MAGIC,KATTACH_CMFSPATH,KATTACH_FS_SQUASHFS,KATTACH_RESERVED_LOOPBACK_CMFS);
        kattach_sysexec(lcmd);

        return;

}

#if !defined(KATTACH_BLD_VKAOS)
void
kattach_thedawn_genauth(void)
{
	FILE *stream;
	char cfgfile[255];
	char dfpass[32];
	struct timeval pwdtime;
	time_t pwdsecs;
	int ret = 0;

	if (kattach_cfg.aquser[0] == '\0') {
		sprintf(kattach_cfg.aquser,"aqcli");
	}

	if (kattach_cfg.aqpass[0] == '\0') {
		sprintf(dfpass,"kaos!%s",KATTACH_VERSION);
		sprintf(kattach_cfg.aqpass,"%s",kattach_thedawn_genpass(dfpass));
	}

	if (kattach_cfg.rootpass[0] == '\0') {
		kattach_cfg.root = 0;
		printf("\n [!] WARNING: Root password not set. Disabling root access \n");
	}

	/* passwd file */
	memset(cfgfile,0,sizeof(cfgfile));
	sprintf(cfgfile,"%s/%s",KATTACH_CFGPATH,KATTACH_CONF_PASSWD);
	stream = fopen(cfgfile,"w");

	if (stream == (FILE *)0) {
		printf("\n [!] FATAL -- Unable to write %s\n",cfgfile);
	} else {
		fprintf(stream,"root:x:0:0:root:/root:/bin/ash\n");
		fprintf(stream,"bin:x:1:1:bin:/dev/null:/bin/false\n");
		fprintf(stream,"nobody:x:99:99:unpriviledged user:/dev/null:/bin/false\n");
		fprintf(stream,"%s:x:100:77:AppQueue CLI user:/appq:/kaos/core/aq/appqueue\n",kattach_cfg.aquser);
		fclose(stream);
	}

	/* shadow file */
	memset(cfgfile,0,sizeof(cfgfile));
	sprintf(cfgfile,"%s/%s",KATTACH_CFGPATH,KATTACH_CONF_SHADOW);
	stream = fopen(cfgfile,"w");

	if (stream == (FILE *)0) {
		printf("\n [!] FATAL -- Unable to write %s\n",cfgfile);
	} else {
		gettimeofday(&pwdtime,NULL);
		pwdsecs = pwdtime.tv_sec;
		if (kattach_cfg.root) {
			fprintf(stream,"root:%s:%u:0:99999:7:::\n",kattach_cfg.rootpass,(unsigned int) (pwdsecs / 86400));
		} else {
			fprintf(stream,"root:x:%u:0:99999:7:::\n",(unsigned int) (pwdsecs / 86400));
		}
		fprintf(stream,"bin:x:%u:0:99999:7:::\n",(unsigned int) (pwdsecs / 86400));
		fprintf(stream,"nobody:x:%u:0:99999:7:::\n",(unsigned int) (pwdsecs / 86400));
		fprintf(stream,"%s:%s:%u:0:99999:7:::\n",kattach_cfg.aquser,kattach_cfg.aqpass,(unsigned int) (pwdsecs / 86400));
		fclose(stream);
		ret = chmod(cfgfile, KATTACH_PERM_SECURE_RO);
	}
	return;
}
#endif /* defined(KATTACH_BLD_VKAOS) */
