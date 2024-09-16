#pragma leco tool

import cavan;
import jojo;
import jute;
import hai;
import hashley;
import silog;

using namespace jute::literals;

static cavan::depmap g_dep_map {};
static cavan::depmap g_dep_mgmt_map {};
static cavan::kvmap g_props {};

static hai::cstr apply_props(jute::view ver) {
  while (ver.size() > 3 && ver[0] == '$' && ver[1] == '{' && ver[ver.size() - 1] == '}') {
    auto prop = ver.subview(2, ver.size() - 3).middle;
    ver = g_props[prop];
  }
  return ver.cstr();
}

static void parse_parent(cavan::pom * n, unsigned depth) {
  for (auto & [k, v] : n->props) g_props.take(k, v, depth);
  for (auto & d : n->deps) g_dep_map.take(d.grp, d.art, d.ver, depth);

  for (auto & d : n->deps_mgmt) {
    if (d.scp == "import"_s) {
      auto ver = apply_props(d.ver);
      silog::log(silog::debug, "importing %s:%s:%s", d.grp.begin(), d.art.begin(), ver.begin());

      auto pom = cavan::read_pom(d.grp, d.art, ver);
      parse_parent(&pom, depth + 1);
    } else {
      g_dep_mgmt_map.take(d.grp, d.art, d.ver, depth);
    }
  }

  auto & [grp, art, ver] = n->parent;
  if (!grp.size() && !art.size() && !ver.size()) return;

  silog::log(silog::debug, "parsing %s:%s:%s", grp.begin(), art.begin(), ver.begin());

  auto parent = cavan::read_pom(grp, art, ver);
  parse_parent(&parent, depth + 1);
}

static void run(hai::cstr xml) {
  auto pom = cavan::read_pom(traits::move(xml));
  parse_parent(&pom, 1);

  for (auto & [k, v, _] : g_dep_map) {
    jute::view vs = (v.size() == 0) ? g_dep_mgmt_map[k] : v;
    auto ver = apply_props(vs);
    silog::log(silog::info, "dependency %s:%s", k.begin(), ver.begin());
  }
}

int main(int argc, char ** argv) try {
  for (auto i = 1; i < argc; i++) {
    run(jojo::read_cstr(jute::view::unsafe(argv[i])));
  }
} catch (...) {
  return 1;
}
