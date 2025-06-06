#pragma leco tool

import cavan;
import jojo;
import jute;
import mtime;
import print;

int main(int argc, char ** argv) try {
  const auto shift = [&] { return jute::view::unsafe(argc == 1 ? "" : (--argc, *++argv)); };

  auto file = shift();
  if (file == "") die("missing file");

  cavan::file_reader = jojo::read_cstr;

  auto pom = cavan::read_pom(file);

  for (auto & p : cavan::resolve_classpath(pom)) putln(p);
} catch (...) {
  return 3;
}
