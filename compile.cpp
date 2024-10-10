#pragma leco tool
#include <stdlib.h>
#include <string.h>

import cavan;
import hai;
import hashley;
import jojo;
import jute;
import silog;

using namespace jute::literals;

static const jute::heap m2repo { jute::view::unsafe(getenv("HOME")) + "/.m2/repository" };

class context {
  cavan::deps * m_deps {};
  hai::fn<bool, cavan::dep &> m_excl = [](auto &) { return false; };
  hai::fn<const cavan::dep &, const cavan::dep &> m_dm = [](auto & d) -> auto & { return d; };

  void add_dep(const cavan::dep & d, unsigned depth) try {
    auto dpom = cavan::read_pom(*d.grp, d.art, *d.ver);
    cavan::eff_pom(dpom);

    m_deps->push_back(d, depth);

    hashley::niamh exc { 27 };
    if (d.exc) {
      for (auto [g, a] : *d.exc) exc[(g + ":" + a).cstr()] = 1;
    }

    context ctx { m_deps };
    ctx.m_excl = [&](auto & d) {
      auto key = d.grp + ":" + d.art;
      if (exc[*key]) return true;
      return m_excl(d);
    };
    ctx.m_dm = [&](auto & d) -> auto & {
      if (dpom->deps_mgmt.has(d)) return m_dm(dpom->deps_mgmt[d].dep);
      return m_dm(d);
    };
    ctx.add_deps(dpom, false);
  } catch (...) {
    cavan::whilst("processing dependency " + *d.grp + ":" + d.art + ":" + *d.ver);
  }

  void add_deps(cavan::pom * pom, bool test_scope) {
    for (auto & [d, depth] : pom->deps) {
      if (m_deps->has(d)) continue;
      if (m_excl(d)) continue;
      if (d.opt) continue;
      if (d.cls != "jar"_s && d.cls != ""_s) continue;
      if (test_scope && d.scp != "test"_s && d.scp != "compile"_s) continue;
      if (!test_scope && d.scp != "compile"_s) continue;
      add_dep(m_dm(d), depth);
    }
  }

public:
  explicit constexpr context(cavan::deps * ds) : m_deps { ds } {}

  void add_pom(cavan::pom * pom, bool test_scope) {
    m_dm = [&](auto & d) -> auto & {
      if (pom->deps_mgmt.has(d)) return pom->deps_mgmt[d].dep;
      return d;
    };
    add_deps(pom, test_scope);
  }
};

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

static int compile(jute::view src, bool test_scope) {
  auto base = cavan::infer_base_folder(src);
  auto pom_file = (base + "/pom.xml").cstr();
  silog::trace("processing", pom_file);

  auto tgt = base + "/target/classes";
  auto tst_tgt = base + "/target/test-classes";
  auto tmpnam = (base + "/target/cavan-compile.args").cstr();

  auto pom = cavan::read_pom(pom_file);
  cavan::eff_pom(pom);

  for (auto m : pom->ppom->modules) {
    auto [dir, fn] = jute::view { pom->ppom->filename }.rsplit('/');
    auto cpom = dir + "/" + m + "/pom.xml";
    try {
      auto pom = cavan::read_pom(cpom.cstr());
      cavan::eff_pom(pom);
    } catch (...) {
      cavan::whilst("reading module " + m);
    }
  }

  jojo::write(tmpnam, "-d "_hs + (test_scope ? tst_tgt : tgt) + "\n");
  jojo::append(tmpnam, "-cp "_hs + tgt);
  if (test_scope) jojo::append(tmpnam, ":"_hs + tst_tgt);

  cavan::deps deps {};
  context { &deps }.add_pom(pom, test_scope);
  for (auto & [d, _] : deps) output_dep(tmpnam, d);

  jojo::append(tmpnam, "\n"_hs);

  silog::trace("compiling", tmpnam);
  auto cmd = ("javac @"_s + tmpnam + " " + src).cstr();
  return system(cmd.begin());
}

int main(int argc, char ** argv) try {
  if (argc != 2) cavan::fail("usage: compile.exe <java-file>");

  char fname[10240] {};
  realpath(argv[1], fname);

  bool test_scope = strstr(fname, "src/test/");

  return compile(jute::view::unsafe(fname), test_scope);
} catch (...) {
  return 1;
}
