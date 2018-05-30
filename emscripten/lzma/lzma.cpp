
#include <iostream>
#include <fstream>
#include <ostream>
#include <istream>
#include <string>
#include <stdlib.h>
#include "Lzma2Enc.h"
using namespace std;

static void *SzAlloc(ISzAllocPtr, size_t size) { return malloc(size); }
static void SzFree(ISzAllocPtr, void *address) { free(address); }
static ISzAlloc g_Alloc = { SzAlloc, SzFree };
static ifstream *inStream=NULL;
static ofstream *outStream=NULL;
static size_t OutWrite(const ISeqOutStream *p, const void *buf, size_t size)
{
	outStream->write((char *) buf, size);
	return size;
}
static SRes InRead(const ISeqInStream *p, void *buf, size_t *size)
{
	size_t reqs=*size;
	inStream->read((char *)buf,*size);
	*size=inStream->gcount();
	cout << "InRead("<<reqs<<") "<<(*size) << " EOF:" << inStream->eof();
	return SZ_OK;
}

void ShowUsage(char *prgName)
{
    cerr << "Usage: " << prgName << " infile outfile" << endl;
    cerr << endl << "Options:" << endl;
}

int main(int argc, char **argv)
{
    // Default arguments
    char *inName = NULL, *outName = NULL;
    bool wrapper=false;

    // Get arguments
    for (int i = 1; i < argc; ++i)
    {
        string arg(argv[i]);
        if (arg == "-wrapper")
            wrapper=true;
        else if (arg == "-nostrip") {}
        else if (arg == "-i") {}
        else if (!inName)
            inName = argv[i];
        else if (!outName)
            outName = argv[i];
        else
        {
            ShowUsage(argv[0]);
            return 0;
        }
    }
    if (!inName||!outName)
    {
        ShowUsage(argv[0]);
        return 0;
    }

    // Read input file
    size_t fileSize = 0;
    ifstream inFile(inName, ios_base::in | ios_base::binary);
    inStream=&inFile;
    if (!inFile.fail())
    {
    		ofstream outFile(outName, ios_base::out | ios_base::binary);
        	outStream=&outFile;
    	    CLzmaEncHandle enc;
    	    SRes res;
    	    CLzmaEncProps props;

    	    enc = LzmaEnc_Create(&g_Alloc);
    	    if (enc == 0)
    	    {
    	    	cerr << "Couldn't create LZMA encoder" << endl;
    	    	return 0;
    	    }

    	    LzmaEncProps_Init(&props);

    	    props.level=2;

    	    res = LzmaEnc_SetProps(enc, &props);

    	    if (res == SZ_OK)
    	    {
    	        //Byte header;
    	    	SizeT propsSize = LZMA_PROPS_SIZE;
    	    	Byte encProps[propsSize+8];
    	    	for (int k=0;k<8;k++)
    	    		encProps[propsSize+k]=0xFF;

    	        LzmaEnc_WriteProperties(enc,encProps,&propsSize);
    	        outStream->write((char *) encProps, propsSize+8);

    	        //outstream << header;
    	        ISeqOutStream os={ OutWrite };
    	        ISeqInStream is={ InRead };

    	        res = LzmaEnc_Encode(enc, &os,&is,NULL,&g_Alloc,&g_Alloc);
    	        if (res!=SZ_OK)
    	        	cerr << "Failed to compress: "<< res << endl;
    	    }
    	    else
    	    	cerr << "Failed to set props" << endl;
    	    LzmaEnc_Destroy(enc,&g_Alloc,&g_Alloc);
    	inFile.close();
    	outFile.close();
        if (wrapper) {
        	cout << "Writing wrapper" << endl;
            ofstream outFile(inName, ios_base::out );
            if (outFile.fail())
                cerr << "Unable to open file \"" << inName << "\"." << endl;
            outFile << "JPZMALoad(\"" << outName << "\",eval);" << endl;
            outFile.close();
        }

    }
    else
        cerr << "Unable to open file \"" << inName << "\"." << endl;

    return 0;
}

