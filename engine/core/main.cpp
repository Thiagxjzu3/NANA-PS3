
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

static const u8* glyphFor(char c)
{
    static const u8 blank[7] = {0,0,0,0,0,0,0};

    static const u8 A[7] = {14,17,17,31,17,17,17};
    static const u8 B[7] = {30,17,17,30,17,17,30};
    static const u8 C[7] = {14,17,16,16,16,17,14};
    static const u8 D[7] = {30,17,17,17,17,17,30};
    static const u8 E[7] = {31,16,16,30,16,16,31};
    static const u8 F[7] = {31,16,16,30,16,16,16};
    static const u8 G[7] = {14,17,16,23,17,17,15};
    static const u8 H[7] = {17,17,17,31,17,17,17};
    static const u8 I[7] = {14,4,4,4,4,4,14};
    static const u8 J[7] = {7,2,2,2,18,18,12};
    static const u8 K[7] = {17,18,20,24,20,18,17};
    static const u8 L[7] = {16,16,16,16,16,16,31};
    static const u8 M[7] = {17,27,21,21,17,17,17};
    static const u8 N[7] = {17,25,21,19,17,17,17};
    static const u8 O[7] = {14,17,17,17,17,17,14};
    static const u8 P[7] = {30,17,17,30,16,16,16};
    static const u8 Q[7] = {14,17,17,17,21,18,13};
    static const u8 R[7] = {30,17,17,30,20,18,17};
    static const u8 S[7] = {15,16,16,14,1,1,30};
    static const u8 T[7] = {31,4,4,4,4,4,4};
    static const u8 U[7] = {17,17,17,17,17,17,14};
    static const u8 V[7] = {17,17,17,17,17,10,4};
    static const u8 W[7] = {17,17,17,21,21,21,10};
    static const u8 X[7] = {17,10,4,4,4,10,17};
    static const u8 Y[7] = {17,10,4,4,4,4,4};
    static const u8 Z[7] = {31,1,2,4,8,16,31};

    static const u8 N0[7] = {14,17,19,21,25,17,14};
    static const u8 N1[7] = {4,12,4,4,4,4,14};
    static const u8 N2[7] = {14,17,1,2,4,8,31};
    static const u8 N3[7] = {30,1,1,14,1,1,30};
    static const u8 N4[7] = {2,6,10,18,31,2,2};
    static const u8 N5[7] = {31,16,16,30,1,1,30};
    static const u8 N6[7] = {14,16,16,30,17,17,14};
    static const u8 N7[7] = {31,1,2,4,8,8,8};
    static const u8 N8[7] = {14,17,17,14,17,17,14};
    static const u8 N9[7] = {14,17,17,15,1,1,14};

    static const u8 dash[7] = {0,0,0,31,0,0,0};
    static const u8 dot[7] = {0,0,0,0,0,12,12};

    switch (c)
    {
        case 'A': return A;
        case 'B': return B;
        case 'C': return C;
        case 'D': return D;
        case 'E': return E;
        case 'F': return F;
        case 'G': return G;
        case 'H': return H;
        case 'I': return I;
        case 'J': return J;
        case 'K': return K;
        case 'L': return L;
        case 'M': return M;
        case 'N': return N;
        case 'O': return O;
        case 'P': return P;
        case 'Q': return Q;
        case 'R': return R;
        case 'S': return S;
        case 'T': return T;
        case 'U': return U;
        case 'V': return V;
        case 'W': return W;
        case 'X': return X;
        case 'Y': return Y;
        case 'Z': return Z;

        case '0': return N0;
        case '1': return N1;
        case '2': return N2;
        case '3': return N3;
        case '4': return N4;
        case '5': return N5;
        case '6': return N6;
        case '7': return N7;
        case '8': return N8;
        case '9': return N9;

        case '-': return dash;
        case '.': return dot;
        default: return blank;
    }
}

static void sysutil_exit_callback(u64 status, u64 param, void *userdata)
{
    (void)param;
    (void)userdata;

    if (status == SYSUTIL_EXIT_GAME)
        running = 0;
}

static void fillRect(int x, int y, int width, int height, u32 color)
{
    if (width <= 0 || height <= 0)
        return;

    rsxSetScissor(
        context,
        (u16)x,
        (u16)y,
        (u16)width,
        (u16)height
    );

    rsxSetClearColor(context, color);

    rsxClearSurface(
        context,
        GCM_CLEAR_R |
        GCM_CLEAR_G |
        GCM_CLEAR_B |
        GCM_CLEAR_A
    );
}

static void drawGlyph(int x, int y, int scale, char c, u32 color)
{
    const u8 *glyph = glyphFor(c);

    for (int row = 0; row < 7; ++row)
    {
        int col = 0;

        while (col < 5)
        {
            if (!(glyph[row] & (1 << (4 - col))))
            {
                ++col;
                continue;
            }

            int start = col;

            while (col < 5 &&
                   (glyph[row] & (1 << (4 - col))))
            {
                ++col;
            }

            fillRect(
                x + start * scale,
                y + row * scale,
                (col - start) * scale,
                scale,
                color
            );
        }
    }
}

static void drawTextCentered(
    const char *text,
    int y,
    int scale,
    u32 color
)
{
    int length = (int)strlen(text);

    if (length <= 0)
        return;

    int textWidth = length * 6 * scale - scale;
    int x = ((int)display_width - textWidth) / 2;

    for (int i = 0; i < length; ++i)
    {
        drawGlyph(
            x + i * 6 * scale,
            y,
            scale,
            text[i],
            color
        );
    }
}

static void drawScreen(void)
{
    int height = (int)display_height;
    int width = (int)display_width;

    // Fondo azul oscuro.
    fillRect(
        0, 0,
        width, height,
        0x00101828
    );

    int titleScale = height / 72;
    int subtitleScale = height / 180;
    int smallScale = height / 360;

    if (titleScale < 4) titleScale = 4;
    if (subtitleScale < 2) subtitleScale = 2;
    if (smallScale < 2) smallScale = 2;

    // Titulo principal.
    drawTextCentered(
        "NANA",
        height * 28 / 100 - (7 * titleScale) / 2,
        titleScale,
        0x00F4E9EC
    );

    // Linea rosa.
    fillRect(
        width / 8,
        height * 44 / 100,
        width * 3 / 4,
        5,
        0x00C77991
    );

    // Nombre del proyecto.
    drawTextCentered(
        "NANA-PS3",
        height * 49 / 100,
        subtitleScale,
        0x00C77991
    );

    // Informacion.
    drawTextCentered(
        "PRUEBA GRAFICA",
        height * 65 / 100,
        smallScale,
        0x00F4E9EC
    );

    drawTextCentered(
        "TEXTO BITMAP RSX",
        height * 72 / 100,
        smallScale,
        0x00F4E9EC
    );

    // Indicacion de salida.
    drawTextCentered(
        "PULSA X PARA SALIR",
        height * 85 / 100,
        smallScale,
        0x00C77991
    );

    // Restaurar el area completa.
    rsxSetScissor(
        context,
        0, 0,
        (u16)display_width,
        (u16)display_height
    );
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

    // Dibujar la pantalla solamente una vez.
    rsxSetClearDepthStencil(context, 0xffff);
    drawScreen();

    // Presentar el resultado y esperar el cambio de pantalla.
    flip();

    // A partir de aquí solo leer el mando.
    while (running)
    {
        sysUtilCheckCallback();

        if (ioPadGetInfo(&padinfo) == 0)
        {
            for (int i = 0; i < MAX_PADS; ++i)
            {
                if (padinfo.status[i] &&
                    ioPadGetData(i, &paddata) == 0)
                {
                    if (paddata.BTN_CROSS)
                    {
                        running = 0;
                        break;
                    }
                }
            }
        }

        usleep(16000);
    }

    sysUtilUnregisterCallback(0);

    // La imagen ya fue presentada y no quedan dibujos por enviar.
    rsxFinish(context, 1);

    free(host_addr);

    return 0;
}
