/*
 * useractivity.c: A plugin for the Video Disk Recorder
 *
 * See the README file for copyright information and how to reach the author.
 *
 * $Id$
 */

#include <vdr/plugin.h>
#include "i18n.h"
#include "activity.h"

static const char *VERSION        = "0.0.1";
static const char *DESCRIPTION    = "Prevents shutdown if there are active users";
//static const char *MAINMENUENTRY  = "Active users";

class cPluginUseractivity : public cPlugin {
private:
  // Add any member variables or functions you may need here.
public:
  cPluginUseractivity(void);
  virtual ~cPluginUseractivity();
  virtual const char *Version(void) { return VERSION; }
  virtual const char *Description(void) { return tr(DESCRIPTION); }
  virtual const char *CommandLineHelp(void);
  virtual bool ProcessArgs(int argc, char *argv[]);
  virtual bool Initialize(void);
  virtual bool Start(void);
  virtual void Stop(void);
  virtual void Housekeeping(void);
  virtual void MainThreadHook(void);
  virtual cString Active(void);
  virtual const char *MainMenuEntry(void) { return NULL; /* tr(MAINMENUENTRY) */ }
  virtual cOsdObject *MainMenuAction(void);
  virtual cMenuSetupPage *SetupMenu(void);
  virtual bool SetupParse(const char *Name, const char *Value);
  virtual bool Service(const char *Id, void *Data = NULL);
  virtual const char **SVDRPHelpPages(void);
  virtual cString SVDRPCommand(const char *Command, const char *Option, int &ReplyCode);
  };

cPluginUseractivity::cPluginUseractivity(void)
{
  // Initialize any member variables here.
  // DON'T DO ANYTHING ELSE THAT MAY HAVE SIDE EFFECTS, REQUIRE GLOBAL
  // VDR OBJECTS TO EXIST OR PRODUCE ANY OUTPUT!
}

cPluginUseractivity::~cPluginUseractivity()
{
  // Clean up after yourself!
}

const char *cPluginUseractivity::CommandLineHelp(void)
{
  // Return a string that describes all known command line options.
  return NULL;
}

bool cPluginUseractivity::ProcessArgs(int argc, char *argv[])
{
  // Implement command line argument processing here if applicable.
  return true;
}

bool cPluginUseractivity::Initialize(void)
{
  // Initialize any background activities the plugin shall perform.
  return true;
}

bool cPluginUseractivity::Start(void)
{
  // Start any background activities the plugin shall perform.
  RegisterI18n(Phrases);
  return true;
}

void cPluginUseractivity::Stop(void)
{
  // Stop any background activities the plugin shall perform.
}

void cPluginUseractivity::Housekeeping(void)
{
  // Perform any cleanup or other regular tasks.
}

void cPluginUseractivity::MainThreadHook(void)
{
  // Perform actions in the context of the main program thread.
  // WARNING: Use with great care - see PLUGINS.html!
}

cString cPluginUseractivity::Active(void)
{
  // Return a message string if shutdown should be postponed
  if(cUserActivity::ActiveUsers())
    return tr("There are active users in the system");
  else
    return NULL;
}

cOsdObject *cPluginUseractivity::MainMenuAction(void)
{
  // Perform the action when selected from the main VDR menu.
  return NULL;
}

cMenuSetupPage *cPluginUseractivity::SetupMenu(void)
{
  // Return a setup menu in case the plugin supports one.
  return NULL;
}

bool cPluginUseractivity::SetupParse(const char *Name, const char *Value)
{
  // Parse your own setup parameters and store their values.
  return false;
}

bool cPluginUseractivity::Service(const char *Id, void *Data)
{
  // Handle custom service requests from other plugins
  return false;
}

const char **cPluginUseractivity::SVDRPHelpPages(void)
{
  // Return help text for SVDRP commands this plugin implements
  static const char *HelpPages[] = {
    "GETI\n"
    "    Display minimum user inactivity in minutes.",
    "LSTU\n"
    "    Display a list of users.",
    "SETI <minutes>\n"
    "    Set minimum user inactivity in minutes.",
    NULL
    };
  return HelpPages;
}

cString cPluginUseractivity::SVDRPCommand(const char *Command, const char *Option, int &ReplyCode)
{
  // Process SVDRP commands this plugin implements
  if (strcasecmp(Command, "GETI") == 0) {
    // we use the default reply code here
    return cString::sprintf("Minimum user inactivity is %d minutes.", cUserActivity::GetMinUserInactivity());
  }
  else if (strcasecmp(Command, "LSTU") == 0) {
    // we use the default reply code here
    return cString(cUserActivity::GetUsers());
  }
  else if (strcasecmp(Command, "SETI") == 0) {
    ReplyCode = 501;
    if (*Option) {
      int minutes = atoi(Option);
      if (minutes >= 0) {
        cUserActivity::SetMinUserInactivity(minutes);
        ReplyCode = 250;
        return cString::sprintf("Minimum user inactivity set to %d minutes.", minutes);
      }
      else 
        return cString::sprintf("Invalid Option \"%s\"", Option);
    }
    else
      return cString("No option given");
  }
  return NULL;
}

VDRPLUGINCREATOR(cPluginUseractivity); // Don't touch this!
