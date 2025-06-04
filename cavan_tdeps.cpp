module cavan;
import hai;
import hashley;
import silog;

struct q_node {
  cavan::pom * pom {};
  hashley::niamh exc { 63 };
  bool test {};
  q_node * ctx {};
  q_node * next {};
  q_node * next_alloc {};
};
struct queue {
  q_node * first {};
  q_node * last {};
};

static class q_allocs {
  q_node * m_ptr;

public:
  ~q_allocs() {
    while (m_ptr) { 
      auto p = m_ptr;
      m_ptr = m_ptr->next_alloc;
      delete p;
    }
  }

  void manage(q_node * n) {
    n->next_alloc = m_ptr;
    m_ptr = n;
  }
} g_q_allocs;

static auto q_enqueue(queue * q, cavan::pom * pom, q_node * ctx) {
  auto * n = new q_node { .pom = pom, .ctx = ctx };
  g_q_allocs.manage(n);

  if (!q->first) {
    q->first = n;
    q->last = n;
  } else {
    q->last->next = n;
    q->last = n;
  }
  return n;
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

static void ctx_manage(q_node * ctx, cavan::dep * d) {
  if (!ctx) return;

  ctx_manage(ctx->ctx, d);

  ctx->pom->deps_mgmt.manage(d);
}
static bool ctx_excl(q_node * ctx, jute::view key) {
  if (!ctx) return false;
  if (ctx->exc[key]) return true;
  return ctx_excl(ctx->ctx, key);
}

hai::chain<cavan::pom *> cavan::resolve_transitive_deps(pom * pom) {
  cavan::read_parent_chain(pom);
  cavan::warm_modules_from_parent_chain(pom);

  resolved r {};
  queue q {};

  q_enqueue(&q, pom, nullptr)->test = true;
  r_add(&r, pom->grp, pom->art);

  hai::chain<cavan::pom *> deps { 10240 };
  while (q.first) {
    auto n = q_dequeue(&q);
    auto pom = n->pom;
    deps.push_back(pom);

    cavan::eff_pom(pom);

    for (auto [d, _]: pom->deps) try {
      if (r_has(&r, *d.grp, d.art)) continue;
 
      if (ctx_excl(n, *(d.grp + ":" + d.art))) continue;

      auto v = d.ver;
      d.ver = {};
      ctx_manage(n, &d);
      if (*d.ver == "") d.ver = v;

      if (d.opt) continue;
      if (d.cls != "jar" && d.cls != "") continue;
      if (d.scp != "" && d.scp != "compile" && d.scp != "provided" && d.scp != "test") continue;

      if (d.scp == "test" && !n->test) continue;
      if (d.scp == "provided" && n->ctx) continue;

      auto dpom = cavan::read_pom(*d.grp, d.art, *d.ver);
      auto dn = q_enqueue(&q, dpom, n);
      if (d.exc) for (auto [g, a]: *d.exc) {
        dn->exc[(g + ":" + a).cstr()] = 1;
      }
      r_add(&r, *d.grp, d.art);
    } catch (...) {
      while (n) {
        silog::log(silog::error, "requested from %s", n->pom->filename.begin());
        n = n->ctx;
      }
      throw;
    }
  }

  return deps;
}
