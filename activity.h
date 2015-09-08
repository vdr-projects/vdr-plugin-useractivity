/*
 * activity.h: A plugin for the Video Disk Recorder
 *
 * See the README file for copyright information and how to reach the author.
 *
 * $Id$
 */

#ifndef _USERACTILITY_ACTIVITY_H
#define _USERACTILITY_ACTIVITY_H

#include <utmp.h>

class cUserActivity {
  private:
#ifdef USE_XSS
    static int DisplayIdleTime(char *display);
#endif
    static int DeviceIdleTime(char *device);
    static int IdleTime(struct utmp *uptr);
  public:
    static bool ActiveUsers(void);    
    static void SetMinUserInactivity(int minutes);    
    static int GetMinUserInactivity(void);    
    static char *GetUsers(void);
#if VDRVERSNUM >= 10501
    static int GetUserInactivity(void);
    static void UserActivity(void);
#endif
};

#endif
