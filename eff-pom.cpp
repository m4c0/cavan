#pragma leco tool

import cavan;
import jojo;
import jute;
import hai;
import hashley;
import silog;

class kvmap {
  struct kv {
    hai::cstr key {};
    hai::cstr val {};
    unsigned depth {};
  };

  hai::varray<kv> m_bucket { 1024000 };
  hashley::rowan m_deps {};

public:
  void take(jute::view key, jute::view val, unsigned depth) {
    auto & idx = m_deps[key.cstr().begin()];
    if (idx == 0) {
      m_bucket.push_back(kv { key.cstr(), val.cstr(), depth });
      idx = m_bucket.size();
      return;
    }

    auto & dd = m_bucket[idx - 1];
    if (dd.depth <= depth) return;

    dd.val = val.cstr();
    dd.depth = depth;
  }

  [[nodiscard]] jute::view operator[](jute::view key) const {
    auto idx = m_deps[key.cstr().begin()];
    if (idx == 0) return "TBD";
    return m_bucket[idx - 1].val;
  }

  [[nodiscard]] auto begin() const { return m_bucket.begin(); }
  [[nodiscard]] auto end() const { return m_bucket.end(); }
};
class dep_map : kvmap {
public:
  using kvmap::begin;
  using kvmap::end;
  using kvmap::operator[];

  void take(jute::view grp, jute::view art, jute::view ver, unsigned depth) {
    auto key = grp + ":" + art;
    kvmap::take(key.cstr(), ver, depth);
  }
  [[nodiscard]] jute::view version_of(jute::view grp, jute::view art) const {
    auto key = grp + ":" + art;
    return kvmap::operator[](key.cstr());
  }
};

static dep_map g_dep_map {};
static dep_map g_dep_mgmt_map {};
static kvmap g_props {};

static void parse_parent(cavan::pom * n, unsigned depth) {
  for (auto & d : n->deps) g_dep_map.take(d.grp, d.art, d.ver, depth);
  for (auto & d : n->deps_mgmt) g_dep_mgmt_map.take(d.grp, d.art, d.ver, depth);
  for (auto & [k, v] : n->props) g_props.take(k, v, depth);

  auto & [grp, art, ver] = n->parent;
  if (!grp.size() && !art.size() && !ver.size()) return;

  silog::log(silog::debug, "parsing %s:%s:%s", grp.begin(), art.begin(), ver.begin());

  auto parent = cavan::read_pom(grp, art, ver);
  parse_parent(&parent, depth + 1);
}

static void run(void *, hai::cstr & xml) {
  auto pom = cavan::read_pom(xml);
  parse_parent(&pom, 1);

  for (auto & [k, v, _] : g_dep_map) {
    jute::view ver = (v.size() == 0) ? g_dep_mgmt_map[k] : v;
    if (ver.size() > 3 && ver[0] == '$' && ver[1] == '{' && ver[ver.size() - 1] == '}') {
      auto prop = ver.subview(2, ver.size() - 3).middle;
      ver = g_props[prop];
    }

    silog::log(silog::info, "dependency %s:%s", k.begin(), ver.cstr().begin());
  }
}

int main(int argc, char ** argv) try {
  for (auto i = 1; i < argc; i++) {
    jojo::read(jute::view::unsafe(argv[i]), nullptr, run);
  }
} catch (...) {
  return 1;
}
