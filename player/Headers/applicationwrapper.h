#ifndef APPLICATIONWRAPPER_H
#define APPLICATIONWRAPPER_H

class Server;

class ApplicationWrapper
{
public:
	ApplicationWrapper(void);
	~ApplicationWrapper(void);

	void tick();

public:
	Server* server_;
};


#endif