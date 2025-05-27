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

static auto ctx_check_ver(q_node * ctx, const cavan::dep & d, jute::heap ver) {
  if (!ctx) return ver;

  auto & dm = ctx->pom->deps_mgmt;
  if (dm.has(d)) ver = dm[d].dep.ver;

  return ctx_check_ver(ctx->ctx, d, ver);
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

    for (auto &[d, _]: pom->deps) {
      if (d.opt) continue;
      if (d.cls != "jar" && d.cls != "") continue;
      if (d.scp != "compile") continue;

      if (r_has(&r, *d.grp, d.art)) continue;

      // TODO: pull scope, exclusion along with version
      auto ver = ctx_check_ver(n, d, d.ver);

      // TODO: exclusions
      auto dpom = cavan::read_pom(*d.grp, d.art, *ver);
      q_enqueue(&q, dpom, n);
      r_add(&r, *d.grp, d.art);
    }
  }
} catch (...) {
  return 3;
}
