
#include "catch.hpp"
#include "src/InputParser.h"
#include "src/trimming.h"
#include "src/packing.h"
#include "src/output.h"
#include "src/debug.h"
#include <sstream>

using namespace spright;

namespace {
  template<typename T>
  bool le_size(const T& slice, int w, int h) {
    // here one can set a breakpoint to tighten the size constraints
    if (slice.width * slice.height < w * h)
      return true;

    if (slice.width * slice.height > w * h)
      return false;

    return true;
  }

  std::vector<Slice> pack(const char* definition) {
    auto input = std::stringstream(definition);
    auto parser = InputParser(Settings{ });
    parser.parse(input);
    static auto s_sprites = std::vector<Sprite>();
    s_sprites = std::move(parser).sprites();
    for (auto& sprite : s_sprites)
      trim_sprite(sprite);
    return pack_sprites(s_sprites);
  }

  [[maybe_unused]] void dump(Slice slice) {
    static auto i = 0;
    const auto filename = FilenameSequence("dump-{000-}.png").get_nth_filename(i++);
    auto image = get_slice_image(slice);
    draw_debug_info(image, slice);
    save_image(image, filename);
  }

  Slice pack_single_sheet(const char* definition) {
    auto slices = std::vector<Slice>();
    REQUIRE_NOTHROW(slices = pack(definition));
    CHECK(slices.size() == 1);
    if (slices.size() != 1)
      return { };
  #if 0
    dump(slices[0]);
  #endif
    return slices[0];
  };
} // namespace

TEST_CASE("packing - Basic") {
  auto slice = pack_single_sheet(R"(
    input "test/Items.png"
      colorkey
      atlas
  )");
  CHECK(le_size(slice, 59, 58));

  slice = pack_single_sheet(R"(
    allow-rotate true
    input "test/Items.png"
      colorkey
      atlas
  )");
  CHECK(le_size(slice, 59, 58));

  slice = pack_single_sheet(R"(
    duplicates share
    input "test/Items.png"
      colorkey
      atlas
  )");
  CHECK(le_size(slice, 55, 58));

  slice = pack_single_sheet(R"(
    allow-rotate true
    duplicates share
    input "test/Items.png"
      colorkey
      atlas
  )");
  CHECK(le_size(slice, 55, 58));

  slice = pack_single_sheet(R"(
    max-width 128
    max-height 128
    input "test/Items.png"
      colorkey
      atlas
  )");
  CHECK(slice.width <= 128);
  CHECK(slice.height <= 128);
  CHECK(le_size(slice, 59, 58));

  slice = pack_single_sheet(R"(
    width 128
    max-height 128
    input "test/Items.png"
      colorkey
      atlas
  )");
  CHECK(slice.width == 128);
  CHECK(slice.height <= 128);
  CHECK(le_size(slice, 128, 32));

  slice = pack_single_sheet(R"(
    max-width 128
    height 128
    input "test/Items.png"
      colorkey
      atlas
  )");
  CHECK(slice.width <= 128);
  CHECK(slice.height == 128);
  CHECK(le_size(slice, 30, 128));

  slice = pack_single_sheet(R"(
    max-width 40
    input "test/Items.png"
      colorkey
      atlas
  )");
  CHECK(slice.width <= 40);
  CHECK(le_size(slice, 40, 85));

  slice = pack_single_sheet(R"(
    max-height 40
    input "test/Items.png"
      colorkey
      atlas
  )");
  CHECK(slice.height <= 40);
  CHECK(le_size(slice, 89, 38));

  slice = pack_single_sheet(R"(
    power-of-two true
    input "test/Items.png"
      colorkey
      atlas
  )");
  CHECK(slice.width == 64);
  CHECK(slice.height == 64);

  slice = pack_single_sheet(R"(
    padding 1
    input "test/Items.png"
      colorkey
      atlas
  )");
  CHECK(le_size(slice, 63, 65));

  slice = pack_single_sheet(R"(
    padding 1
    power-of-two true
    input "test/Items.png"
      colorkey
      atlas
  )");
  CHECK(ceil_to_pot(slice.width) == slice.width);
  CHECK(ceil_to_pot(slice.height) == slice.height);
  CHECK(le_size(slice, 64, 128));

  slice = pack_single_sheet(R"(
    max-height 16
    common-divisor 16
    input "test/Items.png"
      colorkey
      atlas
  )");
  CHECK(slice.height <= 16);
  CHECK(le_size(slice, 496, 16));

  slice = pack_single_sheet(R"(
    padding 0 1
    common-divisor 16
    max-height 20
    input "test/Items.png"
      colorkey
      atlas
  )");
  CHECK(slice.height <= 20);
  CHECK(le_size(slice, 498, 18));

  slice = pack_single_sheet(R"(
    padding 1
    common-divisor 16
    max-height 30
    input "test/Items.png"
      colorkey
      atlas
  )");
  CHECK(slice.height <= 30);
  CHECK(le_size(slice, 528, 18));

  slice = pack_single_sheet(R"(
    padding 1 0
    common-divisor 16
    max-height 20
    input "test/Items.png"
      colorkey
      atlas
  )");
  CHECK(slice.height <= 20);
  CHECK(le_size(slice, 526, 16));

  slice = pack_single_sheet(R"(
    max-height 30
    common-divisor 24
    extrude 1
    input "test/Items.png"
      colorkey
      atlas
  )");
  CHECK(slice.height <= 30);
  CHECK(le_size(slice, 806, 26));

  slice = pack_single_sheet(R"(
    padding 8 0
    common-divisor 16
    input "test/Items.png"
      colorkey
      atlas
  )");
  CHECK(le_size(slice, 136, 136));

  slice = pack_single_sheet(R"(
    padding 8 0
    duplicates share
    allow-rotate
    common-divisor 16
    input "test/Items.png"
      colorkey
      atlas
  )");
  CHECK(le_size(slice, 112, 112));

  slice = pack_single_sheet(R"(
    padding 8 0
    max-width 16
    input "test/Items.png"
      colorkey
      atlas
  )");
  CHECK(slice.width == 16);
  CHECK(le_size(slice, 492, 16));

  slice = pack_single_sheet(R"(
    padding 8 1
    max-height 18
    input "test/Items.png"
      colorkey
      atlas
  )");
  CHECK(slice.height == 18);
  CHECK(le_size(slice, 478, 18));
}

TEST_CASE("packing - Errors") {
  CHECK_THROWS(pack(R"(
    padding 1
    max-width 16
    max-height 16
    input "test/Items.png"
      colorkey
      atlas
  )"));

  CHECK_THROWS(pack(R"(
    padding 0 1
    max-width 16
    max-height 16
    input "test/Items.png"
      colorkey
      atlas
  )"));
}

TEST_CASE("packing - Multiple sheets") {
  auto slices = pack(R"(
    allow-rotate
    duplicates share
    max-width 30
    square
    power-of-two
    input "test/Items.png"
      colorkey
      atlas
  )");
  REQUIRE(slices.size() == 13);
  CHECK(slices[0].width <= 30);
  CHECK(slices[0].width == slices[0].height);
  CHECK(ceil_to_pot(slices[12].width) == slices[12].width);
  CHECK(slices[12].width == slices[12].height);

  CHECK_NOTHROW(slices = pack(R"(
    max-width 40
    max-height 40
    input "test/Items.png"
      colorkey
      atlas
  )"));
  REQUIRE(slices.size() == 3);
  CHECK(le_size(slices[0], 39, 40));
  CHECK(le_size(slices[1], 38, 40));
  CHECK(le_size(slices[2], 16, 30));

  CHECK_NOTHROW(slices = pack(R"(
    max-width 40
    max-height 40
    square
    input "test/Items.png"
      colorkey
      atlas
  )"));
  REQUIRE(slices.size() == 3);
  CHECK(le_size(slices[0], 40, 40));
  CHECK(le_size(slices[1], 32, 32));
  CHECK(slices[2].width == slices[2].height);

  CHECK_NOTHROW(slices = pack(R"(
    max-width 40
    max-height 40
    power-of-two true
    input "test/Items.png"
      colorkey
      atlas
  )"));
  REQUIRE(slices.size() == 4);
  CHECK(le_size(slices[0], 32, 32));
  CHECK(le_size(slices[1], 32, 32));
  CHECK(le_size(slices[2], 32, 32));
  CHECK(le_size(slices[3], 32, 16));

  slices = pack("");
  CHECK(slices.size() == 0);

  CHECK_THROWS(pack("padding 1"));

  CHECK_NOTHROW(slices = pack(R"(
    max-width 16
    max-height 16
    input "test/Items.png"
      colorkey
      atlas
  )"));
  REQUIRE(slices.size() == 14);
  CHECK(slices[0].width <= 16);
  CHECK(slices[0].height <= 16);

  CHECK_NOTHROW(slices = pack(R"(
    padding 1 0
    max-width 16
    max-height 16
    input "test/Items.png"
      colorkey
      atlas
  )"));
  REQUIRE(slices.size() == 15);
  CHECK(slices[0].width <= 16);
  CHECK(slices[0].height <= 16);
}
