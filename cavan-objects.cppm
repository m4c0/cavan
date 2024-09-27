export module cavan:objects;
import jute;
import hai;
import hashley;

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

  export struct prop {
    jute::view key {};
    jute::view val {};
  };
  export using props = hai::chain<prop>;
} // namespace cavan
