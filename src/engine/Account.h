/********************************************************************\
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
 *                                                                  *
\********************************************************************/
/** @addtogroup Engine
    @{ */
/** @file Account.h 
    @brief Account handling public routines   
    @author Copyright (C) 1997 Robin D. Clark
    @author Copyright (C) 1997-2003 Linas Vepstas <linas@linas.org>
*/

#ifndef XACC_ACCOUNT_H
#define XACC_ACCOUNT_H

#include "gnc-engine.h"
#include "guid.h"
#include "kvp_frame.h"
#include "qofbook.h"


typedef gnc_numeric (*xaccGetBalanceFn)( Account *account );
typedef gnc_numeric (*xaccGetBalanceInCurrencyFn) (Account *account,
						   gnc_commodity *report_commodity,
						   gboolean include_children);

/** The account types are used to determine how the transaction data
 * in the account is displayed.   These values can be safely changed
 * from one release to the next.  Note that if values are added,
 * the file IO translation routines need to be updated. Note 
 * also that GUI code depends on these numbers.
 *
 * @note ***IMPORTANT***: If you do change the enumeration names (not the
 * numbers), you need to update xaccAccountTypeEnumAsString --- used
 * for text file exports */

typedef enum 
{
  BAD_TYPE = -1, /**< Not a type */
  NO_TYPE = -1,/**< Not a type */
  
  BANK = 0, /**< The bank account type denotes a savings or checking account
	     * held at a bank.  Often interest bearing. */
  CASH = 1,/**< The cash account type is used to denote a shoe-box or pillowcase
	    * stuffed with cash. */
  CREDIT = 3, /**< The Credit card account is used to denote credit (e.g. amex)
	       * and debit (e.g. visa, mastercard) card accounts */
  ASSET = 2,  /**< asset (and liability) accounts indicate generic, generalized accounts
	       * that are none of the above. */
  LIABILITY = 4, /**< liability (and asset) accounts indicate generic, generalized accounts
		  * that are none of the above. */
  STOCK = 5,  /**< Stock accounts will typically be shown in registers
	       * which show three columns: price, number of shares, and value. */
  MUTUAL= 6, /**< Mutual Fund accounts will typically be shown in registers
	      * which show three columns: price, number of shares, and value. */
  CURRENCY = 7, /**< The currency account type indicates that the
		 * account is a currency trading account.  In many
		 * ways, a currency trading account is like a stock
		 * trading account. It is shown in the register with
		 * three columns: price, number of shares, and
		 * value. Note: Since version 1.7.0, this account is
		 * no longer needed to exchange currencies between
		 * accounts, so this type will probably become
		 * deprecated sometime in the future.  */
  INCOME = 8, /**< Income accounts are used to denote income */
  
  EXPENSE = 9,/**< Expense accounts are used to denote expenses. */
  
  EQUITY = 10,/**< Equity account is used to balance the balance sheet. */
  
  RECEIVABLE = 11,  /**< A/R account type */

  PAYABLE = 12,  /**< A/P account type */

  NUM_ACCOUNT_TYPES = 13,  /**< stop here; the following types
			    * just aren't ready for prime time */
  
  /* bank account types */
  CHECKING = 13, /**< bank account type -- don't use this for now, see NUM_ACCOUNT_TYPES  */
  SAVINGS = 14, /**< bank account type -- don't use this for now, see NUM_ACCOUNT_TYPES  */
  MONEYMRKT = 15, /**< bank account type -- don't use this for now, see NUM_ACCOUNT_TYPES  */
  CREDITLINE = 16,     /**< line of credit -- don't use this for now, see NUM_ACCOUNT_TYPES  */
} GNCAccountType;



/** @name Account Constructors, Edit/Commit, Comparison */
/** @{ */

/** Constructor */
Account    * xaccMallocAccount (QofBook *book);

/** The xaccCloneAccount() does the same as xaccCloneAccountSimple, except that it
 *    also uses the 'gemini' kvp value to mark the account from
 *    which it was copied. */
Account    * xaccCloneAccount (const Account *from, QofBook *book);

/** The xaccCloneAccountSimple() routine makes a simple copy of the
 *  indicated account, placing it in the indicated book.  It copies
 *  the account type, name, description, and the kvp values;
 *  it does not copy splits/transactions.  Note also that it 
 *  does NOT use the 'gemini' kvp value to indicate where it 
 *  was copied from.*/
Account    * xaccCloneAccountSimple (const Account *from, QofBook *book);

/** The xaccAccountBeginEdit() subroutine is the first phase of
 *    a two-phase-commit wrapper for account updates. */ 
void         xaccAccountBeginEdit (Account *account);

/** ThexaccAccountCommitEdit() subroutine is the second phase of
 *    a two-phase-commit wrapper for account updates. */ 
void         xaccAccountCommitEdit (Account *account);

/** The xaccAccountDestroy() routine can be used to get rid of an
 *    account.  The account should have been opened for editing 
 *    (by calling xaccAccountBeginEdit()) before calling this routine.*/
void         xaccAccountDestroy (Account *account);

/** Compare two accounts for equality - this is a deep compare. */
gboolean xaccAccountEqual(Account *a, Account* b, gboolean check_guids);

/** The xaccAccountOrder() subroutine defines a sorting order 
 *    on accounts.  It takes pointers to two accounts, and
 *    returns -1 if the first account is "less than" the second,
 *    returns +1 if the first is "greater than" the second, and
 *    0 if they are equal.  To determine the sort order, first
 *    the account codes are compared, and if these are equal, then 
 *    account types, and, if these are equal, the account names.
 */
int          xaccAccountOrder (Account **account_1, Account **account_2);

/** @} */

/* ------------------ */

/** @name Account lookup and GUID routines */
/** @{ */

/** The xaccAccountGetGUID() subroutine will return the
 *    globally unique id associated with that account. */
const GUID * xaccAccountGetGUID (Account *account);
 /**  The xaccAccountReturnGUID() subroutine returns the 
  *   same GUID as xaccAccountGetGUID, but as a struct. */
GUID         xaccAccountReturnGUID (Account *account);

/** The xaccAccountLookup() subroutine will return the
 *    account associated with the given id, or NULL
 *    if there is no such account. */
Account    * xaccAccountLookup (const GUID *guid, QofBook *book);

/**    xaccAccountLookupDirect performs the same function as
 * xaccAccountLookup but takes a GUID struct directly. */
Account    * xaccAccountLookupDirect (GUID guid, QofBook *book);

/** The xaccAccountLookupTwin() routine will find the "twin" of this
 *    account 'acc' in the given other 'book' (if the twin exists).
 *
 *    When accounts are copied or cloned, both of the pair are marked
 *    with the guid of thier copy, thus allowing the sibling-copy of
 *    an account to be found.  Since the sibling may end up in a
 *    different book, we need a way of finding it, given only that we
 *    know the book, and that we know its twin.  
 *
 *    That's what this routine does.  Given some book 'book', and an
 *    account 'acc', it will find the sibling account of 'acc' that is
 *    in 'book', and return it.  If not found, it returns NULL.  This
 *    routine uses the 'gemini' kvp values to do its work. */
Account * xaccAccountLookupTwin (Account *acc,  QofBook *book);
/** @} */

/* ------------------ */

/** @name Account general setters/getters */
/** @{ */

/** @return The book where the account is stored */
QofBook * xaccAccountGetBook (Account *account);

/** Set the account's type */
void xaccAccountSetType (Account *account, GNCAccountType);
/** Set the account's name */
void xaccAccountSetName (Account *account, const char *name);
/** Set the account's accounting code */
void xaccAccountSetCode (Account *account, const char *code);
/** Set the account's description */
void xaccAccountSetDescription (Account *account, const char *desc);
/** Set the account's notes */
void xaccAccountSetNotes (Account *account, const char *notes);
/** Set the last num field of an Account */
void xaccAccountSetLastNum (Account *account, const char *num);

/** Set the account's type */
GNCAccountType xaccAccountGetType (Account *account);
/** Get the account's name */
const char *   xaccAccountGetName (Account *account);
/** Get the account's accounting code */
const char *   xaccAccountGetCode (Account *account);
/** Get the account's description */
const char *   xaccAccountGetDescription (Account *account);
/** Get the account's notes */
const char *   xaccAccountGetNotes (Account *account);
/** Get the last num field of an Account */
const char *   xaccAccountGetLastNum (Account *account);

/** The xaccAccountGetFullName routine returns the fully qualified name
 * of the account using the given separator char. The name must be
 * g_free'd after use. The fully qualified name of an account is the
 * concatenation of the names of the account and all its ancestor
 * accounts starting with the topmost account and ending with the
 * given account. Each name is separated by the given character.
 *
 * @note: WAKE UP!
 * Unlike all other gets, the string returned by xaccAccountGetFullName() 
 * must be freed by you the user !!!
 * hack alert -- since it breaks the rule of string allocation, maybe this
 * routine should not be in this library, but some utility library?
 */
char *         xaccAccountGetFullName (Account *account, const char separator);

/** Set a string that identifies the Finance::Quote backend that
 *  should be used to retrieve online prices.  See price-quotes.scm
 *  for more information
 *
 *  @deprecated Price quote information is now stored on the
 *  commodity, not the account. */
void        dxaccAccountSetPriceSrc (Account *account, const char *src);
/** Get a string that identifies the Finance::Quote backend that
 *  should be used to retrieve online prices.  See price-quotes.scm
 *  for more information.
 *
 *  @deprecated Price quote information is now stored on the
 *  commodity, not the account. */
const char * dxaccAccountGetPriceSrc (Account *account);

/** Returns a per-account flag: Prior to reconciling an account which
    charges or pays interest, this flag tells whether to prompt the
    user to enter a transaction for the interest charge or
    payment. This per-account flag overrides the global preference. */
gboolean xaccAccountGetAutoInterestXfer (Account *account, gboolean default_value);
/** Sets a per-account flag: Prior to reconciling an account which
    charges or pays interest, this flag tells whether to prompt the
    user to enter a transaction for the interest charge or
    payment. This per-account flag overrides the global preference. */
void     xaccAccountSetAutoInterestXfer (Account *account, gboolean value);
/** @} */

/* @name Account Commodity setters/getters
 *
 * New commodity access routines.
 *
 * The account structure no longer stores two commodities ('currency'
 * and 'security'). Instead it stores only one commodity. This single
 * commodity is the one formerly known as 'security'.  Use
 * xaccAccountSetCommodity() and xaccAccountGetCommodity() to set and
 * fetch it. (This transition has been done between version 1.6.x and
 * 1.7.0.)
 *
 * Basically, the engine eliminates the 'currency' field of the
 * Account structure. Instead, the common currency is stored with the
 * transaction.  The 'value' of a split is a translation of the
 * Split's 'amount' (which is the amount of the Account's commodity
 * involved) into the Transaction's balancing currency. */
/** @{ */

/** Set the account's commodity */
void xaccAccountSetCommodity (Account *account, gnc_commodity *comm);
/** Get the account's commodity 
 *
 * This is from the new commodity access routines.
 *
 * The account structure no longer stores two commodities ('currency'
 * and 'security'). Instead it stores only one commodity. This single
 * commodity is the one formerly known as 'security'.  Use
 * xaccAccountSetCommodity() and xaccAccountGetCommodity() to set and
 * fetch it. (This transition has been done between version 1.6.x and
 * 1.7.0.)
 *
 * Basically, the engine eliminates the 'currency' field of the
 * Account structure. Instead, the common currency is stored with the
 * transaction.  The 'value' of a split is a translation of the
 * Split's 'amount' (which is the amount of the Account's commodity
 * involved) into the Transaction's balancing currency. */
gnc_commodity * xaccAccountGetCommodity (Account *account);
/** DOCUMENT ME! */
int  xaccAccountGetCommoditySCU (Account *account);
/** DOCUMENT ME! */
int  xaccAccountGetCommoditySCUi (Account *account);
/** DOCUMENT ME! */
void xaccAccountSetCommoditySCU (Account *account, int frac);
/** DOCUMENT ME! */
void xaccAccountSetCommoditySCUandFlag (Account *account, int frac);
/** DOCUMENT ME! */
void  xaccAccountSetNonStdSCU (Account *account, gboolean flag);
/** DOCUMENT ME! */
gboolean  xaccAccountGetNonStdSCU (Account *account);
/**@}*/


/** @name Account Balance */
/*@{*/
/** Get the current balance of the account */
gnc_numeric     xaccAccountGetBalance (Account *account);
/** Get the current balance of the account, only including cleared transactions */
gnc_numeric     xaccAccountGetClearedBalance (Account *account);
/** Get the current balance of the account, only including reconciled transactions */
gnc_numeric     xaccAccountGetReconciledBalance (Account *account);
gnc_numeric     xaccAccountGetPresentBalance (Account *account);
gnc_numeric     xaccAccountGetProjectedMinimumBalance (Account *account);
/** Get the balance of the account as of the date specified */
gnc_numeric     xaccAccountGetBalanceAsOfDate (Account *account, time_t date);

gnc_numeric xaccAccountConvertBalanceToCurrency(Account *account, /* for book */
						gnc_numeric balance,
						gnc_commodity *balance_currency,
						gnc_commodity *new_currency);
gnc_numeric xaccAccountGetBalanceInCurrency (Account *account,
					     gnc_commodity *report_commodity,
					     gboolean include_children);
gnc_numeric xaccAccountGetClearedBalanceInCurrency (Account *account,
						    gnc_commodity *report_commodity,
						    gboolean include_children);
gnc_numeric xaccAccountGetReconciledBalanceInCurrency (Account *account,
						       gnc_commodity *report_commodity,
						       gboolean include_children);
gnc_numeric xaccAccountGetPresentBalanceInCurrency (Account *account,
						    gnc_commodity *report_commodity,
						    gboolean include_children);
gnc_numeric xaccAccountGetProjectedMinimumBalanceInCurrency (Account *account,
							     gnc_commodity *report_commodity,
							     gboolean include_children);
/*@}*/


/** @name Account Children and Parents. 
 * The set of accounts is represented as a doubly-linked tree, so that given 
 * any account, both its parent and its children can be easily found.  
 * To make the management of sets of accounts easier, an account does not
 * directly point at its children, but rather at an 'Account Group' that
 * stores the children.  At the top of the tree heirarchy lies a single
 * root node, the root account group.
 * 
 * The account tree heirarchy is unique, in that a given account can 
 * have only one parent account. 
 */
/** @{ */

/** This routine returns the group holding the set of subaccounts 
 * for this account.  */
AccountGroup * xaccAccountGetChildren (Account *account);

/** This routine returns the group which contains this account.
 */
AccountGroup * xaccAccountGetParent (Account *account);

/** This routine returns the parent of the group that is the parent
 * of this account.  It is equivalent to the nested call
 * xaccGroupGetParentAccount (xaccAccountGetParent ())
 * Note that if the account is in the root group node, then its
 * parent will be NULL.
 */
Account *      xaccAccountGetParentAccount (Account *account);

/** This routine returns a flat list of all of the accounts
 * that are descendents of this account.  This includes not
 * only the the children, but the cheldren of the children, etc.
 * This routine is equivalent to the nested calls
 * xaccGroupGetSubAccounts (xaccAccountGetChildren())
 *
 * The returned list should be freed with g_list_free() when 
 * no longer needed.
 */
GList *        xaccAccountGetDescendants (Account *account);

/** DOCUMENT ME! */
void            xaccAccountSetReconcileChildrenStatus(Account *account, gboolean status);

/** DOCUMENT ME! */
gboolean        xaccAccountGetReconcileChildrenStatus(Account *account);

/** Returns true if the account has 'ancestor' as an ancestor.
 *  An ancestor account my be the accounts parent, its parent's parent,
 * its parent,s parent's parent, etc.
 *  Returns false if either one is NULL. 
 */
gboolean       xaccAccountHasAncestor (Account *account, Account *ancestor);
/** @} */

/* ------------------ */

/** @name Account KvpFrame getters/setters */
/** @{ */
KvpFrame * xaccAccountGetSlots (Account *account);
void xaccAccountSetSlots_nc(Account *account, KvpFrame *frame);

/** Delete any old data in the account's kvp data.
 * This includes the old currency and security fields. */
void xaccAccountDeleteOldData (Account *account);
/** @} */

/* ------------------ */

/** @name GNCAccountType conversion/checking
 */
/* @{ */
/**
 * Conversion routines for the account types to/from strings
 * that are used in persistant storage, communications.  These
 * strings should *not* be translated to the local language.
 * Typical converstion is INCOME -> "INCOME". */
const char * xaccAccountTypeEnumAsString (GNCAccountType type); 
/**
 * Conversion routines for the account types to/from strings
 * that are used in persistant storage, communications.  These
 * strings should *not* be translated to the local language.
 * Typical converstion is INCOME -> "INCOME". */
gboolean xaccAccountStringToType (const char* str, GNCAccountType *type);
/**
 * Conversion routines for the account types to/from strings
 * that are used in persistant storage, communications.  These
 * strings should *not* be translated to the local language.
 * Typical converstion is INCOME -> "INCOME". */
GNCAccountType xaccAccountStringToEnum (const char* str);

/** The xaccAccountGetTypeStr() routine returns a string suitable for 
 *  use in the GUI/Interface.  These strings should be translated
 *  to the local language. */
const char * xaccAccountGetTypeStr (GNCAccountType type); 
/** The xaccAccountGetTypeStr() routine returns a string suitable for 
 *  use in the GUI/Interface.  These strings should be translated
 *  to the local language. */
GNCAccountType xaccAccountGetTypeFromStr (const gchar *str);

/** Return TRUE if accounts of type parent_type can have accounts
 * of type child_type as children. */
gboolean xaccAccountTypesCompatible (GNCAccountType parent_type,
                                     GNCAccountType child_type);
/* @} */

/* ------------------ */

/** @name Account split/transaction list management */
/*@{*/
/** The xaccAccountInsertSplit() method will insert the indicated
 *    split into the indicated account.  If the split already 
 *    belongs to another account, it will be removed from that
 *    account first.*/
void         xaccAccountInsertSplit (Account *account, Split *split);

/** The xaccAccountGetSplitList() routine returns a pointer to a GList of
 *    the splits in the account.  
 * @note This GList is the account's internal 
 *    data structure: do not delete it when done; treat it as a read-only
 *    structure.  Note that some routines (such as xaccAccountRemoveSplit())
 *    modify this list directly, and could leave you with a corrupted 
 *    pointer. */
SplitList*      xaccAccountGetSplitList (Account *account);

/** \warning  Unimplemented */
gpointer xaccAccountForEachSplit(Account *account,
                                 SplitCallback,
                                 gpointer data);

/** The xaccAccountForEachTransaction() routine will traverse all of
   the transactions in the given 'account' and call the callback
   function 'proc' on each transaction.  Processing will continue
   if-and-only-if 'proc' returns 0. The user data pointer
   'data' will be passed on to the callback function 'proc'.

   This function does not descend recursively to traverse transactions
   in child accounts.

   'proc' will be called exactly once for each transaction that is
   pointed to by at least one split in the given account.

   The result of this function will be 0 if-and-only-if
   every relevant transaction was traversed exactly once. 
   Else the return value is the last non-zero value returned by proc.

   Note that the traversal occurs only over the transactions that 
   are locally cached in the local gnucash engine.  If the gnucash 
   engine is attached to a remote database, the database may contain
   (many) transactions that are not mirrored in the local cache.
   This routine will not cause an SQL database query to be performed;
   it will not traverse transactions present only in the remote
   database.
*/
gint xaccAccountForEachTransaction(Account *account,
                                   TransactionCallback proc,
                                   void *data);

/** Returns a pointer to the transaction, not a copy. */
Transaction * xaccAccountFindTransByDesc(Account *account, 
                                   const char *description);

/** Returns a pointer to the split, not a copy. */
Split * xaccAccountFindSplitByDesc(Account *account, const char *description);

/** The xaccAccountFixSplitDateOrder() subroutine checks to see if 
 *    a split is in proper sorted date order with respect 
 *    to the other splits in this account. */
void         xaccAccountFixSplitDateOrder (Account *account, Split *split);

/** The xaccTransFixSplitDateOrder() checks to see if 
 *    all of the splits in this transaction are in
 *    proper date order. */
void         xaccTransFixSplitDateOrder (Transaction *trans);
/*@}*/

/* ------------------ */

/** @name Account lots */
/*@{*/
/** The xaccAccountInsertLot() method will register the indicated lot 
 *    with this account.   Any splits later inserted into this lot must 
 *    belong to this account.  If the lot is already in another account,
 *    the lot, and all of the splits in it, will be moved from that
 *    account to this account. */
void xaccAccountInsertLot (Account *, GNCLot *);
void xaccAccountRemoveLot (Account *, GNCLot *);

 /** The xaccAccountGetLotList() routine returns a pointer to the GList of
 *    the lots in this account.  The same warnings as above apply. */
LotList*        xaccAccountGetLotList (Account *account);

/** The xaccAccountForEachLot() method will apply the function 'proc'
 *    to each lot in the account.  If 'proc' returns a non-NULL value,
 *    further application will be stopped, and the resulting value
 *    will be returned.  There is no guarenteed order over which
 *    the Lots will be traversed.
 */
gpointer xaccAccountForEachLot(Account *acc,
                              gpointer (*proc)(GNCLot *lot, gpointer user_data),
                              gpointer user_data);


/** Find a list of open lots that match the match_func.  Sort according
 * to sort_func.  If match_func is NULL, then all open lots are returned.
 * If sort_func is NULL, then the returned list has no particular order.
 * The caller must free to returned list.
 */
LotList * xaccAccountFindOpenLots (Account *acc,
				   gboolean (*match_func)(GNCLot *lot,
							  gpointer user_data),
				   gpointer user_data, GCompareFunc sort_func);

/*@}*/


/* ------------------ */

/** @name Account Reconciliation information getters/setters */
/** @{ */
/** DOCUMENT ME! */
gboolean       xaccAccountGetReconcileLastDate (Account *account,
                                                time_t *last_date);
/** DOCUMENT ME! */
void           xaccAccountSetReconcileLastDate (Account *account,
                                                time_t last_date);

/** DOCUMENT ME! */
gboolean       xaccAccountGetReconcileLastInterval (Account *account,
						    int *months, int *days);
/** DOCUMENT ME! */
void           xaccAccountSetReconcileLastInterval (Account *account,
						    int months, int days);

/** DOCUMENT ME! */
gboolean       xaccAccountGetReconcilePostponeDate (Account *account,
                                                    time_t *postpone_date);
/** DOCUMENT ME! */
void           xaccAccountSetReconcilePostponeDate (Account *account,
                                                    time_t postpone_date);

/** DOCUMENT ME! */
gboolean       xaccAccountGetReconcilePostponeBalance (Account *account,
                                                       gnc_numeric *balance);
/** DOCUMENT ME! */
void           xaccAccountSetReconcilePostponeBalance (Account *account,
                                                       gnc_numeric balance);

/** DOCUMENT ME! */
void           xaccAccountClearReconcilePostpone (Account *account);
/** @} */


/** DOCUMENT ME! */
typedef enum 
  {
  PLACEHOLDER_NONE,
  PLACEHOLDER_THIS,
  PLACEHOLDER_CHILD,
  } GNCPlaceholderType;

/** @name Account Placeholder flag */
/** @{ */
/** DOCUMENT ME! */
gboolean        xaccAccountGetPlaceholder (Account *account);
/** DOCUMENT ME! */
void            xaccAccountSetPlaceholder (Account *account,
                                           gboolean option);
/** DOCUMENT ME! */
GNCPlaceholderType xaccAccountGetDescendantPlaceholder (Account *account);
/** @} */


/** @name Account Tax related getters/setters */
/** @{ */

/** DOCUMENT ME! */
gboolean        xaccAccountGetTaxRelated (Account *account);
/** DOCUMENT ME! */
void            xaccAccountSetTaxRelated (Account *account,
                                          gboolean tax_related);

/** DOCUMENT ME! */
const char *    xaccAccountGetTaxUSCode (Account *account);
/** DOCUMENT ME! */
void            xaccAccountSetTaxUSCode (Account *account, const char *code);
/** DOCUMENT ME! */
const char *    xaccAccountGetTaxUSPayerNameSource (Account *account);
/** DOCUMENT ME! */
void            xaccAccountSetTaxUSPayerNameSource (Account *account,
                                                    const char *source);
/** @} */


/** @name Account marking */
/*@{*/
/** Set a mark on the account.  The meaning of this mark is
 * completely undefined. Its presented here as a utility for the
 * programmer, to use as desired.  Handy for performing customer traversals
 * over the account tree.  The mark is *not* stored in the database/file
 * format.  When accounts are newly created, the mark is set to zero.
 */
void           xaccAccountSetMark (Account *account, short mark); 

/** Get the mark set by xaccAccountSetMark */
short          xaccAccountGetMark (Account *account);

/** The xaccClearMark will find the topmost group, and clear the mark in
 * the entire group tree.  */
void           xaccClearMark (Account *account, short val);

/** The xaccClearMarkDown will clear the mark only in this and in
 * sub-accounts.*/
void           xaccClearMarkDown (Account *account, short val);
/** Will clear the mark for all the accounts of the AccountGroup .*/
void           xaccClearMarkDownGr (AccountGroup *group, short val);
/*@}*/


/** @name Account deprecated routines. */
/** @{ */

/** @deprecated The current API associates only one thing with an
 * account: the 'commodity'. Use xaccAccountGetCommodity() to fetch
 * it.
 *
 * These two funcs take control of their gnc_commodity args. Don't free */
void DxaccAccountSetCurrency (Account *account, gnc_commodity *currency);

/** @deprecated The current API associates only one thing with an
 * account: the 'commodity'. Use xaccAccountGetCommodity() to fetch
 * it.
 *
 * These two funcs take control of their gnc_commodity args. Don't free */
void DxaccAccountSetSecurity (Account *account, gnc_commodity *security);

/** @deprecated The current API associates only one thing with an
 * account: the 'commodity'. Use xaccAccountGetCommodity() to fetch
 * it. */
gnc_commodity * DxaccAccountGetCurrency (Account *account);

/** @deprecated The current API associates only one thing with an
 * account: the 'commodity'. Use xaccAccountGetCommodity() to fetch
 * it. */
gnc_commodity * DxaccAccountGetSecurity (Account *account);

/** @deprecated The current API associates only one thing with an
 * account: the 'commodity'. Use xaccAccountGetCommodity() to fetch
 * it. */
void DxaccAccountSetCurrencySCU (Account *account, int frac);
/** @deprecated The current API associates only one thing with an
 * account: the 'commodity'. Use xaccAccountGetCommodity() to fetch
 * it. */
int  DxaccAccountGetCurrencySCU (Account *account);

/** Set the timezone to be used when interpreting the results from a
 *  given Finance::Quote backend.  Unfortunately, the upstream sources
 *  don't label their output, so the user has to specify this bit.
 *
 *  @deprecated Price quote information is now stored on the
 *  commodity, not the account. */
void         dxaccAccountSetQuoteTZ (Account *account, const char *tz);
/** Get the timezone to be used when interpreting the results from a
 *  given Finance::Quote backend.  Unfortunately, the upstream sources
 *  don't label their output, so the user has to specify this bit.
 *
 *  @deprecated Price quote information is now stored on the
 *  commodity, not the account. */
const char * dxaccAccountGetQuoteTZ (Account *account);
/**@}*/


/** @name Account parameter names */
/** @{ */
#define ACCOUNT_KVP		"kvp"
#define ACCOUNT_NAME_		"name"
#define ACCOUNT_CODE_		"code"
#define ACCOUNT_DESCRIPTION_	"desc"
#define ACCOUNT_NOTES_		"notes"
#define ACCOUNT_BALANCE_	"balance"
#define ACCOUNT_CLEARED_	"cleared"
#define ACCOUNT_RECONCILED_	"reconciled"
#define ACCOUNT_PRESENT_	"present"
#define ACCOUNT_FUTURE_MINIMUM_ "future-minimum"
#define ACCOUNT_TAX_RELATED	"tax-related-p"
/** @} */

/** This is the type-override when you want to match all accounts.  Used
 * in the gnome-search parameter list.  Be careful when you use this. */
#define ACCOUNT_MATCH_ALL_TYPE	"account-match-all"

#endif /* XACC_ACCOUNT_H */
/** @} */
