/************************************************************************
**                                                                     **
**                   The YapTab/YapOr/OPTYap systems                   **
**                                                                     **
** YapTab extends the Yap Prolog engine to support sequential tabling  **
** YapOr extends the Yap Prolog engine to support or-parallelism       **
** OPTYap extends the Yap Prolog engine to support or-parallel tabling **
**                                                                     **
**                                                                     **
**      Yap Prolog was developed at University of Porto, Portugal      **
**                                                                     **
************************************************************************/

/* **********************************************************************
**                      Atomic locks for PTHREADS                      **
************************************************************************/

#include <pthread.h>

#define INIT_LOCK(LOCK_VAR)    pthread_mutex_init(&(LOCK_VAR), NULL)
#define DESTROY_LOCK(LOCK_VAR) pthread_mutex_destroy(&(LOCK_VAR))
#define TRY_LOCK(LOCK_VAR)     pthread_mutex_trylock(&(LOCK_VAR))
#if DEBUG_LOCKS
#define LOCK(LOCK_VAR)         if (debug_locks) fprintf(stderr,"[%d] %s:%d: LOCK(%p)\n",  (int)pthread_self(),	\
						__BASE_FILE__, __LINE__,&(LOCK_VAR));   \
                                        pthread_mutex_lock(&(LOCK_VAR))
#define UNLOCK(LOCK_VAR)       if (debug_locks) fprintf(stderr,"[%d] %s:%d: UNLOCK(%p)\n",  (int)pthread_self(),	\
						__BASE_FILE__, __LINE__,&(LOCK_VAR));   \
                                        pthread_mutex_unlock(&(LOCK_VAR))
#else
#define LOCK(LOCK_VAR)         pthread_mutex_lock(&(LOCK_VAR))
#define UNLOCK(LOCK_VAR)       pthread_mutex_unlock(&(LOCK_VAR))
#endif
static inline int
xIS_LOCKED(pthread_mutex_t *LOCK_VAR) {
  if (pthread_mutex_trylock(LOCK_VAR) == 0) {
    pthread_mutex_unlock(LOCK_VAR);
    return TRUE;
  }
  return FALSE;
}
static inline int
xIS_UNLOCKED(pthread_mutex_t *LOCK_VAR) {
  if (pthread_mutex_trylock(LOCK_VAR) == 0) {
    pthread_mutex_unlock(LOCK_VAR);
    return FALSE;
  }
  return TRUE;
}
#define IS_LOCKED(LOCK_VAR)    xIS_LOCKED(&(LOCK_VAR))
#define IS_UNLOCKED(LOCK_VAR)  xIS_UNLOCKED(&(LOCK_VAR))

#define INIT_RWLOCK(X)         pthread_rwlock_init(&(X), NULL)
#define DESTROY_RWLOCK(X)      pthread_rwlock_destroy(&(X))
#define READ_LOCK(X)           pthread_rwlock_rdlock(&(X))
#define READ_UNLOCK(X)         pthread_rwlock_unlock(&(X))
#define WRITE_LOCK(X)          pthread_rwlock_wrlock(&(X))
#define WRITE_UNLOCK(X)        pthread_rwlock_unlock(&(X))


#if THREADS

/* pthread mutex */

#if DEBUG_LOCKS

#define MUTEX_LOCK(LOCK_VAR)     if (debug_locks) fprintf(stderr,"[%d] %s:%d: MULOCK(%p)\n",  (int)pthread_self(),	\
						__BASE_FILE__, __LINE__,(LOCK_VAR));   \
                                        pthread_mutex_lock((LOCK_VAR))
#define MUTEX_TRYLOCK(LOCK_VAR)  pthread_mutex_TRYlock((LOCK_VAR))
#define MUTEX_UNLOCK(LOCK_VAR) if (debug_locks) fprintf(stderr,"[%d] %s:%d: UNMULOCK(%p)\n",  (int)pthread_self(),	\
						__BASE_FILE__, __LINE__,(LOCK_VAR));   \
                                        pthread_mutex_unlock((LOCK_VAR))
#else
#define MUTEX_LOCK(LOCK_VAR) pthread_mutex_lock((LOCK_VAR))
#define MUTEX_TRYLOCK(LOCK_VAR)  pthread_mutex_trylock((LOCK_VAR))
#define MUTEX_UNLOCK(LOCK_VAR) pthread_mutex_unlock((LOCK_VAR))
#endif

#else

#define MUTEX_LOCK(LOCK_VAR) 
#define MUTEX_TRYLOCK(LOCK_VAR)
#define MUTEX_UNLOCK(LOCK_VAR) 


#endif
