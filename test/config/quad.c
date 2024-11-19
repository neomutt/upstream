/**
 * @file
 * Test code for the Quad object
 *
 * @authors
 * Copyright (C) 2018-2023 Richard Russon <rich@flatcap.org>
 * Copyright (C) 2023 наб <nabijaczleweli@nabijaczleweli.xyz>
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
#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include "mutt/lib.h"
#include "config/lib.h"
#include "core/lib.h"
#include "common.h" // IWYU pragma: keep
#include "test_common.h"

// clang-format off
static struct ConfigDef Vars[] = {
  { "Apple",      DT_QUAD, 0, 0, NULL,              }, /* test_initial_values */
  { "Banana",     DT_QUAD, 3, 0, NULL,              },
  { "Cherry",     DT_QUAD, 0, 0, NULL,              },
  { "Damson",     DT_QUAD, 0, 0, NULL,              }, /* test_string_set */
  { "Elderberry", DT_QUAD, 0, 0, NULL,              }, /* test_string_get */
  { "Fig",        DT_QUAD, 0, 0, NULL,              }, /* test_native_set */
  { "Guava",      DT_QUAD, 0, 0, NULL,              }, /* test_native_get */
  { "Hawthorn",   DT_QUAD, 0, 0, NULL,              }, /* test_reset */
  { "Ilama",      DT_QUAD, 0, 0, validator_fail,    },
  { "Jackfruit",  DT_QUAD, 0, 0, validator_succeed, }, /* test_validator */
  { "Kumquat",    DT_QUAD, 0, 0, validator_warn,    },
  { "Lemon",      DT_QUAD, 0, 0, validator_fail,    },
  { "Mango",      DT_QUAD, 0, 0, NULL,              }, /* test_inherit */
  { "Nectarine",  DT_QUAD, 0, 0, NULL,              }, /* test_toggle */
  { "Olive",      DT_BOOL, 0, 0, NULL,              },
  { "Papaya",     DT_QUAD | D_ON_STARTUP, 3, 0, NULL, }, /* startup */
  { NULL },
};
// clang-format on

static bool test_initial_values(struct ConfigSubset *sub, struct Buffer *err)
{
  log_line(__func__);

  struct ConfigSet *cs = sub->cs;

  char VarApple = cs_subset_quad(sub, "Apple");
  char VarBanana = cs_subset_quad(sub, "Banana");

  TEST_MSG("Apple = %d", VarApple);
  TEST_MSG("Banana = %d", VarBanana);

  TEST_CHECK_NUM_EQ(VarApple, 0);

  TEST_CHECK_NUM_EQ(VarBanana, 3);

  cs_str_string_set(cs, "Apple", "ask-yes", err);
  cs_str_string_set(cs, "Banana", "ask-no", err);

  struct Buffer *value = buf_pool_get();

  int rc;

  buf_reset(value);
  rc = cs_str_initial_get(cs, "Apple", value);
  if (!TEST_CHECK_NUM_EQ(CSR_RESULT(rc), CSR_SUCCESS))
  {
    TEST_MSG("%s", buf_string(value));
    return false;
  }

  if (!TEST_CHECK_STR_EQ(buf_string(value), "no"))
  {
    TEST_MSG("Apple's initial value is wrong: '%s'", buf_string(value));
    return false;
  }

  VarApple = cs_subset_quad(sub, "Apple");
  TEST_MSG("Apple = %d", VarApple);
  TEST_MSG("Apple's initial value is '%s'", buf_string(value));

  buf_reset(value);
  rc = cs_str_initial_get(cs, "Banana", value);
  if (CSR_RESULT(rc) != CSR_SUCCESS)
  {
    TEST_MSG("%s", buf_string(value));
    return false;
  }

  if (!TEST_CHECK_STR_EQ(buf_string(value), "ask-yes"))
  {
    TEST_MSG("Banana's initial value is wrong: '%s'", buf_string(value));
    return false;
  }

  VarBanana = cs_subset_quad(sub, "Banana");
  TEST_MSG("Banana = %d", VarBanana);
  TEST_MSG("Banana's initial value is '%s'", NONULL(buf_string(value)));

  buf_reset(value);
  rc = cs_str_initial_set(cs, "Cherry", "ask-yes", value);
  if (!TEST_CHECK_NUM_EQ(CSR_RESULT(rc), CSR_SUCCESS))
  {
    TEST_MSG("%s", buf_string(value));
    return false;
  }

  buf_reset(value);
  rc = cs_str_initial_get(cs, "Cherry", value);
  if (!TEST_CHECK_NUM_EQ(CSR_RESULT(rc), CSR_SUCCESS))
  {
    TEST_MSG("%s", buf_string(value));
    return false;
  }

  char VarCherry = cs_subset_quad(sub, "Cherry");
  TEST_MSG("Cherry = '%s'", VarCherry ? "yes" : "no");
  TEST_MSG("Cherry's initial value is '%s'", NONULL(buf_string(value)));

  buf_pool_release(&value);
  log_line(__func__);
  return true;
}

static bool test_string_set(struct ConfigSubset *sub, struct Buffer *err)
{
  log_line(__func__);

  const char *valid[] = { "no", "yes", "ask-no", "ask-yes" };
  const char *invalid[] = {
    "nope",
    "ye",
    "",
    NULL,
  };

  struct ConfigSet *cs = sub->cs;
  const char *name = "Damson";
  char VarDamson;

  int rc;
  for (unsigned int i = 0; i < mutt_array_size(valid); i++)
  {
    cs_str_native_set(cs, name, ((i + 1) % 4), NULL);

    TEST_MSG("Setting %s to %s", name, valid[i]);
    buf_reset(err);
    rc = cs_str_string_set(cs, name, valid[i], err);
    if (!TEST_CHECK_NUM_EQ(CSR_RESULT(rc), CSR_SUCCESS))
    {
      TEST_MSG("%s", buf_string(err));
      return false;
    }

    VarDamson = cs_subset_quad(sub, "Damson");
    if (VarDamson == ((i + 1) % 4))
    {
      TEST_MSG("Value of %s wasn't changed", name);
      return false;
    }
    TEST_MSG("%s = %d, set by '%s'", name, VarDamson, valid[i]);

    if (i == 2)
    {
      buf_reset(err);
      rc = cs_str_string_set(cs, name, valid[i], err);
      if (!TEST_CHECK_NUM_EQ(CSR_RESULT(rc), CSR_SUCCESS))
      {
        TEST_MSG("%s", buf_string(err));
        return false;
      }

      if (rc & CSR_SUC_NO_CHANGE)
      {
        TEST_MSG("Value of %s wasn't changed", name);
        continue;
      }
    }
    short_line();
  }

  for (unsigned int i = 0; i < mutt_array_size(invalid); i++)
  {
    buf_reset(err);
    rc = cs_str_string_set(cs, name, invalid[i], err);
    if (TEST_CHECK(CSR_RESULT(rc) != CSR_SUCCESS))
    {
      TEST_MSG("Expected error: %s", buf_string(err));
    }
    else
    {
      VarDamson = cs_subset_quad(sub, "Damson");
      TEST_MSG("%s = %d, set by '%s'", name, VarDamson, invalid[i]);
      TEST_MSG("This test should have failed");
      return false;
    }
    short_line();
  }

  name = "Papaya";
  rc = cs_str_string_set(cs, name, "ask-yes", err);
  TEST_CHECK_NUM_EQ(CSR_RESULT(rc), CSR_SUCCESS);

  rc = cs_str_string_set(cs, name, "ask-no", err);
  TEST_CHECK(CSR_RESULT(rc) != CSR_SUCCESS);

  log_line(__func__);
  return true;
}

static bool test_string_get(struct ConfigSubset *sub, struct Buffer *err)
{
  log_line(__func__);

  struct ConfigSet *cs = sub->cs;
  const char *name = "Elderberry";

  int valid[] = { MUTT_NO, MUTT_YES, MUTT_ASKNO, MUTT_ASKYES };

  int rc;
  for (unsigned int i = 0; i < mutt_array_size(valid); i++)
  {
    cs_str_native_set(cs, name, valid[i], NULL);
    buf_reset(err);
    rc = cs_str_string_get(cs, name, err);
    if (CSR_RESULT(rc) != CSR_SUCCESS)
    {
      TEST_MSG("Get failed: %s", buf_string(err));
      return false;
    }
    char VarElderberry = cs_subset_quad(sub, "Elderberry");
    TEST_MSG("%s = %d, %s", name, VarElderberry, buf_string(err));
  }

  // cs_str_native_set(cs, name, 4, NULL);
  // buf_reset(err);
  // TEST_MSG("Expect error for next test");
  // rc = cs_str_string_get(cs, name, err);
  // if (!TEST_CHECK(CSR_RESULT(rc) != CSR_SUCCESS))
  // {
  //   TEST_MSG("%s", buf_string(err));
  //   return false;
  // }

  log_line(__func__);
  return true;
}

static bool test_native_set(struct ConfigSubset *sub, struct Buffer *err)
{
  log_line(__func__);

  struct ConfigSet *cs = sub->cs;
  const char *name = "Fig";
  char value = MUTT_YES;

  TEST_MSG("Setting %s to %d", name, value);
  cs_str_native_set(cs, name, MUTT_NO, NULL);
  buf_reset(err);
  int rc = cs_str_native_set(cs, name, value, err);
  if (!TEST_CHECK_NUM_EQ(CSR_RESULT(rc), CSR_SUCCESS))
  {
    TEST_MSG("%s", buf_string(err));
    return false;
  }

  char VarFig = cs_subset_quad(sub, "Fig");
  if (!TEST_CHECK(VarFig == value))
  {
    TEST_MSG("Value of %s wasn't changed", name);
    return false;
  }

  TEST_MSG("%s = %d, set to '%d'", name, VarFig, value);

  short_line();
  buf_reset(err);
  TEST_MSG("Setting %s to %d", name, value);
  rc = cs_str_native_set(cs, name, value, err);
  if (!TEST_CHECK((rc & CSR_SUC_NO_CHANGE) != 0))
  {
    TEST_MSG("%s", buf_string(err));
    return false;
  }

  if (rc & CSR_SUC_NO_CHANGE)
  {
    TEST_MSG("Value of %s wasn't changed", name);
  }

  int invalid[] = { -1, 4 };
  for (unsigned int i = 0; i < mutt_array_size(invalid); i++)
  {
    short_line();
    cs_str_native_set(cs, name, MUTT_NO, NULL);
    TEST_MSG("Setting %s to %d", name, invalid[i]);
    buf_reset(err);
    rc = cs_str_native_set(cs, name, invalid[i], err);
    if (TEST_CHECK(CSR_RESULT(rc) != CSR_SUCCESS))
    {
      TEST_MSG("Expected error: %s", buf_string(err));
    }
    else
    {
      VarFig = cs_subset_quad(sub, "Fig");
      TEST_MSG("%s = %d, set by '%d'", name, VarFig, invalid[i]);
      TEST_MSG("This test should have failed");
      return false;
    }
  }

  name = "Papaya";
  rc = cs_str_native_set(cs, name, MUTT_ASKYES, err);
  TEST_CHECK_NUM_EQ(CSR_RESULT(rc), CSR_SUCCESS);

  rc = cs_str_native_set(cs, name, MUTT_ASKNO, err);
  TEST_CHECK(CSR_RESULT(rc) != CSR_SUCCESS);

  log_line(__func__);
  return true;
}

static bool test_native_get(struct ConfigSubset *sub, struct Buffer *err)
{
  log_line(__func__);

  struct ConfigSet *cs = sub->cs;
  const char *name = "Guava";

  cs_str_native_set(cs, name, MUTT_YES, NULL);
  buf_reset(err);
  intptr_t value = cs_str_native_get(cs, name, err);
  if (!TEST_CHECK(value != INT_MIN))
  {
    TEST_MSG("Get failed: %s", buf_string(err));
    return false;
  }
  TEST_MSG("%s = %ld", name, value);

  log_line(__func__);
  return true;
}

static bool test_reset(struct ConfigSubset *sub, struct Buffer *err)
{
  log_line(__func__);

  struct ConfigSet *cs = sub->cs;
  const char *name = "Hawthorn";
  cs_str_native_set(cs, name, MUTT_YES, NULL);
  buf_reset(err);

  char VarHawthorn = cs_subset_quad(sub, "Hawthorn");
  TEST_MSG("%s = %d", name, VarHawthorn);
  int rc = cs_str_reset(cs, name, err);
  if (!TEST_CHECK_NUM_EQ(CSR_RESULT(rc), CSR_SUCCESS))
  {
    TEST_MSG("%s", buf_string(err));
    return false;
  }

  VarHawthorn = cs_subset_quad(sub, "Hawthorn");
  if (!TEST_CHECK(VarHawthorn != true))
  {
    TEST_MSG("Value of %s wasn't changed", name);
    return false;
  }

  TEST_MSG("Reset: %s = %d", name, VarHawthorn);

  short_line();
  name = "Ilama";
  buf_reset(err);

  char VarIlama = cs_subset_quad(sub, "Ilama");
  TEST_MSG("Initial: %s = %d", name, VarIlama);
  dont_fail = true;
  rc = cs_str_string_set(cs, name, "ask-yes", err);
  if (!TEST_CHECK_NUM_EQ(CSR_RESULT(rc), CSR_SUCCESS))
    return false;
  VarIlama = cs_subset_quad(sub, "Ilama");
  TEST_MSG("Set: %s = %d", name, VarIlama);
  dont_fail = false;

  rc = cs_str_reset(cs, name, err);
  if (TEST_CHECK(CSR_RESULT(rc) != CSR_SUCCESS))
  {
    TEST_MSG("Expected error: %s", buf_string(err));
  }
  else
  {
    TEST_MSG("%s", buf_string(err));
    return false;
  }

  VarIlama = cs_subset_quad(sub, "Ilama");
  if (!TEST_CHECK_NUM_EQ(VarIlama, MUTT_ASKYES))
  {
    TEST_MSG("Value of %s changed", name);
    return false;
  }

  TEST_MSG("Reset: %s = %d", name, VarIlama);

  name = "Papaya";
  rc = cs_str_reset(cs, name, err);
  TEST_CHECK_NUM_EQ(CSR_RESULT(rc), CSR_SUCCESS);

  StartupComplete = false;
  rc = cs_str_native_set(cs, name, MUTT_NO, err);
  TEST_CHECK_NUM_EQ(CSR_RESULT(rc), CSR_SUCCESS);
  StartupComplete = true;

  rc = cs_str_reset(cs, name, err);
  TEST_CHECK(CSR_RESULT(rc) != CSR_SUCCESS);

  log_line(__func__);
  return true;
}

static bool test_validator(struct ConfigSubset *sub, struct Buffer *err)
{
  log_line(__func__);

  struct ConfigSet *cs = sub->cs;
  const char *name = "Jackfruit";
  cs_str_native_set(cs, name, MUTT_NO, NULL);
  buf_reset(err);
  int rc = cs_str_string_set(cs, name, "yes", err);
  if (TEST_CHECK_NUM_EQ(CSR_RESULT(rc), CSR_SUCCESS))
  {
    TEST_MSG("%s", buf_string(err));
  }
  else
  {
    TEST_MSG("%s", buf_string(err));
    return false;
  }
  char VarJackfruit = cs_subset_quad(sub, "Jackfruit");
  TEST_MSG("String: %s = %d", name, VarJackfruit);
  short_line();

  cs_str_native_set(cs, name, MUTT_NO, NULL);
  buf_reset(err);
  rc = cs_str_native_set(cs, name, 1, err);
  if (TEST_CHECK_NUM_EQ(CSR_RESULT(rc), CSR_SUCCESS))
  {
    TEST_MSG("%s", buf_string(err));
  }
  else
  {
    TEST_MSG("%s", buf_string(err));
    return false;
  }
  VarJackfruit = cs_subset_quad(sub, "Jackfruit");
  TEST_MSG("Native: %s = %d", name, VarJackfruit);
  short_line();

  name = "Kumquat";
  cs_str_native_set(cs, name, MUTT_NO, NULL);
  buf_reset(err);
  rc = cs_str_string_set(cs, name, "yes", err);
  if (TEST_CHECK_NUM_EQ(CSR_RESULT(rc), CSR_SUCCESS))
  {
    TEST_MSG("%s", buf_string(err));
  }
  else
  {
    TEST_MSG("%s", buf_string(err));
    return false;
  }
  char VarKumquat = cs_subset_quad(sub, "Kumquat");
  TEST_MSG("String: %s = %d", name, VarKumquat);
  short_line();

  cs_str_native_set(cs, name, MUTT_NO, NULL);
  buf_reset(err);
  rc = cs_str_native_set(cs, name, 1, err);
  if (TEST_CHECK_NUM_EQ(CSR_RESULT(rc), CSR_SUCCESS))
  {
    TEST_MSG("%s", buf_string(err));
  }
  else
  {
    TEST_MSG("%s", buf_string(err));
    return false;
  }
  VarKumquat = cs_subset_quad(sub, "Kumquat");
  TEST_MSG("Native: %s = %d", name, VarKumquat);
  short_line();

  name = "Lemon";
  cs_str_native_set(cs, name, MUTT_NO, NULL);
  buf_reset(err);
  rc = cs_str_string_set(cs, name, "yes", err);
  if (TEST_CHECK(CSR_RESULT(rc) != CSR_SUCCESS))
  {
    TEST_MSG("Expected error: %s", buf_string(err));
  }
  else
  {
    TEST_MSG("%s", buf_string(err));
    return false;
  }
  char VarLemon = cs_subset_quad(sub, "Lemon");
  TEST_MSG("String: %s = %d", name, VarLemon);
  short_line();

  cs_str_native_set(cs, name, MUTT_NO, NULL);
  buf_reset(err);
  rc = cs_str_native_set(cs, name, 1, err);
  if (TEST_CHECK(CSR_RESULT(rc) != CSR_SUCCESS))
  {
    TEST_MSG("Expected error: %s", buf_string(err));
  }
  else
  {
    TEST_MSG("%s", buf_string(err));
    return false;
  }
  VarLemon = cs_subset_quad(sub, "Lemon");
  TEST_MSG("Native: %s = %d", name, VarLemon);

  log_line(__func__);
  return true;
}

static void dump_native(struct ConfigSet *cs, const char *parent, const char *child)
{
  intptr_t pval = cs_str_native_get(cs, parent, NULL);
  intptr_t cval = cs_str_native_get(cs, child, NULL);

  TEST_MSG("%15s = %ld", parent, pval);
  TEST_MSG("%15s = %ld", child, cval);
}

static bool test_inherit(struct ConfigSet *cs, struct Buffer *err)
{
  log_line(__func__);
  bool result = false;

  const char *account = "fruit";
  const char *parent = "Mango";
  char child[128];
  snprintf(child, sizeof(child), "%s:%s", account, parent);

  struct ConfigSubset *sub = cs_subset_new(NULL, NULL, NeoMutt->notify);
  sub->cs = cs;
  struct Account *a = account_new(account, sub);

  struct HashElem *he = cs_subset_create_inheritance(a->sub, parent);
  if (!he)
  {
    TEST_MSG("Error: %s", buf_string(err));
    goto ti_out;
  }

  // set parent
  buf_reset(err);
  int rc = cs_str_string_set(cs, parent, "yes", err);
  if (!TEST_CHECK_NUM_EQ(CSR_RESULT(rc), CSR_SUCCESS))
  {
    TEST_MSG("Error: %s", buf_string(err));
    goto ti_out;
  }
  dump_native(cs, parent, child);
  short_line();

  // set child
  buf_reset(err);
  rc = cs_str_string_set(cs, child, "no", err);
  if (!TEST_CHECK_NUM_EQ(CSR_RESULT(rc), CSR_SUCCESS))
  {
    TEST_MSG("Error: %s", buf_string(err));
    goto ti_out;
  }
  if (rc & CSR_SUC_NO_CHANGE)
  {
    TEST_MSG("Value of %s wasn't changed", parent);
  }
  dump_native(cs, parent, child);
  short_line();

  // reset child
  buf_reset(err);
  rc = cs_str_reset(cs, child, err);
  if (!TEST_CHECK_NUM_EQ(CSR_RESULT(rc), CSR_SUCCESS))
  {
    TEST_MSG("Error: %s", buf_string(err));
    goto ti_out;
  }
  dump_native(cs, parent, child);
  short_line();

  // reset parent
  buf_reset(err);
  rc = cs_str_reset(cs, parent, err);
  if (!TEST_CHECK_NUM_EQ(CSR_RESULT(rc), CSR_SUCCESS))
  {
    TEST_MSG("Error: %s", buf_string(err));
    goto ti_out;
  }
  dump_native(cs, parent, child);

  log_line(__func__);
  result = true;
ti_out:
  account_free(&a);
  cs_subset_free(&sub);
  return result;
}

static bool test_toggle(struct ConfigSubset *sub, struct Buffer *err)
{
  log_line(__func__);

  struct ToggleTest
  {
    char before;
    char after;
  };

  struct ToggleTest tests[] = {
    { MUTT_NO, MUTT_YES },
    { MUTT_YES, MUTT_NO },
    { MUTT_ASKNO, MUTT_ASKYES },
    { MUTT_ASKYES, MUTT_ASKNO },
  };

  struct ConfigSet *cs = sub->cs;
  const char *name = "Nectarine";
  int rc;

  struct HashElem *he = cs_get_elem(cs, name);
  if (!he)
    return false;

  rc = quad_he_toggle(NULL, he, err);
  if (!TEST_CHECK_NUM_EQ(CSR_RESULT(rc), CSR_ERR_CODE))
  {
    TEST_MSG("Toggle succeeded when is shouldn't have");
    return false;
  }

  rc = quad_he_toggle(NeoMutt->sub, NULL, err);
  if (!TEST_CHECK_NUM_EQ(CSR_RESULT(rc), CSR_ERR_CODE))
  {
    TEST_MSG("Toggle succeeded when is shouldn't have");
    return false;
  }

  for (size_t i = 0; i < mutt_array_size(tests); i++)
  {
    char before = tests[i].before;
    char after = tests[i].after;
    TEST_MSG("test %zu", i);

    cs_str_native_set(cs, name, before, NULL);
    buf_reset(err);
    intptr_t value = cs_he_native_get(cs, he, err);
    if (!TEST_CHECK(value != INT_MIN))
    {
      TEST_MSG("Get failed: %s", buf_string(err));
      return false;
    }

    char copy = value;
    if (!TEST_CHECK(copy == before))
    {
      TEST_MSG("Initial value is wrong: %s", buf_string(err));
      return false;
    }

    rc = quad_he_toggle(NeoMutt->sub, he, err);
    if (!TEST_CHECK_NUM_EQ(CSR_RESULT(rc), CSR_SUCCESS))
    {
      TEST_MSG("Toggle failed: %s", buf_string(err));
      return false;
    }

    char VarNectarine = cs_subset_quad(sub, "Nectarine");
    if (!TEST_CHECK(VarNectarine == after))
    {
      TEST_MSG("Toggle value is wrong: %s", buf_string(err));
      return false;
    }
  }

  buf_reset(err);
  struct ConfigSubset sub2 = { 0 };
  rc = quad_he_toggle(&sub2, he, err);
  if (!TEST_CHECK(CSR_RESULT(rc) != CSR_SUCCESS))
  {
    TEST_MSG("Expected error: %s", buf_string(err));
  }

  name = "Olive";
  he = cs_get_elem(cs, name);
  if (!he)
    return false;

  buf_reset(err);
  rc = quad_he_toggle(NeoMutt->sub, he, err);
  if (!TEST_CHECK(CSR_RESULT(rc) != CSR_SUCCESS))
  {
    TEST_MSG("Expected error: %s", buf_string(err));
  }

  log_line(__func__);
  return true;
}

void test_config_quad(void)
{
  struct ConfigSubset *sub = NeoMutt->sub;
  struct ConfigSet *cs = sub->cs;

  StartupComplete = false;
  dont_fail = true;
  if (!TEST_CHECK(cs_register_variables(cs, Vars)))
    return;
  dont_fail = false;
  StartupComplete = true;

  notify_observer_add(NeoMutt->notify, NT_CONFIG, log_observer, 0);

  set_list(cs);

  struct Buffer *err = buf_pool_get();
  TEST_CHECK(test_initial_values(sub, err));
  TEST_CHECK(test_string_set(sub, err));
  TEST_CHECK(test_string_get(sub, err));
  TEST_CHECK(test_native_set(sub, err));
  TEST_CHECK(test_native_get(sub, err));
  TEST_CHECK(test_reset(sub, err));
  TEST_CHECK(test_validator(sub, err));
  TEST_CHECK(test_inherit(cs, err));
  TEST_CHECK(test_toggle(sub, err));
  buf_pool_release(&err);
}
