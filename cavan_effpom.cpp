module cavan;
import silog;

namespace {
  template <typename V> class kvmap {
    struct kv {
      hai::cstr key {};
      V val {};
      unsigned depth {};
    };

    hai::varray<kv> m_bucket { 1024000 };
    hashley::rowan m_deps {};

  public:
    void take(jute::view key, V val, unsigned depth) {
      auto & idx = m_deps[key];
      if (idx == 0) {
        m_bucket.push_back(kv { key.cstr(), traits::move(val), depth });
        idx = m_bucket.size();
        return;
      }

      auto & dd = m_bucket[idx - 1];
      if (dd.depth <= depth) return;

      dd.val = traits::move(val);
      dd.depth = depth;
    };
    [[nodiscard]] jute::view operator[](jute::view key) const {
      auto idx = m_deps[key];
      if (idx == 0) return "TBD";
      return m_bucket[idx - 1].val;
    };

    [[nodiscard]] constexpr auto begin() const { return m_bucket.begin(); }
    [[nodiscard]] constexpr auto end() const { return m_bucket.end(); }
  };
  class propmap : public kvmap<hai::cstr> {
  public:
    hai::cstr apply(jute::view str) const {
      hai::cstr res = str.cstr();
      for (unsigned i = 0; i < str.size() - 3; i++) {
        if (str[i] != '$') continue;
        if (str[i + 1] != '{') continue;

        unsigned j {};
        for (j = i; j < str.size() && str[j] != '}'; j++) {
        }

        if (j == str.size()) return res;

        jute::view before { str.begin(), i };
        jute::view prop { str.begin() + i + 2, j - i - 2 };
        jute::view after { str.begin() + j + 1, str.size() - j - 1 };

        auto concat = before + (*this)[prop] + after;
        res = concat.cstr();
        str = res;
        i--;
      }
      return res;
    };
  };
  class depmap : kvmap<hai::cstr> {
  public:
    using kvmap::begin;
    using kvmap::end;
    using kvmap::operator[];

    void take(jute::view grp, jute::view art, jute::view ver, unsigned depth) {
      auto key = grp + ":" + art;
      kvmap::take(key.cstr(), ver.cstr(), depth);
    };
    [[nodiscard]] jute::view version_of(jute::view grp, jute::view art) const {
      auto key = grp + ":" + art;
      return kvmap::operator[](key.cstr());
    };
  };

  class context {
    depmap m_dep_map {};
    depmap m_dep_mgmt_map {};
    propmap m_props {};

    void parse_parent(const cavan::pom * n, unsigned depth) {
      for (auto & [k, v] : n->props) m_props.take(k, v.cstr(), depth);
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
