module cavan;
import silog;

namespace {
  class dep_map {
    cavan::deps(cavan::pom::*m_lister);
    cavan::deps * m_list;
    hai::chain<unsigned> m_depths { 1024 };
    hashley::rowan m_idx {};

    void do_pom(cavan::pom * pom, unsigned depth) {
      for (auto & d : pom->*m_lister) {
        auto key = d.grp + ":" + d.art;
        auto & idx = m_idx[key.cstr()];
        if (idx && m_depths.seek(idx - 1) <= depth) continue;

        if (d.scp == "import"_s) {
          if (!d.pom) {
            d.ver = cavan::apply_props(pom, d.ver);
            d.pom = cavan::read_pom(d.grp, d.art, *d.ver);
          }
          do_pom(&*d.pom, depth + 1);
          continue;
        }

        m_list->push_back(d);
        m_depths.push_back(depth);
        idx = m_depths.size();
      }

      if (!pom->ppom) return;
      do_pom(&*pom->ppom, depth + 1);
    }

  public:
    explicit dep_map(cavan::pom * pom, cavan::deps(cavan::pom::*list)) {
      m_lister = list;
      m_list = &(pom->*list);
      for (auto & d : *m_list) {
        m_depths.push_back(1);

        auto key = d.grp + ":" + d.art;
        m_idx[key.cstr()] = m_depths.size();
      }

      do_pom(pom, 1);
    }

    [[nodiscard]] auto ver_of(const cavan::dep & d) const {
      auto key = d.grp + ":" + d.art;
      auto idx = m_idx[key.cstr()];
      if (idx == 0) return d.ver;
      return m_list->seek(idx - 1).ver;
    }
  };
} // namespace

void cavan::eff_pom(cavan::pom * pom) try {
  cavan::read_parent_chain(pom);
  cavan::merge_props(pom);

  dep_map dm { pom, &cavan::pom::deps_mgmt };
  dep_map { pom, &cavan::pom::deps };

  for (auto & d : pom->deps) {
    if (d.ver.size() == 0) d.ver = dm.ver_of(d);
    d.ver = cavan::apply_props(pom, d.ver);
  }
} catch (...) {
  cavan::whilst("calculating effective pom of " + pom->grp + ":" + pom->art + ":" + pom->ver);
}
