#pragma leco tool

import cavan;
import jojo;
import jute;
import hai;
import silog;

static void parse_parent(cavan::pom * n) {
  auto & [grp, art, ver] = n->parent;
  if (!grp.size() && !art.size() && !ver.size()) return;

  silog::log(silog::debug, "parsing %s:%s:%s", grp.begin(), art.begin(), ver.begin());

  auto parent = cavan::read_pom(grp, art, ver);
  parse_parent(&parent);
}

static void run(void *, hai::cstr & xml) {
  auto pom = cavan::read_pom(xml);
  parse_parent(&pom);
}

int main(int argc, char ** argv) try {
  for (auto i = 1; i < argc; i++) {
    jojo::read(jute::view::unsafe(argv[i]), nullptr, run);
  }
} catch (...) {
  return 1;
}
