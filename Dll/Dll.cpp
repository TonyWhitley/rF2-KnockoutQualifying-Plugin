// Test harness to run the DLL which is built as a lib for debugging rather as a DLL
//

#include <iostream>
#include "KnockoutQualifying.hpp"

#define NUM_PARTICIPANTS 2

int main()
{
    std::cout << "Hello World!\n";

	MultiSessionRulesV01 info;
	info.mNumParticipants = NUM_PARTICIPANTS;
	info.mSession = 5;

	MultiSessionParticipantV01 participants[NUM_PARTICIPANTS] ;
	MultiSessionParticipantV01 *participants_p = participants;
	info.mParticipant = participants_p;

	for (int i = 0; i < info.mNumParticipants; i++)
	{
		char driverName[20];
		sprintf_s(driverName, sizeof(driverName), "fred%d", i);
		strcpy_s(participants[i].mDriverName,
			sizeof(participants[i].mDriverName),
			driverName);
	}

	KnockoutQualifyingPlugin fred;
	fred = KnockoutQualifyingPlugin();
	//fred.GetPluginName();
	fred.Load();
	fred.AccessMultiSessionRules(info);
}

