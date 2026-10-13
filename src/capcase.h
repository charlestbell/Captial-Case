/* Capital case conversion: first letter of each word upper case, the rest lower case.
 * A word is a run of letters/digits; apostrophes inside a word (don't, O'Neil) do not
 * start a new word. The platform supplies IS_WORDCHAR, TO_UPPER and TO_LOWER. */
#ifndef CAPCASE_H
#define CAPCASE_H
#include <wchar.h>

static void capital_case(wchar_t *s)
{
    int in_word = 0;
    for (; *s; s++) {
        if (IS_WORDCHAR(*s)) {
            *s = in_word ? TO_LOWER(*s) : TO_UPPER(*s);
            in_word = 1;
        } else if ((*s == L'\'' || *s == 0x2019) && in_word && IS_WORDCHAR(s[1])) {
            /* apostrophe inside a word: keep the word going */
        } else {
            in_word = 0;
        }
    }
}
#endif
