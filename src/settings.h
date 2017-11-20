#ifndef SETTINGS_H
#define SETTINGS_H

class Settings
{
public:
	bool repl;
	bool trace;
	bool unitTests;
	bool printSyntaxTree;
	bool printInstructions;

	Settings() 
	:
		repl(false),
		trace(false),
		unitTests(false),
		printSyntaxTree(false),
		printInstructions(false)
	{
	}
};

#endif

