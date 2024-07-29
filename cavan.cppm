#pragma leco add_impl cavan_deps
#pragma leco add_impl cavan_lint
#pragma leco add_impl cavan_pom
#pragma leco add_impl cavan_tokenizer
export module cavan;
import jute;
import hai;
import hashley;
import missingno;
import yoyo;

using namespace jute::literals;

namespace cavan {
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
  hai::cstr id{};
  type type{};
};
export using tokens = hai::varray<token>;

export struct dep {
  hai::cstr grp{};
  hai::cstr art{};
  hai::cstr ver{};
  hai::cstr scp{"compile"_s.cstr()};
  bool opt{};
  hashley::rowan exc{};
};
export using deps = hai::varray<dep>;

export struct pom {
  hai::cstr filename{};

  hai::cstr grp{};
  hai::cstr art{};
  hai::cstr ver{};

  struct {
    hai::cstr grp{};
    hai::cstr art{};
    hai::cstr ver{};
  } parent{};

  cavan::deps deps{};
  cavan::deps deps_mgmt{};
};

[[nodiscard]] constexpr bool match(const token &t, type tp) {
  return t.type == tp;
}
[[nodiscard]] constexpr bool match(const token &t, type tp, jute::view id) {
  return t.type == tp && t.id == id;
}
[[nodiscard]] mno::req<void> take_tag(jute::view exp_id, const token *&t,
                                      hai::cstr *out);
[[nodiscard]] mno::req<void> take_if(const token *&t, jute::view id,
                                     auto &&fn) {
  if (!match(*t, T_OPEN_TAG, id))
    return {};

  t++;
  for (; !match(*t, T_END) && !match(*t, T_CLOSE_TAG, id); t++) {
    mno::req<void> res = fn();
    if (!res.is_valid())
      return res;
  }

  if (!match(*t, T_CLOSE_TAG, id))
    return mno::req<void>::failed("missing end of " + id);

  return {};
}
[[nodiscard]] mno::req<void> take(const token *&t, jute::view id, auto &&fn) {
  if (!match(*t, T_OPEN_TAG, id))
    return mno::req<void>::failed("missing expected tag " + id);

  return take_if(t, id, fn);
}

export [[nodiscard]] mno::req<tokens> split_tokens(const hai::cstr &cstr);
export [[nodiscard]] mno::req<tokens> read_tokens(yoyo::reader &r);

export [[nodiscard]] mno::req<void> lint_tag(const token *&t);
export [[nodiscard]] mno::req<void> lint_xml(const tokens &tokens);

export [[nodiscard]] mno::req<deps> list_deps(const token *&t);

export [[nodiscard]] mno::req<pom> parse_pom(const tokens &tokens);

export [[nodiscard]] mno::req<pom> read_pom(jute::view grp, jute::view art,
                                            jute::view ver);
} // namespace cavan
