#include <optional>
#include <string>

enum struct ShaderStage { Vert, Frag };

class Shader {
public:
  Shader(ShaderStage stage, std::optional<std::string> texture) {}
};

template<int _state>
struct ShaderBuilder {
private:
  constexpr static int shader_stage_offset = 0;
  constexpr static int shader_stage_mask = (0b11 << shader_stage_offset);
  constexpr static int shader_stage_not_setted = (0b00 << shader_stage_offset);
  constexpr static int shader_stage_vertex = (0b01 << shader_stage_offset);
  constexpr static int shader_stage_fragment = (0b10 << shader_stage_offset);

  constexpr static int texture_offset = 2;
  constexpr static int texture_mask = 0b1;
  constexpr static int texture_not_setted = (0b0 << texture_offset);
  constexpr static int texture_setted = (0b1 << texture_offset);

  consteval bool is_shader_not_setted(int state) {
    return (state & shader_stage_mask) == shader_stage_not_setted;
  }
  consteval bool is_shader_setted(int state) {
    return
      (state & shader_stage_mask) == shader_stage_vertex ||
      (state & shader_stage_mask) == shader_stage_fragment;
  }
  consteval int set_vertex_shader_bits(int state) {
    return state | shader_stage_vertex;
  }
  consteval int set_fragment_shader_bits(int state) {
    return state | shader_stage_fragment;
  }

  consteval bool is_texture_not_setted(int state) {
    return (state & texture_mask) == texture_not_setted;
  }
  consteval bool shader_is_fragment(int state) {
    return (state & shader_stage_mask) == shader_stage_fragment;
  }
  consteval int set_texture_bit(int state) {
    return state | (texture_setted);
  }

  std::optional<std::string> _texture;

public:
  ShaderBuilder(std::optional<std::string> &&texture) {
    _texture = std::move(texture);
  }

  ShaderBuilder() {}

  auto set_vertex_stage() &&
    requires (is_shader_not_setted(_state)) {
      return ShaderBuilder<set_vertex_shader_bits(_state)>(std::move(_texture));
    }

  auto set_fragment_stage() &&
    requires (is_shader_not_setted(_state)) {
      return ShaderBuilder<set_fragment_shader_bits(_state)>(std::move(_texture));
    };

  auto set_texture(const std::string &texture) &&
    requires (shader_is_fragment(_state) && is_texture_not_setted(_state)) {
      _texture = texture;
      return ShaderBuilder<set_texture_bit(_state)>(std::move(_texture));
    }

  Shader build() && requires (is_shader_setted(_state)) {
    ShaderStage stage = _state & shader_stage_mask == shader_stage_vertex
      ? ShaderStage::Vert : ShaderStage::Frag;
    return Shader(stage, _texture);
  }
};


constexpr int default_state = 0;
auto start_build() {
  return ShaderBuilder<default_state>();
}

int main() {
  Shader shader_vert = start_build()
    .set_vertex_stage()
    .build();

  Shader shader_frag = start_build()
    .set_fragment_stage()
    .set_texture("tex")
    .build();

  // not compile

  // Shader shader_err1 = start_build()
  //   .set_vertex_stage()
  //   .set_texture("tex")
  //   .build();

  // Shader shader_err2 = start_build()
  //   .build();

  // Shader shader_err3 = start_build()
  //   .set_vertex_stage()
  //   .set_fragment_stage()
  //   .build();

  // auto some_state = start_build()
  //   .set_vertex_stage();
  // Shader shader_from_some_state = some_state.build();
}
