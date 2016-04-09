/*
 * This file is part of the µOS++ distribution.
 *   (https://github.com/micro-os-plus)
 * Copyright (c) 2016 Liviu Ionescu.
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom
 * the Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#include <cmsis-plus/rtos/os.h>
// #include <cmsis-plus/rtos/os-lists.h>

// Better be the last, to undef putchar()
#include <cmsis-plus/diag/trace.h>

namespace os
{
  namespace rtos
  {

    // ========================================================================

    /**
     * @details
     * The initial list status is empty.
     */
    Double_list::Double_list ()
    {
      clear ();
    }

    /**
     * @details
     * There must be no nodes in the list.
     */
    Double_list::~Double_list ()
    {
      assert(head_ == nullptr);
      assert(count_ == 0);
    }

    /**
     * @details
     * Reset the count and clear the head pointer.
     */
    void
    Double_list::clear (void)
    {
      head_ = nullptr;
      count_ = 0;
    }

    /**
     * @details
     * If the list has more than one node, update the neighbours to
     * point to each other, skipping the node.
     *
     * For lists with only one node, simply clear the list.
     *
     * For more robustness, to prevent unexpected accesses,
     * the links in the removed node are nullified.
     */
    void
    Double_list::remove (Double_list_links& node)
    {
      // Check if not already removed.
      if (node.next == nullptr)
        {
          return;
        }

      if (count_ > 1)
        {
          if (head_ == &node)
            {
              // Move head to next node, this one will vanish.
              head_ = (Double_list_links*) node.next;
            }

          // Make neighbours point to each other.
          node.prev->next = node.next;
          node.next->prev = node.prev;

          --count_;
        }
      else if (count_ == 1)
        {
          clear ();
        }

      // Nullify both pointers in the removed node.
      node.prev = nullptr;
      node.next = nullptr;
    }

    // ========================================================================

    /**
     * @class Waiting_threads_list
     * @details
     * There are at least two strategies:
     * - keep the list ordered by priorities and have the top node
     *  easily accessible the head
     * - preserve the insertion order and perform a full list traversal
     *  to determine the top node.
     *
     * The first strategy requires a partial list traverse with each
     * insert, to find the place to insert the node, but makes
     * retrieving the top priority node trivial, by a single access
     * to the list head.
     *
     * The second strategy might minimise the overall processing
     * time, but always requires a full list traversal to determine
     * the top priority node.
     *
     * On the other hand, typical waiting lists contain only one
     * element, and in this case there is no distinction. Mutex
     * objects occasionally might have two entries (and rarely more).
     * Condition variables might also have several waiting threads,
     * the number is usually small. In these cases, the distinction
     * between the two strategies is also minimal.
     *
     * In the rare cases when the waiting list is large, the first
     * strategy favours top node retrieval, possibly
     * improving the response time, and is thus preferred.
     */

    /**
     * @details
     * Based on priority, the node is inserted
     * - at the end of the list,
     * - at the beginning of the list,
     * - in the middle of the list, which
     * requires a partial list traversal (done from the end).
     *
     * If the list is empty, the new node is added with references to
     * itself, to satisfy the circular double linked list requirements.
     */
    void
    Waiting_threads_list::add (Waiting_thread_node& node)
    {
      if (head_ == nullptr)
        {
          // Make the node point to itself, to satisfy the double
          // linked circular list.
          node.prev = &node;
          node.next = &node;

          head_ = &node;
          count_ = 1;
        }
      else
        {
          thread::priority_t prio = node.thread.sched_prio ();

          Waiting_thread_node* after = (Waiting_thread_node*) (head_->prev);

          if (prio <= after->thread.sched_prio ())
            {
              // Insert at the end of the list.
            }
          else if (prio > head ()->thread.sched_prio ())
            {
              // Insert at the beginning of the list
              // and update the new head.
              head_ = &node;
#if defined(OS_TRACE_RTOS_LISTS)
              trace::printf ("%s() head \n", __func__);
#endif
            }
          else
            {
              // Insert in the middle of the list.
              // The loop is guaranteed to terminate and the
              // weight is small, sched_prio() is only an accessor.
              while (prio > after->thread.sched_prio ())
                {
                  after = (Waiting_thread_node*) after->prev;
                }
            }

          // Make the new node point to its neighbours.
          node.prev = after;
          node.next = after->next;

          // Make the neighbours point to the n. The order is important.
          after->next->prev = &node;
          after->next = &node;

          ++count_;
        }
    }

    /**
     * @details
     * Atomically get the top thread from the list, remove the node
     * and wake-up the thread.
     */
    void
    Waiting_threads_list::resume_one (void)
    {
      Thread* thread;
        {
          interrupts::Critical_section ics; // ----- Critical section -----

          // If the list is empty, silently return.
          if (empty ())
            {
              return;
            }

          // The top priority is to remove the entry from the list
          // so that subsequent wakeups to address different threads.
          thread = &(head ()->thread);
          remove (*head ());
        }
      assert(thread != nullptr);

      thread::state_t state = thread->sched_state ();
      if (state != thread::state::destroyed)
        {
          thread->resume ();
        }
      else
        {
#if defined(OS_TRACE_RTOS_LISTS)
          trace::printf ("%s() gone \n", __func__);
#endif
        }
    }

    void
    Waiting_threads_list::resume_all (void)
    {
      while (!empty ())
        {
          resume_one ();
        }
    }

    // ========================================================================

    Timestamp_node::Timestamp_node (Double_list& lst, clock::timestamp_t ts) :
        Double_list_links
          { lst }, //
        timestamp (ts)
    {
      ;
    }

    Timestamp_node::~Timestamp_node ()
    {
      ;
    }

    // ========================================================================

    Timeout_thread_node::Timeout_thread_node (Double_list& lst,
                                              clock::timestamp_t ts, Thread& th) :
        Timestamp_node
          { lst, ts }, //
        thread (th)
    {
      ;
    }

    Timeout_thread_node::~Timeout_thread_node ()
    {
      ;
    }

    void
    Timeout_thread_node::action (void)
    {
      Thread* th = &this->thread;
      ((Clock_timestamps_list&) list).remove (*this);

      thread::state_t state = th->sched_state ();
      if (state != thread::state::destroyed)
        {
          th->resume ();
        }
    }

    // ========================================================================

#if !defined(OS_INCLUDE_RTOS_PORT_TIMER)

    Timer_node::Timer_node (Double_list& lst, clock::timestamp_t ts, Timer& tm) :
        Timestamp_node
          { lst, ts }, //
        timer (tm)
    {
      ;
    }

    Timer_node::~Timer_node ()
    {
      ;
    }

    /**
     * @details
     * Remove the node from the list and perform the timer actions.
     */
    void
    Timer_node::action (void)
    {
      ((Clock_timestamps_list&) list).remove (*this);
      timer.interrupt_service_routine ();
    }

#endif

    // ========================================================================

    /**
     * @details
     * The list is kept in ascending time stamp order.
     *
     * Based on time stamp, the node is inserted
     * - at the end of the list,
     * - at the beginning of the list,
     * - in the middle of the list, which
     * requires a partial list traversal (done from the end).
     *
     * If the list is empty, the new node is added with references to
     * itself, to satisfy the circular double linked list requirements.
     *
     */
    void
    Clock_timestamps_list::add (Timestamp_node& node)
    {
      if (head_ == nullptr)
        {
          // Make the node point to itself, to satisfy the double
          // linked circular list.
          node.prev = &node;
          node.next = &node;

#if defined(OS_TRACE_RTOS_LISTS)
          trace::printf ("%s() %u \n", __func__, (uint32_t) node.timestamp);
#endif

          head_ = &node;
          count_ = 1;
        }
      else
        {
          clock::timestamp_t timestamp = node.timestamp;

          Timeout_thread_node* after = (Timeout_thread_node*) (head_->prev);

          if (timestamp >= after->timestamp)
            {
              // Insert at the end of the list.
            }
          else if (timestamp < head ()->timestamp)
            {
              // Insert at the beginning of the list
              // and update the new head.
              head_ = &node;
#if defined(OS_TRACE_RTOS_LISTS)
              trace::printf ("%s() head \n", __func__);
#endif
            }
          else
            {
              // Insert in the middle of the list.
              // The loop is guaranteed to terminate.
              while (timestamp < after->timestamp)
                {
                  after = (Timeout_thread_node*) (after->prev);
                }
            }

          ++count_;

#if defined(OS_TRACE_RTOS_LISTS)
          trace::printf ("%s() %u after %u #%d\n", __func__,
                         (uint32_t) timestamp, (uint32_t) after->timestamp,
                         count_);
#endif

          assert(after->timestamp != 0);

          // Make the new node point to its neighbours.
          node.prev = after;
          node.next = after->next;

          // Make the neighbours point to the node. The order is important.
          after->next->prev = &node;
          after->next = &node;
        }
    }

    /**
     * @details
     * With the list ordered, check if the list head time stamp was
     * reached and run the node action.
     *
     * Repeat for all nodes that have overdue time stamps.
     */
    void
    Clock_timestamps_list::check_timestamp (clock::timestamp_t now)
    {
      // Multiple threads can wait for the same time stamp, so
      // iterate until a node with future time stamp is identified.
      for (; !empty ();)
        {
          clock::timestamp_t head_ts = ((Timeout_thread_node*) head_)->timestamp;
          if (now >= head_ts)
            {
#if defined(OS_TRACE_RTOS_LISTS)
              trace::printf ("%s() %u \n", __func__,
                             (uint32_t) systick_clock.now ());
#endif
              head ()->action ();
            }
          else
            {
              break;
            }
        }
    }

  // ----------------------------------------------------------------------

  } /* namespace rtos */
} /* namespace os */