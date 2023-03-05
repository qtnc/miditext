/*
	BASSenc_FLAC 2.4 C/C++ header file
	Copyright (c) 2017-2018 Un4seen Developments Ltd.

	See the BASSENC_FLAC.CHM file for more detailed documentation
*/

#ifndef BASSENC_FLAC_H
#define BASSENC_FLAC_H

#include "bassenc.h"

#if BASSVERSION!=0x204
#error conflicting BASS and BASSenc_FLAC versions
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifndef BASSENCFLACDEF
#define BASSENCFLACDEF(f) WINAPI f
#endif

DWORD BASSENCFLACDEF(BASS_Encode_FLAC_GetVersion)();

HENCODE BASSENCFLACDEF(BASS_Encode_FLAC_Start)(DWORD handle, const char *options, DWORD flags, ENCODEPROCEX *proc, void *user);
HENCODE BASSENCFLACDEF(BASS_Encode_FLAC_StartFile)(DWORD handle, const char *options, DWORD flags, const char *filename);

#ifdef __cplusplus
}

#ifdef _WIN32
static inline HENCODE BASS_Encode_FLAC_Start(DWORD handle, const WCHAR *options, DWORD flags, ENCODEPROCEX *proc, void *user)
{
	return BASS_Encode_FLAC_Start(handle, (const char*)options, flags|BASS_UNICODE, proc, user);
}

static inline HENCODE BASS_Encode_FLAC_StartFile(DWORD handle, const WCHAR *options, DWORD flags, const WCHAR *filename)
{
	return BASS_Encode_FLAC_StartFile(handle, (const char*)options, flags|BASS_UNICODE, (const char*)filename);
}
#endif
#endif

#endif
