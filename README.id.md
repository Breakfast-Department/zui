<p align="right">
  <a href="README.md">English</a> · Bahasa Indonesia
</p>

<p align="center">
  <img src="assets/logo/zui-logo.png" alt="Logo ZUI" width="180">
</p>

<h1 align="center">ZUI</h1>

<p align="center">
  Toolkit GUI C11 tahap awal yang dibuat langsung untuk Wayland dan dirender dengan EGL/OpenGL.
</p>

<p align="center">
  <a href="https://github.com/yusuf601/zui/actions/workflows/ci.yml"><img src="https://github.com/yusuf601/zui/actions/workflows/ci.yml/badge.svg" alt="Status CI"></a>
  <img src="https://img.shields.io/badge/C-11-00599C.svg" alt="C11">
  <img src="https://img.shields.io/badge/platform-Wayland-6C4FBB.svg" alt="Wayland">
  <a href="LICENSE"><img src="https://img.shields.io/badge/license-MIT-green.svg" alt="Lisensi MIT"></a>
</p>

> [!WARNING]
> ZUI masih dalam tahap pengembangan awal. API dapat berubah tanpa pemberitahuan. Saat ini ZUI hanya menargetkan Wayland dan belum menyediakan paket instalasi, alur konsumsi eksternal yang stabil, atau rangkaian pengujian otomatis.

- [Ikhtisar](#ikhtisar)
- [Fitur Saat Ini](#fitur-saat-ini)
- [Status dan Batasan Proyek](#status-dan-batasan-proyek)
- [Kebutuhan Sistem](#kebutuhan-sistem)
- [Mulai Cepat](#mulai-cepat)
- [Panduan Pengguna](#panduan-pengguna)
- [Ikhtisar API Publik](#ikhtisar-api-publik)
- [Panduan Developer](#panduan-developer)
- [Pemecahan Masalah](#pemecahan-masalah)
- [Roadmap](#roadmap)
- [Berkontribusi](#berkontribusi)
- [Lisensi](#lisensi)

## Ikhtisar

ZUI menyediakan fondasi GUI native berukuran kecil untuk aplikasi Linux yang berjalan dalam sesi Wayland. ZUI terhubung langsung ke Wayland, membuat window berbasis EGL, merender widget melalui OpenGL, dan menyediakan API C yang ringkas.

Proyek ini sudah dapat digunakan untuk eksperimen, belajar, dan berkontribusi pada toolkit yang masih muda. ZUI belum ditujukan sebagai framework GUI yang siap produksi atau lintas platform.

## Fitur Saat Ini

- Window Wayland native menggunakan XDG Shell.
- Pengelolaan context dan surface EGL.
- Rendering OpenGL untuk window, persegi, teks, icon, dan image.
- Titlebar custom dengan tombol close, minimize, maximize, dan hide bawaan.
- Label, tombol interaktif, icon SVG, dan image raster.
- Tree widget dengan padding, spacing, warna latar, dan sudut membulat.
- Cursor widget yang dapat diatur.
- Pemuatan font dari file atau memory.
- Build GCC dan Clang melalui CMake.

## Status dan Batasan Proyek

Saat ini ZUI memiliki cakupan yang sengaja dibuat kecil:

- Linux dengan compositor Wayland adalah satu-satunya runtime yang didukung.
- API publik masih berkembang dan dapat berubah di antara commit.
- Proyek membangun target statis `zui`, tetapi belum memiliki aturan install/export CMake.
- Instalasi asset runtime belum diformalkan; shader, icon bawaan, dan logo harus tetap dapat diakses.
- Pengujian otomatis dan release publik belum tersedia.
- Backend X11, Windows, dan macOS belum diimplementasikan.

## Kebutuhan Sistem

Anda membutuhkan:

- desktop Linux yang menjalankan sesi Wayland;
- compiler yang mendukung C11, seperti GCC atau Clang;
- CMake 3.16 atau versi yang lebih baru;
- pkg-config;
- file development Wayland client, EGL, dan cursor;
- file development EGL dan OpenGL.

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

Nama paket dapat berbeda pada release distribusi yang lebih lama.

## Mulai Cepat

Clone dan build ZUI:

```sh
git clone https://github.com/yusuf601/zui.git
cd zui
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
```

Opsi `ZUI_BUILD_EXAMPLES` aktif secara default. Jalankan contoh bawaan dari root repository:

```sh
./build/zui_example
```

Program harus dijalankan di dalam sesi Wayland yang dapat digunakan. Proses build juga menyalin asset runtime bawaan ZUI ke `build/assets/`.

## Panduan Pengguna

### Lifecycle Aplikasi

Aplikasi ZUI mengikuti tujuh langkah:

1. Sertakan `zui/zui.h`.
2. Panggil `zui_init()`.
3. Buat `ZuiWindow`.
4. Susun titlebar dan tree widget content.
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

Karena ZUI belum menyediakan aturan install/export, alur awal yang didukung adalah melakukan build dan eksperimen di dalam repository. Prosedur integrasi system-wide yang stabil akan didokumentasikan setelah packaging dan instalasi asset runtime tersedia.

### Window dan Titlebar

Buat window dengan `zui_window_create(width, height, title)`. Setelah itu, Anda dapat mengatur judul, radius sudut, ukuran minimum, ukuran maksimum, dan logo.

Setiap window menyediakan:

- `zui_window_titlebar()` untuk titlebar custom;
- `zui_window_content()` untuk container content utama.

Titlebar memiliki area start, center, dan end. Tambahkan widget dengan `zui_titlebar_set_start()`, `zui_titlebar_set_center()`, atau `zui_titlebar_set_end()`. Widget kontrol window bawaan tersedia melalui fungsi tombol close, minimize, maximize, dan hide.

### Widget dan Callback

Buat teks dengan `zui_label_create()` dan kontrol interaktif dengan `zui_button_create()`. Callback klik menerima `ZuiWidget *` yang diklik dan pointer `user_data` yang didaftarkan melalui `zui_button_on_click()`.

Operasi widget umum mencakup:

- menampilkan atau menyembunyikan widget;
- mengatur warna latar;
- mengatur radius sudut;
- memilih cursor;
- memasangnya ke parent.

Selalu periksa nilai hasil constructor yang mengembalikan pointer ketika kegagalan alokasi atau pemuatan resource perlu ditangani.

### Layout dan Styling

Tambahkan child dengan `zui_widget_add_child(parent, child)`. Container content window saat ini menyusun child secara vertikal dan meletakkannya di tengah. Kontrol layout publik saat ini menyediakan padding dan spacing; arah layout dan alignment belum dapat dikonfigurasi melalui setter publik.

Warna menggunakan nilai RGBA floating-point yang dinormalisasi. Helper `ZUI_COLOR`, `ZUI_COLOR_RGB`, dan `ZUI_COLOR_HEX` mempersingkat deklarasi warna umum.

### Font, Icon, dan Image

- Muat font dengan `zui_font_load()` atau `zui_font_load_memory()`, kemudian pasang dengan `zui_label_set_font()`.
- Muat sumber SVG dengan `zui_icon_load_svg()` atau `zui_icon_load_svg_data()`, kemudian buat `ZuiIcon` dengan ukuran yang dibutuhkan.
- Buat image raster dari file atau memory dengan `zui_image_create()` dan `zui_image_create_from_memory()`.

Implementasi icon dan image mengunggah texture ketika digunakan oleh renderer. Pembuatan resource dapat gagal, jadi caller harus memeriksa pointer yang dikembalikan.

### Ownership dan Cleanup

Menambahkan widget ke parent menjadikannya bagian dari tree widget tersebut. Menghancurkan parent akan menghancurkan seluruh child yang terpasang secara rekursif, jadi jangan menghancurkan child yang sama secara terpisah.

Font yang dimuat dan object `ZuiIconSource` memiliki fungsi destroy eksplisit. Memasang font ke label atau membuat icon dari icon source tidak memindahkan ownership resource mandiri tersebut; pertahankan resource selama masih digunakan dan hancurkan setelah widget yang bergantung padanya selesai digunakan.

Hancurkan window tingkat atas sebelum `zui_shutdown()`.

### Asset Runtime

ZUI saat ini bergantung pada file shader bawaan dan menggunakan icon serta logo bawaan untuk kontrol window default. CMake menyalin `assets/` ke build tree, sedangkan runtime mencari shader di beberapa lokasi yang ditujukan untuk development.

Sampai dukungan instalasi ditambahkan, jalankan contoh dari root repository atau direktori build dan pertahankan salinan tree asset di samping executable. Font, file SVG, dan image milik aplikasi juga harus dapat dibaca dari path yang diberikan.

## Ikhtisar API Publik

Header publik adalah referensi API yang otoritatif.

| Area | API representatif | Header |
| --- | --- | --- |
| Lifecycle | `zui_init`, `zui_poll_events`, `zui_shutdown` | [`app.h`](include/zui/app.h) |
| Window | `zui_window_create`, titlebar, dan kontrol window | [`window.h`](include/zui/window.h) |
| Widget dan cursor | label, tombol, callback, `zui_widget_set_cursor` | [`widget.h`](include/zui/widget.h) |
| Layout | pemasangan child, padding, spacing | [`layout.h`](include/zui/layout.h) |
| Warna | `ZuiColor` dan macro warna | [`color.h`](include/zui/color.h) |
| Font | pemuatan file/memory dan metrik teks | [`font.h`](include/zui/font.h) |
| Icon SVG | pemuatan source, pengaturan ukuran, tint | [`icon.h`](include/zui/icon.h) |
| Image | pemuatan file/memory, ukuran, visibility | [`image.h`](include/zui/image.h) |

Sertakan [`zui/zui.h`](include/zui/zui.h) saat Anda membutuhkan seluruh permukaan API publik.

## Panduan Developer

### Struktur Repository

| Path | Tanggung jawab |
| --- | --- |
| `include/zui/` | Header publik yang tersedia untuk aplikasi. |
| `include/zui/internal/` | Interface privat yang digunakan bersama oleh unit implementasi ZUI. |
| `src/core/` | State aplikasi, window, widget, layout, font, icon, dan image. |
| `src/platform/wayland/` | Koneksi Wayland, input, window, cursor, dan protocol yang dihasilkan. |
| `src/renderer/opengl/` | Setup EGL dan rendering OpenGL. |
| `assets/` | Shader runtime, font bawaan, icon, dan logo proyek. |
| `examples/` | Program kecil yang menggunakan API publik. |
| `external/` | Source GLAD, NanoSVG, dan stb yang disertakan langsung. |

### Arsitektur Runtime

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

`src/core/app.c` memiliki state global platform, EGL, dan renderer. Backend Wayland menerjemahkan event compositor dan input menjadi operasi window/widget. Rendering window menghitung layout, menggambar tree widget, menukar buffer EGL, dan menjadwalkan frame berikutnya.

### Build Development

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

### Panduan Coding

- Gunakan C11 dan ikuti format file di sekitar kode yang diubah.
- Letakkan deklarasi untuk aplikasi di `include/zui/` dan detail implementasi di `include/zui/internal/`.
- Pertahankan konfigurasi warning GCC/Clang yang ketat di `CMakeLists.txt`.
- Jaga perubahan tetap terfokus; hindari refactor yang tidak berkaitan.
- Periksa setiap alokasi, object platform, dan resource eksternal pada batas tempat kegagalan dapat terjadi.
- Jaga penambahan API publik tetap kecil serta dokumentasikan aturan ownership dan lifetime.
- Jangan mengedit file protocol Wayland hasil generate secara manual kecuali memang sedang melakukan regenerasi.

### Verifikasi dan CI

Sebelum membuka pull request, lakukan configure dan build kombinasi yang terdampak menggunakan GCC dan Clang jika tersedia. Setidaknya, build aplikasi contoh dan pastikan tidak ada warning compiler baru.

Workflow CI saat ini melakukan build berikut:

| Compiler | Konfigurasi |
| --- | --- |
| GCC | Debug dan Release |
| Clang | Debug dan Release |

CI juga menjalankan clang-tidy, tetapi langkah tersebut saat ini tidak memblokir build. ZUI belum memiliki rangkaian pengujian otomatis; build yang berhasil tidak boleh dianggap sebagai validasi perilaku atau visual.

## Pemecahan Masalah

### CMake tidak dapat menemukan dependency

Periksa modul pkg-config terlebih dahulu:

```sh
pkg-config --modversion wayland-client wayland-egl wayland-cursor egl
```

Jika ada modul yang tidak ditemukan, pasang paket development terkait dari bagian [Kebutuhan Sistem](#kebutuhan-sistem), kemudian jalankan configure kembali.

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

Jalankan contoh dari root repository dengan `./build/zui_example` atau dari build tree, dan pastikan `build/assets/` tersedia. Untuk resource milik aplikasi, pastikan path yang diberikan dapat dibaca dari lokasi runtime aplikasi.

## Roadmap

Arah proyek saat ini sengaja dibuat luas:

- menstabilkan dan mendokumentasikan API publik;
- menambahkan dukungan install/export CMake dan packaging;
- memperkenalkan unit test dan integration test otomatis;
- memperluas contoh dan dokumentasi referensi;
- mengeksplorasi backend platform tambahan setelah backend Wayland matang.

Belum ada tanggal release yang dijanjikan.

## Berkontribusi

Issue dan pull request yang terfokus dipersilakan. Sebelum berkontribusi:

1. Baca [Panduan Developer](#panduan-developer).
2. Reproduksi perilaku di Wayland.
3. Jaga scope perubahan dan perbarui kedua bahasa README ketika dokumentasi berubah.
4. Lakukan build dengan konfigurasi GCC dan Clang yang relevan.
5. Jelaskan apa yang telah diverifikasi dan apa yang belum dapat diuji secara visual.

## Lisensi

ZUI tersedia di bawah [Lisensi MIT](LICENSE).

Copyright © 2026 Breakfast-Department.
