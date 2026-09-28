#include <stdio.h>
#include <string.h>

#include <zui/version.h>

static void print_version(void)
{
  printf("Zui %s\n", ZUI_VERSION);
}

static void print_help(const char *prog)
{
  printf("Zui Development Tool\n\n");

  printf("Commands:\n");
  printf("  --info        Show Zui environment information\n\n");
  printf("  --version   Show Zui version\n");
  printf("  --help      Show this help message\n");
}

static void print_info(void)
{
  printf("Zui %s\n\n", ZUI_VERSION);
  printf("Description: Lightweight modern GUI toolkit for Wayland\n");
  printf("Backend:     Wayland\n");
  printf("Renderer:    OpenGL\n");
  printf("Language:    C11\n");
}

int main(int argc, char *argv[])
{
  if (argc == 1) {
    print_help(argv[0]);
    return 0;
  }

  const char *arg = argv[1];

  if (strcmp(arg, "--version") == 0 ||
    strcmp(arg, "-v") == 0) {
    print_version();
    return 0;
  }

  if (strcmp(arg, "--help") == 0||
    strcmp(arg, "-h") == 0) {
    print_help(argv[0]);
    return 0;
  }

  if (strcmp(arg, "--info") == 0 || strcmp(arg, "-i") == 0) {
    print_info();
    return 0;
  }

  fprintf(stderr, "zuic: unknown command '%s'\n", arg);
  fprintf(stderr, "Try '%s --help' for more information.\n", argv[0]);

  return 1;
}
