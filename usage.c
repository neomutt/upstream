/**
 * @file
 * Display Usage Information for NeoMutt
 *
 * @authors
 * Copyright (C) 2025 Richard Russon <rich@flatcap.org>
 *
 * @copyright
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 2 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * @page neo_usage Display Usage Information for NeoMutt
 *
 * Display Usage Information for NeoMutt
 */

#include "config.h"
#include <stdbool.h>
#include <stdio.h>
#include "mutt/lib.h"
#include "cli/lib.h"
#include "muttlib.h"

/**
 * print_header - Print a section header
 * @param section   Help section
 * @param desc      Description
 * @param use_color Highlight parts of the text
 */
static void print_header(const char *section, const char *desc, bool use_color)
{
  if (use_color)
    printf("\033[1;4m%s\033[0m: %s\n", section, desc);
  else
    printf("%s: %s\n", section, desc);
}

/**
 * show_cli_overview - Display NeoMutt command line
 * @param use_color Highlight parts of the text
 */
static void show_cli_overview(bool use_color)
{
  printf("%s ", _("NeoMutt has four modes of operation:"));
  if (use_color)
    puts("\033[1;4mhelp\033[0m, \033[1;4minfo\033[0m, \033[1;4msend\033[0m, \033[1;4mgui\033[0m");
  else
    puts("help, info, send, gui");

  puts(_("The default mode, if no command line arguments are specified, is gui."));
  puts("");

  print_header("shared", _("Options that apply to all modes"), use_color);
  puts(_("  neomutt -F <config>                  Use this config file"));
  puts(_("          -n                           Don't use system config file"));
  puts(_("          -e <command>                 Run extra commands"));
  puts(_("          -m <type>                    Set default mailbox type"));
  puts(_("          -d <level>                   Set logging level"));
  puts(_("          -l <file>                    Set logging file"));
  puts("");

  print_header("help", _("Get command line help for NeoMutt"), use_color);
  puts(_("  neomutt -h <mode>                    Detailed help for a mode"));
  puts(_("          -v[v]                        Version or license"));
  puts("");

  print_header("info", _("Ask NeoMutt for config information"), use_color);
  puts(_("  neomutt -A <alias> [...]             Lookup email aliases"));
  puts(_("          -D [-D] [-O] [-S]            Dump the config"));
  puts(_("          -Q <option> [...] [-O] [-S]  Query config options"));
  puts("");

  print_header("send", _("Send an email from the command line"), use_color);
  puts(_("  neomutt -a <file> [...]              Attach files"));
  puts(_("          -b <address>                 Add Bcc: address"));
  puts(_("          -C                           Use crypto"));
  puts(_("          -c <address>                 Add Cc: address"));
  puts(_("          -E                           Edit message"));
  puts(_("          -H <draft>                   Use draft email"));
  puts(_("          -i <include>                 Include body file"));
  puts(_("          -s <subject>                 Set Subject:"));
  puts(_("          -- <address> [...]           Add To: addresses"));
  puts("");

  print_header("gui", _("Start the NeoMutt GUI"), use_color);
  puts(_("  neomutt                              Start the NeoMutt GUI"));
  puts(_("          -f <mailbox>                 Open this mailbox"));
  puts(_("          -G                           Open NNTP browser"));
  puts(_("          -g <server>                  Use this NNTP server"));
  puts(_("          -p                           Resume postponed email"));
  puts(_("          -R                           Open mailbox read-only"));
  puts(_("          -y                           Open mailbox browser"));
  puts(_("          -Z                           Check for new mail"));
  puts(_("          -z                           Check for any mail"));
  puts("");

  if (use_color)
    printf("%s \033[1m%s\033[0m\n", _("For detailed help, run:"), "neomutt -h all");
  else
    printf("%s %s\n", _("For detailed help, run:"), "neomutt -h all");
}

/**
 * show_cli_help - Show Command Line Help for Help
 * @param use_color Highlight parts of the text
 */
static void show_cli_help(bool use_color)
{
  puts(
  "Get command line help for NeoMutt\n"
  "\n"
  "neomutt -h [section] -v[v]\n"
  "\n"
  "  -h            Short overview of options\n"
  "  -h <section>  Detailed help about: help,config,logging,info,send,gui,all\n"
  "  -v            Print the NeoMutt version and compile-time definitions\n"
  "  -vv           Print the NeoMutt license and copyright information\n"
  "\n"
  "Examples:\n"
  "  neomutt -h info\n"
  "  neomutt -v\n"
  "\n"
  );
  // exits after display
}

/**
 * show_cli_shared - Show Command Line Help for Shared
 * @param use_color Highlight parts of the text
 */
static void show_cli_shared(bool use_color)
{
  puts(
  "Tell NeoMutt which config files to use\n"
  "\n"
  "  -F <config>   Specify an alternative initialization file to read\n"
  "                This option may be used multiple times\n"
  "  -n            Do not read the system-wide configuration file\n"
  "\n"
  "Examples:\n"
  "  neomutt -F work.rc\n"
  "  neomutt -F work.rc -F colours.rc\n"
  "  neomutt -n\n"
  "\n"
  );
  // link to web guide -- which files are read
  // or auto-generate it!
  puts(
  "Enable logging\n"
  "\n"
  "neomutt -d <level> -l <file>\n"
  "\n"
  "  -d <level>    Set the logging level (range: 0-5)\n"
  "                0 (disabled) .. 5 (very verbose)\n"
  "  -l <file>     Write the log to this file\n"
  "\n"
  "Examples:\n"
  "  neomutt -d 2\n"
  "  neomutt -d 5 -l neolog\n"
  "\n"
  "  -e <command>  Specify a command to be run after reading the config files\n"
  "  -m <type>     Specify a default mailbox format type for newly created folders\n"
  "                The type is either MH, MMDF, Maildir or mbox (case-insensitive)\n"
  );
  // default file '~/.neomuttdebug0'
  // Files are rotated, max 5
  // overrides config file
}

/**
 * show_cli_info - Show Command Line Help for Info
 * @param use_color Highlight parts of the text
 */
static void show_cli_info(bool use_color)
{
  puts(
  "Ask NeoMutt for config information\n"
  "\n"
  "neomutt -A <alias> [...]\n"
  "neomutt -D [-D] [-O] [-S]\n"
  "neomutt -Q <option> [...] [-O] [-S]\n"
  "\n"
  "  -A <alias> [...] Lookup an email alias\n"
  "                Multiple aliases can be looked up at once (space-separated)\n"
  "  -D            Dump all the config options\n"
  "  -D -D         (or -DD) Like -D, but only show changed config\n"
  "  -Q <option> [...] Query one configuration option\n"
  "                Multiple options can be looked up at once (space-separated)\n"
  "  -O            Add one-liner documentation\n"
  "  -S            Hide the value of sensitive options\n"
  "\n"
  "Examples:\n"
  "  neomutt -A flatcap gahr\n"
  "  neomutt -D -O\n"
  "  neomutt -DD -S\n"
  "  neomutt -O -Q alias_format\n"
  "\n"
  );
  // exits after display
  // -D / -Q expect to be run AFTER -e has been processed!
}

/**
 * show_cli_send - Show Command Line Help for Send
 * @param use_color Highlight parts of the text
 */
static void show_cli_send(bool use_color)
{
  puts(
  "Send an email from the command line\n"
  "\n"
  "  neomutt [-CEn] [-e <command>] [-F <config>] [-H <draft>] [-i <include>]\n"
  "          [-b <address>] [-c <address>] [-s <subject>] [-a <file> [...] --]\n"
  "          <address> [...]\n"
  "  neomutt [-Cn] [-e <command>] [-F <config>] [-b <address>] [-c <address>]\n"
  "          [-s <subject>] [-a <file> [...] --] <address> [...] < message\n"
  "\n"
  "  -a <file>     Attach one or more files to a message\n"
  "                Add any addresses after the '--' argument\n"
  "  -b <address>  Specify a blind carbon copy (Bcc) recipient\n"
  "  -C            Enable Command-line Crypto (signing/encryption)\n"
  "  -c <address>  Specify a carbon copy (Cc) recipient\n"
  "  -E            Edit draft (-H) or include (-i) file during message composition\n"
  "  -H <draft>    Specify a draft file with header and body for message composing\n"
  "  -i <include>  Specify an include file to be embedded in the body of a message\n"
  "  -s <subject>  Specify a subject (must be enclosed in quotes if it has spaces)\n"
  "  --            Special argument forces NeoMutt to stop option parsing and treat\n"
  "                remaining arguments as addresses even if they start with a dash\n"
  "\n"
  );
  // example with piping or < message
}

/**
 * show_cli_gui - Show Command Line Help for Gui
 * @param use_color Highlight parts of the text
 */
static void show_cli_gui(bool use_color)
{
  puts(
  "Start the NeoMutt GUI\n"
  "\n"
  "  neomutt [-CEn] [-e <command>] [-F <config>] [-H <draft>] [-i <include>]\n"
  "          [-b <address>] [-c <address>] [-s <subject>] [-a <file> [...] --]\n"
  "          <address> [...]\n"
  "  neomutt [-Cn] [-e <command>] [-F <config>] [-b <address>] [-c <address>]\n"
  "          [-s <subject>] [-a <file> [...] --] <address> [...] < message\n"
  "  neomutt [-nRy] [-e <command>] [-F <config>] [-f <mailbox>] [-m <type>]\n"
  "\n"
  "  -f <mailbox>  Specify a mailbox (as defined with 'mailboxes' command) to load\n"
  "  -G            Start NeoMutt with a listing of subscribed newsgroups\n"
  "  -g <server>   Like -G, but start at specified news server\n"
  "  -p            Resume a prior postponed message, if any\n"
  "  -R            Open mailbox in read-only mode\n"
  "  -y            Start NeoMutt with a listing of all defined mailboxes\n"
  "  -Z            Open the first mailbox with new message or exit immediately with\n"
  "                exit code 1 if none is found in all defined mailboxes\n"
  "  -z            Open the first or specified (-f) mailbox if it holds any message\n"
  "                or exit immediately with exit code 1 otherwise\n"
  "\n"
  );
}

/**
 * show_cli - Show Instructions on how to run NeoMutt
 * @param mode      Details, e.g. #HM_CONFIG
 * @param use_color Highlight parts of the text
 */
void show_cli(enum HelpMode mode, bool use_color)
{
  if (use_color)
    printf("\033[1;36m%s\033[0m\n\n", mutt_make_version());
  else
    printf("%s\n\n", mutt_make_version());

  switch (mode)
  {
    case HM_NONE:
      show_cli_overview(use_color);
      break;
    case HM_SHARED:
      show_cli_shared(use_color);
      break;
    case HM_HELP:
      show_cli_help(use_color);
      break;
    case HM_INFO:
      show_cli_info(use_color);
      break;
    case HM_SEND:
      show_cli_send(use_color);
      break;
    case HM_GUI:
      show_cli_gui(use_color);
      break;
    case HM_ALL:
      show_cli_shared(use_color);
      show_cli_help(use_color);
      show_cli_info(use_color);
      show_cli_send(use_color);
      show_cli_gui(use_color);
      break;
  }
}
