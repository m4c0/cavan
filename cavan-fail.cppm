export module cavan:fail;
import jute;
import silog;

namespace cavan {
  export struct error {};

  inline void fail(const char * msg) {
    silog::log(silog::error, "Failed to tokenise: %s", msg);
    throw error {};
  }

  template <auto N> inline void fail(jute::twine<N> msg) { fail(msg.cstr().begin()); }
} // namespace cavan
