#pragma once

#include "input.h"
#include "FilenameSequence.h"

namespace spright {

enum class Definition {
  none,
  group,

  output,
  width,
  height,
  max_width,
  max_height,
  power_of_two,
  square,
  align_width,
  allow_rotate,
  padding,
  duplicates,
  alpha,
  pack,

  path,
  input,
  colorkey,
  grid,
  grid_cells,
  grid_offset,
  grid_spacing,
  row,
  skip,
  span,
  atlas,
  layers,

  sprite,
  id,
  rect,
  pivot,
  tag,
  trim,
  trim_threshold,
  trim_margin,
  trim_channel,
  crop,
  extrude,
  common_divisor,

  MAX
};

struct State {
  Definition definition{ };
  int level;
  std::string indent;
  int current_grid_cell_x{ };
  int current_grid_cell_y{ };
  int current_sequence_index{ };

  std::filesystem::path output;
  int width{ };
  int height{ };
  int max_width{ };
  int max_height{};
  bool power_of_two{ };
  bool square{ };
  int align_width{ };
  bool allow_rotate{ };
  int border_padding{ };
  int shape_padding{ };
  Duplicates duplicates{ };
  Alpha alpha{ };
  RGBA alpha_colorkey{ };
  Pack pack{ };

  std::filesystem::path path;
  FilenameSequence sheet;
  std::string default_layer_suffix;
  std::vector<std::string> layer_suffixes;
  RGBA colorkey{ };
  std::map<std::string, std::string> tags;
  std::string sprite_id;
  Size grid{ };
  Size grid_cells{ };
  Size grid_offset{ };
  Size grid_spacing{ };
  Size span{ 1, 1 };
  int atlas_merge_distance{ -1 };
  Pivot pivot{ PivotX::center, PivotY::middle };
  PointF pivot_point{ };
  Rect rect{ };
  Trim trim{ Trim::rect };
  int trim_threshold{ 1 };
  int trim_margin{ };
  bool trim_gray_levels{ };
  bool crop{ };
  int extrude{ };
  Size common_divisor{ 1, 1 };
};

Definition get_definition(std::string_view command);
Definition get_affected_definition(Definition definition);
std::string_view get_definition_name(Definition definition);

void apply_definition(State& state,
    Definition definition,
    std::vector<std::string_view>& arguments);

bool has_grid(const State& state);

} // namespace