/*
 * activity.h: A plugin for the Video Disk Recorder
 *
 * See the README file for copyright information and how to reach the author.
 *
 * $Id$
 */

#ifndef _USERACTILITY_ACTIVITY_H
#define _USERACTILITY_ACTIVITY_H

class cUserActivity {
  private:
#ifdef USE_XSS
    static int DisplayIdleTime(char *display);
#endif
    static int DeviceIdleTime(char *device);
  public:
    static bool ActiveUsers(void);    
    static void SetMinUserInactivity(int minutes);    
    static int GetMinUserInactivity(void);    
    static char *GetUsers(void);
};

#endif
