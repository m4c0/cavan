module cavan;
import silog;

namespace {
  template <typename V> class kvmap {
    struct kv {
      V val {};
      unsigned depth {};
    };

    hai::varray<kv> m_bucket { 1024000 };
    hashley::rowan m_deps {};

  public:
    void take(jute::view key, V val, unsigned depth) {
      auto & idx = m_deps[key];
      if (idx == 0) {
        m_bucket.push_back(kv { val, depth });
        idx = m_bucket.size();
        return;
      }

      auto & dd = m_bucket[idx - 1];
      if (dd.depth <= depth) return;

      dd.val = traits::move(val);
      dd.depth = depth;
    };
    [[nodiscard]] const auto & operator[](jute::view key) const {
      auto idx = m_deps[key];
      if (idx == 0) throw "unresolvable version";
      return m_bucket[idx - 1].val;
    };

    [[nodiscard]] constexpr auto begin() const { return m_bucket.begin(); }
    [[nodiscard]] constexpr auto end() const { return m_bucket.end(); }
    [[nodiscard]] constexpr auto size() const { return m_bucket.size(); }
  };
  class propmap : public kvmap<cavan::prop *> {
  public:
    jute::heap apply(jute::heap str) const {
      for (unsigned i = 0; i < str.size() - 3; i++) {
        if ((*str)[i] != '$') continue;
        if ((*str)[i + 1] != '{') continue;

        unsigned j {};
        for (j = i; j < str.size() && (*str)[j] != '}'; j++) {
        }

        if (j == str.size()) return str;

        jute::view before { str.begin(), i };
        jute::view prop { str.begin() + i + 2, j - i - 2 };
        jute::view after { str.begin() + j + 1, str.size() - j - 1 };

        str = before + (*this)[prop]->val + after;
        i--;
      }
      return str;
    };
  };
  class depmap : kvmap<cavan::dep *> {
  public:
    using kvmap::begin;
    using kvmap::end;
    using kvmap::size;

    void take(cavan::dep * d, unsigned depth) {
      auto key = d->grp + ":" + d->art;
      kvmap::take(key.cstr(), d, depth);
    };
    [[nodiscard]] auto * dep_of(jute::view grp, jute::view art) const {
      auto key = grp + ":" + art;
      return kvmap::operator[](key.cstr());
    };
  };

  class context {
    depmap m_dep_map {};
    depmap m_dep_mgmt_map {};
    propmap m_props {};

    void parse_parent(cavan::pom * n, unsigned depth) {
      for (auto & d : n->props) m_props.take(d.key, &d, depth);
      for (auto & d : n->deps) m_dep_map.take(&d, depth);

      for (auto & d : n->deps_mgmt) {
        if (d.scp == "import"_s) {
          if (!d.pom) {
            auto ver = m_props.apply(d.ver);
            // silog::log(silog::debug, "importing %s:%s:%s", d.grp.begin(), d.art.begin(), ver.begin());
            d.pom.reset(new cavan::pom { cavan::read_pom(d.grp, d.art, *ver) });
          }
          parse_parent(&*d.pom, depth + 1);
        } else {
          m_dep_mgmt_map.take(&d, depth);
        }
      }

      if (!n->ppom) return;
      parse_parent(&*n->ppom, depth + 1);
    }

  public:
    void run(cavan::pom * pom) {
      parse_parent(pom, 1);

      for (auto & [d, _] : m_dep_map) {
        auto vs = (d->ver.size() == 0) ? m_dep_mgmt_map.dep_of(d->grp, d->art)->ver : d->ver;
        auto ver = m_props.apply(vs);

        silog::log(silog::info, "dependency %s:%s:%s", d->grp.cstr().begin(), d->art.cstr().begin(),
                   (*ver).cstr().begin());
      }
    }
  };
} // namespace

void cavan::eff_pom(cavan::pom * p) {
  context c {};
  c.run(p);
}
