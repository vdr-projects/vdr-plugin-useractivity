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
#include <vdr/thread.h>

class cUserActivity : public cThread {
  private:
    bool automatic;
    int lastInactivity;
    cCondWait *sleep;
#ifdef USE_XSS
    static int DisplayIdleTime(char *display);
#endif
    static int DeviceIdleTime(char *device);
    static int IdleTime(struct utmp *uptr);
    static int MinIdleTime();
    static bool AutomaticStart();
  protected:
    virtual void Action(void);
  public:
    cUserActivity();
    ~cUserActivity();
    bool ActiveUsers(void);    
    bool IsAutomatic(void) { return automatic; }
    void Stop(void);
    static void SetMinUserInactivity(int minutes);    
    static int GetMinUserInactivity(void);    
    static char *GetUsers(void);
    static int GetUserInactivity(void);
    static void UserActivity(void);
};

#endif
