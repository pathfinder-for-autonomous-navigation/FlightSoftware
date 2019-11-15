/** @file rwmutex.cpp
 * @author Tanishq Aggarwal
 * @date 6 Feb 2018
 * @brief Header file for a reader-writer lock.
 *
 * Contains a read-write mutex class based on ChibiOS primitives.
 * Implementation inspired by https://doc.qt.io/archives/qq/qq11-mutex.html.
 * Written in ChibiOS style to make code cleaner.
 */

#include <ChRt.h>

#ifndef RW_MUTEX_H_
#define RW_MUTEX_H_

/** @brief Reader-writer object.
 *
 *  Contains a mutex for actual locking and a semaphore
 *  to keep track of the number of readers. The semaphore
 *  has a default capacity of 32; a writer automatically fills
 *  the semaphore.
 *
 *  Implementation inspired by https://doc.qt.io/archives/qq/qq11-mutex.html.
 * **/
struct rwmutex_t {
    bool initialized = false;
    //! Mutex that is locked if a process requires exclusive write access to the
    //! data
    // protected by this readers-writers mutex.
    mutex_t mtx;
    //! Semaphore that keeps track of how many readers are present on this mutex.
    //! Locks
    // out if either too many readers are present or if a writer is present.
    semaphore_t sem;
    //! The number of allowed readers on this readers-writers lock. Defaults to
    //! 32.
    int max_readers = 32;
};

// TODO test this implementation!

/** @brief Initializes a reader-writer object.
 *  @param rwmtx Pointer to a reader-writer object.
 * **/
void rwMtxObjectInit(rwmutex_t *rwmtx);

/** @brief Locks a reader-writer object for reading.
 *
 *  If a thread has currently locked the lock for writing, or if
 *  the maximum number of readers is exceeded, the thread requesting
 *  this lock will be locked.
 *
 *  @param rwmtx Pointer to a reader-writer object.
 * **/
void rwMtxRLock(rwmutex_t *rwmtx);

/** @brief Unlocks a reader-writer object for reading.
 *  @param rwmtx Pointer to a reader-writer object.
 * **/
void rwMtxRUnlock(rwmutex_t *rwmtx);

/** @brief Locks a reader-writer object for writing.
 *
 *  If a thread has currently locked the lock for writing or reading,
 *  the thread requesting this lock will be locked.
 *
 *  @param rwmtx Pointer to a reader-writer object.
 * **/
void rwMtxWLock(rwmutex_t *rwmtx);

/** @brief Unlocks a reader-writer object for writing.
 *  @param rwmtx Pointer to a reader-writer object.
 * **/
void rwMtxWUnlock(rwmutex_t *rwmtx);

#endif