
#include <Windows.h>
#include "fileio.h"

//////////////////////////////////////////////////////////////////////////

bool FileSelectorLoadRFX(char *pFileName)
{
	static OPENFILENAME ofn;
	wchar_t szFileName[MAX_PATH] = L"";

	memset(&ofn,0, sizeof(ofn));

	ofn.lStructSize = sizeof(ofn); // SEE NOTE BELOW
	ofn.hwndOwner = NULL;//hWnd;
	ofn.lpstrFilter = L"SFX Files (*.sfx)\0*.sfx\0All Files (*.*)\0*.*\0";
	ofn.lpstrFile = szFileName;
	ofn.nMaxFile = MAX_PATH;
	ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
	ofn.lpstrDefExt = L"sfx";

	if(GetOpenFileName(&ofn))
	{
		wchar_t *pSrc = &szFileName[0];
		char *pDest = pFileName;

		while (*pSrc)
		{
			*pDest++ = *pSrc++;
		}
		*pDest = 0;

		return true;
	}

	return false;
}

//////////////////////////////////////////////////////////////////////////

bool FileSelectorSaveRFX(char *pFileName)
{
	static OPENFILENAME ofn;
	wchar_t szFileName[MAX_PATH] = L"";

	memset(&ofn,0, sizeof(ofn));

	ofn.lStructSize = sizeof(ofn); // SEE NOTE BELOW
	ofn.hwndOwner = NULL;//hWnd;
	ofn.lpstrFilter = L"SFX Files (*.sfx)\0*.sfx\0All Files (*.*)\0*.*\0";
	ofn.lpstrFile = szFileName;
	ofn.nMaxFile = MAX_PATH;
	ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
	ofn.lpstrDefExt = L"sfx";

	if(GetSaveFileName(&ofn))
	{
		wchar_t *pSrc = &szFileName[0];
		char *pDest = pFileName;

		while (*pSrc)
		{
			*pDest++ = *pSrc++;
		}
		*pDest = 0;

		return true;
	}

	return false;
}

//////////////////////////////////////////////////////////////////////////

bool FileSelectorLoadWAV(char *pFileName)
{
	static OPENFILENAME ofn;
	wchar_t szFileName[MAX_PATH] = L"";

	memset(&ofn,0, sizeof(ofn));

	ofn.lStructSize = sizeof(ofn); // SEE NOTE BELOW
	ofn.hwndOwner = NULL;//hWnd;
	ofn.lpstrFilter = L"WAV Files (*.wav)\0*.wav\0All Files (*.*)\0*.*\0";
	ofn.lpstrFile = szFileName;
	ofn.nMaxFile = MAX_PATH;
	ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
	ofn.lpstrDefExt = L"wav";

	if(GetOpenFileName(&ofn))
	{
		wchar_t *pSrc = &szFileName[0];
		char *pDest = pFileName;

		while (*pSrc)
		{
			*pDest++ = *pSrc++;
		}
		*pDest = 0;

		return true;
	}

	return false;
}

//////////////////////////////////////////////////////////////////////////

bool FileSelectorSaveWAV(char *pFileName)
{
	static OPENFILENAME ofn;
	wchar_t szFileName[MAX_PATH] = L"";

	memset(&ofn,0, sizeof(ofn));

	ofn.lStructSize = sizeof(ofn); // SEE NOTE BELOW
	ofn.hwndOwner = NULL;//hWnd;
	ofn.lpstrFilter = L"Wav Files (*.wav)\0*.wav\0All Files (*.*)\0*.*\0";
	ofn.lpstrFile = szFileName;
	ofn.nMaxFile = MAX_PATH;
	ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
	ofn.lpstrDefExt = L"wav";

	if(GetSaveFileName(&ofn))
	{
		wchar_t *pSrc = &szFileName[0];
		char *pDest = pFileName;

		while (*pSrc)
		{
			*pDest++ = *pSrc++;
		}
		*pDest = 0;

		return true;
	}

	return false;
}

//////////////////////////////////////////////////////////////////////////
