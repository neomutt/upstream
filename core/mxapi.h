/**
 * @file
 * API for mx backends
 *
 * @authors
 * Copyright (C) 1996-2000,2010,2013 Michael R. Elkins <me@mutt.org>
 * Copyright (C) 2018-2019 Richard Russon <rich@flatcap.org>
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

#ifndef MUTT_CORE_MXAPI_H
#define MUTT_CORE_MXAPI_H

#include "config.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/types.h>
#include "mailbox.h"

struct Account;
struct Email;
struct stat;

/**
 * struct Message - A local copy of an email
 */
struct Message
{
  FILE *fp;             ///< pointer to the message data
  char *path;           ///< path to temp file
  char *committed_path; ///< the final path generated by mx_msg_commit()
  bool write;           ///< nonzero if message is open for writing
  struct
  {
    bool read : 1;
    bool flagged : 1;
    bool replied : 1;
    bool draft : 1;
  } flags;
  time_t received; ///< the time at which this message was received
};


/* flags for mutt_open_mailbox() */
typedef uint8_t OpenMailboxFlags;   ///< Flags for mutt_open_mailbox(), e.g. #MUTT_NOSORT
#define MUTT_OPEN_NO_FLAGS       0  ///< No flags are set
#define MUTT_NOSORT        (1 << 0) ///< Do not sort the mailbox after opening it
#define MUTT_APPEND        (1 << 1) ///< Open mailbox for appending messages
#define MUTT_READONLY      (1 << 2) ///< Open in read-only mode
#define MUTT_QUIET         (1 << 3) ///< Do not print any messages
#define MUTT_NEWFOLDER     (1 << 4) ///< Create a new folder - same as #MUTT_APPEND,
                                    ///< but uses mutt_file_fopen() with mode "w" for mbox-style folders.
                                    ///< This will truncate an existing file.
#define MUTT_PEEK          (1 << 5) ///< Revert atime back after taking a look (if applicable)
#define MUTT_APPENDNEW     (1 << 6) ///< Set in mx_open_mailbox_append if the mailbox doesn't exist.
                                    ///< Used by maildir/mh to create the mailbox.
/**
 * enum MxStatus - Return values from mbox_check(), mbox_check_stats(),
 * mbox_snc(), and mbox_close()
 */
enum MxStatus
{
  MX_STATUS_ERROR = -1, ///< An error occurred
  MX_STATUS_OK,         ///< No changes
  MX_STATUS_NEW_MAIL,   ///< New mail received in Mailbox
  MX_STATUS_LOCKED,     ///< Couldn't lock the Mailbox
  MX_STATUS_REOPENED,   ///< Mailbox was reopened
  MX_STATUS_FLAGS,      ///< Nondestructive flags change (IMAP)
};

/**
 * enum MxOpenReturns - Return values for mbox_open()
 */
enum MxOpenReturns
{
  MX_OPEN_OK,           ///< Open succeeded
  MX_OPEN_ERROR,        ///< Open failed with an error
  MX_OPEN_ABORT,        ///< Open was aborted
};

/**
 * @defgroup mx_api Mailbox API
 *
 * The Mailbox API
 *
 * Each backend provides a set of functions through which the Mailbox, messages,
 * tags and paths are manipulated.
 */
struct MxOps
{
  enum MailboxType type;  ///< Mailbox type, e.g. #MUTT_IMAP
  const char *name;       ///< Mailbox name, e.g. "imap"
  bool is_local;          ///< True, if Mailbox type has local files/dirs

  /**
   * @defgroup mx_ac_owns_path ac_owns_path()
   * @ingroup mx_api
   *
   * ac_owns_path - Check whether an Account owns a Mailbox path
   * @param a    Account
   * @param path Mailbox Path
   * @retval true  Account handles path
   * @retval false Account does not handle path
   *
   * **Contract**
   * - @a a    is not NULL
   * - @a path is not NULL
   */
  bool (*ac_owns_path)(struct Account *a, const char *path);

  /**
   * @defgroup mx_ac_add ac_add()
   * @ingroup mx_api
   *
   * ac_add - Add a Mailbox to an Account
   * @param a Account to add to
   * @param m Mailbox to add
   * @retval true  Success
   * @retval false Error
   *
   * **Contract**
   * - @a a is not NULL
   * - @a m is not NULL
   */
  bool (*ac_add)(struct Account *a, struct Mailbox *m);

  /**
   * @defgroup mx_mbox_open mbox_open()
   * @ingroup mx_api
   *
   * mbox_open - Open a Mailbox
   * @param m Mailbox to open
   * @retval enum #MxOpenReturns
   *
   * **Contract**
   * - @a m is not NULL
   */
  enum MxOpenReturns (*mbox_open)(struct Mailbox *m);

  /**
   * @defgroup mx_mbox_open_append mbox_open_append()
   * @ingroup mx_api
   *
   * mbox_open_append - Open a Mailbox for appending
   * @param m     Mailbox to open
   * @param flags Flags, see #OpenMailboxFlags
   * @retval true Success
   * @retval false Failure
   *
   * **Contract**
   * - @a m is not NULL
   */
  bool (*mbox_open_append)(struct Mailbox *m, OpenMailboxFlags flags);

  /**
   * @defgroup mx_mbox_check mbox_check()
   * @ingroup mx_api
   *
   * mbox_check - Check for new mail
   * @param m Mailbox
   * @retval enum #MxStatus
   *
   * **Contract**
   * - @a m is not NULL
   */
  enum MxStatus (*mbox_check)(struct Mailbox *m);

  /**
   * @defgroup mx_mbox_check_stats mbox_check_stats()
   * @ingroup mx_api
   *
   * mbox_check_stats - Check the Mailbox statistics
   * @param m     Mailbox to check
   * @param flags Function flags
   * @retval enum #MxStatus
   *
   * **Contract**
   * - @a m is not NULL
   */
  enum MxStatus (*mbox_check_stats)(struct Mailbox *m, uint8_t flags);

  /**
   * @defgroup mx_mbox_sync mbox_sync()
   * @ingroup mx_api
   *
   * mbox_sync - Save changes to the Mailbox
   * @param m Mailbox to sync
   * @retval enum #MxStatus
   *
   * **Contract**
   * - @a m is not NULL
   */
  enum MxStatus (*mbox_sync)(struct Mailbox *m);

  /**
   * @defgroup mx_mbox_close mbox_close()
   * @ingroup mx_api
   *
   * mbox_close - Close a Mailbox
   * @param m Mailbox to close
   * @retval enum #MxStatus
   *
   * **Contract**
   * - @a m is not NULL
   */
  enum MxStatus (*mbox_close)(struct Mailbox *m);

  /**
   * @defgroup mx_msg_open msg_open()
   * @ingroup mx_api
   *
   * msg_open - Open an email message in a Mailbox
   * @param m     Mailbox
   * @param msg   Message to open
   * @param msgno Index of message to open
   * @retval true Success
   * @retval false Error
   *
   * **Contract**
   * - @a m   is not NULL
   * - @a msg is not NULL
   * - 0 <= @a msgno < msg->msg_count
   */
  bool (*msg_open)(struct Mailbox *m, struct Message *msg, int msgno);

  /**
   * @defgroup mx_msg_open_new msg_open_new()
   * @ingroup mx_api
   *
   * msg_open_new - Open a new message in a Mailbox
   * @param m   Mailbox
   * @param msg Message to open
   * @param e   Email
   * @retval true Success
   * @retval false Failure
   *
   * **Contract**
   * - @a m   is not NULL
   * - @a msg is not NULL
   */
  bool (*msg_open_new)(struct Mailbox *m, struct Message *msg, const struct Email *e);

  /**
   * @defgroup mx_msg_commit msg_commit()
   * @ingroup mx_api
   *
   * msg_commit - Save changes to an email
   * @param m   Mailbox
   * @param msg Message to commit
   * @retval  0 Success
   * @retval -1 Failure
   *
   * **Contract**
   * - @a m   is not NULL
   * - @a msg is not NULL
   */
  int (*msg_commit)      (struct Mailbox *m, struct Message *msg);

  /**
   * @defgroup mx_msg_close msg_close()
   * @ingroup mx_api
   *
   * msg_close - Close an email
   * @param m   Mailbox
   * @param msg Message to close
   * @retval  0 Success
   * @retval -1 Failure
   *
   * **Contract**
   * - @a m   is not NULL
   * - @a msg is not NULL
   */
  int (*msg_close)       (struct Mailbox *m, struct Message *msg);

  /**
   * @defgroup mx_msg_padding_size msg_padding_size()
   * @ingroup mx_api
   *
   * msg_padding_size - Bytes of padding between messages
   * @param m Mailbox
   * @retval num Bytes of padding
   *
   * **Contract**
   * - @a m is not NULL
   */
  int (*msg_padding_size)(struct Mailbox *m);

  /**
   * @defgroup mx_msg_save_hcache msg_save_hcache()
   * @ingroup mx_api
   *
   * msg_save_hcache - Save message to the header cache
   * @param m Mailbox
   * @param e Email
   * @retval  0 Success
   * @retval -1 Failure
   *
   * **Contract**
   * - @a m is not NULL
   * - @a e is not NULL
   */
  int (*msg_save_hcache) (struct Mailbox *m, struct Email *e);

  /**
   * @defgroup mx_tags_edit tags_edit()
   * @ingroup mx_api
   *
   * tags_edit - Prompt and validate new messages tags
   * @param m      Mailbox
   * @param tags   Existing tags
   * @param buf    Buffer to store the tags
   * @param buflen Length of buffer
   * @retval -1 Error
   * @retval  0 No valid user input
   * @retval  1 Buf set
   *
   * **Contract**
   * - @a m   is not NULL
   * - @a buf is not NULL
   */
  int (*tags_edit)       (struct Mailbox *m, const char *tags, char *buf, size_t buflen);

  /**
   * @defgroup mx_tags_commit tags_commit()
   * @ingroup mx_api
   *
   * tags_commit - Save the tags to a message
   * @param m Mailbox
   * @param e Email
   * @param buf Buffer containing tags
   * @retval  0 Success
   * @retval -1 Failure
   *
   * **Contract**
   * - @a m   is not NULL
   * - @a e   is not NULL
   * - @a buf is not NULL
   */
  int (*tags_commit)     (struct Mailbox *m, struct Email *e, char *buf);

  /**
   * @defgroup mx_path_probe path_probe()
   * @ingroup mx_api
   *
   * path_probe - Does this Mailbox type recognise this path?
   * @param path Path to examine
   * @param st   stat buffer (for local filesystems)
   * @retval num Type, e.g. #MUTT_IMAP
   *
   * **Contract**
   * - @a path is not NULL
   */
  enum MailboxType (*path_probe)(const char *path, const struct stat *st);

  /**
   * @defgroup mx_path_canon path_canon()
   * @ingroup mx_api
   *
   * path_canon - Canonicalise a Mailbox path
   * @param buf    Path to modify
   * @param buflen Length of buffer
   * @retval  0 Success
   * @retval -1 Failure
   *
   * **Contract**
   * - @a buf is not NULL
   */
  int (*path_canon)      (char *buf, size_t buflen);

  /**
   * @defgroup mx_path_pretty path_pretty()
   * @ingroup mx_api
   *
   * path_pretty - Abbreviate a Mailbox path
   * @param buf    Path to modify
   * @param buflen Length of buffer
   * @param folder Base path for '=' substitution
   * @retval  0 Success
   * @retval -1 Failure
   *
   * **Contract**
   * - @a buf is not NULL
   */
  int (*path_pretty)     (char *buf, size_t buflen, const char *folder);

  /**
   * @defgroup mx_path_parent path_parent()
   * @ingroup mx_api
   *
   * path_parent - Find the parent of a Mailbox path
   * @param buf    Path to modify
   * @param buflen Length of buffer
   * @retval  0 Success
   * @retval -1 Failure
   *
   * **Contract**
   * - @a buf is not NULL
   */
  int (*path_parent)     (char *buf, size_t buflen);

  /**
   * @defgroup mx_path_is_empty path_is_empty()
   * @ingroup mx_api
   *
   * path_is_empty - Is the Mailbox empty?
   * @param path Mailbox to check
   * @retval 1 Mailbox is empty
   * @retval 0 Mailbox contains mail
   * @retval -1 Error
   *
   * **Contract**
   * - @a path is not NULL and not empty
   */
  int (*path_is_empty)     (const char *path);
};

#endif /* MUTT_CORE_MXAPI_H */

