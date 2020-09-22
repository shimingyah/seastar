#pragma once

#include <pthread.h>
#include <atomic>
#include <functional>
#include <list>
#include <map>

#include <boost/lockfree/queue.hpp>

#include "seastar/core/circular_buffer.hh"
#include "seastar/core/iostream.hh"
#include "seastar/core/temporary_buffer.hh"

namespace seastar {

class reactor;
class channel;

struct user_packet {
  // std::vector<net::fragment> _fragments;
  temporary_buffer<char> _buf;
  std::function<void()> _done;
  channel* _channel;
};

using tls_queue = boost::lockfree::queue<user_packet*>;
using tls_queue_list = std::vector<tls_queue*>;

class packet_queue {
 public:
  explicit packet_queue(reactor* r);
  virtual ~packet_queue();

  bool enqueue(user_packet* pack);
  bool dequeue_bulk(user_packet** p);

 private:
  tls_queue* impl();
  void dequeue_tls_queues();

 private:
  pthread_key_t tls_queue_key;
  tls_queue_list _queues;
  std::atomic<size_t> _index;
  reactor* _reactor;
};

}  // namespace seastar
