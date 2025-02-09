/**
 * @file
 * Parse objects
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
 * @page cli_objects Parse objects
 *
 * Parse objects
 */

#include "config.h"
#include "mutt/lib.h"
#include "objects.h"

/**
 * sa_clear - Empty a StringArray
 * @param sa String Array
 *
 * @note The Array itself isn't freed
 */
static void sa_clear(struct StringArray *sa)
{
  char **cp = NULL;
  ARRAY_FOREACH(cp, sa)
  {
    FREE(cp);
  }

  ARRAY_FREE(sa);
}

/**
 * cli_config_clear - Clear a CliConfig
 * @param cc CliConfig to clear
 */
static void cli_config_clear(struct CliConfig *cc)
{
  sa_clear(&cc->user_files);
}

/**
 * cli_logging_clear - Clear a CliLogging
 * @param cl CliLogging to clear
 */
static void cli_logging_clear(struct CliLogging *cl)
{
  buf_dealloc(&cl->log_level);
  buf_dealloc(&cl->log_file);
}

/**
 * cli_info_clear - Clear a CliInfo
 * @param ci CliInfo to clear
 */
static void cli_info_clear(struct CliInfo *ci)
{
  sa_clear(&ci->alias_queries);
  sa_clear(&ci->queries);
}

/**
 * cli_send_clear - Clear a CliSend
 * @param cs CliSend to clear
 */
static void cli_send_clear(struct CliSend *cs)
{
  sa_clear(&cs->attach);
  sa_clear(&cs->cc_list);
  sa_clear(&cs->bcc_list);
  sa_clear(&cs->addresses);

  buf_dealloc(&cs->draft_file);
  buf_dealloc(&cs->include_file);
  buf_dealloc(&cs->subject);
}

/**
 * cli_gui_clear - Clear a CliGui
 * @param cg CliGui to clear
 */
static void cli_gui_clear(struct CliGui *cg)
{
  sa_clear(&cg->commands);

  buf_dealloc(&cg->folder);
  buf_dealloc(&cg->nntp_server);
  buf_dealloc(&cg->mbox_type);
}

/**
 * command_line_new - Create a new CommandLine
 * @retval ptr New CommandLine
 */
struct CommandLine *command_line_new(void)
{
  return MUTT_MEM_CALLOC(1, struct CommandLine);
}

/**
 * command_line_free - Free a CommandLine
 * @param ptr CommandLine to free
 */
void command_line_free(struct CommandLine **ptr)
{
  if (!ptr || !*ptr)
    return;

  struct CommandLine *cl = *ptr;

  // cl->help - nothing to do
  cli_config_clear(&cl->config);
  cli_logging_clear(&cl->logging);
  cli_info_clear(&cl->info);
  cli_send_clear(&cl->send);
  cli_gui_clear(&cl->gui);

  FREE(ptr);
}
