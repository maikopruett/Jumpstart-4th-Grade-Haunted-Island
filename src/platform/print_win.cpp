// Windows printing: the standard print dialog, then the DIB stretched to fit
// the page (as 4G.EXE itself did with its printer DC).
#include "platform/print.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commdlg.h>

#include <cstdint>
#include <cstring>

bool Print_DIB(const void* dib) {
    const BITMAPINFO* bmi = (const BITMAPINFO*)dib;
    int w = bmi->bmiHeader.biWidth, h = bmi->bmiHeader.biHeight;
    if (w <= 0 || h == 0) return false;
    if (h < 0) h = -h;
    const uint8_t* bits = (const uint8_t*)dib + 0x28 + 0x400;

    PRINTDLGW pd;
    memset(&pd, 0, sizeof pd);
    pd.lStructSize = sizeof pd;
    pd.Flags = PD_RETURNDC | PD_NOPAGENUMS | PD_NOSELECTION;
    pd.nCopies = 1;
    if (!PrintDlgW(&pd) || !pd.hDC) return false;
    HDC dc = pd.hDC;
    if (pd.hDevMode) GlobalFree(pd.hDevMode);
    if (pd.hDevNames) GlobalFree(pd.hDevNames);

    DOCINFOW doc;
    memset(&doc, 0, sizeof doc);
    doc.cbSize = sizeof doc;
    doc.lpszDocName = L"JumpStart 4th Grade";
    bool ok = false;
    if (StartDocW(dc, &doc) > 0) {
        if (StartPage(dc) > 0) {
            int pw = GetDeviceCaps(dc, HORZRES), ph = GetDeviceCaps(dc, VERTRES);
            // Fit to the page keeping the aspect ratio, centred.
            double sx = (double)pw / w, sy = (double)ph / h, s = sx < sy ? sx : sy;
            int dw = (int)(w * s), dh = (int)(h * s);
            SetStretchBltMode(dc, HALFTONE);
            SetBrushOrgEx(dc, 0, 0, nullptr);
            ok = StretchDIBits(dc, (pw - dw) / 2, (ph - dh) / 2, dw, dh, 0, 0, w, h, bits, bmi, DIB_RGB_COLORS,
                               SRCCOPY) != GDI_ERROR;
            EndPage(dc);
        }
        if (ok) EndDoc(dc);
        else AbortDoc(dc);
    }
    DeleteDC(dc);
    return ok;
}
