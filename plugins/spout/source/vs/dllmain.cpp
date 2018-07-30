// dllmain.cpp : Defines the entry point for the DLL application.
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>
#include "../vs/spout/Spout.h"
#include <gl/gl.h>
#include <gl/glu.h> // For glerror

#define G_DLLEXPORT extern "C" __declspec(dllexport)

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

G_DLLEXPORT void *GidCreateSender(const char *name, int width, int height) {
	SpoutSender *s=new SpoutSender();
	s->SetMemoryShareMode(true);
	s->CreateSender(name,width,height);
	return s;
}

G_DLLEXPORT void GidDestroySender(void *si)
{
	SpoutSender *s=(SpoutSender *)si;
	s->ReleaseSender();
	delete s;
}

G_DLLEXPORT bool GidSendTexture(void *si,GLuint tid,int width,int height,GLuint fbo)
{
	SpoutSender *s=(SpoutSender *)si;
	return s->SendTexture(tid, GL_TEXTURE_2D, width, height, true,fbo);
}

G_DLLEXPORT void *GidCreateReceiver() {
	SpoutReceiver *s=new SpoutReceiver();
	s->SetMemoryShareMode(true);
	return s;
}

G_DLLEXPORT bool GidBindReceiver(void *si,char *name,unsigned int *width,unsigned int *height, bool active) {
	SpoutReceiver *s=(SpoutReceiver *)si;
	unsigned int lwidth=0;
	unsigned int lheight=0;
	bool res=s->CreateReceiver(name,lwidth,lheight,active);
	*width=lwidth;
	*height=lheight;
	return res;
}

G_DLLEXPORT void GidDestroyReceiver(void *si)
{
	SpoutReceiver *s=(SpoutReceiver *)si;
	s->ReleaseReceiver();
	delete s;
}

G_DLLEXPORT bool GidReceiveTexture(void *si,char *sname,GLuint tid,unsigned int *width,unsigned int *height,GLuint fbo)
{
	SpoutReceiver *s=(SpoutReceiver *)si;
	unsigned int lwidth=*width;
	unsigned int lheight=*height;
	bool res=s->ReceiveTexture(sname,lwidth,lheight,tid, GL_TEXTURE_2D, true,fbo);
	*width=lwidth;
	*height=lheight;
	return res;
}

struct SenderDetail {
	unsigned int width;
	unsigned int height;
	char name[256];
};

G_DLLEXPORT int GidFindSenders(void *si,int bcount,struct SenderDetail *buffer)
{
	SpoutReceiver *s=(SpoutReceiver *)si;
	int sc=s->GetSenderCount();
	if (bcount>sc) bcount=sc;
	for (int i=0;i<bcount;i++) {
		unsigned int lwidth,lheight;
		HANDLE hShareHandle;
		DWORD dwFormat;
		s->GetSenderName(i, buffer[i].name, 256);
		s->GetSenderInfo(buffer[i].name, lwidth, lheight, hShareHandle, dwFormat);
		buffer[i].width=lwidth;
		buffer[i].height=lheight;
	}
	return sc;
}
