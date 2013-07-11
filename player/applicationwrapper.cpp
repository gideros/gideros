#include "applicationwrapper.h"
#include "libnetwork.h"
#include <direct.h>

ApplicationWrapper::ApplicationWrapper(void)
{
	server_ = new Server(15000);
}

ApplicationWrapper::~ApplicationWrapper(void)
{
	delete server_;
}

void ApplicationWrapper::tick()
{
	Event event;
	server_->tick(&event);

	if (event.eventCode == eDataReceived)
	{
		const std::vector<char>& data = event.data;

		switch (data[0])
		{
			case 0:			// create folder
			{
				std::string folderName = &data[1];
				_mkdir(folderName.c_str());
				break;
			}
			
			case 1:			// create file
			{
				std::string fileName = &data[1];
				FILE* fos = fopen(fileName.c_str(), "wb");
				int pos = 1 + fileName.size() + 1;
				fwrite(&data[pos], data.size() - pos, 1, fos);
				fclose(fos);
				break;
			}
			case 2:
			{
				//play();
				break;
			}
			case 3:
			{
				//stop();
				break;
			}
		}
	}
}
