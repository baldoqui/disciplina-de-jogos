#include <cstdlib>
#include <iostream>

#include "NPC.hpp"

void NPC::sprite(std::string path) {
  if (!m_texture.loadFromFile(path)) {
    std::cerr << "NPC::sprite: could not load image file '" << path << "'\n";
    std::exit(EXIT_FAILURE);
  }
  // resetRect = true so the sprite shows the whole new texture.
  m_sprite.setTexture(m_texture, true);
}
