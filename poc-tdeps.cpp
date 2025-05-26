#pragma leco tool

import cavan;
import hai;
import jute;
import print;
import traits;

struct q_data {
  cavan::pom * pom;
};
struct q_node : q_data {
  hai::uptr<q_node> next {};
};
struct queue {
  hai::uptr<q_node> first {};
  q_node * last {};
};

static auto q_enqueue(queue * q, cavan::pom * pom) {
  auto n = hai::uptr<q_node>::make(q_data { pom });
  auto * ptr = &*n;
  if (q->last) {
    q->last->next = traits::move(n);
  } else {
    q->first = traits::move(n);
  }
  q->last = ptr;
}
static auto q_dequeue(queue * q) {
  q_data data = *(q->first);
  q->first = traits::move(q->first->next);
  if (!q->first) q->last = nullptr;
  return data;
}

static void preload_modules(cavan::pom * pom) {
  auto _ = cavan::read_modules(pom);
  if (pom->ppom) preload_modules(pom->ppom);
}

int main(int argc, char ** argv) try {
  const auto shift = [&] { return jute::view::unsafe(argc == 1 ? "" : (--argc, *++argv)); };

  auto file = shift();
  if (file == "") die("missing file");

  auto pom = cavan::read_pom(file);
  cavan::read_parent_chain(pom);
  preload_modules(pom);

  queue q {};
  q_enqueue(&q, pom);

  while (q.last) {
    auto [pom] = q_dequeue(&q);
    putln(pom->filename);
  }
} catch (...) {
  return 3;
}
