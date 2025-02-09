/**
 * @file
 * Parse the Command Line
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
 * @page cli_parse Parse the Command Line
 *
 * Parse the Command Line
 */

#include "config.h"
#include <stdbool.h>
#include <unistd.h>
#include "mutt/lib.h"
#include "objects.h"

/**
 * cli_parse - Parse the Command Line
 * @param[in]  argc Number of arguments
 * @param[in]  argv Arguments
 * @param[out] cli  Results
 * @retval true Success
 */
bool cli_parse(int argc, char **argv, struct CommandLine *cli)
{
  if ((argc < 1) || !argv || !cli)
    return false;

  int count = 0;
  for (int i = 1; i < argc; i++)
  {
    if (argv[i][0] == '\0')
      break;

    if ((argv[i][0] == '-') && (argv[i][1] != '\0'))
      break;

    ARRAY_ADD(&cli->send.addresses, mutt_str_dup(argv[i]));
    count++;
  }

  if (count > 0)
  {
    cli->send.is_set = true;
    argc -= count;
    argv += count;
  }

  bool rc = true;

  int double_dash = argc;

  bool first = true;
  for (optind = 1; optind < double_dash;)
  {
    // This loop collects any non-options, e.g. addresses, queries
    for (; optind < argc; optind++)
    {
      if ((argv[optind][0] == '-') && (argv[optind][1] != '\0'))
      {
        if ((argv[optind][1] == '-') && (argv[optind][2] == '\0'))
          double_dash = optind; // quit outer loop after getopt
        break;                  // drop through to getopt
      }

      // Mop up a non-option: continuation of -A/-Q, an address or an attachment
      if (!ARRAY_EMPTY(&cli->info.alias_queries))
      {
        ARRAY_ADD(&cli->info.alias_queries, mutt_str_dup(argv[optind]));
      }
      else if (!ARRAY_EMPTY(&cli->info.queries))
      {
        ARRAY_ADD(&cli->info.queries, mutt_str_dup(argv[optind]));
      }
      else if (!ARRAY_EMPTY(&cli->send.attach))
      {
        ARRAY_ADD(&cli->send.attach, mutt_str_dup(argv[optind]));
      }
      else
      {
        ARRAY_ADD(&cli->send.addresses, mutt_str_dup(argv[optind]));
        cli->send.is_set = true;
      }
    }

    // When running tests, cli_parse() gets called many times.
    // Ensure that (optind == 0) the first time to force getopt() to reset itself.
    if (first)
    {
      optind = 0;
      first = false;
    }
    int opt = getopt(argc, argv, "+A:a:b:Cc:Dd:Ee:F:f:Gg:hH:i:l:m:nOpQ:RSs:TvyZz");
    if (opt == -1)
      continue;

    switch (opt)
    {
      // ------------------------------------------------------------
      // Help
      case 'h': // help
      {
        cli->help.help = true;
        cli->help.is_set = true;
        break;
      }
      case 'v': // version, license
      {
        if (cli->help.version)
          cli->help.license = true;
        else
          cli->help.version = true;

        cli->help.is_set = true;
        break;
      }

      // ------------------------------------------------------------
      // Config
      case 'n': // no system config file
      {
        cli->config.disable_system = true;
        cli->config.is_set = true;
        break;
      }
      case 'F': // user config file
      {
        ARRAY_ADD(&cli->config.user_files, mutt_str_dup(optarg));
        cli->config.is_set = true;
        break;
      }

      // ------------------------------------------------------------
      // Logging
      case 'd': // log level
      {
        buf_strcpy(&cli->logging.log_level, optarg);
        cli->logging.is_set = true;
        break;
      }
      case 'l': // log file
      {
        buf_strcpy(&cli->logging.log_file, optarg);
        cli->logging.is_set = true;
        break;
      }

      // ------------------------------------------------------------
      // Info
      case 'A': // alias lookup
      {
        ARRAY_ADD(&cli->info.alias_queries, mutt_str_dup(optarg));
        cli->info.is_set = true;
        break;
      }
      case 'D': // dump config, dump changed
      {
        if (cli->info.dump_config)
          cli->info.dump_changed = true;
        else
          cli->info.dump_config = true;

        cli->info.is_set = true;
        break;
      }
      case 'O': // one-liner help
      {
        cli->info.show_help = true;
        cli->info.is_set = true;
        break;
      }
      case 'Q': // query config&cli->send.attach
      {
        ARRAY_ADD(&cli->info.queries, mutt_str_dup(optarg));
        cli->info.is_set = true;
        break;
      }
      case 'S': // hide sensitive
      {
        cli->info.hide_sensitive = true;
        cli->info.is_set = true;
        break;
      }

      // ------------------------------------------------------------
      // Send
      case 'a': // attach file
      {
        ARRAY_ADD(&cli->send.attach, mutt_str_dup(optarg));
        cli->send.is_set = true;
        break;
      }
      case 'b': // bcc:
      {
        ARRAY_ADD(&cli->send.bcc_list, mutt_str_dup(optarg));
        cli->send.is_set = true;
        break;
      }
      case 'c': // cc:
      {
        ARRAY_ADD(&cli->send.cc_list, mutt_str_dup(optarg));
        cli->send.is_set = true;
        break;
      }
      case 'C': // crypto
      {
        cli->send.use_crypto = true;
        cli->send.is_set = true;
        break;
      }
      case 'E': // edit file
      {
        cli->send.edit_infile = true;
        cli->send.is_set = true;
        break;
      }
      case 'H': // draft file
      {
        buf_strcpy(&cli->send.draft_file, optarg);
        cli->send.is_set = true;
        break;
      }
      case 'i': // include file
      {
        buf_strcpy(&cli->send.include_file, optarg);
        cli->send.is_set = true;
        break;
      }
      case 's': // subject:
      {
        buf_strcpy(&cli->send.subject, optarg);
        cli->send.is_set = true;
        break;
      }

      // ------------------------------------------------------------
      // GUI
      case 'e': // enter commands
      {
        ARRAY_ADD(&cli->gui.commands, mutt_str_dup(optarg));
        cli->gui.is_set = true;
        break;
      }
      case 'f': // start folder
      {
        buf_strcpy(&cli->gui.folder, optarg);
        cli->gui.is_set = true;
        break;
      }
      case 'g': // news server
      {
        cli->gui.start_nntp = true;
        buf_strcpy(&cli->gui.nntp_server, optarg);
        cli->gui.is_set = true;
        break;
      }
      case 'G': // list newsgroups
      {
        cli->gui.start_nntp = true;
        cli->gui.is_set = true;
        break;
      }
      case 'm': // mbox type
      {
        buf_strcpy(&cli->gui.mbox_type, optarg);
        cli->gui.is_set = true;
        break;
      }
      case 'p': // postponed
      {
        cli->gui.start_postponed = true;
        cli->gui.is_set = true;
        break;
      }
      case 'R': // read-only
      {
        cli->gui.read_only = true;
        cli->gui.is_set = true;
        break;
      }
      case 'y': // browser
      {
        cli->gui.start_browser = true;
        cli->gui.is_set = true;
        break;
      }
      case 'z': // any mail
      {
        cli->gui.start_any_mail = true;
        cli->gui.is_set = true;
        break;
      }
      case 'Z': // new mail
      {
        cli->gui.start_new_mail = true;
        cli->gui.is_set = true;
        break;
      }

      // ------------------------------------------------------------
      default:
      {
        cli->help.help = true;
        cli->help.is_set = true;
        rc = false;
        break;
      }
    }
  }

  // Any remaining args happened after '--', so must be addresses
  if (optind < argc)
  {
    cli->send.is_set = true;
    while (optind < argc)
    {
      ARRAY_ADD(&cli->send.addresses, mutt_str_dup(argv[optind++]));
    }
  }

  return rc;
}
