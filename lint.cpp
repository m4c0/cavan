#pragma leco tool
import cavan;
import hai;
import jojo;
import jute;

using namespace cavan;

static void lint(void *, hai::cstr & xml) { lint_xml(split_tokens(xml)); }

int main(int argc, char ** argv) try {
  for (auto i = 1; i < argc; i++) {
    jojo::read(jute::view::unsafe(argv[i]), nullptr, lint);
  }
} catch (...) {
  return 1;
}
