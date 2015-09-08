/*
 * activity.c: A plugin for the Video Disk Recorder
 *
 * See the README file for copyright information and how to reach the author.
 *
 * $Id$
 */

#include <stdlib.h>
#include <stdio.h>
#include <syslog.h>
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
#if VDRVERSNUM >= 10501
#include <vdr/shutdown.h>
#endif
#include "activity.h"

#ifdef USE_XSS
int cUserActivity::DisplayIdleTime(char *display) {
  Display *d;
  static XScreenSaverInfo *mitInfo = 0;
  static int event, error, result=-1;

  if (!(d = XOpenDisplay(display))) {
    syslog(LOG_ERR, "useractivity: Unable to open DISPLAY %s\n", display);
    return result;
  }

  if(XScreenSaverQueryExtension(d,&event,&error)) {
    if (!mitInfo) mitInfo = XScreenSaverAllocInfo();
    XScreenSaverQueryInfo(d, DefaultRootWindow (d), mitInfo);
    result = (mitInfo->idle/1000)/60;
  }
  else {
    syslog(LOG_ERR, "useractivity: MIT-SCREEN-SAVER missing\n");
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


bool cUserActivity::ActiveUsers(void) {
  struct utmp *uptr;
  int idle;
  bool result=false;

  setutent();
  while((uptr = getutent())!=NULL) {
    if(uptr->ut_type == USER_PROCESS) {
      idle = DeviceIdleTime(uptr->ut_line);
#ifdef USE_XSS
      if(idle < 0 && strchr(uptr->ut_line, ':') != NULL)
        idle = DisplayIdleTime(uptr->ut_line);
#endif
      if(idle >= 0 && idle < Setup.MinUserInactivity) {
        result = true;
        break;
      }
    }
  }
  endutent();
  return result;
}

void cUserActivity::SetMinUserInactivity(int minutes) {
  Setup.MinUserInactivity = minutes;
}    

int cUserActivity::GetMinUserInactivity(void) {
  return Setup.MinUserInactivity;
}

char *cUserActivity::GetUsers(void) {
  struct utmp *uptr;
  int idle;
  using namespace std;
  stringstream stream;

#if VDRVERSNUM >= 10501
  stream << "VDR user has been inactive " << GetUserInactivity() << " minutes." << endl;
#endif
  stream << "USER           DEVICE         IDLE" << endl;
  setutent();
  while((uptr = getutent())!=NULL) {
    if(uptr->ut_type == USER_PROCESS) {
      idle = DeviceIdleTime(uptr->ut_line);
#ifdef USE_XSS
      if(idle < 0 && strchr(uptr->ut_line, ':') != NULL)
        idle = DisplayIdleTime(uptr->ut_line);
#endif
      stream.width(15);
      stream << left << uptr->ut_user;
      stream.width(15);
      stream << left << uptr->ut_line;
      stream.width(0);
      stream << idle << endl;
    }
  }
  endutent();
  string resultString;
  resultString = stream.str();
  char *result = new char[resultString.size()+1];
  strcpy(result, resultString.c_str());
  return result;
}

#if VDRVERSNUM >= 10501
int cUserActivity::GetUserInactivity(void) {
  return GetMinUserInactivity() ? 
           GetMinUserInactivity() - 1 -
             (ShutdownHandler.GetUserInactiveTime() - time(NULL))/60 : -1;
}

void cUserActivity::UserActivity(void) {
  ShutdownHandler.SetUserInactiveTimeout();
}
#endif

