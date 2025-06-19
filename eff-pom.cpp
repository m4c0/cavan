#pragma leco tool

import cavan;
import jojo;
import jute;
import hai;
import print;

using namespace jute::literals;

static void run(jute::view fname) {
  auto pom = cavan::read_pom(fname);
  cavan::eff_pom(pom);

  for (auto & [d, _] : pom->deps_mgmt) {
    putln("dep mgmt -- ", d.grp, ":", d.art, ":", d.ver, ":", d.scp, ":", d.typ);
    if (d.exc)
      for (auto & [g, a] : *d.exc) {
        put("\e[31m");
        putln("    excl -- ", g, ":", a);
        put("\e[0m");
      }
  }
  putln();
  for (auto & [d, _] : pom->deps) {
    pom->deps_mgmt.manage(&d);
    putln("     dep -- ", d.grp, ":", d.art, ":", d.ver, ":", d.scp, ":", d.typ);
    if (d.exc)
      for (auto & [g, a] : *d.exc) {
        put("\e[31m");
        putln("    excl -- ", g, ":", a);
        put("\e[0m");
      }
  }
}

int main(int argc, char ** argv) try {
  cavan::file_reader = jojo::read_cstr;
  for (auto i = 1; i < argc; i++) {
    putln("reading ", argv[i]);
    run(jute::view::unsafe(argv[i]));
  }
} catch (...) {
  return 1;
}
