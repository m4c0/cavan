#pragma leco tool

import cavan;
import jojo;
import jute;
import hai;
import print;

using namespace jute::literals;

static void run(jute::view fname, jute::view filter) {
  auto pom = cavan::read_pom(fname);
  cavan::eff_pom(pom);

  for (auto & [d, depth] : pom->deps_mgmt) {
    if (!d.art.starts_with(filter)) continue;
    putln("dep mgmt -- ", d.grp, ":", d.art, ":", d.ver, ":", d.scp, ":", d.typ, " depth=", depth);
    if (d.exc)
      for (auto & [g, a] : *d.exc) {
        put("\e[31m");
        putln("    excl -- ", g, ":", a);
        put("\e[0m");
      }
  }
  putln();
  for (auto & [d, depth] : pom->deps) {
    if (!d.art.starts_with(filter)) continue;
    pom->deps_mgmt.manage(&d);
    putln("     dep -- ", d.grp, ":", d.art, ":", d.ver, ":", d.scp, ":", d.typ, " depth=", depth);
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
  if (argc == 2) {
    putln("reading ", argv[1]);
    run(jute::view::unsafe(argv[1]), "");
  } else if (argc == 3) {
    putln("reading ", argv[1], " filter by [", argv[2], "]");
    run(jute::view::unsafe(argv[1]), jute::view::unsafe(argv[2]));
  } else {
    die("invalid usage");
  }
} catch (...) {
  return 1;
}
