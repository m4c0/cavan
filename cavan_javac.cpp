module;
#include <stdlib.h>

module cavan;
import jojo;

static const jute::heap m2repo { jute::view::unsafe(getenv("HOME")) + "/.m2/repository" };

namespace {
  class context {
    cavan::deps * m_deps {};
    hai::fn<bool, cavan::dep &> m_excl = [](auto &) { return false; };
    hai::fn<void, cavan::dep *> m_dm = [](auto) {};
    bool m_recurse {};

    void add_dep(const cavan::dep & d, unsigned depth) try {
      auto dpom = cavan::read_pom(*d.grp, d.art, *d.ver);
      cavan::eff_pom(dpom);

      m_deps->push_back(d, depth);

      hashley::niamh exc { 27 };
      if (d.exc) {
        for (auto [g, a] : *d.exc) exc[(g + ":" + a).cstr()] = 1;
      }

      if (!m_recurse) return;

      context ctx { m_deps, true };
      ctx.m_excl = [&](auto & d) {
        auto key = d.grp + ":" + d.art;
        if (exc[*key]) return true;
        return m_excl(d);
      };
      ctx.m_dm = [&](auto * d) {
        dpom->deps_mgmt.manage(d);
        return m_dm(d);
      };
      ctx.add_deps(dpom, false, depth);
    } catch (...) {
      cavan::whilst("processing dependency " + *d.grp + ":" + d.art + ":" + *d.ver);
    }

    void add_deps(cavan::pom * pom, bool test_scope, int pdepth) {
      for (auto [d, depth] : pom->deps) {
        if (d.opt) continue;
        if (d.cls != "jar"_s && d.cls != ""_s) continue;
        if (test_scope && d.scp != "test"_s && d.scp != "compile"_s && d.scp != "provided") continue;
        if (!test_scope && d.scp != "compile"_s && d.scp != "provided") continue;

        if (m_deps->has(d)) continue;
        if (m_excl(d)) continue;
        m_dm(&d);
        add_dep(d, depth + pdepth);
      }
    }

  public:
    explicit constexpr context(cavan::deps * ds, bool recurse) : m_deps { ds }, m_recurse { recurse } {}

    void add_pom(cavan::pom * pom, bool test_scope) {
      m_dm = [&](auto * d) { pom->deps_mgmt.manage(d); };
      add_deps(pom, test_scope, 0);
    }
  };
} // namespace

static void output_dep(const auto & tmpnam, const cavan::dep & d) {
  auto dpom = cavan::read_pom(*d.grp, d.art, *d.ver);
  jute::view dpom_fn { dpom->filename };
  if (!dpom_fn.starts_with(*m2repo)) {
    auto [dir, fn] = dpom_fn.rsplit('/');
    jojo::append(tmpnam, ":"_hs + dir + "/target/classes");
    return;
  }

  auto jar = m2repo;
  auto grp = *d.grp;
  while (grp.size() > 0) {
    auto [l, r] = grp.split('.');
    jar = jar + "/" + l;
    grp = r;
  }
  jar = jar + "/" + d.art + "/" + d.ver + "/" + d.art + "-" + d.ver + "." + d.typ;

  jojo::append(tmpnam, ":"_hs + jar);
}

hai::array<cavan::pom *> cavan::read_modules(const cavan::pom * pom) try {
  hai::array<cavan::pom *> res { pom->modules.size() };
  auto dir = jute::view { pom->filename }.rsplit('/').before;
  for (auto i = 0; i < res.size(); i++) {
    auto cpom = dir + "/" + pom->modules.seek(i) + "/pom.xml";
    try {
      res[i] = cavan::read_pom(cpom.cstr());
    } catch (...) {
      cavan::whilst("reading module " + pom->modules.seek(i));
    }
  }
  return res;
} catch (...) {
  cavan::whilst("reading modules of " + jute::view { pom->filename });
}

hai::cstr cavan::generate_javac_argfile(cavan::pom * pom, bool test_scope, bool recurse) {
  auto base = jute::view { pom->filename }.rsplit('/').before;

  auto tmpnam = (base + "/target/cavan-compile.args").cstr();

  auto tgt = base + "/target/classes";
  auto tst_tgt = base + "/target/test-classes";

  cavan::eff_pom(pom);
  if (pom->ppom)
    for (auto p : cavan::read_modules(pom->ppom)) cavan::eff_pom(p);

  jojo::write(tmpnam, "-d "_hs + (test_scope ? tst_tgt : tgt) + "\n");
  jojo::append(tmpnam, "--release 17\n"_hs);
  jojo::append(tmpnam, "-cp "_hs + tgt);
  if (test_scope) jojo::append(tmpnam, ":"_hs + tst_tgt);

  cavan::deps deps {};
  context { &deps, recurse }.add_pom(pom, test_scope);
  for (auto & [d, _] : deps) output_dep(tmpnam, d);

  jojo::append(tmpnam, "\n"_hs);
  return tmpnam;
}
