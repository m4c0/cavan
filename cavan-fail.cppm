export module cavan:fail;
import jute;
import silog;

export namespace cavan {
  struct error {};

  [[noreturn]] inline void fail(const char * msg) {
    silog::log(silog::error, "%s", msg);
    throw error {};
  }
  template <auto N> [[noreturn]] inline void fail(jute::twine<N> msg) { fail(msg.cstr().begin()); }

  [[noreturn]] inline void whilst(const char * msg) {
    silog::log(silog::error, "while %s", msg);
    throw;
  }
  template <auto N> [[noreturn]] inline void whilst(jute::twine<N> msg) { whilst(msg.cstr().begin()); }
} // namespace cavan
