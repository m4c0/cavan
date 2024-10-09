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

static void add_deps(const auto & tmpnam, cavan::pom * pom, bool test_scope, hai::fn<bool, cavan::dep &> excl) try {
  for (auto & [d, _] : pom->deps) {
    if (excl(d)) continue;
    if (d.cls != "jar"_s && d.cls != ""_s) continue;
    if (test_scope && d.scp != "test"_s && d.scp != "compile"_s) continue;
    if (!test_scope && d.scp != "compile"_s) continue;

    auto dpom = cavan::read_pom(d.grp, d.art, *d.ver);
    cavan::eff_pom(dpom);

    jute::view dpom_fn { dpom->filename };
    if (!dpom_fn.starts_with(*m2repo)) {
      auto [dir, fn] = dpom_fn.rsplit('/');
      jojo::append(tmpnam, ":"_hs + dir + "/target/classes");
      continue;
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

    // TODO: dependency recursion?

    if (!d.exc) continue;

    hashley::rowan exc {};
    for (auto [g, a] : *d.exc) exc[(g + ":" + a).cstr()] = 1;
    add_deps(tmpnam, dpom, false, [&exc, &excl](auto & d) {
      if (exc[(d.grp + ":" + d.art).cstr()]) return true;
      return excl(d);
    });
  }
} catch (...) {
  cavan::whilst("processing dependencies of " + pom->grp + ":" + pom->art + ":" + pom->ver);
}

static auto infer_base_folder(jute::view src) {
  while (src != "") {
    auto [l, r] = src.rsplit('/');
    if (r == "src") return l;
    src = l;
  }
  cavan::fail("file not in maven repo");
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

  add_deps(tmpnam, pom, test_scope, [](auto &) { return false; });

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
