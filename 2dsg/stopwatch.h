#ifndef STOPWATCH_H
#define STOPWATCH_H

class StopWatch
{
public:
	StopWatch();

	double clock();
	void pause();
	void resume();
	bool isPaused() const;

private:
	bool isRunning_;
	double last_;
	double total_;
};


#endif