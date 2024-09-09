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
class dep_map {
  hai::varray<dd> m_bucket { 1024000 };
  hashley::rowan m_deps {};

public:
  void take(jute::view grp, jute::view art, jute::view ver, unsigned depth) {
    auto id = (grp + ":" + art).cstr();
    auto & idx = m_deps[id.begin()];
    if (idx == 0) {
      m_bucket.push_back(dd { grp.cstr(), art.cstr(), ver.cstr(), depth });
      idx = m_bucket.size();
      return;
    }

    auto & dd = m_bucket[idx - 1];
    if (dd.depth <= depth) return;

    dd.ver = ver.cstr();
    dd.depth = depth;
  }

  [[nodiscard]] jute::view version_of(jute::view grp, jute::view art) const {
    auto id = (grp + ":" + art).cstr();
    auto idx = m_deps[id.begin()];
    if (idx == 0) return "TBD";
    return m_bucket[idx - 1].ver;
  }

  [[nodiscard]] auto begin() const { return m_bucket.begin(); }
  [[nodiscard]] auto end() const { return m_bucket.end(); }
};

static dep_map g_dep_map {};
static dep_map g_dep_mgmt_map {};

static void parse_parent(cavan::pom * n, unsigned depth) {
  for (auto & d : n->deps) g_dep_map.take(d.grp, d.art, d.ver, depth);
  for (auto & d : n->deps_mgmt) g_dep_mgmt_map.take(d.grp, d.art, d.ver, depth);

  auto & [grp, art, ver] = n->parent;
  if (!grp.size() && !art.size() && !ver.size()) return;

  silog::log(silog::debug, "parsing %s:%s:%s", grp.begin(), art.begin(), ver.begin());

  auto parent = cavan::read_pom(grp, art, ver);
  parse_parent(&parent, depth + 1);
}

static void run(void *, hai::cstr & xml) {
  auto pom = cavan::read_pom(xml);
  parse_parent(&pom, 1);

  for (auto & d : g_dep_map) {
    jute::view ver = (d.ver.size() == 0) ? g_dep_mgmt_map.version_of(d.grp, d.art) : d.ver;
    silog::log(silog::info, "dependency %s:%s:%s", d.grp.begin(), d.art.begin(), ver.cstr().begin());
  }
}

int main(int argc, char ** argv) try {
  for (auto i = 1; i < argc; i++) {
    jojo::read(jute::view::unsafe(argv[i]), nullptr, run);
  }
} catch (...) {
  return 1;
}
