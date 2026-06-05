#ifndef SCINTILLA_H
#define SCINTILLA_H

#ifdef __cplusplus
extern "C" {
#endif

#if defined(_WIN32)
int Scintilla_RegisterClasses(void *hInstance);
int Scintilla_ReleaseResources(void);
#endif

#ifdef __cplusplus
}
#endif

#include <stdint.h>
typedef uintptr_t uptr_t;
typedef intptr_t sptr_t;

#include "Sci_Position.h"

typedef sptr_t (*SciFnDirect)(sptr_t ptr, unsigned int iMessage, uptr_t wParam, sptr_t lParam);
typedef sptr_t (*SciFnDirectStatus)(sptr_t ptr, unsigned int iMessage, uptr_t wParam, sptr_t lParam, int *pStatus);

#define INVALID_POSITION -1
#define SCI_START 2000
#define SCI_OPTIONAL_START 3000
#define SCI_LEXER_START 4000
#define SCI_ADDTEXT 2001
#define SCI_INSERTTEXT 2003
#define SCI_CLEARALL 2004
#define SCI_DELETERANGE 2645
#define SCI_GETLENGTH 2006
#define SCI_GETCHARAT 2007
#define SCI_GETCURRENTPOS 2008
#define SCI_GETANCHOR 2009
#define SCI_GETSTYLEAT 2010
#define SCI_REDO 2011
#define SCI_SETUNDOCOLLECTION 2012
#define SCI_SELECTALL 2013
#define SCI_SETSAVEPOINT 2014
#define SCI_GETSELECTIONSTART 2143
#define SCI_GETSELECTIONEND 2145
#define SCI_GETSELTEXT 2161
#define SCI_REPLACESEL 2170
#define SCI_GETTEXT 2182
#define SCI_GETTEXTLENGTH 2183
#define SCI_SETTEXT 2181
#define SCI_SETCURRENTPOS 2141
#define SCI_GETLINECOUNT 2154
#define SCI_GETLINE 2153
#define SCI_GOTOLINE 2024
#define SCI_GOTOPOS 2025
#define SCI_GETCURLINE 2027
#define SCI_GETSELECTIONMODE 2423
#define SCI_SETSELECTIONMODE 2422
#define SCI_GETEDGECOLUMN 2360
#define SCI_SETEDGECOLUMN 2361
#define SCI_GETEDGEMODE 2362
#define SCI_SETEDGEMODE 2363
#define SCI_GETZOOM 2374
#define SCI_SETZOOM 2373

#define SC_SEL_STREAM 0
#define SC_SEL_RECTANGLE 1
#define SC_SEL_LINES 2

#define SCI_GETTEXTRANGEFULL 2039

struct Sci_CharacterRangeFull {
    Sci_Position cpMin;
    Sci_Position cpMax;
};

struct Sci_TextRangeFull {
    Sci_CharacterRangeFull chrg;
    char *lpstrText;
};

enum { SC_MOD_INSERTTEXT = 0x1, SC_MOD_DELETETEXT = 0x2 };
enum { SCN_MODIFIED = 2008 };

struct Sci_NotifyHeader {
    void *hwndFrom;
    uptr_t idFrom;
    unsigned int code;
};

struct SCNotification {
    Sci_NotifyHeader nmhdr;
    Sci_Position position;
    int ch;
    int modifiers;
    int modificationType;
    const char *text;
    Sci_Position length;
    Sci_Position linesAdded;
    int message;
    uptr_t wParam;
    sptr_t lParam;
    Sci_Position line;
    int foldLevelNow;
    int foldLevelPrev;
    int margin;
    int listType;
    int x;
    int y;
};

#endif
