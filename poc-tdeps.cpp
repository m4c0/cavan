#pragma leco tool

import cavan;
import hai;
import hashley;
import jute;
import print;

struct q_node {
  cavan::pom * pom {};
  q_node * ctx {};
  q_node * next {};
};
struct queue {
  q_node * first {};
  q_node * last {};
};

static auto q_enqueue(queue * q, cavan::pom * pom, q_node * ctx) {
  auto * n = new q_node { pom, ctx };
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
  return n;
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

static const cavan::deps * ctx_owner(q_node * ctx, const cavan::dep & d) {
  if (!ctx) return {};

  auto * dm = ctx_owner(ctx->ctx, d);
  if (dm) return dm;

  dm = &ctx->pom->deps_mgmt;
  return dm->has(d) ? dm : nullptr;
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

  q_enqueue(&q, pom, nullptr);
  r_add(&r, pom->grp, pom->art);

  while (q.first) {
    auto n = q_dequeue(&q);
    auto pom = n->pom;
    putln(pom->filename);

    cavan::eff_pom(pom);

    for (auto [d, _]: pom->deps) {
      if (r_has(&r, *d.grp, d.art)) continue;

      auto * dm = ctx_owner(n, d);
      if (dm) {
        auto ver = (*dm)[d].dep.ver;
        dm->manage(&d);
        d.ver = ver;
      }

      if (d.opt) continue;
      if (d.cls != "jar" && d.cls != "") continue;
      if (d.scp != "compile") continue;

      // TODO: exclusions
      auto dpom = cavan::read_pom(*d.grp, d.art, *d.ver);
      q_enqueue(&q, dpom, n);
      r_add(&r, *d.grp, d.art);
    }
  }
} catch (...) {
  return 3;
}
