/********************************************************************\
 * Account.h -- the Account data structure                          *
 * Copyright (C) 1997 Robin D. Clark                                *
 * Copyright (C) 1997, 1998 Linas Vepstas                           *
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
 * along with this program; if not, write to the Free Software      *
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.        *
 *                                                                  *
 *   Author: Rob Clark                                              *
 * Internet: rclark@cs.hmc.edu                                      *
 *  Address: 609 8th Street                                         *
 *           Huntington Beach, CA 92648-4632                        *
\********************************************************************/

#ifndef __ACCOUNT_H__
#define __ACCOUNT_H__

#include "config.h"
#include "AccInfo.h"
#include "Transaction.h"


/** PROTOTYPES ******************************************************/

Account     *xaccMallocAccount( void );
void         xaccInitAccount( Account * );
void         xaccFreeAccount( Account * );

/* 
 * The xaccAccountBeginEdit() and xaccAccountCommitEdit() subroutines
 * provide a pseudo-two-phase-commit wrapper for account updates. 
 * They are mildly useful for detecting attempted updates outside
 * of thier scope. However, they do not provide any true two-phase-anything
 * in the current implementation.
 */
void         xaccAccountBeginEdit (Account *);
void         xaccAccountCommitEdit (Account *);

int          xaccGetAccountID (Account *);

/*
 * The xaccAccountInsertSplit() method will insert the indicated
 *    split into the indicated account.  If the split already 
 *    belongs to another account, it will be removed from that
 *    account first. 
 */
void         xaccAccountInsertSplit (Account *, Split *);

/* The xaccCheckDateOrder() surboutine checks to see if 
 *    a split is in proper sorted date order with respect 
 *    to the other splits in this account.
 *
 * The xaccCheckTransDateOrder() checks to see if 
 *    all of the splits in this transaction are in
 *    proper date order.
 */
int          xaccCheckDateOrder   (Account *, Split *);
int          xaccCheckTransDateOrder (Transaction *);

/* The xaccIsAccountInList() subroutine returns the number of times
 * that an account appears in the account list. */
int          xaccIsAccountInList (Account * acc, Account **list);
void         xaccZeroRunningBalances (Account **list);

/* The xaccConsolidateTransactions() subroutine scans through
 *    all of the transactions in an account, and compares them.
 *    if any of them are exact duplicates, the duplicates are removed.
 *    duplicates may occur when accounts from multiple sources are 
 *    merged.  Note that this can be a dangerous operation to perform.
 */

void        xaccConsolidateTransactions (Account *);

/* The xaccMoveFarEnd() method changes the account to which the 
 *    "far end" of the split belongs.  The "far end" is as follows:
 *    Double-entry transactions by thier nature consist of a set of 
 *    two or more splits. If the transaction has precisely two splits,
 *    then the "far end" is the "other split" of the pair.  If
 *    the transaction consists of three or more splits, then the 
 *    "far end" is undefined.  All that the xaccMoveFareEnd() method
 *    does is reparent the "other split" to the indicated account.
 *    The first argument is the split whose fare end will be changed,
 *    the second argument is the new far-end account.
 */

void xaccMoveFarEnd (Split *, Account *);
void xaccMoveFarEndByName (Split *, const char *);

void xaccAccountSetType (Account *, int);
void xaccAccountSetName (Account *, char *);
void xaccAccountSetDescription (Account *, char *);
void xaccAccountSetNotes (Account *, char *);
void xaccAccountSetCurrency (Account *, char *);

int            xaccAccountGetType (Account *);
char *         xaccAccountGetName (Account *);
char *         xaccAccountGetDescription (Account *);
char *         xaccAccountGetNotes (Account *);
char *         xaccAccountGetCurrency (Account *);
AccountGroup * xaccAccountGetChildren (Account *);
AccountGroup * xaccAccountGetParent (Account *);

double         xaccAccountGetBalance (Account *);
double         xaccAccountGetClearedBalance (Account *);
double         xaccAccountGetReconciledBalance (Account *);
Split *        xaccAccountGetSplit (Account *acc, int i);
Split **       xaccAccountGetSplitList (Account *acc);
int            xaccAccountGetNumSplits (Account *acc);


/** GLOBALS *********************************************************/

extern int next_free_unique_account_id;

#endif
