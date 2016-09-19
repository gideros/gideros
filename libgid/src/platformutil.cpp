#include "platformutil.h"
#include <string>
#include <string.h>
#include <stdio.h>

#include <sys/stat.h>
#include <time.h>
time_t fileLastModifiedTime(const char* file)
{
	struct stat s;
	stat(file, &s);

	return s.st_mtime;
}

time_t fileAge(const char* file)
{
	return time(NULL) - fileLastModifiedTime(file);
}

off_t fileSize(const char* file)
{
	struct stat s;
	stat(file, &s);

	return s.st_size;
}

#if defined(WINSTORE)
#elif defined(_WIN32)
#include <Windows.h>
void getDirectoryListing(const char* dir, std::vector<std::string>* files, std::vector<std::string>* directories)
{
	files->clear();
	directories->clear();

	WIN32_FIND_DATAA ffd;
	HANDLE hFind;

	std::string dirstar;

	int dirlen = strlen(dir);
	if (dirlen > 0 && (dir[dirlen - 1] == '/' || dir[dirlen - 1] == '\\'))
		dirstar = std::string(dir) + "*";
	else
		dirstar = std::string(dir) + "/*";

	hFind = FindFirstFileA(dirstar.c_str(), &ffd);

	do
	{
		if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			if (strcmp(ffd.cFileName, ".") == 0 || strcmp(ffd.cFileName, "..") == 0)
				continue;

		if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			directories->push_back(ffd.cFileName);
		else
			files->push_back(ffd.cFileName);
	} while (FindNextFileA(hFind, &ffd) != 0);

	FindClose(hFind);
}
#else
#include <dirent.h>
void getDirectoryListing(const char* dir, std::vector<std::string>* files, std::vector<std::string>* directories)
{
	struct dirent *entry;
	DIR *dp;

	dp = opendir(dir);
	if (dp == NULL)
		return;

	while((entry = readdir(dp)))
	{
		if (entry->d_type == DT_DIR)
			if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
				continue;

		if (entry->d_type == DT_DIR)
			directories->push_back(entry->d_name);
		else if (entry->d_type == DT_REG)
			files->push_back(entry->d_name);
	}

	closedir(dp);
}
#endif

#include <stack>

void getDirectoryListingR(const char* dir, std::vector<std::string>* filesout, std::vector<std::string>* directoriesout)
{
	filesout->clear();
	directoriesout->clear();

	std::stack<std::string> stack;

	std::string directory = dir;

	int dirlen = strlen(dir);
	if (dirlen > 0 && (dir[dirlen - 1] == '/' || dir[dirlen - 1] == '\\'))
		directory.resize(directory.size() - 1);

	stack.push("");

	while (!stack.empty())
	{
		std::string dir = stack.top();
		stack.pop();

		std::vector<std::string> files, directories;
		getDirectoryListing((directory + dir).c_str(), &files, &directories);

		for (std::size_t i = 0; i < files.size(); ++i)
		{
			std::string str = dir + "/" + files[i];	
			filesout->push_back(str.c_str() + 1);
		}

		for (std::size_t i = 0; i < directories.size(); ++i)
		{
			std::string str = dir + "/" + directories[i];	
			directoriesout->push_back(str.c_str() + 1);
			stack.push(dir + "/" + directories[i]);
		}
	}
}


#if defined(WINSTORE)
#include <algorithm>
#include <collection.h>
std::vector<std::string> getLocalIPs()
{
	struct IPs {
		IPs() { result.clear(); }
		void operator()(Windows::Networking::HostName^ hostname) { 
			if (hostname->IPInformation != nullptr)
			{
				std::wstring ws(hostname->CanonicalName->Data());
				result.push_back(std::string(ws.begin(), ws.end()));
			}
		}

		std::vector<std::string> result;
	};

	auto hostNames = Windows::Networking::Connectivity::NetworkInformation::GetHostNames();

	IPs ip = std::for_each(begin(hostNames), end(hostNames), IPs());
	return ip.result;
}
#elif defined(_WIN32)
#include <winsock2.h>
#include <iphlpapi.h>
#pragma comment(lib, "IPHLPAPI.lib")

//#define MALLOC(x) HeapAlloc(GetProcessHeap(), 0, (x))
//#define FREE(x) HeapFree(GetProcessHeap(), 0, (x))
#define MALLOC(x) malloc(x)
#define FREE(x) free(x)

/* Note: could also use malloc() and free() */

#define printf
std::vector<std::string> getLocalIPs()
{
	std::vector<std::string> result;


    /* Declare and initialize variables */

// It is possible for an adapter to have multiple
// IPv4 addresses, gateways, and secondary WINS servers
// assigned to the adapter. 
//
// Note that this sample code only prints out the 
// first entry for the IP address/mask, and gateway, and
// the primary and secondary WINS server for each adapter. 

    PIP_ADAPTER_INFO pAdapterInfo;
    PIP_ADAPTER_INFO pAdapter = NULL;
    DWORD dwRetVal = 0;
    UINT i;

/* variables used to print DHCP time info */
    struct tm newtime;
    char buffer[32];

    ULONG ulOutBufLen = sizeof (IP_ADAPTER_INFO);
    pAdapterInfo = (IP_ADAPTER_INFO *) MALLOC(sizeof (IP_ADAPTER_INFO));
    if (pAdapterInfo == NULL) {
        printf("Error allocating memory needed to call GetAdaptersinfo\n");
        return result;
    }
// Make an initial call to GetAdaptersInfo to get
// the necessary size into the ulOutBufLen variable
    if (GetAdaptersInfo(pAdapterInfo, &ulOutBufLen) == ERROR_BUFFER_OVERFLOW) {
        FREE(pAdapterInfo);
        pAdapterInfo = (IP_ADAPTER_INFO *) MALLOC(ulOutBufLen);
        if (pAdapterInfo == NULL) {
            printf("Error allocating memory needed to call GetAdaptersinfo\n");
            return result;
        }
    }

    if ((dwRetVal = GetAdaptersInfo(pAdapterInfo, &ulOutBufLen)) == NO_ERROR) {
        pAdapter = pAdapterInfo;
        while (pAdapter) {
			result.push_back(pAdapter->IpAddressList.IpAddress.String);
#if 0
			errno_t error;
			printf("\tComboIndex: \t%d\n", pAdapter->ComboIndex);
            printf("\tAdapter Name: \t%s\n", pAdapter->AdapterName);
            printf("\tAdapter Desc: \t%s\n", pAdapter->Description);
            printf("\tAdapter Addr: \t");
            for (i = 0; i < pAdapter->AddressLength; i++) {
                if (i == (pAdapter->AddressLength - 1))
                    printf("%.2X\n", (int) pAdapter->Address[i]);
                else
                    printf("%.2X-", (int) pAdapter->Address[i]);
            }
            printf("\tIndex: \t%d\n", pAdapter->Index);
            printf("\tType: \t");
            switch (pAdapter->Type) {
            case MIB_IF_TYPE_OTHER:
                printf("Other\n");
                break;
            case MIB_IF_TYPE_ETHERNET:
                printf("Ethernet\n");
                break;
            case MIB_IF_TYPE_TOKENRING:
                printf("Token Ring\n");
                break;
            case MIB_IF_TYPE_FDDI:
                printf("FDDI\n");
                break;
            case MIB_IF_TYPE_PPP:
                printf("PPP\n");
                break;
            case MIB_IF_TYPE_LOOPBACK:
                printf("Lookback\n");
                break;
            case MIB_IF_TYPE_SLIP:
                printf("Slip\n");
                break;
            default:
                printf("Unknown type %ld\n", pAdapter->Type);
                break;
            }

            printf("\tIP Address: \t%s\n",
                   pAdapter->IpAddressList.IpAddress.String);
            printf("\tIP Mask: \t%s\n", pAdapter->IpAddressList.IpMask.String);

            printf("\tGateway: \t%s\n", pAdapter->GatewayList.IpAddress.String);
            printf("\t***\n");

            if (pAdapter->DhcpEnabled) {
                printf("\tDHCP Enabled: Yes\n");
                printf("\t  DHCP Server: \t%s\n",
                       pAdapter->DhcpServer.IpAddress.String);

                printf("\t  Lease Obtained: ");
                /* Display local time */
                error = _localtime32_s(&newtime, (__time32_t*) &pAdapter->LeaseObtained);
                if (error)
                    printf("Invalid Argument to _localtime32_s\n");
                else {
                    // Convert to an ASCII representation 
                    error = asctime_s(buffer, 32, &newtime);
                    if (error)
                        printf("Invalid Argument to asctime_s\n");
                    else
                        /* asctime_s returns the string terminated by \n\0 */
                        printf("%s", buffer);
                }

                printf("\t  Lease Expires:  ");
                error = _localtime32_s(&newtime, (__time32_t*) &pAdapter->LeaseExpires);
                if (error)
                    printf("Invalid Argument to _localtime32_s\n");
                else {
                    // Convert to an ASCII representation 
                    error = asctime_s(buffer, 32, &newtime);
                    if (error)
                        printf("Invalid Argument to asctime_s\n");
                    else
                        /* asctime_s returns the string terminated by \n\0 */
                        printf("%s", buffer);
                }
            } else
                printf("\tDHCP Enabled: No\n");

            if (pAdapter->HaveWins) {
                printf("\tHave Wins: Yes\n");
                printf("\t  Primary Wins Server:    %s\n",
                       pAdapter->PrimaryWinsServer.IpAddress.String);
                printf("\t  Secondary Wins Server:  %s\n",
                       pAdapter->SecondaryWinsServer.IpAddress.String);
            } else
                printf("\tHave Wins: No\n");
#endif
            pAdapter = pAdapter->Next;
            printf("\n");
        }
    } else {
        printf("GetAdaptersInfo failed with error: %d\n", dwRetVal);

    }
    if (pAdapterInfo)
        FREE(pAdapterInfo);

    return result;
}
#undef printf

#undef FREE
#undef MALLOC

#elif defined(__ANDROID__)

std::vector<std::string> jnb_getLocalIPs();

std::vector<std::string> getLocalIPs()
{
	return jnb_getLocalIPs();
}

#elif EMSCRIPTEN

std::vector<std::string> getLocalIPs()
{
 std::vector<std::string> none;
 return none;
}


#else

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <ifaddrs.h>
#include <errno.h>

#define printf
std::vector<std::string> getLocalIPs()
{
	std::vector<std::string> result;

	struct ifaddrs *myaddrs, *ifa;
    void *in_addr;
    char buf[64];

    if(getifaddrs(&myaddrs) != 0)
    {
		return result;
        //perror("getifaddrs");
        //exit(1);
    }

    for (ifa = myaddrs; ifa != NULL; ifa = ifa->ifa_next)
    {
        if (ifa->ifa_addr == NULL)
            continue;
        if (!(ifa->ifa_flags & IFF_UP))
            continue;

        switch (ifa->ifa_addr->sa_family)
        {
            case AF_INET:
            {
                struct sockaddr_in *s4 = (struct sockaddr_in *)ifa->ifa_addr;
                in_addr = &s4->sin_addr;
                break;
            }

			case AF_INET6:
            {
                struct sockaddr_in6 *s6 = (struct sockaddr_in6 *)ifa->ifa_addr;
                in_addr = &s6->sin6_addr;
                break;
            }

            default:
                continue;
        }

        if (!inet_ntop(ifa->ifa_addr->sa_family, in_addr, buf, sizeof(buf)))
        {
            printf("%s: inet_ntop failed!\n", ifa->ifa_name);
        }
        else
        {
            printf("%s: %s\n", ifa->ifa_name, buf);
			if (strcmp(buf, "127.0.0.1") != 0)
				result.push_back(buf);
        }
    }

    freeifaddrs(myaddrs);
    return result;
}
#undef printf

#endif


#if defined(_WIN32) && !defined(WINSTORE)
double iclock()
{
	static LARGE_INTEGER freq;
	static LARGE_INTEGER start;
	static bool init = false;

	if (init == false)
	{
		QueryPerformanceFrequency(&freq);
		QueryPerformanceCounter(&start);
		init = true;
	}

	LARGE_INTEGER li;
	QueryPerformanceCounter(&li);

	LONGLONG delta = li.QuadPart - start.QuadPart;

	long double result = (long double)delta / (long double)freq.QuadPart;

	return (double)result;
}
#elif defined(__APPLE__)
#import <CoreFoundation/CoreFoundation.h>
double iclock()
{
	static double begin = CFAbsoluteTimeGetCurrent();
	return CFAbsoluteTimeGetCurrent() - begin;
}
#elif defined(__ANDROID__) || defined(STRICT_LINUX)
static double nanoTime()
{
	struct timespec t;
	if(clock_gettime(CLOCK_MONOTONIC, &t))
		return 0;
	return t.tv_sec + t.tv_nsec * 1e-9;
}
double iclock()
{
	static double begin = nanoTime();
	return nanoTime() - begin;
}
#else
double iclock()
{
	double CPS = CLOCKS_PER_SEC;

	clock_t clock = ::clock();

	static clock_t lastClock;
	static bool firstCall = true;
	static double result = 0;

	if (firstCall)
	{
		lastClock = clock;
		firstCall = false;
	}

	clock_t delta = clock - lastClock;
	result += delta / CPS;
	lastClock = clock;

	return result;
}
#endif

extern "C"
{
#include "md5.h"
}

void md5_frombuffer(const unsigned char *buffer,unsigned int size, unsigned char md5sum[16])
{
	md5_context ctx;

	md5_starts( &ctx );
	md5_update( &ctx, buffer, size );
	md5_finish( &ctx, md5sum );
}


bool md5_fromfile(const char* filename, unsigned char md5sum[16])
{
	FILE* f = fopen(filename, "rb");

	if (f == NULL)
		return false;

	md5_context ctx;
	unsigned char buf[1000];

	md5_starts( &ctx );

	int i;
	while( ( i = fread( buf, 1, sizeof( buf ), f ) ) > 0 )
	{
		md5_update( &ctx, buf, i );
	}

	md5_finish( &ctx, md5sum );
	
	fclose(f);
	
	return true;
}

extern "C"
{
#include "aes.h"
}

void aes_encrypt(const unsigned char *input,unsigned char *output,int size,const unsigned char *key,const unsigned char *iv,int padtype)
{
	if (iv)
		AES128_CBC_encrypt_buffer(output,input,size,key,iv,padtype);
	else
	{
		while (size>=16)
		{
			AES128_ECB_encrypt(input,key,output);
			input+=16;
			output+=16;
			size-=16;
		}
	}
}

void aes_decrypt(const unsigned char *input,unsigned char *output,int size,const unsigned char *key,const unsigned char *iv,int padtype)
{
	if (iv)
		AES128_CBC_decrypt_buffer(output,input,size,key,iv,padtype);
	else
	{
		while (size>=16)
		{
			AES128_ECB_decrypt(input,key,output);
			input+=16;
			output+=16;
			size-=16;
		}
	}
}

