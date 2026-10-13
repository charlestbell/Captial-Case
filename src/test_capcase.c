/* Logic test, runs on any platform: gcc test_capcase.c -o t && ./t */
#include <wctype.h>
#include <stdio.h>
#include <locale.h>
#define IS_WORDCHAR(c) iswalnum(c)
#define TO_UPPER(c) towupper(c)
#define TO_LOWER(c) towlower(c)
#include "capcase.h"

static int fails;
static void check(const wchar_t *in, const wchar_t *want)
{
    wchar_t buf[256];
    wcscpy(buf, in);
    capital_case(buf);
    if (wcscmp(buf, want)) { fails++; printf("FAIL: '%ls' -> '%ls' (want '%ls')\n", in, buf, want); }
}

int main(void)
{
    setlocale(LC_ALL, "C.UTF-8");
    check(L"hello world", L"Hello World");
    check(L"HELLO WORLD", L"Hello World");
    check(L"hElLo   wOrLd", L"Hello   World");
    check(L"don't stop", L"Don't Stop");
    check(L"o'neil's car", L"O'neil's Car");
    check(L"rock-and-roll", L"Rock-And-Roll");
    check(L"line one\r\nline two", L"Line One\r\nLine Two");
    check(L"(quoted) \"text\"", L"(Quoted) \"Text\"");
    check(L"élan über straße", L"Élan Über Straße");
    check(L"3rd place 42", L"3rd Place 42");
    check(L"' leading", L"' Leading");
    check(L"", L"");
    printf(fails ? "%d failures\n" : "all tests passed\n", fails);
    return fails != 0;
}
