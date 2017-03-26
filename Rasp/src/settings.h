#ifndef SETTINGS_H
#define SETTINGS_H

class Settings
{
public:
	bool trace;
	bool printSyntaxTree;
	bool printInstructions;

	Settings() 
	:
		trace(false),
		printSyntaxTree(false),
		printInstructions(false)
	{
	}
};

#endif

