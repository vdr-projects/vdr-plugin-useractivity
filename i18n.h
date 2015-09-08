/*
 * i18n.h: Internationalization
 *
 * See the README file for copyright information and how to reach the author.
 *
 * $Id:$
 */

#ifndef _I18N__H
#define _I18N__H

#include <vdr/i18n.h>

#ifndef trNOOP
#define trNOOP(s) (s)
#endif

#if VDRVERSNUM < 10507
extern const tI18nPhrase Phrases[];
#endif

#endif //_I18N__H
