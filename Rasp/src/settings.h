#ifndef SETTINGS_H
#define SETTINGS_H

class Settings
{
public:
	bool repl;
	bool trace;
	bool debugUnitTests;
	bool printSyntaxTree;
	bool printInstructions;

	Settings() 
	:
		repl(false),
		trace(false),
		debugUnitTests(false),
		printSyntaxTree(false),
		printInstructions(false)
	{
	}
};

#endif

