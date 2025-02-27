/*
 * Copyright © 2004 Red Hat, Inc.
 * Copyright (C) 2012-2021 MATE Developers
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Red Hat not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  Red Hat makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * RED HAT DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL RED HAT
 * BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Author:  Matthias Clasen, Red Hat, Inc.
 */
#ifndef LIST_H
#define LIST_H

typedef struct _List List;
typedef void (*Callback)(void *data, void *user_data);

struct _List {
  void *data;

  List *next;
};

typedef int (*ListFindFunc)(void *data, void *user_data);

void list_foreach(List *list, Callback func, void *user_data);
List *list_prepend(List *list, void *data);
void list_free(List *list);
List *list_find(List *list, ListFindFunc func, void *user_data);
List *list_remove(List *list, const void *data);
int list_length(List *list);

List *list_copy(List *list);

#endif /* LIST_H */
