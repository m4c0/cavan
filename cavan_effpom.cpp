module cavan;
import silog;

namespace {
  class context {
    cavan::depmap m_dep_map {};
    cavan::depmap m_dep_mgmt_map {};
    cavan::propmap m_props {};

    void parse_parent(const cavan::pom * n, unsigned depth) {
      for (auto & [k, v] : n->props) m_props.take(k, v, depth);
      for (auto & d : n->deps) m_dep_map.take(d.grp, d.art, d.ver, depth);

      for (auto & d : n->deps_mgmt) {
        if (d.scp == "import"_s) {
          auto ver = m_props.apply(d.ver);
          // silog::log(silog::debug, "importing %s:%s:%s", d.grp.begin(), d.art.begin(), ver.begin());

          auto pom = cavan::read_pom(d.grp, d.art, ver);
          parse_parent(&pom, depth + 1);
        } else {
          m_dep_mgmt_map.take(d.grp, d.art, d.ver, depth);
        }
      }

      auto & [grp, art, ver] = n->parent;
      if (!grp.size() && !art.size() && !ver.size()) return;

      // silog::log(silog::debug, "parsing %s:%s:%s", grp.begin(), art.begin(), ver.begin());

      auto parent = cavan::read_pom(grp, art, ver);
      parse_parent(&parent, depth + 1);
    }

  public:
    void run(const cavan::pom & pom) {
      parse_parent(&pom, 1);

      for (auto & [k, v, _] : m_dep_map) {
        auto [grp, art] = jute::view { k }.split(':');

        jute::view vs = (v.size() == 0) ? m_dep_mgmt_map[k] : v;
        auto ver = m_props.apply(vs);

        silog::log(silog::info, "dependency %s:%s", k.begin(), ver.begin());
      }
    }
  };
} // namespace

void cavan::eff_pom(const cavan::pom & p) {
  context c {};
  c.run(p);
}
