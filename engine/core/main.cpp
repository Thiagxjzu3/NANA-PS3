
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

static void drawScreen(void)
{
    const u32 clearMask =
        GCM_CLEAR_R |
        GCM_CLEAR_G |
        GCM_CLEAR_B |
        GCM_CLEAR_A;

    // Fondo azul oscuro.
    rsxSetScissor(context, 0, 0,
        (u16)display_width, (u16)display_height);

    rsxSetClearColor(context, 0x00101828);
    rsxClearSurface(context, clearMask);

    // Franja rosa central.
    u16 barWidth = (u16)(display_width * 3 / 4);
    u16 barHeight = 8;
    u16 barX = (u16)((display_width - barWidth) / 2);
    u16 barY = (u16)(display_height / 2 - 25);

    rsxSetScissor(context, barX, barY, barWidth, barHeight);
    rsxSetClearColor(context, 0x00C77991);
    rsxClearSurface(context, clearMask);

    // Rectángulo blanco debajo de la franja.
    u16 boxWidth = (u16)(display_width / 3);
    u16 boxHeight = (u16)(display_height / 6);
    u16 boxX = (u16)((display_width - boxWidth) / 2);
    u16 boxY = (u16)(display_height / 2 + 15);

    rsxSetScissor(context, boxX, boxY, boxWidth, boxHeight);
    rsxSetClearColor(context, 0x00F4E9EC);
    rsxClearSurface(context, clearMask);

    // Restaurar el área completa.
    rsxSetScissor(context, 0, 0,
        (u16)display_width, (u16)display_height);
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

        rsxSetClearDepthStencil(context, 0xffff);

        drawScreen();
        flip();

        usleep(10000);
    }

    sysUtilUnregisterCallback(0);
    gcmSetWaitFlip(context);
    rsxFinish(context, 1);
    free(host_addr);

    return 0;
}
