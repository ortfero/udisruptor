# udisruptor
C++ approach to LMAX disruptor

## Snippet

```cpp
#include <stdio.h>
#include <udisruptor/ring_buffer.hpp>
#include <udisruptor/multisequencer.hpp>
#include <udisruptor/sequencer.hpp>

constexpr auto events_count = 10000000;
constexpr auto buffer_size = 100000;

int main() {
  udisruptor::ring_buffer<int64_t> buffer{buffer_size};
  udisruptor::sequencer sequencer{buffer.capacity()};

  auto const consumer = [&](udisruptor::sequence* consumer_seq) {
    auto events_consumed = 0;
    while(events_consumed != events_count) {
      auto const next = consumer_seq->next();
      auto const until = sequencer.try_fetch_all(next);
      if(until == next) {
        std::this_thread::sleep_for(std::chrono::microseconds{20});
        continue;
      }
      for(auto i = next; i != until; ++i) {
        auto const value = buffer[i];
        if(value != i)
          puts("Oops");
        ++events_consumed;
      }
      *consumer_seq = until - 1;
    }
  };
  
  auto worker = std::thread{consumer, sequencer.add_consumer()};
  
  if(!buffer || !sequencer) {
    printf("Disruptor is not ready\n");
    abort();
  }
  
  // producing
  for(auto i = 0; i != events_count; ++i) {
    auto const index = sequencer.claim();
    buffer[index] = index;
    sequencer.publish(index);
  }
  
  worker.join();

  return 0;
}
```

## Benchmarks

### Microbenchmarks

| Operation                               | Time (ns) |
|-----------------------------------------|----------:|
| sequencer: claim/publish/try_fetch      |        7  |
| multisequencer: claim/publish/try_fetch |       20  |


### Throughtput, millions events per second

| Sequencer            | Producers | Throughput |
|----------------------|----------:|------------|
| ```sequencer```      |         1 |       65.9 |
| ```multisequencer``` |         1 |       48.1 |
| ```multisequencer``` |         2 |       12.7 |
| ```multisequencer``` |         4 |       12.4 |
| ```multisequencer``` |         8 |       14.5 |

*Intel(R) Core(TM) i7-10510U CPU @ 1.80GHz (8 cores)*
