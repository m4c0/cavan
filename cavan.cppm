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

export bool match(const token &t, type tp) { return t.type == tp; }
export bool match(const token &t, type tp, jute::view id) {
  return t.type == tp && t.id == id;
}

export [[nodiscard]] mno::req<tokens> split_tokens(const hai::cstr &cstr);
export [[nodiscard]] mno::req<tokens> read_tokens(yoyo::reader &r);

export [[nodiscard]] mno::req<void> lint_xml(const tokens &tokens);

export [[nodiscard]] mno::req<deps> list_deps(const tokens &ts);

export [[nodiscard]] mno::req<pom> parse_pom(const tokens &tokens);
} // namespace cavan
