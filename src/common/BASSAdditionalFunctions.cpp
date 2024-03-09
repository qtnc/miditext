#include "wxUtils.hpp"
#include "bass.h"
#include "bassenc.h"
#include<cstdlib>
#include<cstring>
#include<string>
#include<vector>
#include "stringUtils.hpp"
using namespace std;

bool BASS_SimpleInit (int device) {
return BASS_Init(device, 48000, 0, 0, 0) || BASS_ERROR_ALREADY==BASS_ErrorGetCode();
}

bool BASS_RecordSimpleInit (int device) {
return BASS_RecordInit(device) || BASS_ERROR_ALREADY==BASS_ErrorGetCode();
}

vector<pair<int,string>> getDeviceList ( BOOL(*CALLBACK func)(DWORD, BASS_DEVICEINFO*) , bool includeLoopback = false) {
vector<pair<int,string>> list;
BASS_DEVICEINFO info;
for (int i=0; func(i, &info); i++) {
if (!(info.flags&BASS_DEVICE_ENABLED)) continue;
if ((info.flags&BASS_DEVICE_LOOPBACK) && !includeLoopback) continue;
list.push_back(make_pair(i, info.name));
}
return list;
}

vector<pair<int,string>> BASS_GetDeviceList (bool includeLoopback = false) {
return getDeviceList(BASS_GetDeviceInfo, includeLoopback);
}

vector<pair<int,string>> BASS_RecordGetDeviceList (bool includeLoopback = false) {
return getDeviceList(BASS_RecordGetDeviceInfo, includeLoopback);
}

static void CALLBACK DSPCopyProc (HDSP dsp, DWORD source, void* buffer, DWORD length, void* dest) {
BASS_StreamPutData(reinterpret_cast<uintptr_t>(dest), buffer, length);
}

static void CALLBACK SyncFreeProc (HSYNC sync, DWORD channel, DWORD param, void* dest) {
BASS_StreamFree(reinterpret_cast<uintptr_t>(dest));
}

DWORD BASS_StreamCreateCopy (DWORD source, bool decode) {
BASS_CHANNELINFO info;
BASS_ChannelGetInfo(source, &info);
DWORD dest = BASS_StreamCreate(info.freq, info.chans, BASS_SAMPLE_FLOAT | (decode? BASS_STREAM_DECODE : BASS_STREAM_AUTOFREE), STREAMPROC_PUSH, nullptr);
BASS_ChannelSetDSP(source, DSPCopyProc, reinterpret_cast<void*>(dest), 0);
BASS_ChannelSetSync(source, BASS_SYNC_FREE, 0, SyncFreeProc, reinterpret_cast<void*>(dest));
return dest;
}


int BASS_CastGetListenerCount (DWORD encoder) {
const char *stats;
int listeners = -1;
if (stats=BASS_Encode_CastGetStats(encoder, BASS_ENCODE_STATS_SHOUT, NULL)) {
    const char *t=strstr(stats, "<CURRENTLISTENERS>"); // Shoutcast listener count
    if (t) listeners=atoi(t+18);
} else if (stats=BASS_Encode_CastGetStats(encoder, BASS_ENCODE_STATS_ICE, NULL)) {
    const char *t=strstr(stats, "<Listeners>"); // Icecast listener count
    if (t) listeners=atoi(t+11);
}
return listeners;
}

string BASS_BuildWildcardFilter (unsigned long* pluginList, size_t pluginCount) {
vector<pair<string,string>> formats = {
{ "MPEG1 Layer3", "*.mp3" },
{ "OGG Vorbis", "*.ogg" },
{ "Apple interchange file format", "*.aif;*.aiff;*.aifc" },
{ "Wave Microsoft", "*.wav" },
{ "MPEG1 Layer1/2", "*.mp1;*.mp2" },
{ "Impulse tracker module", "*.it;*.itz" },
{ "ScreamTracker3 Module", "*.s3m;*.s3z" },
{ "ProTracker Module", "*.mod;*.mdz;*.miz" },
{ "MultiTracker module", "*.mtm;*.nst" },
{ "Unreal extended module", "*.umx" },
{ "BASS MP3 compressed module", "*.mo3" },
{ "PLS playlist", "*.pls" },
{ "M3U playlist", "*.m3u;*.m3u8" },
{ "Zip files", "*.zip" }
};
for (size_t i=0; i<pluginCount; i++) {
auto info = BASS_PluginGetInfo(pluginList[i]);
if (!info) continue;
for (size_t j=0; j<info->formatc; j++) {
auto& f = info->formats[j];
if (!f.name || !f.exts) continue;
formats.push_back(make_pair(f.name, f.exts));
}}
string allExtensions;
for (auto& p: formats) {
if (!allExtensions.empty()) allExtensions+=';';
allExtensions+=p.second;
}
string allfmt = "All files (*.*)|*.*|All supported files|" + allExtensions;
for (auto& p: formats) {
allfmt += '|';
allfmt += p.first;
allfmt += " (";
allfmt += p.second;
allfmt += ")|";
allfmt += p.second;
}
return allfmt;
}
