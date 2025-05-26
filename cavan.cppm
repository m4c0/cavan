#pragma leco add_impl cavan_deps
#pragma leco add_impl cavan_effpom
#pragma leco add_impl cavan_javac
#pragma leco add_impl cavan_lint
#pragma leco add_impl cavan_pom
#pragma leco add_impl cavan_profiles
#pragma leco add_impl cavan_props
#pragma leco add_impl cavan_tokenizer
export module cavan;
export import :depmap;
export import :objects;
export import :fail;
import jute;
import hai;
import hashley;

using namespace jute::literals;

namespace cavan {
  export struct error;

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

    hashley::niamh prop_index { 173 };
    bool effective {};
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

  [[nodiscard]] tokens split_tokens(jute::view xml);

  void lint_tag(const token *& t);
  void lint_xml(const tokens & tokens);

  [[nodiscard]] deps list_deps(const token *& t);
  void list_profiles(const token *& t, pom * res);

  export [[nodiscard]] hai::cstr path_of(jute::view grp, jute::view art, jute::view ver, jute::view type);

  export [[nodiscard]] pom * read_pom(jute::view file);
  export [[nodiscard]] pom * read_pom(jute::view grp, jute::view art, jute::view ver);
  export [[nodiscard]] pom * read_pom_of_source(jute::view java_file);
  export void read_parent_chain(pom * p);

  export void merge_props(pom * p);
  export jute::heap apply_props(pom * p, jute::heap str);

  export void eff_pom(pom * p);

  export [[nodiscard]] hai::array<pom *> read_modules(const pom * p);

  export hai::cstr generate_javac_argfile(pom * pom, bool test_scope, bool recurse);
} // namespace cavan
