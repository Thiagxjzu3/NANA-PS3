
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <unistd.h>

#include <ppu-types.h>
#include <sys/process.h>
#include <io/pad.h>
#include <rsx/rsx.h>
#include <sysutil/sysutil.h>

#include "rsxutil.h"

SYS_PROCESS_PARAM(1001, 0x100000);

static int running = 1;

static void sysutil_exit_callback(u64 status, u64 param, void *userdata)
{
    (void)param;
    (void)userdata;

    if (status == SYSUTIL_EXIT_GAME)
        running = 0;
}

int main(int argc, const char *argv[])
{
    (void)argc;
    (void)argv;

    padInfo padinfo;
    padData paddata;

    void *host_addr = memalign(1024 * 1024, HOST_SIZE);

    if (!host_addr)
        return 1;

    init_screen(host_addr, HOST_SIZE);

    ioPadInit(7);
    sysUtilRegisterCallback(0, sysutil_exit_callback, NULL);

    setRenderTarget(curr_fb);

    while (running)
    {
        sysUtilCheckCallback();

        ioPadGetInfo(&padinfo);

        for (int i = 0; i < MAX_PADS; ++i)
        {
            if (padinfo.status[i] &&
                ioPadGetData(i, &paddata) == 0 &&
                paddata.BTN_CROSS)
            {
                running = 0;
                break;
            }
        }

        if (!running)
            break;

        rsxSetClearColor(context, 0x00C77991);
        rsxSetClearDepthValue(context, 0xffff);

        rsxClearSurface(
            context,
            GCM_CLEAR_R |
            GCM_CLEAR_G |
            GCM_CLEAR_B |
            GCM_CLEAR_A |
            GCM_CLEAR_S |
            GCM_CLEAR_Z
        );

        flip();

        usleep(10000);
    }

    sysUtilUnregisterCallback(0);
    gcmSetWaitFlip(context);
    rsxFinish(context, 1);
    free(host_addr);

    return 0;
}
