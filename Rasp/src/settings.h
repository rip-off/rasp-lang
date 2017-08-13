#ifndef SETTINGS_H
#define SETTINGS_H

class Settings
{
public:
	bool trace;
	bool debugUnitTests;
	bool printSyntaxTree;
	bool printInstructions;

	Settings() 
	:
		trace(false),
		debugUnitTests(false),
		printSyntaxTree(false),
		printInstructions(false)
	{
	}
};

#endif

