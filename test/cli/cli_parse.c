/**
 * @file
 * Test code for Command Line Parsing
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

#define TEST_NO_MAIN
#include "config.h"
#include "acutest.h"
#include <math.h>
#include <stdbool.h>
#include <stddef.h>
#include "mutt/lib.h"
#include "cli/lib.h"
#include "test_common.h"

static void serialise_bool(bool b, struct Buffer *res)
{
  if (b)
    buf_addch(res, 'Y');
  else
    buf_addch(res, 'N');
}

static void serialise_buffer(struct Buffer *value, struct Buffer *res)
{
  if (buf_is_empty(value))
    buf_add_printf(res, ":-");
  else
    buf_add_printf(res, ":%s", buf_string(value));
}

static void serialise_array(const struct StringArray *sa, struct Buffer *res)
{
  buf_addstr(res, ":{");

  char **cp = NULL;
  ARRAY_FOREACH(cp, sa)
  {
    buf_addstr(res, *cp);
    if (ARRAY_FOREACH_IDX_cp < (ARRAY_SIZE(sa) - 1))
      buf_addch(res, ',');
  }

  buf_addch(res, '}');
}

static void serialise_help(struct CliHelp *ch, struct Buffer *res)
{
  if (!ch->is_set)
    return;

  buf_addstr(res, "H(");

  serialise_bool(ch->help, res);
  serialise_bool(ch->version, res);
  serialise_bool(ch->license, res);

  buf_addstr(res, ")");
}

static void serialise_config(struct CliConfig *cc, struct Buffer *res)
{
  if (!cc->is_set)
    return;

  buf_addstr(res, "C(");

  serialise_bool(cc->disable_system, res);

  serialise_array(&cc->user_files, res);

  buf_addstr(res, ")");
}

static void serialise_logging(struct CliLogging *cl, struct Buffer *res)
{
  if (!cl->is_set)
    return;

  buf_addstr(res, "L(");

  serialise_buffer(&cl->log_level, res);
  serialise_buffer(&cl->log_file, res);

  buf_addstr(res, ")");
}

static void serialise_info(struct CliInfo *ci, struct Buffer *res)
{
  if (!ci->is_set)
    return;

  buf_addstr(res, "I(");

  serialise_bool(ci->dump_config, res);
  serialise_bool(ci->dump_changed, res);
  serialise_bool(ci->show_help, res);
  serialise_bool(ci->hide_sensitive, res);

  serialise_array(&ci->alias_queries, res);
  serialise_array(&ci->queries, res);

  buf_addstr(res, ")");
}

static void serialise_send(struct CliSend *cs, struct Buffer *res)
{
  if (!cs->is_set)
    return;

  buf_addstr(res, "S(");

  serialise_bool(cs->use_crypto, res);
  serialise_bool(cs->edit_infile, res);

  serialise_array(&cs->attach, res);
  serialise_array(&cs->bcc_list, res);
  serialise_array(&cs->cc_list, res);
  serialise_array(&cs->addresses, res);

  serialise_buffer(&cs->draft_file, res);
  serialise_buffer(&cs->include_file, res);
  serialise_buffer(&cs->subject, res);

  buf_addstr(res, ")");
}

static void serialise_gui(struct CliGui *cg, struct Buffer *res)
{
  if (!cg->is_set)
    return;

  buf_addstr(res, "G(");

  serialise_bool(cg->read_only, res);
  serialise_bool(cg->start_postponed, res);
  serialise_bool(cg->start_browser, res);
  serialise_bool(cg->start_nntp, res);
  serialise_bool(cg->start_new_mail, res);
  serialise_bool(cg->start_any_mail, res);

  serialise_array(&cg->commands, res);

  serialise_buffer(&cg->folder, res);
  serialise_buffer(&cg->nntp_server, res);
  serialise_buffer(&cg->mbox_type, res);

  buf_addstr(res, ")");
}

static void serialise_cli(struct CommandLine *cli, struct Buffer *res)
{
  buf_reset(res);
  serialise_help(&cli->help, res);
  serialise_config(&cli->config, res);
  serialise_logging(&cli->logging, res);
  serialise_info(&cli->info, res);
  serialise_send(&cli->send, res);
  serialise_gui(&cli->gui, res);
}

static void args_split(const char *args, struct StringArray *sa)
{
  // Fake entry at argv[0]
  ARRAY_ADD(sa, mutt_str_dup("neomutt"));

  char *src = mutt_str_dup(args);
  if (!src)
    return;

  char *start = src;
  for (char *p = start; *p; p++)
  {
    if (p[0] != ' ')
      continue;

    p[0] = '\0';
    ARRAY_ADD(sa, mutt_str_dup(start));

    start = p + 1;
  }

  ARRAY_ADD(sa, mutt_str_dup(start));

  FREE(&src);
}

static void args_clear(struct StringArray *sa)
{
  char **cp = NULL;
  ARRAY_FOREACH(cp, sa)
  {
    FREE(cp);
  }

  ARRAY_FREE(sa);
}

void test_cli_parse(void)
{
  // bool cli_parse(int argc, char **argv, struct CommandLine *cli);

  bool rc;

  // Degenerate
  {
    char *args = "apple banana";
    struct CommandLine *cli = command_line_new();
    struct StringArray sa = ARRAY_HEAD_INITIALIZER;
    args_split(args, &sa);

    rc = cli_parse(0, sa.entries, cli);
    TEST_CHECK(rc == false);

    rc = cli_parse(2, NULL, cli);
    TEST_CHECK(rc == false);

    rc = cli_parse(2, sa.entries, NULL);
    TEST_CHECK(rc == false);

    args_clear(&sa);
    command_line_free(&cli);

    command_line_free(NULL);
  }

  // Simple tests
  {
    static const char *Tests[][2] = {
      // clang-format off
      // No args
      { "",                      "" },

      // Help
      { "-h",                    "H(YNN)" },
      { "-v",                    "H(NYN)" },
      { "-h -v",                 "H(YYN)" },
      { "-v -v",                 "H(NYY)" },
      { "-vv",                   "H(NYY)" },
      { "-vhv",                  "H(YYY)" },

      // Config
      { "-n",                    "C(Y:{})" },
      { "-F apple",              "C(N:{apple})" },
      { "-F apple -F banana",    "C(N:{apple,banana})" },
      { "-nF apple",             "C(Y:{apple})" },
      { "-F apple -n -F banana", "C(Y:{apple,banana})" },

      // Logging
      { "-d 3",                  "L(:3:-)" },
      { "-d3",                   "L(:3:-)" },
      { "-l apple",              "L(:-:apple)" },
      { "-lapple",               "L(:-:apple)" },
      { "-d 3 -l apple",         "L(:3:apple)" },
      { "-d3 -lapple",           "L(:3:apple)" },

      // Info
      { "-D",                    "I(YNNN:{}:{})" },
      { "-D -D",                 "I(YYNN:{}:{})" },
      { "-D -O",                 "I(YNYN:{}:{})" },
      { "-D -S",                 "I(YNNY:{}:{})" },
      { "-DOSD",                 "I(YYYY:{}:{})" },
      { "-A apple",              "I(NNNN:{apple}:{})" },
      { "-A apple -A banana",    "I(NNNN:{apple,banana}:{})" },
      { "-A apple banana",       "I(NNNN:{apple,banana}:{})" },
      { "-Q apple",              "I(NNNN:{}:{apple})" },
      { "-Q apple -Q banana",    "I(NNNN:{}:{apple,banana})" },
      { "-Q apple banana",       "I(NNNN:{}:{apple,banana})" },

      // Send
      { "-C",                    "S(YN:{}:{}:{}:{}:-:-:-)" },
      { "-E",                    "S(NY:{}:{}:{}:{}:-:-:-)" },
      { "-EC",                   "S(YY:{}:{}:{}:{}:-:-:-)" },
      { "-a apple",              "S(NN:{apple}:{}:{}:{}:-:-:-)" },
      { "-a apple -a banana",    "S(NN:{apple,banana}:{}:{}:{}:-:-:-)" },
      { "-a apple banana",       "S(NN:{apple,banana}:{}:{}:{}:-:-:-)" },
      { "-b apple",              "S(NN:{}:{apple}:{}:{}:-:-:-)" },
      { "-b apple -b banana",    "S(NN:{}:{apple,banana}:{}:{}:-:-:-)" },
      { "-c apple",              "S(NN:{}:{}:{apple}:{}:-:-:-)" },
      { "-c apple -c banana",    "S(NN:{}:{}:{apple,banana}:{}:-:-:-)" },
      { "apple",                 "S(NN:{}:{}:{}:{apple}:-:-:-)" },
      { "apple banana",          "S(NN:{}:{}:{}:{apple,banana}:-:-:-)" },
      { "apple banana cherry",   "S(NN:{}:{}:{}:{apple,banana,cherry}:-:-:-)" },
      { "-H apple",              "S(NN:{}:{}:{}:{}:apple:-:-)" },
      { "-H apple -H banana",    "S(NN:{}:{}:{}:{}:banana:-:-)" },
      { "-i apple",              "S(NN:{}:{}:{}:{}:-:apple:-)" },
      { "-i apple -i banana",    "S(NN:{}:{}:{}:{}:-:banana:-)" },
      { "-s apple",              "S(NN:{}:{}:{}:{}:-:-:apple)" },
      { "-s apple -s banana",    "S(NN:{}:{}:{}:{}:-:-:banana)" },

      // GUI
      { "-R",                    "G(YNNNNN:{}:-:-:-)" },
      { "-p",                    "G(NYNNNN:{}:-:-:-)" },
      { "-y",                    "G(NNYNNN:{}:-:-:-)" },
      { "-G",                    "G(NNNYNN:{}:-:-:-)" },
      { "-Z",                    "G(NNNNYN:{}:-:-:-)" },
      { "-z",                    "G(NNNNNY:{}:-:-:-)" },
      { "-R -y -G -Z",           "G(YNYYYN:{}:-:-:-)" },
      { "-R -p -G -z",           "G(YYNYNY:{}:-:-:-)" },
      { "-y -p -G -Z",           "G(NYYYYN:{}:-:-:-)" },
      { "-e apple",              "G(NNNNNN:{apple}:-:-:-)" },
      { "-e apple -e banana",    "G(NNNNNN:{apple,banana}:-:-:-)" },
      { "-f apple",              "G(NNNNNN:{}:apple:-:-)" },
      { "-f apple -f banana",    "G(NNNNNN:{}:banana:-:-)" },
      { "-g apple",              "G(NNNYNN:{}:-:apple:-)" },
      { "-g apple -g banana",    "G(NNNYNN:{}:-:banana:-)" },
      { "-m apple",              "G(NNNNNN:{}:-:-:apple)" },
      { "-m apple -m banana",    "G(NNNNNN:{}:-:-:banana)" },

      // Complex tests
      { "apple",                            "S(NN:{}:{}:{}:{apple}:-:-:-)" },
      { "apple --",                         "S(NN:{}:{}:{}:{apple}:-:-:-)" },
      { "apple -- banana",                  "S(NN:{}:{}:{}:{apple,banana}:-:-:-)" },
      { "apple -- banana",                  "S(NN:{}:{}:{}:{apple,banana}:-:-:-)" },
      { "-A apple banana -- cherry",        "I(NNNN:{apple,banana}:{})S(NN:{}:{}:{}:{cherry}:-:-:-)" },
      { "-Q apple banana -- cherry damson", "I(NNNN:{}:{apple,banana})S(NN:{}:{}:{}:{cherry,damson}:-:-:-)" },
      // clang-format on
    };

    struct Buffer *res = buf_pool_get();

    for (size_t i = 0; i < mutt_array_size(Tests); i++)
    {
      struct CommandLine *cli = command_line_new();
      struct StringArray sa = ARRAY_HEAD_INITIALIZER;

      TEST_CASE(Tests[i][0]);
      args_split(Tests[i][0], &sa);

      rc = cli_parse(ARRAY_SIZE(&sa), sa.entries, cli);
      TEST_CHECK(rc == true);

      serialise_cli(cli, res);
      TEST_CHECK_STR_EQ(buf_string(res), Tests[i][1]);

      args_clear(&sa);
      command_line_free(&cli);
    }

    buf_pool_release(&res);
  }

  // Failing tests
  {
    // One bad option and plenty that should take a parameter
    static const char *Tests[] = {
      "-9", "-A", "-a", "-b", "-F", "-f", "-c", "-d",
      "-l", "-e", "-g", "-H", "-i", "-m", "-Q", "-s",
    };

    struct Buffer *res = buf_pool_get();

    opterr = 0; // Disable stderr warnings

    for (size_t i = 0; i < mutt_array_size(Tests); i++)
    {
      struct CommandLine *cli = command_line_new();
      struct StringArray sa = ARRAY_HEAD_INITIALIZER;

      TEST_CASE(Tests[i]);
      args_split(Tests[i], &sa);

      rc = cli_parse(ARRAY_SIZE(&sa), sa.entries, cli);
      TEST_CHECK(rc == false);

      serialise_cli(cli, res);
      TEST_CHECK_STR_EQ(buf_string(res), "H(YNN)");

      args_clear(&sa);
      command_line_free(&cli);
    }

    buf_pool_release(&res);
  }
}
