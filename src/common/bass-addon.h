/*
	BASS 2.4 add-on C/C++ header file
	Copyright (c) 2003-2012 Un4seen Developments Ltd.
*/

#ifndef BASS_ADDON_H
#define BASS_ADDON_H

#include "bass.h"

#ifdef __APPLE__
#include <TargetConditionals.h>
#endif
#ifdef ANDROID
#include <jni.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef void *BASSFILE;

typedef struct {
	DWORD flags; // ADDON_xxx flags
	void (WINAPI *Free)(void *inst);
	QWORD (WINAPI *GetLength)(void *inst, DWORD mode);
	const char *(WINAPI *GetTags)(void *inst, DWORD tags); // optional
	QWORD (WINAPI *GetFilePosition)(void *inst, DWORD mode); // optional
	void (WINAPI *GetInfo)(void *inst, BASS_CHANNELINFO *info);
	BOOL (WINAPI *CanSetPosition)(void *inst, QWORD pos, DWORD mode);
	QWORD (WINAPI *SetPosition)(void *inst, QWORD pos, DWORD mode);
	QWORD (WINAPI *GetPosition)(void *inst, QWORD pos, DWORD mode); // optional
	HSYNC (WINAPI *SetSync)(void *inst, DWORD type, QWORD param, SYNCPROC *proc, void *user); // not required if not setting syncs
	void (WINAPI *RemoveSync)(void *inst, HSYNC sync); // optional
	BOOL (WINAPI *CanResume)(void *inst); // optional
	DWORD (WINAPI *SetFlags)(void *inst, DWORD flags); // optional
	BOOL (WINAPI *Attribute)(void *inst, DWORD attrib, float *value, BOOL set); // optional
} ADDON_FUNCTIONS;

#define ADDON_OWNPOS	1 // handles all position tracking (including POS/END syncs)
#define ADDON_DECODETO	2 // supports BASS_POS_DECODETO seeking

typedef struct {
	void (WINAPI *Free)(void *inst);
#ifdef ANDROID
	BOOL (WINAPI *SetParameters)(void *inst, const void *param, JNIEnv *env);
	BOOL (WINAPI *GetParameters)(void *inst, void *param, JNIEnv *env);
#else
	BOOL (WINAPI *SetParameters)(void *inst, const void *param);
	BOOL (WINAPI *GetParameters)(void *inst, void *param);
#endif
	BOOL (WINAPI *Reset)(void *inst);
} ADDON_FUNCTIONS_FX;

typedef HSTREAM (CALLBACK STREAMCREATEPROC)(BASSFILE file, DWORD flags); // BASS_StreamCreateFile/User/URL plugin function
typedef HSTREAM (CALLBACK STREAMCREATEURLPROC)(const char *url, DWORD offset, DWORD flags, DOWNLOADPROC *proc, void *user); // BASS_StreamCreateURL plugin function (unsupported URI scheme)
typedef BOOL (CALLBACK BASSCONFIGPROC)(DWORD option, DWORD flags, void *value); // config plugin function
typedef HFX (CALLBACK BASSFXPROC)(DWORD chan, DWORD type, int priority); // FX plugin function

// BASSCONFIGPROC flags
#define BASSCONFIG_SET	1 // set the config (otherwise get it)
#define BASSCONFIG_PTR	2 // value is a pointer

// RegisterPlugin modes
#define PLUGIN_CONFIG_ADD		0 // add a config plugin
#define PLUGIN_CONFIG_REMOVE	1 // remove a config plugin
#define PLUGIN_FX_ADD			2 // add an FX plugin
#define PLUGIN_FX_REMOVE		3 // remove an FX plugin

typedef struct {
	void (WINAPI *SetError)(int error); // set error code
	void (WINAPI *RegisterPlugin)(void *proc, DWORD mode); // add/remove a plugin function
	HSTREAM (WINAPI *CreateStream)(DWORD freq, DWORD chan, DWORD flags, STREAMPROC *proc, void *inst, const ADDON_FUNCTIONS *funcs); // create a stream
	HFX (WINAPI *SetFX)(DWORD chan, DSPPROC *proc, void *inst, int priority, const ADDON_FUNCTIONS_FX *funcs); // set FX on a channel
	void *(WINAPI *GetInst)(DWORD handle, const void *funcs); // get stream or FX instance data

	void *(WINAPI *GetLock)(HSTREAM handle); // get stream access lock
	HSYNC (WINAPI *NewSync)(HSTREAM handle, DWORD type, SYNCPROC *proc, void *user); // add a sync to a channel
	BOOL (WINAPI *TriggerSync)(HSTREAM handle, HSYNC sync, QWORD pos, DWORD data); // trigger a sync
	QWORD (WINAPI *GetCount)(HSTREAM handle, BOOL output); // get raw count (output=0) or current output position (output=1)
	QWORD (WINAPI *GetPosition)(HSTREAM handle, QWORD count, DWORD mode); // get raw "count" translated to source position

	struct { // file functions
		BASSFILE (WINAPI *Open)(BOOL mem, const void *file, QWORD offset, QWORD length, DWORD flags, DWORD exflags); // open a file
		BASSFILE (WINAPI *OpenURL)(const char *url, DWORD offset, DWORD flags, DOWNLOADPROC *proc, void *user, DWORD exflags); // open a URL
		BASSFILE (WINAPI *OpenUser)(DWORD system, DWORD flags, const BASS_FILEPROCS *proc, void *user, DWORD exflags); // open a custom file
		void (WINAPI *Close)(BASSFILE file); // close an opened file
		const void *(WINAPI *GetFileName)(BASSFILE file, BOOL *unicode); // get the filename/url (NULL=memory/custom file)
		BOOL (WINAPI *SetStream)(BASSFILE file, HSTREAM handle); // set stream handle (for auto closing and META/DOWNLOAD syncs)
		DWORD (WINAPI *GetFlags)(BASSFILE file); // get BASSFILE_xxx flags
		void (WINAPI *SetFlags)(BASSFILE file, DWORD flags); // set BASSFILE_xxx flags (BASSFILE_RESTRATE/NOLIMIT only)
		DWORD (WINAPI *Read)(BASSFILE file, void *buf, DWORD len); // read from file
		BOOL (WINAPI *Seek)(BASSFILE file, QWORD pos); // seek in file
		QWORD (WINAPI *GetPos)(BASSFILE file, DWORD mode); // get file position (mode=BASS_FILEPOS_xxx)
		BOOL (WINAPI *Eof)(BASSFILE file); // End of file?
		const char *(WINAPI *GetTags)(BASSFILE file, DWORD tags); // get tags (ID3/ID3V2/LYRICS3/RIFF_INFO/HTTP/ICY/META)
		// net/buffered-only stuff
		BOOL (WINAPI *StartThread)(BASSFILE f, DWORD bitrate, DWORD offset); // start download thread (bitrate in bytes/sec)
		BOOL (WINAPI *CanResume)(BASSFILE file); // enough data buffered to resume?
	} file;

	struct { // sample data processing functions (len=samples, dst=src is ok)
		void (WINAPI *Float2Int)(const float *src, void *dst, DWORD len, DWORD res); // convert floating-point data to 8/16-bit (res=1/2)
		void (WINAPI *Int2Float)(const void *src, float *dst, DWORD len, DWORD res); // convert 8/16/24/32-bit (res=1/2/3/4) to floating-point
		void (WINAPI *Swap)(const void *src, void *dst, DWORD len, DWORD res); // swap byte order of 16/32-bit (res=2/4) data
	} data;
} BASS_FUNCTIONS;

// file "flags"
#define BASSFILE_BUFFERED	1	// net/buffered file
#define BASSFILE_NOLIMIT	2	// enable reading beyond end of audio data (into tags)
#define BASSFILE_BLOCK		BASS_STREAM_BLOCK
#define BASSFILE_RESTRATE	BASS_STREAM_RESTRATE
#define BASSFILE_UNICODE	BASS_UNICODE

// file "exflags"
#define BASSFILE_EX_TAGS	1	// read tags
#define BASSFILE_EX_MMAP	4	// memory-map file

// additional "tags" type
#define BASS_TAG_MEMORY		-1	// get file memory address

typedef const void *(WINAPI BASSPLUGIN)(DWORD face); // plugin interface function

// BASSplugin "faces"
#define BASSPLUGIN_INFO			0 // BASS_PLUGININFO
#define BASSPLUGIN_CREATE		1 // STREAMCREATEPROC
#define BASSPLUGIN_CREATEURL	2 // STREAMCREATEURLPROC

#define BASS_FREQ_INIT		0x80000001 // BASS_Init freq
#define BASS_FREQ_CURRENT	0x80000002 // device's current rate (or BASS_Init freq)

#define BASS_TAG_DOWNLOADPROC	0x80000001 // pointer to DOWNLOADPROC and user parameter

#define BASS_SYNC_EX 0x10000000
typedef void (CALLBACK SYNCPROCEX)(HSYNC handle, DWORD channel, DWORD data, void *user, QWORD pos);

#define BASS_STREAMPROC_REPEAT	0x40000000 // call STREAMPROC again for any remainder

#define BASS_POS_RESTART	0x80000000 // restarting

#define BASS_CONFIG_ADDON		0x8000
#define BASS_CONFIG_INUPDATE	0x8001
#define BASS_CONFIG_ADDON_JNI	0x8002

#ifndef GetBassFunc
#if defined(_WIN32) && !defined(_WIN32_WCE)
extern __declspec(dllimport) const BASS_FUNCTIONS _;
#define bassfunc (&_)
#define GetBassFunc() 1
#elif defined(__APPLE__) && !TARGET_OS_IPHONE
extern const BASS_FUNCTIONS _;
#define bassfunc (&_)
#define GetBassFunc() 1
#else
extern const BASS_FUNCTIONS *bassfunc;
#define GetBassFunc() (bassfunc=(BASS_FUNCTIONS*)BASS_GetConfigPtr(BASS_CONFIG_ADDON))
#endif

// SetError macros
#define noerror() return (bassfunc->SetError(BASS_OK),TRUE)
#define noerrorn(n) return (bassfunc->SetError(BASS_OK),n)
#define error(n) return (bassfunc->SetError(n),FALSE) // error = 0/NULL
#define errorn(n) return (bassfunc->SetError(n),-1) // error = -1
#define errorp(n,t) return (bassfunc->SetError(n),(t*)NULL) // error = NULL
#endif

#ifdef ANDROID
typedef struct {
	jobject object;
	jobject user;
	jmethodID method;
} JCALLBACKSTUFF;

typedef struct {
	JNIEnv *(*GetEnv)();

	jstring (*NewString)(JNIEnv *env, const char *s);

	void *(*GetByteBuffer)(JNIEnv *env, jobject buffer, jbyteArray *barray);
	DWORD (*SetBufferFreeSync)(JNIEnv *env, DWORD handle, jobject buffer, jbyteArray barray, void *mem);

	// callback stuff
	struct {
		void *(*NewDownloadProc)(JNIEnv *env, jobject proc, jobject user, const DOWNLOADPROC **nproc);
		void *(*NewFileProcs)(JNIEnv *env, jobject procs, jobject user, const BASS_FILEPROCS **nprocs);
		void (*Free)(void *callback);
		DWORD (*SetFreeSync)(JNIEnv *env, DWORD handle, void *callback);
		void *(*New)(JNIEnv *env, jobject proc, jobject user, jmethodID method);
	} callback;

} BASSJNI_FUNCTIONS;

#ifndef GetJniFunc
extern const BASSJNI_FUNCTIONS *jnifunc;
#define GetJniFunc() (jnifunc=(BASSJNI_FUNCTIONS*)BASS_GetConfigPtr(BASS_CONFIG_ADDON_JNI))
#endif

#define BASS_ERROR_JAVA_CLASS 2000
#endif

#ifdef __cplusplus
}
#endif

#endif
