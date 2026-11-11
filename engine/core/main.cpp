
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

extern u32 *color_buffer[2];
extern u32 color_pitch;

static void sysutil_exit_callback(u64 status, u64 param, void *userdata)
{
    (void)param;
    (void)userdata;

    if (status == SYSUTIL_EXIT_GAME)
        running = 0;
}

static const u8* glyphFor(char c)
{
    static const u8 A[7] = {14,17,17,31,17,17,17};
    static const u8 E[7] = {31,16,16,30,16,16,31};
    static const u8 I[7] = {14,4,4,4,4,4,14};
    static const u8 N[7] = {17,25,21,21,19,17,17};
    static const u8 O[7] = {14,17,17,17,17,17,14};
    static const u8 P[7] = {30,17,17,30,16,16,16};
    static const u8 R[7] = {30,17,17,30,20,18,17};
    static const u8 S[7] = {15,16,16,14,1,1,30};
    static const u8 T[7] = {31,4,4,4,4,4,4};
    static const u8 X[7] = {17,17,10,4,10,17,17};
    static const u8 THREE[7] = {14,17,1,6,1,17,14};
    static const u8 DASH[7] = {0,0,0,31,0,0,0};
    static const u8 SPACE[7] = {0,0,0,0,0,0,0};

    switch (c) {
        case 'A': return A;
        case 'E': return E;
        case 'I': return I;
        case 'N': return N;
        case 'O': return O;
        case 'P': return P;
        case 'R': return R;
        case 'S': return S;
        case 'T': return T;
        case 'X': return X;
        case '3': return THREE;
        case '-': return DASH;
        case ' ': return SPACE;
        default: return SPACE;
    }
}

static void fillRect(u32 *buffer, u32 pitchPixels,
                     int x, int y, int width, int height, u32 color)
{
    for (int py = 0; py < height; ++py) {
        int sy = y + py;
        if (sy < 0 || sy >= (int)display_height)
            continue;

        for (int px = 0; px < width; ++px) {
            int sx = x + px;
            if (sx >= 0 && sx < (int)display_width)
                buffer[sy * pitchPixels + sx] = color;
        }
    }
}

static void drawText(u32 *buffer, u32 pitchPixels,
                     int x, int y, const char *text,
                     u32 color, int scale)
{
    int penX = x;

    for (const char *p = text; *p; ++p) {
        const u8 *glyph = glyphFor(*p);

        for (int row = 0; row < 7; ++row) {
            for (int col = 0; col < 5; ++col) {
                if (glyph[row] & (1 << (4 - col))) {
                    fillRect(buffer, pitchPixels,
                        penX + col * scale,
                        y + row * scale,
                        scale, scale, color);
                }
            }
        }

        penX += 6 * scale;
    }
}

static int textWidth(const char *text, int scale)
{
    int length = 0;
    while (text[length])
        ++length;

    return length ? (length * 6 - 1) * scale : 0;
}

static void drawTitleScreen(void)
{
    u32 *buffer = color_buffer[curr_fb];
    u32 pitchPixels = color_pitch / sizeof(u32);

    for (u32 y = 0; y < display_height; ++y) {
        u32 *row = buffer + y * pitchPixels;

        for (u32 x = 0; x < display_width; ++x)
            row[x] = 0x00101828;
    }

    const char *title = "NANA-PS3";
    const char *prompt = "PRESS X TO EXIT";

    const int titleScale = 8;
    const int promptScale = 3;

    int titleX = ((int)display_width -
                  textWidth(title, titleScale)) / 2;

    int titleY = (int)display_height / 2 - 82;

    int promptX = ((int)display_width -
                   textWidth(prompt, promptScale)) / 2;

    int promptY = (int)display_height / 2 + 48;

    fillRect(buffer, pitchPixels,
        (int)display_width / 2 - 170,
        titleY + 72, 340, 3, 0x00c77991);

    fillRect(buffer, pitchPixels,
        (int)display_width / 2 - 170,
        titleY + 72, 42, 3, 0x00f0c4cf);

    fillRect(buffer, pitchPixels,
        (int)display_width / 2 + 128,
        titleY + 72, 42, 3, 0x00f0c4cf);

    drawText(buffer, pitchPixels, titleX, titleY,
        title, 0x00f4e9ec, titleScale);

    drawText(buffer, pitchPixels, promptX, promptY,
        prompt, 0x00c9b9c0, promptScale);

    __sync_synchronize();
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

    // Draw the title screen once, then wait for input.
    drawTitleScreen();
    flip();

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

        usleep(10000);
    }

    // Preserve the tested shutdown sequence.
    sysUtilUnregisterCallback(0);
    gcmSetWaitFlip(context);
    rsxFinish(context, 1);
    free(host_addr);

    return 0;
}
