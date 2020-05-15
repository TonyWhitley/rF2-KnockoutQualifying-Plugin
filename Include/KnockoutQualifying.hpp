//ÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜÜ
//Ý                                                                         Þ
//Ý Module: Header file for knockout qualifying                             Þ
//Ý                                                                         Þ
//Ý Description: Implements knockout qualifying for vehicle races           Þ
//Ý                                                                         Þ
//Ý This source code module, and all information, data, and algorithms      Þ
//Ý associated with it, are part of isiMotor Technology (tm).               Þ
//Ý                 PROPRIETARY AND CONFIDENTIAL                            Þ
//Ý Copyright (c) 1996-2015 Image Space Incorporated.  All rights reserved. Þ
//Ý                                                                         Þ
//Ý Change history:                                                         Þ
//Ý   tag.2015.09.30: created                                               Þ
//Ý                                                                         Þ
//ßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßß

#ifndef _KNOCKOUT_QUALIFYING_HPP_
#define _KNOCKOUT_QUALIFYING_HPP_

#include "InternalsPlugin.hpp"     // base class for plugin objects to derive from


class KnockoutQualifyingPlugin : public InternalsPluginV07 // REMINDER: exported function GetPluginVersion() should return 1 if you are deriving from this InternalsPluginV01, 2 for InternalsPluginV02, etc.
{
 protected:

  struct SessionData
  {
    long mMinutes;                      // minutes (0 means to use default rules)
    long mScored;                       // number of scored participants (actual number will be adjusted based on actual total and change from previous session)

    SessionData()                       { mMinutes = 0; mScored = 0; }
  };

  // Custom variables
  long mNumQualSessionsOverride;        // 0 = use default rules, 1-4 = number of qual sessions
  bool mFixedNumberScored;              // use fixed number of vehicles scored for Q2->Q4, rather than doing it proportionally compared to the previous session(s)
  bool mTimedLapRequired;               // timed lap required in previous session in order to compete in Q2->Q4
  SessionData m1SessionData[ 1 ];       // data for each session if there is 1 session (any value of 0 means to use default rules)
  SessionData m2SessionData[ 2 ];       // data for each session if there are 2 sessions (any value of 0 means to use default rules)
  SessionData m3SessionData[ 3 ];       // data for each session if there are 3 sessions (any value of 0 means to use default rules)
  SessionData m4SessionData[ 4 ];       // data for each session if there are 4 sessions (any value of 0 means to use default rules)

  // Store some scoring info which might help use decide how many qualifying sessions there should be
  char mTrackName[ 64 ];                // current track name
  double mLapDist;                      // distance around track

  //
  long mQualifying1Seconds;             // We base the duration of subsequent qualifying sessions on the first one, so remember what it was.
  long mPreviousSession;                // Session from the last time AccessMultiSessionRules() was called, to see if it's new or not
  long mPreviousNumScored;              // Previous number scored (don't change if session hasn't changed)

  //
  struct DriverProgress : public MultiSessionParticipantV01
  {
    long mQualSessionReached;           // Last qual session that the participant was scored in
    DriverProgress *mNext;              // Next in list
  };
  DriverProgress *mDriverProgressList;  // List of drivers that we are aware of, and their progress through qualifying

  // protected method(s)
  long ConvertNonDefaultSettingToMinutes( long settingIndex );
  void DeleteAllDriverProgressData();
  DriverProgress *UpdateDriverProgress( const MultiSessionParticipantV01 *participant, bool beyondPractice ); // Updates and returns driver progress for given participant (even if it has to create a new one)

 public:

  //
  KnockoutQualifyingPlugin();
  ~KnockoutQualifyingPlugin();

  //
  void Load();                                                                  // scene/track load

  //
  bool WantsScoringUpdates()                                { return( true ); } // whether we want scoring updates
  void UpdateScoring( const ScoringInfoV01 &info );                             // update plugin with scoring info (approximately five times per second)

  //
  bool GetCustomVariable( long i, CustomVariableV01 &var );
  void AccessCustomVariable( CustomVariableV01 &var );
  void GetCustomVariableSetting( CustomVariableV01 &var, long i, CustomSettingV01 &setting );

  //
  bool WantsMultiSessionRulesAccess()                       { return( true ); } // change to true in order to read or write multi-session rules
  bool AccessMultiSessionRules( MultiSessionRulesV01 &info );                   // current internal rules passed in; return true if you want to change them
};


#endif // _KNOCKOUT_QUALIFYING_HPP_


