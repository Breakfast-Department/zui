<p align="right">
  <a href="README.md">English</a> · Bahasa Indonesia
</p>

<p align="center">
  <img src="assets/logo/zui-logo.png" alt="Logo ZUI" width="180">
</p>

<h1 align="center">ZUI</h1>

<p align="center">
  Toolkit GUI berbasis C11 yang dibuat langsung untuk Wayland dan menggunakan EGL/OpenGL untuk rendering.
</p>

<p align="center">
  <a href="https://github.com/yusuf601/zui/actions/workflows/ci.yml"><img src="https://github.com/yusuf601/zui/actions/workflows/ci.yml/badge.svg" alt="Status CI"></a>
  <img src="https://img.shields.io/badge/C-11-00599C.svg" alt="C11">
  <img src="https://img.shields.io/badge/platform-Wayland-6C4FBB.svg" alt="Wayland">
  <a href="LICENSE"><img src="https://img.shields.io/badge/license-MIT-green.svg" alt="Lisensi MIT"></a>
</p>

> [!WARNING]
> ZUI masih dalam tahap early development. API dapat berubah tanpa pemberitahuan. Saat ini ZUI hanya mendukung Wayland dan belum menyediakan installation package, workflow integrasi eksternal yang stabil, atau automated test suite.

- [Overview](#overview)
- [Current Features](#current-features)
- [Project Status and Limitations](#project-status-and-limitations)
- [Requirements](#requirements)
- [Quick Start](#quick-start)
- [User Guide](#user-guide)
- [Public API Overview](#public-api-overview)
- [Developer Guide](#developer-guide)
- [Troubleshooting](#troubleshooting)
- [Roadmap](#roadmap)
- [Contribute](#contribute)
- [License](#license)

## Overview

ZUI menyediakan fondasi GUI native untuk aplikasi Linux yang berjalan dalam sesi Wayland. Toolkit ini terhubung langsung ke Wayland, membuat EGL-backed windows, merender widget melalui OpenGL, dan menyediakan public API berbasis C yang ringkas.

Proyek ini dapat digunakan untuk eksperimen, belajar, dan berkontribusi pada toolkit yang masih berada di tahap awal. ZUI belum ditujukan sebagai GUI framework yang production-ready atau cross-platform.

## Current Features

- Window Wayland native menggunakan XDG Shell.
- Pengelolaan EGL context dan surface.
- Rendering OpenGL untuk window, rectangle, text, icon, dan image.
- Titlebar custom dengan tombol close, minimize, maximize, dan hide bawaan.
- Label, tombol interaktif, icon SVG, dan image raster.
- Widget tree dengan padding, spacing, background color, dan rounded corners.
- Cursor widget yang dapat diatur.
- Font loading dari file atau memory.
- Build GCC dan Clang melalui CMake.

## Project Status and Limitations

Untuk saat ini, ZUI sengaja mempertahankan scope yang kecil:

- Linux dengan compositor Wayland adalah satu-satunya runtime yang didukung.
- Public API masih berkembang dan dapat berubah sewaktu-waktu.
- Proyek membangun target statis `zui`, tetapi belum memiliki aturan install/export CMake.
- Instalasi runtime assets belum diformalkan; shader, icon bawaan, dan logo harus tetap dapat diakses.
- Automated test suite dan public release belum tersedia.
- Backend X11, Windows, dan macOS belum diimplementasikan.

## Requirements

Anda membutuhkan:

- desktop Linux yang menjalankan sesi Wayland;
- compiler yang mendukung C11, seperti GCC atau Clang;
- CMake 3.16 atau versi yang lebih baru;
- pkg-config;
- Wayland client, EGL, dan cursor development headers serta libraries;
- EGL dan OpenGL development headers serta libraries.

Pasang dependency build untuk distribusi Anda.

### Debian / Ubuntu

```sh
sudo apt update
sudo apt install build-essential cmake pkg-config libwayland-dev libegl-dev libgl-dev
```

### Fedora

```sh
sudo dnf install gcc cmake pkgconf-pkg-config wayland-devel libglvnd-devel
```

### Arch Linux

```sh
sudo pacman -S --needed base-devel cmake pkgconf wayland libglvnd
```

Nama paket dapat berbeda pada versi distribusi yang lebih lama.

## Quick Start

Clone repository, lalu build ZUI:

```sh
git clone https://github.com/yusuf601/zui.git
cd zui
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
```

Opsi `ZUI_BUILD_EXAMPLES` aktif secara default. Jalankan contoh bawaan dari repository root:

```sh
./build/zui_example
```

Program harus dijalankan di dalam sesi Wayland yang aktif. Proses build juga menyalin runtime assets bawaan ZUI ke `build/assets/`.

## User Guide

### Application Lifecycle

Aplikasi ZUI mengikuti tujuh langkah:

1. Sertakan `zui/zui.h`.
2. Panggil `zui_init()`.
3. Buat `ZuiWindow`.
4. Susun titlebar dan content widget tree.
5. Tampilkan window.
6. Proses event dan render selama window masih berjalan.
7. Hancurkan window dan panggil `zui_shutdown()`.

Berikut aplikasi minimal yang hanya menggunakan API publik saat ini:

```c
#include <zui/zui.h>
#include <stdio.h>

static void on_click(ZuiWidget *widget, void *user_data)
{
    (void)widget;
    (void)user_data;
    puts("Hello from ZUI!");
}

int main(void)
{
    if (!zui_init()) {
        return 1;
    }

    ZuiWindow *window = zui_window_create(800, 500, "My ZUI App");
    if (!window) {
        zui_shutdown();
        return 1;
    }

    ZuiButton *button = zui_button_create("Click me");
    zui_button_on_click(button, on_click, NULL);
    zui_widget_add_child(zui_window_content(window), (ZuiWidget *)button);

    zui_window_show(window);
    while (zui_window_running(window)) {
        zui_poll_events();
        zui_window_render(window);
    }

    zui_window_destroy(window);
    zui_shutdown();
    return 0;
}
```

Lihat [`examples/main.c`](examples/main.c) untuk contoh lebih lengkap yang menggunakan titlebar, label, tombol dengan styling, callback, dan kontrol window.

Karena ZUI belum menyediakan aturan install/export, cara yang didukung saat ini adalah melakukan build dan eksperimen langsung di dalam repository. Workflow integrasi system-wide akan didokumentasikan setelah packaging dan instalasi runtime assets tersedia.

### Windows and Titlebars

Buat window dengan `zui_window_create(width, height, title)`. Setelah itu, Anda dapat mengatur title, corner radius, minimum size, maximum size, dan logo.

Setiap window menyediakan:

- `zui_window_titlebar()` untuk titlebar custom;
- `zui_window_content()` untuk content container utama.

Titlebar memiliki area start, center, dan end. Tambahkan widget dengan `zui_titlebar_set_start()`, `zui_titlebar_set_center()`, atau `zui_titlebar_set_end()`. Built-in window controls tersedia melalui fungsi close, minimize, maximize, dan hide button.

### Widgets and Callbacks

Buat teks dengan `zui_label_create()` dan kontrol interaktif dengan `zui_button_create()`. Callback klik menerima `ZuiWidget *` yang diklik dan pointer `user_data` yang didaftarkan melalui `zui_button_on_click()`.

Operasi widget umum mencakup:

- menampilkan atau menyembunyikan widget;
- mengatur warna latar;
- mengatur radius sudut;
- memilih cursor;
- memasangnya ke parent.

Selalu periksa nilai yang dikembalikan constructor. Alokasi memory atau resource loading dapat gagal dan menghasilkan pointer `NULL`.

### Layout and Styling

Tambahkan child dengan `zui_widget_add_child(parent, child)`. Window content container saat ini menyusun child secara vertikal dan meletakkannya di tengah. Public layout API menyediakan padding dan spacing; layout direction dan alignment belum dapat dikonfigurasi melalui public setter.

Warna menggunakan nilai RGBA floating-point yang dinormalisasi. Helper `ZUI_COLOR`, `ZUI_COLOR_RGB`, dan `ZUI_COLOR_HEX` mempersingkat deklarasi warna umum.

### Fonts, Icons, and Images

- Muat font dengan `zui_font_load()` atau `zui_font_load_memory()`, kemudian pasang dengan `zui_label_set_font()`.
- Muat sumber SVG dengan `zui_icon_load_svg()` atau `zui_icon_load_svg_data()`, kemudian buat `ZuiIcon` dengan ukuran yang dibutuhkan.
- Buat image raster dari file atau memory dengan `zui_image_create()` dan `zui_image_create_from_memory()`.

Implementasi icon dan image mengunggah texture saat digunakan oleh renderer. Pembuatan resource dapat gagal, jadi caller harus memeriksa pointer yang dikembalikan.

### Ownership and Cleanup

Menambahkan widget ke parent menjadikannya bagian dari widget tree tersebut. Menghancurkan parent akan menghancurkan seluruh child yang terpasang secara rekursif, jadi jangan menghancurkan child yang sama secara terpisah.

Font yang dimuat dan object `ZuiIconSource` memiliki fungsi destroy eksplisit. Memasang font ke label atau membuat icon dari icon source tidak memindahkan ownership resource mandiri tersebut; pertahankan resource selama masih digunakan dan hancurkan setelah widget yang bergantung padanya selesai digunakan.

Hancurkan window tingkat atas sebelum `zui_shutdown()`.

### Runtime Assets

ZUI saat ini bergantung pada shader bawaan serta menggunakan icon dan logo bawaan untuk kontrol window default. CMake menyalin `assets/` ke build tree, sedangkan runtime mencari shader di beberapa development path.

Sampai dukungan instalasi ditambahkan, jalankan contoh dari repository root atau build directory dan pertahankan salinan asset tree di samping executable. Font, file SVG, dan image milik aplikasi juga harus dapat dibaca dari path yang diberikan.

## Public API Overview

Public headers adalah referensi API utama.

| Area | Representative API | Header |
| --- | --- | --- |
| Lifecycle | `zui_init`, `zui_poll_events`, `zui_shutdown` | [`app.h`](include/zui/app.h) |
| Window | `zui_window_create`, titlebar, dan kontrol window | [`window.h`](include/zui/window.h) |
| Widget dan cursor | label, tombol, callback, `zui_widget_set_cursor` | [`widget.h`](include/zui/widget.h) |
| Layout | pemasangan child, padding, spacing | [`layout.h`](include/zui/layout.h) |
| Warna | `ZuiColor` dan macro warna | [`color.h`](include/zui/color.h) |
| Font | pemuatan file/memory dan metrik teks | [`font.h`](include/zui/font.h) |
| Icon SVG | pemuatan source, pengaturan ukuran, tint | [`icon.h`](include/zui/icon.h) |
| Image | pemuatan file/memory, ukuran, visibility | [`image.h`](include/zui/image.h) |

Gunakan [`zui/zui.h`](include/zui/zui.h) untuk mengakses seluruh public API.

## Developer Guide

### Repository Layout

| Path | Responsibility |
| --- | --- |
| `include/zui/` | Header publik yang tersedia untuk aplikasi. |
| `include/zui/internal/` | Internal interfaces yang digunakan oleh implementasi ZUI. |
| `src/core/` | Application state, window, widget, layout, font, icon, dan image. |
| `src/platform/wayland/` | Koneksi Wayland, input, window, cursor, dan generated protocol files. |
| `src/renderer/opengl/` | EGL setup dan rendering OpenGL. |
| `assets/` | Shader runtime, font bawaan, icon, dan logo proyek. |
| `examples/` | Program kecil yang menggunakan API publik. |
| `external/` | Source GLAD, NanoSVG, dan stb yang disertakan langsung. |

### Runtime Architecture

Alur runtime utama adalah:

```text
zui_init()
  -> connect to Wayland and discover globals
  -> initialize EGL
  -> create a window and its EGL surface
  -> initialize the OpenGL renderer on first render
  -> poll events, lay out widgets, and draw frames
  -> destroy windows and call zui_shutdown()
```

`src/core/app.c` menyimpan global state untuk platform, EGL, dan renderer. Backend Wayland menerjemahkan compositor dan input events menjadi operasi window/widget. Window rendering menghitung layout, menggambar widget tree, melakukan EGL buffer swap, dan menjadwalkan frame berikutnya.

### Development Builds

Buat build Debug dengan informasi debug tambahan dan tanpa optimisasi:

```sh
cmake -S . -B build-debug -DCMAKE_BUILD_TYPE=Debug
cmake --build build-debug --parallel
```

Build hanya library, tanpa `zui_example`:

```sh
cmake -S . -B build-lib -DZUI_BUILD_EXAMPLES=OFF
cmake --build build-lib --parallel
```

Target utama adalah:

- `glad` — loader OpenGL/EGL yang disertakan langsung;
- `zui` — library statis ZUI;
- `copy_assets` — menyalin asset bawaan ke build tree;
- `zui_example` — executable contoh ketika example diaktifkan.

### Coding Guidelines

- Gunakan C11 dan ikuti style yang sudah digunakan di file terkait.
- Letakkan deklarasi untuk aplikasi di `include/zui/` dan detail implementasi di `include/zui/internal/`.
- Pertahankan konfigurasi warning GCC/Clang yang ketat di `CMakeLists.txt`.
- Jaga perubahan tetap terfokus; hindari refactor yang tidak berkaitan.
- Periksa hasil setiap alokasi serta operasi platform atau resource yang dapat gagal.
- Jaga penambahan API publik tetap kecil serta dokumentasikan aturan ownership dan lifetime.
- Jangan mengedit generated Wayland protocol files secara manual kecuali memang sedang melakukan regenerasi.

### Verification and CI

Sebelum membuka pull request, jalankan configure dan build untuk kombinasi compiler serta configuration yang terdampak. Gunakan GCC dan Clang jika tersedia. Setidaknya, build aplikasi contoh dan pastikan tidak ada compiler warning baru.

Workflow CI saat ini melakukan build berikut:

| Compiler | Konfigurasi |
| --- | --- |
| GCC | Debug dan Release |
| Clang | Debug dan Release |

CI juga menjalankan clang-tidy, tetapi langkah tersebut saat ini tidak memblokir build. ZUI belum memiliki automated test suite; build yang berhasil tidak boleh dianggap sebagai validasi perilaku atau visual.

## Troubleshooting

### CMake tidak dapat menemukan dependency

Periksa modul pkg-config terlebih dahulu:

```sh
pkg-config --modversion wayland-client wayland-egl wayland-cursor egl
```

Jika ada modul yang tidak ditemukan, pasang development package terkait dari bagian [Requirements](#requirements), kemudian jalankan ulang CMake configure.

### ZUI tidak dapat terhubung ke Wayland

Pastikan proses berjalan dalam sesi Wayland:

```sh
echo "$XDG_SESSION_TYPE"
echo "$WAYLAND_DISPLAY"
```

`XDG_SESSION_TYPE` umumnya harus menghasilkan `wayland` dan `WAYLAND_DISPLAY` tidak boleh kosong. Sesi X11 murni tidak didukung.

### Inisialisasi EGL atau OpenGL gagal

Pastikan driver grafis menyediakan EGL dan OpenGL, kemudian periksa pesan error yang ditulis ZUI ke standard error. Sesi remote, nested, atau headless mungkin tidak menyediakan konfigurasi EGL yang dapat digunakan.

### Shader, icon, font, atau image tidak ditemukan

Jalankan contoh dari repository root dengan `./build/zui_example` atau dari build tree, lalu pastikan `build/assets/` tersedia. Untuk resource milik aplikasi, pastikan path yang diberikan dapat dibaca dari lokasi runtime aplikasi.

## Roadmap

Arah proyek saat ini sengaja dibuat luas:

- menstabilkan dan mendokumentasikan API publik;
- menambahkan dukungan install/export CMake dan packaging;
- memperkenalkan unit test dan integration test otomatis;
- memperluas contoh dan dokumentasi referensi;
- mengeksplorasi backend platform tambahan setelah backend Wayland matang.

Belum ada release schedule yang ditetapkan.

## Contribute

Issue dan pull request yang terfokus dipersilakan. Sebelum berkontribusi:

1. Baca [Developer Guide](#developer-guide).
2. Reproduksi perilaku di Wayland.
3. Jaga scope perubahan dan perbarui kedua bahasa README ketika dokumentasi berubah.
4. Lakukan build dengan konfigurasi GCC dan Clang yang relevan.
5. Jelaskan apa yang telah diverifikasi dan apa yang belum dapat diuji secara visual.

## License

ZUI tersedia di bawah [Lisensi MIT](LICENSE).

Copyright © 2026 Breakfast-Department.
