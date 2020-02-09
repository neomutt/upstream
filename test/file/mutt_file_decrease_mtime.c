/**
 * @file
 * Test code for mutt_file_decrease_mtime()
 *
 * @authors
 * Copyright (C) 2019 Richard Russon <rich@flatcap.org>
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
#include "acutest.h"
#include "config.h"
#include <sys/stat.h>
#include "mutt/lib.h"

void test_mutt_file_decrease_mtime(void)
{
  // time_t mutt_file_decrease_mtime(const char *fp, struct stat *st);

  {
    struct stat stat = { 0 };
    TEST_CHECK(mutt_file_decrease_mtime(NULL, &stat) != 0);
  }

  {
    TEST_CHECK(mutt_file_decrease_mtime("apple", NULL) != 0);
  }
}
