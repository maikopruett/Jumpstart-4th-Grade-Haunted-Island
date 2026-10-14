// Native printing (replaces the Win32 printer DC code of 4G.EXE).
#pragma once

// Prints a DIB (BITMAPINFOHEADER, 256 RGBQUADs, bottom-up 8-bit rows padded
// to 4 bytes) scaled to fit the page. Shows the system print panel.
// Returns true if the job was printed.
bool Print_DIB(const void* dib);
