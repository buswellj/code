/*
 * kattach (kernel attach)
 * Copyright (c) 2009-2010 Carbon Mountain LLC.
 * All Rights Reserved.
 *
 * John Buswell <buswellj@carbonmountain.com>
 * version 0.6.0.0
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
 */

/* global shared memory */
kattach_dev_t *kattach_devices_shm;
kattach_cfg_t *kattach_cfg_shm;
kattach_install_t *kattach_install_shm;
kattach_vmst_t *kattach_vmst_shm;
kattach_vmp_t *kattach_vmports_shm;
kattach_vbr_t *kattach_vbridge_shm;
kattach_vmi_t *kattach_vmimages_shm;
kattach_am_t *kattach_appmods_shm;
kattach_netdev_t *kattach_netdev_shm;
kattach_vns_t *kattach_vns_shm;
kattach_cfggrp_t *kattach_cfggrp_shm;
kattach_fw_t *kattach_fw_shm;
cm_ak_ping_pong_t *kattach_appqueue;             /* kattach - appqueue messaging */
