#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>
#include <inttypes.h>
#include <pthread.h>
#include <semaphore.h>
#include "teller.h"
#include "account.h"
#include "error.h"
#include "debug.h"
#include "account.c"
#include "branch.h"

/*
 * deposit money into an account
 */
int Teller_DoDeposit(Bank *bank, AccountNumber accountNum, AccountAmount amount)
{
  assert(amount >= 0);

  DPRINTF('t', ("Teller_DoDeposit(account 0x%" PRIx64 " amount %" PRId64 ")\n",
                accountNum, amount));

  Account *account = Account_LookupByNumber(bank, accountNum);

  if (account == NULL)
  {
    return ERROR_ACCOUNT_NOT_FOUND;
  }
  sem_wait(&(bank->branches[AccountNum_GetBranchID(accountNum)].lockToBranch));
  pthread_mutex_lock(&account->accLocking);
  Account_Adjust(bank, account, amount, 1);
  sem_post(&(bank->branches[AccountNum_GetBranchID(accountNum)].lockToBranch));
  pthread_mutex_unlock(&account->accLocking);
  return ERROR_SUCCESS;
}

/*
 * withdraw money from an account
 */
int Teller_DoWithdraw(Bank *bank, AccountNumber accountNum, AccountAmount amount)
{
  assert(amount >= 0);

  DPRINTF('t', ("Teller_DoWithdraw(account 0x%" PRIx64 " amount %" PRId64 ")\n",
                accountNum, amount));

  Account *account = Account_LookupByNumber(bank, accountNum);
  if (account == NULL){
    return ERROR_ACCOUNT_NOT_FOUND;
  }
  sem_wait(&(bank->branches[AccountNum_GetBranchID(accountNum)].lockToBranch));
  pthread_mutex_lock(&account->accLocking);
  if (amount > Account_Balance(account)){
    sem_post(&(bank->branches[AccountNum_GetBranchID(accountNum)].lockToBranch));
    pthread_mutex_unlock(&account->accLocking);
    return ERROR_INSUFFICIENT_FUNDS;
  }

  Account_Adjust(bank, account, -amount, 1);
    sem_post(&(bank->branches[AccountNum_GetBranchID(accountNum)].lockToBranch));
    pthread_mutex_unlock(&account->accLocking);
  return ERROR_SUCCESS;
}

/*
 * do a tranfer from one account to another account
 */
int Teller_DoTransfer(Bank *bank, AccountNumber srcAccountNum,
                      AccountNumber dstAccountNum,
                      AccountAmount amount){
  assert(amount >= 0);

  DPRINTF('t', ("Teller_DoTransfer(src 0x%" PRIx64 ", dst 0x%" PRIx64
                ", amount %" PRId64 ")\n",
                srcAccountNum, dstAccountNum, amount));

  Account *srcAccount = Account_LookupByNumber(bank, srcAccountNum);
  if (srcAccount == NULL)
  {
    return ERROR_ACCOUNT_NOT_FOUND;
  }

  Account *dstAccount = Account_LookupByNumber(bank, dstAccountNum);
  if (dstAccount == NULL)
  {
    return ERROR_ACCOUNT_NOT_FOUND;
  }
  if(srcAccount == dstAccount) {
  return ERROR_SUCCESS;
  }
  int updateBranch = !Account_IsSameBranch(srcAccountNum, dstAccountNum);
  if(updateBranch == 0){
      if(srcAccount->accountNumber < dstAccount->accountNumber) {  
        pthread_mutex_lock(&(srcAccount->accLocking));
        pthread_mutex_lock(&(dstAccount->accLocking));
      } else {
        pthread_mutex_lock(&(dstAccount->accLocking)); 
        pthread_mutex_lock(&(srcAccount->accLocking));
      }

    if (amount > Account_Balance(srcAccount)){
      pthread_mutex_unlock(&(srcAccount->accLocking));
      pthread_mutex_unlock(&(dstAccount->accLocking));
      return ERROR_INSUFFICIENT_FUNDS;
    }

    Account_Adjust(bank, srcAccount, -amount, updateBranch);
    Account_Adjust(bank, dstAccount, amount, updateBranch);
    pthread_mutex_unlock(&(srcAccount->accLocking));
    pthread_mutex_unlock(&(dstAccount->accLocking));
    return ERROR_SUCCESS;
  }else{
    int srcBranchID, dstBranchID;
    srcBranchID = AccountNum_GetBranchID(srcAccountNum);
    dstBranchID = AccountNum_GetBranchID(dstAccountNum);
    if (srcBranchID < dstBranchID) {           
      sem_wait(&(bank->branches[srcBranchID].lockToBranch));   
      sem_wait(&(bank->branches[dstBranchID].lockToBranch));
      pthread_mutex_lock(&(srcAccount->accLocking));
      pthread_mutex_lock(&(dstAccount->accLocking));         
    } else {
      sem_wait(&(bank->branches[dstBranchID].lockToBranch));
      sem_wait(&(bank->branches[srcBranchID].lockToBranch));
      pthread_mutex_lock(&(dstAccount->accLocking));  
      pthread_mutex_lock(&(srcAccount->accLocking));
    }
    if (amount > Account_Balance(srcAccount)){
      sem_post(&(bank->branches[srcBranchID].lockToBranch));   
      sem_post(&(bank->branches[dstBranchID].lockToBranch));
      pthread_mutex_unlock(&(srcAccount->accLocking));
      pthread_mutex_unlock(&(dstAccount->accLocking));
      return ERROR_INSUFFICIENT_FUNDS;
    }
    Account_Adjust(bank, srcAccount, -amount, updateBranch);
    Account_Adjust(bank, dstAccount, amount, updateBranch);
    sem_post(&(bank->branches[srcBranchID].lockToBranch));   
    sem_post(&(bank->branches[dstBranchID].lockToBranch));
    pthread_mutex_unlock(&(srcAccount->accLocking));
    pthread_mutex_unlock(&(dstAccount->accLocking));
    return ERROR_SUCCESS;
  }

}
