
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>

#include <ppu-types.h>
#include <sys/process.h>
#include <io/pad.h>
#include <rsx/rsx.h>
#include <sysutil/sysutil.h>

#include "rsxutil.h"

SYS_PROCESS_PARAM(1001, 0x100000);

static int running = 1;

extern "C" {

static void program_exit_callback()
{
    sysUtilUnregisterCallback(0);
    gcmSetWaitFlip(context);
    rsxFinish(context, 1);
}

static void sysutil_exit_callback(u64 status, u64 param, void *userdata)
{
    if (status == SYSUTIL_EXIT_GAME)
        running = 0;
}

}

int main(int argc, const char *argv[])
{
    padInfo padinfo;
    padData paddata;

    void *host_addr = memalign(1024 * 1024, HOST_SIZE);

    if (!host_addr)
        return 1;

    init_screen(host_addr, HOST_SIZE);

    ioPadInit(7);

    atexit(program_exit_callback);
    sysUtilRegisterCallback(0, sysutil_exit_callback, NULL);

    setRenderTarget(curr_fb);

    while (running)
    {
        sysUtilCheckCallback();

        ioPadGetInfo(&padinfo);

        for (int i = 0; i < MAX_PADS; i++)
        {
            if (padinfo.status[i])
            {
                if (ioPadGetData(i, &paddata) == 0)
                {
                    if (paddata.BTN_CROSS)
                        running = 0;
                }
            }
        }

        rsxSetClearColor(context, 0x101020);
        rsxSetClearDepthStencil(context, 0xffff);

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
    }

    free(host_addr);

    return 0;
}
