#pragma leco tool

import cavan;
import jute;
import print;

int main(int argc, char ** argv) try {
  const auto shift = [&] { return jute::view::unsafe(argc == 1 ? "" : (--argc, *++argv)); };

  auto file = shift();
  if (file == "") die("missing file");

  auto pom = cavan::read_pom(file);

  for (auto p : cavan::resolve_transitive_deps(pom)) {
    putln(cavan::path_of(p->grp, p->art, p->ver, "jar"));
  }
} catch (...) {
  return 3;
}
