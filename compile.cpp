#pragma leco tool
#include <stdlib.h>
#include <string.h>

import cavan;
import hai;
import hashley;
import jojo;
import jute;

using namespace jute::literals;

static const jute::heap m2repo { jute::view::unsafe(getenv("HOME")) + "/.m2/repository" };

class context {
  cavan::deps * m_deps {};
  hai::fn<bool, cavan::dep &> m_excl = [](auto &) { return false; };

  void add_dep(const cavan::dep & d, unsigned depth, bool test_scope) try {
    if (d.cls != "jar"_s && d.cls != ""_s) return;
    if (test_scope && d.scp != "test"_s && d.scp != "compile"_s) return;
    if (!test_scope && d.scp != "compile"_s) return;

    auto dpom = cavan::read_pom(d.grp, d.art, *d.ver);
    cavan::eff_pom(dpom);

    m_deps->push_back(d, depth);

    return;

    hashley::niamh exc { 27 };
    if (d.exc) {
      for (auto [g, a] : *d.exc) exc[(g + ":" + a).cstr()] = 1;
    }

    context ctx { m_deps };
    ctx.m_excl = [&](auto & d) {
      auto key = (d.grp + ":" + d.art).cstr();
      if (exc[key]) return true;
      return m_excl(d);
    };
    ctx.add_deps(dpom, false);
  } catch (...) {
    cavan::whilst("processing dependency " + d.grp + ":" + d.art + ":" + *d.ver);
  }

public:
  explicit constexpr context(cavan::deps * ds) : m_deps { ds } {}

  void add_deps(cavan::pom * pom, bool test_scope) {
    for (auto & [d, depth] : pom->deps) {
      if (m_excl(d)) continue;
      add_dep(d, depth, test_scope);
    }
  }
};

static auto infer_base_folder(jute::view src) {
  while (src != "") {
    auto [l, r] = src.rsplit('/');
    if (r == "src") return l;
    src = l;
  }
  cavan::fail("file not in maven repo");
}

static void output_dep(const auto & tmpnam, const cavan::dep & d) {
  auto dpom = cavan::read_pom(d.grp, d.art, *d.ver);
  jute::view dpom_fn { dpom->filename };
  if (!dpom_fn.starts_with(*m2repo)) {
    auto [dir, fn] = dpom_fn.rsplit('/');
    jojo::append(tmpnam, ":"_hs + dir + "/target/classes");
    return;
  }

  auto jar = m2repo;
  auto grp = d.grp;
  while (grp.size() > 0) {
    auto [l, r] = grp.split('.');
    jar = jar + "/" + l;
    grp = r;
  }
  jar = jar + "/" + d.art + "/" + d.ver + "/" + d.art + "-" + d.ver + "." + d.typ;

  jojo::append(tmpnam, ":"_hs + jar);
}

static int compile(char * fname) {
  bool test_scope = strstr(fname, "src/test/");

  auto src = jute::view::unsafe(fname);
  auto base = infer_base_folder(src);
  auto tgt = base + "/target/classes";
  auto tst_tgt = base + "/target/test-classes";
  auto pom_file = (base + "/pom.xml").cstr();
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
  context { &deps }.add_deps(pom, test_scope);
  for (auto & [d, _] : deps) output_dep(tmpnam, d);

  jojo::append(tmpnam, "\n"_hs);
  jojo::append(tmpnam, src);

  auto cmd = ("javac @"_s + tmpnam).cstr();
  return system(cmd.begin());
}

int main(int argc, char ** argv) try {
  if (argc != 2) cavan::fail("usage: compile.exe <java-file>");

  char buffer[10240] {};
  realpath(argv[1], buffer);

  return compile(buffer);
} catch (...) {
  return 1;
}
