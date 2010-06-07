
/*
 * Copyright (C) 2008 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <dirent.h>
#include <unistd.h>
#include <sched.h>
#include <fcntl.h>

#include <sys/mount.h>

#include <linux/loop.h>
#include <linux/dm-ioctl.h>

#include <cutils/config_utils.h>
#include <cutils/properties.h>

#include "volmgr.h"
#include "vold.h"
#include "media.h"
#include "blkdev.h"
#include "loop.h"

static void *_align(void *ptr, unsigned int a)
{
        register unsigned long agn = --a;

        return (void *) (((unsigned long) ptr + agn) & ~agn);
}

blkdev_t *loop_init(volume_t *vol, const char *src)
{
	int rc;
    int i;
    int fd;
    char filename[255];
	blkdev_t *ret = NULL;
	media_t *media = NULL;

    LOG_VOL("loopback_init()");

    for (i = 0; i < MAX_LOOP; i++) {
        struct loop_info info;

        sprintf(filename, "/dev/block/loop%d", i);

        if ((fd = open(filename, O_RDWR)) < 0) {
            LOGE("Unable to open %s (%s)", filename, strerror(errno));
            return NULL;
        }

        rc = ioctl(fd, LOOP_GET_STATUS, &info);
        if (rc < 0 && errno == ENXIO)
            break;

        close(fd);

        if (rc < 0) {
            LOGE("Unable to get loop status for %s (%s)", filename,
                 strerror(errno));
            return NULL;
        }
    }

    if (i == MAX_LOOP) {
        LOGE("Out of loop devices");
        return NULL;
    }

    int file_fd;

    if ((file_fd = open(src, O_RDWR)) < 0) {
        LOGE("Unable to open %s (%s)", src,
             strerror(errno));
        return NULL;
	}

    if (ioctl(fd, LOOP_SET_FD, file_fd) < 0) {
        LOGE("Error setting up loopback interface (%s)", strerror(errno));
        return NULL;
    }

    close(fd);
    close(file_fd);

    LOG_VOL("Loop setup on %s for %s", filename, src);

	media = media_create(filename, "Loopback Device", "012345", media_loop);
	if(media == NULL)
		return NULL;

	ret = blkdev_create(NULL, filename, 1, 1, media, "loop");
    return ret;
}

int loop_stop(volume_t *dm)
{
    /*char devname[255];
    int device_fd;
    int rc = 0;

    LOG_VOL("loopback_stop():");

    device_fd = open(dm->type_data.loop.loop_dev, O_RDONLY);
    if (device_fd < 0) {
        LOG_ERROR("Failed to open loop (%d)", errno);
        return -errno;
    }

    if (ioctl(device_fd, LOOP_CLR_FD, 0) < 0) {
        LOG_ERROR("Failed to destroy loop (%d)", errno);
        rc = -errno;
    }

    close(device_fd);
    return rc;*/
	return 0;
}

