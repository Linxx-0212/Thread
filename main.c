/* #define DEBUG */

#include <errno.h>
#include <inttypes.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <signal.h>
#include <sys/time.h>

#include "floral.h"
#include "utils.h"
#include "main.h"
#include "thread.h"
#include "utils.h"

/*
 * Initial global state.
 */
floral_state STATE = {
  .threads = NULL,
  .current = NULL
};
/**
 * Initializes the choloros green thread library.
 *
 * Creates the initial green thread from the currently executing context. The
 * `preempt` parameters specifies whether the scheduler is preemptive or not.
 * This function should only be called once.
 *
 * @param preempt true if the scheduler should preempt, false otherwise
 */
void grn_init(bool preempt) {
  STATE.current = grn_new_thread(false);
  assert_malloc(STATE.current);
  STATE.current->status = RUNNING;
  if (preempt) {
    // FIXME: The user has requested preemption. Enable the functionality.
  }
}

/**
 * Creates a new green thread and executes `fn` inside that thread.
 *
 * Allocates and initializes a new green thread so that the parameter `fn` is
 * executed inside of the new thread. Each thread is allocated its own stack.
 * After allocating and initialization the new thread, the current thread yields
 * its execution.
 *
 * @param fn The function to execute inside a new green thread.
 *
 * @return The thread ID of the newly spawned process.
 */
int grn_spawn(grn_fn fn) {
  UNUSED(fn);

  grn_thread *thread = grn_new_thread(true);
  thread->context.rsp = (uint64_t)(thread->stack + STACK_SIZE);
  thread->context.rsp -= sizeof(void**);;
  *(void**)thread->context.rsp = (void*)fn;
  
  thread->context.rsp -= sizeof(void**);;
  *(void**)thread->context.rsp = (void*)start_thread;
  thread->status = READY;
  grn_yield();

  return thread->id;

  // FIXME: Allocate a new thread, initialize its context, then yield.

}

/**
 * Garbage collects ZOMBIEd threads.
 *
 * Frees the resources for all threads marked ZOMBIE.
 */
void grn_gc() {
  // FIXME: Free the memory of zombied threads.6
  grn_thread *iter_thread = next_thread(STATE.current);

  while (iter_thread != STATE.current) {
    if(iter_thread->status == ZOMBIE) {
      remove_thread(iter_thread);
    }
    iter_thread = next_thread(iter_thread);
  }
  if (STATE.current->status == ZOMBIE)
    remove_thread(STATE.current);

}

/**
 * Yields the execution time of the current thread to another thread.
 *
 * If there is at least one READY thread, this function chooses one through an
 * arbitrary search and context switches into it. The current thread is marked
 * READY if it was previous RUNNING, otherwise, its status remained unchanged.
 * The status of the thread being switched to is marked RUNNING. If no READY
 * thread is found, this function return -1. Otherwise, it returns 0.
 *
 * @return 0 if execution was yielded, -1 if no yielding occured
 */
int grn_yield() {
  // FIXME: Yield the current thread's execution time to another READY thread.
  grn_thread *Next_thread = next_thread(STATE.current);
  bool flag = false;
  //debug_thread_print(Next_thread);

  //debug_thread_print(STATE.current);
  //fprintf(stderr, "%s\n", next_thread);
  while(Next_thread != STATE.current) {
    
    if (Next_thread->status == READY) {
      flag = true;
      break;
    }
    Next_thread = next_thread(Next_thread);
  }
  if (flag == false)
    return -1;
  if (STATE.current->status == RUNNING)
    STATE.current->status = READY;
  Next_thread->status = RUNNING;
  grn_thread *tmp = STATE.current;
  STATE.current = Next_thread;
  //fprintf(stderr, "%s\n", "111");
  grn_context_switch(&(tmp->context),&(STATE.current->context));
  //fprintf(stderr, "%s\n", "111");
  return 0;
}

/**
 * Blocks until all threads are finished executing.
 *
 * TODO: Keep track of parent->children relationships so that a thread only
 * waits for the threads it spawned. TODO: Take in a list of thread IDs as a
 * parameter and wait for those threads.
 *
 * @return 0 on successful wait, nonzero otherwise
 */
int grn_wait() {
  // Loop until grn_yield returns nonzero.
  while (!grn_yield());

  return 0;
}

/**
 * Exits from the calling thread.
 *
 * If the calling thread is the initial thread, then this function exits the
 * progam. Otherwise, the calling thread is marked ZOMBIE so that it is never
 * rescheduled and is eventually garbage collected. This function never returns.
 */
void grn_exit() {
  debug("Thread %" PRId64 " is exiting.\n", STATE.current->id);
  if (STATE.current->id == 0) {
    exit(0);
  }

  STATE.current->status = ZOMBIE;
  grn_yield();
}

/**
 * For compatbility across name manglers.
 */
void _grn_exit() { grn_exit(); }

/**
 * Returns a pointer to the current thread if there is one. This pointer is only
 * valid during the lifetime of the thread.
 *
 * @return a pointer to the current thread or NULL if the library hasn't been
 * initialized
 */
grn_thread *grn_current() {
  return STATE.current;
}
