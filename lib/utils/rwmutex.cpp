/** @file rwmutex.cpp
 * @author Tanishq Aggarwal
 * @date 6 Feb 2018
 * @brief Implementation of reader-writer lock.
 */

#include "rwmutex.hpp"

void rwMtxObjectInit(rwmutex_t *rwmtx) {
    chMtxObjectInit(&rwmtx->mtx);
    chSemObjectInit(&rwmtx->sem, rwmtx->max_readers);
}

void rwMtxRLock(rwmutex_t *rwmtx) { chSemWait(&rwmtx->sem); }

void rwMtxRUnlock(rwmutex_t *rwmtx) { chSemSignal(&rwmtx->sem); }

void rwMtxWLock(rwmutex_t *rwmtx) {
    chMtxLock(&rwmtx->mtx);
    while(chSemGetCounterI(&rwmtx->sem) > 0) chSemWait(&rwmtx->sem);
    chMtxUnlock(&rwmtx->mtx);
}

void rwMtxWUnlock(rwmutex_t *rwmtx) {
    chSemReset(&rwmtx->sem, rwmtx->max_readers);
}

void rwMtxRLockI(rwmutex_t *rwmtx) { chSemFastWaitI(&rwmtx->sem); }

void rwMtxRUnlockI(rwmutex_t *rwmtx) { chSemSignalI(&rwmtx->sem); }

void rwMtxWLockI(rwmutex_t *rwmtx) {
    chMtxLock(&rwmtx->mtx);
    while(chSemGetCounterI(&rwmtx->sem) > 0) chSemFastWaitI(&rwmtx->sem);
    chMtxUnlock(&rwmtx->mtx);
}

void rwMtxWUnlockI(rwmutex_t *rwmtx) {
    chSemResetI(&rwmtx->sem, rwmtx->max_readers);
}