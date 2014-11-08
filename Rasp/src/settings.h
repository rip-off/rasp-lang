#ifndef SETTINGS_H
#define SETTINGS_H

class Settings
{
public:
	bool trace;
	bool verbose;

	Settings() 
	:
		trace(false),
		verbose(false)
	{
	}
};

#endif

