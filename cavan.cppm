#pragma leco add_impl cavan_tokenizer
export module cavan;
import jute;
import hai;
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
};
export using deps = hai::varray<dep>;

export bool match(const token &t, type tp) { return t.type == tp; }
export bool match(const token &t, type tp, jute::view id) {
  return t.type == tp && t.id == id;
}

export mno::req<tokens> split_tokens(const hai::cstr &cstr);
export mno::req<tokens> read_tokens(yoyo::reader &r);
} // namespace cavan