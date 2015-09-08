/*
 * activity.c: A plugin for the Video Disk Recorder
 *
 * See the README file for copyright information and how to reach the author.
 *
 * $Id$
 */

#include <stdlib.h>
#include <stdio.h>
#ifdef USE_XSS
#include <X11/Xlib.h>
#include <X11/extensions/scrnsaver.h>
#endif
#include <utmp.h>
#include <string.h>
#include <sstream>
#include <sys/stat.h>
#include <time.h>
#include <vdr/config.h>
#include <vdr/shutdown.h>
#include "activity.h"

#define MANUALSTART   600 // seconds the next timer must be in the future to assume manual start
#define SLEEPSECCONDS 60  // seconds to sleep while checking for activity in automatic mode

cUserActivity::cUserActivity() {
  automatic = AutomaticStart();
  lastInactivity = 0;
  sleep = new cCondWait();
}

cUserActivity::~cUserActivity() {
  Stop();
  delete sleep;
}

#ifdef USE_XSS
int cUserActivity::DisplayIdleTime(char *display) {
  Display *d;
  static XScreenSaverInfo *mitInfo = 0;
  static int event, error, result=-1;

  if (!(d = XOpenDisplay(display))) {
    esyslog("vdr-useractivity: Unable to open DISPLAY %s\n", display);
    return result;
  }

  if(XScreenSaverQueryExtension(d,&event,&error)) {
    if (!mitInfo) mitInfo = XScreenSaverAllocInfo();
    XScreenSaverQueryInfo(d, DefaultRootWindow (d), mitInfo);
    result = (mitInfo->idle/1000)/60;
  }
  else {
    esyslog("vdr-useractivity: MIT-SCREEN-SAVER missing\n");
    return result;
  }

  XCloseDisplay(d);
  return result;
}
#endif

int cUserActivity::DeviceIdleTime(char *device) {
  using namespace std;
  stringstream stream;
  struct stat stats;
  time_t idle,now;

  time(&now);

  stream << DEV_DIR << device;
  if(stat(stream.str().c_str(), &stats) == 0)
    idle = (now - stats.st_atime)/60;
  else
    idle = -1;

  return idle;
}

int cUserActivity::IdleTime(struct utmp *uptr) {
  int idle = -1;
#ifdef USE_XSS
  /* Detecting display idle time for ut_hosts     *
   * without screen. This is based on Ubuntu 7.10 *
   * where X has valid device in ut_line.         */
  char *cptr = strchr(uptr->ut_host, ':');
  if(cptr != NULL ) {
    if(strchr(cptr+1,'.') == NULL)
      idle = DisplayIdleTime(uptr->ut_host);
  }
#endif
  if(idle < 0)
    idle = DeviceIdleTime(uptr->ut_line); 
  return idle;
}

int cUserActivity::MinIdleTime(void) {
  struct utmp *uptr;
  int idle;
  int result=-1;

  setutent();
  while((uptr = getutent())!=NULL) {
    if(uptr->ut_type == USER_PROCESS) {
      idle = IdleTime(uptr);
      if(idle >= 0 && (idle < result || result == -1)) {
        result = idle;
      }
    }
  }
  endutent();
  return result;
}

bool cUserActivity::AutomaticStart() {
  time_t Delta = Setup.NextWakeupTime ? Setup.NextWakeupTime - time(NULL) : 0;

  if(!Setup.NextWakeupTime || abs(Delta) > MANUALSTART) {
    // Apparently the user started VDR manually
    dsyslog("vdr-useractivity: assuming manual start of VDR");
    return false;
  }
  else {
    dsyslog("vdr-useractivity: scheduled wakeup time in %ld minutes, assuming automatic start of VDR", Delta / 60);
    return true;
  }
}

void cUserActivity::Action(void) {
  int inactivity;
  while (Running()) {
    if(sleep->Wait(SLEEPSECCONDS * 1000))
      break;
    inactivity = min(GetUserInactivity(), MinIdleTime());
    if(inactivity < 0) {
      dsyslog("vdr-useractivity: invalid inactivity = %d", inactivity);
      Stop();
      break;      
    }
    if(inactivity < lastInactivity)
    { dsyslog("vdr-useractivity: activity detected, inactivity = %d, lastInactivity = %d", inactivity, lastInactivity);
      Stop();
      break;
    }
    lastInactivity = inactivity;
  }
}

bool cUserActivity::ActiveUsers(void) {
  if(automatic)
    return false;

  struct utmp *uptr;
  int idle;
  bool result=false;

  setutent();
  while((uptr = getutent())!=NULL) {
    if(uptr->ut_type == USER_PROCESS) {
      idle = IdleTime(uptr);
      if(idle >= 0 && idle < Setup.MinUserInactivity) {
        result = true;
        break;
      }
    }
  }
  endutent();
  return result;
}

void cUserActivity::Stop(void) {
  automatic = false;
  sleep->Signal();
  if(Running())
    Cancel(3);
}

void cUserActivity::SetMinUserInactivity(int minutes) {
  Setup.MinUserInactivity = minutes;
  ShutdownHandler.SetUserInactiveTimeout();
}    

int cUserActivity::GetMinUserInactivity(void) {
  return Setup.MinUserInactivity;
}

char *cUserActivity::GetUsers(void) {
  struct utmp *uptr;
  int idle;
  using namespace std;
  stringstream stream;

  stream << "VDR user has been inactive " << GetUserInactivity() << " minutes." << endl;
  stream << "USER           DEVICE         IDLE      HOST" << endl;
  setutent();
  while((uptr = getutent())!=NULL) {
    if(uptr->ut_type == USER_PROCESS) {
      idle = IdleTime(uptr);
      stream.width(15);
      stream << left << uptr->ut_user;
      stream.width(15);
      stream << left << uptr->ut_line;
      stream.width(10);
      stream << left << idle;
      stream.width(0);
      stream << uptr->ut_host << endl;
    }
  }
  endutent();
  string resultString;
  resultString = stream.str();
  char *result = new char[resultString.size()+1];
  strcpy(result, resultString.c_str());
  return result;
}

int cUserActivity::GetUserInactivity(void) {
  return GetMinUserInactivity() ? 
           GetMinUserInactivity() - 1 -
             (ShutdownHandler.GetUserInactiveTime() - time(NULL))/60 : -1;
}

void cUserActivity::UserActivity(void) {
  ShutdownHandler.SetUserInactiveTimeout();
}

