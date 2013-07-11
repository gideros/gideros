#ifndef TICKER_H
#define TICKER_H

class Ticker
{
public:
	virtual ~Ticker() {}
	virtual void tick() = 0;
};

#endif
