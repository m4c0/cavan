#pragma leco tool

import cavan;
import hai;
import hashley;
import jute;
import print;

struct q_node {
  cavan::pom * pom {};
  q_node * next {};
};
struct queue {
  q_node * first {};
  q_node * last {};
};

static auto q_enqueue(queue * q, cavan::pom * pom) {
  auto * n = new q_node { pom };
  if (!q->first) {
    q->first = n;
    q->last = n;
  } else {
    q->last->next = n;
    q->last = n;
  }
}
static auto q_dequeue(queue * q) {
  if (!q->first) throw 0;

  auto n = q->first;
  q->first = q->first->next;
  if (!q->first) q->last = nullptr;
  return hai::uptr { n };
}

struct resolved {
  hashley::niamh deps { 1023 };
};
static void r_add(resolved * r, jute::view grp, jute::view art) {
  auto key = (grp + ":" + art).cstr();
  r->deps[key] = 1;
}
static bool r_has(resolved * r, jute::view grp, jute::view art) {
  auto key = (grp + ":" + art).cstr();
  return r->deps[key] == 1;
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

  resolved r {};
  queue q {};

  q_enqueue(&q, pom);
  r_add(&r, pom->grp, pom->art);

  while (q.first) {
    auto pom = q_dequeue(&q)->pom;
    putln(pom->filename);

    cavan::eff_pom(pom);

    for (auto &[d, _]: pom->deps) {
      if (d.opt) continue;
      if (d.cls != "jar" && d.cls != "") continue;
      if (d.scp != "compile") continue;

      if (r_has(&r, *d.grp, d.art)) continue;

      // TODO: exclusions
      // TODO: check shallower dep mgmt
      auto dpom = cavan::read_pom(*d.grp, d.art, *d.ver);
      q_enqueue(&q, dpom);
      r_add(&r, *d.grp, d.art);
    }
  }
} catch (...) {
  return 3;
}
