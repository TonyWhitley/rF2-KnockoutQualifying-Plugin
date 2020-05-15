//‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹‹
//›                                                                         ﬁ
//› Module: Header file for knockout qualifying                             ﬁ
//›                                                                         ﬁ
//› Description: Implements knockout qualifying for vehicle races           ﬁ
//›                                                                         ﬁ
//› This source code module, and all information, data, and algorithms      ﬁ
//› associated with it, are part of isiMotor Technology (tm).               ﬁ
//›                 PROPRIETARY AND CONFIDENTIAL                            ﬁ
//› Copyright (c) 1996-2015 Image Space Incorporated.  All rights reserved. ﬁ
//›                                                                         ﬁ
//› Change history:                                                         ﬁ
//›   tag.2015.09.30: created                                               ﬁ
//›                                                                         ﬁ
//ﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂ

#include "KnockoutQualifying.hpp"
#include <cstring>
#include <cstdio>


// plugin information

extern "C" __declspec( dllexport )
const char * __cdecl GetPluginName()                   { return( "Knockout Qualifying Plugin - 2015.09.30" ); }

extern "C" __declspec( dllexport )
PluginObjectType __cdecl GetPluginType()               { return( PO_INTERNALS ); }

extern "C" __declspec( dllexport )
int __cdecl GetPluginVersion()                         { return( 7 ); } // InternalsPluginV07 functionality (if you change this return value, you must derive from the appropriate class!)

extern "C" __declspec( dllexport )
PluginObject * __cdecl CreatePluginObject()            { return( static_cast< PluginObject * >( new KnockoutQualifyingPlugin ) ); }

extern "C" __declspec( dllexport )
void __cdecl DestroyPluginObject( PluginObject *obj )  { delete( static_cast< KnockoutQualifyingPlugin * >( obj ) ); }


// KnockoutQualifying class

KnockoutQualifyingPlugin::KnockoutQualifyingPlugin()
{
  mNumQualSessionsOverride = 0;
  mFixedNumberScored = false;
  mTimedLapRequired = true;
  // m1SessionData[] has its own constructor
  // m2SessionData[] has its own constructor
  // m3SessionData[] has its own constructor
  // m4SessionData[] has its own constructor

  for( long i = 0; i < sizeof( mTrackName ); ++i )
    mTrackName[ i ] = '\0';
  mLapDist = 0.0;

  mQualifying1Seconds = 1800;
  mPreviousSession = -1;
  mPreviousNumScored = 2;

  mDriverProgressList = NULL;
}


KnockoutQualifyingPlugin::~KnockoutQualifyingPlugin()
{
  DeleteAllDriverProgressData();
}


void KnockoutQualifyingPlugin::Load()
{
  DeleteAllDriverProgressData();
  mLapDist = 0.0;
}


void KnockoutQualifyingPlugin::UpdateScoring( const ScoringInfoV01 &info )
{
  // copy track name, somewhat safely
  const long size = min( sizeof( mTrackName ), sizeof( info.mTrackName ) );
  for( long i = 0; i < size; ++i )
    mTrackName[ i ] = info.mTrackName[ i ];
  mTrackName[ sizeof( mTrackName ) - 1 ] = '\0';

  // copy lap distance
  mLapDist = info.mLapDist;
}


long KnockoutQualifyingPlugin::ConvertNonDefaultSettingToMinutes( long settingIndex )
{
  if( settingIndex == 0 )
  {
    // error - shouldn't be calling this ... perhaps this strange result will alert somehow, sans a proper error-reporting mechanism
    return( 97 );
  }
#if 1 // Just do straight-up minutes for now, but it would be cleaner in the UI if we did 5-minute increments after 15.
  else
  {
    return( settingIndex );
  }
#else // 1-minute increments up to 15, then 5 minute increments up to 3 hours
  else if( settingIndex < 16 )
  {
    // settings 1-15 matches up with 1-15 minutes
    return( settingIndex );
  }
  else
  {
    // setting 16 is 20 minutes, setting 17 is 25 minutes, setting 18 is 30 minutes, etc.
    return( ( ( settingIndex - 15 ) * 5 ) + 15 );
  }
#endif
}


bool KnockoutQualifyingPlugin::GetCustomVariable( long i, CustomVariableV01 &var )
{
  if( i == 0 )
  {
    // rF2 will automatically create this variable and default it to 1 (true) unless we create it first, in which case we can choose the default.
    strcpy_s( var.mCaption, " Enabled" );
    var.mNumSettings = 2;
    var.mCurrentSetting = 0;
    return( true );
  }
  else if( i == 1 )
  {
    strcpy_s( var.mCaption, "NumQualSessions" );
    var.mNumSettings = 5;
    var.mCurrentSetting = 0;
    return( true );
  }
  else if( i == 2 )
  {
    strcpy_s( var.mCaption, "FixedNumberScored" );
    var.mNumSettings = 2;
    var.mCurrentSetting = 0;
    return( true );
  }
  else if( i == 3 )
  {
    strcpy_s( var.mCaption, "TimedLapRequired" );
    var.mNumSettings = 2;
    var.mCurrentSetting = 1;
    return( true );
  }
  else if( i < 24 )
  {
    long count = i - 3;
    long total, session, data;
    for( total = 1; total <= 4; ++total )
    {
      for( session = 1; session <= total; ++session )
      {
        for( data = 0; data < 2; ++data )
        {
          --count;
          if( 0 == count )
            break;
        }
        if( 0 == count )
          break;
      }
      if( 0 == count )
        break;
    }

    if(      0 == data )
    {
      sprintf_s( var.mCaption, "Total%dSession%dMinutes", total, session );
      var.mNumSettings = 181;
      var.mCurrentSetting = 0;
    }
    else
    {
      sprintf_s( var.mCaption, "Total%dSession%dScored", total, session );
      var.mNumSettings = 105;
      if( 1 == session )
        var.mCurrentSetting = 43;
      else if( 2 == session )
        var.mCurrentSetting = ( total - 1 ) * 12;
      else if( 3 == session )
        var.mCurrentSetting = ( total - 2 ) * 12;
      else
        var.mCurrentSetting = 12;
    }

    return( true );
  }
  else
  {
    return( false );
  }
}


void KnockoutQualifyingPlugin::AccessCustomVariable( CustomVariableV01 &var )
{
  if( 0 == strcmp( var.mCaption, "NumQualSessions" ) )
  {
    mNumQualSessionsOverride = var.mCurrentSetting;
  }
  else if( 0 == strcmp( var.mCaption, "FixedNumberScored" ) )
  {
    mFixedNumberScored = ( var.mCurrentSetting != 0 );
  }
  else if( 0 == strcmp( var.mCaption, "TimedLapRequired" ) )
  {
    mTimedLapRequired = ( var.mCurrentSetting != 0 );
  }
  else
  {
    long total, session;
    if( ( sscanf_s( var.mCaption, "Total%dSession%d", &total, &session ) == 2 ) && ( session <= total ) )
    {
      SessionData *sessionData = NULL;
      switch( total )
      {
        case 1: sessionData = m1SessionData; break;
        case 2: sessionData = m2SessionData; break;
        case 3: sessionData = m3SessionData; break;
        case 4: sessionData = m4SessionData; break;
      }

      if( sessionData != NULL )
      {
        if( NULL != strstr( var.mCaption, "Minutes" ) )
          sessionData[ session - 1 ].mMinutes = var.mCurrentSetting;
        else if( NULL != strstr( var.mCaption, "Scored" ) )
          sessionData[ session - 1 ].mScored = var.mCurrentSetting;
      }
    }
  }
}


void KnockoutQualifyingPlugin::GetCustomVariableSetting( CustomVariableV01 &var, long i, CustomSettingV01 &setting )
{
  // Default in case we forget something
  strcpy_s( setting.mName, "Error" );

  if( 0 == strcmp( var.mCaption, "NumQualSessions" ) )
  {
    switch( i )
    {
      case 0: strcpy_s( setting.mName, "Default" ); break;
      case 1: strcpy_s( setting.mName, "1" ); break;
      case 2: strcpy_s( setting.mName, "2" ); break;
      case 3: strcpy_s( setting.mName, "3" ); break;
      case 4: strcpy_s( setting.mName, "4" ); break;
    }
  }
  else if( 0 == strcmp( var.mCaption, "FixedNumberScored" ) )
  {
    if( 0 == i ) strcpy_s( setting.mName, "Variable" );
    else         strcpy_s( setting.mName, "Fixed" );
  }
  else if( 0 == strcmp( var.mCaption, "TimedLapRequired" ) )
  {
    if( 0 == i ) strcpy_s( setting.mName, "No" );
    else         strcpy_s( setting.mName, "Yes" );
  }
  else if( 0 == strncmp( var.mCaption, "Total", 5 ) ) // half-hearted attempt to make sure we are using one of the "Total<x>Session<y><something>" variables ...
  {
    if( 0 == i )
    {
      strcpy_s( setting.mName, "Default" );
    }
    else if( NULL != strstr( var.mCaption, "Minutes" ) )
    {
      sprintf_s( setting.mName, "%d", ConvertNonDefaultSettingToMinutes( i ) );
    }
    else if( NULL != strstr( var.mCaption, "Scored" ) )
    {
      sprintf_s( setting.mName, "%d", i );
    }
  }
}


void KnockoutQualifyingPlugin::DeleteAllDriverProgressData()
{
  while( mDriverProgressList != NULL )
  {
    DriverProgress *next = mDriverProgressList->mNext;
    delete mDriverProgressList;
    mDriverProgressList = next;
  }
}


KnockoutQualifyingPlugin::DriverProgress *KnockoutQualifyingPlugin::UpdateDriverProgress( const MultiSessionParticipantV01 *participant, bool beyondPractice )
{
  // Look through list for a match
  DriverProgress **dp = &mDriverProgressList;
  while( *dp != NULL )
  {
    if( ( 0 == strcmp( (*dp)->mDriverName, participant->mDriverName ) ) &&
        ( 0 == strcmp( (*dp)->mVehicleName, participant->mVehicleName ) ) &&
        ( 0 == memcmp( (*dp)->mUpgradePack, participant->mUpgradePack, sizeof( participant->mUpgradePack ) ) ) &&
        ( !beyondPractice || ( (*dp)->mQualParticipantIndex == participant->mQualParticipantIndex ) ) )
    {
      // Update existing one and return it
      *static_cast< MultiSessionParticipantV01 * >( *dp ) = *participant;
      return( *dp );
    }

    dp = &( (*dp)->mNext );
  }

  // Create new one and return it
  *dp = new DriverProgress;
  *static_cast< MultiSessionParticipantV01 * >( *dp ) = *participant;
  (*dp)->mQualSessionReached = 0;
  (*dp)->mNext = NULL;
  return( *dp );
}


bool KnockoutQualifyingPlugin::AccessMultiSessionRules( MultiSessionRulesV01 &info )
{
  // We don't do anything or even remember anything before qualifying starts
  if( info.mSession < 5 )
  {
    DeleteAllDriverProgressData();
    return( false );
  }

  // To identify drivers, we can use MultiSessionParticipantV01::mQualParticipantIndex which should be valid and unique once qualifying begins.
  const bool beyondPractice = ( info.mSession >= 5 );

  // Decide if there are 2 or 3 sessions, depending on current stock car rules ... there's a little bit of error in the lap distances, so 1950m is
  // a low-side approximation of 1.25 miles.
  bool isRoadCourseOrShortOval = true;
  if( mLapDist > 1950.0 )
  {
    // copy and convert to lower-case for easier comparison
    char trackTypeUpperCase[ sizeof( info.mTrackType ) ];
    for( long i = 0; i < sizeof( info.mTrackType ); ++i )
    {
      if( ( info.mTrackType[ i ] >= 'A' ) && ( info.mTrackType[ i ] <= 'Z' ) )
        trackTypeUpperCase[ i ] = info.mTrackType[ i ] + ( 'a' - 'A' );
      else
        trackTypeUpperCase[ i ] = info.mTrackType[ i ];
    }

    // contains 'speedway' anywhere
    if( strstr( trackTypeUpperCase, "speedway" ) != NULL )
      isRoadCourseOrShortOval = false;

    // contains 'short track' anywhere
    // UNDONE - not sure if this is necessary ... I don't think any track over 1.25 miles would have 'short track' as the track type
    if( strstr( trackTypeUpperCase, "short track" ) != NULL )
      isRoadCourseOrShortOval = false;

    // contains 'oval' as an independent word (because maybe that could be part of a bigger word like cOVALent?)
    const char *oval = strstr( trackTypeUpperCase, "oval" );
    if( ( oval != NULL ) && ( ( oval[ 4 ] == '\0' ) || ( oval[ 4 ] == ' ' ) ) )
    {
      if( ( oval == trackTypeUpperCase ) || ( oval[ -1 ] == ' ' ) )
        isRoadCourseOrShortOval = false;
    }
  }

  // Check session and/or other variable to see what we need to do here
  long numScored = info.mNumParticipants;
  bool setGridPos = false;
  if( ( -2 == info.mSpecialSlotID ) || ( info.mSession >= 9 ) ) // 9 = warmup (where we set the grid), 10+ = race (looks like we need to set it in case warmup was skipped)
  {
    // tag.2016.04.19 - Try not to overwrite any grid-editing that was done. So only set the grid if we have not previously done so.
    if( ( -2 == info.mSpecialSlotID ) || ( mPreviousSession < 9 ) )
      setGridPos = true;
  }
  else if( 5 == info.mSession ) // 5 = qualifying session #1
  {
    // Different rules depending on track layout ...
    if( isRoadCourseOrShortOval )
    {
      info.mNumQualSessions = 2;
      // UNDONE - override time?
      info.mMaxSeconds = 1800;
    }
    else
    {
      info.mNumQualSessions = 3;
      // UNDONE - override time?
      info.mMaxSeconds = 1500;
    }

    // Overrides
    if( mNumQualSessionsOverride > 0 )
      info.mNumQualSessions = mNumQualSessionsOverride;

    const long sessionIndex = info.mSession - 5; // 5 = qualifying session #1
    const SessionData * const sessionData = ( 1 == info.mNumQualSessions ) ? m1SessionData : ( 2 == info.mNumQualSessions ) ? m2SessionData :
                                            ( 3 == info.mNumQualSessions ) ? m3SessionData :                                  m4SessionData;
    if( sessionData[ sessionIndex ].mMinutes != 0 )
      info.mMaxSeconds = 60 * ConvertNonDefaultSettingToMinutes( sessionData[ sessionIndex ].mMinutes );

    // override session name if appropriate
    if( info.mNumQualSessions > 1 )
      strcpy_s( info.mName, "Qualifying 1" );
    else
      strcpy_s( info.mName, "Qualifying" );

    // Store the duration of the first qualifying session
    mQualifying1Seconds = info.mMaxSeconds;
  }
  else if( ( info.mSession >= 6 ) && ( info.mSession <= 8 ) ) // 6-8 = qualifying sessions #2 - #4
  {
    // Only ca
    // Decide on:
    // 1) length of session
    // 2) number of participants who can continue participating
    long minutes = mQualifying1Seconds / 60;
    if( isRoadCourseOrShortOval )
    {
      minutes = max( 3, ( minutes + 2 ) / 3 );
      numScored = max( 2, ( ( mPreviousNumScored * 12 ) + 42 ) / 43 ); // 12/43 make it from qual 1 to qual 2
    }
    else
    {
      if( 6 == info.mSession )
      {
        minutes = max( 3, ( ( 2 * minutes ) + 4 ) / 5 );
        numScored = max( 3, ( ( mPreviousNumScored * 24 ) + 42 ) / 43 ); // 24/43 make it from qual 1 to qual 2
      }
      else
      {
        minutes = max( 3, ( minutes + 4 ) / 5 );
        numScored = max( 2, ( mPreviousNumScored + 1 ) / 2 ); // cut in half from qual 2 to qual 3
      }
    }

    info.mMaxSeconds = 60 * minutes;

    // Overrides
    // Note: we shouldn't actually be here unless ( 2 <= info.mNumQualSessions <=4 )
    const long sessionIndex = info.mSession - 5; // 5 = qualifying session #1
    const SessionData * const sessionData = ( 1 == info.mNumQualSessions ) ? m1SessionData : ( 2 == info.mNumQualSessions ) ? m2SessionData :
                                            ( 3 == info.mNumQualSessions ) ? m3SessionData :                                  m4SessionData;
    if( sessionData[ sessionIndex ].mMinutes != 0 )
      info.mMaxSeconds = 60 * ConvertNonDefaultSettingToMinutes( sessionData[ sessionIndex ].mMinutes );

    const long curBaseScored = sessionData[ sessionIndex ].mScored;
    const long prevBaseScored = sessionData[ sessionIndex - 1 ].mScored;
    if( ( curBaseScored != 0 ) && ( prevBaseScored >= curBaseScored ) )
    {
      if( mFixedNumberScored )
      {
        // User wants a fixed number of vehicles scored, rather than a variable number proportional to the custom variables.
        numScored = curBaseScored;
      }
      else
      {
        // By adding (prevBaseScored-1), this algorithm rounds the proportion *up*.
        const long minScored = info.mNumQualSessions + 1 - sessionIndex; // keep a minimum but dwindling number of vehicles in each session
        numScored = max( minScored, ( ( mPreviousNumScored * curBaseScored ) + prevBaseScored - 1 ) / prevBaseScored );
      }
    }

    // If session didn't change, don't change the number scored. This can happen because this function can get called in the middle of
    // a session when someone joins. And we don't want to suddenly change the number of vehicles scored in the middle of Q2 (or Q3 or Q4),
    // just because a new client joined.
    if( mPreviousSession == info.mSession )
      numScored = mPreviousNumScored;

    // override session name if appropriate
    sprintf_s( info.mName, "Qualifying %d", info.mSession - 4 );
  }

  // Store the previous number scored in case we need it next time this is called. Note that this may get updated during Q1, but it can't
  // actually change during Q2, Q3, or Q4 except at the beginning of the session due to the above logic.
  mPreviousNumScored = numScored;

  // Now sort everybody for deciding who is scored (in a qualifying session) or what their grid position is (for a race session)
  MultiSessionParticipantV01 **rankedDriver = new MultiSessionParticipantV01 *[ info.mNumParticipants ];
  if( rankedDriver != NULL )
  {
    // Collect pointers
    for( long i = 0; i < info.mNumParticipants; ++i )
      rankedDriver[ i ] = &info.mParticipant[ i ];

    // Sort them (simple but slow bubble sort ... performance is of little concern here)
    for( long i = 0; i < info.mNumParticipants; ++i )
    {
      for( long j = ( i + 1 ); j < info.mNumParticipants; ++j )
      {
        DriverProgress *iProgress = UpdateDriverProgress( rankedDriver[ i ], beyondPractice );
        DriverProgress *jProgress = UpdateDriverProgress( rankedDriver[ j ], beyondPractice );

        // Decide whether to swap
        bool swap = false;

        // If someone reached a later qualifying session
        if( iProgress->mQualSessionReached > jProgress->mQualSessionReached )
        {
          // keep order the same
        }
        else if( iProgress->mQualSessionReached < jProgress->mQualSessionReached )
        {
          swap = true;
        }
        else
        {
          // They reached the same qual session. If valid, compare the times in that session. If no times for either,
          // go to any earlier sessions, etc.
          long prevQualIndex;
          for( prevQualIndex = iProgress->mQualSessionReached - 5; prevQualIndex >= 0; --prevQualIndex )
          {
            if( rankedDriver[ i ]->mQualificationTime[ prevQualIndex ] > 0.0f ) // i has a laptime ...
            {
              if( ( rankedDriver[ j ]->mQualificationTime[ prevQualIndex ] > 0.0f ) &&
                  ( rankedDriver[ j ]->mQualificationTime[ prevQualIndex ] < rankedDriver[ i ]->mQualificationTime[ prevQualIndex ] ) )
              {
                // both drivers have a laptime and j's is less than i's
                swap = true;
              }
            }
            else // i doesn't have a laptime
            {
              if( rankedDriver[ j ]->mQualificationTime[ prevQualIndex ] > 0.0f )
              {
                // only j has a laptime
                swap = true;
              }
              else
              {
                // neither have a laptime, so continue onto earlier sessions ...
                continue;
              }
            }

            // We have reached a decision on swapping, so break out early.
            break;
          }

          // Check if we fell out of the loop because there were no qual sessions with comparable data.
          if( prevQualIndex < 0 )
          {
            // Neither reached a qual session, or no times were set for either driver in any session
            // reached, so use internal ranking from practice.
            if( rankedDriver[ j ]->mQualParticipantIndex < rankedDriver[ i ]->mQualParticipantIndex )
              swap = true;
          }
        }

        // Now swap if necessary
        if( swap )
        {
          MultiSessionParticipantV01 *swap = rankedDriver[ i ];
          rankedDriver[ i ] = rankedDriver[ j ];
          rankedDriver[ j ] = swap;
        }
      }
    }

    // Set the server scored variable for each participant, depending on if they made the cut-off
    for( long i = 0; i < info.mNumParticipants; ++i )
    {
      rankedDriver[ i ]->mServerScored = ( i < numScored );

      // tag.2015.07.14 - If a timed lap is required in the previous session in order to continue, check if there is one and turn off server scoring if there is not.
      if( mTimedLapRequired && ( info.mSession >= 6 ) && ( info.mSession <= 8 ) )
      {
        const long prevQualIndex = info.mSession - 5 - 1;
        if( rankedDriver[ i ]->mQualificationTime[ prevQualIndex ] <= 0.0f )
          rankedDriver[ i ]->mServerScored = false;
      }
    }

    // Set qual session reached
    if( ( info.mSession >= 5 ) && ( info.mSession <= 8 ) )
    {
      for( long i = 0; i < info.mNumParticipants; ++i )
      {
        if( rankedDriver[ i ]->mServerScored )
          UpdateDriverProgress( rankedDriver[ i ], beyondPractice )->mQualSessionReached = info.mSession;
      }
    }

    // Set grid position if necessary
    if( setGridPos )
    {
      for( long i = 0; i < info.mNumParticipants; ++i )
        rankedDriver[ i ]->mGridPosition = i + 1; // it's 1-based
    }

    // cleanup
    delete [] rankedDriver;
    rankedDriver = NULL;
  }

  // Save session and return flag to say that we (might have) edited the rules.
  mPreviousSession = info.mSession;
  return( true );
}


