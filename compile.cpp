#pragma leco tool
#include <stdlib.h>
#include <string.h>

import cavan;
import jojo;
import jute;
import mtime;

using namespace jute::literals;

static auto infer_base_folder(jute::view src) {
  while (src != "") {
    auto [l, r] = src.rsplit('/');
    if (r == "src") return l;
    src = l;
  }
  cavan::fail("file not in maven repo");
}

static int compile(char * fname) {
  auto src = jute::view::unsafe(fname);
  auto base = infer_base_folder(src);
  auto tgt = base + "/target/classes";
  auto pom_file = (base + "/pom.xml").cstr();
  auto tmpnam = (base + "/target/cavan-compile.args").cstr();

  auto pom = cavan::read_pom(jojo::read_cstr(pom_file));
  cavan::eff_pom(&pom);

  jojo::write(tmpnam, "-d "_hs + tgt + "\n");
  jojo::append(tmpnam, "-cp "_hs + tgt);

  bool test_scope = strstr(fname, "src/test/");

  jute::heap m2repo { jute::view::unsafe(getenv("HOME")) + "/.m2/repository" };

  for (auto & d : pom.deps) {
    if (d.cls != "jar"_s && d.cls != ""_s) continue;
    if (test_scope && d.scp != "test"_s && d.scp != "compile"_s) continue;
    if (!test_scope && d.scp != "compile"_s) continue;

    // TODO: traverse deps

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

  jojo::append(tmpnam, "\n"_hs);
  jojo::append(tmpnam, src);

  auto cmd = ("javac @"_s + tmpnam).cstr();
  return system(cmd.begin());
}

int main(int argc, char ** argv) try {
  if (argc != 2) cavan::fail("usage: compile.exe <java-file>");

  jojo::on_error([](void *, jute::view msg) {
    silog::log(silog::error, "IO error: %s", msg.cstr().begin());
    struct io_error {};
    throw io_error {};
  });

  return compile(argv[1]);
} catch (...) {
  return 1;
}
