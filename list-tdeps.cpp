#pragma leco tool

import cavan;
import hai;
import hashley;
import jojo;
import jute;
import missingno;
import silog;

using namespace cavan;
using namespace jute::literals;

struct node {
  cavan::dep dep;
  cavan::dep parent;
  cavan::pom * owner;
};
class output {
  hai::chain<node> m_nodes { 1024 };
  hashley::niamh m_idx { 1103 };

public:
  void push_back(node n) {
    auto key = n.dep.grp + ":" + n.dep.art;
    auto & i = m_idx[*key];
    if (i) return;
    m_nodes.push_back(n);
    i = m_nodes.size();
  }

  [[nodiscard]] auto operator[](cavan::dep d) const {
    auto key = d.grp + ":" + d.art;
    auto i = m_idx[*key];
    return i ? m_nodes.seek(i) : node {};
  }

  auto begin() { return m_nodes.begin(); }
  auto end() { return m_nodes.end(); }
};

static void add_deps(output & out, cavan::pom * pom, cavan::dep parent, bool test_scope) {
  cavan::eff_pom(pom);

  for (auto & [d, _] : pom->deps) {
    if (d.opt) continue;
    if (d.cls != "jar"_s && d.cls != ""_s) continue;
    if (test_scope && d.scp != "test"_s && d.scp != "compile"_s && d.scp != "provided") continue;
    if (!test_scope && d.scp != "compile"_s && d.scp != "provided") continue;

    out.push_back({ d, parent, pom });
  }
}

static void run(jute::view fname) {
  output out {};

  auto pom = cavan::read_pom(fname);
  add_deps(out, pom, {}, true);

  for (auto & [d, _, owner] : out) try {
      auto dpom = cavan::read_pom(*d.grp, d.art, *d.ver);
      add_deps(out, dpom, d, false);
    } catch (...) {
      while (owner) {
        silog::log(silog::error, "while processing deps of %s", owner->filename.begin());
        auto [_, p, o] = out[d];
        d = p;
        owner = o;
      }
      throw;
    }
}

int main(int argc, char ** argv) try {
  for (auto i = 1; i < argc; i++) {
    run(jute::view::unsafe(argv[i]));
  }
} catch (...) {
  return 1;
}
