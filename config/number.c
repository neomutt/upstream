/**
 * @file
 * Type representing a number
 *
 * @authors
 * Copyright (C) 2017-2018 Richard Russon <rich@flatcap.org>
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
 * @page config_number Type: Number
 *
 * Type representing a number.
 */

#include "config.h"
#include <stddef.h>
#include <limits.h>
#include <stdint.h>
#include "mutt/lib.h"
#include "number.h"
#include "set.h"
#include "types.h"

/**
 * number_string_set - Set a Number by string - Implements ::cst_string_set()
 */
static int number_string_set(const struct ConfigSet *cs, void *var, struct ConfigDef *cdef,
                             const char *value, struct Buffer *err)
{
  if (!cs || !cdef)
    return CSR_ERR_CODE; /* LCOV_EXCL_LINE */

  if (!value || !value[0])
  {
    mutt_buffer_printf(err, _("Option %s may not be empty"), cdef->name);
    return CSR_ERR_INVALID | CSR_INV_TYPE;
  }

  int num = 0;
  if (mutt_str_atoi(value, &num) < 0)
  {
    mutt_buffer_printf(err, _("Invalid number: %s"), value);
    return CSR_ERR_INVALID | CSR_INV_TYPE;
  }

  if ((num < SHRT_MIN) || (num > SHRT_MAX))
  {
    mutt_buffer_printf(err, _("Number is too big: %s"), value);
    return CSR_ERR_INVALID | CSR_INV_TYPE;
  }

  if ((num < 0) && (cdef->type & DT_NOT_NEGATIVE))
  {
    mutt_buffer_printf(err, _("Option %s may not be negative"), cdef->name);
    return CSR_ERR_INVALID | CSR_INV_VALIDATOR;
  }

  if (var)
  {
    if (num == (*(short *) var))
      return CSR_SUCCESS | CSR_SUC_NO_CHANGE;

    if (cdef->validator)
    {
      int rc = cdef->validator(cs, cdef, (intptr_t) num, err);

      if (CSR_RESULT(rc) != CSR_SUCCESS)
        return rc | CSR_INV_VALIDATOR;
    }

    *(short *) var = num;
  }
  else
  {
    cdef->initial = num;
  }

  return CSR_SUCCESS;
}

/**
 * number_string_get - Get a Number as a string - Implements ::cst_string_get()
 */
static int number_string_get(const struct ConfigSet *cs, void *var,
                             const struct ConfigDef *cdef, struct Buffer *result)
{
  if (!cs || !cdef)
    return CSR_ERR_CODE; /* LCOV_EXCL_LINE */

  int value;

  if (var)
    value = *(short *) var;
  else
    value = (int) cdef->initial;

  mutt_buffer_printf(result, "%d", value);
  return CSR_SUCCESS;
}

/**
 * number_native_set - Set a Number config item by int - Implements ::cst_native_set()
 */
static int number_native_set(const struct ConfigSet *cs, void *var,
                             const struct ConfigDef *cdef, intptr_t value,
                             struct Buffer *err)
{
  if (!cs || !var || !cdef)
    return CSR_ERR_CODE; /* LCOV_EXCL_LINE */

  if ((value < SHRT_MIN) || (value > SHRT_MAX))
  {
    mutt_buffer_printf(err, _("Invalid number: %ld"), value);
    return CSR_ERR_INVALID | CSR_INV_TYPE;
  }

  if ((value < 0) && (cdef->type & DT_NOT_NEGATIVE))
  {
    mutt_buffer_printf(err, _("Option %s may not be negative"), cdef->name);
    return CSR_ERR_INVALID | CSR_INV_VALIDATOR;
  }

  if (value == (*(short *) var))
    return CSR_SUCCESS | CSR_SUC_NO_CHANGE;

  if (cdef->validator)
  {
    int rc = cdef->validator(cs, cdef, value, err);

    if (CSR_RESULT(rc) != CSR_SUCCESS)
      return rc | CSR_INV_VALIDATOR;
  }

  *(short *) var = value;
  return CSR_SUCCESS;
}

/**
 * number_native_get - Get an int from a Number config item - Implements ::cst_native_get()
 */
static intptr_t number_native_get(const struct ConfigSet *cs, void *var,
                                  const struct ConfigDef *cdef, struct Buffer *err)
{
  if (!cs || !var || !cdef)
    return INT_MIN; /* LCOV_EXCL_LINE */

  return *(short *) var;
}

/**
 * number_reset - Reset a Number to its initial value - Implements ::cst_reset()
 */
static int number_reset(const struct ConfigSet *cs, void *var,
                        const struct ConfigDef *cdef, struct Buffer *err)
{
  if (!cs || !var || !cdef)
    return CSR_ERR_CODE; /* LCOV_EXCL_LINE */

  if (cdef->initial == (*(short *) var))
    return CSR_SUCCESS | CSR_SUC_NO_CHANGE;

  if (cdef->validator)
  {
    int rc = cdef->validator(cs, cdef, cdef->initial, err);

    if (CSR_RESULT(rc) != CSR_SUCCESS)
      return rc | CSR_INV_VALIDATOR;
  }

  *(short *) var = cdef->initial;
  return CSR_SUCCESS;
}

/**
 * number_init - Register the Number config type
 * @param cs Config items
 */
void number_init(struct ConfigSet *cs)
{
  const struct ConfigSetType cst_number = {
    "number",
    number_string_set,
    number_string_get,
    number_native_set,
    number_native_get,
    number_reset,
    NULL,
  };
  cs_register_type(cs, DT_NUMBER, &cst_number);
}
