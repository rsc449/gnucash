/********************************************************************\
 * Period.c -- Implement accounting Periods                         *
 *                                                                  *
 * This program is free software; you can redistribute it and/or    *
 * modify it under the terms of the GNU General Public License as   *
 * published by the Free Software Foundation; either version 2 of   *
 * the License, or (at your option) any later version.              *
 *                                                                  *
 * This program is distributed in the hope that it will be useful,  *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of   *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the    *
 * GNU General Public License for more details.                     *
 *                                                                  *
 * You should have received a copy of the GNU General Public License*
 * along with this program; if not, contact:                        *
 *                                                                  *
 * Free Software Foundation           Voice:  +1-617-542-5942       *
 * 59 Temple Place - Suite 330        Fax:    +1-617-542-2652       *
 * Boston, MA  02111-1307,  USA       gnu@gnu.org                   *
\********************************************************************/
/*
 * FILE:
 * Period.c
 *
 * FUNCTION:
 * Implement accounting periods.
 *
 * CAUTION: This is currently a semi-functional, poorly implementation
 * of the design described in src/doc/book.txt
 *
 * HISTORY:
 * created by Linas Vepstas November 2001
 * Copyright (c) 200-2003 Linas Vepstas <linas@linas.org>
 */

#include "AccountP.h"
#include "gnc-engine-util.h"
#include "gnc-event-p.h"
#include "gnc-lot.h"
#include "gnc-lot-p.h"
#include "Group.h"
#include "GroupP.h"
#include "kvp-util-p.h"
#include "Period.h"
#include "TransactionP.h"
#include "qofbackend-p.h"
#include "qofbook.h"
#include "qofbook-p.h"
#include "qofid-p.h"

/* This static indicates the debugging module that this .o belongs to.  */
static short module = MOD_BOOK;

/* ================================================================ */
/* Reparent transaction to new book.  This routine does this by 
 * deleting the transaction in the old book, and creating a copy
 * in the new book.  While technically correct, this is maybe too 
 * much churn on the backend ... 
 */

void
gnc_book_insert_trans_clobber (QofBook *book, Transaction *trans)
{
   Transaction *newtrans;
   GList *node;

   if (!trans || !book) return;
   
   /* If this is the same book, its a no-op. */
   if (trans->book == book) return;

   ENTER ("trans=%p %s", trans, trans->description);
   newtrans = xaccDupeTransaction (trans);
   for (node = newtrans->splits; node; node = node->next)
   {
      Split *s = node->data;
      s->parent = newtrans;
   }

   /* Utterly wipe out the transaction from the old book. */
   xaccTransBeginEdit (trans);
   xaccTransDestroy (trans);
   xaccTransCommitEdit (trans);

   /* fiddle the transaction into place in the new book */
   qof_entity_store(book->entity_table, newtrans, &newtrans->guid, GNC_ID_TRANS);
   newtrans->book = book;

   xaccTransBeginEdit (newtrans);
   for (node = newtrans->splits; node; node = node->next)
   {
      Account *twin;
      Split *s = node->data;

      /* move the split into the new book ... */
      s->book = book;
      qof_entity_store(book->entity_table, s, &s->guid, GNC_ID_SPLIT);

      /* find the twin account, and re-parent to that. */
      twin = xaccAccountLookupTwin (s->acc, book);
      if (!twin)
      {
         PERR ("near-fatal: twin account not found");
      }
      else
      {
        xaccAccountInsertSplit (twin, s);
        twin->balance_dirty = TRUE;
        twin->sort_dirty = TRUE;
      }
   }

   xaccTransCommitEdit (newtrans);
   gnc_engine_generate_event (&newtrans->guid, GNC_ID_TRANS, GNC_EVENT_CREATE);
   LEAVE ("trans=%p %s", trans, trans->description);
}

/* ================================================================ */
/* Reparent transaction to new book.  This routine does this by 
 * moving GUID's to the new book's entity tables.
 */

void
gnc_book_insert_trans (QofBook *book, Transaction *trans)
{
   GList *node;

   if (!trans || !book) return;
   
   /* If this is the same book, its a no-op. */
   if (trans->book == book) return;

   /* If the old and new book don't share backends, then clobber-copy;
    * i.e. destroy it in one backend, create it in another.  */
   if (book->backend != trans->book->backend)
   {
      gnc_book_insert_trans_clobber (book, trans);
      return;
   }
   ENTER ("trans=%p %s", trans, trans->description);

   /* Fiddle the transaction into place in the new book */
   xaccTransBeginEdit (trans);

   qof_entity_remove (trans->book->entity_table, &trans->guid);
   trans->book = book;
   qof_entity_store(book->entity_table, trans, &trans->guid, GNC_ID_TRANS);

   for (node = trans->splits; node; node = node->next)
   {
      Account *twin;
      Split *s = node->data;

      /* Move the split into the new book ... */
      qof_entity_remove (s->book->entity_table, &s->guid);
      s->book = book;
      qof_entity_store(book->entity_table, s, &s->guid, GNC_ID_SPLIT);

      /* Find the twin account, and re-parent to that. */
      twin = xaccAccountLookupTwin (s->acc, book);
      if (!twin)
      {
         PERR ("near-fatal: twin account not found");
      }
      else
      {
        xaccAccountInsertSplit (twin, s);
        twin->balance_dirty = TRUE;
        twin->sort_dirty = TRUE;
      }
   }

   xaccTransCommitEdit (trans);
   gnc_engine_generate_event (&trans->guid, GNC_ID_TRANS, GNC_EVENT_MODIFY);
   LEAVE ("trans=%p %s", trans, trans->description);
}

/* ================================================================ */
/* Reparent lot to new book.  This routine does this by 
 * completely deleting and recreating the lot.
 */

void
gnc_book_insert_lot_clobber (QofBook *book, GNCLot *lot)
{
   PERR ("Not Implemented");
}

/* ================================================================ */
/* Reparent lot to new book.  This routine does this by 
 * moving GUID's to the new book's entity tables.
 */

void
gnc_book_insert_lot (QofBook *book, GNCLot *lot)
{
   Account *twin;
   if (!lot || !book) return;
   
   /* If this is the same book, its a no-op. */
   if (lot->book == book) return;

   if (book->backend != lot->book->backend)
   {
      gnc_book_insert_lot_clobber (book, lot);
      return;
   }
   ENTER ("lot=%p", lot);
   qof_entity_remove (lot->book->entity_table, &lot->guid);
   lot->book = book;
   qof_entity_store(book->entity_table, lot, &lot->guid, GNC_ID_LOT);

   twin = xaccAccountLookupTwin (lot->account, book);
   if (!twin)
   {
      PERR ("near-fatal: twin account not found");
   }
   else
   {
      xaccAccountInsertLot (twin, lot);
   }
   LEAVE ("lot=%p", lot);
}

/* ================================================================ */
/* Return TRUE if any of the splits in the transaction belong 
 * to an open lot. */

static gboolean
trans_has_open_lot (Transaction *trans)
{
   SplitList *split_list, *node;

   split_list = xaccTransGetSplitList (trans);
   for (node = split_list; node; node=node->next)
   {
      Split *s = node->data;
      GNCLot *lot = xaccSplitGetLot(s);
      if (NULL == lot) continue;
      if (gnc_lot_is_closed(lot)) continue;
      return TRUE;
   }

   return FALSE;
}

/* Remove any transactions that have associated open lots. 
 * These transactions cannot be moved to a closed book.
 */

static TransList *
remove_open_lots_from_trans_list (TransList *trans_list)
{
   GList *node;

   for (node=trans_list; node; )
   {
      Transaction *trans = node->data;
      GList *next = node->next;
      if (trans_has_open_lot (trans))
      {
         trans_list = g_list_remove_link (trans_list, node);
         g_list_free_1 (node);
      }
      node = next;
   }

   return trans_list;
}

/* ================================================================ */
/* Return a unique list of lots that are involved with the listed
 * transactions.
 */

static LotList *
create_lot_list_from_trans_list (TransList *trans_list)
{
   LotList *lot_list = NULL;
   TransList *tnode;

   for (tnode=trans_list; tnode; tnode=tnode->next)
   {
      Transaction *trans = tnode->data;
      SplitList *split_list = xaccTransGetSplitList (trans);
      SplitList *snode;
      for (snode = split_list; snode; snode=snode->next)
      {
         Split *s = snode->data;
         GNCLot *lot = xaccSplitGetLot(s);
         if (NULL == lot) continue;
         if (g_list_find (lot_list, lot)) continue;
         lot_list = g_list_prepend (lot_list, lot);
      }
   }
   return lot_list;
}

/* ================================================================ */

void 
gnc_book_partition (QofBook *dest_book, QofBook *src_book, QofQuery *query)
{
   AccountGroup *src_grp, *dst_grp;
   QofBackend *be;
   time_t now;
   TransList *trans_list, *tnode;
   LotList *lot_list, *lnode;

   if (!src_book || !dest_book || !query) return;
   ENTER (" src_book=%p dest_book=%p", src_book, dest_book);

   be = src_book->backend;
   if (be && be->begin)
   {
      (*be->begin)(be, GNC_ID_PERIOD, dest_book);
   }
   
   /* First, copy the book's KVP tree */
   /* hack alert -- FIXME -- this should really be a merge, not a
    * clobber copy, but I am too lazy to write a kvp merge routine,
    * and it is not needed for the current usage. */
   kvp_frame_delete (dest_book->kvp_data);
   dest_book->kvp_data = kvp_frame_copy (src_book->kvp_data);

   /* Next, copy all of the accounts */
   /* hack alert -- FIXME -- this should really be a merge, not a
    * clobber copy, but I am too lazy to write an account-group merge 
    * routine, and it is not needed for the current usage. */
   src_grp = xaccGetAccountGroup (src_book);
   dst_grp = xaccGetAccountGroup (dest_book);
   xaccAccountGroupBeginEdit (dst_grp);
   xaccAccountGroupBeginEdit (src_grp);
   xaccGroupCopyGroup (dst_grp, src_grp);
   xaccAccountGroupCommitEdit (src_grp);
   xaccAccountGroupCommitEdit (dst_grp);

   /* Next, run the query */
   xaccAccountGroupBeginEdit (dst_grp);
   xaccAccountGroupBeginEdit (src_grp);
   qof_query_set_book (query, src_book);
   trans_list = qof_query_run (query);

   /* Move closed lots over to destination. Do this before 
    * moving transactions, which should avoid damage to lots. */
   trans_list = remove_open_lots_from_trans_list (trans_list);
   lot_list = create_lot_list_from_trans_list (trans_list);
   for (lnode = lot_list; lnode; lnode = lnode->next)
   {
      GNCLot *lot = lnode->data;
      gnc_book_insert_lot (dest_book, lot);
   }

   /* Move the transactions over */
   for (tnode = trans_list; tnode; tnode=tnode->next)
   {
      Transaction *trans = tnode->data;
      gnc_book_insert_trans (dest_book, trans);
   }

   xaccAccountGroupCommitEdit (src_grp);
   xaccAccountGroupCommitEdit (dst_grp);

   /* Make note of the sibling books */
   now = time(0);
   gnc_kvp_gemini (src_book->kvp_data, NULL, &dest_book->guid, now);
   gnc_kvp_gemini (dest_book->kvp_data, NULL, &src_book->guid, now);

   if (be && be->commit)
   {
      (*be->commit)(be, GNC_ID_PERIOD, dest_book);
   }
   LEAVE (" ");
}

/* ================================================================ */
/* Find nearest equity account */

static Account *
find_nearest_equity_acct (Account *acc)
{
   AccountList *acc_list, *node;
   AccountGroup *parent;
   Account *next_up, *candidate;

   /* see if we can find an equity account that is peered to this account */
   parent = xaccAccountGetParent (acc);
   g_return_val_if_fail (parent, NULL);

   acc_list = xaccGroupGetAccountList (parent);
   for (node=acc_list; node; node=node->next)
   {
      candidate = (Account *) node->data;
      if ((EQUITY == xaccAccountGetType (candidate)) &&
          gnc_commodity_equiv(xaccAccountGetCommodity(acc),
                              xaccAccountGetCommodity(candidate)))
      {
         return candidate;
      }
   }

   /* If we got to here, we did not find a peer equity account. 
    * So go up one layer, and look there */
   next_up = xaccGroupGetParentAccount (parent);
   if (next_up) 
   {
      candidate = find_nearest_equity_acct (next_up);
      if (candidate) return candidate;
   }

   /* If we got to here, then we are at the top group, and there is no 
    * equity account to be found.  So we need to create one. */
   
   candidate = xaccMallocAccount (xaccGroupGetBook(parent));
   xaccAccountBeginEdit (candidate);
   xaccGroupInsertAccount (parent, candidate);
   xaccAccountSetType (candidate, EQUITY);
   xaccAccountSetName (candidate, xaccAccountGetTypeStr(EQUITY));
   xaccAccountSetCommodity (candidate, xaccAccountGetCommodity(acc));
   xaccAccountCommitEdit (candidate);
   
   return candidate;
}

/* ================================================================ */
/* Traverse all accounts, get account balances */

static void
add_closing_balances (AccountGroup *closed_grp, 
                      QofBook *open_book,
                      QofBook *closed_book,
                      Account *equity_account,
                      Timespec *post_date, Timespec *date_entered, 
                      const char *desc)
{
   AccountList *acc_list, *node;

   if (!closed_grp) return;
   ENTER (" enter=%s post=%s desc=%s", gnc_print_date(*date_entered),
       gnc_print_date (*post_date), desc);
   xaccAccountBeginEdit (equity_account);

   /* Walk accounts in closed book */
   acc_list = xaccGroupGetAccountList (closed_grp);
   for (node=acc_list; node; node=node->next)
   {
      KvpFrame *cwd;
      KvpValue *vvv;
      Account *twin;
      AccountGroup *childs;
      Account * candidate = (Account *) node->data;
      GNCAccountType tip = xaccAccountGetType (candidate);

      /* find the peer account of this account in the open book  */
      twin = xaccAccountLookupTwin (candidate, open_book);

      /* -------------------------------- */
      /* add KVP to open account, indicating the progenitor
       * of this account. */
      xaccAccountBeginEdit (twin);
      cwd = xaccAccountGetSlots (twin);
      cwd = kvp_frame_get_frame_slash (cwd, "/book/");

      vvv = kvp_value_new_guid (xaccAccountGetGUID (candidate));
      kvp_frame_set_slot_nc (cwd, "prev-acct", vvv);
      
      vvv = kvp_value_new_guid (&closed_book->guid);
      kvp_frame_set_slot_nc (cwd, "prev-book", vvv);

      xaccAccountSetSlots_nc (twin, twin->kvp_data);
      
      /* -------------------------------- */
      /* add KVP to closed account, indicating where 
       * the next book is. */
      xaccAccountBeginEdit (candidate);
      cwd = xaccAccountGetSlots (candidate);
      cwd = kvp_frame_get_frame_slash (cwd, "/book/");

      vvv = kvp_value_new_guid (&open_book->guid);
      kvp_frame_set_slot_nc (cwd, "next-book", vvv);
      
      vvv = kvp_value_new_guid (xaccAccountGetGUID (twin));
      kvp_frame_set_slot_nc (cwd, "next-acct", vvv);

      xaccAccountSetSlots_nc (candidate, candidate->kvp_data);

      /* -------------------------------- */
      /* We need to carry a balance on any account that is not
       * and income or expense or equity account */
      if ((INCOME != tip) && (EXPENSE != tip) && (EQUITY != tip)) 
      {
         Split *se, *st;
         Transaction *trans;
         Account *equity;
         gnc_numeric baln;

         baln = xaccAccountGetBalance (candidate);

         /* Find the equity account into which we'll poke the 
          * balancing transaction */
         if (NULL == equity_account)
         {
            equity = find_nearest_equity_acct (twin);
            xaccAccountBeginEdit (equity);
         }
         else
         {
            equity = equity_account;
         }

         /* -------------------------------- */
         /* Create the balancing transaction */
         trans = xaccMallocTransaction (open_book);
         xaccTransBeginEdit (trans);

         xaccTransSetDatePostedTS (trans, post_date);
         xaccTransSetDateEnteredTS (trans, date_entered);
         xaccTransSetDescription (trans, desc);
         xaccTransSetCurrency (trans, xaccAccountGetCommodity(equity));

         st = xaccMallocSplit(open_book);
         xaccTransAppendSplit(trans, st);
         xaccAccountInsertSplit (twin, st);
         
         se = xaccMallocSplit(open_book);
         xaccTransAppendSplit(trans, se);
         xaccAccountInsertSplit (equity, se);

         xaccSplitSetAmount (st, baln);
         xaccSplitSetValue (st, baln);
         xaccSplitSetAmount (se, gnc_numeric_neg(baln));
         xaccSplitSetValue (se, gnc_numeric_neg(baln));

         /* Add KVP data showing where the balancing 
          * transaction came from */
         cwd = xaccTransGetSlots (trans);
         cwd = kvp_frame_get_frame_slash (cwd, "/book/");

         vvv = kvp_value_new_guid (&closed_book->guid);
         kvp_frame_set_slot_nc (cwd, "closed-book", vvv);
         
         vvv = kvp_value_new_guid (xaccAccountGetGUID(candidate));
         kvp_frame_set_slot_nc (cwd, "closed-acct", vvv);
         
         xaccTransCommitEdit (trans);

         if (NULL == equity_account)
         {
            xaccAccountCommitEdit (equity);
         }
         /* -------------------------------- */
         /* Add KVP to closed account, indicating where the
          * balance was carried forward to. */
         cwd = xaccAccountGetSlots (candidate);
         cwd = kvp_frame_get_frame_slash (cwd, "/book/");

         vvv = kvp_value_new_guid (xaccTransGetGUID(trans));
         kvp_frame_set_slot_nc (cwd, "balancing-trans", vvv);
      }

      /* We left an open dangling above ... */
      xaccAccountCommitEdit (candidate);
      xaccAccountCommitEdit (twin);

      /* Recurse down to the children */
      childs = xaccAccountGetChildren(candidate);
      if (childs) 
      {
         PINFO ("add closing baln to subaccts of %s", 
                 candidate->description);
         add_closing_balances (childs, open_book, closed_book,
                          equity_account,
                          post_date, date_entered, desc);
      }
   }
   xaccAccountCommitEdit (equity_account);
   LEAVE (" ");
}

/* ================================================================ */
/* Split a book into two by date */

QofBook * 
gnc_book_close_period (QofBook *existing_book, Timespec calve_date,
                       Account *equity_account,
                       const char * memo)
{
   QofQuery *query;
   QofQueryPredData *pred_data;
   GSList *param_list;
   QofBook *closing_book;
   KvpFrame *exist_cwd, *partn_cwd;
   KvpValue *vvv;
   Timespec ts;

   if (!existing_book) return NULL;
   ENTER (" date=%s memo=%s", gnc_print_date(calve_date), memo);

   /* Get all transactions that are *earlier* than the calve date,
    * and put them in the new book.  */
   query = qof_query_create_for (GNC_ID_TRANS);
   pred_data = qof_query_date_predicate (QOF_COMPARE_LTE,
                                         QOF_DATE_MATCH_NORMAL,
                                         calve_date);
   param_list = qof_query_build_param_list (TRANS_DATE_POSTED, NULL);
   qof_query_add_term (query, param_list, pred_data, QOF_QUERY_FIRST_TERM);

   closing_book = qof_book_new();
   qof_book_set_backend (closing_book, existing_book->backend);
   closing_book->book_open = 'n';
   gnc_book_partition (closing_book, existing_book, query);

   qof_query_destroy (query);

   /* Now add the various identifying kvp's */
   /* cwd == 'current working directory' */
   exist_cwd = kvp_frame_get_frame_slash (existing_book->kvp_data, "/book/");
   partn_cwd = kvp_frame_get_frame_slash (closing_book->kvp_data, "/book/");
   
   /* Mark the boundary date between the books */
   vvv = kvp_value_new_timespec (calve_date);
   kvp_frame_set_slot_nc (exist_cwd, "open-date", vvv);
   kvp_frame_set_slot_nc (partn_cwd, "close-date", vvv);

   /* Mark partition as being closed */
   ts.tv_sec = time(0);
   ts.tv_nsec = 0;
   vvv = kvp_value_new_timespec (ts);
   kvp_frame_set_slot_nc (partn_cwd, "log-date", vvv);

   /* Set up pointers to each book from the other. */
   vvv = kvp_value_new_guid (&existing_book->guid);
   kvp_frame_set_slot_nc (partn_cwd, "next-book", vvv);

   vvv = kvp_value_new_guid (&closing_book->guid);
   kvp_frame_set_slot_nc (exist_cwd, "prev-book", vvv);

   /* add in transactions to equity accounts that will
    * hold the colsing balances */
   add_closing_balances (xaccGetAccountGroup(closing_book), 
                        existing_book, closing_book,
                        equity_account,
                        &calve_date, &ts, memo);
   LEAVE (" ");
   return closing_book;
}

/* ============================= END OF FILE ====================== */
