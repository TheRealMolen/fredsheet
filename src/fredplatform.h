#pragma once

#if defined(_WIN32)           
// hide conflicts btw names in raylib and win32 api
#define NOGDI             // All GDI defines and routines
#define NOUSER            // All USER defines and routines

#define VC_EXTRALEAN
#define WIN32_LEAN_AND_MEAN
#include <windows.h>



// win32 annoying pain -- would come in through commdlg.h but that's excluded by NOGDI, needed to avoid conflicts with raylib

#ifdef __cplusplus
extern "C" {            /* Assume C declarations for C++ */
#endif  /* __cplusplus */

//
// Define API decoration for direct importing of DLL references.
//
#ifndef WINCOMMDLGAPI
#if !defined(_COMDLG32_)
#define WINCOMMDLGAPI DECLSPEC_IMPORT
#else
#define WINCOMMDLGAPI
#endif
#endif // WINCOMMDLGAPI

typedef UINT_PTR (CALLBACK *LPOFNHOOKPROC) (HWND, UINT, WPARAM, LPARAM);
typedef struct tagOFNA {
   DWORD        lStructSize;
   HWND         hwndOwner;
   HINSTANCE    hInstance;
   LPCSTR       lpstrFilter;
   LPSTR        lpstrCustomFilter;
   DWORD        nMaxCustFilter;
   DWORD        nFilterIndex;
   LPSTR        lpstrFile;
   DWORD        nMaxFile;
   LPSTR        lpstrFileTitle;
   DWORD        nMaxFileTitle;
   LPCSTR       lpstrInitialDir;
   LPCSTR       lpstrTitle;
   DWORD        Flags;
   WORD         nFileOffset;
   WORD         nFileExtension;
   LPCSTR       lpstrDefExt;
   LPARAM       lCustData;
   LPOFNHOOKPROC lpfnHook;
   LPCSTR       lpTemplateName;
#ifdef _MAC
   LPEDITMENU   lpEditInfo;
   LPCSTR       lpstrPrompt;
#endif
#if (_WIN32_WINNT >= 0x0500)
   void *        pvReserved;
   DWORD        dwReserved;
   DWORD        FlagsEx;
#endif // (_WIN32_WINNT >= 0x0500)
} OPENFILENAMEA, *LPOPENFILENAMEA;
typedef struct tagOFNW {
   DWORD        lStructSize;
   HWND         hwndOwner;
   HINSTANCE    hInstance;
   LPCWSTR      lpstrFilter;
   LPWSTR       lpstrCustomFilter;
   DWORD        nMaxCustFilter;
   DWORD        nFilterIndex;
   LPWSTR       lpstrFile;
   DWORD        nMaxFile;
   LPWSTR       lpstrFileTitle;
   DWORD        nMaxFileTitle;
   LPCWSTR      lpstrInitialDir;
   LPCWSTR      lpstrTitle;
   DWORD        Flags;
   WORD         nFileOffset;
   WORD         nFileExtension;
   LPCWSTR      lpstrDefExt;
   LPARAM       lCustData;
   LPOFNHOOKPROC lpfnHook;
   LPCWSTR      lpTemplateName;
#ifdef _MAC
   LPEDITMENU   lpEditInfo;
   LPCSTR       lpstrPrompt;
#endif
#if (_WIN32_WINNT >= 0x0500)
   void *        pvReserved;
   DWORD        dwReserved;
   DWORD        FlagsEx;
#endif // (_WIN32_WINNT >= 0x0500)
} OPENFILENAMEW, *LPOPENFILENAMEW;
#ifdef UNICODE
typedef OPENFILENAMEW OPENFILENAME;
typedef LPOPENFILENAMEW LPOPENFILENAME;
#else
typedef OPENFILENAMEA OPENFILENAME;
typedef LPOPENFILENAMEA LPOPENFILENAME;
#endif // UNICODE
#define OFN_PATHMUSTEXIST            0x00000800
#define OFN_FILEMUSTEXIST            0x00001000
WINCOMMDLGAPI BOOL  APIENTRY GetOpenFileNameA(LPOPENFILENAMEA);
WINCOMMDLGAPI BOOL  APIENTRY GetOpenFileNameW(LPOPENFILENAMEW);
#ifdef UNICODE
#define GetOpenFileName  GetOpenFileNameW
#else
#define GetOpenFileName  GetOpenFileNameA
#endif // !UNICODE
WINCOMMDLGAPI BOOL  APIENTRY GetSaveFileNameA(LPOPENFILENAMEA);
WINCOMMDLGAPI BOOL  APIENTRY GetSaveFileNameW(LPOPENFILENAMEW);
#ifdef UNICODE
#define GetSaveFileName  GetSaveFileNameW
#else
#define GetSaveFileName  GetSaveFileNameA
#endif // !UNICODE

#ifdef __cplusplus
}
#endif  /* __cplusplus */



#endif

