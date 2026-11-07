#ifndef GAME_NPC_HPP
#define GAME_NPC_HPP

#include <SFML/Graphics.hpp>
#include "Entity.hpp"
#include <stdlib.h>

/**
 * @file NPC.hpp
 * @brief A non-player entity with a visible sprite.
 *
 * NPC is an Entity plus a sprite for on-screen display. The sprite is a public
 * member: the game loop reads and writes it directly each frame to position and
 * draw the NPC.
 */

class NPC {
public:

    sf::Texture m_texture;

    sf::Sprite  m_sprite;

    RigidBody2D body;        ///< Motion state (position, velocity, acceleration).
    Collision2D collision;   ///< Collision shape (half-extents).

    /// While false, the NPC is ignored by simulation and collision passes.
    bool active{true};

    /// Numeric identifier. Zero means "not yet assigned".
    EntityId id{0};

    /// Convenience: current world-space bounding box (position + shape).
    [[nodiscard]] AABB bounds() const noexcept {
        return collision.bounds(body.position);
    }

    /// Load an image file into the NPC's texture.
    /// Associate the NPC's sprite with the texture.
    /// If fails to load the image file, terminates the program immediately,
    ///   with an error message displayed on the terminal.
    void sprite (std::string path);


};

#endif // GAME_NPC_HPP
