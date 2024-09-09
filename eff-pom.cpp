#pragma leco tool

import cavan;
import jojo;
import jute;
import hai;
import hashley;
import silog;

struct dd {
  hai::cstr grp {};
  hai::cstr art {};
  hai::cstr ver {};
  unsigned depth {};
};
static hai::varray<dd> g_deps_bucket { 1024000 };
static hashley::rowan g_deps {};

static void parse_parent(cavan::pom * n, unsigned depth) {
  for (auto & d : n->deps) {
    auto id = (jute::view { d.grp } + ":" + d.art).cstr();
    auto & v = g_deps[id.begin()];
    if (v == 0) {
      g_deps_bucket.push_back(dd {
          .grp = jute::view { d.grp }.cstr(),
          .art = jute::view { d.art }.cstr(),
          .ver = jute::view { d.ver }.cstr(),
          .depth = depth,
      });
      v = g_deps_bucket.size();
      continue;
    }
    auto & dd = g_deps_bucket[v - 1];
    if (dd.depth <= depth) continue;

    dd.ver = jute::view { d.ver }.cstr();
    dd.depth = depth;
  }

  auto & [grp, art, ver] = n->parent;
  if (!grp.size() && !art.size() && !ver.size()) return;

  silog::log(silog::debug, "parsing %s:%s:%s", grp.begin(), art.begin(), ver.begin());

  auto parent = cavan::read_pom(grp, art, ver);
  parse_parent(&parent, depth + 1);
}

static void run(void *, hai::cstr & xml) {
  auto pom = cavan::read_pom(xml);
  parse_parent(&pom, 1);

  for (auto & d : g_deps_bucket) {
    silog::log(silog::info, " - %s:%s:%s", d.grp.begin(), d.art.begin(), d.ver.begin());
  }
}

int main(int argc, char ** argv) try {
  for (auto i = 1; i < argc; i++) {
    jojo::read(jute::view::unsafe(argv[i]), nullptr, run);
  }
} catch (...) {
  return 1;
}
