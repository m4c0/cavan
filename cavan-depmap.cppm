export module cavan:depmap;
import :fail;
import :objects;
import hai;
import hashley;
import jute;

namespace cavan {
  export class deps {
    struct pair {
      dep dep {};
      unsigned depth {};
    };

    hai::chain<pair> m_list { 128 };
    hashley::niamh m_idx { 1103 };

  public:
    [[nodiscard]] constexpr auto begin() { return m_list.begin(); }
    [[nodiscard]] constexpr auto end() { return m_list.end(); }
    [[nodiscard]] constexpr auto begin() const { return m_list.begin(); }
    [[nodiscard]] constexpr auto end() const { return m_list.end(); }

    [[nodiscard]] constexpr auto size() const { return m_list.size(); }

    constexpr bool has(const dep & d) const {
      auto key = d.grp + ":" + d.art;
      return 0 != m_idx[*key];
    }
    constexpr auto & operator[](const dep & d) const {
      auto key = d.grp + ":" + d.art;
      auto idx = m_idx[*key];
      if (idx == 0) fail(*key + " not found");
      return m_list.seek(idx - 1);
    }

    void push_back(dep d, unsigned depth = 1) {
      auto & idx = m_idx[*(d.grp + ":" + d.art)];

      if (!idx) {
        m_list.push_back({ d, depth });
        idx = m_list.size();
        return;
      }

      auto & pair = m_list.seek(idx - 1);
      if (pair.depth <= depth) return;

      pair.dep = d;
    }

    void replace_grp_in_key(dep * d, jute::heap grp) {
      auto & idx = m_idx[*(d->grp + ":" + d->art)];

      d->grp = grp;
      m_idx[*(d->grp + ":" + d->art)] = idx;

      idx = 0;
    }

    void manage(dep * d) const {
      auto idx = m_idx[*(d->grp + ":" + d->art)];
      if (idx == 0) return;
      auto & dm = m_list.seek(idx - 1).dep;

      if (d->scp == "") d->scp = dm.scp;
      if (!d->exc) d->exc = dm.exc;
      if (d->ver.size() == 0) d->ver = dm.ver;
      d->opt |= dm.opt;
    }
  };
} // namespace cavan
