#pragma leco add_impl cavan_deps
#pragma leco add_impl cavan_effpom
#pragma leco add_impl cavan_lint
#pragma leco add_impl cavan_pom
#pragma leco add_impl cavan_props
#pragma leco add_impl cavan_tokenizer
export module cavan;
export import :fail;
import jute;
import hai;
import hashley;

using namespace jute::literals;

namespace cavan {
  export struct error;

  export enum type {
    T_NULL,
    T_OPEN_TAG,
    T_CLOSE_TAG,
    T_TAG,
    T_TEXT,
    T_DIRECTIVE,
    T_END,
  };
  export struct token {
    jute::view text {};
    type type {};
  };
  export using tokens = hai::varray<token>;

  export struct excl {
    jute::view grp {};
    jute::view art {};
  };

  export struct pom;
  export struct dep {
    jute::view grp {};
    jute::view art {};
    jute::heap ver {};
    jute::view typ { "jar" };
    jute::view scp { "compile" };
    jute::view cls {};
    bool opt {};
    hai::sptr<hai::chain<excl>> exc {};
    pom * pom;
  };
  export using deps = hai::chain<dep>;

  export struct prop {
    jute::view key {};
    jute::view val {};
  };
  export using props = hai::chain<prop>;

  export struct pom {
    hai::cstr filename {};
    hai::cstr xml {};
    pom * ppom {};

    jute::view grp {};
    jute::view art {};
    jute::view ver {};

    struct {
      jute::view grp {};
      jute::view art {};
      jute::view ver {};
    } parent {};

    cavan::deps deps {};
    cavan::deps deps_mgmt {};
    cavan::props props {};

    hai::chain<jute::view> modules {};

    hashley::rowan prop_index {};
  };

  [[nodiscard]] constexpr bool match(const token & t, type tp) { return t.type == tp; }
  [[nodiscard]] constexpr bool match(const token & t, type tp, jute::view id) { return t.type == tp && t.text == id; }

  void take_tag(jute::view exp_id, const token *& t, jute::view * out);
  void take_if(const token *& t, jute::view id, auto && fn) {
    if (!match(*t, T_OPEN_TAG, id)) return;

    t++;
    for (; !match(*t, T_END) && !match(*t, T_CLOSE_TAG, id); t++) fn();

    if (!match(*t, T_CLOSE_TAG, id)) fail("missing end of " + id);
  }
  void take(const token *& t, jute::view id, auto && fn) {
    if (!match(*t, T_OPEN_TAG, id)) fail("missing expected tag " + id);

    take_if(t, id, fn);
  }

  export [[nodiscard]] tokens split_tokens(jute::view xml);

  export void lint_tag(const token *& t);
  export void lint_xml(const tokens & tokens);

  export [[nodiscard]] deps list_deps(const token *& t);

  export [[nodiscard]] pom * parse_pom(const tokens & tokens);
  export [[nodiscard]] pom * read_pom(jute::view file);
  export [[nodiscard]] pom * read_pom(jute::view grp, jute::view art, jute::view ver);
  export void read_parent_chain(pom * p);

  export void merge_props(pom * p);
  export jute::heap apply_props(pom * p, jute::heap str);

  export void eff_pom(pom * p);
} // namespace cavan
