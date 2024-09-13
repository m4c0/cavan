#pragma leco add_impl cavan_deps
#pragma leco add_impl cavan_lint
#pragma leco add_impl cavan_pom
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
  type type{};
};
export using tokens = hai::varray<token>;

export struct dep {
  hai::cstr grp{};
  hai::cstr art{};
  hai::cstr ver{};
  hai::cstr typ{"jar"_s.cstr()};
  hai::cstr scp{"compile"_s.cstr()};
  hai::cstr cls{};
  bool opt{};
  hashley::rowan exc{};
};
export using deps = hai::varray<dep>;

export struct prop {
  hai::cstr key {};
  hai::cstr val {};
};
export using props = hai::varray<prop>;

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
  cavan::props props {};
};

[[nodiscard]] constexpr bool match(const token &t, type tp) {
  return t.type == tp;
}
[[nodiscard]] constexpr bool match(const token &t, type tp, jute::view id) {
  return t.type == tp && t.text == id;
}

void take_tag(jute::view exp_id, const token *&t, hai::cstr *out);
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

export [[nodiscard]] tokens split_tokens(const hai::cstr & cstr);

export void lint_tag(const token *& t);
export void lint_xml(const tokens & tokens);

export [[nodiscard]] deps list_deps(const token *& t);

export [[nodiscard]] pom parse_pom(const tokens & tokens);
export [[nodiscard]] pom read_pom(const hai::cstr & xml);
export [[nodiscard]] pom read_pom(jute::view grp, jute::view art, jute::view ver);
} // namespace cavan
