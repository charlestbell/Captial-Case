#ifndef UNICODE
#define UNICODE
#endif
#ifndef _UNICODE
#define _UNICODE
#endif
#include <windows.h>
#include <stdlib.h>

static wchar_t win_upper(wchar_t c) { return (wchar_t)(ULONG_PTR)CharUpperW((LPWSTR)(ULONG_PTR)c); }
static wchar_t win_lower(wchar_t c) { return (wchar_t)(ULONG_PTR)CharLowerW((LPWSTR)(ULONG_PTR)c); }
#define IS_WORDCHAR(c) IsCharAlphaNumericW(c)
#define TO_UPPER(c) win_upper(c)
#define TO_LOWER(c) win_lower(c)
#include "capcase.h"

enum { ID_INPUT = 101, ID_CONVERT, ID_OUTPUT, ID_COPY, ID_LBL_IN, ID_LBL_OUT, ID_PASTE };

static HWND hInput, hOutput, hConvert, hCopy, hLblIn, hLblOut, hPaste;
static HFONT hFont;
static UINT dpi = 96;

static int S(int v) { return MulDiv(v, dpi, 96); }

static wchar_t *get_text(HWND h)
{
    int n = GetWindowTextLengthW(h);
    wchar_t *buf = malloc((n + 1) * sizeof(wchar_t));
    if (buf) GetWindowTextW(h, buf, n + 1);
    return buf;
}

static void do_convert(void)
{
    wchar_t *t = get_text(hInput);
    if (!t) return;
    capital_case(t);
    SetWindowTextW(hOutput, t);
    free(t);
}

static void do_copy(HWND hwnd)
{
    wchar_t *t = get_text(hOutput);
    if (!t) return;
    size_t bytes = (wcslen(t) + 1) * sizeof(wchar_t);
    if (OpenClipboard(hwnd)) {
        EmptyClipboard();
        HGLOBAL mem = GlobalAlloc(GMEM_MOVEABLE, bytes);
        if (mem) {
            memcpy(GlobalLock(mem), t, bytes);
            GlobalUnlock(mem);
            if (!SetClipboardData(CF_UNICODETEXT, mem)) GlobalFree(mem);
        }
        CloseClipboard();
        SetWindowTextW(hCopy, L"Copied!");
        SetTimer(hwnd, 1, 2000, NULL);
    }
    free(t);
}

/* Paste: clipboard text -> input, convert, then copy the result back to the clipboard. */
static void do_paste(HWND hwnd)
{
    wchar_t *t = NULL;
    if (IsClipboardFormatAvailable(CF_UNICODETEXT) && OpenClipboard(hwnd)) {
        HGLOBAL mem = GetClipboardData(CF_UNICODETEXT);
        const wchar_t *src = mem ? GlobalLock(mem) : NULL;
        if (src) {
            t = _wcsdup(src);
            GlobalUnlock(mem);
        }
        CloseClipboard();
    }
    if (!t) {
        SetWindowTextW(hPaste, L"No text on clipboard");
        SetTimer(hwnd, 2, 1500, NULL);
        return;
    }
    SetWindowTextW(hInput, t);
    free(t);
    do_convert();
    do_copy(hwnd);
}

static void apply_font(void)
{
    NONCLIENTMETRICSW ncm = { sizeof(ncm) };
    SystemParametersInfoW(SPI_GETNONCLIENTMETRICS, sizeof(ncm), &ncm, 0);
    ncm.lfMessageFont.lfHeight = -S(15);
    if (hFont) DeleteObject(hFont);
    hFont = CreateFontIndirectW(&ncm.lfMessageFont);
    HWND all[] = { hInput, hOutput, hConvert, hCopy, hLblIn, hLblOut, hPaste };
    for (int i = 0; i < 7; i++) SendMessageW(all[i], WM_SETFONT, (WPARAM)hFont, TRUE);
}

static void layout(HWND hwnd)
{
    RECT rc;
    GetClientRect(hwnd, &rc);
    int m = S(12), w = rc.right - 2 * m, lblH = S(20), btnH = S(32), btnW = S(150), gap = S(8);
    int boxH = (rc.bottom - 2 * m - 2 * lblH - 3 * btnH - 5 * gap) / 2;
    if (boxH < S(40)) boxH = S(40);
    int y = m;
    MoveWindow(hPaste, m, y, btnW, btnH, TRUE); y += btnH + gap;
    MoveWindow(hLblIn, m, y, w, lblH, TRUE);   y += lblH;
    MoveWindow(hInput, m, y, w, boxH, TRUE);   y += boxH + gap;
    MoveWindow(hConvert, m, y, btnW, btnH, TRUE); y += btnH + gap;
    MoveWindow(hLblOut, m, y, w, lblH, TRUE);  y += lblH;
    MoveWindow(hOutput, m, y, w, boxH, TRUE);  y += boxH + gap;
    MoveWindow(hCopy, m, y, btnW, btnH, TRUE);
}

static HWND make(HWND p, const wchar_t *cls, const wchar_t *txt, DWORD style, int id)
{
    return CreateWindowExW(wcscmp(cls, L"EDIT") == 0 ? WS_EX_CLIENTEDGE : 0, cls, txt,
                           WS_CHILD | WS_VISIBLE | style, 0, 0, 0, 0, p,
                           (HMENU)(INT_PTR)id, GetModuleHandleW(NULL), NULL);
}

/* Ctrl+A selects all in the edit boxes (multi-line edits don't do this by default). */
static WNDPROC oldEditProc;
static LRESULT CALLBACK EditProc(HWND h, UINT msg, WPARAM wp, LPARAM lp)
{
    if (msg == WM_KEYDOWN && wp == 'A' && (GetKeyState(VK_CONTROL) & 0x8000)) {
        SendMessageW(h, EM_SETSEL, 0, -1);
        return 0;
    }
    if (msg == WM_CHAR && wp == 1) return 0; /* swallow the Ctrl+A beep */
    return CallWindowProcW(oldEditProc, h, msg, wp, lp);
}

static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    switch (msg) {
    case WM_CREATE: {
        DWORD ml = ES_MULTILINE | ES_AUTOVSCROLL | WS_VSCROLL | WS_TABSTOP;
        hPaste   = make(hwnd, L"BUTTON", L"Paste && convert", BS_PUSHBUTTON | WS_TABSTOP, ID_PASTE);
        hLblIn   = make(hwnd, L"STATIC", L"Input", 0, ID_LBL_IN);
        hInput   = make(hwnd, L"EDIT", L"", ml | ES_WANTRETURN, ID_INPUT);
        hConvert = make(hwnd, L"BUTTON", L"Convert", BS_DEFPUSHBUTTON | WS_TABSTOP, ID_CONVERT);
        hLblOut  = make(hwnd, L"STATIC", L"Output", 0, ID_LBL_OUT);
        hOutput  = make(hwnd, L"EDIT", L"", ml | ES_READONLY, ID_OUTPUT);
        hCopy    = make(hwnd, L"BUTTON", L"Copy to clipboard", BS_PUSHBUTTON | WS_TABSTOP, ID_COPY);
        oldEditProc = (WNDPROC)SetWindowLongPtrW(hInput, GWLP_WNDPROC, (LONG_PTR)EditProc);
        SetWindowLongPtrW(hOutput, GWLP_WNDPROC, (LONG_PTR)EditProc);
        apply_font();
        layout(hwnd);
        SetFocus(hInput);
        return 0;
    }
    case WM_SIZE: layout(hwnd); return 0;
    case WM_GETMINMAXINFO:
        ((MINMAXINFO *)lp)->ptMinTrackSize.x = S(320);
        ((MINMAXINFO *)lp)->ptMinTrackSize.y = S(320);
        return 0;
    case WM_DPICHANGED: {
        dpi = HIWORD(wp);
        RECT *r = (RECT *)lp;
        apply_font();
        SetWindowPos(hwnd, NULL, r->left, r->top, r->right - r->left, r->bottom - r->top,
                     SWP_NOZORDER | SWP_NOACTIVATE);
        return 0;
    }
    case WM_CTLCOLORSTATIC:
        if ((HWND)lp == hOutput) { /* keep the read-only output box white */
            SetBkColor((HDC)wp, GetSysColor(COLOR_WINDOW));
            return (LRESULT)GetSysColorBrush(COLOR_WINDOW);
        }
        break;
    case WM_COMMAND:
        if (LOWORD(wp) == ID_CONVERT) do_convert();
        else if (LOWORD(wp) == ID_COPY) do_copy(hwnd);
        else if (LOWORD(wp) == ID_PASTE) do_paste(hwnd);
        return 0;
    case WM_TIMER:
        KillTimer(hwnd, wp);
        if (wp == 1) SetWindowTextW(hCopy, L"Copy to clipboard");
        else SetWindowTextW(hPaste, L"Paste && convert");
        return 0;
    case WM_DESTROY:
        if (hFont) DeleteObject(hFont);
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcW(hwnd, msg, wp, lp);
}

typedef UINT (WINAPI *GetDpiForWindowFn)(HWND);

int WINAPI wWinMain(HINSTANCE hInst, HINSTANCE hPrev, LPWSTR cmd, int show)
{
    (void)hPrev; (void)cmd;
    WNDCLASSEXW wc = { sizeof(wc) };
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInst;
    wc.hCursor = LoadCursorW(NULL, (LPCWSTR)IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
    wc.hIcon = (HICON)LoadImageW(hInst, MAKEINTRESOURCEW(1), IMAGE_ICON,
                                 GetSystemMetrics(SM_CXICON), GetSystemMetrics(SM_CYICON), 0);
    wc.hIconSm = (HICON)LoadImageW(hInst, MAKEINTRESOURCEW(1), IMAGE_ICON,
                                   GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), 0);
    wc.lpszClassName = L"CapitalCaseWnd";
    RegisterClassExW(&wc);

    HDC screen = GetDC(NULL);
    dpi = GetDeviceCaps(screen, LOGPIXELSX);
    ReleaseDC(NULL, screen);

    HWND hwnd = CreateWindowExW(0, wc.lpszClassName, L"Capital Case", WS_OVERLAPPEDWINDOW,
                                CW_USEDEFAULT, CW_USEDEFAULT, S(520), S(520),
                                NULL, NULL, hInst, NULL);
    GetDpiForWindowFn gdfw = (GetDpiForWindowFn)GetProcAddress(GetModuleHandleW(L"user32.dll"), "GetDpiForWindow");
    if (gdfw) {
        UINT d = gdfw(hwnd);
        if (d && d != dpi) {
            dpi = d;
            apply_font();
            SetWindowPos(hwnd, NULL, 0, 0, S(520), S(520), SWP_NOMOVE | SWP_NOZORDER);
        }
    }
    ShowWindow(hwnd, show);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessageW(&msg, NULL, 0, 0)) {
        if (!IsDialogMessageW(hwnd, &msg)) {
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }
    }
    return (int)msg.wParam;
}
