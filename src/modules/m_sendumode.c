/*
 *   Unreal Internet Relay Chat Daemon, src/modules/m_sendumode.c
 *   (C) 2000-2001 Carsten V. Munk and the UnrealIRCd Team
 *   Moved to modules by Fish (Justin Hammond)
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 1, or (at your option)
 *   any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include "config.h"
#include "struct.h"
#include "common.h"
#include "sys.h"
#include "numeric.h"
#include "msg.h"
#include "channel.h"
#include <time.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
#include <io.h>
#endif
#include <fcntl.h>
#include "h.h"
#ifdef STRIPBADWORDS
#include "badwords.h"
#endif
#ifdef _WIN32
#include "version.h"
#endif

DLLFUNC int m_sendumode(aClient *cptr, aClient *sptr, int parc, char *parv[]);

/* Place includes here */
#define MSG_SENDUMODE   "SENDUMODE"
#define TOK_SENDUMODE   "AP"
#define MSG_SMO         "SMO"
#define TOK_SMO         "AU"

#ifndef DYNAMIC_LINKING
ModuleInfo m_sendumode_info
#else
#define m_sendumode_info mod_header
ModuleInfo mod_header
#endif
  = {
  	2,
	"sendumode",	/* Name of module */
	"$Id$", /* Version */
	"command /sendumode", /* Short description of module */
	NULL, /* Pointer to our dlopen() return value */
	NULL 
    };


/* The purpose of these ifdefs, are that we can "static" link the ircd if we
 * want to
*/

/* This is called on module init, before Server Ready */
#ifdef DYNAMIC_LINKING
DLLFUNC int	mod_init(int module_load)
#else
int    m_sendumode_init(int module_load)
#endif
{
	/*
	 * We call our add_Command crap here
	*/
	add_Command(MSG_SENDUMODE, TOK_SENDUMODE, m_sendumode, MAXPARA);
	add_Command(MSG_SMO, TOK_SMO, m_sendumode, MAXPARA);
}

/* Is first run when server is 100% ready */
#ifdef DYNAMIC_LINKING
DLLFUNC int	mod_load(int module_load)
#else
int    m_sendumode_load(int module_load)
#endif
{
}


/* Called when module is unloaded */
#ifdef DYNAMIC_LINKING
DLLFUNC void	mod_unload(void)
#else
void	m_sendumode_unload(void)
#endif
{
	if (del_Command(MSG_SENDUMODE, TOK_SENDUMODE, m_sendumode) < 0)
	{
		sendto_realops("Failed to delete command sendumode when unloading %s",
				m_sendumode_info.name);
	}
	if (del_Command(MSG_SMO, TOK_SMO, m_sendumode) < 0)
	{
		sendto_realops("Failed to delete command smo when unloading %s",
				m_sendumode_info.name);
	}

}

/*
** m_sendumode - Stskeeps
**      parv[0] = sender prefix
**      parv[1] = target
**      parv[2] = message text
** Pretty handy proc.. 
** Servers can use this to f.x:
**   :server.unreal.net SENDUMODE F :Client connecting at server server.unreal.net port 4141 usw..
** or for sending msgs to locops.. :P
*/
DLLFUNC int m_sendumode(aClient *cptr, aClient *sptr, int parc, char *parv[])
{
	char *message;
	char *p;

	message = parc > 2 ? parv[2] : NULL;

	if (BadPtr(message))
	{
		sendto_one(sptr, err_str(ERR_NEEDMOREPARAMS),
		    me.name, parv[0], "SENDUMODE");
		return 0;
	}

	if (!IsServer(sptr))
	{
		sendto_one(sptr, err_str(ERR_NOPRIVILEGES), me.name, parv[0]);
		return 0;
	}

	sendto_serv_butone(IsServer(cptr) ? cptr : NULL,
	    ":%s SMO %s :%s", parv[0], parv[1], message);


	for (p = parv[1]; *p; p++)
	{
		switch (*p)
		{
		  case 'e':
			  sendto_snomask(SNO_EYES, "%s", parv[2]);
			  break;
		  case 'o':
			  sendto_umode(UMODE_OPER, "%s", parv[2]);
			  break;
		  case 'O':
			  sendto_umode(UMODE_LOCOP, "%s", parv[2]);
			  break;
		  case 'h':
			  sendto_umode(UMODE_HELPOP, "%s", parv[2]);
			  break;
		  case 'N':
			  sendto_umode(UMODE_NETADMIN | UMODE_TECHADMIN, "%s",
			      parv[2]);
			  break;
		  case 'A':
			  sendto_umode(UMODE_ADMIN, "%s", parv[2]);
			  break;
/*		  case '1':
			  sendto_umode(UMODE_CODER, "%s", parv[2]);
			  break;
*/
		  case 'I':
			  sendto_umode(UMODE_HIDING, "%s", parv[2]);
			  break;
		  case 'w':
			  sendto_umode(UMODE_WALLOP, "%s", parv[2]);
			  break;
		  case 's':
			  sendto_umode(UMODE_SERVNOTICE, "%s", parv[2]);
			  break;
		  case 'T':
			  sendto_umode(UMODE_TECHADMIN, "%s", parv[2]);
			  break;
		  case '*':
		  	  sendto_all_butone(NULL, &me, ":%s NOTICE :%s", 
			   	me.name, parv[2]);  	  	
					  	  	 	 
			  break;
		}
	}
	return 0;
}

